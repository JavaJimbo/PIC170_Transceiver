/* 
 * File:   RF69_Robotnik.h
 * Author: JimSe
 *
 * Created on March 20, 2022, 7:02 AM
 */

//#include <stddef.h>                     // Defines NULL
//#include <stdbool.h>                    // Defines true
//#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "Defs.h"
#include "RH_RF69.h"

//#include <string.h>
//#include <ctype.h>

//#include "Defs.h"
//#include "GenericTypeDefs.h"
//#include "RH_RF69.h"
// #include <plib.h>

#ifndef RF69_ROBOTNIK_H
#define	RF69_ROBOTNIK_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define RH_HAVE_SERIAL
    
// The acknowledgement bit in the FLAGS
// The top 4 bits of the flags are reserved for RadioHead. The lower 4 bits are reserved
// for application layer use.
#define RH_FLAGS_ACK 0x80

/// the default retry timeout in milliseconds
#define RH_DEFAULT_TIMEOUT 6
    
 
#define ACK_DELAY 5  // $$$$

/// The default number of retries
#define RH_DEFAULT_RETRIES 8
    
    
    // Defines bits of the FLAGS header reserved for use by the RadioHead library and 
// the flags available for use by applications
#define RH_FLAGS_RESERVED                 0xf0
#define RH_FLAGS_APPLICATION_SPECIFIC     0x0f
#define RH_FLAGS_NONE                     0

// Default timeout for waitCAD() in ms
// #define RH_CAD_DEFAULT_TIMEOUT            10000
    
    // This is the address that indicates a broadcast
    #define RH_BROADCAST_ADDRESS 0xff    
    
    typedef enum
    {
	Frequency1MHz = 0,  ///< SPI bus frequency close to 1MHz
	Frequency2MHz,      ///< SPI bus frequency close to 2MHz
	Frequency4MHz,      ///< SPI bus frequency close to 4MHz
	Frequency8MHz,      ///< SPI bus frequency close to 8MHz
	Frequency16MHz      ///< SPI bus frequency close to 16MHz
    } SPIFrequency;    
     
        /// \brief Defines different operating modes for the transport hardware
    ///
    /// These are the different values that can be adopted by the _mode variable and 
    /// returned by the mode() member function,
    typedef enum
    {
	RHModeInitialising = 0, ///< Transport is initialising. Initial default value until init() is called..
	RHModeSleep,            ///< Transport hardware is in low power sleep mode (if supported)
	RHModeIdle,             ///< Transport is idle.
	RHModeTx,               ///< Transport is in the process of transmitting a message.
	RHModeRx,               ///< Transport is in the process of receiving a message.
	RHModeCad               ///< Transport is in the process of detecting channel activity (if supported)
    } RHMode;
    

    
    /// \brief Defines register values for a set of modem configuration registers
    ///
    /// Defines register values for a set of modem configuration registers
    /// that can be passed to setModemRegisters() if none of the choices in
    /// ModemConfigChoice suit your need setModemRegisters() writes the
    /// register values from this structure to the appropriate RF69 registers
    /// to set the desired modulation type, data rate and deviation/bandwidth.
    typedef struct
    {
	uint8_t    reg_02;   ///< Value for register RH_RF69_REG_02_DATAMODUL
	uint8_t    reg_03;   ///< Value for register RH_RF69_REG_03_BITRATEMSB
	uint8_t    reg_04;   ///< Value for register RH_RF69_REG_04_BITRATELSB
	uint8_t    reg_05;   ///< Value for register RH_RF69_REG_05_FDEVMSB
	uint8_t    reg_06;   ///< Value for register RH_RF69_REG_06_FDEVLSB
	uint8_t    reg_19;   ///< Value for register RH_RF69_REG_19_RXBW
	uint8_t    reg_1a;   ///< Value for register RH_RF69_REG_1A_AFCBW
	uint8_t    reg_37;   ///< Value for register RH_RF69_REG_37_PACKETCONFIG1
    } ModemConfig;
  
    /// Choices for setModemConfig() for a selected subset of common
    /// modulation types, and data rates. If you need another configuration,
    /// use the register calculator.  and call setModemRegisters() with your
    /// desired settings.  
    /// These are indexes into MODEM_CONFIG_TABLE. We strongly recommend you use these symbolic
    /// definitions and not their integer equivalents: its possible that new values will be
    /// introduced in later versions (though we will try to avoid it).
    /// CAUTION: some of these configurations do not work corectly and are marked as such.
    typedef enum
    {
	FSK_Rb2Fd5 = 0,	   ///< FSK, Whitening, Rb = 2kbs,    Fd = 5kHz
	FSK_Rb2_4Fd4_8,    ///< FSK, Whitening, Rb = 2.4kbs,  Fd = 4.8kHz 
	FSK_Rb4_8Fd9_6,    ///< FSK, Whitening, Rb = 4.8kbs,  Fd = 9.6kHz 
	FSK_Rb9_6Fd19_2,   ///< FSK, Whitening, Rb = 9.6kbs,  Fd = 19.2kHz
	FSK_Rb19_2Fd38_4,  ///< FSK, Whitening, Rb = 19.2kbs, Fd = 38.4kHz
	FSK_Rb38_4Fd76_8,  ///< FSK, Whitening, Rb = 38.4kbs, Fd = 76.8kHz
	FSK_Rb57_6Fd120,   ///< FSK, Whitening, Rb = 57.6kbs, Fd = 120kHz
	FSK_Rb125Fd125,    ///< FSK, Whitening, Rb = 125kbs,  Fd = 125kHz
	FSK_Rb250Fd250,    ///< FSK, Whitening, Rb = 250kbs,  Fd = 250kHz
	FSK_Rb55555Fd50,   ///< FSK, Whitening, Rb = 55555kbs,Fd = 50kHz for RFM69 lib compatibility

	GFSK_Rb2Fd5,	    ///< GFSK, Whitening, Rb = 2kbs,    Fd = 5kHz
	GFSK_Rb2_4Fd4_8,    ///< GFSK, Whitening, Rb = 2.4kbs,  Fd = 4.8kHz
	GFSK_Rb4_8Fd9_6,    ///< GFSK, Whitening, Rb = 4.8kbs,  Fd = 9.6kHz
	GFSK_Rb9_6Fd19_2,   ///< GFSK, Whitening, Rb = 9.6kbs,  Fd = 19.2kHz
	GFSK_Rb19_2Fd38_4,  ///< GFSK, Whitening, Rb = 19.2kbs, Fd = 38.4kHz
	GFSK_Rb38_4Fd76_8,  ///< GFSK, Whitening, Rb = 38.4kbs, Fd = 76.8kHz
	GFSK_Rb57_6Fd120,   ///< GFSK, Whitening, Rb = 57.6kbs, Fd = 120kHz
	GFSK_Rb125Fd125,    ///< GFSK, Whitening, Rb = 125kbs,  Fd = 125kHz
	GFSK_Rb250Fd250,    ///< GFSK, Whitening, Rb = 250kbs,  Fd = 250kHz
	GFSK_Rb55555Fd50,   ///< GFSK, Whitening, Rb = 55555kbs,Fd = 50kHz

	OOK_Rb1Bw1,         ///< OOK, Whitening, Rb = 1kbs,    Rx Bandwidth = 1kHz. 
	OOK_Rb1_2Bw75,      ///< OOK, Whitening, Rb = 1.2kbs,  Rx Bandwidth = 75kHz. 
	OOK_Rb2_4Bw4_8,     ///< OOK, Whitening, Rb = 2.4kbs,  Rx Bandwidth = 4.8kHz. 
	OOK_Rb4_8Bw9_6,     ///< OOK, Whitening, Rb = 4.8kbs,  Rx Bandwidth = 9.6kHz. 
	OOK_Rb9_6Bw19_2,    ///< OOK, Whitening, Rb = 9.6kbs,  Rx Bandwidth = 19.2kHz. 
	OOK_Rb19_2Bw38_4,   ///< OOK, Whitening, Rb = 19.2kbs, Rx Bandwidth = 38.4kHz. 
	OOK_Rb32Bw64,       ///< OOK, Whitening, Rb = 32kbs,   Rx Bandwidth = 64kHz. 

//	Test,
    } ModemConfigChoice;

// These are indexed by the values of ModemConfigChoice
// Stored in flash (program) memory to save SRAM
// It is important to keep the modulation index for FSK between 0.5 and 10
// modulation index = 2 * Fdev / BR
// Note that I have not had much success with FSK with Fd > ~5
// You have to construct these by hand, using the data from the RF69 Datasheet :-(
// or use the SX1231 starter kit software (Ctl-Alt-N to use that without a connected radio)
#define CONFIG_FSK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_FSK | RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_NONE)
#define CONFIG_GFSK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_FSK | RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0)
#define CONFIG_OOK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_OOK | RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_NONE)

// Choices for RH_RF69_REG_37_PACKETCONFIG1:
#define CONFIG_NOWHITE (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_NONE | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
#define CONFIG_WHITE (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_WHITENING | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
#define CONFIG_MANCHESTER (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_MANCHESTER | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
    

    
static const ModemConfig MODEM_CONFIG_TABLE[] =
{
    //  02,        03,   04,   05,   06,   19,   1a,  37
    // FSK, No Manchester, no shaping, whitening, CRC, no address filtering
    // AFC BW == RX BW == 2 x bit rate
    // Low modulation indexes of ~ 1 at slow speeds do not seem to work very well. Choose MI of 2.
    { CONFIG_FSK,  0x3e, 0x80, 0x00, 0x52, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb2Fd5
    { CONFIG_FSK,  0x34, 0x15, 0x00, 0x4f, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb2_4Fd4_8
    { CONFIG_FSK,  0x1a, 0x0b, 0x00, 0x9d, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb4_8Fd9_6

    { CONFIG_FSK,  0x0d, 0x05, 0x01, 0x3b, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb9_6Fd19_2
    { CONFIG_FSK,  0x06, 0x83, 0x02, 0x75, 0xf3, 0xf3, CONFIG_WHITE}, // FSK_Rb19_2Fd38_4
    { CONFIG_FSK,  0x03, 0x41, 0x04, 0xea, 0xf2, 0xf2, CONFIG_WHITE}, // FSK_Rb38_4Fd76_8

    { CONFIG_FSK,  0x02, 0x2c, 0x07, 0xae, 0xe2, 0xe2, CONFIG_WHITE}, // FSK_Rb57_6Fd120
    { CONFIG_FSK,  0x01, 0x00, 0x08, 0x00, 0xe1, 0xe1, CONFIG_WHITE}, // FSK_Rb125Fd125
    { CONFIG_FSK,  0x00, 0x80, 0x10, 0x00, 0xe0, 0xe0, CONFIG_WHITE}, // FSK_Rb250Fd250 ***     
    { CONFIG_FSK,  0x02, 0x40, 0x03, 0x33, 0x42, 0x42, CONFIG_WHITE}, // FSK_Rb55555Fd50

    //  02,        03,   04,   05,   06,   19,   1a,  37
    // GFSK (BT=1.0), No Manchester, whitening, CRC, no address filtering
    // AFC BW == RX BW == 2 x bit rate
    { CONFIG_GFSK, 0x3e, 0x80, 0x00, 0x52, 0xf4, 0xf5, CONFIG_WHITE}, // GFSK_Rb2Fd5
    { CONFIG_GFSK, 0x34, 0x15, 0x00, 0x4f, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb2_4Fd4_8
    { CONFIG_GFSK, 0x1a, 0x0b, 0x00, 0x9d, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb4_8Fd9_6

    { CONFIG_GFSK, 0x0d, 0x05, 0x01, 0x3b, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb9_6Fd19_2
    { CONFIG_GFSK, 0x06, 0x83, 0x02, 0x75, 0xf3, 0xf3, CONFIG_WHITE}, // GFSK_Rb19_2Fd38_4
    { CONFIG_GFSK, 0x03, 0x41, 0x04, 0xea, 0xf2, 0xf2, CONFIG_WHITE}, // GFSK_Rb38_4Fd76_8

    { CONFIG_GFSK, 0x02, 0x2c, 0x07, 0xae, 0xe2, 0xe2, CONFIG_WHITE}, // GFSK_Rb57_6Fd120
    { CONFIG_GFSK, 0x01, 0x00, 0x08, 0x00, 0xe1, 0xe1, CONFIG_WHITE}, // GFSK_Rb125Fd125
    { CONFIG_GFSK, 0x00, 0x80, 0x10, 0x00, 0xe0, 0xe0, CONFIG_WHITE}, // GFSK_Rb250Fd250
    { CONFIG_GFSK, 0x02, 0x40, 0x03, 0x33, 0x42, 0x42, CONFIG_WHITE}, // GFSK_Rb55555Fd50

    //  02,        03,   04,   05,   06,   19,   1a,  37
    // OOK, No Manchester, no shaping, whitening, CRC, no address filtering
    // with the help of the SX1231 configuration program
    // AFC BW == RX BW
    // All OOK configs have the default:
    // Threshold Type: Peak
    // Peak Threshold Step: 0.5dB
    // Peak threshiold dec: ONce per chip
    // Fixed threshold: 6dB
    { CONFIG_OOK,  0x7d, 0x00, 0x00, 0x10, 0x88, 0x88, CONFIG_WHITE}, // OOK_Rb1Bw1
    { CONFIG_OOK,  0x68, 0x2b, 0x00, 0x10, 0xf1, 0xf1, CONFIG_WHITE}, // OOK_Rb1_2Bw75
    { CONFIG_OOK,  0x34, 0x15, 0x00, 0x10, 0xf5, 0xf5, CONFIG_WHITE}, // OOK_Rb2_4Bw4_8
    { CONFIG_OOK,  0x1a, 0x0b, 0x00, 0x10, 0xf4, 0xf4, CONFIG_WHITE}, // OOK_Rb4_8Bw9_6
    { CONFIG_OOK,  0x0d, 0x05, 0x00, 0x10, 0xf3, 0xf3, CONFIG_WHITE}, // OOK_Rb9_6Bw19_2
    { CONFIG_OOK,  0x06, 0x83, 0x00, 0x10, 0xf2, 0xf2, CONFIG_WHITE}, // OOK_Rb19_2Bw38_4
    { CONFIG_OOK,  0x03, 0xe8, 0x00, 0x10, 0xe2, 0xe2, CONFIG_WHITE}, // OOK_Rb32Bw64

//    { CONFIG_FSK,  0x68, 0x2b, 0x00, 0x52, 0x55, 0x55, CONFIG_WHITE}, // works: Rb1200 Fd 5000 bw10000, DCC 400
//    { CONFIG_FSK,  0x0c, 0x80, 0x02, 0x8f, 0x52, 0x52, CONFIG_WHITE}, // works 10/40/80
//    { CONFIG_FSK,  0x0c, 0x80, 0x02, 0x8f, 0x53, 0x53, CONFIG_WHITE}, // works 10/40/40
};    
    

BYTE RH_RF69_init(void);
void InitPIC32_SPI(unsigned long SPIbitrate);
uint8_t SendReceiveSPI(uint8_t dataOut);
uint8_t SPItransfer(uint8_t dataOut);
uint8_t  spiRead(uint8_t reg);
uint8_t  spiWrite(uint8_t reg, uint8_t val);
uint8_t  spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len);
uint8_t  spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len);
void setModeRx();
void setModeTx();
void setTxPower(int8_t power, BYTE ishighpowermodule);
void setModemRegisters(const ModemConfig* config);
BYTE setModemConfig(ModemConfigChoice index);
void setPreambleLength(uint16_t bytes);
void setSyncWords(const uint8_t* syncWords, uint8_t len);
void setEncryptionKey(uint8_t* key);
void setSPIFrequency(SPIFrequency frequency);
BYTE available();
// BYTE recv(uint8_t* buf, uint8_t* len);
BYTE send(const uint8_t* data, uint8_t len);
uint16_t getRxMessage (uint8_t *ptrMessage, uint16_t *MessageLength);
uint16_t maxMessageLength();
void setOpMode(uint8_t mode);
BYTE waitPacketSent(uint16_t timeout);
void RH_RF69readFifo();
// BYTE RHrecvfrom(uint8_t* buf, uint8_t* len, uint8_t* from);
BYTE available();
void setModeIdle();
BYTE printRegisters();
BYTE printRegister(uint8_t reg);
BYTE setFrequency(float centre);
BYTE rssiRead();
BYTE sleep();
void setHeaderId(uint8_t id);
BYTE sendto(uint8_t* buf, uint8_t len, uint8_t address);
void setHeaderTo(uint8_t to);
// BYTE waitAvailableTimeout(uint16_t timeout);
bool sendtoWait(uint8_t* buf, uint8_t len, uint8_t address);
void acknowledge(uint8_t id, uint8_t from);
void setHeaderFlags(uint8_t set, uint8_t clear);
BYTE recvfrom(uint8_t* buf, uint16_t* len, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags);
BYTE recv(uint8_t* buf, uint16_t* len);
// BYTE waitAvailableTimeout(uint16_t timeout);
bool recvfromAck(uint8_t* buf, uint16_t* len, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags);
// bool recvfromAckTimeout(uint8_t* buf, uint8_t* len, uint16_t timeout, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags);
void setHeaderFrom(uint8_t from);
void setThisAddress(uint8_t thisAddress);
uint32_t GetRetramissions();

#ifdef	__cplusplus
}
#endif

#endif	/* RF69_ROBOTNIK_H */

