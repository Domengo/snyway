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

#ifndef _PH2EXT_H
#define _PH2EXT_H

#include "dspdext.h"
#include "ph2stru.h"

void Ph2_DPSK_Detect_Init(DPSKInfo *pDpsk);
void Ph2_DPSK_Init(DPSKInfo *pDpsk, UBYTE modem_mode);
void Ph2_DPSK_Modfilter(Ph2Struct *pPH2, UBYTE bit, SWORD qModemCarrier);
void Ph2_DPSK_Mod(Ph2Struct *pPH2, UBYTE bit, SWORD qModemCarrier);
void Ph2_DPSK_Silence(Ph2Struct *pPH2);
void Ph2_Rtrn_Init(Ph2Struct *pPH2);
void Ph2_Send_Rtrn_Silence(Ph2Struct *pPH2);
void Ph2_FFT(FFT_Data *I, CQWORD *, CQWORD *pcOut);
void Ph2_FFT_Init(FFT_Data *, CQWORD *, UBYTE *, UBYTE, UBYTE);
void Ph2_Detect_Info_0_Init(Info_0Struc *pI0);
void Ph2_Detect_Info_1a_Init(Info_1aStruc *);
void Ph2_Detect_Info_1c_Init(Info_1cStruc *);
void Ph2_Check_Tone_N(Ph2Struct *pPH2, UBYTE N);
void Ph2_Send_Silence(Ph2Struct *pPH2);
void Ph2_Calc_Pre_Filter_Gain(QDWORD pqdGain[6][11][25], UBYTE sym_rate_idx);
void Ph2_Send_Silence_75MS(Ph2Struct *pPH2);
void Ph2_Send_Info_0c(Ph2Struct *pPH2);
void Ph2_Create_Tone_B(Ph2Struct *pPH2);
void Ph2_Create_Tone_B_40MS(Ph2Struct *pPH2);
void Ph2_Tone_B_Phase_Rev_RTD(Ph2Struct *pPH2);
void Ph2_Create_Tone_B_10MS(Ph2Struct *pPH2);
void Ph2_Tone_B_Phase_Rev(Ph2Struct *pPH2);
void Ph2_Create_Tone_B_10MS_Before_L1(Ph2Struct *pPH2);
void Ph2_Create_L1(Ph2Struct *pPH2);
void Ph2_Create_L2(Ph2Struct *pPH2);
void Ph2_Setup_Info1c(Ph2Struct *pPH2);
void Ph2_Send_Info_1c(Ph2Struct *pPH2);
void Ph2_Detect_Tone_A(Ph2Struct *pPH2);
void Ph2_Det_Tone_A_PV_40MS(Ph2Struct *pPH2);
void Ph2_Det_Tone_A_PV_RTD(Ph2Struct *pPH2);
void Ph2_Detect_L1(Ph2Struct *pPH2);
void Ph2_Detect_L2(Ph2Struct *pPH2);
void Ph2_Detect_Tone_A_After_L2(Ph2Struct *pPH2);
void Ph2_Detect_Tone_A2(Ph2Struct *pPH2);
void Ph2_Detect_Info_1a(Ph2Struct *pPH2);

SBYTE Ph2_DPSK_Demod(Ph2Struct *pPH2);
UBYTE Ph2_DPSK_PhaseReversal_Detected(Ph2Struct *pPH2);
SBYTE Ph2_V34Detect_Info_0(UBYTE, Info_0Struc *);
SBYTE Ph2_V34Detect_Info_1a(UBYTE, Info_1aStruc *);
SBYTE Ph2_V34Detect_Info_1c(UBYTE, Info_1cStruc *);

/* Phase 2 Answer modem */
void Ph2_Send_Info_0A(Ph2Struct *pPH2);
void Ph2_Create_Tone_A_50MS(Ph2Struct *pPH2);
void Ph2_Create_Tone_A_Phase_Change(Ph2Struct *pPH2);
void Ph2_Create_Tone_A_40MS(Ph2Struct *pPH2);
void Ph2_Create_L1_ANS(Ph2Struct *pPH2);
void Ph2_Create_L2_ANS(Ph2Struct *pPH2);
void Ph2_Create_Tone_A_After_L2(Ph2Struct *pPH2);
void Ph2_Create_Tone_A_PR_10MS(Ph2Struct *pPH2);
void Ph2_Create_Tone_A_10MS(Ph2Struct *pPH2);
void Ph2_Create_Tone_A(Ph2Struct *pPH2);
void Ph2_Create_Tone_A_Set_1A(Ph2Struct *pPH2);
void Ph2_Send_Info_1A(Ph2Struct *pPH2);
void Ph2_Cleanup_Tx(Ph2Struct *pPH2);
void Ph2_Detect_Info_0(Ph2Struct *pPH2);
void Ph2_Det_Tone_B(Ph2Struct *pPH2);
void Ph2_Det_Tone_B_PV_40MS(Ph2Struct *pPH2);
void Ph2_Det_Recovery(Ph2Struct *pPH2);
void Ph2_Det_Tone_B1(Ph2Struct *pPH2);
void Ph2_CarrierLoss_Detect(Ph2Struct *pPH2);
void Ph2_Det_Tone_B_PV_670MS(Ph2Struct *pPH2);
void Ph2_Detect_Info_1c(Ph2Struct *pPH2);
void Ph2_Receive_Info_1(Ph2Struct *pPH2);
void Ph2_Dummy(Ph2Struct *pPH2);
void Ph2_Det_B_Recovery(Ph2Struct *pPH2);
void Ph2_StateInit(Ph2Struct *pPH2);
void Ph2_Init(UBYTE **pTable);
void Ph2_Prob_Init(ProbStruc *);
void Ph2_Create_SymRate_Table(Ph2Struct *pPH2, Info_0Struc *pI0tx, Info_0Struc *pI0rx, UBYTE SymRate);
void Ph2_RxProbing_Init(ProbStruc *I);
void Ph2_Probing_Result(SymRate_Info *, ProbStruc *, UBYTE);
void Ph2_Preemp_Idx(UBYTE sym_rate_idx, ProbStruc *probing);
void Ph2_HW_Setup(UBYTE **pTable);
void Ph2_DPCS(UBYTE **pTable);
void Ph2_Biquad_Tone_Det(Ph2Struct *pPH2);
void Ph2_DetCheck_ToneAB(Ph2Struct *pPH2, UBYTE N);
void Ph2_CreateInfo(UBYTE *pB, UBYTE *pI, CONST SBYTE *pST);
void Ph2_Handler(UBYTE **pTable);
void Ph2_ShiftDataToInfo(UBYTE *pData, UBYTE *pInfo, CONST SBYTE *pShiftTable);
void Ph2_UpdateTxDelayCnt(Ph2Struct *pPH2);    /* in ph2txa.c */
void Ph2_RetrainParamInit(Ph2Struct *pPH2);
void Ph2_Get_NoiseEgy(Ph2Struct *pPH2, QWORD *qPcmIn);
UBYTE Ph2_SNR_calcu(Ph2Struct *pPH2);
void Ph2_NLD_Analyze(Ph2Struct *pPH2);
void Ph2_Create_Probing(Ph2Struct *pPH2, QWORD qAmp, SWORD *pRepeat_counter);
void Ph2_Estimate_Power_Spectrum(Ph2Struct *pPH2);
void V34_DFT_Init(DFT_Info *I, QWORD *pqQ1_buf, QWORD *pqQ2_buf, QWORD *pqCoef, QWORD *pqMag_sqr, SWORD N, UBYTE Num_Freq);
void V34_Phase3_Init(UBYTE **);

QDWORD Ph2_Pre_Filter_Gain(QDWORD qNorm_freq, SWORD preemp_idx, UBYTE ubMode);
UBYTE Ph2_Check_Edges(UBYTE, QWORD *, QWORD *, ProbStruc *);
QWORD Ph2_Tone_Power(QWORD, QWORD *);
UBYTE Ph2_ShiftData(UBYTE *pData, UBYTE shift);
SBYTE Ph2_ChkCrc(UBYTE *pData, UWORD ref);
UBYTE Ph2_Is6_8_12_16(UBYTE i);
UBYTE V34_DFT(DFT_Info *I, SWORD sample);

#if SUPPORT_V90A
void Ph2_SetupInfo1A_V90A(Ph2Struct *pPH2);
SBYTE Ph2_Detect_Info_0d(UBYTE bit, Info_0Struc *pI0);
#endif
#if SUPPORT_V90A
void V90a_Init_Analog(UBYTE **);
#endif
#if SUPPORT_V92A
void V92a_Init_Analog(UBYTE **);
#endif

#if SUPPORT_V90D
void V90d_Init_Digital(UBYTE **pTable);
SBYTE Ph2_V90dDetect_Info_1a(UBYTE bit, Info_1aStruc *pI1a);
#endif

#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)
void Ph2_MH_Send_RT(Ph2Struct *pPH2);
void Ph2_MH_Send_Sequence(Ph2Struct *pPH2);
void Ph2_MH_Send_Silence(Ph2Struct *pPH2);
void Ph2_Init_MH_Response(Ph2Struct *pPH2);
void Ph2_Init_MH_Request(Ph2Struct *pPH2);
void Ph2_MH_Setup_Response(Ph2Struct *pPH2);
void Ph2_MH_Detect_Sequence(Ph2Struct *pPH2);
void Ph2_MH_Detect_RTSilence(Ph2Struct *pPH2);
void Ph2_MH_Detect_Dummy(Ph2Struct *pPH2);
void Ph2_MH_Detect_End_RT(Ph2Struct *pPH2);
void Ph2_MH_Detect_RT(Ph2Struct *pPH2);
UBYTE Ph2_MH_Info_Detect(UBYTE bit, Info_0Struc *pI0);
#endif

extern CONST QWORD V34_tDPSK_HM_Coef_Call[];
extern CONST QWORD V34_tDPSK_HM_Coef_Ans[];
extern CONST QWORD V34_tDPSK_LP[48];
extern CONST QWORD V34_tDPSK_CMCOEF[V34_DPSK_CALL_LEN];
extern CONST QWORD V34_tDPSK_ANCOEF[V34_DPSK_ANS_LEN];
extern CONST QWORD V34_tPREEMP_BETA[];
extern CONST QWORD V34_tPREEMP_GAMA[];
extern CONST QWORD V34_tPREEMP_ALPHA[];
extern CONST UBYTE V34_tMAX_DRATE[2][6];
extern CONST UBYTE V34_tONE_OVER_SYM[6];
extern CONST QWORD V34_tONE_OVER_SYM_OFFSET[6];
extern CONST QWORD V34_tCARRIER[6][2];
extern CONST QWORD BIQ_COEF_ANS[18];
extern CONST QWORD BIQ_COEF_CALL[18];
extern CONST SBYTE ShiftTable0[];
extern CONST SBYTE ShiftTable1c[];
extern CONST SBYTE ShiftTable1a[];
#if SUPPORT_V90
extern CONST SBYTE ShiftTable0d[];
#endif
#if SUPPORT_V90A
extern CONST SBYTE ShiftTable1a_V90[];
#endif
#if SUPPORT_V92
extern CONST SBYTE ShiftTable1a_V92[];
#endif
#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)
extern CONST SBYTE ShiftTable_MH[];
#endif

#if SUPPORT_V34FAX
void   Ph2_Detect_Info_h_Init(Info_hStruc *);
void   Ph2_Detect_Info_h(Ph2Struct *pPH2);
void   Ph2_Create_Tone_A_Set_H(Ph2Struct *pPH2);
void   Ph2_Send_Info_H(Ph2Struct *pPH2);
SBYTE  Ph2_V34Detect_Info_h(UBYTE, Info_hStruc *);
void   V34Fax_PCET_Init(UBYTE **pTable);

extern CONST SBYTE ShiftTableh[];
#endif

#include "commmac.h"
#include "q_const.h"
#include "gstdio.h"
#include "ptable.h"
#include "dpcs.h"

#endif
