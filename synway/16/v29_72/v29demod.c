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

#include "v29ext.h"

void V29_Demodulate_init(V29Struct *pV29)
{
    pV29->DemodPhase = 128;

    /* initialize Hilbert Transform filter */
    DspFirInit(&pV29->pHilbFIR, (SWORD *)tHilbert_Coef, pV29->qHilbDelay, HILB_TAP_LEN);

    /* initialize timing delay line index */
    pV29->ubOffset = 0;
    pV29->Poly.pcqTimingDlineHead = (pV29->pcTimingDline + V29_TIMING_DELAY_HALF);
}

void V29_Demodulate(V29Struct *pV29)
{
    CQWORD cqDout, cqDout1;
    UBYTE  i;
    SWORD  qC, qS;
    UWORD  Phase;
    SDWORD qTempR, qTempI;
    CQWORD *pcHead;

    Phase = pV29->DemodPhase;

    for (i = 0; i < V29_SYM_SIZE; i++)
    {
        /* Gain is 8.8 format, after multiply, convert back to 1.15 format */
        pV29->qDemodIn[i] = QQMULQR8(pV29->PCMinPtr[i], pV29->qSagcScale);

        /* Doing 90-degree phase change for quadrature signal */
        cqDout.i = DspFir_Hilbert(&(pV29->pHilbFIR), pV29->qDemodIn[i]);
        cqDout.r = pV29->pHilbFIR.pDline[pV29->pHilbFIR.nOffset + HILB_HALF_TAP_LEN];

        /* find cos, sin values */
        SinCos_Lookup_Fine(Phase, &qS, &qC);
        Phase_Update(&Phase, V29_CARRIER_1700_DELTA_PHASE);

        /* calc. & store demodulated I and Q values */
        qTempR = QQMULQD(cqDout.r, qC);
        qTempI = QQMULQD(cqDout.i, qS);
        pV29->cDemodIQBuf[i].r = QD15Q(qTempR + qTempI);

        qTempR = QQMULQD(cqDout.i, qC);
        qTempI = QQMULQD(cqDout.r, qS);
        pV29->cDemodIQBuf[i].i = QD15Q(qTempR - qTempI);

        cqDout1.r = pV29->cDemodIQBuf[i].r;
        cqDout1.i = pV29->cDemodIQBuf[i].i;

        pcHead = pV29->pcTimingDline + pV29->ubOffset;

        /* insert sample into timing delay line (double buffer) */
        *pcHead = cqDout1;
        *(pV29->Poly.pcqTimingDlineHead++) = cqDout1;

        pV29->ubOffset ++;

        if (pV29->ubOffset >= V29_TIMING_DELAY_HALF)
        {
            pV29->ubOffset = 0;
            pV29->Poly.pcqTimingDlineHead -= V29_TIMING_DELAY_HALF;
        }
    }

    /* update 'Phase' */
    pV29->DemodPhase = Phase;

}

void V29_CarrLoss_Detect(V29Struct *pV29)
{
    QDWORD qdEgy;
    UBYTE  i;

    qdEgy = 0;

    for (i = 0; i < V29_SYM_SIZE; i++)
    {
        qdEgy += QQMULQD(pV29->qDemodIn[i], pV29->qDemodIn[i]) >> 3;
    }

    pV29->qdCarrLoss_egy += (qdEgy - pV29->qdCarrLoss_egy) >> 2;

    if (pV29->uCarrLoss_Count < 1024)
    {
        pV29->qdCarrLossEgy_Ref += (pV29->qdCarrLoss_egy >> 6);

        if (pV29->uCarrLoss_Count == 1023)
        {
            pV29->qdCarrLossEgy_Ref >>= 8;
        }

        pV29->uCarrLoss_Count++;
    }
    else
    {
#if 0

        if (DumpTone1_Idx < 500000) { DumpTone1[DumpTone1_Idx++] = pV29->qdCarrLossEgy_Ref; }

        if (DumpTone2_Idx < 500000) { DumpTone2[DumpTone2_Idx++] = pV29->qdCarrLoss_egy; }

#endif

        if (pV29->qdCarrLoss_egy < pV29->qdCarrLossEgy_Ref)
        {
            if (pV29->CarrLoss_Flag == 0)
            {
                pV29->CarrLoss_Flag  = 1;
            }
            else
            {
                pV29->qdCarrLoss_egy    = 0;
                pV29->CarrLoss_Flag     = 2;
                pV29->qdCarrLossEgy_Ref = 0;
                pV29->uCarrLoss_Count   = 0;
            }
        }
        else if (pV29->CarrLoss_Flag == 1)
        {
            pV29->CarrLoss_Flag  = 0;
        }
    }
}
