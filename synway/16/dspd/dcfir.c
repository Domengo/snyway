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

#include "dspdext.h"

#if WITH_DC

void Init_DC(DCStruct *pDC)
{
    pDC->DC_Start = 0;
    pDC->DC_Count = 1;
    pDC->qdLpY = 0;
    pDC->qdDcY = 0;
}

#if !USE_ASM

void DcEstimator(DCStruct *pDC, QWORD *pPCM, UBYTE BufSize, SBYTE shift)
{
    UBYTE  i;
    QWORD  dc;
    QDWORD dtemp;

    for (i = 0; i < BufSize; i++)
    {
        dtemp = ((QDWORD)pPCM[i]) << DC_SHIFT_NUM;

        pDC->qdLpY += (dtemp - pDC->qdLpY) >> LP_FILTER_SHIFT;
    }

    pDC->qdDcY += (pDC->qdLpY - pDC->qdDcY) >> (-shift);  /* Because TI only left shift, shift is negative value */

    /* Dc estimator */
    dc = (SWORD)(pDC->qdDcY >> DC_SHIFT_NUM);

    for (i = 0; i < BufSize; i++)
    {
        pPCM[i] -= dc;
    }
}

#endif
#endif
