/*******************************************************************************
  TMR Peripheral Library Interface Source File

  Company
    Microchip Technology Inc.

  File Name
    plib_tmr3.c

  Summary
    TMR3 peripheral library source file.

  Description
    This file implements the interface to the TMR peripheral library.  This
    library provides access to and control of the associated peripheral
    instance.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "device.h"
#include "plib_tmr3.h"
#include "definitions.h"                // SYS function prototypes
#include "Defs.h"

static TMR_TIMER_OBJECT tmr3Obj;
static BYTE TM3Rflag = true;
unsigned long ElapsedMilliSeconds = 0;

//unsigned long millis()
//{
//    return ElapsedMilliSeconds;
//}

unsigned long GetMillis()
{
    return ElapsedMilliSeconds;
}

void ResetMillis()
{
    ElapsedMilliSeconds = 0;
}

void TMR3_Initialize(void)
{
    /* Disable Timer */
    T3CONCLR = _T3CON_ON_MASK;

    /*
    SIDL = 0
    TCKPS =0
    T32   = 0
    TCS = 0
    */
    T3CONSET = 0x0;

    /* Clear counter */
    TMR3 = 0x0;

    /*Set period */
    PR3 = 4799U;

    /* Enable TMR Interrupt */
    IEC0SET = _IEC0_T3IE_MASK;

}



void TMR3_Start(void)
{
    T3CONSET = _T3CON_ON_MASK;
}

unsigned char getT3Flag()
{
    return (TM3Rflag);
}

void setT3Flag()
{
    TM3Rflag = true;
}

void ClearT3Flag()
{
    TM3Rflag = false;
}



void TMR3_Stop (void)
{
    T3CONCLR = _T3CON_ON_MASK;
}

void TMR3_PeriodSet(uint16_t period)
{
    PR3  = period;
}

uint16_t TMR3_PeriodGet(void)
{
    return (uint16_t)PR3;
}

uint16_t TMR3_CounterGet(void)
{
    return (uint16_t)(TMR3);
}


uint32_t TMR3_FrequencyGet(void)
{
    return (48000000);
}

void TIMER_3_InterruptHandler (void)
{
    static int InterruptCounter = 0;
    
    uint32_t status  = 0U;
    status = IFS0bits.T3IF;
    IFS0CLR = _IFS0_T3IF_MASK;
    
    InterruptCounter++;
    if (InterruptCounter >= 10)
    {
        InterruptCounter = 0;
        setT3Flag();       
        ElapsedMilliSeconds++;
    }
    
    // TEST_OUT_Toggle(); // $$$$

    if((tmr3Obj.callback_fn != NULL))
    {
        tmr3Obj.callback_fn(status, tmr3Obj.context);
    }
}


void TMR3_InterruptEnable(void)
{
    IEC0SET = _IEC0_T3IE_MASK;
}


void TMR3_InterruptDisable(void)
{
    IEC0CLR = _IEC0_T3IE_MASK;
}


void TMR3_CallbackRegister( TMR_CALLBACK callback_fn, uintptr_t context )
{
    /* Save callback_fn and context in local memory */
    tmr3Obj.callback_fn = callback_fn;
    tmr3Obj.context = context;
}
