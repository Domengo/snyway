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

#ifndef _V29STRU_H
#define _V29STRU_H

#include "v29def.h"
#include "dspdstru.h"

typedef struct _V29_struct V29Struct;
typedef void (*V29Fnptr)(V29Struct *);

struct _V29_struct
{
    /* pointer to host-allocated memory */
    UBYTE **pTable;

    QWORD *PCMinPtr;
    QWORD *PCMoutPtr;

    UBYTE ImageSendMode;

    UBYTE isTxDataMode;

    V29Fnptr pfTxVecptr[V29_TX_FSM_SIZE];     /* FSM for 'TxVec'               */
    V29Fnptr pfRxVecptr[V29_RX_FSM_SIZE];     /* FSM for 'RxVec'               */

    SBYTE    nTxState;           /* index for FSM                 */
    SBYTE    nRxState;           /* index for FSM                 */

    SWORD    nTxDelayCnt;        /* delay counter for 'TxVec'     */
    SWORD    nRxDelayCnt;        /* delay counter for 'RxVec'     */

    UBYTE    ubRateSig;
    UBYTE    ubBitsPerSym;

    /* transmit vectors (pointer to functions) */
    V29Fnptr pfDataInVec;     /* input data */
    V29Fnptr pfScramVec;      /* scrambling */
    V29Fnptr pfEncodeVec;     /* encoding */

    /* receive vectors (pointer to functions) */
    V29Fnptr pfDetectVec;      /* detection */
    V29Fnptr pfTimingVec;      /* timing loop but here just to get values from demodulator */
    V29Fnptr pfEqVec;          /* equalizer */
    V29Fnptr pfRotateVec;      /* rotater */
    V29Fnptr pfTrainScramVec;  /* training scrambler */
    V29Fnptr pfTrainEncodeVec; /* training encoder */
    V29Fnptr pfSliceVec;       /* slicing */
    V29Fnptr pfTimUpdateVec;   /* Timing Recovery */
    V29Fnptr pfDerotateVec;    /* de-rotater */
    V29Fnptr pfCarrierVec;     /* carrier recovery */
    V29Fnptr pfEqUpdateVec;    /* equalizer update */
    V29Fnptr pfDecodeVec;      /* Viterbi decoding */
    V29Fnptr pfDescramVec;     /* descrambling */
    V29Fnptr pfDataOutVec;     /* output data */

    /* -------------- TX variables -------------- */
    /* scrambler */
    UDWORD   udScramSReg;
    UBYTE    ubScramInbits;
    UBYTE    ubScramOutbits;

    /* Encoder */
    SBYTE    sbOld_phase;

    /* signal mapper */
    CQWORD   cqSigMapIQ;

    /* pulse-shape filter */
    QIFIRStruct  pPsfI;
    QIFIRStruct  pPsfQ;

    QWORD    qPsfDlineI[V32V29_PSF_DELAY_LEN << 1];
    QWORD    qPsfDlineQ[V32V29_PSF_DELAY_LEN << 1];

    QWORD    pqPsfOutIptr[V29_SYM_SIZE];
    QWORD    pqPsfOutQptr[V29_SYM_SIZE];

    /* modulation */
    UWORD    ModPhase;

    QWORD    qSagcScale;

    /* demodulation */
    QWORD   qDemodIn[V29_SYM_SIZE];
    UWORD    DemodPhase;

    CQWORD  cDemodIQBuf[V29_SYM_SIZE];

    QFIRStruct  pHilbFIR;
    QWORD   qHilbDelay[2*HILB_TAP_LEN];

    /* detection or  correlation */
    QDWORD qdCorrelationI;
    QDWORD qdCorrelationQ;
    QDWORD qdCorrThres;
    QWORD  qCorrDline[V29_CORR_NUM_SAMPLES];
    UBYTE  ubCorrelation_Flag;
    UBYTE  ubCorrIdx;
    UBYTE  ubCorr_Sum_Counter;

    /* timing loop for timing recovery */
    UBYTE    ubTimingPhase;

    CQWORD   pcTimingDline[2*V29_TIMING_DELAY_HALF];
    UBYTE    ubOffset;

    PolyFilterStruct Poly;

    /* equalizer */
    QCFIRStruct   cfirEq;
    CQWORD cEqCoef[V29_EQ_LENGTH];
    CQWORD cEqDline[4*V29_EQ_LENGTH];
    CQWORD cqEqOutIQ;

    /* rotater */
    QWORD    qRotateCos;
    QWORD    qRotateSin;

    UWORD    uRotatePhase;

    CQWORD   cqRotateIQ;

    /* training scrambler */
    UWORD    TrainScramSReg;
    UBYTE    ubTrainScramOutbits;

    /* training encoder */
    CQWORD   cqTrainIQ;

    /* slicer */
    UBYTE    ubSigDemapSize;

    CQWORD *pcSigDemapptr;
    CQWORD   cqSliceIQ;

    /* de-rotater */
    CQWORD   cqEqErrorIQ;

    /* Carrier Recovery */
    SDWORD    qdCarErrPhase;
    QWORD    qCarAcoef;
    QWORD    qCarBcoef;

    /* Timing Recovery */
    QWORD    qTimingErr1;
    QWORD    qTimingErr2;

    CQWORD   cqTimingEqOutBuf[3];
    CQWORD   cqTimingSliceBuf[2];

    UBYTE  ubTimErr1Freeze;
    UBYTE  ubTimShift;

    /* equalizer update */
    SWORD    qEqBeta;

    /* Decoder for V29 */
    UBYTE    ubDecodeOut;
    SBYTE    sbPast_phase;

    /* descrambler */
    UDWORD   DescramSReg;
    UBYTE    ubDescramInbits;
    UBYTE    ubDescramOutbits;

    DftStruct Dft;
    QDWORD qdMagSq[V29_TONE_NUMBER];
    QWORD  qQ1Q2[V29_TONE_NUMBER << 1];

    QDWORD qdCarrLoss_egy;
    UBYTE  CarrLoss_Flag;
    QDWORD qdCarrLossEgy_Ref;
    UWORD  uCarrLoss_Count;

    UBYTE  ubGuardToneCnt;
    UBYTE  ubABtoneCnt;

    QDWORD qdSagcEnergy;
    UBYTE  ubSagc_Flag;

    SWORD  sTimeOutCounter;
    UBYTE  ubTraintype;

    UBYTE  V21DetectNumber;
    UWORD  V21DetectDelayCnt;
    UBYTE  SilenceCount;

#if SUPPORT_ENERGY_REPORT
    UBYTE  Detect_FaxTone;
#endif
};

#endif
