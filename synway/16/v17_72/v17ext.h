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

#ifndef _V17EXT_H
#define _V17EXT_H

#include "v3217ext.h"
#include "v17stru.h"

void  V17_Dummy(V17Struct *pV17);
void  V17_DPCS_Init(UBYTE **pTable);
void  V17_DPCS_ISR(UBYTE **pTable);
void  V17_init(V17Struct *pV17);
void  V17_isr(V17Struct *pV17);
void  V17_FSM_init(V17Struct *pV17);
void  V17_TX_Sym(V17Struct *pV17);
void  V17_RX_Sym(V17Struct *pV17);
void  V17_TX_Init(V17Struct *pV17);
void  V17_TX_DataIn_A(V17Struct *pV17);
void  V17_TX_DataIn_silent(V17Struct *pV17);
void  V17_TX_DataIn_AB(V17Struct *pV17);
void  V17_TX_DataIn_Send_eq_training(V17Struct *pV17);
void  V17_TX_DataIn_bridge(V17Struct *pV17);
void  V17_TX_DataIn_Send_TCF(V17Struct *pV17);
void  V17_Send_A(V17Struct *pV17);
void  V17_Send_SILENCE(V17Struct *pV17);
void  V17_Send_seg1(V17Struct *pV17);
void  V17_Send_seg2(V17Struct *pV17);
void  V17_Send_seg3(V17Struct *pV17);
void  V17_Send_seg4(V17Struct *pV17);
void  V17_Send_TCF(V17Struct *pV17);
void  V17_TX_SigMap_Send_A(V32ShareStruct *pV32Share);
void  V17_TX_SigMap_Send_AB(V32ShareStruct *pV32Share);
void  V17_RX_Init(V17Struct *pV17);
void  V17_Rx_Atone_or_ABtone(V17Struct *pV17);
void  V17_Rx_Segm1AB_after_A(V17Struct *pV17);
void  V17_Rx_Segm1_Long(V17Struct *pV17);
void  V17_Rx_Segm1_Short(V17Struct *pV17);
void  V17_Rx_Segm2_Long(V17Struct *pV17);
void  V17_Rx_Segm2_Short(V17Struct *pV17);
void  V17_Rx_Segm3_Long(V17Struct *pV17);
void  V17_Rx_Segm4(V17Struct *pV17);
void  V17_Rx_SegmData(V17Struct *pV17);
void  V17_Guard_or_ABtone_Detect(V17Struct *pV17);
void  V17_AB_Detect_after_Guardtone(V17Struct *pV17);
void  V17_RX_DataOut_Seg1L(V17Struct *pV17);
void  V17_RX_DataOut_Seg1S(V17Struct *pV17);
void  V17_RX_DataOut_Seg2L(V17Struct *pV17);
void  V17_RX_DataOut_Seg2S(V17Struct *pV17);
void  V17_RX_DataOut_Seg3(V17Struct *pV17);
void  V17_RX_DataOut_Seg4(V17Struct *pV17);
void  V17_RX_DataOut_DataMode(V17Struct *pV17);
void  V17_RX_TimeJam_Init(V32ShareStruct *pV32Share);
void  V17_RX_TimeJam(V32ShareStruct *pV32Share);
void  V17_RX_S_TrainSigMap(V32ShareStruct *pV32Share);
void  V17_Rotate(V17Struct *pV17);
void  V17_BypassRotate(V17Struct *pV17);
void  V17_CarrLoss_Detect(V17Struct *pV17);
void  V17_AutoCorrDetect_init(V17Struct *pV17);
void  V17_AutoCorr_Detect(V17Struct *pV17);
void  V17_SyncEq(V32ShareStruct *pV32Share);
void  V17_SyncDemod(V17Struct *pV17);

extern CONST UBYTE    bit_des[8];
extern CONST CQWORD V17_IQTable_4800NR[];
extern CONST QWORD qV17_DetectToneCoefTab[];
extern CONST UBYTE V17_SigTransMapTable[4];

#endif
