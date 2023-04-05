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

#ifndef _V34EXT_H
#define _V34EXT_H

#include "shareext.h"
#include "v34stru.h"

void  V34_Agc_Init(V34AgcStruc *pAGC);
void  V34_Agc(V34AgcStruc *, CQWORD *);
void  V34_Descram_Byte_GPC(UBYTE *, UDWORD *, UBYTE *, UBYTE);
void  V34_Descram_Bit_GPC(UBYTE *, UDWORD *, UBYTE *, UBYTE);
void  V34_Create_E(V34Struct *);
void  V34_Echo_Init(V34Struct *);
void  V34_Echo_Canceller(V34Struct *pV34);
void  V34_Test_Retrain_State(UBYTE **);
void  V34_Check_Retrain_Tone(RetrainStruc *p);
void  V34_Check_Rtrn_Req(ReceiveStruc *pRx);
void  V34_Phase4_Renego_Init(V34Struct *pV34);
void  V34_Dummy(V34Struct *pV34);
void  V34_Clrdwn_Det_S(V34Struct *pV34);
void  V34_Clrdwn_Training(V34Struct *pV34);
void  V34_Clrdwn_Det_MP(V34Struct *pV34);
void  V34_Clrdwn_Det_MP_PI(V34Struct *pV34);
void  V34_Clrdwn_wait(V34Struct *pV34);
void  V34_Finish_Clear_Down(V34Struct *pV34);
void  V34_Rate_Renego_Init(V34Struct *pV34);
void  V34_2T3_Equalizer_Update(CQWORD cqErr, QWORD qBeta, TimeCarrRecovStruc *pTCR);
void  V34_Cleanup_Equalizer(QCFIRStruct *eqfir);

#if RX_PRECODE_ENABLE
void  V34_NoiseWhiten_Init(NoiseWhitenStruct *pNW);
void  V34_NoiseWhiten(NoiseWhitenStruct *pNW, CQWORD *pcErr);
#endif

void  V34_Create_Mp(UBYTE *pInfo_buf, MpStruc *pMp);
void  V34_Detect_Mp_Init(MpStruc *);
void  V34_Non_Linear_Decoder(QDWORD nl_scale, CQWORD *pcX_signal);
void  V34_Phase3_Send_J(V34Struct *);
void  V34_Phase3_Send_PP_Call(V34Struct *);
void  V34_Phase3_Send_PP(V34Struct *);
void  V34_Phase3_Send_S(V34Struct *);
void  V34_Phase3_Send_S_Bar(V34Struct *);
void  V34_Phase3_Send_TRN(V34Struct *);
void  V34_Phase3_Send_Silence(V34Struct *);
void  V34_Phase3_Send_Silence_70MS(V34Struct *);
void  V34_Phase3_Mod_Silence(V34Struct *);

void  V34_Phase3_Det_S(V34Struct *);
void  V34_Det_S_bar_Sub(ReceiveStruc *pRx);
void  V34_Phase3_Rec_S_Det_S_Bar(V34Struct *);
void  V34_Phase3_Wait_S_Bar_16T(V34Struct *);
void  V34_Phase3_EQ_1(V34Struct *);
void  V34_Phase3_Rec_PP_EQ(V34Struct *);
void  V34_Phase3_Rec_TRN_EQ(V34Struct *);
void  V34_Phase3_Rec_TRN_EQ_TC(V34Struct *);
void  V34_Phase3_Det_J(V34Struct *);
void  V34_Phase3_Det_S_Call(V34Struct *);
void  V34_Phase3_Dummy_Call(V34Struct *);
void  V34_Phase3_Rx_Dummy(V34Struct *);
void  V34_Phase3_Rec_J_Dummy(V34Struct *);
void  V34_Phase3_Rec_J_Setup(V34Struct *);
void  V34_Phase3_Wait_MD(V34Struct *);

void  V34_Phase4_Send_S(V34Struct *);
void  V34_Phase4_Send_S_Bar(V34Struct *);
void  V34_Phase4_Send_J_Bar(V34Struct *);
void  V34_Phase4_Send_TRN(V34Struct *);
void  V34_Phase4_Send_TRN_2000MS(V34Struct *);
void  V34_Phase4_Send_MP(V34Struct *);
void  V34_Phase4_Finish_MP(V34Struct *);
void  V34_Phase4_Finish_MP_Pi(V34Struct *);
void  V34_Phase4_Flush_MP_Pi(V34Struct *);
void  V34_Phase4_Prepare_E(V34Struct *);
void  V34_Phase4_Send_E(V34Struct *);
void  V34_Phase4_Send_B1_Frame(V34Struct *);
void  V34_Begin_Data_Transfer(V34Struct *);

/* Phase4: modem receive vectors */
void  V34_Phase4_Det_S_Bar(V34Struct *);
void  V34_Phase4_Wait_S_Bar_16T(V34Struct *);
void  V34_Phase4_EQ_1(V34Struct *);
void  V34_Phase4_Det_TRN(V34Struct *);
void  V34_Phase4_Det_TRN_TC(V34Struct *);
void  V34_Phase4_Rec_MP(V34Struct *);
void  V34_Phase4_Rec_MP_Pi(V34Struct *);
void  V34_Phase4_Detect_E(V34Struct *);
void  V34_Phase4_Rec_B1_Frame(V34Struct *);
void  V34_Phase4_VA_Delay(V34Struct *);
void  V34_Phase4_Rec_Superframe(V34Struct *);
void  V34_Phase4_Det_J_Bar(V34Struct *);

/* Data mode Tx and Rx vectors */
void  V34_Transmiter(V34Struct *);
void  V34_Receiver(ReceiveStruc *pRx, UBYTE **pTable, SWORD qToIO);

void  V34_Phase3_Ec_Dummy(V34Struct *pV34);

void  Phase3_Init(V34Struct *);
void  Phase4_Init(V34Struct *);
void  V34_Phase4_Init(UBYTE **);

void  V34_Demodulate_Init(DeModulateStruc *pDeMod);
void  V34_Demodulate(ReceiveStruc *pRx, CQWORD *pcOut);
void  V34_S_Init(V34Struct *);

void  V34_Para_Rx_Init(RX_ModemDataStruc *RxMod, ModemData *pM, UBYTE M);
void  V34_Parameter_Function_Init_Tx(V34Struct *pV34);
void  V34_Parameter_Function_Init_Rx(V34Struct *pV34);
void  V34_Carrier_Init(V34Struct *pV34);
void  V34_Send_Mapping_Frame(V34Struct *pV34);
void  V34_Init(V34Struct *pV34);
void  V34_RXShell_Mapper_Init(RX_ShellMapStruct *);

void  V34_VA_Decode(DeCodeStruc *DeCode, CQWORD *out9_7);
void  V34_VA_Init(VA_INFO *dec);
void  V34_Retrieve_Infobits(ReceiveStruc *pRx, SWORD High_Map_Frame);

#if SUPPORT_V54
void  V34_Digital_Loop_Back_Init(UBYTE **pTable);
void  V34_LAL_Codec_Setup(UBYTE **pTable);
void  V34_LAL_Startup(UBYTE **pTable);
void  V34_LAL_Det_S(V34Struct *pV34);
void  V34_LAL_Det_S_Bar(V34Struct *pV34);
void  V34_LAL_Look_For_E(V34Struct *pV34);
#endif

void  V34_Demapper(CQWORD cU, SWORD *pRx_Q, SWORD *pRx_Z);
void  V34_State16_D2Min(CQWORD *, CQWORD *);
void  V34_State16_D4Min(iD4SYMBOL *, iD4SYMBOL *, UBYTE *);
void  V34_Decoder_Init(DeCodeStruc *);
void  V34_Precode_Sub(ReceiveStruc *pRx, CQWORD *pcYout);
void  V34_Decoder(ReceiveStruc *pRx, CQWORD *pcS);
void  V34_4Point_Slicer(RenegoStruc *pRN, CQWORD *pcSym);
void  V34_TimingLoop_Init(TimeCarrRecovStruc *pTCR);
void  V34_TimingLoop(TimeCarrRecovStruc *pTCR, CQWORD *cqUs, CQWORD *cqD_sym);
void  V34_Shift_Buf(CQWORD *pcBuf, UBYTE ubBuf_len, CQWORD cqNewIn);
void  V34_Timing_Rec_Init(TimeCarrRecovStruc *pTCR);
void  V34_Timing_Rec(TimeCarrRecovStruc *, CQWORD *pcUin, CQWORD *pcUout);
void  V34_CarrierLoop_Init(TimeCarrRecovStruc *pTCR);
void  V34_CarrierLoop(TimeCarrRecovStruc *pTCR, CQWORD *pcUin, CQWORD *pcUout);
void  V34_S_Detector_Init(S_Tone_DetStruc *SToneDet);
void  V34_ARS_Calc(ReceiveStruc *pRx, CQWORD *cqErrsym);
void  V34_ARS_est(V34Struct *);
void  V34_HW_Setup(UBYTE **pTable);
void  V34_DPCS_Data(UBYTE **pTable);
void  V34_DPCS_Phase3(UBYTE **pTable);
void  V34_DPCS_Phase4(UBYTE **pTable);
void  V34_Init_Sync_Detector(VA_INFO *dec);
void  V34_Disconnect(UBYTE **);
void  V34_Phase3_Handler(UBYTE **pTable);
void  V34_Phase4_Handler(UBYTE **pTable);
void  V34_Data_Handler(UBYTE **pTable);
void  V34_LAL_Handler(UBYTE **pTable);
void  V34_DFT_Init(DFT_Info *pDFT, QWORD *pqQ1_buf, QWORD *pqQ2_buf, QWORD *pqCoef, QWORD *pqMag_sqr, SWORD N, UBYTE Num_Freq);
void  V34_Phase4_ReceiverSetup(V34Struct *pV34);
void  V34_Pre_Data_Share(V34Struct *pV34);

#if PHASEJITTER
void  V34_jt_loop_init(JTstruct *pJT, UBYTE reset);
void  V34_jt_loop(JTstruct *pJT, CQWORD *pcUs, CQWORD *pcUt, UBYTE adapt);
void  V34_jt_decision(JTstruct *jt);
#endif

SBYTE  V34_ARS_est_Sub(V34RxStruct *pV34Rx);
UBYTE  V34_Detect_S_SBAR(ReceiveStruc *pRx, CQWORD *, SWORD *);
SBYTE  V34_Detect_Mp(UBYTE bit, MpStruc *pMp) ;
CQWORD V34_Create_J(V34Struct *pV34);
UBYTE  V34_Detect_J_Sequence(UBYTE in_bit, V34Struct *pV34);
UBYTE  V34_Symbol_To_Bits(CQWORD cqC, CQWORD *pcOutsym, UWORD constel_size);
UDWORD V34_De_ShellMapper(UBYTE m[4][2], UWORD M, RX_ShellMapStruct *);
CQWORD V34_Slicer(CQWORD *);
CQWORD V34_Rotator(CQWORD *pcInput, QWORD qS, QWORD qC);
UBYTE  V34_S_Detector(ReceiveStruc *pRx, CQWORD *pcTimingOut);
UBYTE  V34_DFT(DFT_Info *pDFT, SWORD sample);

extern CONST QWORD S_HLK_TAB[6];
extern CONST QWORD PP_HLK_TAB[6];
extern CONST QWORD TRN_HLK_TAB[6];
extern CONST QWORD AGC_HLK_TAB[6];

extern CONST UBYTE V34_tS16D4set_ptr[8][4];
extern CONST UBYTE V34_tTrellis16[128];
extern CONST UBYTE V34_tCarrier_tbl[6][2];
extern CONST QWORD V34_tCarrier_offset[6][2];
extern CONST Retrain_Tone V34_tRetrain_Tone_Table[6][2];
extern CONST QWORD V34_tRTD_FACTOR[6];
extern CONST QWORD V34_tEQ_ARS_THRESHOLD[6][14];
extern CONST QWORD V34_qARS_THRESHOLD_SCALE[14];
extern CONST QWORD V34_tSys_Delay[6];
extern CONST UBYTE V34bis_tBit_Rate[6];
extern CONST UBYTE V34_tBit_Rate[6];
extern CONST QWORD V34_tRTD_Sym[6];
extern CONST UBYTE V34_tSym_Num[6];
extern CONST QWORD V34_tCarrier_offset_factor[6];

#endif
