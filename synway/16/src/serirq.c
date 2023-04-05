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
/*                     S E R I R Q  . C                               */
/**------------------------------------------------------------------**/
/* Task        : Uses chat program to demonstrate                     */
/*                  serial port IRQ programming                       */
/**------------------------------------------------------------------**/
/* Author          : Michael Tischer / Bruno Jennrich                 */
/* Developed on  : 04/08/1994                                         */
/* Last update : 04/07/1995    06/16/97 by Yalan                      */
/**------------------------------------------------------------------**/
/* COMPILER OPT.  : Disable stack check!                              */
/*                  Disable vector check!                             */
/*                  Disable ALL optimizations !                       */
/* COMPILER       : Borland C++ 3.1, Microsoft Visual C++ 1.5         */
/**********************************************************************/

#include <stdio.h>
#include <dos.h>
#include <ctype.h>
#include <process.h>
#include <conio.h>
#include <string.h>

#include "types.h"
#include "serutil.h"
#include "irqutil.h"
#include "uartapi.h"

#if DUMPSIMDATA
extern UBYTE DumpOrSimuType;

extern FILE *PCMinWrFile;
extern FILE *DTEinWrFile;
#endif

extern int iSerPort;
extern int iSerIRQ;
extern int iSerBaud;
extern int iSerParity;
extern int iSerData;
extern int iSerStop;

long lBaud;

void (_interrupt _FP *lpOldIRQ)();
int iUART;

extern UBYTE TxIntEnd;

/**********************************************************************/
/* GetSer : Interrupt routine, initiated by serial port               */
/**------------------------------------------------------------------**/
/* Info : This function accepts data from the serial                  */
/*        port and displays it in the "remote" window.                */
/*        This function also contains requirements for                */
/*        all other port events that initiate                         */
/*  an IRQ.                                                           */
/**********************************************************************/

#define DEBUG_FLAG   (0)

#if DEBUG_FLAG
UBYTE Uart_Flag = 0;
#endif

void __interrupt _FP GetSer(void)
{
    SDWORD Counter;
    SDWORD Size;
    UBYTE bIRQID;
    UBYTE lineStatus;

    _disable();

    bIRQID = (UBYTE)inp(iSerPort + SER_IRQ_ID);

    if (!(bIRQID & SER_ID_PENDING))
    {
        do
        {
            bIRQID &= SER_ID_MASK;

            if (bIRQID == SER_ID_RECEIVED)
            {
                Counter = (UartRxOut - UartRxIn - 1) & UART_MASK;

                if (Counter < UART_SIZE_THRESHOLD)
                {
                    UartSendXOFF();
                }

                while (Counter)
                {
                    lineStatus = inp(iSerPort + SER_LINE_STATUS);

                    if (!(lineStatus & SER_LSR_DATARECEIVED))
                    {
                        break;
                    }

                    UartRxBuffer[UartRxIn] = (UBYTE)inp(iSerPort + SER_RXBUFFER);

#if DEBUG_FLAG

                    if (UartRxBuffer[UartRxIn] == 35 && Uart_Flag == 0)
                    {
                        Uart_Flag = 1;
                    }
                    else if (UartRxBuffer[UartRxIn] == 37 && Uart_Flag == 1)
                    {
                        Uart_Flag = 2;
                    }
                    else if (UartRxBuffer[UartRxIn] == 38 && Uart_Flag == 2)
                    {
                        Uart_Flag = 3;
                    }
                    else if (UartRxBuffer[UartRxIn] == 42 && Uart_Flag == 3)
                    {
                        Uart_Flag = 4;
                    }
                    else if (UartRxBuffer[UartRxIn] == 42 && Uart_Flag == 4)
                    {
                        Uart_Flag = 5;
                    }
                    else if (UartRxBuffer[UartRxIn] == 38 && Uart_Flag == 5)
                    {
                        Uart_Flag = 6;
                    }
                    else if (UartRxBuffer[UartRxIn] == 37 && Uart_Flag == 6)
                    {
                        Uart_Flag = 7;
                    }
                    else if (UartRxBuffer[UartRxIn] == 35 && Uart_Flag == 7)
                    {
                        Uart_Flag = 8;
                    }
                    else
                    {
                        Uart_Flag = 0;
                    }

#endif

                    if (Counter > 1)
                    {
                        UartRxIn = (UartRxIn + 1) & UART_MASK;
                    }
                    else
                    {
                        TRACE0("SERIRQ: ERROR. Buffer underrun");
                    }

                    Counter--;
                }

#if DEBUG_FLAG

                if (Uart_Flag == 8)
                {
#if DUMPSIMDATA

                    if (DumpOrSimuType > 2)
                    {
                        fclose(PCMinWrFile);
                        fclose(DTEinWrFile);
                    }

#endif
                    irq_SendEOI(iSerIRQ);                           /* End of interrupt */
                    exit(1);
                }

#endif
            }
            else if (bIRQID == SER_ID_SENT)
            {
                Size = (UartTxIn - UartTxOut) & UART_MASK;

                if (Size > 0)
                {
                    /* Send max 16 chars to FIFO */
                    Counter = 16;

                    if (Size < 16)
                    {
                        Counter = Size;
                    }

                    while (Counter > 0)
                    {
                        outp(iSerPort + SER_TXBUFFER, UartTxBuffer[UartTxOut ++]);
                        UartTxOut &= UART_MASK;
                        Counter--;
                    }
                }
                else if (TxIntEnd == 0)
                {
                    TxIntEnd = 1;
                }
            }

            bIRQID = (UBYTE)inp(iSerPort + SER_IRQ_ID);
        }
        while (!(bIRQID & SER_ID_PENDING));
    }
    else
    {
        lpOldIRQ();
    }

    irq_SendEOI(iSerIRQ);                           /* End of interrupt */
    _enable();
}

/**********************************************************************/
/* M A I N   P R O G R A M                                            */
/**********************************************************************/
UBYTE SerPortInit(void)
{
    UartTxIn  = 0;
    UartTxOut = 0;
    UartRxIn  = 0;
    UartRxOut = 0;

    lBaud = iSerBaud;/*For FAX: 19200; for MODEM: 115200*/

    iUART = ser_Init(iSerPort, lBaud, iSerData | iSerStop | iSerParity);

    if (iUART == NOSER)
    {
        TRACE0("No COM port found!");
        TRACE0("Run program using CONSOLE ONLY!!");
        return 0;
    }

    if (iUART > INS8250)
    {
        /*TRACE0("16550 UART type or higher detected, Set FIFO trigger.");*/
        ser_FIFOLevel(iSerPort, SER_FIFO_TRIGGER8);
    }

    lpOldIRQ = ser_SetIRQHandler(iSerPort,       /* Install serial */
                                 iSerIRQ,       /* interrupt.       */
                                 GetSer,
                                 SER_IER_RECEIVED |
                                 SER_IER_SENT);

    return (1);
}

/* Restore old handler */
void SerRestorOldHandler(void)
{
    ser_RestoreIRQHandler(iSerPort, iSerIRQ, lpOldIRQ);

    if (iUART > INS8250)
    {
        ser_FIFOLevel(iSerPort, 0);
    }
}

void ClrSerInt(void)
{
    (UBYTE)inp(iSerPort + SER_IRQ_ID);
}
