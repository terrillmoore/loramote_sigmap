/**
  Generated Pin Manager File

  Company:
    Microchip Technology Inc.

  File Name:
    pin_manager.c

  Summary:
    This is the Pin Manager file generated using MPLAB® Code Configurator

  Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB® Code Configurator - v2.10
        Device            :  PIC16F1509
        Driver Version    :  1.02
    The generated drivers are tested against the following:
        Compiler          :  XC8 v1.33
        MPLAB             :  MPLAB X 2.26
*/

/*
Copyright (c) 2013 - 2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*/

#include <xc.h>
#include "pin_manager.h"
#include "HardwareProfile.h"

void PIN_MANAGER_Initialize(void)
{
    //*** TRIS CONFIG *** //
    // A
    USB_DET_TRIS = INPUT;    
    VBAT_TRIS = INPUT;    
    TEMP_SENSOR_TRIS = INPUT;   
    LIGHT_SENSOR_TRIS = INPUT;
    VOUT_EN_TRIS = OUTPUT;
    SENSE_PWR_TRIS = OUTPUT;    
    LED_GREEN_TRIS = OUTPUT;   
    LED_RED_TRIS = OUTPUT; 
    // B
    LCD_SDI_TRIS = OUTPUT;
    LCD_SCK_TRIS = OUTPUT;
    LCD_RESET_TRIS = OUTPUT;
    LCD_SDO_TRIS = OUTPUT;
    LCD_DC_TRIS = OUTPUT;
    // C
    SWITCH_1_TRIS = INPUT;
    SWITCH_2_TRIS = INPUT;
    MODEM_WAKE_TRIS = INPUT;
    EUSART_TX_TRIS = OUTPUT;
    EUSART_RX_TRIS = INPUT;
    FLOAT_POS_TRIS = OUTPUT;
    FLOAT_NEG_TRIS = INPUT;
    //*** LAT CONFIG *** //
    // A
    VOUT_EN_LAT = ENABLE;
    SENSE_PWR_LAT = PWR_ENABLE;
    LED_GREEN_LAT = LED_OFF;
    LED_RED_LAT = LED_OFF;
    // B
    LCD_RESET_LAT = 0;
    LCD_SDI_LAT = 0;
    LCD_DC_LAT = 0;
    FLOAT_POS_LAT = 0;
    FLOAT_NEG_LAT = 0;
    // C
    MODEM_WAKE_LAT = 0;
    EUSART_TX_LAT = 0;
    //*** ANSEL CONFIG *** //
    USB_DET_ANSEL = 0;
    VBAT_ANSEL = 1;
    TEMP_SENSE_ANSEL = 1;
    LIGHT_SENSE_ANSEL = 1;
    EUSART_RX_ANSEL = 0;
    EUSART_TX_ANSEL = 0;
}
/**
 End of File
*/