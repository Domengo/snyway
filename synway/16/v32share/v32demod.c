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
/*                                                                         */
/* Demodulation                                                            */
/* ------------                                                            */
/*                                                                         */
/* INPUT:  qDemodIn', where the  samples to be demodulated are stored      */
/*                                                                         */
/* OUTPUT: writes the demodulated I, Q values into 'cDemodIQBuf'           */
/***************************************************************************/

#include "v3217ext.h"

void V32_DemodInit(V32ShareStruct *pV32Share)
{
    pV32Share->uDemodPhase      = 0;
    pV32Share->nDemodDeltaPhase = 16384; /* 16384=65536*1800Hz/7200 */

#if V32_FREQUENCY_OFFSET
    pV32Share->nDemodDeltaOffset = 0;
    pV32Share->nDemodDeltaCount  = 0;
#endif

    DspFirInit(&pV32Share->firHilb, (QWORD *)nV32_HilbCoefTab, pV32Share->qHilbDline, V32_HILB_TAP_LEN);

    /* initialize timing delay line index */
    pV32Share->ubOffset = 0;
    pV32Share->Poly.pcqTimingDlineHead = pV32Share->cqTimingDline + V32_TIMING_DELAY_HALF;
}


/* ---------- Demodulate ---------- */

#if !USE_ASM

void V32_Demodulate(V32ShareStruct *pV32Share)
{
    QDWORD temp;
    CQWORD cqHilbOut;
    CQWORD cqDemodOut;
    UWORD  phase;
    SWORD  delta_phase;
    QWORD  qCos, qSin;
    CQWORD *pcHead;
    QWORD  qInVal;
    UBYTE  i;

    phase       = pV32Share->uDemodPhase;
    delta_phase = pV32Share->nDemodDeltaPhase;

    /* demodulate */
    for (i = 0; i < V32_SYM_SIZE; i++)
    {
#if 0 /* After Echo Cancel PCM in */

        if (DumpTone1_Idx < 10000000) { DumpTone1[DumpTone1_Idx++] = pV32Share->qDemodIn[i]; }

#endif

        /* Gain is 8.8 format, after multiply, convert back to 1.15 format, then Write back scaled PCM samples */
        qInVal = QQMULQR8(pV32Share->qDemodIn[i], pV32Share->qSagcScale);

        pV32Share->qDemodIn[i] = qInVal;

        /* Doing 90-degree phase change for quadrature signal */
        cqHilbOut.i = DspFir_Hilbert(&pV32Share->firHilb, qInVal);
        cqHilbOut.r = pV32Share->firHilb.pDline[pV32Share->firHilb.nOffset + V32_HILB_HALF_TAP_LEN];

        /* find cos, sin values */
        SinCos_Lookup_Fine(phase, &qSin, &qCos);//SinCos_Lookup(&phase, delta_phase, &qSin, &qCos);
        Phase_Update(&phase, delta_phase);

#if V32_FREQUENCY_OFFSET
        pV32Share->nDemodDeltaCount += pV32Share->nDemodDeltaOffset;

        while (pV32Share->nDemodDeltaCount < 0)
        {
            pV32Share->nDemodDeltaCount += 256;
            --phase;
        }

        while (pV32Share->nDemodDeltaCount > 256)
        {
            pV32Share->nDemodDeltaCount -= 256;
            ++phase;
        }

#endif

        /* calculate demodulated I and Q values */
        temp         = QQMULQD(cqHilbOut.r, qCos);
        temp        += QQMULQD(cqHilbOut.i, qSin);
        cqDemodOut.r = QDR15Q(temp);

        temp         = QQMULQD(cqHilbOut.i, qCos);
        temp        -= QQMULQD(cqHilbOut.r, qSin);
        cqDemodOut.i = QDR15Q(temp);

        /* store demodulated I and Q values */
        pV32Share->cDemodIQBuf[i].r = cqDemodOut.r;
        pV32Share->cDemodIQBuf[i].i = cqDemodOut.i;

        pcHead = pV32Share->cqTimingDline + pV32Share->ubOffset;

        /* insert sample into timing delay line (double buffer) */
        *pcHead = cqDemodOut;
        *pV32Share->Poly.pcqTimingDlineHead++ = cqDemodOut;

        pV32Share->ubOffset ++;

        if (pV32Share->ubOffset >= V32_TIMING_DELAY_HALF)
        {
            pV32Share->ubOffset = 0;
            pV32Share->Poly.pcqTimingDlineHead -= V32_TIMING_DELAY_HALF;
        }
    }

    pV32Share->uDemodPhase = phase;
}

#endif

/**----------------------------------------------------
** suppose the speed can be increased by unrolling and
** taking advantage of regular values of qCos & qSin
**----------------------------------------------------
**/
