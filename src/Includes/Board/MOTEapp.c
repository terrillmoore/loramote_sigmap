/********************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************/

/** INCLUDES *******************************************************/
#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
// Application Includes
#include "HardwareProfile.h"
#include "pin_manager.h"
#include "MOTEapp.h"
#include "USBapp.h"
#include "SSD1306oLED.h"
// Communication Includes
#include "eusart.h"
#include "buttons.h"
#include "TMRapp.h"
#include "tmr1.h"
#include "tmr2.h"
#include "input.h"
#include "adc.h"
#include "SSD1306oLED.h"
#include "memory.h"

typedef enum {moteStartup, moteNextDr, moteReset, moteSetDevaddr, moteSetAppskey, moteSetNwkskey, moteSetAdr, moteSetRx2, moteSetSync, moteSetChStatus, moteJoinAbp, moteWaitAccepted, moteIniTx, moteWaitIniTxOk, moteRest, moteTxUncnf, moteTxWait, moteDone, moteComFailure} MOTE_T;
MOTE_T  activeState = moteStartup;

static uint8_t modemResonseBuffer[64];
static uint8_t responsePortNum[3];
static uint8_t responseId[16];
static uint8_t responseData[8];

static uint8_t cmd[64];

static uint8_t modemPairingCode[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static bool responseReady = false;
static MOTE_MODE_T moteState = MOTE_STARTUP;

static bool autoMode = false;
static bool manualTrigger = false;
static uint8_t S1 = 1;
static uint8_t S2 = 1;
static uint8_t prevS1 = 1; // auto/manual
static uint8_t prevS2 = 1; // manual send

//typedef enum {runningEnter, runningSensorPrepare, runningSensorDisplay, runningPrepareTx, runningSendTx, runningPeriodic, runningSleep} MOTE_RUNNING_T;
//MOTE_RUNNING_T  runningState = runningEnter;

typedef enum {runningEnter, runningSensor, runningSensorMenu, runningDataRateDisplay, runningDataRateSelect, runningSensorDisplay, runningUplink, runningUplinkSelect, runningUplinkTransmit, runningPortNumber, runningDownlink, runningDownlinkDisplay, runningPeriodic, runningPeriodicDisplay, runningSleep} MOTE_RUNNING_T;
MOTE_RUNNING_T  runningState = runningEnter;

//static void MOTEapp_TimerBasedTask(appTimerHandlerBit_t);
static void EndLine(void);
static void sendCommand(const char *);
static void sendDataCommand(const char *string, uint8_t*, uint8_t);
static void sendSetCommand(uint8_t*);
static MOTE_T moteJoiningProcess(bool, bool);
static MOTE_RUNNING_T moteRunningProcess(bool, bool, bool, bool);
uint8_t MOTEapp_getByteCounte(void);
static uint8_t unhex(uint8_t);
static uint8_t hexToStr(uint8_t);
static uint8_t moteApp_GoToSleep(void);
static void moteApp_clearBuffers(void);
void moteApp_add8bToDataBuffer(uint8_t, uint8_t);
void moteApp_add16bToDataBuffer(uint16_t, uint8_t);
static void MOTEapp_SetADR(bool);

#define COM_RETRY_ATTEMPTS  10
static uint8_t appDevEui[16];
static uint8_t appKey[32];
static uint8_t retryCount = 0;
static uint8_t modemBufferIndex = 0;

static uint16_t light = 0;
static uint16_t temperature = 0;
static uint8_t randomPortNum = 0;
static bool    messageReceived = false;
static uint16_t pkts = 0;

static uint8_t dr = 0; // cycle through 0-3

//#define DEBUG
#if defined DEBUG
#define uncnfVscnfRatio     3
#define SleepTimeOut        30           // In Secs; This is for Menu Timout
#define LightCapTime        5           // * 10 mSec; Captures the light and mV of PIC
#define PeriodicInSleep     5           // How many times does device wake up before Periodic (Sleep) Transmission
                                       
#else
#define uncnfVscnfRatio     3
#define SleepTimeOut        10          // In Secs; This is for Menu Timout
#define LightCapTime        5          // * 10 mSec; Captures the light and mV of PIC
#define PeriodicInSleep     33          // How many times does device wake up before Periodic (Sleep) Transmission
                                      
#endif
static uint8_t secTicker = 0;
static uint8_t minTicker = 0;
static uint8_t m10SecTicker = 0;
static uint8_t sleepTicker = 0;

uint8_t nibble2int(char c)
{
    if (c>='0' && c<='9') {
        return c-'0';
    } else if (c >= 'A' && c <= 'F') {
        return 10+c-'A';
    } else if (c >= 'a' && c <= 'f') {
        return 10+c-'a';        
    } else {
        return 0; // error
    }
}

void modemResync(void)
{
    bool escapeEvent = false;
    uint16_t escapeCounter = 0;
          
    // In some cases the modem may be in sleep. This function makes sure the modem
    // is awake and synchronized at the right baudrate. This also works if the
    // modem was already awake. Currently there's a bug in the modem fw which
    // makes it misinterpret the firs command after resynchronizing.

    // First send wake up / restart autobaud
    TXSTAbits.TXEN = 0;
    EUSART_TX_LAT = 0;
    moteApp_delayms(10);
//    TXSTAbits.SENDB = 1;
//    TXREG = 0x00;
//    while (TXSTAbits.SENDB == 1)
//    {};
    TXSTAbits.TXEN = 1;
    moteApp_delayms(5);
    // Now send autobaud detect character
    TXREG = 0x55;
    while (TXSTAbits.TRMT == 0)
    {};
    // Now send a command to be ignored
    sendCommand("z");
    
    moteApp_delayms(40);
    while (escapeEvent)
    {
        if(MOTEapp_getModemResponse())
           escapeEvent = true; 
        else if (++escapeCounter > 5000)
            escapeEvent = true; 
    }
}

void modemSleep()
{
    sendCommand("sys sleep 999999999");
    while(1 == PIE1bits.TXIE)
        ;
    moteApp_delayms(20);
}

void MOTEapp_handlerState(MOTE_MODE_T state)
{
    moteState = state;
}

MOTE_MODE_T moteHandler(void)
{
    bool gotoSleep = false;
    bool gotoNextState = false;
    bool s2Button = false;
    bool readMessage = false;
    buttonStates_t buttonPress = Released;


    
    
    // Check USB connection state
    if (USB_DET_PORT)
        moteState = MOTE_SWAP;

    // Get Timer Ticks
    if ( appTimer.mSecTick )
    {
        // Clear Flags for next task
        TMRapp_ClearBitFlags();
        // Execute Task
        TMRapp_Handler();
        MOTEapp_TimerBasedTask(appTimer);
    }

    // Handle button press events for menu navigation
    buttonPress = service_pushbutton();
    if (buttonPress == ShortPress1)
    {
        secTicker = 0;
        gotoNextState = true;   // Button Event has occured
    }
    else if (buttonPress == ShortPress2)
    {
        secTicker = 0;
        s2Button = true;   // Button Event has occured
    }

    // Gather response from LoRa Module if available
    if ( MOTEapp_getModemResponse() )
        readMessage = true;
    else
        readMessage = false;

    // Handle Timed Task
    if ( (runningState == runningUplink) || (runningState == runningSensorMenu) )
    {
        // Get Sensor Data every 10mS
        if (m10SecTicker >= LightCapTime)
        {
            uint16_t Traw = 0;
            moteApp_delayms(10);
            for(uint8_t i = 32; i>0; i--)
            {
                light = (light + ADC_GetConversion(Light_channel)) / 2;
            }
            moteApp_delayms(10);
            for(uint8_t i = 32; i>0; i--)
            {
                Traw = (Traw + ADC_GetConversion(Temp_channel)) / 2;
            }
            temperature = ADC_TempConversion(Traw);
            m10SecTicker = 0;
        }
    }
    // Sleep Event if idle
    if(secTicker)
    {
        if (secTicker >= SleepTimeOut)
        {
            gotoSleep = true;
            secTicker = 0;
        }
    }

    switch(moteState)
    {
        default:        // Invalid
            while(1);   // Hold For Error;
        break;
        case MOTE_STARTUP:
            oled_clear();
            oled_putString("MOTE Mode",1,0);
            oled_putString("Starting",2,1);
            oled_putString("ReRun PwrTst<S2>",0,3);
            activeState = moteStartup;
            moteState = MOTE_JOINING;
        break;
        case MOTE_JOINING:
            if ( moteJoiningProcess(readMessage, s2Button) )
            {
                oled_clear();
                oled_putString("Mobile Mote -App",0,0);
                oled_putString("Navigate  Select", 0,2);
                oled_putString(" (S1)      (S2) ", 0,3);
                runningState = runningEnter;
                moteState = MOTE_RUNNING;
            }
        break;
        case MOTE_RUNNING:
            moteRunningProcess(gotoNextState, s2Button, readMessage, gotoSleep);
        break;
        case MOTE_SWAP:
            // Do Nothing
        break;
    }
    return moteState;
}

#define MODE_TX 1
#define MODE_RX 2



static MOTE_T moteJoiningProcess(bool messageReady, bool switchEvent)
{
    static int ch = 0;
    bool joiningComplete = false;
    static int mode = 0;
    static unsigned char pkt_cnt = 0;
    static unsigned char float_val = 255;
    static bool pending_confirm = false;
    char buf[50];
    static int skip_count = 0;
    

    /*if (switchEvent)
    {
        activeState = moteComFailure;
        while(!SWITCH_2_PORT);
    }*/

    switch (activeState)
    {
        default:        // Invalid
            while(1);   // Hold for Error
        break;
        case moteStartup:
            
            // HACK to block for Espruino
            //EUSART_TX_TRIS = INPUT;
            //while (1) {};
            
            //sendCommand("sys reset");
            
            LED_RED_LAT = 0;
            LED_GREEN_LAT = 0;
            
            
            // flush buffer
            while (MOTEapp_getModemResponse());
            
                    
            sendCommand("sys get ver");
            oled_clear();
            oled_putString("get ver",0,0);
            moteApp_delayms(100);
            activeState = moteReset;
            //activeState = moteStartup;
        break;
        case moteReset:
            if ( messageReady )
            {
                if(strncmp(&modemResonseBuffer[2], "2903", 4) == 0)   // Check 1st [2] Bytes for RN
                {
                    sendCommand("mac set devaddr DEAD00A1");
                    oled_clear();
                    oled_putString("set devaddr",0,0);
                    moteApp_delayms(80);
                    activeState = moteSetDevaddr;
                }
                else
                {
                    oled_clear();
                    oled_putString(modemResonseBuffer,0,3);
                    moteApp_delayms(1000);
                    activeState = moteStartup;
                }
            }
        break;
        case moteSetDevaddr:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    sendCommand("mac set appskey 2B7E151628AED2A6ABF7158809CF4F3C");
                    oled_clear();
                    oled_putString("set appskey",0,0);
                    moteApp_delayms(80);
                    activeState = moteSetAppskey;
                }
                else // Retry Command
                {
                    activeState = moteStartup;
                }
            }
        break;
        case moteSetAppskey:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    
                    sendCommand("mac set nwkskey 2B7E151628AED2A6ABF7158809CF4F3C");
                    oled_clear();
                    oled_putString("set nwkskey",0,0);
                    moteApp_delayms(80);
                    activeState = moteSetNwkskey;
                }
                else // Retry Command
                {
                    activeState = moteStartup;
                }
            }
        break;
        case moteSetNwkskey:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                   
                    sendCommand("mac set adr off");
                    oled_clear();
                    oled_putString("set adr",0,0);
                    moteApp_delayms(80);
                    activeState = moteSetAdr;
                }
                else // Retry Command
                {
                    activeState = moteStartup;
                }
            }
        break;
        case moteSetAdr:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    
                    sendCommand("mac set sync 34");
                    oled_clear();
                    oled_putString("set sync",0,0);
                    moteApp_delayms(80);
                    activeState = moteSetSync;
                }
                else // Retry Command
                {
                    activeState = moteStartup;
                }
            }
        break;
        case moteSetSync:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    
                    sendCommand("mac set rx2 8 923300000");
                    oled_clear();
                    oled_putString("set rx2",0,0);
                    moteApp_delayms(80);
                    ch = 0; // start with channel 0
                    activeState = moteSetRx2;
                    //activeState = moteSetChStatus;
                }
                else // Retry Command
                {
                    activeState = moteStartup;
                }
            }
        break;
        case moteSetRx2:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    // only enable subband #2
                    sprintf(cmd,"mac set ch status %d %s",
                            ch, 
                            (ch>=8 && ch<=15)?"on":"off");
                    sendCommand(cmd);
                    oled_clear();
                    oled_putString("set ch",0,0);
                    oled_putString(&cmd[18],0,1);                    
                    moteApp_delayms(80);
                    ch++;
                    if (ch<72) {
                        activeState = moteSetRx2;
                    } else {
                        activeState = moteSetChStatus;
                    }
                }
                else // Retry Command
                {
                    activeState = moteStartup;
                }
            }
        break;
        case moteSetChStatus:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    moteApp_delayms(200);
                    sendCommand("mac join abp");
                    oled_clear();
                    oled_putString("join abp",0,0);
                    moteApp_delayms(80);
                    activeState = moteJoinAbp;
                }
                else // Retry Command
                {
                    activeState = moteStartup;
                }
            }
        break;
        case moteJoinAbp:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    // waiting now for "accepted"                    
                    oled_clear();
                    oled_putString("Wait accepted...",0,0);
                    moteApp_delayms(80);
                    activeState = moteWaitAccepted;
                }
                else // Retry Command
                {
                    activeState = moteStartup;
                }
            }
        break;
        case moteWaitAccepted:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "accepted") == 0)
                {     
                    //while (MOTEapp_getModemResponse());
                    
                    // send initial packet
                    
                    sendCommand("mac tx uncnf 1 00000000");
                    oled_clear();
                    oled_putString("ini tx",0,0);
                    moteApp_delayms(150);
                    activeState = moteIniTx;
                    LED_GREEN_LAT = 1;
                }
                else // Retry Command
                {
                    oled_clear();
                    oled_putString("moteWaitAccepted",0,0);
                    activeState = moteStartup;
                    moteApp_delayms(500);
                    LED_RED_LAT = 1;
                }
            }
        break;
        case moteIniTx:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    oled_clear();
                    oled_putString("wait mac_tx_ok",0,0);
                    moteApp_delayms(3000);
                    activeState = moteWaitIniTxOk;
                    LED_RED_LAT = 1;
                }

                else // Retry Command
                {
                    oled_clear();
                    oled_putString("moteIniTx",0,0);
                    activeState = moteStartup;
                    moteApp_delayms(500);
                    LED_GREEN_LAT = 0;
                }
            }
        break;
        case moteWaitIniTxOk:
            //if ( messageReady )
            {
                //if(strcmp(modemResonseBuffer, "mac_tx_ok") == 0)
                {
                    LED_RED_LAT = 1;
                    LED_GREEN_LAT = 0;
                    // next we rest for a while                                        
                    oled_clear();                 
                    oled_putString("Mode: ",0,0);                
                    oled_putString(autoMode?"AUTO  ":"MANUAL",6,0);
                    oled_putString("S1 = auto on/off",0,1);
                    oled_putString("S2 = manual send",0,2);
                    
                    moteApp_delayms(80);
                    ch = 0;
                    activeState = moteRest;
                }
                /*else if (strcmp(modemResonseBuffer, "") == 0) {
                    // glitch where we get an extra return
                    moteApp_delayms(1);
                    LED_GREEN_LAT = 0;
                    moteApp_delayms(1);
                    LED_GREEN_LAT = 1;
                    moteApp_delayms(1);                    
                    activeState = moteWaitIniTxOk;
                    
                }
                else // Retry Command
                {
                    oled_clear();
                    oled_putString("WaitIniTxOk",0,0);
                    oled_putString(modemResonseBuffer,0,2);
                    sprintf(cmd,"buflen=%d",strlen(modemResonseBuffer));
                    oled_putString(cmd,0,3);
                    activeState = moteStartup;
                    moteApp_delayms(1000);
                }*/
            }
        break;
        case moteRest:
            if (autoMode || manualTrigger) {
                ch++;
            }
            moteApp_delayms(100);
            /* check buttons */
            S1 = SWITCH_1_PORT; // 1 == unpressed, 0 = pressed
            S2 = SWITCH_2_PORT;
            if (S1 == 0 && prevS1 == 1) {
                /* rising edge of switch 1 */
                /* alternate mode */
                autoMode = !autoMode;
                oled_putString("Mode: ",0,0);                
                oled_putString(autoMode?"AUTO  ":"MANUAL",6,0);
            }
            if (S2 == 0 && prevS2 == 1) {
                /* rising edge of switch 2 */
                /* schedule a measurement */
                manualTrigger = true;
            }
            prevS1 = S1;
            prevS2 = S2;
            if (ch>20) {
                manualTrigger=false;
                ch = 0;
                // make a measurement
                uint16_t Traw = 0;
                moteApp_delayms(10);
                for(uint8_t i = 32; i>0; i--)
                {
                    light = (light + ADC_GetConversion(Light_channel)) / 2;
                }
                moteApp_delayms(10);
                for(uint8_t i = 32; i>0; i--)
                {
                    Traw = (Traw + ADC_GetConversion(Temp_channel)) / 2;
                }
                temperature = ADC_TempConversion(Traw);
                m10SecTicker = 0;
                
                pkts++;
                
                // display measurement
                oled_clear();
                oled_putString("Mode: ",0,0);                
                oled_putString(autoMode?"AUTO  ":"MANUAL",6,0);
                oled_putString("Light:",0,1);
                oled_putString("Temp :",0,2);
                oled_putString("C",12,2);
                oled_putUint16(light,6,1);
                oled_putUint16(temperature,6,2);                
                oled_putString("Pkts|:",0,3);
                oled_putUint16(pkts,6,3);
                
                
                sprintf(cmd,"mac tx uncnf 1 %04X%04X",
                        pkts,dr);
                sendCommand(cmd);
                
                activeState = moteTxUncnf;
            }
        break;
        case moteTxUncnf:
            //if ( messageReady )
            {
                //if(strcmp(modemResonseBuffer, "ok") == 0)
                {                    
                    oled_putString("Pkts/:",0,3);
                    oled_putUint16(pkts,6,3);
                    moteApp_delayms(80);
                    activeState = moteTxWait;
                }
                /*else 
                {
                    
                    oled_clear();
                    oled_putString("TxUncnf",0,0);
                    activeState = moteStartup;
                    moteApp_delayms(500);
                }*/                
            }
        break;
        case moteTxWait:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "mac_tx_ok") == 0)
                {
                    dr = (dr + 1) % 4; // 0-3
                    sprintf(cmd,"mac set dr %d",dr);                                        
                    sendCommand(cmd);
                    oled_putString("Pkts-:",0,3);
                    oled_putUint16(pkts,6,3);
                    activeState = moteNextDr;
                }
            }
        break;
        
        case moteNextDr:
            if ( messageReady )
            {
                if(strcmp(modemResonseBuffer, "ok") == 0)
                {
                    //oled_putString("Pkts-:",0,3);
                    //oled_putUint16(pkts,6,3);
                    activeState = moteRest;
                }
            }
        break;
        
        case moteDone:
            joiningComplete = true;
            moteApp_delayms(80);
        break;
        case moteComFailure:
            DATAEE_WriteByte(0x00, 0x00);
            while((!SWITCH_1_PORT) && (!SWITCH_2_PORT));
            moteApp_delayms(40);
            RESET();
            // Do Nothing; Wait for USB; Hold for Display
        break;
    }
    return joiningComplete;
}

static uint8_t dotCount = 0;
static bool periodicState = true;
static uint8_t sleepTxOccurances;
static uint8_t dataBuffer[12];
static uint8_t dataRatePosition = 0;
static uint8_t dataRateSelection = 2;
static bool adrStatus = false;

static MOTE_RUNNING_T moteRunningProcess(bool changeStates, bool selectButton, bool messageReady, bool sleepEvent)
{
    uint8_t readByte = 0;

    if ((sleepEvent) && (periodicState))
    {
        if (sleepTicker >= PeriodicInSleep) // No Manual Transmission for a time; Do one Auto
        {   
            // Measure Sensors
            uint16_t Traw = 0;
            moteApp_delayms(10);
            for(uint8_t i = 32; i>0; i--)
            {
                light = (light + ADC_GetConversion(Light_channel)) / 2;
            }
            moteApp_delayms(10);
            for(uint8_t i = 32; i>0; i--)
            {
                Traw = (Traw + ADC_GetConversion(Temp_channel)) / 2;
            }
            temperature = ADC_TempConversion(Traw);
            // Do Normal Operation
            sleepTicker = 0;
            oled_clear();
            oled_putString("Issue Perodic TX",0,0);
            oled_putString("On Port #: ",0,1);
            oled_putUint8(randomPortNum,10,1);
            oled_putString("L:       T: ",0,2);
            oled_putUint16(light,3,2);
            oled_putUint8(temperature,12,2);
            oled_putString("(NAV)    (SEL) ",0,3);          
            // Prepare Buffers and data
            moteApp_clearBuffers();
            randomPortNum = TMR2_ReadTimer();
            // Make Sure Port is in allowed Range
            if (!randomPortNum)
                randomPortNum = 5;
            else if (randomPortNum > 223)
                randomPortNum = randomPortNum - 50;
            // Prepare DataBuffer for Tx
            moteApp_add8bToDataBuffer(randomPortNum, 0);
            dataBuffer[3] = 0x20;
            moteApp_add16bToDataBuffer(light, 4);
            moteApp_add8bToDataBuffer(temperature, 9);
            NOP();
            sleepTxOccurances = 0;
            oled_putString("C",15,1);
            sendDataCommand("radio tx ", dataBuffer, 12);
            secTicker = SleepTimeOut - (SleepTimeOut / 3);
            runningState = runningUplinkTransmit;
        }
        else
        {
            sleepTicker++;
            runningState = runningSleep;
            selectButton = true;
        }
    }
    
    moteApp_GoToSleep();

    // Menu Operation
    switch(runningState)
    {
        default:        // Invalid
            while(1);   // Hold for Error
        break;
    }
    return runningState; // no return value needed but we declare one for this function.
}

uint8_t MOTEapp_GetDataRateSetting(void)
{
    uint8_t readData;
    readData = DATAEE_ReadByte(0x10);
    moteApp_delayms(20);
        
    return readData;
}

static void MOTEapp_SetADR(bool state)
{
    adrStatus = state;
}

void MOTEapp_SetDataRate(uint8_t dataRate)
{
    dataRateSelection = dataRate;
}

void MOTEapp_WriteDataRate(void)
{    
//    DATAEE_WriteByte(0x10, dataRate);
    moteApp_delayms(100);
        
    if (adrStatus)
    {
        sendCommand("mac set adr on");
        moteApp_delayms(100);
    }
    else
    {
        sendCommand("mac set adr off");
        moteApp_delayms(100);
    }

    switch (dataRateSelection)
    {
        case 0:
            sendCommand("mac set dr 0");
        break;    
        case 1:
            sendCommand("mac set dr 1");
        break;
        case 2:
            sendCommand("mac set dr 2");
        break;
        case 3:
            sendCommand("mac set dr 3");
        break;
        case 4:
            sendCommand("mac set dr 4");
        break;
        case 5:
            sendCommand("mac set dr 5");
        break;
        case 6:
            sendCommand("mac set dr 6");
        break;
        case 7:
            sendCommand("mac set dr 7");
        break;
//        case 8:
//            sendCommand("mac set adr on");
//        break;
    }
    moteApp_delayms(100);
    sendCommand("mac save");
    moteApp_delayms(300);
}

void MOTEapp_TimerBasedTask(appTimerHandlerBit_t timerTask)
{
    // Excute 1mS task Regardless
    // --> 1mSec Task
    // Variable TimeLength Task
    if (timerTask.mSecTenTick)
    {
        // --> 10mSec Task
        m10SecTicker++;
    }
    if (timerTask.mSecHundredTick)
    {
        // --> 100mSec Task
        
    }
    if (timerTask.SecTick)
    {
        // --> 1Sec Task
        secTicker++;
    }
    if (timerTask.MinTick)
    {
        // --> 1Min Task
        minTicker++;
    }
    if (timerTask.HourTick)
    {
        // --> 1Hour Task
    }
    if (timerTask.HalfDayTick)
    {
        // --> 12Hour Task
    }
    if (timerTask.DayTick)
    {
        // --> 1Day Task
    }
}

bool MOTEapp_getModemResponse(void)
{
    bool terminatorReceived = false;    // Return false; Unless Terminator character (0x0D) is captured
    if (EUSART_DataReady)       // See if Bytes are available
    {
        uint8_t bufferByte = 0; // Restore for read
        bufferByte = EUSART_Read();
        // See what we got
        if (bufferByte == 0x0D)
        {
            modemResonseBuffer[modemBufferIndex] = 0x00; // Add Null terminator
            terminatorReceived = true;
            modemBufferIndex = 0; // Prepare index for next message
        }
        else if ( (bufferByte == 0x00) || (bufferByte == 0x0A) )
        {
            // Do nothing
        }
        else
        {
            modemResonseBuffer[modemBufferIndex] = bufferByte; // Add Byte to Buffer
            // Increment Buffer for next byte
            modemBufferIndex++;
        }
    }
    return terminatorReceived;
}

void MOTEapp_setResponseReady(void)
{
    responseReady = true;   // Set Local Variable
}

static void EndLine(void)
{
    EUSART_Write(0x0D); // CR; Carriage Return
    EUSART_Write(0x0A); // NF; Line Feed
}

static void sendCommand(const char *string)
{
    // Prepare Buffer for Response
    while (*string)
    {
        EUSART_Write(*string++);
    }
    EndLine();   
}

static void sendDataCommand(const char *string, uint8_t* appData, uint8_t dataLength)
{
    // String
    while (*string)
    {
        EUSART_Write(*string++);
    }
    // Port No & space
    for (uint8_t byteCount = 0; byteCount < 3; byteCount++)
    {
        EUSART_Write(appData[byteCount]);
    }
    //Space
    EUSART_Write(appData[3]);
    //Data
    for (uint8_t byteCount = 4; byteCount < dataLength; byteCount++)
    {
        EUSART_Write(hexToStr(appData[byteCount] >> 4));
        EUSART_Write(hexToStr(appData[byteCount] & 0x0F));
    }
    EndLine();
}

static uint8_t unhex(uint8_t c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return '-';
}

static uint8_t hexToStr(uint8_t character)
{
    // Mask out Top Nibble
    character = character & 0x0F;
    // Check if: 0-9; Else if: A-F
    if (character < 0x0A)
    {
        return character + '0';
    }
    else if ( (character >= 0x0A) && (character <= 0x0F) )
    {
        return character + '0' + 7;
    }
    else
    {
        return 0;
    }
}

static uint8_t moteApp_GoToSleep(void)
{
    uint8_t status = 0;
    while (SWITCH_2_PORT == 0);     // Hold for Button Release
    modemSleep();
    oled_displayOff(true); 
    ADCON0bits.ADON = 0;       // ADC Off
    INTCONbits.GIE = 0;
    GIE = 0;
    PEIE = 0;
    // Prepare USB Detect
    USB_DET_ANSEL = 1;
    // Prepare IOC to wake us up
    USB_DET_EN = 1;      // Enables CMP1 interrupt.
    SWITCH_1_IOC = 1;
    SWITCH_2_IOC = 1;
    MODEM_WAKE_IOC = 1;
    IOC_ENABLE = 1;
    // Dummy Reads to Establish IOC level
    status = SWITCH_1_PORT;
    status = SWITCH_2_PORT;
    status = MODEM_WAKE_PORT;
    if (periodicState)
    {
        //WDTCONbits.SWDTEN = 1;
    }
    IOC_FLAG = 0;
    USB_DET_FLAG = 0;
    NOP();
    SLEEP();
    NOP();
    // Reconfigure Critical peripherals
    //WDTCONbits.SWDTEN = 0;
    EUSART_FlushBuffer();
    // Disable IOC
    IOC_ENABLE = 0;
    SWITCH_1_IOC = 0;
    SWITCH_2_IOC = 0;
    MODEM_WAKE_IOC = 0;
    PIE2bits.C1IE = 0;
    GIE = 1;

    PIN_MANAGER_Initialize();
    EUSART_Initialize();
    // Process Wake Up Event
    if (IOC_FLAG)
    {   // PushButton or Module IOC woke us
        IOC_FLAG = 0;
        status = 1;
    }
// TODO:
//    else if (Comparator)    // USB EVENT
//    {
//        status = 2;
//    }
    else
    {   // Time woke us. Reload; so wake from sleep is short
        secTicker = 10;
        status = 0;
    }
    USB_DET_ANSEL = 0;
    oled_displayOff(false);
    oled_init();         // Dispaly

    modemResync();
    return status;
}

void moteApp_clearCommand(void)
{
    sendCommand("");
}

void moteApp_toggleLeds(bool state)
{
    moteApp_delayms(80);
    if (state)
    {
        sendCommand("sys set pindig GPIO10 1\r\n");
        moteApp_delayms(80);
        sendCommand("sys set pindig GPIO11 1\r\n");
    }
    else
    {
        sendCommand("sys set pindig GPIO10 0\r\n");
        moteApp_delayms(80);
        sendCommand("sys set pindig GPIO11 0\r\n");
    }
    moteApp_delayms(80);
}

static void moteApp_clearBuffers(void)
{
    for (uint8_t counter = 0; counter < 3; counter++)
    {
        responsePortNum[counter] = 0;
    }
    for (uint8_t counter = 0; counter < 34; counter++)
    {
        responseId[counter] = 0;
    }
    for (uint8_t counter = 0; counter < 10; counter++)
    {
        responseData[counter] = 0;
    }
}

void moteApp_add8bToDataBuffer(uint8_t number, uint8_t bufferLocation)
{
    dataBuffer[bufferLocation + 2] = '0' + (number % 10);
    number /= 10;
    dataBuffer[bufferLocation + 1] = '0' + (number % 10);
    number /= 10;
    dataBuffer[bufferLocation] = '0' + (number % 10);
}

void moteApp_add16bToDataBuffer(uint16_t number, uint8_t bufferLocation)
{
    dataBuffer[bufferLocation + 4] = '0' + (number % 10);
    number /= 10;
    dataBuffer[bufferLocation + 3] = '0' + (number % 10);
    number /= 10;
    dataBuffer[bufferLocation + 2] = '0' + (number % 10);
    number /= 10;
    dataBuffer[bufferLocation + 1] = '0' + (number % 10);
    number /= 10;
    dataBuffer[bufferLocation] = '0' + (number % 10);
}

void moteApp_delayms(uint16_t length)
{
    uint16_t counter;
    for(counter = 0; counter < length; counter++)
    {
        __delay_ms(1);
    }
}
