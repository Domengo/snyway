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

#ifndef _V27EXT_H
#define _V27EXT_H

#include "dspdext.h"
#include "v27stru.h"

void  V27_Dpcs_Isr(UBYTE **pTable);
void  V27_init_main(V27Struct *pV27);
void  V27_Dummy(V27Struct *pV27);
void  V27_HS_Segment_UnMod_Carrier(V27Struct *pV27);
void  V27_HS_Segment_Silent(V27Struct *pV27);
void  V27_HS_Segment_180_Phase(V27Struct *pV27);
void  V27_HS_Segment_0_180_Phase(V27Struct *pV27);
void  V27_HS_Segment_Scrambled_One(V27Struct *pV27);
void  V27_TX_Data(V27Struct *pV27);
void  V27_Scrambler_init(V27Struct *pV27);
void  V27_TX_Scrambler(V27Struct *pV27);
void  V27_Scram_Bit_GPC(V27Struct *pV27);
void  V27_Segment4_Encoder(V27Struct *pV27);
void  V27_TX_Encoder(V27Struct *pV27);
void  V27_iFilter_init(V27Struct *pV27);
void  V27_TX_Psf(V27Struct *pV27);
void  V27_TX_Modulate(V27Struct *pV27);
void  V27_FSM_init(V27Struct *pV27);
void  V27_Segment_UnMod_Carrier(V27Struct *pV27);
void  V27_Segment_Silent(V27Struct *pV27);
void  V27_Segment_180_Phase(V27Struct *pV27);
void  V27_Segment_0_180_Phase(V27Struct *pV27);
void  V27_Segment_Scrambled_One(V27Struct *pV27);
void  V27_Data_Mode(V27Struct *pV27);
void  V27_Segment_Eq_delay(V27Struct *pV27);
void  V27_init(V27Struct *pV27);
void  V27_TX_init(V27Struct *pV27);
void  V27_RX_init(V27Struct *pV27);
void  V27_isrT(V27Struct *pV27);
void  V27_isrR(V27Struct *pV27);
void  V27_isrTR(V27Struct *pV27);
void  V27_TX_Sym(V27Struct *pV27);
void  V27_RX_Sym(V27Struct *pV27);
void  V27_Guard_or_ABtone_Detect(V27Struct *pV27);
void  V27_AB_Detect_after_Guardtone(V27Struct *pV27);
void  V27_Receive_180_Phase(V27Struct *pV27);
void  V27_Receive_Data(V27Struct *pV27);
void  V27_RX_Decoder(V27Struct *pV27);
void  V27_Demodulate_init(V27Struct *pV27);
void  V27_Demodulate(V27Struct *pV27);
void  V27_CarrLoss_Detect(V27Struct *pV27);
void  V27_DerotateErr(V27Struct *pV27);
void  V27_RX_Derotate_Get_TRN(V27Struct *pV27);
void  V27_RX_Derotate(V27Struct *pV27);
void  V27_RX_Rotate_Dummy(V27Struct *pV27);
void  V27_RX_Rotate(V27Struct *pV27);
void  V27_Descrambler_init(V27Struct *pV27);
void  V27_RX_Descram(V27Struct *pV27);
void  V27_Descram_Bit_GPC(V27Struct *pV27);
void  V27_AutoCorrDetect_init(V27Struct *pV27);
void  V27_AutoCorr_Detect(V27Struct *pV27);
void  V27_CalSagcScaleEnergy(V27Struct *pV27);
void  V27_Sagc(V27Struct *pV27, QWORD Reference_Energy);
void  V27_EqSkipFilter(V27Struct *pV27);
void  V27_Eq(V27Struct *pV27);
void  V27_RX_EqUpdate(V27Struct *pV27);
void  V27_Slicer_init(V27Struct *pV27);
void  V27_RX_Slicer(V27Struct *pV27);
void  V27_TimingRecovery(V27Struct *pV27);
void  V27_RX_Timing_Dummy(V27Struct *pV27);
void  V27_RX_TrainEncoder(V27Struct *pV27);
void  V27_RX_TrainScrambler(V27Struct *pV27);
void  V27_Rx_Segm1_Carrier(V27Struct *pV27);
void  V27_Get_Segment_UnMod_Carrier(V27Struct *pV27);
void  V27_Rx_Segm2_Silent(V27Struct *pV27);
void  V27_Rx_Segm3_180_Phase(V27Struct *pV27);
void  V27_Segment_0_180_Phase_Raw(V27Struct *pV27);
void  V27_Segment_0_180_Phase_Fine(V27Struct *pV27);
void  V27_Get_Segment_Scrambled_One(V27Struct *pV27);
void  V27_Get_Data(V27Struct *pV27);
void  V27_CarrierRecovery_init(V27Struct *pV27);
void  V27_CarrierUpdate(V27Struct *pV27, CQWORD cqSliceIQ);
void  V27_CarUpdateUsingTrainIQ(V27Struct *pV27);
void  V27_CarUpdateUsingSliceIQ(V27Struct *pV27);
void  V27_RX_TimingUpdate_init(V27Struct *pV27);
void  V27_RX_TimingUpdate_Train_fine(V27Struct *pV27);
void  V27_RX_TimingUpdate(V27Struct *pV27);
void  V27_TimingLoopFilter(V27Struct *pV27, SWORD qTE_input);
void  V27_TimingUpdate(V27Struct *pV27, CQWORD cqTimeIQ);

extern CONST CQWORD tV27_Segment4_LOOKUP_IQ[];
extern CONST CQWORD tV27_LOOKUP_IQ[];
extern CONST SBYTE  V27_qPhase_array_4800[];
extern CONST SBYTE  V27_qPhase_array_2400[];
extern CONST SBYTE  V27_qPhase_array_Segment4[];
extern CONST SBYTE  V27_LOOKUP_DIBITS_1200[];
extern CONST SBYTE  V27_LOOKUP_DIBITS_1600[];
extern CONST QWORD  V27_DetectTone_1600_Coef[];
extern CONST QWORD  V27_DetectTone_1200_Coef[];
extern CONST CQWORD tV27_IQTable_Decode_4800[];
extern CONST CQWORD tV27_IQTable_Decode_2400[];
extern CONST SWORD  tV27_low_pass_coef_1600[];
extern CONST SWORD  tV27_low_pass_coef_1200[];

extern CONST SWORD  V27_RxDelayTable_4800[];
extern CONST SWORD  V27_RxDelayTable_2400[];
extern CONST SWORD  V27_TxDelayTable_4800[];
extern CONST SWORD  V27_TxDelayTable_2400[];

#if V27_SHORT_TRAIN_SUPPORT
extern CONST SWORD  V27_RxDelayTable_ShortTrain[];
extern CONST SWORD  V27_TxDelayTable_ShortTrain[];
#endif

#endif
