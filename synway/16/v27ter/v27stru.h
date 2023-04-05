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

#ifndef _V27STRU_H
#define _V27STRU_H

#include "v27def.h"
#include "dspdstru.h"

typedef struct _V27_struct V27Struct;
typedef void (*V27Fnptr)(V27Struct *);

struct _V27_struct
{
    /* pointer to host-allocated memory */
    UBYTE **pTable;

    SWORD *PCMinPtr;
    SWORD *PCMoutPtr;

    UBYTE ImageSendMode;

    UBYTE isTxDataMode;

    V27Fnptr pfTxVecptr[V27_TX_FSM_SIZE];     /* FSM for 'TxVec'               */
    V27Fnptr pfRxVecptr[V27_RX_FSM_SIZE];     /* FSM for 'RxVec'               */

    SBYTE nTxState;            /* index for FSM                 */
    SBYTE nRxState;            /* index for FSM                 */

    SWORD nTxDelayCnt;         /* delay counter for 'TxVec'     */
    SWORD nRxDelayCnt;         /* delay counter for 'RxVec'     */

    UBYTE ubRateSig;
    UBYTE ubBitsPerSym;
    UBYTE ubSymBufSize;
    UBYTE ubNumSym;

    /* transmit vectors (pointer to functions) */
    V27Fnptr pfDataInVec;     /* input data */
    V27Fnptr pfScramVec;      /* scrambling */
    V27Fnptr pfEncodeVec;     /* encoding */

    /* receive vectors (pointer to functions) */
    V27Fnptr pfDetectVec;      /* detection */
    V27Fnptr pfTimingVec;      /* timing loop but here just to get values from demodulator */
    V27Fnptr pfEqVec;          /* equalizer */
    V27Fnptr pfRotateVec;      /* rotater */
    V27Fnptr pfTrainScramVec;  /* training scrambler */
    V27Fnptr pfTrainEncodeVec; /* training encoder */
    V27Fnptr pfSliceVec;       /* slicing */
    V27Fnptr pfTimUpdateVec;   /* Updating Timing */
    V27Fnptr pfDerotateVec;    /* de-rotater */
    V27Fnptr pfCarrierVec;     /* carrier recovery */
    V27Fnptr pfEqUpdateVec;    /* equalizer update */
    V27Fnptr pfDecodeVec;      /* Viterbi decoding */
    V27Fnptr pfDescramVec;     /* descrambling */
    V27Fnptr pfDataOutVec;     /* output data */

    /*************** TX variables ****************/
    /* scrambler */
    UDWORD udScramSReg;
    UBYTE  ubScramInvt;
    UBYTE  ubScramRptCount;
    UBYTE  ubScramInbits;
    UBYTE  ubScramOutbits;

    /* Encoder */
    SBYTE  sbOld_phase;

    /* signal mapper */
    CQWORD cqSigMapIQ;

    /* pulse-shape filter */
    QIFIRStruct pPsfI;
    QIFIRStruct pPsfQ;

    QWORD qPsfDlineI[(V27_FILTER_LENGTH/V27_SYM_SIZE_1600) << 1];
    QWORD qPsfDlineQ[(V27_FILTER_LENGTH/V27_SYM_SIZE_1600) << 1];

    QWORD pqPsfOutIptr[V27_SYM_SIZE_1200];
    QWORD pqPsfOutQptr[V27_SYM_SIZE_1200];

    /* modulation */
    UWORD  ModPhase;

    QWORD  qSagcScale;
    QWORD  qdSagcEnergy;

    UBYTE  ubSagc_Flag;

    /* demodulation */
    UWORD    DemodPhase;

    QWORD    qDemodIn[V27_SYM_SIZE_1200];

    CQWORD   cDemodIQ_Prev[V27_SYM_SIZE_1200];
    CQWORD   cDemodIQBuf[V27_SYM_SIZE_1200];

    QFIRStruct   pHilbFIR;
    QWORD    qHilbDelay[2*HILB_TAP_LEN];

    /* correlation */
    QDWORD   qdCorrelation;
    UBYTE    ubCorrIdx;
    CQWORD   cqCorrDline[V27_CORR_NUM_SAMPLES_1200];

    /* timing recovery */
    UBYTE   ubTimingPhase;

    CQWORD  pcTimingDline[2*V27_TIMING_DELAY_HALF];
    UBYTE   ubOffset;

    PolyFilterStruct Poly;

    /* equalizer */
    QCFIRStruct cfirEq;
    CQWORD cEqCoef[V27_EQ_LENGTH];
    CQWORD pcEqDelay[2*V27_EQ_LENGTH];
    CQWORD cqEqOutIQ;

    /* equalizer update */
    QWORD qEqBeta;
    /* de-rotater */
    CQWORD cqEqErrorIQ;

    /* rotater */
    QWORD qRotateCos;
    QWORD qRotateSin;

    UWORD uRotatePhase;

    CQWORD cqRotateIQ;

    UDWORD TrainScramSReg;
    UBYTE  ubTrainScramOutbits;

    /* for encoder */
    SBYTE sbTrainold_phase ;

    CQWORD cqTrainIQ;

    /* slicer */
    UBYTE ubSigDemapSize;

    CQWORD *pcSigDemapptr;
    CQWORD cqSliceIQ;

    /* carrier recovery */
    QWORD qCarErrPhase;

    /* timing update */
    QWORD qTimingErr1;
    QWORD qTimingErr2;

    CQWORD cqTimingEqOutBuf[3];
    CQWORD cqTimingSliceBuf[2];

    UBYTE  ubTimErr1Freeze;
    UBYTE  ubTimShift;

    SBYTE sbPast_phase;

    /* descrambler */
    UDWORD DescramSReg;
    UBYTE  ubDescramInvt;
    UBYTE  ubDescramRptCount;
    UBYTE  ubDescramInbits;
    UBYTE  ubDescramOutbits;

    /* data output */
    DftStruct Dft;
    QDWORD    qdMagSq[V27_TONE_NUMBER];
    QWORD     qQ1Q2[V27_TONE_NUMBER << 1];

    SWORD Timer;
    SWORD sTimeOutCounter;

    QDWORD qdCarrLoss_egy;
    UBYTE  CarrLoss_Flag;
    QDWORD qdCarrLossEgy_Ref;
    UWORD  uCarrLoss_Count;

    UBYTE  ubGuardToneCnt;
    UBYTE  ubABtoneCnt;

    UBYTE  V21DetectNumber;
    UWORD  V21DetectDelayCnt;
    UBYTE  SilenceCount;

#if V27_SHORT_TRAIN_SUPPORT
    UBYTE Short_Trn_Flag;
#endif

#if SUPPORT_ENERGY_REPORT
    UBYTE Detect_FaxTone;
#endif
};
#endif
