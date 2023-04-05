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

#ifndef _V3217STR_H
#define _V3217STR_H

#include "dspdstru.h"
#include "v3217def.h"

typedef struct _V32share_struct V32ShareStruct;
typedef void (*V32ShareFnPtr)(V32ShareStruct *);

struct _V32share_struct
{
    SWORD *PCMinPtr;
    QWORD *PCMoutPtr;

    SWORD  *nTxDelayCntTab;             /* table of delay counter */
    SWORD  *nRxDelayCntTab;

    SWORD  nTxDelayCnt;                 /* delay counter for 'TxVec'      */
    SWORD  nRxDelayCnt;

    SBYTE  nTxState;                    /* index for FSM  */
    SBYTE  nRxState;

    UBYTE  ubTxBitsPerSym;              /* 2, 3, 4, 5, 6 bits per symbol */
    UBYTE  ubRxBitsPerSym;

    /* transmit vectors (pointer to functions) */
    V32ShareFnPtr pfScramVec;       /* scrambling */
    V32ShareFnPtr pfDiffEncVec;     /* differential encoding */
    V32ShareFnPtr pfEncodeVec;      /* trellis encoding */
    V32ShareFnPtr pfSigMapVec;      /* signal mapping */

    /* receive vectors (pointer to functions) */
    V32ShareFnPtr pfEqVec;          /* equalizer */
    V32ShareFnPtr pfTrainScramVec;  /* training scrambler */
    V32ShareFnPtr pfTrainSigMapVec; /* training signal mapper */
    V32ShareFnPtr pfSliceVec;       /* slicing */
    V32ShareFnPtr pfTimUpdateVec;   /* timing update */
    V32ShareFnPtr pfDerotateVec;    /* de-rotater */
    V32ShareFnPtr pfCarrierVec;     /* carrier recovery */
    V32ShareFnPtr pfEqUpdateVec;    /* equalizer update */
    V32ShareFnPtr pfDecodeVec;      /* Viterbi decoding */
    V32ShareFnPtr pfDescramVec;     /* descrambling */

    V32ShareFnPtr pfDiffDecVec;     /* differential decoding */

    /* signal mapper */
    CONST CQWORD *pcSigMapTab;
    CQWORD        cqSigMapIQ;

    QWORD   qPsfOutBufI[V32_SYM_SIZE];
    QWORD   qPsfOutBufQ[V32_SYM_SIZE];

    UDWORD udTrainScramSReg;
    UBYTE  ubTrainScramOutbits;

    UBYTE  ubDescramOutbits;
    UBYTE ubDiffDecodeOut;

    UBYTE isRxDataMode;
    UBYTE isTxDataMode;

    SWORD sTimingIdxInit;
    QDWORD qdMinValue;
    QDWORD qdCarr_r;
    QDWORD qdCarr_i;

    /* descrambler */
    UDWORD udDescramSReg;

    UBYTE ubDescramLS;
    UBYTE ubDescramRS;
    UBYTE ubDescramUMASK;
    UBYTE ubDescramLMASK;

    CQWORD cqSliceIQ;
    CQWORD cqRotateIQ;

    UWORD  uRotatePhase;

    DftStruct Dft;
    QDWORD    qdMagSq[5];
    QWORD     qQ1Q2[10];

    UBYTE  ubTimErr1Freeze; /* also used by carrier loop */

    /* carrier recovery */
    QWORD qCarErrPhase;

    UBYTE  ubTimShift; /* also used by carrier loop */

    /* rotater */
    QWORD  qRotateCos;
    QWORD  qRotateSin;

    CQWORD cqTrainIQ;

    /* de-rotater */
    CQWORD cqEqErrorIQ;

    /* timing update */
    QWORD  qTimingErr1;
    QWORD  qTimingErr2;
    CQWORD cqTimingEqOutBuf[3];
    CQWORD cqTimingSliceBuf[2];

    PolyFilterStruct Poly;

    UBYTE  ubTrainEncodedBits;

    QCFIRStruct cfirEq_D;/* equalizer */
    QWORD       qEqBeta; /* equalizer update */

    CQWORD cEqCoef_D[V32_EQ_LENGTH];
    CQWORD cEqDline_D[V32_EQ_LENGTH << 2]; /* for 7200 sampling rate  V32_EQ_LENGTH*4, before for 9600 sampling rate it was *2 */
    CQWORD cqEqOutIQ;

#if V32_VITERBI
    UBYTE  ubVitWinIdx;
    QDWORD qdVitWinDist[V32_VIT_NUM_WIN << V32_VIT_STATES_SHIFT];
    UBYTE  ubVitWinPoint[V32_VIT_NUM_WIN << V32_VIT_STATES_SHIFT];
    UBYTE  ubVitWinPath[V32_VIT_NUM_WIN << V32_VIT_STATES_SHIFT];
    QDWORD qdVitPathDist[V32_VIT_ALLPATHS];
    UBYTE  ubVitPathPt[V32_VIT_ALLPATHS];
#endif

    /* Viterbi decoder */
    UBYTE  ubTrellisDecodeOut;        /* output of Trellis decoder, from Slicer */

    /* differential encoder */
    CONST UBYTE *pDiffEncTab;
    UBYTE        ubDiffEncState;

    /* Trellis encoder */
    CONST UBYTE *pTrellisEncTab;
    UBYTE        ubTrellisEncState;

    /* pulse-shape filter */
    QIFIRStruct PsfI;
    QIFIRStruct PsfQ;

    QWORD   qPsfDlineI[V32V29_PSF_DELAY_LEN << 1];
    QWORD   qPsfDlineQ[V32V29_PSF_DELAY_LEN << 1];

    /* Xmit power level control */
    QWORD   qXmitLevelAdjust;

    /* modulation */
    UWORD uModPhase;

    UWORD uDemodPhase;
    SWORD nDemodDeltaPhase;

    QFIRStruct firHilb;
    QWORD qHilbDline[V32_HILB_TAP_LEN << 1];

    /* timing loop */
    CQWORD cqTimingDline[V32_TIMING_DELAY_HALF << 1];
    UBYTE  ubOffset;

    /* slicer */
    UBYTE ubSigDemapSize;          /* max 128 */
    UBYTE ubSigTransMap;           /* what to translate to what ?? */
    CONST CQWORD *pcSigDemapTab;
    CONST SWORD  *pnSigRotmapTab;
    CONST UBYTE  *ubSigIQtoBitsTab;

    /* demodulation */
    QWORD  qSymEgy;
    QWORD  qDemodIn[V32_SYM_SIZE];
    CQWORD cDemodIQBuf[V32_SYM_SIZE];

    /* simple AGC */
    QWORD  qSagcScale;
    UBYTE  ubSagc_Flag;
    QDWORD qdSagcEnergy;

    /* differential decoder */
    CONST UBYTE *ubDiffDecodeTab;
    UBYTE ubDiffDecodeState;

    /* -------------- TX variables -------------- */
    /* scrambler */
    UDWORD udScramSReg;
    UWORD  uScramInbits;
    UWORD  uScramOutbits;

    UBYTE ubScramRS;
    UBYTE ubScramLS;
    UBYTE ubScramUMASK;
    UBYTE ubScramLMASK;

    UBYTE ubDiffEncOut;
    UBYTE ubTrellisEncOut;

#if (SUPPORT_V32 + SUPPORT_V32BIS)
    QCFIRStruct cfirEq_S;

    UBYTE  ubNumSSbarDetected;

    /* used for Carrier Loss Detection In Data Mode */
    QDWORD qdCarrLossErrAcc;      /* accumulate equalizer error */
#endif

#if SUPPORT_V17
    SBYTE  sbTimeJamCnt;

    UBYTE  ubTimeJamOK;   /* 1----TimeJam is obtained;  0 -----not yet */

    CQWORD cqAB_I;      /* initiaized to 0 */
    CQWORD cqAB_Q;      /* initialized to 0 */

    UBYTE  ubJam_sin_phase_idx;
    UBYTE  ubJam_cos_phase_idx;

    UBYTE  ubCnt;

    CQWORD cqNewEqOut[3];  /* For V17_EqSync function in short train */
    CQWORD cqOldEqOut[3];  /* For V17_EqSync function in short train */

#if V32_FREQUENCY_OFFSET
    SWORD nDemodDeltaOffset;
    SWORD nDemodDeltaCount;
    UBYTE ubRotateCountEnable;
    SDWORD sdRotateAcc;
    UWORD uRotateCount;
#endif
#endif
};

#endif
