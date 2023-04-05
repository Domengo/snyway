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

/********************************************************************
This file initilises the detect function and provides the correlation
value for actual transition from segment 2 to segment  3.
********************************************************************/

#include "v29ext.h"

void V29_AutoCorrDetect_init(V29Struct *pV29)
{
    UBYTE i;

    /* init. correlation value, index */
    pV29->ubCorrIdx          = 0;
    pV29->ubCorr_Sum_Counter = 0;
    pV29->ubCorrelation_Flag = 0;
    pV29->qdCorrelationI     = 0;   /* just borrow this val, actually don't care I and Q */
    pV29->qdCorrelationQ     = 0;

    /* init. correlation delay line */
    for (i = 0; i < V29_CORR_NUM_SAMPLES; i++)
    {
        pV29->qCorrDline[i] = 0;
    }
}

void V29_AutoCorr_Detect(V29Struct *pV29)
{
    UBYTE i;
    QDWORD qdTemp;

    /* add new samples to delay line, update correlation value */
    for (i = 0; i < V29_SYM_SIZE; i++) /* Codec 7200, sym_rate: 3 */
    {
        /* replace last delayed value with input value */
        /* Multiply the oldest with the current */
        /* Scale the correlation to prevent overflow */
        qdTemp = QQMULQD(pV29->qCorrDline[pV29->ubCorrIdx], pV29->qDemodIn[i]);
        qdTemp >>= 3;
        pV29->qdCorrelationI += qdTemp;
        pV29->ubCorr_Sum_Counter++;

        /* Save the current sample and wrap around the buffer */
        pV29->qCorrDline[pV29->ubCorrIdx] = pV29->qDemodIn[i];  /* if after AGC, use qDemodIn */

        /* ubCorrIdx is a circular index */
        pV29->ubCorrIdx++;

        /* Wrap around the index if necessary */
        if (pV29->ubCorrIdx == V29_CORR_NUM_SAMPLES)
        {
            pV29->ubCorrIdx = 0;
        }
    }

    if (pV29->ubCorr_Sum_Counter >= V29_CORR_NUM_SAMPLES)
    {
        pV29->ubCorr_Sum_Counter = 0;

#if 0

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV29->qdCorrelationI; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV29->ubCorrelation_Flag; }

#endif

        if (pV29->qdCorrelationI < 0)
        {
            pV29->ubCorrelation_Flag = 1;
        }

        if (pV29->ubCorrelation_Flag == 1)
        {
            pV29->sTimeOutCounter ++;
        }

        pV29->qdCorrelationQ  = pV29->qdCorrelationI;
        pV29->qdCorrelationI  = 0;
    }
}
