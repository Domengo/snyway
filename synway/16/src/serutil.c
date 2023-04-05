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
/*                     S E R U T I L . C                              */
/**------------------------------------------------------------------**/
/* Task        : Functions for direct access to                       */
/*                  serial port                                       */
/**------------------------------------------------------------------**/
/* Author         : Michael Tischer / Bruno Jennrich                  */
/* Developed on   : 04/08/1994                                        */
/* Last update    : 04/07/1995                                        */
/**------------------------------------------------------------------**/
/* COMPILER       : Borland C++ 3.1, Microsoft Visual C++  1.5        */
/**********************************************************************/

#include <conio.h>
#include "serutil.h"
#include "irqutil.h"

/**********************************************************************/
/* ser_UARTType : Get type of UART chip                               */
/**------------------------------------------------------------------**/
/* Input : iSerPort     - base port of interface being tested         */
/* Output : 0 (NOSER)    - no UART chip found                         */
/*           1 (INS8250)  - INS8250 or INS8250-B chip                 */
/*           2 (NS16450)  - INS8250A, INS82C50A, NS16450, NS16C450    */
/*           3 (NS16550A) - NS16550A chip                             */
/*           4 (NS16C552) - NS16C552 chip                             */
/**********************************************************************/
int ser_UARTType(int iSerPort)
{
    UBYTE x, i;

    /* RESET FIFO is absolute neccessary!! */
    outp(iSerPort + SER_FIFO, SER_FIFO_RESETRECEIVE | SER_FIFO_RESETTRANSMIT);

    /*- Check base capabilities ------------------------------------ */
    outp(iSerPort + SER_LINE_CONTROL, 0xAA);   /* Divisor latch set */

    if ((x = inp(iSerPort + SER_LINE_CONTROL)) != 0xAA)
    {
        TRACE1("LINE CONTROL != 0xAA, x=%x", x);
        return NOSER;
    }

    outp(iSerPort + SER_DIVISOR_MSB, 0x55);      /* Specify divisor */

    if (inp(iSerPort + SER_DIVISOR_MSB) != 0x55)
    {
        TRACE0("DIVISOR != 0x55");
        return NOSER;
    }

    outp(iSerPort + SER_LINE_CONTROL, 0x55);   /* Clear divisor latch */

    if (inp(iSerPort + SER_LINE_CONTROL) != 0x55)
    {
        TRACE0("LINE CONTROL != 0x55");
        return NOSER;
    }

    for (i = 0; i < 10; i++)
    {
        outp(iSerPort + SER_IRQ_ENABLE, 0x55);

        if ((x = inp(iSerPort + SER_IRQ_ENABLE))  != 0x05)
        {
            TRACE1(" IRQ enable != 0x05, x=%x", x);

            if (i == 9)
            {
                return NOSER;
            }
        }
    }

    outp(iSerPort + SER_FIFO, 0);               /* Clear FIFO and IRQ */
    outp(iSerPort + SER_IRQ_ENABLE, 0);

    if (inp(iSerPort + SER_IRQ_ID) != 1)
    {
        TRACE0("IRQ ID != 1");
        return NOSER;
    }

    outp(iSerPort + SER_MODEM_CONTROL, 0xF5);
    /*  if((i= inp( iSerPort + SER_MODEM_CONTROL )&0x1b) != 0x15 ) return NOSER;*/

    outp(iSerPort + SER_MODEM_CONTROL, SER_MCR_LOOP);        /* Looping */
    inp(iSerPort + SER_MODEM_STATUS);
    /*  if( ( inp( iSerPort + SER_MODEM_STATUS ) & 0xF0 ) != 0 ) return NOSER;*/

    outp(iSerPort + SER_MODEM_CONTROL, 0x1F);

    if ((inp(iSerPort + SER_MODEM_STATUS) & 0xF0) != 0xF0)
    {
        TRACE0("MODEM STATUS != 0xF0");
        return NOSER;
    }

    outp(iSerPort + SER_MODEM_CONTROL, SER_MCR_DTR | SER_MCR_RTS);

    /* Test for 8250 Chipset, which do not have scratch pad register support */
    outp(iSerPort + SER_SCRATCH, 0x55);   /* Scratch register detected? */

    if (inp(iSerPort + SER_SCRATCH) != 0x55)
    {
        return INS8250;
    }

    outp(iSerPort + SER_SCRATCH, 0);

    /* Check for FIFO supported */
    outp(iSerPort + SER_FIFO, 0xCF);                /* FIFO detected ? */

    if ((inp(iSerPort + SER_IRQ_ID) & 0xC0) != 0xC0)
    {
        return NS16450;
    }

    outp(iSerPort + SER_FIFO, 0);
    /* Alternate function register detected? */
    outp(iSerPort + SER_LINE_CONTROL, SER_LCR_SETDIVISOR);
    outp(iSerPort + SER_2FUNCTION, 0x07);

    if (inp(iSerPort + SER_2FUNCTION) != 0x07)
    {
        outp(iSerPort + SER_LINE_CONTROL, 0);
        return NS16550A;
    }

    outp(iSerPort + SER_LINE_CONTROL, 0);                 /* Reset */
    outp(iSerPort + SER_2FUNCTION, 0);

    return NS16C552;
}

/**********************************************************************/
/* ser_Init : Initialize serial port                                  */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface                          */
/*                      being initialized.                            */
/*           lBaud    - baud rate ( from 1 - 115200 )                 */
/*           bParams  - bit mask of remaining parameters              */
/*                      (s. SER_LCR_... bits)                         */
/* Output : TRUE  - port initialized successfully                     */
/*          FALSE - no port found                                     */
/**********************************************************************/
int ser_Init(int iSerPort, long lBaudRate, UBYTE bParams)
{
    UDWORD uDivisor;
    int    UARTType;

    UARTType = ser_UARTType(iSerPort);

    if (UARTType != NOSER)
    {
        /*TRACE1("UART TYPE:%d", UARTType);*/

        /* Calculate baud rate divisor */
        uDivisor = (UDWORD)(SER_MAXBAUD / lBaudRate);
        outp(iSerPort + SER_LINE_CONTROL, /* Enable divisor access */
             inp(SER_LINE_CONTROL) | SER_LCR_SETDIVISOR);
        /* Set baud rate divisor */
        outp(iSerPort + SER_DIVISOR_LSB, LOBYTE(uDivisor));
        outp(iSerPort + SER_DIVISOR_MSB, HIBYTE(uDivisor));
        /* Disable divisor access */
        outp(iSerPort + SER_LINE_CONTROL, inp(SER_LINE_CONTROL) & ~SER_LCR_SETDIVISOR);

        /* Set other parameters only after resetting baud rate latch,      */
        /* because this operation clears all   */
        /* port parameters!                                                 */

        /* Set transmission parameters other than baud rate  */
        outp(iSerPort + SER_LINE_CONTROL, bParams);

        /* FIFO disable by default */
        outp(iSerPort + SER_FIFO, 0);

        /* Read a byte, to reverse possible error */
        inp(iSerPort + SER_TXBUFFER);
        inp(iSerPort + SER_RXBUFFER);
    }

    return UARTType;
}

/***********************************************************/
/*      ser_Setrate(long lBaudRate):                       */
/*              set serial port raud rate to lBaudRate     */
/***********************************************************/
void ser_Setrate(int iSerPort, long lBaudRate)
{
    UDWORD uDivisor;
    UBYTE  bParams;

    /* save current settings */
    bParams = inp(iSerPort + SER_LINE_CONTROL);
    /* Calculate baud rate divisor */
    uDivisor = (UDWORD)(SER_MAXBAUD / lBaudRate);
    outp(iSerPort + SER_LINE_CONTROL,  /* Enable divisor access */
         inp(SER_LINE_CONTROL) | SER_LCR_SETDIVISOR);
    /* Set baud rate divisor */
    outp(iSerPort + SER_DIVISOR_LSB, LOBYTE(uDivisor));
    outp(iSerPort + SER_DIVISOR_MSB, HIBYTE(uDivisor));
    /* Disable divisor access */
    outp(iSerPort + SER_LINE_CONTROL,
         inp(SER_LINE_CONTROL) & ~SER_LCR_SETDIVISOR);

    /* Set other parameters only after resetting baud rate latch,      */
    /* because this operation clears all   */
    /* port parameters!                                                 */
    /* Set transmission parameters other than baud rate  */
    outp(iSerPort + SER_LINE_CONTROL, bParams);

    /* Read a byte, to reverse possible error */
    inp(iSerPort + SER_TXBUFFER);
}

/**********************************************************************/
/* ser_FIFOLevel : Set FIFO buffer size                               */
/**------------------------------------------------------------------**/
/* Input : 0 - FIFO buffer size = 0, disable and reset (1 byte)       */
/*           SER_FIFO_TRIGGER4/8/14 - size = 4, 8 or 14 bytes         */
/**********************************************************************/
void ser_FIFOLevel(int iSerPort, UBYTE bLevel)
{
    if (bLevel)
    {
        outp(iSerPort + SER_FIFO, bLevel | SER_FIFO_ENABLE);
    }
    else
    {
        outp(iSerPort + SER_FIFO, SER_FIFO_RESETRECEIVE | SER_FIFO_RESETTRANSMIT);
    }
}

/**********************************************************************/
/* ser_IsDataAvailable : Is data available to be read?                */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface being checked.           */
/* Output : == 0 : No byte available to be read                       */
/*           != 0 : Byte is available                                 */
/**------------------------------------------------------------------**/
/* Info : A byte is sent bit by bit, and becomes                      */
/*  a complete byte again only when the receiving port                */
/*        has combined the individual bits. This is what is           */
/*        being checked by this function.                             */
/**********************************************************************/
int ser_IsDataAvailable(int iSerPort)
{
    return inp(iSerPort + SER_LINE_STATUS) & SER_LSR_DATARECEIVED;
}

/**********************************************************************/
/* ser_IsWritingPossible : Can port send next byte ?                  */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface being checked.           */
/* Output : == 0 : Byte cannot be sent.                               */
/*           != 0 : Port ready to send.                               */
/**------------------------------------------------------------------**/
/* Info : A serial port should not be used                            */
/*        to send a byte in the following cases:                      */
/*        1. A received byte has not yet been "retrieved"             */
/*           by the port.                                             */
/*        2. An old send request has not yet been completed.          */
/**********************************************************************/
int ser_IsWritingPossible(int iSerPort)
{
    return (inp(iSerPort + SER_LINE_STATUS) & SER_LSR_TSREMPTY);
}

/**********************************************************************/
/* ser_IsModemStatusSet : Check status of input lines                 */
/**------------------------------------------------------------------**/
/* Input : iSerPort    - base port of interface.                      */
/*           bTestStatus - Bit pattern of lines being tested          */
/*                         (CTS, DSR, RI, CD)                         */
/**********************************************************************/
int ser_IsModemStatusSet(int iSerPort, UBYTE bTestStatus)
{
    return ((UBYTE)inp(iSerPort + SER_MODEM_STATUS) & bTestStatus) == bTestStatus;
}

/**********************************************************************/
/* ser_SetModemControl : Set signal lines for communication with      */
/*                       modem etc.                                   */
/**------------------------------------------------------------------**/
/* Input : iSerPort    - base port of interface.                      */
/*           bNewControl - New status of DTR, RTS etc. lines          */
/**********************************************************************/
void ser_SetModemControl(int iSerPort, UBYTE bNewControl)
{
    outp(iSerPort + SER_MODEM_CONTROL, bNewControl);
}

/**********************************************************************/
/* ser_WriteByte : Send a byte                                        */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface through which            */
/*                      a byte is being sent.                         */
/*           bData    - byte to be sent                               */
/*           uTimeOut - number of passes through loop                 */
/*                      after which a timeout error occurs            */
/*                      if the send was unsuccessful (if              */
/*                      iTimeOut = 0 the system waits "forever".)     */
/*           bSigMask - bit mask of signal lines being tested         */
/*                      (RTS, CTS, CD, RI)                            */
/*           bSigVals - signal line status after applying             */
/*                      above mask.                                   */
/* Output : == 0 - byte was sent                                      */
/*           != 0 - error                                             */
/**********************************************************************/
int ser_WriteByte(int iSerPort, UBYTE bData, UDWORD uTimeOut, UBYTE bSigMask, UBYTE bSigVals)
{
    if (uTimeOut)                                   /* Timeout loop */
    {
        while (!ser_IsWritingPossible(iSerPort) && uTimeOut)
        {
            uTimeOut--;
        }

        if (!uTimeOut)
        {
            return SER_ERRTIMEOUT;
        }
    }
    else
        while (!ser_IsWritingPossible(iSerPort));           /* Wait! */

#if 0

    /* Test signal lines */
    if (((UBYTE) inp(iSerPort + SER_MODEM_STATUS) & bSigMask) == bSigVals)
    {
        /* Pass byte being sent to port */
#endif
        outp(iSerPort + SER_TXBUFFER, bData);
#if 0
        /* Return port error */
        return inp(iSerPort + SER_LINE_STATUS) & SER_LSR_ERRORMSK;
    }
    else
    {
        return SER_ERRSIGNALS;
    }

#endif

    return 0;
}

/**********************************************************************/
/* ser_ReadByte : Receive byte                                        */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface through which            */
/*                      a byte is being received.                     */
/*           pData    - address of byte accepting                     */
/*                      received byte.                                */
/*           uTimeOut - number of passes through loop                 */
/*                      after which a timeout error occurs            */
/*                      if the send was unsuccessful. (If             */
/*                      iTimeOut = 0 the system waits "forever".)     */
/*           bSigMask - bit mask of signal lines being tested         */
/*                      (RTS, CTS, CD, RI)                            */
/*           bSigVals - signal line status after applying             */
/*                      above mask.                                   */
/* Output : == 0 - byte was sent                                      */
/*           != 0 - error                                             */
/**********************************************************************/
int ser_ReadByte(int iSerPort, UBYTE *pData, UDWORD uTimeOut, UBYTE bSigMask, UBYTE bSigVals)
{
    if (uTimeOut)                                   /* Timeout loop */
    {
        while (!ser_IsDataAvailable(iSerPort) && uTimeOut)
        {
            uTimeOut--;
        }

        if (!uTimeOut)
        {
            return SER_ERRTIMEOUT;
        }
    }
    else
        while (!ser_IsDataAvailable(iSerPort));                /* Wait! */

    /* Test signal lines */
    if (((UBYTE) inp(iSerPort + SER_MODEM_STATUS) & bSigMask) == bSigVals)
    {
        /* Read byte received by port */
        *pData = (UBYTE)inp(iSerPort + SER_RXBUFFER);

        return inp(iSerPort + SER_LINE_STATUS) & SER_LSR_ERRORMSK;
    }
    else
    {
        return SER_ERRSIGNALS;
    }
}

/**********************************************************************/
/* ser_WritePacket : Send data packet                                 */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface through which            */
/*                      data is being sent.                           */
/*           pData    - address of data being sent                    */
/*           iLen     - >= 0 : Number of bytes being sent.            */
/*                       < 0 : Buffer size = strlen( pData )          */
/*           uTimeOut - number of passes through loop                 */
/*                      after which a timeout error occurs            */
/*                      if the send was unsuccessful. (If             */
/*                      iTimeOut = 0 the system waits "forever".)     */
/*           bSigMask - bit mask of signal lines being tested         */
/*                      (RTS, CTS, CD, RI)                            */
/*           bSigVals - signal line status after applying             */
/*                      above mask.                                   */
/* Output : == 0 - byte was sent                                      */
/*           != 0 - error                                             */
/**********************************************************************/
int ser_WritePacket(int iSerPort, UBYTE *pData, int iLen, UDWORD uTimeOut, UBYTE bSigMask, UBYTE bSigVals)
{
    int i, e;

    if (iLen < 0)
    {
        iLen = 0;

        while (pData[ iLen ])
        {
            iLen++;
        }
    }

    for (i = 0; i < iLen; i++)
    {
        e = ser_WriteByte(iSerPort, pData[ i ], uTimeOut, bSigMask, bSigVals);

        if (e != 0)
        {
            return e;
        }
    }

    return SER_SUCCESS;
}

/**********************************************************************/
/* ser_ReadPacket : Receive data packet                               */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface through which            */
/*                      data is being received.                       */
/*           pData    - address of data being sent                    */
/*           iLen     - Size of receive buffer                        */
/*           uTimeOut - number of passes through loop                 */
/*                      after which a timeout error occurs            */
/*                      if the send was unsuccessful. (If             */
/*                      iTimeOut = 0 the system waits "forever".)     */
/*           bSigMask - bit mask of signal lines being tested         */
/*                      (RTS, CTS, CD, RI)                            */
/*           bSigVals - signal line status after applying             */
/*                      above mask.                                   */
/* Output : == 0 - byte was sent                                      */
/*           != 0 - error                                             */
/**********************************************************************/
int ser_ReadPacket(int iSerPort, UBYTE *pData, int iLen, UDWORD uTimeOut, UBYTE bSigMask, UBYTE bSigVals)
{
    int i, e;

    for (i = 0; i < iLen; i++)
    {
        e = ser_ReadByte(iSerPort, &pData[i], uTimeOut, bSigMask, bSigVals);

        if (e != 0)
        {
            return e;
        }
    }

    return SER_SUCCESS;
}

/**********************************************************************/
/* ser_CLRIRQ : Disable serial interrupt requests to                  */
/*              IRQ controller.                                       */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface that can no longer       */
/*                      issue IRQs to IRQ controller.                 */
/**********************************************************************/
void ser_CLRIRQ(int iSerPort)
{
    outp(iSerPort + SER_MODEM_CONTROL, inp(iSerPort + SER_MODEM_CONTROL) & ~SER_MCR_IRQENABLED);
}

/**********************************************************************/
/* ser_SETIRQ : Enable serial interrupt requests to                   */
/*              IRQ controller.                                       */
/**------------------------------------------------------------------**/
/* Input : iSerPort - base port of interface that must                */
/*                      issue IRQs to IRQ controller.                 */
/**********************************************************************/
void ser_SETIRQ(int iSerPort)
{
    outp(iSerPort + SER_MODEM_CONTROL, inp(iSerPort + SER_MODEM_CONTROL) | SER_MCR_IRQENABLED);
}

/**********************************************************************/
/* ser_SetIRQHandler : Set interrupt handler                          */
/**------------------------------------------------------------------**/
/* Input : iSerPort  - base port of serial interface                  */
/*                       for which interrupt handler                  */
/*                       is being set.                                */
/*           iSerIRQ   - IRQ(!) line reserved for port.               */
/*           lpHandler - address of interrupt handler.                */
/*           bEnablers - conditions initiating                        */
/*                       an IRQ (see SER_IER_... bits)                */
/* Output : Address of old IRQ handler                                */
/**********************************************************************/
void (_interrupt _FP *ser_SetIRQHandler(int iSerPort, int iSerIRQ, void (_interrupt _FP *lpHandler)(),
                                        UBYTE bEnablers))()
{
    /* Set IRQ enablers */
    outp(iSerPort + SER_IRQ_ENABLE, bEnablers);
    ser_SETIRQ(iSerPort);             /* Issue IRQs to IRQ controller */
    /* Set handler (IRQ is "enabled" there) */
    return irq_SetHandler(iSerIRQ, lpHandler);
}

/**********************************************************************/
/* ser_RestoreIRQHandler : Restore old IRQ handler                    */
/**------------------------------------------------------------------**/
/* Input : iSerPort  - base port of serial interface whose            */
/*                       old interrupt handler is being               */
/*                       restored.                                    */
/*           iSerIRQ   - IRQ line reserved for port.                  */
/*           lpHandler - address of old interrupt handler.            */
/**********************************************************************/
void ser_RestoreIRQHandler(int iSerPort, int iSerIRQ, void (_interrupt _FP *lpHandler)())
{
    ser_CLRIRQ(iSerPort);  /* No more IRQs to IRQ controller */
    /* Set handler and clear all "enablers" */
    ser_SetIRQHandler(iSerPort, iSerIRQ, lpHandler, 0);
    irq_Disable(iSerIRQ);       /* Also disable IRQs by the controller */
}

/**********************************************************************/
/* ser_GetBaud : Get current baud rate for port                       */
/**------------------------------------------------------------------**/
/* Input : iSerPort - Base address of port whose                      */
/*                      baud rate is being determined.                */
/* Output : baud rate                                                 */
/**********************************************************************/
long ser_GetBaud(int iSerPort) /* no used */
{
    UDWORD uDivisor;
    UBYTE  bSettings;

    _disable();
    bSettings = (UBYTE) inp(iSerPort + SER_LINE_CONTROL);
    outp(iSerPort + SER_LINE_CONTROL, bSettings | SER_LCR_SETDIVISOR);
    /* Read baud rate divisor */
    uDivisor = MAKEWORD(inp(iSerPort + SER_DIVISOR_MSB), inp(iSerPort + SER_DIVISOR_LSB));

    outp(iSerPort + SER_LINE_CONTROL, bSettings);
    _enable();

    if (uDivisor)
    {
        return SER_MAXBAUD / uDivisor;
    }

    return 0L;
}
