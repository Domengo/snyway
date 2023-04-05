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

#include "v17ext.h"

/* Signal Mapping Tables */
CONST CQWORD V17_IQTable_4800NR[4] =
{
    { 6,  2},
    { -2,  6},
    { 2, -6},
    { -6, -2}
};

CONST QWORD qV17_DetectToneCoefTab[V17_TONE_NUMBER] =
{
    28378,     /* 600 Hz  */
    -28378,     /* 3000 Hz */
    0,     /* 1800 Hz */
    4277   /* V21 tone 1650 Hz */
};

CONST UBYTE    bit_des[8] = {0, 0, 0, 2, 0, 2, 0, 2};

CONST UBYTE V17_SigTransMapTable[4] =
{
    3,     5,     7,    11
};
