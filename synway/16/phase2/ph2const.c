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

#include "ph2ext.h"


/* Low pass filter */
CONST QWORD V34_tDPSK_LP[48] =
{
#if PH2_NEW_LOWPASS_FILT_COEF
    /* 9600 sampling rate, 330 pass band, 450 stop band
     * 0.01 passband ripple, 30 stopband attenuation
     * Kaiser window, 48 taps
     * P2FIR_GAIN 8 or 7
     */
    -51 * P2FIR_GAIN,
    -106 * P2FIR_GAIN,
    -167 * P2FIR_GAIN,
    -229 * P2FIR_GAIN,
    -286 * P2FIR_GAIN,
    -333 * P2FIR_GAIN,
    -361 * P2FIR_GAIN,
    -365 * P2FIR_GAIN,
    -338 * P2FIR_GAIN,
    -276 * P2FIR_GAIN,
    -174 * P2FIR_GAIN,
    -32 * P2FIR_GAIN,
    150 * P2FIR_GAIN,
    369 * P2FIR_GAIN,
    619 * P2FIR_GAIN,
    891 * P2FIR_GAIN,
    1177 * P2FIR_GAIN,
    1464 * P2FIR_GAIN,
    1741 * P2FIR_GAIN,
    1995 * P2FIR_GAIN,
    2214 * P2FIR_GAIN,
    2389 * P2FIR_GAIN,
    2510 * P2FIR_GAIN,
    2572 * P2FIR_GAIN,
    2572 * P2FIR_GAIN,
    2510 * P2FIR_GAIN,
    2389 * P2FIR_GAIN,
    2214 * P2FIR_GAIN,
    1995 * P2FIR_GAIN,
    1741 * P2FIR_GAIN,
    1464 * P2FIR_GAIN,
    1177 * P2FIR_GAIN,
    891 * P2FIR_GAIN,
    619 * P2FIR_GAIN,
    369 * P2FIR_GAIN,
    150 * P2FIR_GAIN,
    -32 * P2FIR_GAIN,
    -174 * P2FIR_GAIN,
    -276 * P2FIR_GAIN,
    -338 * P2FIR_GAIN,
    -365 * P2FIR_GAIN,
    -361 * P2FIR_GAIN,
    -333 * P2FIR_GAIN,
    -286 * P2FIR_GAIN,
    -229 * P2FIR_GAIN,
    -167 * P2FIR_GAIN,
    -106 * P2FIR_GAIN,
    -51 * P2FIR_GAIN
#else
    7,   -311,   -679,
    -1071,  -1439,  -1735,
    -1911,  -1919,  -1711,
    -1239,   -487,    551,
    1895,   3495,   5327,
    7327,   9423,  11535,
    13567,  15431,  17046,
    18334,  19222,  19678,
    19678,  19222,  18334,
    17046,  15431,  13567,
    11535,   9423,   7327,
    5327,   3495,   1895,
    551,   -487,  -1239,
    -1711,  -1919,  -1911,
    -1735,  -1439,  -1071,
    -679,   -311,      7
#endif
};

CONST SWORD V34_tDPSK_HM_Coef_Call[V34_FILTER_LENGTH_1] =
{
    - 1912, 19680, -1704,     0,
    -1912, 19224, -1240,   152,
    -1736, 18328,  -488,   304,
    -1432, 17048,   552,   424,
    -1072, 15424,  1888,   488,
    -680, 13568,  3496,   496,
    -304, 11528,  5328,   416,
    8,  9424,  7328,   256,
    256,  7328,  9424,     8,
    416,  5328, 11528,  -304,
    496,  3496, 13568,  -680,
    488,  1888, 15424, -1072,
    424,   552, 17048, -1432,
    304,  -488, 18328, -1736,
    152, -1240, 19224, -1912,
    0, -1704, 19680, -1912
};

CONST SWORD V34_tDPSK_HM_Coef_Ans[V34_FILTER_LENGTH_1] =
{
    - 1704, 17540, -1519,     0,
    -1704, 17133, -1105,   135,
    -1547, 16335,  -435,   271,
    -1276, 15194,   492,   378,
    -955, 13747,  1683,   435,
    -606, 12092,  3116,   442,
    -271, 10274,  4749,   371,
    7,  8399,  6531,   228,
    228,  6531,  8399,     7,
    371,  4749, 10274,  -271,
    442,  3116, 12092,  -606,
    435,  1683, 13747,  -955,
    378,   492, 15194, -1276,
    271,  -435, 16335, -1547,
    135, -1105, 17133, -1704,
    0, -1519, 17540, -1704
};

#if 0
CONST QWORD V34_tDPSK_CMCOEF[V34_DPSK_CALL_LEN] =
{
    -279,   0,      -449,   0,     666,
    0,      -873,   0,      858,   0,
    -456,   0,      -399,   0,     1651,
    0,      -3102,  0,      4465,  0,
    -5437,  0,      5790,   0,     -5437,
    0,      4465,   0,      -3102, 0,
    1651,   0,      -399,   0,     -456,
    0,      858,    0,      -873,  0,
    666,    0,      -449,   0,     -279
};
#else
CONST QWORD V34_tDPSK_CMCOEF[V34_DPSK_CALL_LEN] =
{
    (QWORD)0x0000, (QWORD)0xFFF2, (QWORD)0x0000, (QWORD)0xFFE4, (QWORD)0x0000, (QWORD)0x0079, (QWORD)0x0000, (QWORD)0xFEEE,
    (QWORD)0x0000, (QWORD)0x01DF, (QWORD)0x0000, (QWORD)0xFD4E, (QWORD)0x0000, (QWORD)0x0342, (QWORD)0x0000, (QWORD)0xFCCF,
    (QWORD)0x0000, (QWORD)0x0229, (QWORD)0x0000, (QWORD)0x0008, (QWORD)0x0000, (QWORD)0xFCA8, (QWORD)0x0000, (QWORD)0x0776,
    (QWORD)0x0000, (QWORD)0xF434, (QWORD)0x0000, (QWORD)0x0FA3, (QWORD)0x0000, (QWORD)0xEDB8, (QWORD)0x0000, (QWORD)0x133A,
    (QWORD)0x0000, (QWORD)0xEDB8, (QWORD)0x0000, (QWORD)0x0FA3, (QWORD)0x0000, (QWORD)0xF434, (QWORD)0x0000, (QWORD)0x0776,
    (QWORD)0x0000, (QWORD)0xFCA8, (QWORD)0x0000, (QWORD)0x0008, (QWORD)0x0000, (QWORD)0x0229, (QWORD)0x0000, (QWORD)0xFCCF,
    (QWORD)0x0000, (QWORD)0x0342, (QWORD)0x0000, (QWORD)0xFD4E, (QWORD)0x0000, (QWORD)0x01DF, (QWORD)0x0000, (QWORD)0xFEEE,
    (QWORD)0x0000, (QWORD)0x0079, (QWORD)0x0000, (QWORD)0xFFE4, (QWORD)0x0000, (QWORD)0xFFF2, (QWORD)0x0000
};
#endif

#if 0
CONST QWORD V34_tDPSK_ANCOEF[V34_DPSK_ANS_LEN] =
{
    172,    -208,   355,    349,   -65,
    -619,   -914,   -667,   -30,   464,
    416,    31,     -12,    695,   1650,
    1672,   2,      -2683,  -4443, -3531,
    -5,     4000,   5749,   4000,  -5,
    -3531,  -4443,  -2683,  2,     1672,
    1650,   695,    -12,    31,    416,
    464,    -30,    -667,   -914,  -619,
    -65,    349,    355,    -208,  172
};
#else
CONST QWORD V34_tDPSK_ANCOEF[V34_DPSK_ANS_LEN] =
{
    (QWORD)0x0008, (QWORD)0x0002, (QWORD)0x0006, (QWORD)0x0023, (QWORD)0x0033, (QWORD)0xFFFD, (QWORD)0xFF71, (QWORD)0xFEE3,
    (QWORD)0xFEF3, (QWORD)0x0000, (QWORD)0x01A5, (QWORD)0x02B8, (QWORD)0x0227, (QWORD)0x0000, (QWORD)0xFDA8, (QWORD)0xFCD4,
    (QWORD)0xFE07, (QWORD)0x0000, (QWORD)0x00D4, (QWORD)0xFFEF, (QWORD)0xFEDF, (QWORD)0xFFFF, (QWORD)0x03C7, (QWORD)0x0777,
    (QWORD)0x06D1, (QWORD)0x0002, (QWORD)0xF643, (QWORD)0xF06C, (QWORD)0xF3EB, (QWORD)0xFFFF, (QWORD)0x0D5D, (QWORD)0x1323,
    (QWORD)0x0D5D, (QWORD)0xFFFF, (QWORD)0xF3EB, (QWORD)0xF06C, (QWORD)0xF643, (QWORD)0x0002, (QWORD)0x06D1, (QWORD)0x0777,
    (QWORD)0x03C7, (QWORD)0xFFFF, (QWORD)0xFEDF, (QWORD)0xFFEF, (QWORD)0x00D4, (QWORD)0x0000, (QWORD)0xFE07, (QWORD)0xFCD4,
    (QWORD)0xFDA8, (QWORD)0x0000, (QWORD)0x0227, (QWORD)0x02B8, (QWORD)0x01A5, (QWORD)0x0000, (QWORD)0xFEF3, (QWORD)0xFEE3,
    (QWORD)0xFF71, (QWORD)0xFFFD, (QWORD)0x0033, (QWORD)0x0023, (QWORD)0x0006, (QWORD)0x0002, (QWORD)0x0008
};
#endif

CONST QWORD V34_tPREEMP_BETA[5] =
{
    3277, 6554, 9830, 13107, 16384
};

CONST QWORD V34_tPREEMP_GAMA[5] =
{
    3277, 6554, 9830, 13107, 16384
};

CONST QWORD V34_tPREEMP_ALPHA[6] =
{
    0, 3277, 6554, 9830, 13107, 16384
};

CONST QWORD V34_tCARRIER[6][2] =
{
    {1600,  1800},
    {1646,  1829},
    {1680,  1867},
    {1800,  2000},
    {1829,  1920},
    {1959,  1959}
};

CONST UBYTE V34_tONE_OVER_SYM[6] =
{
    13, 11, 11, 10, 10, 9
};

CONST QWORD V34_tONE_OVER_SYM_OFFSET[6] =
{
    21408, 31000, 23031, 30234, 7864, 18224
};

CONST UBYTE V34_tMAX_DRATE[2][6] =
{
    {V34_BIT_21600, V34_BIT_24000, V34_BIT_24000, V34_BIT_26400, V34_BIT_28800, V34_BIT_28800},
    {V34_BIT_21600, V34_BIT_26400, V34_BIT_26400, V34_BIT_28800, V34_BIT_31200, V34_BIT_33600}
};

CONST QWORD BIQ_COEF_ANS[18] =
{
    1,
    325,
    0,
    -325,
    22711,
    -15733,
    1,
    308,
    0,
    -308,
    22445,
    -15859,
    1,
    414,
    0,
    -414,
    23154,
    -15874
};

CONST QWORD BIQ_COEF_CALL[18] =
{
    0,
    650,
    0,
    -650,
    0,
    -31467,
    0,
    607,
    0,
    -607,
    -986,
    -31734,
    0,
    840,
    0,
    -840,
    986,
    -31734
};
