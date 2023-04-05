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
/*  This file initilises the derorate and determines the equalizer errors. */
/***************************************************************************/

#include "v27ext.h"

void V27_DerotateErr(V27Struct *pV27)
{
    QDWORD qdTempR, qdTempI;

    qdTempR  = QQMULQD(pV27->cqEqErrorIQ.r, pV27->qRotateCos);
    qdTempR -= QQMULQD(pV27->cqEqErrorIQ.i, pV27->qRotateSin);

    qdTempI  = QQMULQD(pV27->cqEqErrorIQ.i, pV27->qRotateCos);
    qdTempI += QQMULQD(pV27->cqEqErrorIQ.r, pV27->qRotateSin);

    pV27->cqEqErrorIQ.r = QDR15Q(qdTempR);
    pV27->cqEqErrorIQ.i = QDR15Q(qdTempI);
}

void V27_RX_Derotate_Get_TRN(V27Struct *pV27)
{
    /* find error */
    pV27->cqEqErrorIQ.r = (pV27->cqTrainIQ.r) - pV27->cqRotateIQ.r;
    pV27->cqEqErrorIQ.i = (pV27->cqTrainIQ.i) - pV27->cqRotateIQ.i;

    /* derotate error */
    V27_DerotateErr(pV27);
}

void  V27_RX_Derotate(V27Struct *pV27)
{
    /* find error */
    pV27->cqEqErrorIQ.r = pV27->cqSliceIQ.r - pV27->cqRotateIQ.r;
    pV27->cqEqErrorIQ.i = pV27->cqSliceIQ.i - pV27->cqRotateIQ.i;

    V27_DerotateErr(pV27);
}
