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

/****************************************************************
This file defines all the values for costable,hilbert coefficients
and signal demapping for the received samples.
****************************************************************/

#include "v29ext.h"

CONST SBYTE Phase_array[8] = {1, 0, 2, 3, 6, 7, 5, 4};
CONST SBYTE Q234_Array[8]  = {1, 0, 2, 3, 7, 6, 4, 5};

#if V29_MODEM_ENABLE
CONST SBYTE Phase4800_array[4] = {0, 2, 6, 4};
CONST SBYTE Q23_Array[4]       = {0, 1, 3, 2};
#endif

/* The Gcoefs is the Goertzel coefficient for different tones */
/* The way to calculate the coeffs : 32768*cos(2*PI*f/fs)      */
/* --- In 1.15 format ---                                      */
/*     f  - is the tone to be detected                          */
/*     fs - is the sampling rate                              */
CONST QWORD  V29_DetectTone_Coef[V29_TONE_NUMBER] =
{
    29698,     /* 500  Hz */
    -26842,     /* 2900 Hz */
    2856,     /* 1700 Hz */
    4277      /* V21 1650Hz Mark Tone */
};

CONST CQWORD V29_tIQTable_Decode_9600[16] =
{
    { (5 * V29_MAPPER_9600), (0)},
    { (0), (5 * V29_MAPPER_9600)},
    { (-5 * V29_MAPPER_9600), (0)},
    { (0), (-5 * V29_MAPPER_9600)},

    { (3 * V29_MAPPER_9600), (3 * V29_MAPPER_9600)},
    { (-3 * V29_MAPPER_9600), (3 * V29_MAPPER_9600)},
    { (-3 * V29_MAPPER_9600), (-3 * V29_MAPPER_9600)},
    { (3 * V29_MAPPER_9600), (-3 * V29_MAPPER_9600)},

    { (3 * V29_MAPPER_9600), (0)},
    { (0), (3 * V29_MAPPER_9600)},
    { (-3 * V29_MAPPER_9600), (0)},
    { (0), (-3 * V29_MAPPER_9600)},

    { (V29_MAPPER_9600), (V29_MAPPER_9600)},
    { (-    V29_MAPPER_9600), (V29_MAPPER_9600)},
    { (-    V29_MAPPER_9600), (-    V29_MAPPER_9600)},
    { (V29_MAPPER_9600), (-    V29_MAPPER_9600)}
};

CONST CQWORD V29_tIQTable_Decode_7200[8] =
{
    { (3 * V29_MAPPER_7200), (0)},
    { (0)                  , (3 * V29_MAPPER_7200)},
    { (-3 * V29_MAPPER_7200), (0)},
    { (0)                  , (-3 * V29_MAPPER_7200)},

    {  V29_MAPPER_7200,  V29_MAPPER_7200},
    { -V29_MAPPER_7200,  V29_MAPPER_7200},
    { -V29_MAPPER_7200, -V29_MAPPER_7200},
    {  V29_MAPPER_7200, -V29_MAPPER_7200}
};

#if V29_MODEM_ENABLE
CONST CQWORD V29_tIQTable_Decode_4800[4] =
{
    { (3 * V29_MAPPER_4800), (0)},
    { (0)                  , (3 * V29_MAPPER_4800)},
    { (-3 * V29_MAPPER_4800), (0)},
    { (0)                  , (-3 * V29_MAPPER_4800)},
};
#endif

CONST SBYTE V29_tAbsolutePhase[16] =
{
    0, 2, 4, 6, 1, 3, 5, 7,
    0, 2, 4, 6, 1, 3, 5, 7
};
