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

/* ----------------------------------------------------------------------- */
/* Carrier Recovery Update                                                 */
/* ----------------------------------------------------------------------- */

#include "v27ext.h"

void V27_CarrierRecovery_init(V27Struct *pV27)
{
    pV27->uRotatePhase = 0;
    pV27->qCarErrPhase = 0;
    pV27->qRotateCos   = 0x7FFF;
    pV27->qRotateSin   = 0;
}

void V27_CarrierUpdate(V27Struct *pV27, CQWORD cPhaseIQ)
{
    CQWORD cqErrorIQ;
    QDWORD qdCr_temp;
    QWORD  qErrPhaseIn;

    cqErrorIQ.r = pV27->cqRotateIQ.r;   /* output of the rotate */
    cqErrorIQ.i = pV27->cqRotateIQ.i;

    /* carrier recovery input error */
    qdCr_temp   = QQMULQD(cPhaseIQ.r, cqErrorIQ.i);
    qdCr_temp  -= QQMULQD(cPhaseIQ.i, cqErrorIQ.r);
    qErrPhaseIn = QDR15Q(qdCr_temp);

    if (pV27->ubTimErr1Freeze == 0)
    {
        pV27->qCarErrPhase += qErrPhaseIn;
    }

    pV27->uRotatePhase += ((qErrPhaseIn >> 2) + (pV27->qCarErrPhase >> 9));

    /* get cos, sin value for rotation */
    SinCos_Lookup_Fine(pV27->uRotatePhase, &pV27->qRotateSin, &pV27->qRotateCos);

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = qErrPhaseIn; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV27->qCarErrPhase; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV27->uRotatePhase; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = cPhaseIQ.r; }

#endif
}


void V27_CarUpdateUsingTrainIQ(V27Struct *pV27)
{
    V27_CarrierUpdate(pV27, pV27->cqTrainIQ);
}

void V27_CarUpdateUsingSliceIQ(V27Struct *pV27) /* use slicer symbol for carrier update */
{
    V27_CarrierUpdate(pV27, pV27->cqSliceIQ);
}
