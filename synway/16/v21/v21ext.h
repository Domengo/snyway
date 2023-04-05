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

#ifndef _V21EXT_H
#define _V21EXT_H

#include "dspdext.h"
#include "v21stru.h"

void  V21_DPCS_Init(UBYTE **pTable);

void  V21_Struc_Init(V21Struct *pV21);
void  V21_Init_main(V21Struct *);
void  V21_Ch2_Init_main(V21Struct *pV21);

void  V21_DPCS_isr(UBYTE **pTable);

void  V21_Tx(V21Struct *, SWORD);
void  V21_Ch1Tx(V21Struct *);
void  V21_Ch2Tx(V21Struct *);

void  V21_Rx(V21Struct *, UBYTE);
void  V21_Ch1Rx(V21Struct *);
void  V21_Ch2Rx(V21Struct *);

void  V21_Ch2Td(V21Struct *);

void  V21_ModemCh1(V21Struct *pV21);
void  V21_ModemCh2(V21Struct *pV21);

void  V21_Ch1Tone_Detect(V21Struct *pV21);
void  V21_Ch1_Detect_tone(V21Struct *pV21);
void  V21_Ch2Modem_Detect_tone(V21Struct *pV21);

void  V21_Ch1MarkTone(V21Struct *pV21);
void  V21_Ch2MarkTone(V21Struct *pV21);

#if SUPPORT_ENERGY_REPORT
void  V21Detect_7E(V21Struct *pV21);
#endif

#if SUPPORT_V21CH2FAX
void   V21_Ch2Fax_Detect_tone(V21Struct *pV21);
void   V21_Ch2_DPCS_isr(UBYTE **pTable);
void   V21_Ch2Rd(V21Struct *);
#endif

#if V21_7200
extern CONST QWORD V21_tCallTone_FilterCoef_72[];
extern CONST QWORD V21_tAnsTone_FilterCoef_72[];
extern CONST QWORD V21_tHFilter_Coef_72[];
extern CONST QWORD V21_tLFilter_Coef_72[];
extern CONST QWORD V21_tLpcoef_72[];
extern CONST QWORD V21_tBSFilter_Coef_72[];
#endif

#if V21_9600
extern CONST QWORD V21_tCallTone_FilterCoef_96[];
extern CONST QWORD V21_tAnsTone_FilterCoef_96[];
extern CONST QWORD V21_tHFilter_Coef_96[];
extern CONST QWORD V21_tLFilter_Coef_96[];
extern CONST QWORD V21_tLpcoef_96[];
extern CONST QWORD V21_tBSFilter_Coef_96[];
#endif

#endif
