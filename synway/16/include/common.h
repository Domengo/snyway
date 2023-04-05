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

#ifndef _COMMON_H
#define _COMMON_H

#include "commtype.h"
#include "commmac.h"
#include "circbuff.h"

#define T50_CR                      (0x0d)
#define T50_LF                      (0x0a)
#define T50_BS                      (0x08)
#define T50_ETX                     (0x03)
#define T50_EOT                     (0x04)
#define T50_DLE                     (0x10)
#define T50_DC1                     (0x11)
#define T50_DC2                     (0x12)
#define T50_DC3                     (0x13)
#define T50_CAN                     (0x18)
#define T50_SUB                     (0x1a)
#define T50_A                       (0x41)
#define T50_W                       (0x57)

#define T50_DLE_BIT_REV             (0x08)
#define T50_DC1_BIT_REV             (0x88)
#define T50_DC2_BIT_REV             (0x48)
#define T50_DC3_BIT_REV             (0xc8)
#define T50_CAN_BIT_REV             (0x18)
#define T50_SUB_BIT_REV             (0x58)

extern CONST CHAR *ResponseCode[];
extern CONST CHAR DTE_DCE_INIT_PARM[];

UBYTE COMM_BitReversal(UBYTE byte);
UBYTE COMM_ToUpperCase(UBYTE com);
UBYTE COMM_IsNumber(UBYTE com);
UBYTE COMM_IsHexNumber(UBYTE com, UBYTE *pOutVal);
UBYTE COMM_AsciiToUInt32(UBYTE *pInBuf, UBYTE *pIndex, UDWORD *pOutVal, UBYTE *pIsEnd, UBYTE isHex, UDWORD defVal);
UBYTE COMM_AsciiToUByte(UBYTE *pInBuf, UBYTE *pIndex, UBYTE *pOutVal, UBYTE *pIsEnd, UBYTE isHex, UBYTE defVal);
UBYTE COMM_StringToOctets(UBYTE *pInBuf, UBYTE *pIndex, UBYTE *pOutBuf);
UBYTE COMM_DecToAscii(UWORD num, UBYTE *pOutBuf);
void COMM_HexToAscii(CircBuffer *pCB, UBYTE num);
UBYTE COMM_ParseString(UBYTE *pInBuf, UBYTE *pIndex, UBYTE *pOutBuf);
UBYTE COMM_GetRspOrInd(CircBuffer *DteRd, UBYTE *pOutBuf, UBYTE maxLen);
UBYTE COMM_ParseResultCode(UBYTE *pInBuf, UBYTE len);
#endif