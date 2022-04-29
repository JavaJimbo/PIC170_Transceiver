/*******************************************************************************
 * 
 * 
 * 
 * PROJECT: PIC170_28DUP_DEMO
 * For PIC 32MX170F256B 28 pin micro
 * C language generated by Harmony 3 using XC32 Compiler V3.01
 * 
 * 3-25-22:  
 * 3-26-22: SUCCESS! Transmitting and receiving both work!!!
 * 3-26-22: Speed test results: 266 transmissions per second. 
 *              59 bytes per transmission, or 15733 bytes total per second
 * 3-27-22: Update: 250 x 59 = 14750 bytes per second.    
 * 3-29-22: Retested at above rate.
 * 4-18-22: Recompiled - minor tweaks
 * 4-20-22: Baudrate = 57600.
 * 4-22-22: Recompiled. Works as simple XBEE mode.
 * 4-23-22: Increased payload to max 59 bytes and acking one byte from receiver.
 *          Now transmitting packets every 5.6 milliseconds or over 10500 bytes/second.
 *          Error rate is about 0.27% or 1882 packets out of 683619 
 *          using Adafruit breakout for transmitter.
 * 4-25-22: Added ResetMillis() and GetMillis()
 * 4-27-22: Got UART 1 working at 57600 baud.
 *          Completed code for XBEE emulator at 57600 baud for both UARTs 1 and 2
 * 4-28-22: Extended MAXBUFFER to > 59 bytes and packets are divided up into multiple sends
 *          Increased XBEE baudrate to 921600 to work with XBEE UART on PIC795 H Bridge Controller.
 *          Works well with MIDI pot board.
 *
 *******************************************************************************/
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "Defs.h"
#include "RH_RF69.h"
#include "RF69_Robotnik.h"
#include <string.h>
#include <ctype.h>

extern volatile uint16_t    _txGood;
extern volatile int16_t     _lastRssi;
extern volatile RHMode     _mode;

void RHM69_RxCallback (GPIO_PIN pin, uintptr_t context);
BYTE PackPacket(BYTE *ptrData);

#define FOREVER 1

unsigned char HOSTRxBuffer[MAXBUFFER+1];
short HOSTRxIndex = 0;
short HOSTRxLength = 0;
BYTE HOSTRxBufferFull = FALSE;
BYTE UART1RxBuffer[MAXBUFFER+1];
unsigned char UART1RxBufferFull = false;
char UART1TxBuffer[MAXBUFFER+32] = "\rThis has got to be the one";
short UART1TxLength = 0;
unsigned long DelayTimer = 0;
void DelayMs(unsigned long DelayMilliSeconds);
void ResetUART1(void);

char MessageOut[256] = "\rUART1 READY";


void ResetUART1(void)
{
    UART1_ReadAbort();                                                   // Disable RX interrupts and reset flags    
    UART1_Start(UART1RxBuffer);                                // Re-enable RX interrupts     
}

void UART1_FaultCallback (uintptr_t context)
{  
    ;
}

int main (void)
{       
#ifdef TX_MASTER    
    uint8_t i;
    uint8_t length = 0;
    int errorCounter = 0;
    int ErrorCounter = 0;
    uint8_t DestinationAddress = DESTINATION1_ADDRESS;

    BYTE RadioTxPacket[MAXBUFFER+1], ch;
    int NumRetransmissions = 0;
    int TotalPacketsSent = 0;
    UART_ERROR UART1_Errors = 0;
#endif        
    
    SYS_Initialize ( NULL );
    ERROR_LED_Clear();     
    TMR3_InterruptEnable();    

    DelayMs(100);
        
    UART1_Write (MessageOut, strlen(MessageOut));
    ResetUART1();

    GPIO_PinInterruptCallbackRegister(INT0_PIN, RHM69_RxCallback, (uintptr_t)NULL);   
    INT0_InterruptEnable();
    
#ifdef TX_MASTER     
    printf("\r\rTESTING TRANSMIITER XBEE MODE");
#else
    printf("\r\rTESTING RECEIVER XBEE MODE - MY ADDRESS: %d", MY_ADDRESS);
#endif    
    if (!RH_RF69_init()) printf("\rRFM69 INITIALIZATION ERROR");
    else printf("\rSUCCESS - RFM69 INITIAILIZED");
  
    printf("\rPRINTING REGISTERS:");
    printRegister(0x02);
    printRegister(0x03);
    printRegister(0x04);
    printRegister(0x05);
    printRegister(0x06);
    printRegister(0x19);
    printRegister(0x1a);
    printRegister(0x37);

    
    setFrequency(RF69_FREQ);
    setTxPower(20, true);
    // The encryption key has to be the same as the one in the server
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
   setEncryptionKey(key);    
    
    
    DelayMs(100);    
    while(FOREVER)
    {
        WDTCONbits.WDTCLR = 1;
        
        SYS_Tasks( );     
        
#ifdef TX_MASTER         
       // length = PackPacket(RadioTxPacket);
       // Send a message to the DESTINATION and wait for ACK:     
        //  TotalPacketsSent++;
                                  
        UART1_Errors = UART1_ErrorGet();
        if (UART1_Errors)
        {
            switch (UART1_Errors)
            {
                case UART_ERROR_OVERRUN: printf("\rUART1 OVERRUN ERROR");
                break;
                
                case UART_ERROR_FRAMING: printf("\rUART1 FRAMING ERROR");
                break;

                case UART_ERROR_PARITY: printf("\rUART1 PARITY ERROR");
                break;                
                
                default: printf("\rUNKNOWN UART1 ERROR");
                    break;
            }
            ResetUART1();
        }
        if (UART1BufferFull())
        {   
            int j = 0;
            do {
                i = 0;                 
                do {
                    ch = (BYTE) UART1RxBuffer[j++];
                    RadioTxPacket[i++] = ch;
                    if (ch == ETX) break;
                } while (i < RH_RF69_MAX_MESSAGE_LEN);                
                length = i;                
                ResetUART1();
                if (!sendtoWait(RadioTxPacket, length, DestinationAddress)) 
                {
                    printf("\r#%d: Sending failed (no ack), %d retries", errorCounter++, (int) GetRetramissions());
                    LED_Clear();
                    break;
                }            
                NumRetransmissions = (int) GetRetramissions();
                TotalPacketsSent++;
                if (NumRetransmissions) printf("\r#%d: Total error count: %d: Retries = %d", TotalPacketsSent, ++ErrorCounter, NumRetransmissions);    
                else
                {
                    printf("\r#%d SENT %d BYTES", TotalPacketsSent, (int)length);
                    LED_Set();
                    ResetMillis();
                }
            } while(ch != ETX && j < MAXBUFFER);
        }
        else if (GetMillis() == 100) 
            LED_Clear();
#else
        
        if (available())
        {     
            uint8_t from;
            uint16_t length = MAXBUFFER;
            if (recvfromAck(UART1RxBuffer, &length, &from, NULL,NULL,NULL))
            {
                printf("\rReceive length: %d", length);                                
                UART1_Write(UART1RxBuffer, length);                
                LED_Set();
                ResetMillis();
            }   
            else printf("\rRECEIVE ERROR");
        }    
        else if (GetMillis() == 100) LED_Clear();             
#endif        
        
    } // end while(FOREVER)
    return 0;
}


void DelayMs (unsigned long DelayMilliSeconds)
{
    ResetMillis();
    while(GetMillis() < DelayMilliSeconds);
}

void RHM69_RxCallback (GPIO_PIN pin, uintptr_t context)
{    
#ifndef TX_TEST    
    if (PUSH1_Get() == 0)
    {
        ERROR_LED_Clear();
    }
#endif    
    
    if (INT0_Get() != 0)
    {
           
        if (_mode == RHModeTx)
        {
            // A transmitter message has been fully sent
            setModeIdle(); // Clears FIFO
            _txGood++;
        }
        // Must look for PAYLOADREADY, not CRCOK, since only PAYLOADREADY occurs _after_ AES decryption
        // has been done
        if (_mode == RHModeRx)
        {
            // A complete message has been received with good CRC
            _lastRssi = -((int8_t)(spiRead(RH_RF69_REG_24_RSSIVALUE) >> 1));
            // _lastPreambleTime = millis();

            setModeIdle();
            // Save it in our buffer
            RH_RF69readFifo();
        }    
    }
}


#define MAXPAYLOAD 59
BYTE PackPacket(BYTE *ptrData)
{
    BYTE dataByte = 'A';
    static BYTE firstByte = 'A';
    BYTE i;
    
    dataByte = firstByte++;
    if (firstByte > 'Z') firstByte = 'A';
    
    for (i = 0; i < MAXPAYLOAD; i++)
    {
        if (i == MAXPAYLOAD-1) ptrData[i] = '\0';
        else ptrData[i] = dataByte++;
        if (dataByte > 'Z') dataByte = 'A';
    }
    return i;
}
     