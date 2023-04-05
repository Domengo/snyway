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

#include "v34fext.h"

#if SUPPORT_V34FAX

void V34Fax_SyncEq_Init(V34FaxStruct *pV34Fax)
{
    UBYTE i;

    pV34Fax->sbSyncCounter = 0;

    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        pV34Fax->syncPhaseError[i] = 0;
        pV34Fax->syncMagError[i]   = 0;
    }
}


void V34Fax_SyncEq(V34FaxStruct *pV34Fax)
{
    CQWORD *pcTimSam;
    QCFIRStruct *pcFir = &pV34Fax->cfirEq;
    CQWORD *pcCoef;
    UWORD Len;
    QDWORD msum_r;
    QDWORD msum_i;
    QDWORD msum1_r;
    QDWORD msum1_i;
    QDWORD qdPhaseErr;
    QDWORD qdMagErr;
    CQWORD cqSmp[2];
    UBYTE ubOffset;
    UBYTE i, j, k;
    UBYTE idx;

    ++pV34Fax->sbSyncCounter;

    if ((pV34Fax->sbSyncCounter < 4) || (pV34Fax->sbSyncCounter > 10))
    {
        return;
    }

    ubOffset = (UBYTE)(pV34Fax->ubOffset + (V34FAX_TIMING_DELAY_HALF - (V34FAX_SYM_LEN << 1) - V34FAX_HALFSYM_LEN));

    if (ubOffset >= V34FAX_TIMING_DELAY_HALF)
    {
        ubOffset -= V34FAX_TIMING_DELAY_HALF;
    }

    pcTimSam = pV34Fax->cTimDline + ubOffset;

    Len = pcFir->nTapLen / SH_SAMPLES;  /* 12/4 */

    /* Loop to compute EQ outputs for each of the 12 samples */
    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        /* Compute the 2 EQ Outputs for this sample timing */
        pcCoef  = pcFir->pcCoef;
        msum_i  = 0;
        msum_r  = 0;
        msum1_i = 0;
        msum1_r = 0;

        for (j = 0; j < Len; j++)
        {
            for (k = 0; k < SH_SAMPLES; k++)
            {
                /* First EQ output (previous symbol) */
                idx = i + (k * V34FAX_HALFSYM_LEN);

                msum_r  += QQMULQD(pcTimSam[idx].r, pcCoef->r);
                msum_r  -= QQMULQD(pcTimSam[idx].i, pcCoef->i);

                msum_i  += QQMULQD(pcTimSam[idx].r, pcCoef->i);
                msum_i  += QQMULQD(pcTimSam[idx].i, pcCoef->r);

                /* Second EQ output (current symbol) */
                idx += V34FAX_SYM_LEN;

                if (idx >= 24) /* (V34FAX_SYM_LEN * 2) */
                {
                    idx -= 24;
                }

                msum1_r  += QQMULQD(pcTimSam[idx].r, pcCoef->r);
                msum1_r  -= QQMULQD(pcTimSam[idx].i, pcCoef->i);

                msum1_i  += QQMULQD(pcTimSam[idx].r, pcCoef->i);
                msum1_i  += QQMULQD(pcTimSam[idx].i, pcCoef->r);

                pcCoef++;
            }
        }

        cqSmp[0].r = QDR15Q(msum_r);
        cqSmp[0].i = QDR15Q(msum_i);

        cqSmp[1].r = QDR15Q(msum1_r);
        cqSmp[1].i = QDR15Q(msum1_i);

        /* Compute Error in Phase Difference (90 degrees -> is 0 Error) */
        qdPhaseErr  = QQMULQD(cqSmp[0].r, cqSmp[1].r);
        qdPhaseErr += QQMULQD(cqSmp[0].i, cqSmp[1].i);

        if (qdPhaseErr < 0)
        {
            qdPhaseErr = -qdPhaseErr;
        }

        /* Compute Magnitude Difference  */
        qdMagErr  = QQMULQD(cqSmp[0].r, cqSmp[0].r);
        qdMagErr += QQMULQD(cqSmp[0].i, cqSmp[0].i);
        qdMagErr -= QQMULQD(cqSmp[1].r, cqSmp[1].r);
        qdMagErr -= QQMULQD(cqSmp[1].i, cqSmp[1].i);

        if (qdMagErr < 0)
        {
            qdMagErr = -qdMagErr;
        }

        /* Save error for this demod sample */
        pV34Fax->syncPhaseError[i] += qdPhaseErr;

        pV34Fax->syncMagError[i] += qdMagErr;
    }
}


void V34Fax_SyncTiming(V34FaxStruct *pV34Fax)
{
    UBYTE i;
    UWORD uPh1idx, uPh2idx;
    UWORD uBestIdx;

    uPh1idx = 0;
    uPh2idx = 1;

    if (pV34Fax->syncPhaseError[1] < pV34Fax->syncPhaseError[0])
    {
        uPh1idx = 1;
        uPh2idx = 0;
    }

    /* Get Best Demodulation Sample */
    for (i = 2; i < V34FAX_SYM_LEN; i++)
    {
        if (pV34Fax->syncPhaseError[i] < pV34Fax->syncPhaseError[uPh1idx])
        {
            uPh2idx = uPh1idx;
            uPh1idx = i;
        }
        else if (pV34Fax->syncPhaseError[i] < pV34Fax->syncPhaseError[uPh2idx])
        {
            uPh2idx = i;
        }
    }

    uBestIdx = uPh1idx;

    if (pV34Fax->syncMagError[uPh2idx] < pV34Fax->syncMagError[uPh1idx])
    {
        uBestIdx = uPh2idx;
    }

    pV34Fax->nTimIdx  = (uBestIdx << V34FAX_TIMELINE_RES);
    pV34Fax->nTimIdx -= (V34FAX_HALFSYM_LEN << V34FAX_TIMELINE_RES);  /* Add Back Half Symbol Offset */

    if (pV34Fax->nTimIdx > 0)
    {
        pV34Fax->nTimIdx -= (QWORD)V34FAX_SYM_LEN << V34FAX_TIMELINE_RES;
    }
}


void V34Fax_SyncCarrier(V34FaxStruct *pV34Fax)
{
    QWORD qReal, qImag;
    QDWORD qdAngle;

    qReal = pV34Fax->cqRotatePrevIQ.r + pV34Fax->cqRotateIQ.r;
    qImag = pV34Fax->cqRotatePrevIQ.i + pV34Fax->cqRotateIQ.i;

    qdAngle = DSPD_Atan2(qReal, qImag);

    qdAngle = QDQMULQDR(qdAngle, q1_OVER_PI);

    qdAngle -= 16384;

    qdAngle += pV34Fax->uRotatePhase;

    while (qdAngle > 65536)
    {
        qdAngle -= 65536;
    }

    while (qdAngle < 0)
    {
        qdAngle += 65536;
    }

    SinCos_Lookup_Fine((UWORD)qdAngle, &pV34Fax->qRotateSin, &pV34Fax->qRotateCos);

    pV34Fax->uRotatePhase = (UWORD)qdAngle;
}

#endif
