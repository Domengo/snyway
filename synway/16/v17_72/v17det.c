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

#include "v17ext.h"

void V17_AutoCorrDetect_init(V17Struct *pV17)
{
    UBYTE i;

    /* Auto Correlation parameters */
    pV17->ubCorrIdx      = 0;
    pV17->qdCorrelation  = 0;
    pV17->qdCorrelationI = 0;
    pV17->qdCorrelationQ = 0;

    /* init. correlation delay line */
    for (i = 0; i < V17_CORR_NUM_SAMPLES; i++)
    {
        pV17->cqCorrDline[i].r = 0;
        pV17->cqCorrDline[i].i = 0;
    }
}

void V17_AutoCorr_Detect(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    QDWORD correl, Clipped_correlation;
    UBYTE  i, ubCorrIdx;

    Clipped_correlation = Q31_MAX;
    ubCorrIdx = pV17->ubCorrIdx;

    /* add new samples to delay line, update correlation value */
    for (i = 0; i < V32_SYM_SIZE; i++)
    {
        /* subtract last delayed value from correlation value */
        pV17->qdCorrelationI -= pV17->cqCorrDline[ubCorrIdx].r;
        pV17->qdCorrelationQ -= pV17->cqCorrDline[ubCorrIdx].i;

        /* replace last delayed value with input value */
        pV17->cqCorrDline[ubCorrIdx].r = pV32Share->cDemodIQBuf[i].r;
        pV17->cqCorrDline[ubCorrIdx].i = pV32Share->cDemodIQBuf[i].i;

        /* add input value to correlation value */
        pV17->qdCorrelationI += pV32Share->cDemodIQBuf[i].r;
        pV17->qdCorrelationQ += pV32Share->cDemodIQBuf[i].i;
        /* taking absolute value may ?? */
        /* calculate correlation value */     /* not faster than square    ?? */
        correl = 0;

        if (pV17->qdCorrelationI < 0)
        {
            correl -= pV17->qdCorrelationI;
        }
        else
        {
            correl += pV17->qdCorrelationI;
        }

        if (pV17->qdCorrelationQ < 0)
        {
            correl -= pV17->qdCorrelationQ;
        }
        else
        {
            correl += pV17->qdCorrelationQ;
        }

        if (correl < Clipped_correlation)
        {
            Clipped_correlation = correl;
        }

        ubCorrIdx++;

        if (ubCorrIdx >= V17_CORR_NUM_SAMPLES)
        {
            ubCorrIdx = 0;    /* check for synchronization to */
        }
    }  /* take this wrap-around out */

    pV17->ubCorrIdx     = ubCorrIdx;
    pV17->qdCorrelation = Clipped_correlation;

    //???Must care V21 AT command when data pump, and decide timeout by WinFax
    if (Clipped_correlation < 1000)
    {
        pV17->sTimeOutCounter ++;
    }
    else
    {
        pV17->sTimeOutCounter = 0;
    }
}
