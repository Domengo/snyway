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

#include "shareext.h"

#if 0
void  V34_EcRef_Insert(EC_InsertStru *Ec_Insert, CQWORD sym, CQWORD *pcBulk_delay)
{
    UWORD  uIdx;
    CQWORD cTemp;
    QWORD temp1;
    QWORD temp2;

    temp1 = QQMULQR15(Ec_Insert->qCos, sym.r);
    temp2 = QQMULQR15(Ec_Insert->qSin, sym.i);
    cTemp.r = temp1 - temp2;

    temp1 = QQMULQR15(Ec_Insert->qCos, sym.i);
    temp2 = QQMULQR15(Ec_Insert->qSin, sym.r);
    cTemp.i = temp1 + temp2;

    Ec_Insert->bulk_inidx ++;

    if (Ec_Insert->bulk_inidx == Ec_Insert->max_bulk_len)
    {
        Ec_Insert->bulk_inidx = Ec_Insert->ne_sublen;
    }

    pcBulk_delay[Ec_Insert->bulk_inidx].r = cTemp.r;
    pcBulk_delay[Ec_Insert->bulk_inidx].i = cTemp.i;

    if (Ec_Insert->bulk_inidx >= (Ec_Insert->max_bulk_len - Ec_Insert->ne_sublen))
    {
        uIdx = Ec_Insert->bulk_inidx - Ec_Insert->max_bulk_len + Ec_Insert->ne_sublen;

        pcBulk_delay[uIdx].r = cTemp.r;
        pcBulk_delay[uIdx].i = cTemp.i;
    }
}
#endif