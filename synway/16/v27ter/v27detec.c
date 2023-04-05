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

/* ----------------------------------------------------------------------- */
/* Signal Transition Detection                                             */
/* ----------------------------------------------------------------------- */

#include "v27ext.h"

void V27_AutoCorrDetect_init(V27Struct *pV27)
{
    UBYTE i;

    /* init. correlation value, index */
    pV27->ubCorrIdx     = 0;
    pV27->qdCorrelation = 0;

    /* init. correlation delay line */
    for (i = 0; i < V27_CORR_NUM_SAMPLES_1200; i++)
    {
        pV27->cqCorrDline[i].r = 0;
        pV27->cqCorrDline[i].i = 0;
    }
}

void V27_AutoCorr_Detect(V27Struct *pV27)
{
    QDWORD qdCorrelationI, qdCorrelationQ;
    UBYTE i, Corr_Num_Samples;

    if (pV27->ubSymBufSize == V27_SYM_SIZE_1600)
    {
        Corr_Num_Samples = V27_CORR_NUM_SAMPLES_1600;
    }
    else
    {
        Corr_Num_Samples = V27_CORR_NUM_SAMPLES_1200;
    }

    /* add new samples to delay line, update correlation value */
    for (i = 0; i < pV27->ubSymBufSize; i++)
    {
        /* replace last delayed value with input value */
        pV27->cqCorrDline[pV27->ubCorrIdx].r = pV27->cDemodIQBuf[i].r;
        pV27->cqCorrDline[pV27->ubCorrIdx].i = pV27->cDemodIQBuf[i].i;

        pV27->ubCorrIdx++;

        if (pV27->ubCorrIdx == Corr_Num_Samples)
        {
            pV27->ubCorrIdx = 0;
        }
    }

    qdCorrelationI = 0;
    qdCorrelationQ = 0;

    for (i = 0; i < Corr_Num_Samples; i++)
    {
        /* add input value to correlation value */
        qdCorrelationI += (QDWORD)pV27->cqCorrDline[i].r;
        qdCorrelationQ += (QDWORD)pV27->cqCorrDline[i].i;
    }

    if (qdCorrelationI < 0)
    {
        qdCorrelationI = - qdCorrelationI;
    }

    if (qdCorrelationQ < 0)
    {
        qdCorrelationQ = - qdCorrelationQ;
    }

    pV27->qdCorrelation = qdCorrelationI + qdCorrelationQ;

    if (pV27->qdCorrelation < 20000)
    {
        pV27->sTimeOutCounter ++;
    }
    else
    {
        pV27->sTimeOutCounter = 0;
    }

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV27->qdCorrelation; }

#endif
}
