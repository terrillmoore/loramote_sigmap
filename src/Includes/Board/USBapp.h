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

#ifndef USB_APP_H
#define USB_APP_H

typedef enum {USB_STARTUP = 0x00, USB_RUNNING = 0x01, USB_CDC = 0x02} USB_MODE_T;

void USBapp_handlerState(USB_MODE_T);
void USBapp_Handler(void);
void USBapp_CdcEmulatorInitialize(void);
void USBapp_DeviceCdcEmulatorHandler(void);
void USBapp_LEDUpdateStatus(void);
void USBapp_setrxByteFlag(void);
uint8_t USBapp_getUsbBuffer(uint8_t);
uint8_t USBapp_getBufferLength(void);

void USBapp_setrxByteFlag(void);
uint8_t USBapp_getrxByteFlag(void);

#endif
