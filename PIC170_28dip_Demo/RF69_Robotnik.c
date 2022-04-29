//#include <plib.h>
#include "Defs.h"
#include "RH_RF69.h"
#include "RF69_Robotnik.h"
#include <string.h>
//#include "Delay.h"

    extern void DelayMs(unsigned long DelayMilliSeconds);
    extern unsigned long milliseconds;
    
        uint8_t _seenIds[256];
    
    /// Count of retransmissions we have had to send
    uint32_t _retransmissions;

    /// The last sequence number to be used
    /// Defaults to 0
    uint8_t _lastSequenceNumber;

    // Retransmit timeout (milliseconds)
    /// Defaults to 200
    uint32_t _timeout;   // Was uint16_t

    // Retries (0 means one try only)
    /// Defaults to 3
    uint8_t _retries;    

    uint16_t MessageLength = 0;

    // The selected output power in dBm
    int8_t _power = 0;

    /// Time in millis since the last preamble was received (and the last time the RSSI was measured)
    // uint32_t            _lastPreambleTime;

    /// The radio OP mode to use when mode is RHModeIdle
    uint8_t             _idleMode; 
        
   /// The current transport operating mode    
    volatile RHMode     _mode = RHModeInitialising;
    
    /// True when there is a valid message in the Rx buffer
    volatile BYTE    _rxBufValid;    
    
    /// The message length in _buf
    volatile uint16_t    _bufLen;    
    
    /// Array of octets of teh last received message or the next to transmit message
    uint8_t             _buf[RH_RF69_MAX_MESSAGE_LEN];    

    /// This node id
    uint8_t             _thisAddress = RH_BROADCAST_ADDRESS;
    
    /// Whether the transport is in promiscuous mode
    BYTE                _promiscuous;

    /// TO header in the last received mesasge
    volatile uint8_t    _rxHeaderTo;

    /// FROM header in the last received mesasge
    volatile uint8_t    _rxHeaderFrom;

    /// ID header in the last received mesasge
    volatile uint8_t    _rxHeaderId;

    /// FLAGS header in the last received mesasge
    volatile uint8_t    _rxHeaderFlags;

    /// TO header to send in all messages
    uint8_t             _txHeaderTo = RH_BROADCAST_ADDRESS;

    /// FROM header to send in all messages
    uint8_t             _txHeaderFrom = RH_BROADCAST_ADDRESS;

    /// ID header to send in all messages
    uint8_t             _txHeaderId = 0;

    /// FLAGS header to send in all messages
    uint8_t             _txHeaderFlags = 0;

    /// The value of the last received RSSI value, in some transport specific units
    volatile int16_t     _lastRssi;

    /// Count of the number of bad messages (eg bad checksum etc) received
    volatile uint16_t   _rxBad = 0;

    /// Count of the number of successfully transmitted messaged
    volatile uint16_t   _rxGood = 0;

    /// Count of the number of bad messages (correct checksum etc) received
    volatile uint16_t   _txGood = 0;;
    
    /// Channel activity detected
    volatile BYTE       _cad = 0;

    /// Channel activity timeout in ms
    unsigned int        _cad_timeout = 0;
   
   
    
BYTE RH_RF69_init(void)
{
    // InitPIC32_SPI(1000000); // Use 1 Mhz for RFM69 SPI clock
    setThisAddress(MY_ADDRESS);
    _retransmissions = 0;
    _lastSequenceNumber = 0;
    _timeout = RH_DEFAULT_TIMEOUT;
    _retries = RH_DEFAULT_RETRIES;
    memset(_seenIds, 0, sizeof(_seenIds));    
    
    setModeIdle();
    
    // Configure important RH_RF69 registers
    // Here we set up the standard packet format for use by the RH_RF69 library:
    // 4 bytes preamble
    // 2 SYNC words 2d, d4
    // 2 CRC CCITT octets computed on the header, length and data (this in the modem config data)
    // 0 to 60 bytes data
    // RSSI Threshold -114dBm
    // We dont use the RH_RF69s address filtering: instead we prepend our own headers to the beginning
    // of the RH_RF69 payload       
    
    spiWrite(RH_RF69_REG_3C_FIFOTHRESH, RH_RF69_FIFOTHRESH_TXSTARTCONDITION_NOTEMPTY | 0x0f); // thresh 15 is default        
    
    // RSSITHRESH is default
//    spiWrite(RH_RF69_REG_29_RSSITHRESH, 220); // -110 dbM
    // SYNCCONFIG is default. SyncSize is set later by setSyncWords()
//    spiWrite(RH_RF69_REG_2E_SYNCCONFIG, RH_RF69_SYNCCONFIG_SYNCON); // auto, tolerance 0
    // PAYLOADLENGTH is default
//    spiWrite(RH_RF69_REG_38_PAYLOADLENGTH, RH_RF69_FIFO_SIZE); // max size only for RX
    // PACKETCONFIG 2 is default
    spiWrite(RH_RF69_REG_6F_TESTDAGC, RH_RF69_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAOFF);
    // If high power boost set previously, disable it
    spiWrite(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_NORMAL);
    spiWrite(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_NORMAL);

    // The following can be changed later by the user if necessary.
    // Set up default configuration
    uint8_t syncwords[] = { 0x2d, 0xd4 };
    setSyncWords(syncwords, sizeof(syncwords)); // Same as RF22's
    // Reasonably fast and reliable default speed and modulation
    setModemConfig(GFSK_Rb250Fd250);

    // 3 would be sufficient, but this is the same as RF22's
    setPreambleLength(4);
    
    // setFrequency(434.0);    
    setFrequency(RF69_FREQ);
            
    // No encryption
    setEncryptionKey(NULL);
    // +13dBm, same as power-on default
    setTxPower(13, false);

    return true;
}
    

// srcClkDiv - Source Clock divisor to extract the bitrate=srcClk/srcClkDiv
unsigned int GetPICSrcClkDiv (unsigned long SPIbitrate)
{    
unsigned int SourceClockDivisor = 0;

    if (SPIbitrate == 0) return 2;
    SourceClockDivisor = (unsigned int) (GetPeripheralClock() / SPIbitrate);
    if (SourceClockDivisor > 1024) SourceClockDivisor = 1024;
    else if (SourceClockDivisor < 2) SourceClockDivisor = 2;
    return SourceClockDivisor;
}
    

unsigned char SendReceiveSPI(unsigned char data)
{
    SPI1BUF = data; // Send byte to SPI2BUF.
    while (SPI1STATbits.SPIRBE); // Wait for 
    return SPI1BUF; // Return byte in SPI2BUF
}

uint8_t spiWrite (uint8_t reg, uint8_t val)
{
uint8_t dataIn;

    RFM69_CS_LOW();
    SendReceiveSPI(reg | RH_SPI_WRITE_MASK);
    dataIn = SendReceiveSPI(val);
    RFM69_CS_HIGH();
    
    return (dataIn);
}

uint8_t spiRead(uint8_t reg)
{
    uint8_t val;
    
    RFM69_CS_LOW();
    SPItransfer(reg & ~RH_SPI_WRITE_MASK); // Send the address with the write mask off
    val = SPItransfer(0); // The written value is ignored, reg value is read
    RFM69_CS_HIGH();
    
    return val;
}

uint8_t SPItransfer(uint8_t dataOut)
{
uint8_t dataIn;
    
    dataIn = SendReceiveSPI(dataOut);
    return (dataIn);    
}

uint8_t spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
{
    uint8_t status = 0;
        
    RFM69_CS_LOW();
    status = SPItransfer(reg | RH_SPI_WRITE_MASK); // Send the start address with the write mask on
    while (len--)
	SPItransfer(*src++);
    RFM69_CS_HIGH();       
        
    return status;
}

void setModeRx()
{
    if (_mode != RHModeRx)
    {
	if (_power >= 18)
	{
	    // If high power boost, return power amp to receive mode
	    spiWrite(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_NORMAL);
	    spiWrite(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_NORMAL);
	}
	spiWrite(RH_RF69_REG_25_DIOMAPPING1, RH_RF69_DIOMAPPING1_DIO0MAPPING_01); // Set interrupt line 0 PayloadReady
	setOpMode(RH_RF69_OPMODE_MODE_RX); // Clears FIFO
	_mode = RHModeRx;
    }
}

void setModeTx()
{
    if (_mode != RHModeTx)
    {
	if (_power >= 18)
	{
	    // Set high power boost mode
	    // Note that OCP defaults to ON so no need to change that.
	    spiWrite(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_BOOST);
	    spiWrite(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_BOOST);
	}
	spiWrite(RH_RF69_REG_25_DIOMAPPING1, RH_RF69_DIOMAPPING1_DIO0MAPPING_00); // Set interrupt line 0 PacketSent
	setOpMode(RH_RF69_OPMODE_MODE_TX); // Clears FIFO
	_mode = RHModeTx;
    }
}

void setTxPower(int8_t power, BYTE ishighpowermodule)
{
  _power = power;
  uint8_t palevel;

  if (ishighpowermodule)
  {
    if (_power < -2)
      _power = -2; //RFM69HW only works down to -2.
    if (_power <= 13)
    {
      // -2dBm to +13dBm
      //Need PA1 exclusivelly on RFM69HW
      palevel = RH_RF69_PALEVEL_PA1ON | ((_power + 18) &
      RH_RF69_PALEVEL_OUTPUTPOWER);
    }
    else if (_power >= 18)
    {
      // +18dBm to +20dBm
      // Need PA1+PA2
      // Also need PA boost settings change when tx is turned on and off, see setModeTx()
      palevel = RH_RF69_PALEVEL_PA1ON
	| RH_RF69_PALEVEL_PA2ON
	| ((_power + 11) & RH_RF69_PALEVEL_OUTPUTPOWER);
    }
    else
    {
      // +14dBm to +17dBm
      // Need PA1+PA2
      palevel = RH_RF69_PALEVEL_PA1ON
	| RH_RF69_PALEVEL_PA2ON
	| ((_power + 14) & RH_RF69_PALEVEL_OUTPUTPOWER);
    }
  }
  else
  {
    if (_power < -18) _power = -18;
    if (_power > 13) _power = 13; //limit for RFM69W
    palevel = RH_RF69_PALEVEL_PA0ON
      | ((_power + 18) & RH_RF69_PALEVEL_OUTPUTPOWER);
  }
  spiWrite(RH_RF69_REG_11_PALEVEL, palevel);
}

// Sets registers from a canned modem configuration structure
void setModemRegisters(const ModemConfig* config)
{
    spiBurstWrite(RH_RF69_REG_02_DATAMODUL,     &config->reg_02, 5);
    spiBurstWrite(RH_RF69_REG_19_RXBW,          &config->reg_19, 2);    
    spiWrite(RH_RF69_REG_37_PACKETCONFIG1,       config->reg_37);    
}

// Set one of the canned FSK Modem configs
// Returns true if its a valid choice
BYTE setModemConfig(ModemConfigChoice index)
{
    if (index > (signed int)(sizeof(MODEM_CONFIG_TABLE) / sizeof(ModemConfig)))
        return false;

    ModemConfig cfg;
    memcpy(&cfg, &MODEM_CONFIG_TABLE[index], sizeof(ModemConfig));        
    setModemRegisters(&cfg);
    return true;
}

void setPreambleLength(uint16_t bytes)
{
    spiWrite(RH_RF69_REG_2C_PREAMBLEMSB, bytes >> 8);
    spiWrite(RH_RF69_REG_2D_PREAMBLELSB, bytes & 0xff);
}

void setSyncWords(const uint8_t* syncWords, uint8_t len)
{
    uint8_t syncconfig = spiRead(RH_RF69_REG_2E_SYNCCONFIG);
    if (syncWords && len && len <= 4)
    {
	spiBurstWrite(RH_RF69_REG_2F_SYNCVALUE1, syncWords, len);
	syncconfig |= RH_RF69_SYNCCONFIG_SYNCON;
    }
    else
	syncconfig &= ~RH_RF69_SYNCCONFIG_SYNCON;
    syncconfig &= ~RH_RF69_SYNCCONFIG_SYNCSIZE;
    syncconfig |= (len-1) << 3;
    spiWrite(RH_RF69_REG_2E_SYNCCONFIG, syncconfig);
}

void setEncryptionKey(uint8_t* key)
{
    if (key)
    {
	spiBurstWrite(RH_RF69_REG_3E_AESKEY1, key, 16);
	spiWrite(RH_RF69_REG_3D_PACKETCONFIG2, spiRead(RH_RF69_REG_3D_PACKETCONFIG2) | RH_RF69_PACKETCONFIG2_AESON);
    }
    else
    {
	spiWrite(RH_RF69_REG_3D_PACKETCONFIG2, spiRead(RH_RF69_REG_3D_PACKETCONFIG2) & ~RH_RF69_PACKETCONFIG2_AESON);
    }
}

BYTE available()
{
    if (_mode == RHModeTx)
	return false;
    setModeRx(); // Make sure we are receiving
    return _rxBufValid;
}

BYTE recv(uint8_t* buf, uint16_t* len)
{
    if (!available())
	return false;

    if (buf && len)
    {        
        if (_bufLen > MAXBUFFER) return false;
        *len = _bufLen;
        memcpy(buf, _buf, *len);
    }
    _rxBufValid = false; // Got the most recent message    
    return true;
}

void setModeIdle()
{
    if (_mode != RHModeIdle)
    {
	if (_power >= 18)
	{
	    // If high power boost, return power amp to receive mode
	    spiWrite(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_NORMAL);
	    spiWrite(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_NORMAL);
	}
	setOpMode(_idleMode);
	_mode = RHModeIdle;
    }
}


BYTE send(const uint8_t* data, uint8_t len)
{
    if (len > RH_RF69_MAX_MESSAGE_LEN)
	return false;

    waitPacketSent(10); // Make sure we dont interrupt an outgoing message  $$$$
    setModeIdle(); // Prevent RX while filling the fifo

    //if (!waitCAD())
	//return false;  // Check channel activity

    
    RFM69_CS_LOW();
    SPItransfer(RH_RF69_REG_00_FIFO | RH_RF69_SPI_WRITE_MASK); // Send the start address with the write mask on
    SPItransfer(len + RH_RF69_HEADER_LEN); // Include length of headers
    
    // First the 4 headers
    SPItransfer(_txHeaderTo);
    SPItransfer(_txHeaderFrom);
    SPItransfer(_txHeaderId);
    SPItransfer(_txHeaderFlags);
    // Now the payload
    while (len--)
	SPItransfer(*data++);
    RFM69_CS_HIGH();
    
    setModeTx(); // Start the transmitter
    return true;
}

BYTE waitPacketSent(uint16_t timeout)
{
    ResetMillis();
    while (GetMillis() < timeout)
    {
        if (_mode != RHModeTx) // Any previous transmit finished?
           return true;	
    }
    return false;
}



void setOpMode(uint8_t mode)
{
    uint8_t opmode = spiRead(RH_RF69_REG_01_OPMODE);
    opmode &= ~RH_RF69_OPMODE_MODE;
    opmode |= (mode & RH_RF69_OPMODE_MODE);
    spiWrite(RH_RF69_REG_01_OPMODE, opmode);

    // Wait for mode to change.
    while (!(spiRead(RH_RF69_REG_27_IRQFLAGS1) & RH_RF69_IRQFLAGS1_MODEREADY))
	;
}


BYTE printRegister(uint8_t reg)
{
    uint8_t RegVal;
#ifdef RH_HAVE_SERIAL
    RegVal = spiRead(reg);
    printf("\rREG #%02X: %02X ", reg, RegVal);        
#endif
    return true;
}


BYTE printRegisters()
{
    uint8_t i;
    //for (i = 0; i < 0x50; i++)
    for (i = 0; i < 16; i++)
	printRegister(i);
    // Non-contiguous registers
    printRegister(RH_RF69_REG_58_TESTLNA);
    printRegister(RH_RF69_REG_6F_TESTDAGC);
    printRegister(RH_RF69_REG_71_TESTAFC);

    return true;
}

// Low level function reads the FIFO and checks the address
// Caution: since we put our headers in what the RH_RF69 considers to be the payload, if encryption is enabled
// we have to suffer the cost of decryption before we can determine whether the address is acceptable.
// Performance issue?
void RH_RF69readFifo()
{
uint8_t payloadlen;

    RFM69_CS_LOW();
    SPItransfer(RH_RF69_REG_00_FIFO); // Send the start address with the write mask off
    payloadlen = SPItransfer(0); // First byte is payload len (counting the headers)
    if (payloadlen <= RH_RF69_MAX_ENCRYPTABLE_PAYLOAD_LEN && payloadlen >= RH_RF69_HEADER_LEN)
    {
    	_rxHeaderTo = SPItransfer(0);
        // Check addressing
        if (_promiscuous || _rxHeaderTo == _thisAddress || _rxHeaderTo == RH_BROADCAST_ADDRESS || _rxHeaderTo == ANY_DEVICE)
        {
            // Get the rest of the headers
            _rxHeaderFrom  = SPItransfer(0);
            _rxHeaderId    = SPItransfer(0);
            _rxHeaderFlags = SPItransfer(0);
            // And now the real payload
            MessageLength = (uint16_t)payloadlen - RH_RF69_HEADER_LEN;
            for (_bufLen = 0; _bufLen < MessageLength; _bufLen++)
                _buf[_bufLen] = SPItransfer(0);
#ifdef TX_MASTER
            printf("\rACK Received");
#endif            
            _rxGood++;
            _rxBufValid = true;
        }
    }
    RFM69_CS_HIGH();
    // Any junk remaining in the FIFO will be cleared next time we go to receive mode.
}

uint8_t headerFlags()
{
    return _rxHeaderFlags;
}

uint8_t headerId()
{
    return _rxHeaderId;
}

uint8_t headerTo()
{
    return _rxHeaderTo;
}

uint8_t headerFrom()
{
    return _rxHeaderFrom;
}

        
BYTE setFrequency(float centre)
{
    // Frf = FRF / FSTEP
    uint32_t frf = (uint32_t)((centre * 1000000.0) / RH_RF69_FSTEP);
    spiWrite(RH_RF69_REG_07_FRFMSB, (frf >> 16) & 0xff);
    spiWrite(RH_RF69_REG_08_FRFMID, (frf >> 8) & 0xff);
    spiWrite(RH_RF69_REG_09_FRFLSB, frf & 0xff);
    
    return true;
}

BYTE rssiRead()
{
    // Force a new value to be measured
    // Hmmm, this hangs forever!
#if 0
    spiWrite(RH_RF69_REG_23_RSSICONFIG, RH_RF69_RSSICONFIG_RSSISTART);
    while (!(spiRead(RH_RF69_REG_23_RSSICONFIG) & RH_RF69_RSSICONFIG_RSSIDONE))
	;
#endif
    return -((int8_t)(spiRead(RH_RF69_REG_24_RSSIVALUE) >> 1));
}


BYTE sleep()
{
    if (_mode != RHModeSleep)
    {
	spiWrite(RH_RF69_REG_01_OPMODE, RH_RF69_OPMODE_MODE_SLEEP);
	_mode = RHModeSleep;
    }
    return true;
}

void setHeaderId(uint8_t id)
{
    _txHeaderId = id;
}

BYTE sendto(uint8_t* buf, uint8_t len, uint8_t address)
{
#ifdef TEST_TX    
    TEST_OUT_Set();
#endif    
    setHeaderTo(address);
    return (send(buf, len));
}

void setHeaderTo(uint8_t to)
{
    _txHeaderTo = to;
}

uint16_t getRxMessage (uint8_t *ptrMessage, uint16_t *NumReceived)
{
uint16_t i = 0;

    *NumReceived = MessageLength;
    for (i = 0; i < MessageLength; i++)
        ptrMessage[i] = _buf[i];
    _rxBufValid = false; // Got the most recent message
    return (uint16_t) MessageLength;
}

BYTE recvfrom(uint8_t* buf, uint16_t* len, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags)
{
    if (recv(buf, len))
    {        
        if (from)  *from =  headerFrom();
        if (to)    *to =    headerTo();
        if (id)    *id =    headerId();
        if (flags) *flags = headerFlags();
        return true;
    }
    return false;
}

void setHeaderFlags(uint8_t set, uint8_t clear)
{
    _txHeaderFlags &= ~clear;
    _txHeaderFlags |= set;
}

void acknowledge(uint8_t id, uint8_t from)
{
    setHeaderId(id);
    setHeaderFlags(RH_FLAGS_ACK, RH_FLAGS_APPLICATION_SPECIFIC);
    // We would prefer to send a zero length ACK,
    // but if an RH_RF22 receives a 0 length message with a CRC error, it will never receive
    // a 0 length message again, until its reset, which makes everything hang :-(
    // So we send an ACK of 1 octet
    // REVISIT: should we send the RSSI for the information of the sender?
    uint8_t ack = '!';
    sendto(&ack, sizeof(ack), from); 
    while (_mode == RHModeTx); // waitPacketSent();
}


bool sendtoWait(uint8_t* buf, uint8_t len, uint8_t address)
{
    // Assemble the message
    uint8_t thisSequenceNumber = ++_lastSequenceNumber;        
        
    _retransmissions = 0;
    
    // If transmitting to all units, don't wait for ACKs:
    if (address == RH_BROADCAST_ADDRESS)
    {
        int i;
        // Transmit packet 2x with delay in between:
        for (i = 0; i < 2; i++)
        {
            setHeaderId(thisSequenceNumber);
            setHeaderFlags(RH_FLAGS_NONE, RH_FLAGS_ACK); // Reset flags - though probably not necessary
            sendto(buf, len, address);
            while (_mode == RHModeTx); // This is the equivalent of waitPacketSent();
        
            ResetMillis();
            while (GetMillis() < _timeout);
        }
        return true;
    }
    // Otherwise transmit to specified slave then wait for ACK:
    else
    {
        do
        {
            setHeaderId(thisSequenceNumber);
            setHeaderFlags(RH_FLAGS_NONE, RH_FLAGS_ACK); // Clear the ACK flag
            sendto(buf, len, address);
            while (_mode == RHModeTx); // This is the equivalent of waitPacketSent();
            
            ResetMillis();
            do {
                uint8_t from, to, id, flags;
                if (recvfrom(0, 0, &from, &to, &id, &flags)) return true; // ACK received, all done
            } while (GetMillis() < _timeout);        
            _retransmissions++;                // No ACK received, so try again
        } while (_retransmissions <= _retries);
        _retransmissions--;
    }
    // Retries exhausted
    return false;
} 

bool recvfromAck(uint8_t* buf, uint16_t* len, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags)
{  
    uint8_t _from;
    uint8_t _to;
    uint8_t _id;
    uint8_t _flags;
    // Get the message before its clobbered by the ACK (shared rx and tx buffer in some drivers
    if (available() && recvfrom(buf, len, &_from, &_to, &_id, &_flags))
    {         
        //if (*len > 59) buf[59] = '\0'; $$$$
        //else buf[*len] = '\0';    
        // printf("\rRECEIVED");
    	// Never ACK an ACK
    	if (!(_flags & RH_FLAGS_ACK))
        {
    	    // Its a normal message not an ACK        
    	    if (_to ==_thisAddress || _to == ANY_DEVICE)
            {
                // Its for this node and
                // Its not a broadcast, so ACK it
                // Acknowledge message with ACK set in flags and ID set to received ID
                acknowledge(_id, _from);
                // printf("\r\rACK FROM: %d, TO: %d, LENGTH: %d, \r%s", (int)_from, (int)_to, (int)*len, buf);
            }
            // If we have not seen this message before, then we are interested in it
            if (_id != _seenIds[_from])
            {
                if (from)  *from =  _from;
                if (to)    *to =    _to;
                if (id)    *id =    _id;
                if (flags) *flags = _flags;
                _seenIds[_from] = _id;
                return true;
            }
            // Else just re-ack it and wait for a new one
        }
    }
    // No message for us available
    return false;
}



void setHeaderFrom(uint8_t from)
{
    _txHeaderFrom = from;
}

void setThisAddress(uint8_t thisAddress)
{    
    // Use this address in the transmitted FROM header
    setHeaderFrom(thisAddress);
    _thisAddress = thisAddress;
#ifndef TX_MASTER   
    if (_thisAddress < MIN_RX_ADDRESS) _thisAddress = MIN_RX_ADDRESS;
#endif    
}

uint32_t GetRetramissions()
{
    return _retransmissions;
}

