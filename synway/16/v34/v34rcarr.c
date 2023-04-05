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

#include "v34ext.h"

void V34_CarrierLoop_Init(TimeCarrRecovStruc *pTCR)
{
    pTCR->udError_phase_out = 0;
    pTCR->qdError_phase     = 0;

    pTCR->qS = 0;     /* initial SIN 0 = 0 */
    pTCR->qC = qONE;  /* initial COS 0 = 1 */

    pTCR->qCarAcoef = 17039;/* 0.52; */
    pTCR->qCarBcoef = 15729;/* 0.48; */
    pTCR->qB3       = V34_qCARRIERLOOP_B3;
}

void V34_CarrierLoop(TimeCarrRecovStruc *pTCR, CQWORD *pcUs, CQWORD *pcUt)
{
    UWORD  uError_phase_out;
    QWORD  qError_phase_input;
    QDWORD qdTemp;
    QWORD  qTemp;

    qdTemp  = QQMULQD(pcUt->r, pcUs->i);
    qdTemp -= QQMULQD(pcUt->i, pcUs->r);

    qTemp   = QD15Q(qdTemp);

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pcUt->r - pcUs->r; }

#endif

    qError_phase_input = QQMULQ15(qTemp, q1_OVER_PI);

    /* The first order PLL */
    qdTemp = QQMULQD(pTCR->qCarBcoef, qError_phase_input);

    pTCR->qdError_phase = qdTemp + QDQMULQD(pTCR->qdError_phase, pTCR->qCarAcoef);

    /* The second order PLL */
    qdTemp  = QQMULQD(pTCR->qB3, qError_phase_input);
    qdTemp += pTCR->qdError_phase;

    pTCR->udError_phase_out = (UDWORD)(pTCR->udError_phase_out + qdTemp) & 0x7FFFFFFF;  /* 0 <= pTCR->udError_phase_out <= 0x7FFFFFFF */

#if 0

    if (DumpTone1_Idx < 1000000) { DumpTone1[DumpTone1_Idx++] = pTCR->udError_phase_out >> 1; }

    if (DumpTone2_Idx < 1000000) { DumpTone2[DumpTone2_Idx++] = pTCR->qdError_phase; }

    if (DumpTone3_Idx < 1000000) { DumpTone3[DumpTone3_Idx++] = qdTemp; }

#endif

    uError_phase_out = (UWORD)(ROUND15(pTCR->udError_phase_out) & 0xFFFF);
    SinCos_Lookup_Fine(uError_phase_out, &pTCR->qS, &pTCR->qC);
}

#if !USE_ASM
CQWORD  V34_Rotator(CQWORD *input, QWORD qS, QWORD qC)
{
    CQWORD cqOut;
    QDWORD qdTemp;

    qdTemp  = QQMULQD(input->r, qC);
    qdTemp += QQMULQD(input->i, qS);
    cqOut.r = QDR15Q(qdTemp);

    qdTemp  = QQMULQD(input->i, qC);
    qdTemp -= QQMULQD(input->r, qS);
    cqOut.i = QDR15Q(qdTemp);

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cqOut.r; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = cqOut.i; }

#endif

    return(cqOut);
}
#endif
