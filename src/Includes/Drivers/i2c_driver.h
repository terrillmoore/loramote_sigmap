//DOM-IGNORE-BEGIN
/*
    Filename:       i2c_driver.h
    Processor:      PIC16(L)F1509
    Compiler:       Microchip XC8 v1.12
    
    MICROCHIP SOFTWARE NOTICE AND DISCLAIMER:

    You may use this software, and any derivatives created by any person or
    entity by or on your behalf, exclusively with Microchip's products.
    Microchip and its licensors retain all ownership and intellectual
    property rights in the accompanying software and in all derivatives
    hereto.

    This software and any accompanying information is for suggestion only.
    It does not modify Microchip's standard warranty for its products.  You
    agree that you are solely responsible for testing the software and
    determining its suitability.  Microchip has no obligation to modify, test,
    certify, or support the software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE APPLY TO THIS SOFTWARE, ITS INTERACTION WITH MICROCHIP'S
    PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT, WILL MICROCHIP BE LIABLE, WHETHER IN CONTRACT, WARRANTY, TORT
    (INCLUDING NEGLIGENCE OR BREACH OF STATUTORY DUTY), STRICT LIABILITY,
    INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, FOR COST OR EXPENSE OF
    ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWSOEVER CAUSED, EVEN IF
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
    FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW, MICROCHIP'S TOTAL
    LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
    THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
    THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF
    THESE TERMS.
    
    Author      Date        Description
    ---------------------------------------------------------------------------
    B. Davison  19-Feb-2013 Initial revision.
*/
//DOM-IGNORE-END

#ifndef _I2C_DRIVER_H
#define _I2C_DRIVER_H

#include <stdint.h>

    
    //================================================================
    // CAPxxxx I2C Driver API
    //================================================================
    
    // Reads 'length' bytes at the data 'dataAddress' from the CAPxxxx
    // at I2C address 'deviceAddress' and stores the values in the 
    // 'data' array.
    extern uint8_t i2c_read_block(uint8_t, uint8_t*, uint8_t);

    // Writes 'length' bytes from the 'data' array to the MTCH112 data
    // location 'dataAddress' at I2C address 'deviceAddress'.
    extern uint8_t i2c_write_block(uint8_t, uint8_t*, uint8_t);
    extern uint8_t CAP_blockWriteByte(uint8_t, uint8_t);
    extern uint8_t CAP_blockRead_NoneAddress(uint8_t*,uint8_t);
    
    //================================================================
    // USER EDITS MAY BE REQUIRED
    //----------------------------------------------------------------
    //  These labels may need to be edited to match the chosen I2C 
    //  module if multiple (M)SSP modules exist on your selected PIC 
    //  microcontroller.
    //================================================================
#if defined(_16F1455) ||defined(_16F1454)
    #define I2C_SCLandSDA_MASK      0b00000011  // Used to initialize
    #define I2C_SCLandSDA_ANSEL     ANSELC      // the SDA/SCL pins in
    #define I2C_SCLandSDA_TRIS      TRISC       // I2C_init().
    #define I2C_SCLandSDA_WPU       WPUC        //
#elif defined(_16F1459) || defined(_16LF1459)
    #define I2C_SCLandSDA_MASK      0b01010000  // Used to initialize
    #define I2C_SCLandSDA_ANSEL     ANSELB      // the SDA/SCL pins in
    #define I2C_SCLandSDA_TRIS      TRISB       // I2C_init().
    #define I2C_SCLandSDA_WPU       WPUB        //
#else
    #error "Please define the SCL SDA pins"
#endif
    #define I2C_LABEL_SSPADD        SSP1ADD
    #define I2C_LABEL_SSPSTAT       SSP1STAT
    #define I2C_LABEL_SMP           SSP1STATbits.SMP
    #define I2C_LABEL_SSPCON1       SSP1CON1
    #define I2C_LABEL_SSPM          SSP1CON1bits.SSPM
    #define I2C_LABEL_SSPEN         SSP1CON1bits.SSPEN
    #define I2C_LABEL_SSPCON2       SSP1CON2
    #define I2C_LABEL_SSPCON3       SSP1CON3
    #define I2C_LABEL_SSPIF         PIR1bits.SSP1IF
    #define I2C_LABEL_SSPIE         PIE1bits.SSP1IE
    #define I2C_LABEL_BCLIF         PIR2bits.BCL1IF
    #define I2C_LABEL_BCLIE         PIE2bits.BCL1IE
    #define I2C_LABEL_ACKDT         SSP1CON2bits.ACKDT
    #define I2C_LABEL_ACKEN         SSP1CON2bits.ACKEN
    #define I2C_LABEL_ACKSTAT       SSP1CON2bits.ACKSTAT
    #define I2C_LABEL_SEN           SSP1CON2bits.SEN
    #define I2C_LABEL_RCEN          SSP1CON2bits.RCEN
    #define I2C_LABEL_RSEN          SSP1CON2bits.RSEN
    #define I2C_LABEL_PEN           SSP1CON2bits.PEN
    #define I2C_LABEL_WCOL          SSP1CON1bits.WCOL
    #define I2C_LABEL_SSPBUF        SSP1BUF
    #define I2C_LABEL_BF            SSP1STATbits.BF
    #define I2C_LABEL_RnW           SSP1STATbits.R_nW

    // The following busy flag macro checks the bit values of the
    // Start, Restart, Stop, Receive Enable, and Ack Enable bits of
    // the SSPCON2 register. The default mask values for these bits 
    // are provided by the XC8 compiler in the device header file.
    #define I2C_BUSY_FLAGS          I2C_LABEL_SSPCON2    &  \
                                  ( _SSP1CON2_SEN_MASK   |  \
                                    _SSP1CON2_RSEN_MASK  |  \
                                    _SSP1CON2_PEN_MASK   |  \
                                    _SSP1CON2_RCEN_MASK  |  \
                                    _SSP1CON2_ACKEN_MASK )

    //================================================================
    // DO NOT EDIT BELOW THIS LINE         DO NOT EDIT BELOW THIS LINE
    //================================================================
    #define I2C_error_None                  0
    #define I2C_error_NACK                  1
    #define I2C_error_BusCollision          2
    #define I2C_error_NoResponse            3
    #define I2C_error_WriteCollision        4
    #define I2C_error_ReadTimeout           5
    #define I2C_error_MaxLengthExceeded     6

    #define CAPxxxx_MAX_BUFFER_LENGTH       8
    
    //-------------------------------
    // Global Function Prototypes
    //-------------------------------
    extern void    I2C_init     (void);
    extern uint8_t I2C_read     (uint8_t* buffer,  uint8_t length);
    extern uint8_t I2C_write    (uint8_t  out);
    extern void    I2C_nack     (void);
    extern void    I2C_ack      (void);
    extern void    I2C_start    (void);
    extern void    I2C_restart  (void);
    extern void    I2C_stop     (void);
    extern void    I2C_idle     (void);

    // Global Variable
    uint8_t SLAVE_ADDR = 0x48;
#endif