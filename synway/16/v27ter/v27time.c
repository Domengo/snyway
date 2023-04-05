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

/***********************************************************************/
/* Timing Loop:                                                        */
/*                                                                     */
/* This function has no use in V27 modem. This functions are included  */
/* only to provide TimingIQ array values to the Equalizer input. Only  */
/* Timing Dummy function is assigned in all vectors.                   */
/***********************************************************************/

#include "v27ext.h"

void V27_RX_Timing_Dummy(V27Struct *pV27)
{
    pV27->ubTimingPhase ^= 0x02;

    if (pV27->ubTimingPhase)
    {
        /* first point ( first sample) */
        pV27->Poly.cqTimingIQ[0].r = pV27->cDemodIQBuf[0].r;
        pV27->Poly.cqTimingIQ[0].i = pV27->cDemodIQBuf[0].i;

        /* second point ( third or fourth sample) */
        if (pV27->ubSymBufSize == V27_SYM_SIZE_1600)
        {
            pV27->Poly.cqTimingIQ[1].r = pV27->cDemodIQBuf[4].r;   /* the mid of 0 & 7 */
            pV27->Poly.cqTimingIQ[1].i = pV27->cDemodIQBuf[4].i;
        }
        else
        {
            pV27->Poly.cqTimingIQ[1].r = pV27->cDemodIQBuf[3].r;  /* the mid of 0 & 5 */
            pV27->Poly.cqTimingIQ[1].i = pV27->cDemodIQBuf[3].i;
        }
    }

    pV27->Poly.ubTiming_Found = pV27->ubTimingPhase;
}

/* ---------- Timing Loop ---------- */
void V27_TimingRecovery(V27Struct *pV27)
{
    RX_TimingRecovery(&pV27->Poly, pV27->ubSymBufSize, 2);
}

void V27_RX_TimingUpdate_init(V27Struct *pV27)
{
    UBYTE i;

    pV27->ubTimErr1Freeze = 0;
    pV27->qTimingErr2 = 0;

    for (i = 0; i < 3; i++)
    {
        pV27->cqTimingEqOutBuf[i].r = 0;
        pV27->cqTimingEqOutBuf[i].i = 0;
    }

    for (i = 0; i < 2; i++)
    {
        pV27->cqTimingSliceBuf[i].r = 0;
        pV27->cqTimingSliceBuf[i].i = 0;
    }
}

void  V27_RX_TimingUpdate_Train_fine(V27Struct *pV27)
{
    /* update timing with Training Symbol */
    V27_TimingUpdate(pV27, pV27->cqTrainIQ);
}

/*********************** TimingUpdate for data Mode *************/

void V27_RX_TimingUpdate(V27Struct *pV27)
{
    /* update timing with slicer output */
    V27_TimingUpdate(pV27, pV27->cqSliceIQ);
}

void V27_TimingUpdate(V27Struct *pV27, CQWORD cqTimeIQ)
{
    QDWORD qdTim_err;
    CQWORD *cqY, *cqD;
    QWORD  qTimingErr;

    /* pointers to delayed EQ, slicer samples */
    cqY = pV27->cqTimingEqOutBuf;
    cqD = pV27->cqTimingSliceBuf;

    /* delay EQ symbol */
    cqY[2] = cqY[1];
    cqY[1] = cqY[0];
    cqY[0] = pV27->cqRotateIQ;   /* Slicer input ( Rotator Output) */

    /* delay decision symbol */
    cqD[1] = cqD[0];
    cqD[0].r = cqTimeIQ.r;
    cqD[0].i = cqTimeIQ.i;

    /**************** CALCULATING TIMING ERROR *****************************/
    /*     Error = Re[B * D] + Im[A * C],                                  */
    /*                                                                     */
    /*     where:                                                          */
    /*         B = Re[cqD(k-1) - cqY(k-1)]    D = Re[cqY(k) - cqY(k-2)]    */
    /*         A = Im[cqD(k-1) - cqY(k-1)]    C = Im[cqY(k) - cqY(k-2)]    */
    /***********************************************************************/
    /* calculate the timing error */
    qdTim_err  = QQMULQD((cqD[1].r - cqY[1].r), (cqY[0].r - cqY[2].r));
    qdTim_err += QQMULQD((cqD[1].i - cqY[1].i), (cqY[0].i - cqY[2].i));
    qTimingErr = QDR15Q(qdTim_err);

#if 0

    if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = qTimingErr; }

#endif

    /* loop-filter, update timing */
    V27_TimingLoopFilter(pV27, qTimingErr);
}

/* -- loop-filter, update timing -- */
void V27_TimingLoopFilter(V27Struct *pV27, QWORD qTimingErr)
{
    QWORD qPLL_output1;
    QWORD qPLL_output2;

    /* find previous PLL outputs */
    qPLL_output1 = pV27->qTimingErr1;
    qPLL_output2 = pV27->qTimingErr2;

#if 0

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV27->ubTimErr1Freeze; }

    if (DumpTone6_Idx < 500000) { DumpTone6[DumpTone6_Idx++] = qPLL_output1; }

#endif

    if (pV27->ubTimErr1Freeze == 0)
    {
        qPLL_output1 += qTimingErr;
    }

    qPLL_output2 += ((qTimingErr >> 2) + (qPLL_output1 >> 11));

#if 0

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = qTimingErr; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = qPLL_output1; }

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = qPLL_output2; }

    if (DumpTone6_Idx < 500000) { DumpTone6[DumpTone6_Idx++] = pV27->ubTimErr1Freeze; }

#endif

    /* adjust the timing */
    if (qPLL_output2 > 100)
    {
        pV27->Poly.nTimingIdx += V27_TIME_FAI_STEP;
        qPLL_output2 = 0;
    }
    else if (qPLL_output2 < -100)
    {
        pV27->Poly.nTimingIdx -= V27_TIME_FAI_STEP;
        qPLL_output2 = 0;
    }

#if 0

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV27->ubTimErr1Freeze; }

    if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = pV27->Poly.nTimingIdx; }

#endif

    /* store new PLL outputs */
    pV27->qTimingErr1 = qPLL_output1;
    pV27->qTimingErr2 = qPLL_output2;
}
