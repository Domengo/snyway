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

/*phase-jitter tracking loop*/

#include "v34ext.h"

#if PHASEJITTER

CONST QWORD V34_jt_cnt_val_1[3] =
{
    1000,
    1200,
    -1
};

CONST QWORD V34_jt_gain_val_1[2] =
{
    -q04,   -q01
};

void  V34_jt_loop_init(JTstruct *pJT, UBYTE reset)
{
    UBYTE i;

    pJT->qC = qONE;
    pJT->qS = 0;

    pJT->pow = 0;
    pJT->mu_shift = 15;
    pJT->qdJTout = 0;

    pJT->gain = -q04;
    pJT->counter = 0;
    pJT->idx = 0;

    if (reset == 1)
    {
        for (i = 0; i < V34_JTTAPS; i++)
        {
            pJT->qdCoef[i] = 0;
        }
    }

    for (i = 0; i < V34_JTTAPS * 2; i++)
    {
        pJT->dl[i] = 0;
    }

    pJT->qPde_1 = 0;
    pJT->qdPdey = 0;

    pJT->ubOffset = 0;

    pJT->enable = 1;

    if (reset == 1)
    {
        pJT->xcorr1 = 0;
        pJT->xcorr2 = 0;
    }
}

#if 0 /// LLL temp
void V34_jt_decision(JTstruct *jt)
{
    QDWORD qdX1, qdX2;

    qdX1 = QABS(jt->xcorr1);
    qdX2 = QABS(jt->xcorr2);

    qdX1 >>= JTDECSHIFT;

    if ((qdX1 > qdX2) && (qdX2 > MIN_PD_CORR))
    {
        jt->enable = 1;
        TRACE2("Jitter loop enabled : %d %d", qdX1, qdX2);
    }
    else
    {
        jt->enable = 0;
        TRACE2("Jitter loop disabled : %d %d", qdX1, qdX2);
    }
}
#endif

void V34_jt_loop(JTstruct *pJT, CQWORD *pcUs, CQWORD *pcUt, UBYTE adapt)
{
    UBYTE  i;
    QDWORD qdOut;
    QWORD  pdeQ;
    QWORD  jtdiff;
    QWORD  *pHead, *pCoef;
    QDWORD muROUND;
    UBYTE  muS;
    QDWORD qdTemp1, qdTemp2;
    UWORD  uPhase;
    QWORD  qTemp;

    qdTemp1 = QQMULQD(pcUs->i, pcUt->r);
    qdTemp2 = QQMULQD(pcUs->r, pcUt->i);

    qdTemp1 -= qdTemp2;
    qTemp   = QDR15Q(qdTemp1);
    qdTemp1 = QQMULQD(qTemp, 9);

    if (qdTemp1 < 32767)
    {
        pdeQ = (QWORD)qdTemp1;
    }
    else
    {
        pdeQ = 32767;
    }

    qdTemp1 = ((QDWORD)pdeQ - pJT->qPde_1) << 15;
    qdTemp2 = QDQMULQD(pJT->qdPdey, q095);
    pJT->qdPdey = qdTemp2 + qdTemp1;
    pJT->qPde_1 = pdeQ;

    pdeQ = QD15Q(pJT->qdPdey);

    if (adapt == 1)
    {
        if ((pJT->counter <= 100 + JTWAIT) && (pJT->counter > JTWAIT))
        {
            qdTemp1 = QQMULQD(pdeQ, pdeQ);

            pJT->pow += (qdTemp1 >> 5);

            if (pJT->pow < 0) /* Overflow happened */
            {
                pJT->pow = Q31_MAX;
            }
        }

        if (pJT->counter == 100 + JTWAIT)
        {
            qdTemp1 = (QDWORD)400000; /* Can tune to adjust adaptation rate */
            pJT->mu_shift = 0;

            while ((qdTemp1 < pJT->pow) && (qdTemp1 > 0))
            {
                pJT->mu_shift++;

                qdTemp1 <<= 1;
            }

            TRACE2("jtpow=%" FORMAT_DWORD_PREFIX "d mus=%d", pJT->pow, pJT->mu_shift);
        }

        if (pJT->counter == 4500)
        {
            pJT->mu_shift += 3;
        }
    }

    if (pJT->counter == V34_jt_cnt_val_1[pJT->idx])
    {
        pJT->gain = V34_jt_gain_val_1[pJT->idx++];
    }

    pCoef   = pJT->qdCoef;
    pHead   = &(pJT->dl[pJT->ubOffset]);

    qdTemp1 = 0;

    for (i = 0; i < V34_JTTAPS; i++)
    {
        qdTemp1 += QQMULQD(*pCoef++, *pHead++);
    }

    pJT->qdJTout = QDR15Q(qdTemp1);

    if (adapt == 1)
    {
        muS = 15 + pJT->mu_shift;

        muROUND = (QDWORD) 1 << (muS - 1);
        pCoef = pJT->qdCoef;
        pHead = &(pJT->dl[pJT->ubOffset]);

        for (i = 0; i < V34_JTTAPS; i++)
        {
            qdTemp1 = QQMULQD(pdeQ, *pHead++);

            qTemp = (QWORD)((qdTemp1 + muROUND) >> muS);

            *pCoef++ -= qTemp ;
        }
    }

    jtdiff = pdeQ - pJT->qdJTout;

    pJT->dl[pJT->ubOffset] = jtdiff;
    pJT->dl[pJT->ubOffset + V34_JTTAPS] = jtdiff;
    pJT->ubOffset ++;

    if (pJT->ubOffset >= V34_JTTAPS)
    {
        pJT->ubOffset = 0;
    }

    if (adapt == 1)
    {
        if (pJT->counter > 2000)
        {
            qdTemp1 = QQMULQD(jtdiff, pJT->qdJTout);
            pJT->xcorr1 += (qdTemp1 - pJT->xcorr1) >> 16;

            qdTemp2 = QQMULQD(jtdiff, pdeQ);
            pJT->xcorr2 += (qdTemp2 - pJT->xcorr2) >> 14;
        }
    }

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pJT->qdJTout; }
#endif

    pJT->qdJTout = QQMULQ15(pJT->gain, pJT->qdJTout);

    qdOut = pJT->qdJTout;

    if (qdOut < 0)
    {
        qdOut += qD_360;
    }

    uPhase = (UWORD)QDQMULQD(qdOut, q1_OVER_PI);

    SinCos_Lookup_Fine(uPhase, &pJT->qS, &pJT->qC);
}
#endif
