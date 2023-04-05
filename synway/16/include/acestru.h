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

#ifndef _ACESTRU_H
#define _ACESTRU_H

#include "faxapi.h"
#include "acedef.h"
#include "dspdstru.h"
#include "hdlcstru.h"
#include "circbuff.h"

typedef UBYTE(*AceFnptr)(UBYTE **);

typedef struct
{
    T30ToDceInterface           *pT30ToDce;
    DceToT30Interface           *pDceToT30;
    UBYTE                       currentT30DceCmd;
    UBYTE                       classWaitTime;
    UBYTE                       classState;
    HdlcStruct                  T30HDLC_TX;
    HdlcStruct                  T30HDLC_RX;
    UBYTE                       hdlcBuf[256 + 16];
    UWORD                       hdlcLen;
    UBYTE                       isToAvtEcho;
    UBYTE                       isToRcvOnes;
    UBYTE                       isToTurnOffCarrier;
    UBYTE                       isRcvingHdlc;
    UBYTE                       isXmtingHdlc;
    UBYTE                       isOptFrm;
    UWORD                       timerT0;
    UWORD                       timerSilence;
    UBYTE                       syncCount;
    UWORD                       hdlcTypeRx;
    UWORD                       prevBufLen;
    UBYTE                       LineState;
    UBYTE                       CPState; /* call progress state */
    UWORD                       StartTime;
    UBYTE                       FaxClassType;
    UBYTE                       RingCount;
    UBYTE                       bytesPer10ms;
    UBYTE                       wasCarrLoss;
    UBYTE                       Comma;
    UBYTE                       BusyDet;
#if SUPPORT_V34FAX
    UBYTE                       V34Fax_State;
    UBYTE                       isDleEotRcved;
    UBYTE                       V8Detect_State;/* V8_PASS_NO (0): V8 no pass                 */
    /* V81PASS_V21(1): V8 pass with V21 for V17   */
    /* V81PASS_V8 (2): V8 pass with V8  for V34fax */
#endif
#if SUPPORT_PARSER
    CircBuffer                  *pCBIn;
    CircBuffer                  *pCBOut;
    CHAR                        CurrCommand[AT_SIZE];
    CHAR                        LastCommand[AT_SIZE];
    AceFnptr                    AtActions[17];
#if SUPPORT_FAX
    UBYTE                       wasDle;
    UBYTE                       WasETX;
#endif
#if SUPPORT_T31_PARSER
    AceFnptr                    Class1Actions[11];
    UBYTE                       ClassFDD;
#if SUPPORT_V34FAX
    UBYTE                       selectedV34Chnl;
#endif
#endif
#if SUPPORT_T32_PARSER
    DteToT30Interface           *pDteToT30;
    T30ToDteInterface           *pT30ToDte;
    UBYTE                       prevDataCmd;
#else
    UBYTE                       isV34Enabled;
#endif
    UBYTE                       Echo;
    UBYTE                       Semicolon;
    UBYTE                       Quiet;
    UBYTE                       Verbose;
    UBYTE                       DialToneDet;
    UBYTE                       ubStringFlag;/* EPR331, keep ' ' for strings. */
    UBYTE                       ATidx;
    UBYTE                       Mandate;
    UBYTE                       ubFlowControlType;
    UBYTE                       ubSerPortRate;
    UBYTE                       CarrierDetect_Flag;
#if SUPPORT_MODEM
    UBYTE                       CountPlus;
    UBYTE                       GuardTime;
    UBYTE                       ubModemType;
    UBYTE                       ubControlFlag; /* set result code for ATO1 */
#endif
#endif
} ACEStruct;

#endif
