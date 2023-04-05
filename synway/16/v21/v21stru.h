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

#ifndef _V21STRU_H
#define _V21STRU_H

#include "v21def.h"
#include "dspdstru.h"

typedef struct _V21FskStruct V21Struct;
typedef void (*V21Fnptr)(V21Struct *);

struct _V21FskStruct
{
    UBYTE **pTable;

    SWORD *PCMinPtr;
    SWORD *PCMoutPtr;

    V21Fnptr pfTxVec;
    V21Fnptr pfRxVec;

    UBYTE modem_mode;
    UBYTE DetCounter;

    /* Transmitter variables */
    UBYTE TxBits;
    UWORD Phase;

    /* Receiver variables */
    UBYTE RxNumBits;
    UWORD Count_Tx;

    UBYTE ubCounter;

    SBYTE sbPdelIdx;

    QWORD qAgc_gain;

    UWORD Silence_samples;

    UBYTE Detect_tone;
    UBYTE State;

#if V21_7200
    IIR_Cas5Struct BpfToneIIR_72;
    QWORD pqDelaylineT_72[(V21_BIQUAD_NUM + 1)*2];
#endif

    QFIRStruct BpfHigh;
    QFIRStruct BpfLow;
    QFIRStruct LpFilter;
#if V21_9600
    QWORD pqDelaylineH[2 * V21_FILTER_LENGTH_96];   /* 2 * filter_length */
    QWORD pqDelaylineL[2 * V21_FILTER_LENGTH_96];   /* 2 * filter_length */
    QWORD  pqDelayline[2 * V21_LOWPASS_FILTER_LENGTH_96];

    QWORD pqPdel[V21CH1_PDEL_DELAY_LENGTH_96];/* The larger of V21CH1_PDEL_DELAY_LENGTH and V21CH2_PDEL_DELAY_LENGTH */

    QFIRStruct BpfToneIIR_96;
    QWORD pqDelaylineT_96[2 * V21_FILTER_LENGTH_96];
#else
    QWORD pqDelaylineH[2 * V21_FILTER_LENGTH_72];   /* 2 * filter_length */
    QWORD pqDelaylineL[2 * V21_FILTER_LENGTH_72];   /* 2 * filter_length */
    QWORD  pqDelayline[2 * V21_LOWPASS_FILTER_LENGTH_72];

    QWORD pqPdel[V21CH1_PDEL_DELAY_LENGTH_72];/* The larger of V21CH1_PDEL_DELAY_LENGTH and V21CH2_PDEL_DELAY_LENGTH */
#endif

    UBYTE  pOutBits[16];

    QFIRStruct *BpfTxPtr;

    UBYTE Disconnect_flag;

    UBYTE TxMode;
    UBYTE RxMode;

    UWORD Ch2TimeOutCount;
    UBYTE ubCh2DiscCount;

    UBYTE  Carrier_Flag;
    QDWORD qdCarrierThreshold;

    UBYTE ubSampleRate;
    UBYTE ubBufferSize;
    UBYTE ubMaxSilenceRef;
    UBYTE ubCh1_PDEL_Delay_Len;
    UBYTE ubCh2_PDEL_Delay_Len;

    QWORD qCh1_Modem_Beta;
    QWORD qCh2_Modem_Beta;

    QWORD qCh1_Mark_Delta_Phase;
    QWORD qCh1_Space_Delta_Phase;

    QWORD qCh2_Mark_Delta_Phase;
    QWORD qCh2_Space_Delta_Phase;

    QWORD qEnergy_Ref;
    QWORD qCarrierLoss_Thres;

    QDWORD qdModem_AGC_Ref;
    QDWORD qdCh2Fax_AGC_Ref;

#if SUPPORT_ENERGY_REPORT
    UBYTE  Detect_FaxTone;
    UWORD  uFlagPattern;
    QDWORD qdEnergyAcc;
    QDWORD qdEnergyMax;
    QDWORD qdEnergyMin;
    UWORD  uEnergyCount;
#endif
};

#endif
