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

#ifndef _V29EXT_H
#define _V29EXT_H

#include "dspdext.h"
#include "v29stru.h"

void   V29_Dummy(V29Struct *pV29);
void   V29_HS_TEP(V29Struct *pV29);
void   V29_HS_Segment_1(V29Struct *pV29);
void   V29_HS_Segment_3(V29Struct *pV29);
void   V29_HS_Segment_4(V29Struct *pV29);
void   V29_Data_Transfer(V29Struct *pV29);
void   V29_EQ_Delay(V29Struct *pV29);
void   V29_TX_Scrambler(V29Struct *pV29);
void   V29_Segment2_Encode(V29Struct *pV29);
void   V29_Segment3_Encode(V29Struct *pV29);
void   V29_TX_Encoder(V29Struct *pV29);
void   V29_Init_ifilter(QIFIRStruct *f, SWORD *dline, SWORD *coef, SWORD N, SWORD ifactor);
void   V29_iFilter_init(V29Struct *pV29);
void   V29_TX_Psf(V29Struct *pV29);
void   V29_TX_Modulate(V29Struct *pV29);
void   V29_FSM_init(V29Struct *pV29);
void   V29_TEP(V29Struct *pV29);
void   V29_Segment_1(V29Struct *pV29);
void   V29_Segment_2(V29Struct *pV29);
void   V29_Segment_3(V29Struct *pV29);
void   V29_Segment_4(V29Struct *pV29);
void   V29_Data_Mode(V29Struct *pV29);
void   V29_isr(UBYTE **pTable);
void   V29_isr_main(V29Struct *);
void   V29_TX_Sym(V29Struct *pV29);
void   V29_DPCS_Init(UBYTE **pTable);
void   V29_init_main(V29Struct *pV29);
void   V29_TX_init(V29Struct *pV29);
void   V29_RX_Sym(V29Struct *pV29);
void   V29_RX_init(V29Struct *pV29);
void   V29_CarrierRecovery_init(V29Struct *pV29);
void   V29_CarrierUpdate(V29Struct *pV29);
void   V29_Guard_or_ABtone_Detect(V29Struct *pV29);
void   V29_AB_Detect_after_Guardtone(V29Struct *pV29);
void   V29_Receive_Segment2(V29Struct *pV29);
void   V29_Receive_Segment3(V29Struct *pV29);
void   V29_Receive_DataMode(V29Struct *pV29);
void   V29_RX_Decoder(V29Struct *pV29);
void   V29_Demodulate_init(V29Struct *pV29);
void   V29_Demodulate(V29Struct *pV29);
void   V29_CarrLoss_Detect(V29Struct *pV29);
void   V29_CalSagcScaleEnergy(V29Struct *pV29);
void   V29_Sagc(V29Struct *pV29, QWORD qAGCRef);
void   V29_RX_Derotate_Get_TRN(V29Struct *pV29);
void   V29_RX_Derotate(V29Struct *pV29);
void   V29_RX_Descram(V29Struct *pV29);
void   V29_AutoCorrDetect_init(V29Struct *pV29);
void   V29_AutoCorr_Detect(V29Struct *pV29);
void   V29_RX_Eq_Dummy(V29Struct *pV29);
void   V29_RX_Eq(V29Struct *pV29);
void   V29_RX_EqUpdate(V29Struct *pV29);
void   V29_RX_Rotate_Dummy(V29Struct *pV29);
void   V29_RX_Rotate(V29Struct *pV29);
void   V29_Slicer_init(V29Struct *pV29);
void   V29_RX_Slicer(V29Struct *pV29);
void   V29_RX_Timing_Dummy(V29Struct *pV29);
void   V29_TimingRecovery(V29Struct *pV29);
void   V29_TimingUpdate_init(V29Struct *pV29);
void   V29_RX_TimingUpdate(V29Struct *pV29);
void   V29_TimingLoopFilter(V29Struct *pV29, SWORD TE_input);
void   V29_RX_TrainEncoder(V29Struct *pV29);
void   V29_RX_TrainScrambler(V29Struct *pV29);
void   V29_Rx_Segm1_ToneDetect(V29Struct *pV29);
void   V29_Rx_Segm1_Silent(V29Struct *pV29);
void   V29_Rx_Segm2(V29Struct *pV29);
void   V29_Rx_Segm3(V29Struct *pV29);
void   V29_Rx_Segm4(V29Struct *pV29);
void   V29_Rx_DataMode(V29Struct *pV29);
CQWORD V29_Lookup_iq(V29Struct *pV29);

#if V29_MODEM_ENABLE
extern CONST SBYTE  Phase4800_array[];
extern CONST CQWORD V29_tIQTable_Decode_4800[];
extern CONST SBYTE  Q23_Array[];
#endif

extern CONST SBYTE  Phase_array[];
extern CONST SBYTE  Q234_Array[];
extern CONST QWORD  V29_DetectTone_Coef[];
extern CONST CQWORD V29_tIQTable_Decode_9600[];
extern CONST CQWORD V29_tIQTable_Decode_7200[];
extern CONST SBYTE  V29_tAbsolutePhase[];
extern CONST SWORD  V29_RxDelayTable[];
extern CONST SWORD  V29_TxDelayTable[];

#endif
