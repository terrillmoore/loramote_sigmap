#include <xc.h>
#include "mcc.h"
#include "SSD1306oLED.h"
#include "MOTEapp.h"
#include "adc.h"

void test_oled(void);
void test_switch(void);
void test_comms(void);
void test_leds(void);
void test_rf(void);
void test_adc(void);
void delayms(uint16_t t);

#define VERSION_MAJOR "0"
#define VERSION_MINOR "1"

void delayms(uint16_t t)
{
    uint16_t i;
    for(i = 0; i < t; i++)
    {
        __delay_ms(1);
    }
}

void test_powerup(void)
{
    // OLED TEST
    test_oled();
    moteApp_delayms(20);
    // SWITCH TEST
    test_switch();
    // COMMS TEST
    oled_clear();
    moteApp_delayms(20);
    test_comms();
    // LED BLINK TEST
    test_leds();
    // TEST RF COMMS
    test_adc();

 //   test_rf();
    moteApp_delayms(20);
}

void test_oled(void)
{
    oled_clear();
    // Power Up Display
    oled_putString("MoTe v .",0,0);
    oled_putString(VERSION_MAJOR,0,6);
    oled_putString(VERSION_MINOR,0,8);
    oled_putString("Build Date/Time",0,1);
    oled_putString(__DATE__,0,2);
    oled_putString(__TIME__,0,3);

}

void test_switch(void)
{
    uint16_t timeout = 500;
    uint8_t  buttonCheck = 0x00;
    oled_clear();
    oled_putString("Button Test:",0,0);

    if( (SWITCH_1_PORT == 0) || (SWITCH_2_PORT == 0) )
    {
        oled_putString("Button FAIL",0,3);
    }
    else
    {
        oled_putString("Press Button",0,1);
    }
    while(timeout--)
    {
        if(SWITCH_1_PORT == 0)
        {
            oled_putString("Button1 PASS",0,2);
            buttonCheck = buttonCheck + 0x08;
        }
        if(SWITCH_2_PORT == 0)
        {
            oled_putString("Button2 PASS",0,3);
            buttonCheck = buttonCheck + 0x80;
        }
        if (buttonCheck == 0x88)
        {   // Both Button Passed; Move on
            break;
        }
        moteApp_delayms(10);
    }
}

void test_comms(void)
{
    oled_putString("Comm Test:",0,0);
    moteApp_toggleLeds(true);
    moteApp_delayms(500);
    moteApp_toggleLeds(false);
    moteApp_delayms(500);
    moteApp_toggleLeds(true);
    moteApp_delayms(500);
    moteApp_toggleLeds(false);
    moteApp_delayms(500);
}

void test_leds(void)
{    
    oled_putString("LED Test:",0,1);
//    moteApp_toggleLeds(true);
    LED_GREEN_LAT = 1;
    LED_RED_LAT = 1;
    moteApp_delayms(500);
    LED_GREEN_LAT = 0;
    LED_RED_LAT = 0;
    moteApp_delayms(500);
    LED_GREEN_LAT = 1;
    LED_RED_LAT = 1;
    moteApp_delayms(500);
    LED_GREEN_LAT = 0;
    LED_RED_LAT = 0;
    moteApp_delayms(500);
}

void test_adc(void)
{
    uint16_t L,Traw,F;
    uint8_t count, T;

    oled_clear();
    oled_putString("ADC Test",0,0);
    oled_putString("Temp  =",0,1);
    oled_putChar('C',14,1);
    oled_putString("Light =",0,2);

    for(count=0;count< 50;count++)
    {
//        ADC_StartConversion(FVR_channel);
        moteApp_delayms(50);
        Traw = ADC_GetConversion(Temp_channel);
        T = ADC_TempConversion(Traw);

        moteApp_delayms(50);
        L = ADC_GetConversion(Light_channel);
      
        oled_putUint8(T,10,1);
        oled_putUint16(L,10,2);
    }
}

void test_rf(void)
{
    oled_clear();
    oled_putString("RF Test:",0,0);
    moteApp_delayms(250);
}



