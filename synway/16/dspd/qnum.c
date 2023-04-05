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

void Number100_10_1(UWORD Num, UBYTE UnitsPlace[])
{
    UBYTE temp;

    UnitsPlace[0] = (UBYTE)QQMULQ15(Num, q001);

    temp          = (UBYTE)(Num - UBUBMULU(UnitsPlace[0], 100));

    UnitsPlace[1] = (UBYTE)QQMULQ15(temp, q01);

    UnitsPlace[2] = temp - UBUBMULUB(UnitsPlace[1], 10);
}
