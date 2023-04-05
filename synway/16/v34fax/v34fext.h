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

#ifndef _V34FEXT_H
#define _V34FEXT_H

#include "v34ext.h"
#include "v34fstru.h"

#if SUPPORT_V34FAX

void V34Fax_SyncEq(V34FaxStruct *pV34Fax);
void V34Fax_SyncEq_Init(V34FaxStruct *pV34Fax);
void V34Fax_SyncTiming(V34FaxStruct *pV34Fax);
void V34Fax_SyncCarrier(V34FaxStruct *pV34Fax);

void V34Fax_PCR_SyncInit(V34Struct *pV34);
void V34Fax_PCR_SyncEq(V34Struct *pV34, CQWORD *pIn);
void V34Fax_PCR_SyncDemod(V34Struct *pV34, CQWORD *cqDemodOut);
void V34Fax_PCR_SyncTiming(V34FaxStruct *pV34Fax, SWORD nTimingIdx);
void V34Fax_PCR_SyncCarrier(V34Struct *pV34, CQWORD cqPrevSymbol, CQWORD cqNewSymbol);

/* V34_MPET_demo.c */
void V34Fax_Gain(V34FaxStruct *pV34Fax);
void V34Fax_Rx_BPF_Init(V34FaxStruct *pV34Fax);
void V34Fax_Rx_BPF(V34Struct *pV34);
void V34Fax_Demodulation_Init(V34FaxStruct *pV34Fax);
void V34Fax_Demodulation(V34FaxStruct *pV34Fax);
void V34Fax_Carrier_Update_Init(V34FaxStruct *pV34Fax);
void V34Fax_Carrier_Update(V34FaxStruct *pV34Fax);
void V34Fax_Rotate(V34FaxStruct *pV34Fax);
void V34Fax_Derotate(V34FaxStruct *pV34Fax);
void V34Fax_Decoder(V34FaxStruct *pV34Fax);
void V34Fax_Descramble(V34FaxStruct *pV34Fax);
void V34Fax_EqUpdate(V34FaxStruct *pV34Fax);
void V34Fax_EQ(V34FaxStruct *pV34Fax);
void V34Fax_Sh_Slice(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Slice(V34FaxStruct *pV34Fax);
void V34Fax_Slice(V34FaxStruct *pV34Fax);
CQWORD V34Fax_Create_PPh(UBYTE k, UBYTE I);

/* V34_MPET_handler.c */
void V34Fax_MPET_Handle(UBYTE **pTable);
void V34Fax_MPET_Retrn_by_Remote(V34FaxStruct *pV34Fax);
void V34Fax_MPET_Retrn_by_Local(V34FaxStruct *pV34Fax);

/* V34_MPET_init.c */
void V34Fax_MPET_Init(UBYTE **pTable);
void V34Fax_MPET_Fsm_Init(V34FaxStruct *pV34Fax);
void V34Fax_MPET_Tx_Init(V34FaxStruct *pV34Fax);
void V34Fax_MPET_Rx_Init(V34FaxStruct *pV34Fax);

/* V34_MPET_modu.c */
void V34Fax_Scramble(V34FaxStruct *pV34Fax);
void V34Fax_Encode(V34FaxStruct *pV34Fax);
void V34Fax_PSF_Init(V34FaxStruct *pV34Fax);
void V34Fax_PSF(V34FaxStruct *pV34Fax);
void V34Fax_Modulation_Init(V34FaxStruct *pV34Fax);
void V34Fax_Modulation(V34Struct *pV34);

/* V34_MPET_mph.c */
void  V34Fax_Detect_Mph_Init(MphStruc *pMphRx);
SBYTE V34Fax_Detect_Mph(UBYTE bit, MphStruc *pMphRx);
void  V34Fax_Create_Mph(UBYTE *pBuf, MphStruc *pMphTx);

/* V34_MPET_rx.c */
void V34Fax_Symbol_Rx(V34Struct *pV34);

void V34Fax_PPh_Sh_Detection(V34Struct *pV34);
void V34Fax_PPh_Detection(V34Struct *pV34);
void V34Fax_PPh_Sh_AnsRx(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Sh_CallRx(V34FaxStruct *pV34Fax);
void V34Fax_ShB_Rx(V34FaxStruct *pV34Fax);
void V34Fax_ShB_Rx_ReSync(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Rx(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Train(V34FaxStruct *pV34Fax);
void V34Fax_TrainPPh_Rx(V34FaxStruct *pV34Fax);
void V34Fax_ALT_Rx(V34FaxStruct *pV34Fax);
void V34Fax_MPh_Rx(V34FaxStruct *pV34Fax);
void V34Fax_E_Rx(V34FaxStruct *pV34Fax);
void V34Fax_T30_Rx(V34FaxStruct *pV34Fax);
void V34Fax_ONEs_Rx(V34FaxStruct *pV34Fax);
void V34Fax_Silence_Rx(V34FaxStruct *pV34Fax);

/* V34_MPET_setup.c */
void V34Fax_MPET_fsm_StateProtect(V34FaxStruct *pV34Fax);
void V34Fax_Dummy(V34FaxStruct *pV34Fax);
void V34Fax_Silence_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Sh_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_ShB_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Train_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_ALT_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_MPh_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_E_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_T30_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_ONEs_Rx_SetUp(V34FaxStruct *pV34Fax);
void V34Fax_Rx_SetUp(V34FaxStruct *pV34Fax);

void V34Fax_Silence_Tx_Setup(V34FaxStruct *pV34Fax);
void V34Fax_Sh_Tx_Setup(V34FaxStruct *pV34Fax);
void V34Fax_MPET_AC_Tx_Setup(V34FaxStruct *pV34Fax);
void V34Fax_ShB_Tx_Setup(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Tx_Setup(V34FaxStruct *pV34Fax);
void V34Fax_ALT_Tx_Setup(V34FaxStruct *pV34Fax);
void V34Fax_MPh_Tx_Setup(V34FaxStruct *pV34Fax);
void V34Fax_E_Tx_Setup(V34FaxStruct *pV34Fax);
void V34Fax_T30_Tx_SetUp(V34FaxStruct *pV34Fax);

/* V34_MPET_timing.c */
void V34Fax_Timing_Recovery_Init(V34FaxStruct *pV34Fax);
void V34Fax_Timing_Recovery(V34FaxStruct *pV34Fax);
void V34Fax_Timing_Update_Init(V34FaxStruct *pV34Fax);
void V34Fax_Timing_Update(V34FaxStruct *pV34Fax);

/* V34_MPET_tx.c */
void V34Fax_Symbol_Tx(V34Struct *pV34);
void V34Fax_Silence_Tx(V34FaxStruct *pV34Fax);
void V34Fax_Sh_ShB_Tx(V34FaxStruct *pV34Fax);
void V34Fax_MPET_AC_Tx(V34FaxStruct *pV34Fax);
void V34Fax_PPh_Tx(V34FaxStruct *pV34Fax);
void V34Fax_ALT_Tx(V34FaxStruct *pV34Fax);
void V34Fax_MPh_Tx(V34FaxStruct *pV34Fax);
void V34Fax_E_Tx(V34FaxStruct *pV34Fax);
void V34Fax_T30_Tx(V34FaxStruct *pV34Fax);

/* V34_PCET_ans.c */
void V34Fax_PCET_Det_S(V34Struct *pV34);
void V34Fax_PCET_Det_S_bar(V34Struct *pV34);
void V34Fax_PCET_Wait_S_bar(V34Struct *pV34);
void V34Fax_PCET_EQ_Delay(V34Struct *pV34);
void V34Fax_PCET_PP_EQ(V34Struct *pV34);
void V34Fax_PCET_TRN_EQ(V34Struct *pV34);
void V34Fax_PCET_TRN_EQ_TC(V34Struct *pV34);
void V34Fax_CarrierLoss_Detect(V34Struct *pV34);
//void V34Fax_PCET_Rec_Silence(V34Struct *);
void V34Fax_EQ_Train(UWORD FuncCode, V34Struct *pV34, CQWORD Train_sym, QWORD qBeta);

/* V34_PCET_call.c */
void V34Fax_PCET_PCR_Send_Silence_70MS(V34Struct *pV34);
void V34Fax_PCET_PCR_Send_S(V34Struct *pV34);
void V34Fax_PCET_PCR_Send_S_Bar(V34Struct *pV34);
void V34Fax_PCET_PCR_Send_PP(V34Struct *pV34);
void V34Fax_PCET_Send_TRN(V34Struct *pV34);

/* V34_PCET_handler.c */
void V34Fax_PCET_Handler(UBYTE **pTable);

/* V34_PCET_init.c */
void V34Fax_PCET_Init(UBYTE **pTable);

/* V34_PCR_ans.c */
UBYTE V34Fax_Carrier_Detect(V34Struct *pV34);
//void V34Fax_PCR_Detect_Silence(V34Struct *pV34);
void V34Fax_PCR_Detect_S(V34Struct *pV34);
void V34Fax_PCR_Detect_S_bar(V34Struct *pV34);
void V34Fax_PCR_S_Bar_Train(V34Struct *pV34);
void V34Fax_PCR_PP_EQ_Train(V34Struct *pV34);
void V34Fax_PCR_VA_Delay(V34Struct *pV34);
void V34Fax_PCR_B1_Rx(V34Struct *pV34);
void V34Fax_PCR_Image_Rx(V34Struct *pV34);
void V34Fax_PCR_Rx_Setup(V34Struct *pV34);
void V34Fax_PCR_Receiver(V34Struct *pV34, SWORD qToIO);
void V34Fax_PCR_Decoder(V34Struct *pV34, CQWORD *pcSym);

/* V34_PCR_call.c */
void V34Fax_PCR_Send_B1(V34Struct *pV34);
void V34Fax_PCR_Transmitter(V34Struct *pV34);
void V34Fax_PCR_Silence_Tx(V34Struct *pV34);
void V34Fax_PCR_B1_SetUp(V34Struct *pV34);

/* V34_PCR_handler.c */
void V34Fax_PCR_Handler(UBYTE **pTable);

/* V34_PCR_init.c */
void V34Fax_PCR_Call_Init(UBYTE **pTable);
void V34Fax_PCR_Init(UBYTE **);
void V34Fax_HW_Setup(UBYTE **pTable);

/* V34_PCR_timing.c */
void V34Fax_Timing_Rec_Init(V34FaxStruct *pV34Fax);
void V34Fax_TimingLoop_Init(V34FaxStruct *pV34Fax);
void V34Fax_TimingLoop_Init_ForPP_and_Data(V34FaxStruct *pV34Fax);
void V34Fax_TimingLoop(V34FaxStruct *pV34Fax, CQWORD *cqUs, CQWORD *cqD_sym);
UBYTE V34Fax_Timing_Rec(V34FaxStruct *pV34Fax, CQWORD *pqUin, CQWORD *pcUout);

void V34F_Test_Retrain_State(UBYTE **pTable);

/* V34_MPET_const.c */
extern CONST QWORD V34Fax_PSF_Coeff[];
extern CONST QWORD V34Fax_1200COS[];
extern CONST QWORD V34Fax_1200SIN[];
extern CONST QWORD V34Fax_2400COS[];
extern CONST QWORD V34Fax_2400SIN[];
extern CONST CQWORD V34FAX_MAP_TAB[];
extern CONST QWORD V34Fax_Hilbert_Coeff[];
extern CONST QWORD V34Fax_HPF_Coeff[];
extern CONST QWORD V34Fax_LPF_Coeff[];
extern CONST UBYTE V34fax_2BitReversal[];
extern CONST UBYTE V34fax_4BitReversal[];
extern CONST QWORD qV34Fax_CallDetectToneCoefTab[];
extern CONST QWORD qV34Fax_AnsDetectToneCoefTab[];
extern CONST QWORD qV34Fax_CallDetectACtoneCoefTab[];
extern CONST QWORD qV34Fax_AnsDetectACtoneCoefTab[];

/* V34_MPET_fsm.c */
extern CONST V34FaxFnptr V34Fax_Call_Tx_Rerate_by_Remote_Init_VecTab[];
extern CONST QWORD V34Fax_Call_Tx_Rerate_by_Remote_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Call_Rx_Init_VecTab3[];
extern CONST QWORD V34Fax_Call_Rx_Init_CntTab3[];
extern CONST V34FaxFnptr V34Fax_Call_Tx_NoFirstBlock_Init_VecTab[];
extern CONST QWORD V34Fax_Call_Tx_NoFirstBlock_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Call_Rx_NoFirstBlock_Init_VecTab[];
extern CONST QWORD V34Fax_Call_Rx_NoFirstBlock_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Call_Tx_Init_VecTab[];
extern CONST QWORD V34Fax_Call_Tx_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Call_Rx_Init_VecTab[];
extern CONST QWORD V34Fax_Call_Rx_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Ans_Rx_Rerate_by_Local_Init_VecTab[];
extern CONST QWORD V34Fax_Ans_Rx_Rerate_by_Local_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Ans_Tx_NoFirstBlock_Init_VecTab[];
extern CONST QWORD V34Fax_Ans_Tx_NoFirstBlock_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Ans_Rx_NoFirstBlock_Init_VecTab[];
extern CONST QWORD V34Fax_Ans_Rx_NoFirstBlock_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Ans_Tx_Init_VecTab[];
extern CONST V34FaxFnptr V34Fax_MPET_Retrn_by_Local_Init_VecTab[];
extern CONST QWORD V34Fax_Ans_Tx_Init_CntTab[];
extern CONST QWORD V34Fax_MPET_Retrn_by_Local_Init_CntTab[];
extern CONST V34FaxFnptr V34Fax_Ans_Rx_Init_VecTab[];
extern CONST QWORD V34Fax_Ans_Rx_Init_CntTab[];

/* V34_PCET_fsm.c */
extern CONST V34FnPtr V34Fax_PCET_Tx_Fsm[];
extern CONST V34FnPtr V34Fax_PCET_Rx_Fsm[];

/* V34_PCR_fsm.c */
extern CONST V34FnPtr V34Fax_PCR_Tx_Fsm[];
extern CONST V34FnPtr V34Fax_PCR_Rx_Fsm[];

/* in other directory */
void ASCInit(UBYTE **pTable);

#endif

#endif
