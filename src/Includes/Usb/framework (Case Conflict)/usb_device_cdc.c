/********************************************************************************
  File Information:
    FileName:       usb_function_cdc.c
    Dependencies:   See INCLUDES section
    Processor:      PIC18 or PIC24 USB Microcontrollers
    Hardware:       The code is natively intended to be used on the following
                    hardware platforms: PICDEM(TM) FS USB Demo Board,
                    PIC18F87J50 FS USB Plug-In Module, or
                    Explorer 16 + PIC24 USB PIM.  The firmware may be
                    modified for use on other USB platforms by editing the
                    HardwareProfile.h file.
    Complier:   Microchip C18 (for PIC18) or C30 (for PIC24)
    Company:        Microchip Technology, Inc.

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
    
  Summary:
    This file contains all of functions, macros, definitions, variables,
    datatypes, etc. that are required for usage with the CDC function
    driver. This file should be included in projects that use the CDC
    \function driver.
    
    
    
    This file is located in the "\<Install Directory\>\\Microchip\\USB\\CDC
    Device Driver" directory.
  Description:
    USB CDC Function Driver File
    
    This file contains all of functions, macros, definitions, variables,
    datatypes, etc. that are required for usage with the CDC function
    driver. This file should be included in projects that use the CDC
    \function driver.
    
    This file is located in the "\<Install Directory\>\\Microchip\\USB\\CDC
    Device Driver" directory.
    
    When including this file in a new project, this file can either be
    referenced from the directory in which it was installed or copied
    directly into the user application folder. If the first method is
    chosen to keep the file located in the folder in which it is installed
    then include paths need to be added so that the library and the
    application both know where to reference each others files. If the
    application folder is located in the same folder as the Microchip
    folder (like the current demo folders), then the following include
    paths need to be added to the application's project:
    
    ..\\Include
    
    .
    
    If a different directory structure is used, modify the paths as
    required. An example using absolute paths instead of relative paths
    would be the following:
    
    C:\\Microchip Solutions\\Microchip\\Include
    
    C:\\Microchip Solutions\\My Demo Application                                 
  ********************************************************************************/

/********************************************************************
 Change History:
  Rev    Description
  ----   -----------
  2.3    Decricated the mUSBUSARTIsTxTrfReady() macro.  It is 
         replaced by the USBUSARTIsTxTrfReady() function.

  2.6    Minor definition changes

  2.6a   No Changes

  2.7    Fixed error in the part support list of the variables section
         where the address of the CDC variables are defined.  The 
         PIC18F2553 was incorrectly named PIC18F2453 and the PIC18F4558
         was incorrectly named PIC18F4458.

         http://www.microchip.com/forums/fb.aspx?m=487397

  2.8    Minor change to CDCInitEP() to enhance ruggedness in
         multithreaded usage scenarios.
  
  2.9b   Updated to implement optional support for DTS reporting.

********************************************************************/

/** I N C L U D E S **********************************************************/
#include "usb.h"
#include "usb_device_cdc.h"

#ifdef USB_USE_CDC

/** V A R I A B L E S ********************************************************/
volatile unsigned char cdc_data_tx[CDC_DATA_IN_EP_SIZE] IN_DATA_BUFFER_ADDRESS_TAG;
volatile unsigned char cdc_data_rx[CDC_DATA_OUT_EP_SIZE] OUT_DATA_BUFFER_ADDRESS_TAG;

typedef union
{
    LINE_CODING lineCoding;
    CDC_NOTICE cdcNotice;
} CONTROL_BUFFER;

LINE_CODING line_coding;    // Buffer to store line coding information
CDC_NOTICE cdc_notice;


uint8_t cdc_rx_len;            // total rx length
uint8_t cdc_trf_state;         // States are defined cdc.h
POINTER pCDCSrc;            // Dedicated source pointer
POINTER pCDCDst;            // Dedicated destination pointer
uint8_t cdc_tx_len;            // total tx length
uint8_t cdc_mem_type;          // _ROM, _RAM

USB_HANDLE CDCDataOutHandle;
USB_HANDLE CDCDataInHandle;


CONTROL_SIGNAL_BITMAP control_signal_bitmap;
uint32_t BaudRateGen;			// BRG value calculated from baudrate
/**************************************************************************
  SEND_ENCAPSULATED_COMMAND and GET_ENCAPSULATED_RESPONSE are required
  requests according to the CDC specification.
  However, it is not really being used here, therefore a dummy buffer is
  used for conformance.
 **************************************************************************/
#define dummy_length    0x08
uint8_t dummy_encapsulated_cmd_response[dummy_length];

#if defined(USB_CDC_SET_LINE_CODING_HANDLER)
CTRL_TRF_RETURN USB_CDC_SET_LINE_CODING_HANDLER(CTRL_TRF_PARAMS);
#endif

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void USBCDCSetLineCoding(void);

/** D E C L A R A T I O N S **************************************************/
//#pragma code
//#define mInitRTSPin() {TRISAbits.TRISA2 = 0;}   //Configure RTS as a digital output.
//#define mInitCTSPin() {TRISAbits.TRISA3 = 1;}   //Configure CTS as a digital input.  (Make sure pin is digital if ANxx functions is present on the pin)
//#define mInitDTSPin() {TRISBbits.TRISB4 = 1;}   //Configure DTS as a digital input.  (Make sure pin is digital if ANxx functions is present on the pin)
//#define mInitDTRPin() {TRISDbits.TRISD3 = 0;}   //Configure DTR as a digital output.
/** C L A S S  S P E C I F I C  R E Q ****************************************/
/******************************************************************************
 	Function:
 		void USBCheckCDCRequest(void)
 
 	Description:
 		This routine checks the most recently received SETUP data packet to 
 		see if the request is specific to the CDC class.  If the request was
 		a CDC specific request, this function will take care of handling the
 		request and responding appropriately.
 		
 	PreCondition:
 		This function should only be called after a control transfer SETUP
 		packet has arrived from the host.

	Parameters:
		None
		
	Return Values:
		None
		
	Remarks:
		This function does not change status or do anything if the SETUP packet
		did not contain a CDC class specific request.		 
  *****************************************************************************/
void USBCheckCDCRequest(void)
{
    /*
     * If request recipient is not an interface then return
     */
    if(SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) return;

    /*
     * If request type is not class-specific then return
     */
    if(SetupPkt.RequestType != USB_SETUP_TYPE_CLASS_BITFIELD) return;

    /*
     * Interface ID must match interface numbers associated with
     * CDC class, else return
     */
    if((SetupPkt.bIntfID != CDC_COMM_INTF_ID)&&
       (SetupPkt.bIntfID != CDC_DATA_INTF_ID)) return;
    
    switch(SetupPkt.bRequest)
    {
        //****** These commands are required ******//
        case SEND_ENCAPSULATED_COMMAND:
         //send the packet
            inPipes[0].pSrc.bRam = (uint8_t*)&dummy_encapsulated_cmd_response;
            inPipes[0].wCount.Val = dummy_length;
            inPipes[0].info.bits.ctrl_trf_mem = USB_EP0_RAM;
            inPipes[0].info.bits.busy = 1;
            break;
        case GET_ENCAPSULATED_RESPONSE:
            // Populate dummy_encapsulated_cmd_response first.
            inPipes[0].pSrc.bRam = (uint8_t*)&dummy_encapsulated_cmd_response;
            inPipes[0].info.bits.busy = 1;
            break;
        //****** End of required commands ******//
        case SET_LINE_CODING:
            outPipes[0].wCount.Val = SetupPkt.wLength;
            outPipes[0].pDst.bRam = (uint8_t*)LINE_CODING_TARGET;
            outPipes[0].pFunc = LINE_CODING_PFUNC;
            outPipes[0].info.bits.busy = 1;
            break;
            
        case GET_LINE_CODING:
            USBEP0SendRAMPtr(
                (uint8_t*)&line_coding,
                LINE_CODING_LENGTH,
                USB_EP0_INCLUDE_ZERO);
            break;

        case SET_CONTROL_LINE_STATE:
            control_signal_bitmap._byte = (uint8_t)SetupPkt.wValue;
            //------------------------------------------------------------------            
            //One way to control the RTS pin is to allow the USB host to decide the value
            //that should be output on the RTS pin.  Although RTS and CTS pin functions
            //are technically intended for UART hardware based flow control, some legacy
            //UART devices use the RTS pin like a "general purpose" output pin 
            //from the PC host.  In this usage model, the RTS pin is not related
            //to flow control for RX/TX.
            //In this scenario, the USB host would want to be able to control the RTS
            //pin, and the below line of code should be uncommented.
            //However, if the intention is to implement true RTS/CTS flow control
            //for the RX/TX pair, then this application firmware should override
            //the USB host's setting for RTS, and instead generate a real RTS signal,
            //based on the amount of remaining buffer space available for the 
            //actual hardware UART of this microcontroller.  In this case, the 
            //below code should be left commented out, but instead RTS should be 
            //controlled in the application firmware reponsible for operating the 
            //hardware UART of this microcontroller.
            //---------            
            //CONFIGURE_RTS(control_signal_bitmap.CARRIER_CONTROL);  
            //------------------------------------------------------------------            
            inPipes[0].info.bits.busy = 1;
            break;

        default:
            break;
    }//end switch(SetupPkt.bRequest)

}//end USBCheckCDCRequest

/** U S E R  A P I ***********************************************************/

/**************************************************************************
  Function:
        void CDCInitEP(void)
    
  Summary:
    This function initializes the CDC function driver. This function should
    be called after the SET_CONFIGURATION command (ex: within the context of
    the USBCBInitEP() function).
  Description:
    This function initializes the CDC function driver. This function sets
    the default line coding (baud rate, bit parity, number of data bits,
    and format). This function also enables the endpoints and prepares for
    the first transfer from the host.
    
    This function should be called after the SET_CONFIGURATION command.
    This is most simply done by calling this function from the
    USBCBInitEP() function.
    
    Typical Usage:
    <code>
        void USBCBInitEP(void)
        {
            CDCInitEP();
        }
    </code>
  Conditions:
    None
  Remarks:
    None                                                                   
  **************************************************************************/
void CDCInitEP(void)
{
    //Abstract line coding information
    line_coding.dwDTERate   = 19200;      // baud rate
    line_coding.bCharFormat = 0x00;             // 1 stop bit
    line_coding.bParityType = 0x00;             // None
    line_coding.bDataBits = 0x08;               // 5,6,7,8, or 16

    cdc_rx_len = 0;
    
    /*
     * Do not have to init Cnt of IN pipes here.
     * Reason:  Number of BYTEs to send to the host
     *          varies from one transaction to
     *          another. Cnt should equal the exact
     *          number of BYTEs to transmit for
     *          a given IN transaction.
     *          This number of BYTEs will only
     *          be known right before the data is
     *          sent.
     */
    USBEnableEndpoint(CDC_COMM_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    USBEnableEndpoint(CDC_DATA_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_rx,sizeof(cdc_data_rx));
    CDCDataInHandle = NULL;
  	
//    mInitRTSPin();
//    mInitCTSPin();
    
    cdc_trf_state = CDC_TX_READY;
}//end CDCInitEP


/**************************************************************************
  Function: void CDCNotificationHandler(void)
  Summary: Checks for changes in DSR status and reports them to the USB host.
  Description: Checks for changes in DSR pin state and reports any changes
               to the USB host. 
  Conditions: CDCInitEP() must have been called previously, prior to calling
              CDCNotificationHandler() for the first time.
  Remarks:
    This function is only implemented and needed when the 
    USB_CDC_SUPPORT_DSR_REPORTING option has been enabled.  If the function is
    enabled, it should be called periodically to sample the DSR pin and feed
    the information to the USB host.  This can be done by calling 
    CDCNotificationHandler() by itself, or, by calling CDCTxService() which
    also calls CDCNotificationHandler() internally, when appropriate.
  **************************************************************************/
#define CDCNotificationHandler() {}


/**********************************************************************************
  Function:
    bool USBCDCEventHandler(USB_EVENT event, void *pdata, uint16_t size)
    
  Summary:
    Handles events from the USB stack, which may have an effect on the CDC 
    endpoint(s).

  Description:
    Handles events from the USB stack.  This function should be called when 
    there is a USB event that needs to be processed by the CDC driver.
    
  Conditions:
    Value of input argument 'len' should be smaller than the maximum
    endpoint size responsible for receiving bulk data from USB host for CDC
    class. Input argument 'buffer' should point to a buffer area that is
    bigger or equal to the size specified by 'len'.
  Input:
    event - the type of event that occured
    pdata - pointer to the data that caused the event
    size - the size of the data that is pointed to by pdata
                                                                                   
  **********************************************************************************/
bool USBCDCEventHandler(USB_EVENT event, void *pdata, uint16_t size)
{
    switch( (uint16_t)event )
    {  
        case EVENT_TRANSFER_TERMINATED:
            if(pdata == CDCDataOutHandle)
            {
                CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_rx,sizeof(cdc_data_rx));
            }
            if(pdata == CDCDataInHandle)
            {
                //flush all of the data in the CDC buffer
                cdc_trf_state = CDC_TX_READY;
                cdc_tx_len = 0;
            }
            break;
        default:
            return false;
    }      
    return true;
}

/**********************************************************************************
  Function:
        uint8_t getsUSBUSART(char *buffer, uint8_t len)
    
  Summary:
    getsUSBUSART copies a string of BYTEs received through USB CDC Bulk OUT
    endpoint to a user's specified location. It is a non-blocking function.
    It does not wait for data if there is no data available. Instead it
    returns '0' to notify the caller that there is no data available.

  Description:
    getsUSBUSART copies a string of BYTEs received through USB CDC Bulk OUT
    endpoint to a user's specified location. It is a non-blocking function.
    It does not wait for data if there is no data available. Instead it
    returns '0' to notify the caller that there is no data available.
    
    Typical Usage:
    <code>
        uint8_t numBytes;
        uint8_t buffer[64]
    
        numBytes = getsUSBUSART(buffer,sizeof(buffer)); //until the buffer is free.
        if(numBytes \> 0)
        {
            //we received numBytes bytes of data and they are copied into
            //  the "buffer" variable.  We can do something with the data
            //  here.
        }
    </code>
  Conditions:
    Value of input argument 'len' should be smaller than the maximum
    endpoint size responsible for receiving bulk data from USB host for CDC
    class. Input argument 'buffer' should point to a buffer area that is
    bigger or equal to the size specified by 'len'.
  Input:
    buffer -  Pointer to where received BYTEs are to be stored
    len -     The number of BYTEs expected.
                                                                                   
  **********************************************************************************/
uint8_t getsUSBUSART(uint8_t *buffer, uint8_t len)
{
    cdc_rx_len = 0;
    
    if(!USBHandleBusy(CDCDataOutHandle))
    {
        /*
         * Adjust the expected number of BYTEs to equal
         * the actual number of BYTEs received.
         */
        if(len > USBHandleGetLength(CDCDataOutHandle))
            len = USBHandleGetLength(CDCDataOutHandle);
        
        /*
         * Copy data from dual-ram buffer to user's buffer
         */
        for(cdc_rx_len = 0; cdc_rx_len < len; cdc_rx_len++)
            buffer[cdc_rx_len] = cdc_data_rx[cdc_rx_len];

        /*
         * Prepare dual-ram buffer for next OUT transaction
         */

        CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_rx,sizeof(cdc_data_rx));

    }//end if
    
    return cdc_rx_len;
    
}//end getsUSBUSART

/******************************************************************************
  Function:
	void putUSBUSART(char *data, uint8_t length)
		
  Summary:
    putUSBUSART writes an array of data to the USB. Use this version, is
    capable of transfering 0x00 (what is typically a NULL character in any of
    the string transfer functions).

  Description:
    putUSBUSART writes an array of data to the USB. Use this version, is
    capable of transfering 0x00 (what is typically a NULL character in any of
    the string transfer functions).
    
    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
            putUSBUSART(data,5);
        }
    </code>
    
    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. CDCTxService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    USBUSARTIsTxTrfReady() must return true. This indicates that the last
    transfer is complete and is ready to receive a new block of data. The
    string of characters pointed to by 'data' must equal to or smaller than
    255 BYTEs.

  Input:
    char *data - pointer to a RAM array of data to be transfered to the host
    uint8_t length - the number of bytes to be transfered (must be less than 255).
		
 *****************************************************************************/
void putUSBUSART(uint8_t *data, uint8_t  length)
{
    /*
     * User should have checked that cdc_trf_state is in CDC_TX_READY state
     * before calling this function.
     * As a safety precaution, this fuction checks the state one more time
     * to make sure it does not override any pending transactions.
     *
     * Currently it just quits the routine without reporting any errors back
     * to the user.
     *
     * Bottomline: User MUST make sure that USBUSARTIsTxTrfReady()==1
     *             before calling this function!
     * Example:
     * if(USBUSARTIsTxTrfReady())
     *     putUSBUSART(pData, Length);
     *
     * IMPORTANT: Never use the following blocking while loop to wait:
     * while(!USBUSARTIsTxTrfReady())
     *     putUSBUSART(pData, Length);
     *
     * The whole firmware framework is written based on cooperative
     * multi-tasking and a blocking code is not acceptable.
     * Use a state machine instead.
     */
    USBMaskInterrupts();
    if(cdc_trf_state == CDC_TX_READY)
    {
        mUSBUSARTTxRam((uint8_t*)data, length);     // See cdc.h
    }
    USBUnmaskInterrupts();
}//end putUSBUSART

/******************************************************************************
	Function:
		void putsUSBUSART(char *data)
		
  Summary:
    putsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'puts', to transfer data from a RAM buffer.

  Description:
    putsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'puts', to transfer data from a RAM buffer.
    
    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = "Hello World";
            putsUSBUSART(data);
        }
    </code>
    
    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. CDCTxService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    USBUSARTIsTxTrfReady() must return true. This indicates that the last
    transfer is complete and is ready to receive a new block of data. The
    string of characters pointed to by 'data' must equal to or smaller than
    255 BYTEs.

  Input:
    char *data -  null\-terminated string of constant data. If a
                            null character is not found, 255 BYTEs of data
                            will be transferred to the host.
		
 *****************************************************************************/
 
void putsUSBUSART(char *data)
{
    uint8_t len;
    char *pData;

    /*
     * User should have checked that cdc_trf_state is in CDC_TX_READY state
     * before calling this function.
     * As a safety precaution, this fuction checks the state one more time
     * to make sure it does not override any pending transactions.
     *
     * Currently it just quits the routine without reporting any errors back
     * to the user.
     *
     * Bottomline: User MUST make sure that USBUSARTIsTxTrfReady()==1
     *             before calling this function!
     * Example:
     * if(USBUSARTIsTxTrfReady())
     *     putsUSBUSART(pData, Length);
     *
     * IMPORTANT: Never use the following blocking while loop to wait:
     * while(!USBUSARTIsTxTrfReady())
     *     putsUSBUSART(pData);
     *
     * The whole firmware framework is written based on cooperative
     * multi-tasking and a blocking code is not acceptable.
     * Use a state machine instead.
     */
    USBMaskInterrupts();
    if(cdc_trf_state != CDC_TX_READY)
    {
        USBUnmaskInterrupts();
        return;
    }
    
    /*
     * While loop counts the number of BYTEs to send including the
     * null character.
     */
    len = 0;
    pData = data;
    do
    {
        len++;
        if(len == 255) break;       // Break loop once max len is reached.
    }while(*pData++);
    
    /*
     * Second piece of information (length of data to send) is ready.
     * Call mUSBUSARTTxRam to setup the transfer.
     * The actual transfer process will be handled by CDCTxService(),
     * which should be called once per Main Program loop.
     */
    mUSBUSARTTxRam((uint8_t*)data, len);     // See cdc.h
    USBUnmaskInterrupts();
}//end putsUSBUSART

/**************************************************************************
  Function:
        void putrsUSBUSART(const const char *data)
    
  Summary:
    putrsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'putrs', to transfer data literals and
    data located in program memory.

  Description:
    putrsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'putrs', to transfer data literals and
    data located in program memory.
    
    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            putrsUSBUSART("Hello World");
        }
    </code>
    
    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. CDCTxService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    USBUSARTIsTxTrfReady() must return true. This indicates that the last
    transfer is complete and is ready to receive a new block of data. The
    string of characters pointed to by 'data' must equal to or smaller than
    255 BYTEs.

  Input:
    const const char *data -  null\-terminated string of constant data. If a
                            null character is not found, 255 uint8_ts of data
                            will be transferred to the host.
                                                                           
  **************************************************************************/
void putrsUSBUSART(const const char *data)
{
    uint8_t len;
    const const char *pData;

    /*
     * User should have checked that cdc_trf_state is in CDC_TX_READY state
     * before calling this function.
     * As a safety precaution, this fuction checks the state one more time
     * to make sure it does not override any pending transactions.
     *
     * Currently it just quits the routine without reporting any errors back
     * to the user.
     *
     * Bottomline: User MUST make sure that USBUSARTIsTxTrfReady()
     *             before calling this function!
     * Example:
     * if(USBUSARTIsTxTrfReady())
     *     putsUSBUSART(pData);
     *
     * IMPORTANT: Never use the following blocking while loop to wait:
     * while(cdc_trf_state != CDC_TX_READY)
     *     putsUSBUSART(pData);
     *
     * The whole firmware framework is written based on cooperative
     * multi-tasking and a blocking code is not acceptable.
     * Use a state machine instead.
     */
    USBMaskInterrupts();
    if(cdc_trf_state != CDC_TX_READY)
    {
        USBUnmaskInterrupts();
        return;
    }
    
    /*
     * While loop counts the number of BYTEs to send including the
     * null character.
     */
    len = 0;
    pData = data;
    do
    {
        len++;
        if(len == 255) break;       // Break loop once max len is reached.
    }while(*pData++);
    
    /*
     * Second piece of information (length of data to send) is ready.
     * Call mUSBUSARTTxRom to setup the transfer.
     * The actual transfer process will be handled by CDCTxService(),
     * which should be called once per Main Program loop.
     */

    mUSBUSARTTxRom((const uint8_t*)data,len); // See cdc.h
    USBUnmaskInterrupts();

}//end putrsUSBUSART

/************************************************************************
  Function:
        void CDCTxService(void)
    
  Summary:
    CDCTxService handles device-to-host transaction(s). This function
    should be called once per Main Program loop after the device reaches
    the configured state.
  Description:
    CDCTxService handles device-to-host transaction(s). This function
    should be called once per Main Program loop after the device reaches
    the configured state (after the CDCIniEP() function has already executed).
    This function is needed, in order to advance the internal software state 
    machine that takes care of sending multiple transactions worth of IN USB
    data to the host, associated with CDC serial data.  Failure to call 
    CDCTxService() perioidcally will prevent data from being sent to the
    USB host, over the CDC serial data interface.
    
    Typical Usage:
    <code>
    void main(void)
    {
        USBDeviceInit();
        while(1)
        {
            USBDeviceTasks();
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == true))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Keep trying to send data to the PC as required
                CDCTxService();
    
                //Run application code.
                UserApplication();
            }
        }
    }
    </code>
  Conditions:
    CDCIniEP() function should have already executed/the device should be
    in the CONFIGURED_STATE.
  Remarks:
    None                                                                 
  ************************************************************************/
 
void CDCTxService(void)
{
    uint8_t byte_to_send;
    uint8_t i;
    
    USBMaskInterrupts();
    
    CDCNotificationHandler();
    
    if(USBHandleBusy(CDCDataInHandle)) 
    {
        USBUnmaskInterrupts();
        return;
    }

    /*
     * Completing stage is necessary while [ mCDCUSartTxIsBusy()==1 ].
     * By having this stage, user can always check cdc_trf_state,
     * and not having to call mCDCUsartTxIsBusy() directly.
     */
    if(cdc_trf_state == CDC_TX_COMPLETING)
        cdc_trf_state = CDC_TX_READY;
    
    /*
     * If CDC_TX_READY state, nothing to do, just return.
     */
    if(cdc_trf_state == CDC_TX_READY)
    {
        USBUnmaskInterrupts();
        return;
    }
    
    /*
     * If CDC_TX_BUSY_ZLP state, send zero length packet
     */
    if(cdc_trf_state == CDC_TX_BUSY_ZLP)
    {
        CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,NULL,0);
        //CDC_DATA_BD_IN.CNT = 0;
        cdc_trf_state = CDC_TX_COMPLETING;
    }
    else if(cdc_trf_state == CDC_TX_BUSY)
    {
        /*
         * First, have to figure out how many byte of data to send.
         */
    	if(cdc_tx_len > sizeof(cdc_data_tx))
    	    byte_to_send = sizeof(cdc_data_tx);
    	else
    	    byte_to_send = cdc_tx_len;

        /*
         * Subtract the number of bytes just about to be sent from the total.
         */
    	cdc_tx_len = cdc_tx_len - byte_to_send;
    	  
        pCDCDst.bRam = (uint8_t*)&cdc_data_tx; // Set destination pointer
        
        i = byte_to_send;
        if(cdc_mem_type == USB_EP0_ROM)            // Determine type of memory source
        {
            while(i)
            {
                *pCDCDst.bRam = *pCDCSrc.bRom;
                pCDCDst.bRam++;
                pCDCSrc.bRom++;
                i--;
            }//end while(byte_to_send)
        }
        else
        {
            while(i)
            {
                *pCDCDst.bRam = *pCDCSrc.bRam;
                pCDCDst.bRam++;
                pCDCSrc.bRam++;
                i--;
            }
        }
        
        /*
         * Lastly, determine if a zero length packet state is necessary.
         * See explanation in USB Specification 2.0: Section 5.8.3
         */
        if(cdc_tx_len == 0)
        {
            if(byte_to_send == CDC_DATA_IN_EP_SIZE)
                cdc_trf_state = CDC_TX_BUSY_ZLP;
            else
                cdc_trf_state = CDC_TX_COMPLETING;
        }//end if(cdc_tx_len...)
        CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_tx,byte_to_send);

    }//end if(cdc_tx_sate == CDC_TX_BUSY)
    
    USBUnmaskInterrupts();
}//end CDCTxService

#endif //USB_USE_CDC

/** EOF cdc.c ****************************************************************/
