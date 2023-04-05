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

#ifndef _V34FSTRU_H
#define _V34FSTRU_H

#include "aceext.h"
#include "v34fdef.h"

#if SUPPORT_V34FAX

typedef struct _V34FaxStruct V34FaxStruct;
typedef void (*V34FaxFnptr)(V34FaxStruct *);

typedef struct
{
    UBYTE pData_sequence_mp[188];
    UDWORD frame_sync_mp;

    UBYTE type;                  /* bit 18 */
    UBYTE max_data_rate;         /* bit 20-23, value 1-14. For V34 rate system */
    UBYTE control_channel_rate;  /* bit 27, value 0:1200bps,1:2400bps */

    UBYTE TrellisEncodSelect;    /* bit 29-30, value 0-2, value 3 ITU reserved */
    UBYTE NonlinearEncodParam;   /* bit 31 */
    UBYTE const_shap_select_bit; /* bit 32 */

    UWORD data_rate_cap_mask;    /* bit 35-49 */
    UBYTE asymmetric_control_rate; /* bit 50 */

    /*************************************************************************/
    /* starting from bit 52, Type 0 and Type 1 has different bit assignment  */
    /*************************************************************************/
    /* For MP sequence Type 1: */
    /* [0].r: bit 52-67; [0].i: bit 69-84; [1].r: bit 86-101; [1].i: bit 103-118; [2].r: bit 120-135; [2].i: bit 137-152 */
    CQWORD PrecodeCoeff_h[3];

    SWORD bit_num_cntr_mp;
    UWORD CRC_mp;

    UBYTE pre_max_data_rate;
} MphStruc;

struct _V34FaxStruct
{
    UBYTE **pTable;

    UBYTE modem_mode;

    UBYTE V34fax_EQ_Length;
    UBYTE V34FAX_EQ_Delay;

    UBYTE EQ_Mode;

    UBYTE FreqOffset_Enable;

    V34FaxFnptr pfTxSetUpVec;
    V34FaxFnptr *pfTxSetUpVecTab;
    SWORD *nTxDelayCntTab;
    SWORD nTxState;
    SWORD nTxDelayCnt;

    V34FaxFnptr pfRxSetUpVec;
    V34FaxFnptr *pfRxSetUpVecTab;
    SWORD *nRxDelayCntTab;
    SWORD nRxState;
    SWORD nRxDelayCnt;

    V34FaxFnptr pfDataInVec;
    V34FaxFnptr pfScramVec;
    V34FaxFnptr pfEncodeVec;
    V34FnPtr    pfDetectVec;
    V34FaxFnptr pfEqVec;
    V34FaxFnptr pfSliceVec;
    V34FaxFnptr pfTimUpdateVec;
    V34FaxFnptr pfDerotateVec;
    V34FaxFnptr pfCarrierVec;
    V34FaxFnptr pfEqUpdateVec;
    V34FaxFnptr pfDecodeVec;
    V34FaxFnptr pfDescramVec;
    V34FaxFnptr pfDataOutVec;

    UBYTE ubTxBitsPerSym;
    UBYTE ubRxBitsPerSym;

    UDWORD udScramSReg;
    UWORD uScramInbits;
    UWORD uScramOutBits;

    CQWORD IQ;
    UBYTE ubEncoderOldPhase;

    QIFIRStruct PsfI;
    QIFIRStruct PsfQ;
    QWORD qPsfDlineI[2*V34FAX_PSF_DELAY_LEN];
    QWORD qPsfDlineQ[2*V34FAX_PSF_DELAY_LEN];

    QWORD qPsfOutBufI[V34FAX_SYM_LEN];
    QWORD qPsfOutBufQ[V34FAX_SYM_LEN];

    CONST QWORD *ModCos;
    CONST QWORD *ModSin;
    CONST QWORD *DeModCos;
    CONST QWORD *DeModSin;

    UWORD demodPhase;
    UWORD demodPhaseIncrement;

    CQWORD cDemodIQBuf[V34FAX_SYM_LEN];

    IIR_Cas5Struct bqBpf;
    QWORD dBpfDline[2*(V34FAX_BIQUAD_NUM + 1)];

    QFIRStruct bpfx;
    QWORD dBpfDlineR[2*HILBERT_TAP_LEN];

    QWORD nTimIdx;
    QWORD  nTimError;
    QDWORD nTimE1;
    CQWORD cTimDline[2*V34FAX_TIMING_DELAY_HALF];
    UBYTE ubOffset;
    CQWORD cTimBufIQ[2];

    CQWORD cqEqOutIQ;
    QCFIRStruct cfirEq;
    CQWORD pcEqCoef[V34FAX_EQ_LEN];  /* Control Channel Coef */
    CQWORD pcEqCoefSave[V34FAX_EQ_LEN];  /* Control Channel Saved Coef */
    QDWORD syncPhaseError[V34FAX_SYM_LEN];
    QDWORD syncMagError[V34FAX_SYM_LEN];
    CQWORD pcEqDelay[2*V34FAX_EQ_LEN];
    QWORD  qEqBeta;
    CQWORD cqEqErrorIQ;

    QWORD qRotateCos;
    QWORD qRotateSin;
    UWORD uRotatePhase;
    CQWORD cqRotateIQ;
    CQWORD cqRotatePrevIQ;
    CQWORD cqSliceIQ;

    QWORD qCarErrPhase;

    UBYTE ubDecoderOldPhase;

    UDWORD udDescramSReg;
    UBYTE  ubDescramInbits;
    UBYTE  ubDescramOutbits;

    CQWORD pcRotateIQ[2];
    CQWORD pcSliceIQ[3];
    UWORD timing_adjust_flag;

    MphStruc MphTx;
    MphStruc MphRx;
    UBYTE pMPhByteBuf[24]; /* For MPh bit stream */
    UBYTE ubBitCounter;
    UWORD uByteCounter;
    UWORD uMPhSize;
    UBYTE ubMphDetect;
    UBYTE ubMphSent;

    UDWORD udEpattern;

    UBYTE ubPPh_k;
    UBYTE ubPPh_I;
    UBYTE ubPPh_kref;
    UBYTE ubPPh_Iref;

    UBYTE rx_byte;
    UWORD tx_sym_cnt;

    /* V.34 parameters */
    CQWORD pcCoef[V34FAX_EQ_MAX_LENGTH];  /* Primary Channel */
    QWORD qGain;
    UBYTE rx_carrier_idx;
    QWORD qRx_carrier_offset_idx;

    UBYTE HS_Data_flag;
    UBYTE E_detected;

    UBYTE power_reduction;
    UBYTE trn_length;
    UBYTE high_carrier_freq;
    UBYTE preemp_idx;
    UBYTE symbol_rate;
    UBYTE TRN_constellation_point;

    UBYTE Enter_CC_Order;

    UBYTE rx_bits_cnt;
    UBYTE rx_one_bit_cnt;
    UWORD rx_frame_cnt;

    UBYTE Disconnect_flag;

    UBYTE PCET_silence_cnt;
    UBYTE silence_cnt;

    CQWORD V34Fax_Delay_Line[V34FAX_DLINE_DLEN];

    CQWORD Proc_Timing_Buf[6];
    UBYTE  Proc_Timing_In;
    UBYTE  Proc_Timing_Out;
    UBYTE  Proc_Timing_Num;

#if V34FAX_DEBUG_ERROR
    UDWORD current_error;
    UWORD EQ_change_count;
#endif

    UBYTE Dline_feed_flag;
    UBYTE PPS_detect_flag;
    SWORD max_R;
    SWORD min_R;
    SWORD max_I;
    SWORD min_I;

    TimeCarrRecovStruc *pTCR;

    UWORD Ones_Rx_count;

    QDWORD qdMinPhaseErr;
    SWORD  bestPolyIdx;
    CQWORD cqNewEqOut[3];
    CQWORD cqOldEqOut[3];
    CQDWORD cqdDemodSTone[V34_SYNC_SAMPLES];
    SBYTE  sbSyncCounter;

    UBYTE  MPET_Retrn_AC_count;
    UBYTE  MPET_Retrn_Type;
    UBYTE  MPET_Retrning;

    UWORD  Timeout_thres;
    UWORD  timeout_count;

    UWORD  uTimingLoopThres;
    UWORD  uTimingLoopStep;

    SBYTE  drift_direction;
    UWORD  drift_thresh;
#if FREQUENCY_OFFSET
    QWORD  frequencyOffset;
#endif
    UWORD  timAdjustCount;
    SWORD  timAdjust;
    SWORD  carAdjust;
    UDWORD udCarPhase;

    DftStruct Dft;
    QDWORD    qdMagSq[V34FAX_PPH_SH_DETECT_NUMTONES];
    QWORD     qQ1Q2[V34FAX_PPH_SH_DETECT_NUMTONES << 1];

    DftStruct Dft_AC;
    QDWORD    qdMagSq_AC[3];
    QWORD     qQ1Q2_AC[6];

    UBYTE  PPhDetected;
    UBYTE  PPh_DetCount;
    UBYTE  ShDetected;
    UBYTE  SHDetect_Count;
    UBYTE  PPhDetect_Count;
    UBYTE  PPh_ShDetect_Count;
    UBYTE  PPhDetect_Only;

    UBYTE  ALT_DetCount;
    UBYTE  ONEs_DetCount;
    UBYTE  Ones_Rx_count0;

    UBYTE  EgyDetected;
    UBYTE  reRateFlag;
    QWORD  qSilenceEgy;

    UBYTE  Silence_Count1;
    UBYTE  Silence_Count2;
    QDWORD Silence_Egy_Ref;
    QDWORD Silence_Egy;
    UBYTE  Silence_Detect_Count;

    QDWORD qdS_Agy_Pre;
    QDWORD qdS_Agy_Diff;
    QDWORD qdS_Agy_Max;
    UBYTE  S_Detected;
    UBYTE  PCR_SilenceSkipCount;

    QDWORD qdNoise_Egy;
    QDWORD qdNoise_Egy_Ref;
    QDWORD qdCarrier_Egy;
    UBYTE  Sym_Count;

    UBYTE  PCET_TXend_Flag;
    UBYTE  PCR_TXend_Flag;
    UBYTE  PCR_RXend_Flag;

    UDWORD err_min_dist4;
    UDWORD err_min_dist16;

    UBYTE  AC_Detect_Start_Flag;
    UBYTE  V34Fax_PCR_Start_Flag;
};

#endif

#endif
