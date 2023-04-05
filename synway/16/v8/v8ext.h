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

#ifndef _V8EXT_H
#define _V8EXT_H

#include "v8stru.h"
#include "modifdef.h"

void V8_Init(UBYTE **pTable);
void V8_V21_Init(V21Struct *pV21, UBYTE Modemmode);
void V8_Handler(UBYTE **pTable);
void V8_Pre_JM(V8Struct *pV8);
void V8_Analy(V8Struct *pV8);
void V8_Detect_CM(V8Struct *pV8);
void V8_Detect_CJ(V8Struct *pV8);
void V8_Send_ANSam(V8Struct *pV8);
void V8_Send_JM(V8Struct *pV8);
void V8_Detect_ANSam(V8Struct *pV8);
void V8_Dummy(V8Struct *pV8);
void V8_Detect_JM(V8Struct *pV8);
void V8_Send_CallTone(V8Struct *pV8);
void V8_Send_Silence(V8Struct *pV8);
void V8_ANS_Send_Silence(V8Struct *pV8);
void V8_Send_Te(V8Struct *pV8);
void V8_Send_CM(V8Struct *pV8);
void V8_Send_CJ(V8Struct *pV8);
void V8_Disconnect(UBYTE **pTable);
void V8_V21Transmit(V8Struct *, UBYTE);
void Phase2_DPCS_Init(UBYTE **pTable);
void V21_DPCS_Init(UBYTE **pTable);
void V22_DPCS_Init(UBYTE **pTable);
void V32_DPCS_Init(UBYTE **pTable);
void V8_DPCS_Phase1(UBYTE **pTable);
void V8_SetResponse(UBYTE **pTable, UBYTE ubResponseID, UBYTE *pResponseData, UBYTE ubLen);
#if SUPPORT_CI
void V8_Send_CI(V8Struct *pV8);
#endif

#if SUPPORT_CT
void V8_DCE_Reset(V8Struct *pV8, UBYTE CTFlag);
#else
void V8_DCE_Reset(V8Struct *pV8);
#endif

UBYTE V8_ANSam15_Detect(V8Struct *pV8);
SBYTE V8_V21Receive(V8Struct *pV8);
UBYTE V8_Detect_Sub(V8Struct *pV8, UBYTE which_seq);
UBYTE V8_Recv_Seq(V8Struct *pV8, UBYTE which_seq, SWORD *pOctet_string);

extern CONST UBYTE V8_CM_SETUP[V8_CM_LENGTH2];
extern CONST UBYTE V8_CJ_OCTETS[V8_CJ_LENGTH];
extern CONST QWORD V8_AnsamTone_FIR_Coef[V8_ANSAM_FILTER_LENGTH];

#if SUPPORT_V92D
void V8_Detect_TONEq(V8Struct *pV8);

void V8_Pre_QCA1d(V8Struct *pV8);
void V8_Send_QCA1d(V8Struct *pV8);
void V8_Send_V92_Silence(V8Struct *pV8);
void V8_Send_QTS(V8Struct *pV8);
void V8_Send_QTS_bar(V8Struct *pV8);
void V8_Send_ANSpcm(V8Struct *pV8);

extern CONST SWORD ANSpcm_Table[301];
#endif /* SUPPORT_V92D */

#if SUPPORT_V92A
void V8_Send_QC1a(V8Struct *pV8);
void V8_Send_None(V8Struct *pV8);
void V8_Send_TONEq(V8Struct *pV8);
void V8_Send_TONEq_End(V8Struct *pV8);
void V8_Send_Dummy(V8Struct *pV8);
void V8_Send_QC2a(V8Struct *pV8);

void V8_Detect_QCA1d(V8Struct *pV8);
void V8_Detect_None(V8Struct *pV8);
void V8_Detect_QTS(V8Struct *pV8);
void V8_Detect_QTS_bar(V8Struct *pV8);
void V8_Detect_ANSpcm(V8Struct *pV8);
void V8_Detect_End_ANSPcm(V8Struct *pV8);
void V8_Detect_CRe(V8Struct *pV8);
void V8_Detect_QCA2d(V8Struct *pV8);

UBYTE V8_QTS_DFT_Detect(V90aDftStruct *pDFT, SWORD sample, CQWORD *out);
SBYTE V8_QTS_Angle_Detect(V90aDftStruct *pDFT, CQWORD *out);
void V8_V90a_TimRx_init(TimingRxStruct *pTIM);

extern CONST UBYTE V8_QC1a_SETUP92[V8_QC1A_LENGTH];
extern CONST UBYTE V8_QC2a_SETUP92[V8_QC2A_LENGTH];
#endif /* SUPPORT_V92A */

#if V8_HIGHPASS_FILTER_ENABLE
extern const QWORD V8_HpfCoef[];        /* 2nd order biquad: for Epson */
#endif

#endif
