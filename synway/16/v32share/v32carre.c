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

#include "v3217ext.h"

void V32Share_Dummy(V32ShareStruct *pV32Share)
{
}

void V32_CarrierRecovery_init(V32ShareStruct *pV32Share)
{
    pV32Share->uRotatePhase = 0;
    pV32Share->qCarErrPhase = 0;
    pV32Share->qRotateCos   = 0x7FFF;
    pV32Share->qRotateSin   = 0;
}

void V32_CarrierUpdate(V32ShareStruct *pV32Share, CQWORD cPhaseIQ)
{
    CQWORD cqErrorIQ;
    QDWORD qdCr_temp;
    QWORD  qErrPhaseIn;

    cqErrorIQ.r = pV32Share->cqRotateIQ.r;   /* output of the rotate */
    cqErrorIQ.i = pV32Share->cqRotateIQ.i;

    /* carrier recovery input error */
    qdCr_temp   = QQMULQD(cPhaseIQ.r, cqErrorIQ.i);
    qdCr_temp  -= QQMULQD(cPhaseIQ.i, cqErrorIQ.r);
    qErrPhaseIn = QDR15Q(qdCr_temp);

#if 0

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV32Share->uRotatePhase; }

#endif

    if (pV32Share->ubTimErr1Freeze == 0)
    {
        pV32Share->qCarErrPhase += qErrPhaseIn;
    }

#if V32_FREQUENCY_OFFSET

    if (pV32Share->ubRotateCountEnable)
    {
        pV32Share->sdRotateAcc += ((qErrPhaseIn >> pV32Share->ubTimShift) + (pV32Share->qCarErrPhase >> V32_SIGMAP_SCALE));
        pV32Share->uRotateCount++;

        if (pV32Share->uRotateCount == 1024)
        {
            pV32Share->nDemodDeltaOffset = (QWORD)QDQMULQD(pV32Share->sdRotateAcc, q1_OVER_12);

            pV32Share->ubRotateCountEnable = 0;
        }
    }

#endif

    pV32Share->uRotatePhase += ((qErrPhaseIn >> pV32Share->ubTimShift) + (pV32Share->qCarErrPhase >> V32_SIGMAP_SCALE));

    /* get cos, sin value for rotation */
    SinCos_Lookup_Fine(pV32Share->uRotatePhase, &pV32Share->qRotateSin, &pV32Share->qRotateCos);
}

void V32_CarUpdateUseTrainIQ(V32ShareStruct *pV32Share)
{
    V32_CarrierUpdate(pV32Share, pV32Share->cqTrainIQ);
}

void V32_CarUpdateUseSliceIQ(V32ShareStruct *pV32Share) /* use slicer symbol for carrier update */
{
    V32_CarrierUpdate(pV32Share, pV32Share->cqSliceIQ);
}

void V32_Rotate_Temp(V32ShareStruct *pV32Share)
{
    QDWORD tmp;

    tmp = DSPD_Atan2((QWORD)pV32Share->qdCarr_r, (QWORD)pV32Share->qdCarr_i);
    tmp = QDQMULQDR(tmp, q1_OVER_PI);

    pV32Share->uRotatePhase = (UWORD)(tmp - 44316);

    SinCos_Lookup_Fine(pV32Share->uRotatePhase, &pV32Share->qRotateSin, &pV32Share->qRotateCos);
}
