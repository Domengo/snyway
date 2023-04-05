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

#include "v17ext.h"

void V17_SyncEq(V32ShareStruct *pV32Share)
{
    QCFIRStruct *pcFir = &(pV32Share->cfirEq_D);
    CQWORD *pIn = pV32Share->Poly.cqTimingIQ;
    CQWORD *pcHead;
    CQWORD *pcCoef;
    QDWORD msum_r[V32_SYM_SIZE];
    QDWORD msum_i[V32_SYM_SIZE];
    SWORD nOffset;
    UWORD  i, Len, Len2, Len4;

    Len  = pcFir->nTapLen;
    Len2 = Len << 1;
    Len4 = Len << 2;

    /* Save the 3 input samples to the delayline */
    if ((pcFir->nOffset % V17_SYNC_SAMPLES) == 0)
    {
        nOffset = 0;
    }
    else
    {
        nOffset = V32_SYM_SIZE;
    }

    while (nOffset <= (Len4 - V32_SYM_SIZE))
    {
        pcHead = pcFir->pcDline + nOffset;

        (*pcHead).r = (*(pIn)).r;
        (*pcHead).i = (*(pIn)).i;

        pcHead++;

        (*pcHead).r = (*(pIn + 1)).r;
        (*pcHead).i = (*(pIn + 1)).i;

        pcHead++;

        (*pcHead).r = (*(pIn + 2)).r;
        (*pcHead).i = (*(pIn + 2)).i;

        nOffset += (V17_SYNC_SAMPLES);
    }

    pcFir->nOffset += V32_SYM_SIZE;

    if (pcFir->nOffset >= Len2)
    {
        pcFir->nOffset = 0;
    }

    pcHead = pcFir->pcDline + pcFir->nOffset;
    /* Compute 3 Equalizer Outputs, Each Equalizer output corresponds to */
    /* a timing shift by one sample */

    for (i = 0; i < V32_SYM_SIZE; i++)
    {
        msum_r[i] = 0;
        msum_i[i] = 0;
    }

    pcCoef = pcFir->pcCoef;

    for (i = 0; i < Len; i++)
    {
        msum_r[0] += QQMULQD(pcHead->r, pcCoef->r);
        msum_r[0] -= QQMULQD(pcHead->i, pcCoef->i);
        msum_i[0] += QQMULQD(pcHead->r, pcCoef->i);
        msum_i[0] += QQMULQD(pcHead->i, pcCoef->r);

        pcHead++;

        msum_r[1] += QQMULQD(pcHead->r, pcCoef->r);
        msum_r[1] -= QQMULQD(pcHead->i, pcCoef->i);
        msum_i[1] += QQMULQD(pcHead->r, pcCoef->i);
        msum_i[1] += QQMULQD(pcHead->i, pcCoef->r);

        pcHead++;

        msum_r[2] += QQMULQD(pcHead->r, pcCoef->r);
        msum_r[2] -= QQMULQD(pcHead->i, pcCoef->i);
        msum_i[2] += QQMULQD(pcHead->r, pcCoef->i);
        msum_i[2] += QQMULQD(pcHead->i, pcCoef->r);

        pcCoef++;
    }

#if SUPPORT_V17/* The switch is only for compiling, cannot delete!!! */

    for (i = 0; i < V32_SYM_SIZE; i++)
    {
        pV32Share->cqOldEqOut[i].r = pV32Share->cqNewEqOut[i].r;
        pV32Share->cqOldEqOut[i].i = pV32Share->cqNewEqOut[i].i;
        pV32Share->cqNewEqOut[i].r = QDR15Q(msum_r[i]);
        pV32Share->cqNewEqOut[i].i = QDR15Q(msum_i[i]);
    }

#endif
}

void V17_SyncDemod(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    CQWORD *pcHead;
    CQWORD cqDemod;
    UWORD j, i;

    if (pV17->sbSyncCounter > 0)
    {
        if (pV17->sbSyncCounter & 1)
        {
            j = V32_SYM_SIZE;
        }
        else
        {
            j = 0;
        }

        for (i = 0; i < V32_SYM_SIZE; i++)
        {
            pV17->cqdDemodABTone[i+j].r += (pV32Share->cDemodIQBuf[i].r);
            pV17->cqdDemodABTone[i+j].i += (pV32Share->cDemodIQBuf[i].i);
        }

        pV17->sbSyncCounter --;

        if (pV17->sbSyncCounter == 0)
        {
            for (i = 0; i < 6; i++)
            {
                pV17->cqdDemodABTone[i].r = pV17->cqdDemodABTone[i].r >> V17_SYNC_DEMOD_SHIFT;
                pV17->cqdDemodABTone[i].i = pV17->cqdDemodABTone[i].i >> V17_SYNC_DEMOD_SHIFT;
            }
        }
    }
    else if (pV17->sbSyncCounter <= 0)/* Overwrite timing buffer with average demod signal */
    {
        if (pV32Share->ubOffset < V32_SYM_SIZE)
        {
            pV32Share->ubOffset += (V32_TIMING_DELAY_HALF - V32_SYM_SIZE);
            pV32Share->Poly.pcqTimingDlineHead += (V32_TIMING_DELAY_HALF - V32_SYM_SIZE);
        }
        else
        {
            pV32Share->ubOffset -= V32_SYM_SIZE;
            pV32Share->Poly.pcqTimingDlineHead -= V32_SYM_SIZE;
        }

        if (pV17->sbSyncCounter == 0)
        {
            j = 0;
            pV17->sbSyncCounter = -1;
        }
        else
        {
            j = V32_SYM_SIZE;
            pV17->sbSyncCounter = 0;
        }

        for (i = 0; i < V32_SYM_SIZE; i++)
        {
            pcHead = pV32Share->cqTimingDline + pV32Share->ubOffset;

            /* insert sample into timing delay line (double buffer) */
            cqDemod.r = (QWORD)pV17->cqdDemodABTone[i+j].r;
            cqDemod.i = (QWORD)pV17->cqdDemodABTone[i+j].i;

            *pcHead = cqDemod;
            *pV32Share->Poly.pcqTimingDlineHead++ = cqDemod;

            pV32Share->ubOffset ++;

            if (pV32Share->ubOffset >= V32_TIMING_DELAY_HALF)
            {
                pV32Share->ubOffset = 0;
                pV32Share->Poly.pcqTimingDlineHead -= V32_TIMING_DELAY_HALF;
            }
        }
    }
}
