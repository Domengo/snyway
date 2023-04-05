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

#ifndef _HDLCEXT_H
#define _HDLCEXT_H

#include "hdlcstru.h"

void  HDLC_Init(HdlcStruct *);
UBYTE HDLC_DataFound(HdlcStruct *pHDLC_RX, UBYTE *pBuff, UWORD *Len);
void  HDLC_IdentifyFrame(HdlcStruct *, UBYTE);
void  HDLC_CleanRD(HdlcStruct *);
void  HDLC_FillWB(HdlcStruct *pHDLC_TX, UBYTE *inBuf, UWORD inLen);
void  HDLC_ResetWB(HdlcStruct *);
void  HDLC_AppendFlags(HdlcStruct *, UBYTE);
UBYTE HDLC_Decode(HdlcStruct *);
void  HDLC_Encode(HdlcStruct *pHDLC_TX);
void  HDLC_UpdateCRC16(UWORD *,    UBYTE *, UWORD);
UBYTE HDLC_CheckCRC(HdlcStruct *);
void  HDLC_AppendCRC(HdlcStruct *);

UWORD HDLC_GetT30Type(UBYTE *p);
UWORD HDLC_CorrT30FCF(UBYTE isDisRcved, UWORD type);
#endif
