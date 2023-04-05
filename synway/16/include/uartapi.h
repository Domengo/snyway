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

#ifndef _UARTAPI_H
#define _UARTAPI_H

#include "commdef.h"
#include "gaoapi.h"

#define XON                  (0x11)
#define XOFF                 (0x13)

#define UART_SIZE_THRESHOLD  (128)  /* >= 16 */
#define UART_SIZE            (256) /* >= 2*UART_SIZE_THRESHOLD */
#define UART_MASK            (UART_SIZE - 1)

extern UBYTE UartRxBuffer[];
extern UBYTE UartTxBuffer[];
extern SDWORD UartRxIn, UartRxOut;
extern SDWORD UartTxIn, UartTxOut;

void UartSendXON(void);
void UartSendXOFF(void);
void UartInit(void);
void UartChgState(GAO_ModStatus *pStatus);
void SendCharToUart(UBYTE *pubBuf, UWORD uLen);
void SendCharToUartRx(CHAR *pubBuf, UWORD uLen);
UWORD CheckUartTxBuf();
UBYTE GetOneCharFrUartTx(void);
UWORD GetCharFrUart(UBYTE *pubBuf, UWORD uLen);

#endif
