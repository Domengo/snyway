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

#ifndef _V8STRU_H
#define _V8STRU_H

#include "v21stru.h"
#include "v8def.h"

#if SUPPORT_V92A
#include "v90astru.h"
#endif
typedef struct _V8_struct V8Struct;
typedef void (*V8Fnptr)(V8Struct *);

struct _V8_struct
{
    UBYTE isToSendMenu; // CM/JM
    V21Struct v21;

    QDWORD ANSam_Egy;

    CQDWORD Ansam_dft2085;
    CQDWORD Ansam_dft2100;
    CQDWORD Ansam_dft2115;

    UWORD curPhase_2085;
    UWORD curPhase_2100;
    UWORD curPhase_2115;
    SWORD ANSam_Tone_Count;

    SWORD pRecv_string1[V8_LEN_OF_STR + 2];
    SWORD pRecv_string2[V8_LEN_OF_STR + 2];
    //    SWORD pRecv_string3[V8_LEN_OF_STR + V8_LEN_OF_STR - 4];
    V8Fnptr pfTx[10];
    V8Fnptr pfRx[10];

    UBYTE CM_Octets[V8_CM_LENGTH2];
    UBYTE JM_Octets[V8_CM_LENGTH2];
    UBYTE ubCMLen;
    UBYTE ubJMLen;
#if SUPPORT_CI
    UBYTE CI_Octets[4]; // enough for 30 bits
#endif

    SWORD *pString_ptr;

    UWORD uPhase0;
    UWORD uPhase1;

    UBYTE ANS15_Counter;

    UWORD cur_bit_pattern;
    SWORD receive_state;
    SWORD bit_clock;
    SWORD ANSam_timer;

    UBYTE ubInternal_state;
    SWORD cur_send_bit_index;
    SWORD CJ_Return;
    UBYTE phase1_end;
    SBYTE tx_vec_idx;
    SBYTE rx_vec_idx;
    UBYTE JM_detected;

    UBYTE RxBits_Buffer[V8_V21_RXBUFSIZE];
    UBYTE RxBits_BufferIn_idx;
    UBYTE RxBits_BufferOut_idx;

    QFIRStruct V8AnsamToneH;
    QWORD pqDelaylineH[2 * V8_ANSAM_FILTER_LENGTH];    /* 2 * filter_length */

    UBYTE call_rate;
    SWORD result_code;
    UBYTE modem_mode;
    SBYTE hang_up;
    UBYTE hangup_reason;

    UBYTE V92_Enable;
    UBYTE V90_Enable;
    UBYTE V34_Duplex;
    UBYTE V32_Enable;
    UBYTE V22_Enable;
    UBYTE V21_Enable;

    UBYTE AnsAm_Detected;
    /* UBYTE CM_Detected; */

    SWORD DCE_timer;

    SWORD AnsamTime_Len;

    QWORD DeltaPhase_2100;
    QWORD DeltaPhase_15;
    QWORD DeltaPhase_2085;
    QWORD DeltaPhase_2115;

    QDWORD qdAnsam_AGC_Ref;

#if SUPPORT_CT
    QWORD DeltaPhase_1300;
    UWORD CTCurPhase;
#endif
#if SUPPORT_CT || SUPPORT_CI
    SWORD CtCiTimer;
    SBYTE CtCiState;
#endif

#if SUPPORT_V92D
    UBYTE LAPM;
    UBYTE WXYZ;
    SWORD Uqts;
    UBYTE QCA1d_Finish;
    UBYTE V8_QCA1d_Octets[V8_QCA1d_LENGTH];
    UBYTE V8_QCA1d_Length;
    UBYTE ANSpcm_Sigh;
    UWORD ANSpcm_Index;
    UWORD ANSpcm_Reverse_Count;
    CQDWORD TONEq_dft980;
    UWORD curPhase_980;
    UBYTE TONEq_Counter;
    UBYTE ANSpcm_Finish;
#endif

#if SUPPORT_V92A
    V90aDftStruct DFT;            /* DFT for QTS tones detection */

#if SUPPORT_V8BIS
    QWORD DeltaPhase_400;
    QWORD DeltaPhase_1375;
    QWORD DeltaPhase_2002;

    UBYTE V8_QC1a_Octets[V8_QC2A_LENGTH];
    UBYTE V8bisTimeout;
    UBYTE CRe_Count;
#else
    UBYTE V8_QC1a_Octets[V8_QC1A_LENGTH];
#endif

    UBYTE V8bisEnable;
    UBYTE QCEnabled;
    UBYTE ubQC1aLen;
    UBYTE QCA1d_detected;
    UDWORD QCA1d_ONEs_Sync;
    UBYTE QCA1d_P;
    UBYTE QCA1d_LM;
    UBYTE QCA1d_cnt;
    UBYTE TONEq_finish;
    UBYTE QTSbar_counter;
    UBYTE QTS_bar_detected;

    QDWORD drift;
    TimingRxStruct TIMRx;
    SWORD Rx_sword[12];
    QWORD savePcm[301];
    QWORD sumDiff[6];
#endif

#if SUPPORT_V34FAX
    UBYTE V34fax_Enable;
    UWORD ANSam_TimeOut_Count;
#endif

#if DRAWEPG
    UWORD epg_pos;
#endif

#if V8_HIGHPASS_FILTER_ENABLE
    IIR_Cas5Struct Hpf;
    QWORD  HpfDline[2*(4+1)];
#endif

    SWORD CJ_timeout_Count;
};

#endif
