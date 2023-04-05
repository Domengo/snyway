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

/*************************************************/
/* timing functions for receiver timing loop.    */
/*************************************************/

#include "v29ext.h"

void V29_RX_Timing_Dummy(V29Struct *pV29)
{
    UBYTE i;

    pV29->ubTimingPhase ^= 0x02;

    if (pV29->ubTimingPhase)
    {
        for (i = 0; i < 3; i++)
        {
            pV29->Poly.cqTimingIQ[i].r = pV29->cDemodIQBuf[i].r;
            pV29->Poly.cqTimingIQ[i].i = pV29->cDemodIQBuf[i].i;
        }
    }

    pV29->Poly.ubTiming_Found = pV29->ubTimingPhase;
}

/* ---------- Timing Loop ---------- */
void V29_TimingRecovery(V29Struct *pV29)
{
    RX_TimingRecovery(&pV29->Poly, V29_SYM_SIZE, 3);
}

void V29_TimingUpdate_init(V29Struct *pV29)
{
    UBYTE i;

    pV29->ubTimErr1Freeze = 0;
    pV29->qTimingErr2 = 0;

    for (i = 0; i < 3; i++)
    {
        pV29->cqTimingEqOutBuf[i].r = 0;
        pV29->cqTimingEqOutBuf[i].i = 0;
    }

    for (i = 0; i < 2; i++)
    {
        pV29->cqTimingSliceBuf[i].r = 0;
        pV29->cqTimingSliceBuf[i].i = 0;
    }
}

/* -- loop-filter, update timing -- */
void V29_TimingLoopFilter(V29Struct *pV29, QWORD qTimingErr)
{
    QWORD qPLL_output1;
    QWORD qPLL_output2;

    /* find previous PLL outputs */
    qPLL_output1 = pV29->qTimingErr1;
    qPLL_output2 = pV29->qTimingErr2;

#if 0

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV29->ubTimErr1Freeze; }

    if (DumpTone6_Idx < 500000) { DumpTone6[DumpTone6_Idx++] = qPLL_output1; }

#endif

    if (pV29->ubTimErr1Freeze == 0)
    {
        qPLL_output1 += qTimingErr;
    }

    qPLL_output2 += ((qTimingErr >> 0) + (qPLL_output1 >> 9));

#if 0

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = qTimingErr; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = qPLL_output1; }

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = qPLL_output2; }

    if (DumpTone6_Idx < 500000) { DumpTone6[DumpTone6_Idx++] = pV29->ubTimErr1Freeze; }

#endif

    /* adjust the timing */
    if (qPLL_output2 > 255)
    {
        pV29->Poly.nTimingIdx += V29_TIME_FAI_STEP;
        qPLL_output2 = 0;
    }
    else if (qPLL_output2 < -255)
    {
        pV29->Poly.nTimingIdx -= V29_TIME_FAI_STEP;
        qPLL_output2 = 0;
    }

#if 0

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV29->ubTimErr1Freeze; }

    if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = pV29->Poly.nTimingIdx; }

#endif

    /* store new PLL outputs */
    pV29->qTimingErr1 = qPLL_output1;
    pV29->qTimingErr2 = qPLL_output2;
}

void V29_RX_TimingUpdate(V29Struct *pV29)/* update timing with slicer output */
{
    QDWORD qdTim_err;
    CQWORD *cqY, *cqD;
    QWORD  qTimingErr;

    /* pointers to delayed EQ, slicer samples */
    cqY = pV29->cqTimingEqOutBuf;
    cqD = pV29->cqTimingSliceBuf;

    /* delay EQ symbol */
    cqY[2] = cqY[1];
    cqY[1] = cqY[0];
    cqY[0] = pV29->cqRotateIQ;   /* Slicer input ( Rotator Output) */

    /* delay decision symbol */
    cqD[1] = cqD[0];
    cqD[0].r = pV29->cqSliceIQ.r;  /* Slicer output */
    cqD[0].i = pV29->cqSliceIQ.i;  /* Slicer output */

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
    V29_TimingLoopFilter(pV29, qTimingErr);
}
