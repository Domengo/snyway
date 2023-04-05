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

/**************************************************************************/
/* Demodulation - Using Hilbert Transform                                 */
/*                                                                        */
/* Functions: void Init_Demodulate(INFO *I)                               */
/*        void Demodulate(INFO *I, COMPLEX *Out, SWORD n)                 */
/*                                                                        */
/* INPUT:                                                                 */
/*    INFO *I    - The V.34 info structure                                */
/*    COMPLEX *Out   - the demodulated symbols                            */
/*    SWORD n    - Number of input samples                                */
/*                                                                        */
/* OUTPUT: Out is filled with demodulated symbols                         */
/**************************************************************************/

#include "v34ext.h"

void  V34_Demodulate_Init(DeModulateStruc *pDeMod)
{
    DspFirInit(&pDeMod->hilbfir, (QWORD *)tHilbert_Coef, pDeMod->pqHilbdline, HILB_TAP_LEN);

    pDeMod->rx_carrier_idx = 0;
    pDeMod->qRx_carrier_offset_idx = 0;
}

#if !USE_ASM
void  V34_Demodulate(ReceiveStruc *pRx, CQWORD *pcOut)
{
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    DeModulateStruc *pDeMod = &pRx->DeMod;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    CQWORD cqHout;
    QWORD  qC1;
    QWORD  qS1;
    QDWORD qdTemp;
    QWORD  qTempR, qTempI;
    QDWORD qOffset;
    QWORD  qGain;
    UBYTE  qIdxc, qIdxs;
    UWORD  i;

    /* Check if Timing Recovery require retard one symbol buffer */
    if (pTCR->sample_slip_count <= -3)
    {
        return;
    }

    qIdxc = pDeMod->rx_carrier_idx;
    qIdxs = (qIdxc - 64) & COS_SAMPLE_NUM; /* Step back Pi/2 for Sine value */
    qOffset = pDeMod->qRx_carrier_offset_idx;

    qGain = pAGC->qGain;

    for (i = 0; i < EC_INTER_NUM; i++)
    {
        cqHout.i = DspFir_Hilbert(&pDeMod->hilbfir, pRx->qEchoCancel_Out[i]);
        cqHout.r = pDeMod->hilbfir.pDline[pDeMod->hilbfir.nOffset + HILB_HALF_TAP_LEN];

        qC1 = DSP_tCOS_TABLE[qIdxc] + QQMULQ15(DSP_tCOS_OFFSET[qIdxc], qOffset);
        qS1 = DSP_tCOS_TABLE[qIdxs] + QQMULQ15(DSP_tCOS_OFFSET[qIdxs], qOffset);

        qdTemp  = QQMULQD(cqHout.r, qC1);
        qdTemp += QQMULQD(cqHout.i, qS1);
        qTempR  = QDR15Q(qdTemp);

        qdTemp  = QQMULQD(cqHout.i, qC1);
        qdTemp -= QQMULQD(cqHout.r, qS1);
        qTempI  = QDR15Q(qdTemp);

        pcOut->r = (QWORD)(QQMULQD(qGain, qTempR) >> 7);  /* Output signal */
        pcOut->i = (QWORD)(QQMULQD(qGain, qTempI) >> 7);  /* Output signal */

        ++pcOut;

        qOffset += pRx->qRx_carrier_offset;

        if (qOffset > qONE)
        {
            qOffset -= qONE;
            ++qIdxc;
            ++qIdxs;
        }

        qIdxc = (qIdxc + pRx->rx_carrier_freq) & COS_SAMPLE_NUM;
        qIdxs = (qIdxs + pRx->rx_carrier_freq) & COS_SAMPLE_NUM;
    }

    pDeMod->rx_carrier_idx = qIdxc;
    pDeMod->qRx_carrier_offset_idx = (QWORD)qOffset;
}

#endif
