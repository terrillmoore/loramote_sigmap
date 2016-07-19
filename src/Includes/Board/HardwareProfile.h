/********************************************************************
 FileName:      HardwareProfile - 16F1 LPC USB Development Kit.h
 Dependencies:  See INCLUDES section
 Processor:     PIC18 or PIC24 USB Microcontrollers
 Hardware:      Low Pin Count USB Development Kit
 Compiler:      Microchip C18
 Company:       Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the �Company�) for its PIC� Microcontroller is intended and
 supplied to you, the Company�s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

********************************************************************
 File Description:

 Change History:
  Rev   Date         Description
  1.0   11/19/2004   Initial release
  2.1   02/26/2007   Updated for simplicity and to use common
                     coding style
  2.3   09/15/2008   Broke out each hardware platform into its own
                     "HardwareProfile - xxx.h" file
********************************************************************/
#include <xc.h>
#ifndef HARDWARE_PROFILE__H
#define HARDWARE_PROFILE__H
/*******************************************************************/
/******** Application specific definitions *************************/
/*******************************************************************/

/*******************************************************************/
/******** USB stack hardware selection options *********************/
/*******************************************************************/

/*******************************************************************/
/******** System  *************************/
/*******************************************************************/
#define _XTAL_FREQ   48000000

/*******************************************************************/
/******** PORT/LAT/TRIS *************************/
/*******************************************************************/
/******** -A- *************************/
/******** -0- *************************/
#define USB_DET_PORT    PORTAbits.RA0
#define USB_DET_LAT     LATAbits.LATA0
#define USB_DET_TRIS    TRISAbits.TRISA0
#define USB_DET_ANSEL   ANSELAbits.ANSA0
#define USB_DET_EN      PIE2bits.C1IE
#define USB_DET_FLAG    PIR2bits.C1IF
/******** -1- *************************/
#define VBAT_PORT   PORTAbits.RA1
#define VBAT_LAT    LATAbits.LATA1
#define VBAT_TRIS   TRISAbits.TRISA1
#define VBAT_ANSEL   ANSELAbits.ANSA1
/******** -2- *************************/
#define TEMP_SENSOR_PORT    PORTAbits.RA2
#define TEMP_SENSOR_LAT     LATAbits.LATA2
#define TEMP_SENSOR_TRIS    TRISAbits.TRISA2
#define TEMP_SENSE_ANSEL    ANSELAbits.ANSA2
/******** -3- *************************/
#define LIGHT_SENSOR_PORT   PORTAbits.RA3
#define LIGHT_SENSOR_LAT    LATAbits.LATA3
#define LIGHT_SENSOR_TRIS   TRISAbits.TRISA3
#define LIGHT_SENSE_ANSEL   ANSELAbits.ANSA3
/******** -4- *************************/
#define VOUT_EN_PORT    PORTAbits.RA4
#define VOUT_EN_LAT     LATAbits.LATA4
#define VOUT_EN_TRIS    TRISAbits.TRISA4
/******** -5- *************************/
#define SENSE_PWR_PORT    PORTAbits.RA5
#define SENSE_PWR_LAT     LATAbits.LATA5
#define SENSE_PWR_TRIS    TRISAbits.TRISA5
/******** -6- *************************/
#define LED_GREEN_PORT    PORTAbits.RA6
#define LED_GREEN_LAT     LATAbits.LATA6
#define LED_GREEN_TRIS    TRISAbits.TRISA6
/******** -7- *************************/
#define LED_RED_PORT    PORTAbits.RA7
#define LED_RED_LAT     LATAbits.LATA7
#define LED_RED_TRIS    TRISAbits.TRISA7
/******** -B- *************************/
/******** -0- *************************/
#define LCD_SDI_PORT    PORTBbits.RB0
#define LCD_SDI_LAT     LATBbits.LATB0
#define LCD_SDI_TRIS    TRISBbits.TRISB0
#define MODE_SW_PORT    PORTBbits.RB0
#define MODE_SW_LAT     LATBbits.LATB0
#define MODE_SW_TRIS    TRISBbits.TRISB0
/******** -1- *************************/
#define LCD_SCK_PORT    PORTBbits.RB1
#define LCD_SCK_LAT     LATBbits.LATB1
#define LCD_SCK_TRIS    TRISBbits.TRISB1
/******** -2- *************************/
#define LCD_RESET_PORT    PORTBbits.RB2
#define LCD_RESET_LAT     LATBbits.LATB2
#define LCD_RESET_TRIS    TRISBbits.TRISB2
/******** -3- *************************/
#define LCD_SDO_PORT    PORTBbits.RB3
#define LCD_SDO_LAT     LATBbits.LATB3
#define LCD_SDO_TRIS    TRISBbits.TRISB3
/******** -4- *************************/
#define LCD_CS_PORT    PORTBbits.RB4
#define LCD_CS_LAT     LATBbits.LATB4
#define LCD_CS_TRIS    TRISBbits.TRISB4
/******** -5- *************************/
#define LCD_DC_PORT    PORTBbits.RB5
#define LCD_DC_LAT     LATBbits.LATB5
#define LCD_DC_TRIS    TRISBbits.TRISB5
/******** -6- *************************/
#define FLOAT_POS_PORT    PORTBbits.RB6
#define FLOAT_POS_LAT     LATBbits.LATB6
#define FLOAT_POS_TRIS    TRISBbits.TRISB6
/******** -7- *************************/
#define FLOAT_NEG_PORT    PORTBbits.RB7
#define FLOAT_NEG_LAT     LATBbits.LATB7
#define FLOAT_NEG_TRIS    TRISBbits.TRISB7
/******** -C- *************************/
/******** -0- *************************/
#define SWITCH_1_PORT    PORTCbits.RC0
#define SWITCH_1_LAT     LATCbits.LATC0
#define SWITCH_1_TRIS    TRISCbits.TRISC0
#define SWITCH_1_IOC     IOCCbits.IOCC0
/******** -1- *************************/
#define SWITCH_2_PORT    PORTCbits.RC1
#define SWITCH_2_LAT     LATCbits.LATC1
#define SWITCH_2_TRIS    TRISCbits.TRISC1
#define SWITCH_2_IOC     IOCCbits.IOCC1
/******** -2- *************************/
#define MODEM_WAKE_PORT    PORTCbits.RC2
#define MODEM_WAKE_LAT     LATCbits.LATC2
#define MODEM_WAKE_TRIS    TRISCbits.TRISC2
#define MODEM_WAKE_IOC     IOCCbits.IOCC2
/******** -6- *************************/
#define EUSART_TX_PORT    PORTCbits.RC6
#define EUSART_TX_LAT     LATCbits.LATC6
#define EUSART_TX_TRIS    TRISCbits.TRISC6
#define EUSART_TX_ANSEL   ANSELCbits.ANSC6
/******** -7- *************************/
#define EUSART_RX_PORT    PORTCbits.RC7
#define EUSART_RX_LAT     LATCbits.LATC7
#define EUSART_RX_TRIS    TRISCbits.TRISC7
#define EUSART_RX_ANSEL   ANSELCbits.ANSC7
/*******************************************************************/
/******** Peripheral  *************************/
/*******************************************************************/
#define IOC_ENABLE  INTCONbits.IOCIE
#define IOC_FLAG    INTCONbits.IOCIF

/*******************************************************************/
/******** General  *************************/
/*******************************************************************/
#define LED_ON  1
#define LED_OFF 0

#define ENABLE  1
#define DISABLE 0

#define PWR_ENABLE 0
#define PWR_DISABLE 1

#define INPUT   1
#define OUTPUT  0

#endif  //HARDWARE_PROFILE__H
