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

/**********************************************************************/
/*   This functions takes the floating input from the echo buffer and */
/*   provides complex value output from the demodulator.              */
/**********************************************************************/

#include "v27ext.h"

void V27_Demodulate_init(V27Struct *pV27)
{
    pV27->DemodPhase = 128;

    /* initialize Hilbert Transform filter */
    DspFirInit(&pV27->pHilbFIR, (SWORD *)tHilbert_Coef, pV27->qHilbDelay, HILB_TAP_LEN);

    /* initialize timing delay line index (V27_TIMING_DELAY_HALF = 21) */
    pV27->ubOffset = 0;
    pV27->Poly.pcqTimingDlineHead = pV27->pcTimingDline + V27_TIMING_DELAY_HALF;
}

void V27_Demodulate(V27Struct *pV27)
{
    UBYTE i;
    SWORD cos, sin;
    CQWORD cqDout, cqTempOut;
    UWORD qPhase;
    SDWORD qTempR, qTempI;
    CQWORD *pcHead;

    qPhase = pV27->DemodPhase;

    /* demodulate */
    for (i = 0; i < pV27->ubSymBufSize; i++)
    {
        pV27->qDemodIn[i] = QQMULQR8(pV27->PCMinPtr[i], pV27->qSagcScale);

        /* Doing 90-degree phase change for quadrature signal */
        cqDout.i = DspFir_Hilbert(&(pV27->pHilbFIR), pV27->qDemodIn[i]);
        cqDout.r = pV27->pHilbFIR.pDline[pV27->pHilbFIR.nOffset + HILB_HALF_TAP_LEN];

        /* find cos, sin values */
        SinCos_Lookup(&qPhase, V27_CARRIER_1800_DELTA_PHASE, &sin, &cos);

        /* calc. & store demodulated I and Q values */
        qTempR = QQMULQD(cqDout.r, cos);
        qTempI = QQMULQD(cqDout.i, sin);
        cqTempOut.r = QD15Q(qTempR + qTempI);

        qTempR = QQMULQD(cqDout.i, cos);
        qTempI = QQMULQD(cqDout.r, sin);
        cqTempOut.i = QD15Q(qTempR - qTempI);

        pV27->cDemodIQBuf[i].r = cqTempOut.r;
        pV27->cDemodIQBuf[i].i = cqTempOut.i;

        pV27->cDemodIQ_Prev[i].r = pV27->cDemodIQBuf[i].r;
        pV27->cDemodIQ_Prev[i].i = pV27->cDemodIQBuf[i].i;

        /* insert sample into timing delay line (double buffer) */
        pcHead = pV27->pcTimingDline + pV27->ubOffset;

        *pcHead = pV27->cDemodIQ_Prev[i];
        *(pV27->Poly.pcqTimingDlineHead++) = pV27->cDemodIQ_Prev[i];

        pV27->ubOffset ++;

        if (pV27->ubOffset >= V27_TIMING_DELAY_HALF)
        {
            pV27->ubOffset = 0;
            pV27->Poly.pcqTimingDlineHead -= V27_TIMING_DELAY_HALF;
        }
    }

    pV27->DemodPhase = qPhase;
}

void V27_CarrLoss_Detect(V27Struct *pV27)
{
    QDWORD qdEgy;
    UBYTE  i;

    qdEgy = 0;

    for (i = 0; i < pV27->ubSymBufSize; i++)
    {
        qdEgy += QQMULQD(pV27->qDemodIn[i], pV27->qDemodIn[i]) >> 8;
    }

    pV27->qdCarrLoss_egy += qdEgy;
    pV27->uCarrLoss_Count += pV27->ubSymBufSize;

    if (pV27->uCarrLoss_Count >= 96) /* 10ms energy window */
    {
        if (pV27->qdCarrLossEgy_Ref == 0) /* set threshold to 25% of energy */
        {
            pV27->qdCarrLossEgy_Ref = pV27->qdCarrLoss_egy >> 2;
        }
        else if (pV27->qdCarrLoss_egy < pV27->qdCarrLossEgy_Ref)
        {
            pV27->CarrLoss_Flag     = 2;
            TRACE0("V27: Carrier lost Detected");
        }

        pV27->uCarrLoss_Count = 0;
        pV27->qdCarrLoss_egy = 0;
    }
}
