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

/***************************************************************************/
/* timing functions for receiver timing loop.                              */
/*                                                                         */
/* INPUT:  'pcDemodIQ' which points to the demodulated samples.            */
/*                                                                         */
/* OUTPUT: 'cqTimingIQ' for the equalizer input.                           */
/***************************************************************************/

#include "v3217ext.h"

void V32_TimingUpdate_init(V32ShareStruct *pV32Share)
{
    UBYTE i;

    for (i = 0; i < 3; i++)
    {
        pV32Share->cqTimingEqOutBuf[i].r = 0;
        pV32Share->cqTimingEqOutBuf[i].i = 0;
    }

    for (i = 0; i < 2; i++)
    {
        pV32Share->cqTimingSliceBuf[i].r = 0;
        pV32Share->cqTimingSliceBuf[i].i = 0;
    }
}

/* -- loop-filter, update timing -- */
void V32_TimingLoopFilter(V32ShareStruct *pV32Share, QWORD qTimingErr)
{
    QWORD qPLL_output1;
    QWORD qPLL_output2;

    /* find previous PLL outputs */
    qPLL_output1 = pV32Share->qTimingErr1;
    qPLL_output2 = pV32Share->qTimingErr2;

    if (pV32Share->ubTimErr1Freeze == 0)
    {
        qPLL_output1 += qTimingErr;
    }

    qPLL_output2 += ((qTimingErr >> pV32Share->ubTimShift) + (qPLL_output1 >> V32_SIGMAP_SCALE));

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV32Share->ubTimErr1Freeze; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = qTimingErr; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = qPLL_output1; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = qPLL_output2; }

#endif

    /* timing adjustment and tracking */
    if (qPLL_output2 > 255)
    {
        pV32Share->Poly.nTimingIdx += V32_TIME_FAI_STEP;
        qPLL_output2 = 0;
    }
    else if (qPLL_output2 < (-255))
    {
        pV32Share->Poly.nTimingIdx -= V32_TIME_FAI_STEP;
        qPLL_output2 = 0;
    }

    /* store new PLL outputs */
    pV32Share->qTimingErr1 = qPLL_output1;
    pV32Share->qTimingErr2 = qPLL_output2;

#if 0

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV32Share->Poly.nTimingIdx; }

#endif
}

void V32_TimingUpdate(V32ShareStruct *pV32Share, CQWORD cqTimeIQ)
{
    QDWORD qdTim_err;
    CQWORD *cqY, *cqD;
    QWORD  qTimingErr;

    /* pointers to delayed EQ, slicer samples */
    cqY = pV32Share->cqTimingEqOutBuf;
    cqD = pV32Share->cqTimingSliceBuf;

    /* delay EQ symbol */
    cqY[2] = cqY[1];
    cqY[1] = cqY[0];
    cqY[0] = pV32Share->cqRotateIQ;   /* Slicer input ( Rotator Output) */

    /* delay decision symbol */
    cqD[1]   = cqD[0];
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
    V32_TimingLoopFilter(pV32Share, qTimingErr);
}

void V32_TimingUpdateByTRN2(V32ShareStruct *pV32Share)
{
    /* update timing with Training Symbol */
    V32_TimingUpdate(pV32Share, pV32Share->cqTrainIQ);
}

void V32_TimingUpdateBySliceIQ(V32ShareStruct *pV32Share)
{
    /* update timing with slicer output */
    V32_TimingUpdate(pV32Share, pV32Share->cqSliceIQ);
}
