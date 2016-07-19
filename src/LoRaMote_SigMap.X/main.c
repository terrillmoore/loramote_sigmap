/*
 * File:   main.c
 * Original Author: C14312
 * Author: Chris Merck
 *
 * Created on February 16, 2015, 1:45 PM
 * Build 012: Cycle through DRs
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "mcc.h"
#include "memory.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_cdc.h"
#include "SSD1306oLED.h"
#include "MOTEapp.h"
#include "USBapp.h"
#include "HardwareProfile.h"

// Application Types
typedef enum {USB, MOTE} OPERATION_T;
OPERATION_T operationType;
typedef enum {MAIN_STARTUP, MAIN_CDC_MODE, MAIN_HOST_MODE} MAIN_STATE_T;
MAIN_STATE_T mainState;
typedef enum {APP_LORA_STARTUP, APP_HOST_RUNNING} APP_STATE_T;
APP_STATE_T appState;
// Application Variables
MOTE_MODE_T moteModeState;
// Local Functions PROTOTYPES

// Local Functions PROTOTYPES
static uint8_t powerOnStatus = 0x00;
static uint8_t dataRateRead = 0x00;

// TODO: poweruptest.c needs a header for this prototype
extern void test_powerup(void);

// Application Start
void main(void)
{
    // Prepare Application; MCC generated code
    SYSTEM_Initialize();
    // Custom Initializations
    
    
    
    /*   
     *   watchdog timer 
     *   configured for 131s timeout
     * 
     */
    //WDTCONbits.SWDTEN = 1; // enable watchdog
    //ClrWdt();
    
    
    
    oled_init();         // Dispaly
    USBDeviceInit();	// usb_device.c.  Initializes USB module SFRs and firmware
                        // variables to known states.
    
    
    
    
    USBDeviceAttach();
    // Interrupts Enabled
    PEIE = 1;
    GIE = 1;
    
    // Test Hardware
    moteApp_delayms(300);    
    modemResync();
    moteApp_delayms(300);
//    moteApp_clearCommand();
//    moteApp_delayms(100);   
    
    powerOnStatus = DATAEE_ReadByte(0x00);
#if 0
    if ( powerOnStatus != 0x55)
    {
        test_powerup();
        DATAEE_WriteByte(0x00, 0x55);
    }
#endif

    // Default to USB
    operationType = USB;   // Mode State
    USBapp_handlerState(USB_STARTUP);
    
    
    
    // Initial Display Print
    oled_clear();
    oled_putString("LoRa SigMap",0,0);
    oled_putString(" Dev: A1 ",0,1);
    oled_putString("   build 012   ",0,3);
    moteApp_delayms(700);
   
    // Application Loop
    while(1)
    {
        // Handle USB or Solo Mode
        switch (operationType)
        {
            default:        // Invalid
                while(1);   // Hold For Error;
            break;
            case USB:
                USBapp_Handler();   // It is held in here.
                // USB escaped; Cable is unplugged; Change OPERATION mode.
                operationType = MOTE;
                MOTEapp_handlerState(MOTE_STARTUP);   // Initial state for startup
            break;
            case MOTE:
                if (moteHandler() == MOTE_SWAP)
                {
                    // Return to USB Operation
                    operationType = USB;   // Mode State
                    USBapp_handlerState(USB_STARTUP);
                }
            break;
        }
    }
}