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

void V34Fax_PCR_SyncInit(V34Struct *pV34)
{
    UBYTE i;
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        pV34Fax->cqOldEqOut[i].r = 0;
        pV34Fax->cqOldEqOut[i].i = 0;
        pV34Fax->cqNewEqOut[i].r = 0;
        pV34Fax->cqNewEqOut[i].i = 0;
    }

    for (i = 0; i < V34_SYNC_SAMPLES; i++)
    {
        pV34Fax->cqdDemodSTone[i].r = 0;
        pV34Fax->cqdDemodSTone[i].i = 0;
    }

    pV34Fax->qdMinPhaseErr = Q31_MAX;
    pV34Fax->bestPolyIdx   = 0;
    pV34Fax->sbSyncCounter = V34_SYNC_DEMOD_SYMBOLS;
}


void V34Fax_PCR_SyncEq(V34Struct *pV34, CQWORD *pIn)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc  *pTCR = &pRx->TCR;
    QCFIRStruct *pcFir = &pTCR->eqfir;
    CQWORD *pcHead;
    CQWORD *pcCoef;
    QDWORD msum_r[V34_SYM_SIZE];
    QDWORD msum_i[V34_SYM_SIZE];
    SWORD nOffset;
    UWORD  i, Len, Len2, Len4;

    Len  = pcFir->nTapLen;
    Len2 = Len << 1;
    Len4 = Len << 2;

    /* Save the 3 input samples to the delayline */
    if ((pcFir->nOffset % V34_SYNC_SAMPLES) == 0)
    {
        nOffset = 0;
    }
    else
    {
        nOffset = V34_SYM_SIZE;
    }

    while (nOffset <= (Len4 - V34_SYM_SIZE))
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
        nOffset += V34_SYNC_SAMPLES;
    }

    pcFir->nOffset += V34_SYM_SIZE;

    if (pcFir->nOffset >= Len2)
    {
        pcFir->nOffset = 0;
    }

    pcHead = pcFir->pcDline + pcFir->nOffset;
    /* Compute 3 Equalizer Outputs, Each Equalizer output corresponds to */
    /* a timing shift by one sample */

    for (i = 0; i < V34_SYM_SIZE; i++)
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

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        pV34Fax->cqOldEqOut[i] = pV34Fax->cqNewEqOut[i];

        pV34Fax->cqNewEqOut[i].r = QDR15Q(msum_r[i]);
        pV34Fax->cqNewEqOut[i].i = QDR15Q(msum_i[i]);
    }
}

void V34Fax_PCR_SyncDemod(V34Struct *pV34, CQWORD *cqDemodOut)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    UWORD j, i;

    if (pV34Fax->sbSyncCounter & 1)
    {
        j = V34_SYM_SIZE;
    }
    else
    {
        j = 0;
    }

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        pV34Fax->cqdDemodSTone[i+j].r += (cqDemodOut[i].r);
        pV34Fax->cqdDemodSTone[i+j].i += (cqDemodOut[i].i);
    }

    --pV34Fax->sbSyncCounter;

    if (pV34Fax->sbSyncCounter == 0)
    {
        for (i = 0; i < 6; i++)
        {
            pV34Fax->cqdDemodSTone[i].r = pV34Fax->cqdDemodSTone[i].r >> V34_SYNC_DEMOD_SHIFT;
            pV34Fax->cqdDemodSTone[i].i = pV34Fax->cqdDemodSTone[i].i >> V34_SYNC_DEMOD_SHIFT;
        }
    }
}


void V34Fax_PCR_SyncTiming(V34FaxStruct *pV34Fax, SWORD nTimingIdx)
{
    UBYTE i;
    QDWORD qdPhaseErr, qdMagErr1, qdMagErr2, qdMagErr;

    for (i = 0; i < 3; i++)
    {
        qdPhaseErr  = QQMULQD(pV34Fax->cqOldEqOut[i].r, pV34Fax->cqNewEqOut[i].r);
        qdPhaseErr += QQMULQD(pV34Fax->cqOldEqOut[i].i, pV34Fax->cqNewEqOut[i].i);

        qdMagErr1  = QQMULQD(pV34Fax->cqOldEqOut[i].r, pV34Fax->cqOldEqOut[i].r);
        qdMagErr1 += QQMULQD(pV34Fax->cqOldEqOut[i].i, pV34Fax->cqOldEqOut[i].i);

        qdMagErr2  = QQMULQD(pV34Fax->cqNewEqOut[i].r, pV34Fax->cqNewEqOut[i].r);
        qdMagErr2 += QQMULQD(pV34Fax->cqNewEqOut[i].i, pV34Fax->cqNewEqOut[i].i);

        qdMagErr = qdMagErr1 - qdMagErr2;

        if (qdMagErr < 0)
        {
            qdMagErr = -qdMagErr;
        }

        if (qdPhaseErr < 0)
        {
            qdPhaseErr = -qdPhaseErr;
        }

#if 0
        DumpTone1[DumpTone1_Idx + i*32] = qdPhaseErr;
        DumpTone2[DumpTone2_Idx + i*32] = qdMagErr1;
        DumpTone3[DumpTone3_Idx + i*32] = qdMagErr2;

        if (i == 2)
        {
            DumpTone1_Idx++;
            DumpTone2_Idx++;
            DumpTone3_Idx++;
        }

        if ((DumpTone1_Idx == 32) || (DumpTone1_Idx == 128) || (DumpTone1_Idx == 224))
        {
            DumpTone1_Idx += 64;
            DumpTone2_Idx += 64;
            DumpTone3_Idx += 64;
        }

#endif

        qdPhaseErr += qdMagErr >> 1;

        if (qdPhaseErr < pV34Fax->qdMinPhaseErr)
        {
            pV34Fax->qdMinPhaseErr = qdPhaseErr;
            pV34Fax->bestPolyIdx = nTimingIdx + (i * V34FAX_MAX_INDEX);
        }
    }
}


void V34Fax_PCR_SyncCarrier(V34Struct *pV34, CQWORD cqPrevSymbol, CQWORD cqNewSymbol)
{
    V34RxStruct  *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    QWORD qReal, qImag;
    QDWORD qdAngle;

    qReal = cqPrevSymbol.r + cqNewSymbol.r;
    qImag = cqPrevSymbol.i + cqNewSymbol.i;

    qdAngle = DSPD_Atan2(qReal, qImag);
    qdAngle = QDQMULQDR(qdAngle, q1_OVER_PI);
    qdAngle -= 16384;
    //qdAngle += (UWORD)(ROUND15(pTCR->udError_phase_out) & 0xFFFF);

    while (qdAngle > 65536)
    {
        qdAngle -= 65536;
    }

    while (qdAngle < 0)
    {
        qdAngle += 65536;
    }

    pTCR->udError_phase_out = ((UDWORD)qdAngle << 15) & 0x7FFFFFFF;

    SinCos_Lookup_Fine((UWORD)qdAngle, &pTCR->qS, &pTCR->qC);
}

#endif
