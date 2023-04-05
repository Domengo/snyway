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

#include "v29ext.h"

void V29_CarrierRecovery_init(V29Struct *pV29)
{
    pV29->uRotatePhase = 0;
    pV29->qdCarErrPhase = 0;
    pV29->qRotateCos   = 0x7FFF;
    pV29->qRotateSin   = 0;
    pV29->qCarAcoef    = 30000;
    pV29->qCarBcoef    = 2768;
}

void V29_CarrierUpdate(V29Struct *pV29)
{
    QDWORD qdCr_temp;
    QWORD  qErrPhaseIn;
    CQWORD cqSliceIQ;
    CQWORD cqErrorIQ;
    QDWORD qdTemp;

    cqSliceIQ.r = pV29->cqSliceIQ.r;
    cqSliceIQ.i = pV29->cqSliceIQ.i;

    cqErrorIQ.r = pV29->cqRotateIQ.r;    /* output of the rotate */
    cqErrorIQ.i = pV29->cqRotateIQ.i;

    /* carrier recovery input error */
    qdCr_temp   = QQMULQD(cqSliceIQ.r, cqErrorIQ.i);
    qdCr_temp  -= QQMULQD(cqSliceIQ.i, cqErrorIQ.r);
    qErrPhaseIn = QD15Q(qdCr_temp);
    qErrPhaseIn = QQMULQ15(qErrPhaseIn, q1_OVER_PI);

    qdTemp = QQMULQD(pV29->qCarBcoef, qErrPhaseIn);
    qdTemp += QDQMULQD(pV29->qdCarErrPhase, pV29->qCarAcoef);
    qdTemp += QQMULQD(106, qErrPhaseIn);

    if (pV29->ubTimErr1Freeze == 0)
    {
        pV29->qdCarErrPhase = qdTemp;
    }
    else
    {
        qdTemp = 0;
    }

    pV29->uRotatePhase += (UWORD)(QD15Q(qdTemp));

    /* get cos, sin value for rotation */
    SinCos_Lookup_Fine(pV29->uRotatePhase, &pV29->qRotateSin, &pV29->qRotateCos);

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = qErrPhaseIn; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV29->qCarErrPhase; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV29->uRotatePhase; }

#endif
}
