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

#include <string.h>
#include "v34ext.h"

#if 0 /// LLL temp
/* This Function is just for Short Equalizer */
void  V34_Cleanup_Equalizer(QCFIRStruct *eqfir)
{
    CQWORD *pcCoef;
    CQWORD *pcDline;
    UBYTE i;

    pcCoef = eqfir->pcCoef;
    pcDline = eqfir->pcDline;

    for (i = 0; i < V34_EQ_S_LEN; i++)
    {
        pcCoef[i].r = 0;
        pcCoef[i].i = 0;
    }

    for (i = 0; i < (V34_EQ_S_LEN << 1); i++)
    {
        pcDline[i].r = 0;
        pcDline[i].i = 0;
    }
}
#endif

/***********************************************************************
* INPUT: CQWORD cqErr : cqErr = Decoder output - Eq output
*                       If use viterbi decoder, use delay value of Eq
*                       output since viterbi introduce delay.
*        QWORD qBeta : The adaptation constant
*        cFILTER cfir : The Eq filter delayline and pcCoef. structure
*
* OUTPUT: The Eq pcCoef is updated
*F!********************************************************************/
void V34_2T3_Equalizer_Update(CQWORD cqErr, QWORD qBeta, TimeCarrRecovStruc *pTCR)
{
    QDWORD temp;
    QWORD  qS, qC;
    CQWORD cqRotErr;

    /* First de-rotate error symbol */
    qS = - pTCR->qS;
    qC =   pTCR->qC;

    temp  = QQMULQD(cqErr.r, qC);
    temp += QQMULQD(cqErr.i, qS);
    cqRotErr.r = QDR15Q(temp);

    temp  = QQMULQD(cqErr.i, qC);
    temp -= QQMULQD(cqErr.r, qS);
    cqRotErr.i = QDR15Q(temp);

    DspcFirLMS_2T3(&pTCR->eqfir, qBeta, &cqRotErr);
}

#if RX_PRECODE_ENABLE

void V34_NoiseWhiten_Init(NoiseWhitenStruct *pNW)
{
    UBYTE i;

    for (i = 0; i < V34_PRECODE_COEF_LEN; i++)
    {
        pNW->cNWCoef[i].r = 0;
        pNW->cNWCoef[i].i = 0;
    }

    for (i = 0; i < (V34_PRECODE_COEF_LEN << 1); i++)
    {
        pNW->cNWDline[i].r = 0;
        pNW->cNWDline[i].i = 0;
    }

    pNW->NWDlineIdx  = 0;
    pNW->ubBetaShift = 0;
}

void V34_NoiseWhiten(NoiseWhitenStruct *pNW, CQWORD *pcErr)
{
    CQWORD cqOut, cErrIn;
    CQWORD *pcCoef;
    CQWORD *pcDline;
    QDWORD MacR, MacI;
    QWORD  qTempr, qTempi, qBetaShiftRound;
    SWORD  sShift;
    UWORD  i;

    cErrIn.r = pcErr->r;
    cErrIn.i = pcErr->i;

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cErrIn.r; }
#endif

    pcCoef  = pNW->cNWCoef;
    pcDline = &pNW->cNWDline[pNW->NWDlineIdx];
    MacR = 0;
    MacI = 0;

    for (i = 0; i < V34_PRECODE_COEF_LEN; i++)
    {
        MacR += QQMULQD(pcDline->r, pcCoef->r);
        MacR -= QQMULQD(pcDline->i, pcCoef->i);

        MacI += QQMULQD(pcDline->r, pcCoef->i);
        MacI += QQMULQD(pcDline->i, pcCoef->r);

        ++pcDline;
        ++pcCoef;
    }

    cqOut.r = (QWORD)(ROUND14(MacR));
    cqOut.i = (QWORD)(ROUND14(MacI));

    pcErr->r += cqOut.r;
    pcErr->i += cqOut.i;

    if (pNW->ubBetaShift)
    {
        sShift = pNW->ubBetaShift;
        qBetaShiftRound = ((QWORD)1 << (sShift - 1));

        qTempr = pcErr->r;
        qTempi = pcErr->i;

        pcCoef  = pNW->cNWCoef;
        pcDline = &pNW->cNWDline[pNW->NWDlineIdx];

        for (i = 0; i < V34_PRECODE_COEF_LEN; i++)
        {
            MacR  = QQMULQD(qTempr, pcDline->r);
            MacR += QQMULQD(qTempi, pcDline->i);

            pcCoef->r -= (QWORD)((MacR + qBetaShiftRound) >> sShift);

            MacI  = QQMULQD(qTempi, pcDline->r);
            MacI -= QQMULQD(qTempr, pcDline->i);

            pcCoef->i -= (QWORD)((MacI + qBetaShiftRound) >> sShift);

            pcDline++;
            pcCoef++;
        }
    }

    /* Update delayline with Ref symbol */
    pcDline = &pNW->cNWDline[pNW->NWDlineIdx++];

    pcDline[0]                    = cErrIn;
    pcDline[V34_PRECODE_COEF_LEN] = cErrIn;

    if (pNW->NWDlineIdx >= V34_PRECODE_COEF_LEN)
    {
        pNW->NWDlineIdx = 0;
    }
}

#endif