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
/* De-rotater                                                              */
/*                                                                         */
/* INPUT:   'cqRotateIQ' - rotated EQ output                               */
/*          'cqSliceIQ'  - slicer output                                   */
/*          'TrainIQ'  - training symbol                                   */
/*                                                                         */
/* OUTPUT: 'EqErrorIQ' which is the de-rotated equalizer error.            */
/*                                                                         */
/***************************************************************************/

#include "v3217ext.h"

void V32_DerotateErr(V32ShareStruct *pV32Share)
{
    QDWORD tempR, tempI;

    tempR  = QQMULQD(pV32Share->cqEqErrorIQ.r, pV32Share->qRotateCos);
    tempR -= QQMULQD(pV32Share->cqEqErrorIQ.i, pV32Share->qRotateSin);

    tempI  = QQMULQD(pV32Share->cqEqErrorIQ.i, pV32Share->qRotateCos);
    tempI += QQMULQD(pV32Share->cqEqErrorIQ.r, pV32Share->qRotateSin);

    pV32Share->cqEqErrorIQ.r = QDR15Q(tempR);
    pV32Share->cqEqErrorIQ.i = QDR15Q(tempI);

#if 0

    if (DumpTone1_Idx < 5000000) { DumpTone1[DumpTone1_Idx++] = pV32Share->cqEqErrorIQ.r; }

    if (DumpTone2_Idx < 5000000) { DumpTone2[DumpTone2_Idx++] = pV32Share->cqEqErrorIQ.i; }

#endif
}

/* ---------- De-rotate ---------- */

void V32_Derotate_Get_TRN(V32ShareStruct *pV32Share)
{
    /* find error */
    pV32Share->cqEqErrorIQ.r = pV32Share->cqTrainIQ.r - pV32Share->cqRotateIQ.r;
    pV32Share->cqEqErrorIQ.i = pV32Share->cqTrainIQ.i - pV32Share->cqRotateIQ.i;

    V32_DerotateErr(pV32Share);
}

void V32_Derotate(V32ShareStruct *pV32Share)
{
    /* find error */
    pV32Share->cqEqErrorIQ.r = pV32Share->cqSliceIQ.r - pV32Share->cqRotateIQ.r;
    pV32Share->cqEqErrorIQ.i = pV32Share->cqSliceIQ.i - pV32Share->cqRotateIQ.i;

#if (SUPPORT_V32 + SUPPORT_V32BIS)
    pV32Share->qdCarrLossErrAcc += (QQMULQD(pV32Share->cqEqErrorIQ.i, pV32Share->cqEqErrorIQ.i) + QQMULQD(pV32Share->cqEqErrorIQ.r, pV32Share->cqEqErrorIQ.r)) >> 2;

    if (pV32Share->qdCarrLossErrAcc > Q30_MAX)
    {
        pV32Share->qdCarrLossErrAcc = Q30_MAX;
    }

#endif

    V32_DerotateErr(pV32Share);
}
