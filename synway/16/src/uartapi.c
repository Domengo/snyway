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

#include <dos.h>
#include "gaoapi.h"
#include "uartapi.h"

#if SUPPORT_REALTIME
#include <conio.h>
#include "serutil.h"

extern int iSerPort;
#endif

/* PC COM UART interface */
void SerialSetCTS(void);
void SerialClearCTS(void);
void SerialSetCD(void);
void SerialClearCD(void);
UBYTE SerialCheckRTS(void);
void SerialSetRate(UDWORD rate);

/* Global UART driver buffers */
UBYTE UartRxBuffer[UART_SIZE];
UBYTE UartTxBuffer[UART_SIZE];
SDWORD UartRxIn = 0, UartRxOut = 0;
SDWORD UartTxIn = 0, UartTxOut = 0;

struct UartStateDef
{
    UBYTE ubFlowType;
    UBYTE ubXONFlag;
    UBYTE  CarrierDetect_Flag;
    UBYTE ubSerPortRate;
    UBYTE ubRetrainRerateFlag;
} UartState;

CONST UDWORD SerRateTab[13] =
{
    300,
    600,
    1200,
    2400,
    4800,
    7200,
    9600,
    12000,
    14400,
    19200,
    38400,
    57600,
    115200
};


void SendCharToUart(UBYTE *pubBuf, UWORD uLen)
{
    UWORD i;

    for (i = 0; i < uLen; i++)
    {
        UartTxBuffer[UartTxIn++] = *pubBuf++;
        UartTxIn &= UART_MASK;
    }
}

void SendCharToUartRx(CHAR *pubBuf, UWORD uLen) /* DTE to UART? */
{
    UWORD i;

    for (i = 0; i < uLen; i++)
    {
        UartRxBuffer[UartRxIn++] = (UBYTE)(*pubBuf++);
        UartRxIn &= UART_MASK;
    }
}

/* From modem to UART, then to DTE */
void UartSendXON(void)
{
    UBYTE ubFIFO = 0;

    if (UartState.ubRetrainRerateFlag == 0)
    {
        if (UartState.ubFlowType == FLOWCONTROL_SW)/* Software flow control */
        {
#if SUPPORT_REALTIME

            while (!ubFIFO)
            {
                ubFIFO = (UBYTE)(inp(iSerPort + SER_LINE_STATUS) & 0x60);
            }

            outp(iSerPort + SER_TXBUFFER, XON);
#else
            ubFIFO = XON;
            SendCharToUart(&ubFIFO, 1);
#endif
        }
        else if (UartState.ubFlowType == FLOWCONTROL_HW)/* Hardware flow control */
        {
            SerialClearCTS();
        }

        UartState.ubXONFlag = 1;

        //TRACE0("UART: XON");
    }
}

/* From modem to UART, then to DTE */
/* When this function is working, interrupt should be disabled. */
void UartSendXOFF(void)
{
    UBYTE ubFIFO = 0;

    if (UartState.ubFlowType == FLOWCONTROL_SW)/* Software flow control */
    {
#if SUPPORT_REALTIME

        while (!ubFIFO)
        {
            ubFIFO = (UBYTE)(inp(iSerPort + SER_LINE_STATUS)  & 0x60);
        }

        outp(iSerPort + SER_TXBUFFER, XOFF);

#else
        ubFIFO = XOFF;
        SendCharToUart(&ubFIFO, 1);
#endif
    }
    else if (UartState.ubFlowType == FLOWCONTROL_HW) /* Hardware flow control */
    {
        SerialSetCTS();
    }

    UartState.ubXONFlag = 0;

    //TRACE0("UART: XOFF");
}

void UartInit(void)
{
    UartState.CarrierDetect_Flag = 0;

    SerialClearCD();

    UartState.ubFlowType = FLOWCONTROL_HW;
    UartState.ubSerPortRate = 0; /* Not set by AT command yet */

    UartState.ubXONFlag = 1;
    UartState.ubRetrainRerateFlag = 0;

    UartSendXON();

    SerialClearCTS();/* CTS always cleared if hardware flow control not used */
}

UWORD GetCharFrUart(UBYTE *pubBuf, UWORD uLen)
{
    UWORD i, uTemp;

    uTemp = (UartRxIn - UartRxOut) & UART_MASK;

    if (uLen > uTemp)
    {
        uLen = uTemp;
    }

    _disable();

    if (uLen > 0)
    {
        for (i = 0; i < uLen; i++)
        {
            *pubBuf++ = UartRxBuffer[UartRxOut ++];

            UartRxOut &= UART_MASK;
        }
    }

    if (UartState.ubXONFlag == 0)/* This line can be deleted? */
    {
        uTemp = (UartRxIn - UartRxOut) & UART_MASK;

        if (uTemp < (UART_SIZE >> 2))
        {
            UartSendXON();
        }
    }

    _enable();

    return (uLen);
}

/* need to disabel interrupt? */
UBYTE GetOneCharFrUartTx(void)
{
    UBYTE ubTemp;

    ubTemp = UartTxBuffer[UartTxOut++];
    UartTxOut &= UART_MASK;

    return (ubTemp);
}

UWORD CheckUartTxBuf()
{
    UWORD uTemp;

    if ((UartState.ubFlowType != FLOWCONTROL_HW) || (SerialCheckRTS()))
    {
        uTemp = (UartTxOut - UartTxIn - 1) & UART_MASK;
    }
    else
    {
        uTemp = 0;
    }

    return (uTemp);
}

void UartChgState(GAO_ModStatus *pStatus)
{
    _disable();

    UartState.ubFlowType = pStatus->ubFlowType;

    if (UartState.ubSerPortRate != pStatus->ubSerPortRate)
    {
        UartState.ubSerPortRate = pStatus->ubSerPortRate;

        if (UartState.ubSerPortRate > 0)
        {
            SerialSetRate(SerRateTab[UartState.ubSerPortRate - 1]);
        }
    }

#if SUPPORT_REALTIME

    if (inp(iSerPort + SER_MODEM_STATUS) & SER_MSR_DDSR)
    {
        if (inp(iSerPort + SER_MODEM_STATUS) & SER_MSR_DSR)
        {
            outp(iSerPort + SER_MODEM_CONTROL, inp(iSerPort + SER_MODEM_CONTROL) | SER_MCR_DTR);
            TRACE0("DSR set");
        }
    }

#endif

    if ((pStatus->CarrierDetect_Flag == 0) && (UartState.CarrierDetect_Flag == 1))
    {
        UartState.CarrierDetect_Flag = 0;

        SerialClearCD();
    }
    else if ((pStatus->CarrierDetect_Flag == 1) && (UartState.CarrierDetect_Flag == 0))
    {
        UartState.CarrierDetect_Flag = 1;

        SerialSetCD();
    }

    if (pStatus->RetrainRerate_flag)
    {
        if (UartState.ubRetrainRerateFlag == 0)
        {
            UartSendXOFF();

            UartState.ubRetrainRerateFlag = 1;
        }
    }
    else
    {
        if (UartState.ubRetrainRerateFlag == 1)
        {
            UartState.ubRetrainRerateFlag = 0;
        }
    }

    _enable();
}
