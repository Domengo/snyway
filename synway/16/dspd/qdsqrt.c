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

QWORD QDsqrt(UDWORD qX)  /* Calculat qSqrt ** 2 = qX/2 */
{
    UDWORD c;
    QWORD qSqrt, a, b;

    qX >>= 1;

    if (qX > 32767)
    {
        b = 32767;
    }
    else
    {
        b = (QWORD)qX;
    }

    qSqrt = 0;
    a = 0;

    while ((b - a) > 1)
    {
        qSqrt = (a + b) >> 1;

        c = QQMULQD(qSqrt, qSqrt);

        if (c > qX)
        {
            b = qSqrt;
        }
        else
        {
            a = qSqrt;
        }
    }

    if ((QQMULQD(b, b) - qX) > (qX - QQMULQD(a, a)))
    {
        qSqrt = a;
    }
    else
    {
        qSqrt = b;
    }

    return(qSqrt);
}
