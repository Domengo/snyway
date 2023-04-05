/*****************************************************************************
*             Copyright (c) 1995 - 20xx by GAO Research Inc.                 *
*                          All rights reserved.                              *
*                                                                            *
*  This program is a confidential and an unpublished work of GAO Research    *
*  Inc. that is protected under international and Canadian copyright laws.   *
*  This program is a trade secret and constitutes valuable property of GAO   *
*  Research Inc. All use, reproduction and/or disclosure in whole or in      *
*  part of this program is strictly prohibited unless expressly authorized   *
*  in writing by GAO Research Inc.                                           *
*****************************************************************************/

/**********************************************************************/
/*                     S E R C O N  . C                               */
/**------------------------------------------------------------------**/
/* Task        :      Control serial port signals                     */
/**------------------------------------------------------------------**/
/* Author         :   Yalan Xu                                        */
/* Developed on   :   08/28/1997                                      */
/**------------------------------------------------------------------**/
/* COMPILER OPT.  : Disable stack check!                              */
/*                  Disable vector check!                             */
/*                  Disable ALL optimizations !                       */
/**********************************************************************/
#include <conio.h>
#include "serutil.h"
#include "types.h"

extern SWORD iSerPort;
void ser_Setrate(int iSerPort, long lBaudRate);

/* send "stop sending" signal to DTE */
void SerialSetCTS(void)
{
    outp(iSerPort + SER_MODEM_CONTROL, inp(iSerPort + SER_MODEM_CONTROL) & ~SER_MCR_RTS);
}

/* send "start sending" signal to DTE */
void SerialClearCTS(void)
{
    outp(iSerPort + SER_MODEM_CONTROL, inp(iSerPort + SER_MODEM_CONTROL) | SER_MCR_RTS);
}

/* send "Carrier Detect" signal to DTE */
void SerialSetCD(void)
{
    outp(iSerPort + SER_MODEM_CONTROL, inp(iSerPort + SER_MODEM_CONTROL) | SER_MCR_DTR);
}

/* clear "Carrier Detect" signal to DTE */
void SerialClearCD(void)
{
    outp(iSerPort + SER_MODEM_CONTROL, inp(iSerPort + SER_MODEM_CONTROL) & ~SER_MCR_DTR);
}

UBYTE SerialCheckRTS(void)
{
    return(inp(iSerPort + SER_MODEM_STATUS) & SER_MSR_CTS);
}

void SerialSetRate(UDWORD rate)
{
    ser_Setrate(iSerPort, rate);
}
