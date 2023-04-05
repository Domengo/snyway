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

#include "v27ext.h"

CONST SBYTE V27_qPhase_array_4800[8]     = {1, 0, 2, 3, 6, 7, 5, 4};  /* *45 */
CONST SBYTE V27_qPhase_array_2400[4]     = {0, 2, 6, 4};  /* from V27 standard */
CONST SBYTE V27_qPhase_array_Segment4[2] = {0, 4};

CONST SBYTE V27_LOOKUP_DIBITS_1200[4]    = {0, 1, 3, 2};
CONST SBYTE V27_LOOKUP_DIBITS_1600[8]    = {1, 0, 2, 3, 7, 6, 4, 5};

/* The Gcoefs is the Goertzel coefficient for different tones */
/* The way to calculate the coeffs : 32768*cos(2*PI*f/fs)          */
/* --- In 1.15 format ---                      */
/*     f  - is the tone to be detected                  */
/*     fs - is the sampling rate                  */

/* for baud rate 1600: 1000Hz & 2600Hz, and 1800Hz Carrier, V21 tone 1650Hz */
CONST QWORD  V27_DetectTone_1600_Coef[V27_TONE_NUMBER] =
{
    25997, -4277, 12540, 15447
};

/* for baud rate 1200: 1200Hz & 2400Hz, and 1800Hz Carrier, V21 tone 1650Hz */
CONST QWORD  V27_DetectTone_1200_Coef[V27_TONE_NUMBER] =
{
    23170, 0, 12540, 15447
};

CONST CQWORD tV27_Segment4_LOOKUP_IQ[2] =
{
    {V27_MAG0,  0},
    { -V27_MAG0, 0}
};

CONST CQWORD tV27_LOOKUP_IQ[8] =
{
    {V27_MAG0,            0},
    {V27_MAG45,   V27_MAG45},
    {0,            V27_MAG0},
    { -V27_MAG45,  V27_MAG45},
    { -V27_MAG0,           0},
    { -V27_MAG45, -V27_MAG45},
    {0,           -V27_MAG0},
    {V27_MAG45,  -V27_MAG45}
};

CONST CQWORD tV27_IQTable_Decode_4800[8] =
{
    {V27_MAG0, 0     },
    {0, V27_MAG0     },
    { -V27_MAG0, 0    },
    {0, -V27_MAG0    },
    {V27_MAG45, V27_MAG45  },
    { -V27_MAG45, V27_MAG45 },
    { -V27_MAG45, -V27_MAG45},
    {V27_MAG45, -V27_MAG45 }
};

CONST CQWORD tV27_IQTable_Decode_2400[4] =
{
    {V27_MAG0, 0     },
    {0, V27_MAG0     },
    { -V27_MAG0, 0    },
    {0, -V27_MAG0    }
};

CONST SWORD tV27_low_pass_coef_1600[V27_FILTER_LENGTH] =
{
    0,   0,     0, 32767,     0,     0,   0,    0,
    47, 629, -3204, 31087,  5301, -1164,  68,   96,
    134, 754, -4355, 26401, 12194, -2640, 277,  171,
    187, 562, -3933, 19667, 19667, -3933, 562,  187,
    171, 277, -2640, 12194, 26401, -4355, 754,  134,
    96,  68, -1164,  5301, 31087, -3204, 629,   47
};

CONST SWORD tV27_low_pass_coef_1200[V27_FILTER_LENGTH] =
{
    263,      -1402,      32460,       1706,       -334,         55,
    582,      -3294,      30560,       5958,      -1196,        173,
    670,      -4066,      26992,      11093,      -2223,        310,
    604,      -3944,      22183,      16685,      -3228,        464,
    464,      -3228,      16685,      22183,      -3944,        604,
    310,      -2223,      11093,      26992,      -4066,        670,
    173,      -1196,       5958,      30560,      -3294,        582,
    55,       -334,       1706,      32460,      -1402,        263
};
