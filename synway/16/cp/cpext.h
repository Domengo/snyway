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

#ifndef _CPEXT_H
#define _CPEXT_H

#include "cpstru.h"
#include "dspdext.h"

void CP_init(CpStruct *pCp);
void CP_FSM_init(CpStruct *pCp);

/* Tx FSM functions */
void CP_Send_Dummy(CpStruct *pCp);
void CP_Send_Silence(CpStruct *pCp);
void CP_Send_Nothing(CpStruct *pCp);
void CP_Send_Pulses(CpStruct *pCp);
void CP_Send_DTMFTones(CpStruct *pCp);
void CP_Send_ANSTone(CpStruct *pCp);
void CP_Send_Timeout_Silence(CpStruct *pCp);

#if SUPPORT_FAX_SEND_CNG
void CP_Send_CNG(CpStruct *pCp);
void CP_TxState_CNG(CpStruct *pCp);
#endif

void CP_Dummy(CpStruct *pCp);
void CP_TxState_Send_Silence(CpStruct *pCp);
void CP_TxState_Send_Nothing(CpStruct *pCp);
void CP_TxState_Send_Pulses(CpStruct *pCp);
void CP_TxState_Send_DTMFTones(CpStruct *pCp);
void CP_TxState_Send_TSilence(CpStruct *pCp);
void CP_TxState_ANSTone(CpStruct *pCp);

void CP_Mod_Dummy(CpStruct *pCp);
void CP_Mod(CpStruct *pCp);

void CP_Detect_init(CpStruct *pCp);
void CP_Detect(CpStruct *pCp);

void CP_tx_buf(CpStruct *pCp);
void CP_Isr(CpStruct *pCp);
void CP_tx_init(CpStruct *pCp);

void CP_DpcsIsr(UBYTE **);

void CP_Tone_init_CANADA(CpStruct *pCp);
void CP_Tone_init_CHINA(CpStruct *pCp);
void CP_Tone_init_FRANCE(CpStruct *pCp);
void CP_Tone_init_GERMANY(CpStruct *pCp);
void CP_Tone_init_ISRAEL(CpStruct *pCp);
void CP_Tone_init_KOREA(CpStruct *pCp);
void CP_Tone_init_NETHERLANDS(CpStruct *pCp);
void CP_Tone_init_SPAIN(CpStruct *pCp);
void CP_Tone_init_UK(CpStruct *pCp);
void CP_Tone_init_VANUATU(CpStruct *pCp);
void CP_Tone_init_AUSTRALIA(CpStruct *pCp);
void CP_Tone_init_JAPAN(CpStruct *pCp);

SWORD CP_Cadence_OnTimeMS(SWORD OnTime);
SWORD CP_Cadence_OffTimeMS(SWORD OffTime);

void  Cadence_init(CpStruct *pCp);
UBYTE Ca_Calculate(CpStruct *pCp, SWORD *pPCMin);
UBYTE CheckToneCadence(CpStruct *pCp , CaStruct *pCaRef);
UBYTE CheckPureTone(CpStruct *pCp , CaStruct *pCaRef);

void  DtmfGeneration(DtmfGenStruct *pDtmf, QWORD *PcmOut);
UBYTE InitDtmfGen(DtmfGenStruct *pDtmf, CHAR DialNum);
SBYTE DialChangeToDigital(CHAR DialNum);

extern CONST UWORD DtmfDigitRow[];
extern CONST UWORD DtmfDigitCol[];
extern CONST QWORD CP_tToneGcoefs[];

#endif
