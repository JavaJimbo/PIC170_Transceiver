/* 
 * "Defs.h"
 * Include file for PIC795_RFM69_Controller project
 * Author: Jim Sedgwick
 *
 * Created on March 23, 2022
 */
// #include <plib.h>
#include "definitions.h"                // SYS function prototypes
 
#define TX_MASTER
#define	STX '>'
#define	DLE '/'
#define	ETX '\r'

#ifndef DEFS_H
#define	DEFS_H

#define MAXBUFFER 1024
#define BYTE uint8_t

#define SYS_FREQ 80000000
#define GetPeripheralClock() SYS_FREQ

#define RF69_FREQ 915.0

#define TRUE true
#define FALSE false


#define RF69_CHANNEL SPI_CHANNEL2

#ifndef _MAIN
    extern unsigned long milliseconds;    
#endif
    
#define RF69_FREQ 915.0    
    
#define MIN_RX_ADDRESS 2



#define ANY_DEVICE 0   
    
    
#define RFM69_CS_HIGH() RFM69_CS_Set()
#define RESET_HIGH() RFM69_RESET_Set()
#define RFM69_CS_LOW() RFM69_CS_Clear()
#define RESET_LOW() RFM69_RESET_Clear()
    
    
#endif	/* DEFS_H */

