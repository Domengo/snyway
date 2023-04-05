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

QDWORD  DSPD_Log10(QDWORD qX)
{
    QWORD  i;
    QWORD  qDx;
    QWORD  qTemp;
    QDWORD qdResult;

    if (qX <= 0)
    {
        return (-Q31_MAX);
    }

    i = 0;
    qdResult = qX;

    while (qdResult > qTWO)
    {
        i++;
        qdResult >>= 1;
    }

    while (qdResult < qONE)
    {
        i--;
        qdResult <<= 1;
    }

    qDx = (QWORD)(qdResult - (QDWORD)49152);

    qdResult = QQMULQD(qLOG2 , i); /* qResult = 0.301029995664 * i */

    qTemp  = QQMULQR15(qDx, LOG_CONST);
    qTemp -= 3162;
    qTemp  = QQMULQR15(qDx, qTemp);
    qTemp += 9487;
    qTemp  = QQMULQR15(qDx, qTemp);
    qTemp += 5770;

    qdResult += qTemp;

    return(qdResult);
}
