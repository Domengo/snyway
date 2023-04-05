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

#include "v34ext.h"

/* Retrain tone detect coef */
/* the following CONST is for fixed point 9.7 format */
CONST Retrain_Tone V34_tRetrain_Tone_Table[6][2] =
{
    /* Coef  = 2*COS*(2*Pi*Ftone/Fs), Fs sampling rate,
    Ftone=1200Hz for A tone, Ftone=2400Hz for B tone */

    {/* Symbol rate 2400, sampling rate is 7200Hz */
        { -127,  127}, /* ANS modem */
        { 127, -127} /* CALL modem */
    },

    {/* Symbol rate 2743, sampling rate 8229 */
        { -66, 155},
        {155, -66}
    },

    {/* Symbol rate 2800, sampling rate 8400 */
        { -56, 159},
        {159, -56}
    },

    {/* Symbol rate 3000, sampling rate is 9000 Hz */
        { -27, 170},
        {170, -27}
    },

    {/* Symbol rate 3200, sampling rate is 9600 Hz */
        {0, 180 },
        {180,  0}
    },

    {/* Symbol rate 3429, sampling rate 10287 */
        {26, 190},
        {190, 26}
    }
};

/* For each symbol rate, low carrier first, then high carrier */
/* Calculation method : C = car_freq / sampling_rate * 256    */
/*      COS table lookup is 256 entries.                      */
/*      Integer part is for Carrier_tbl                       */
/*      Fractional part is for Carrier_offset                 */
CONST UBYTE V34_tCarrier_tbl[6][2] =
{
    /* 2400Hz symbol rate, 7200Hz sampling rate, 1600low, 1800high */
    {56, 64},
    /* 2743Hz symbol rate, 8229Hz sampling rate, 1645.7143low, 1828.5714high */
    {51, 56},
    /* 2800Hz symbol rate, 8400Hz sampling rate, 1680low, 1866.6667high */
    {51, 56},
    /* 3000Hz symbol rate, 9000Hz sampling rate, 1800low, 2000high */
    {51, 56},
    /* 3200Hz symbol rate, 9600Hz sampling rate, 1828.5714low, 1920high */
    {48, 51},
    /* 3429Hz symbol rate, 10287Hz sampling rate, 1959.1837low, 1959.1837high */
    {48, 48}
};

CONST QWORD V34_tCarrier_offset[6][2] =
{
    {29127, 0},

#if HAWK_ADI
    {6466,  29030}, /* for 8229 sampling rate CODEC, ADI */
#else /* HAWK_SLAB or HAWK_ST7550 */
    {6554,  29127}, /* for 8228.571428, SiLab, ST, filter */
#endif

    {6553,  29127},
    {6553,  29127},
    {24966, 6553},

#if HAWK_ADI
    {24921, 24921} /* for 10286 sampling rate CODEC, ADI */
#else /* HAWK_SLAB or HAWK_ST7550 */
    {24966, 24966}     /* for 10285.714286, SiLab, ST, filter */
#endif
};

#if FREQUENCY_OFFSET
CONST QWORD V34_tCarrier_offset_factor[6] =
{
    18641,  /* 1/Sampling Rate in Q12 Format */
    16310,
    15978,
    14913,
    13981,
    13049
};
#endif

#if 0
/* Convert time (in ms) to # of mapping frame ( 8 symbols )
* #mapping_frames = Xms * RTD_FACTOR, RTD_FACTOR = (Symbol_Rate / 8)
*/
CONST QWORD V34_tRTD_FACTOR[6] =
{
    9830, 11235, 11468, 12288, 13107, 14045
};
#else
/* Convert time (in ms) to # of Call (Different symbol rate have different
* number of symbols per call)
* #mapping_frames = Xms * RTD_FACTOR, RTD_FACTOR = (Symbol_Rate / #symbols)
*/
CONST QWORD V34_tRTD_FACTOR[6] =
{
    3277,    /* 2400/(1000 * 24) */
    4380,    /* 2743/(1000 * 28) */
    3277,    /* 2800/(1000 * 28) */
    3277,    /* 3000/(1000 * 30) */
    3277,    /* 3200/(1000 * 32) */
#if SAMCONV
    3745     /* 3429/(1000 * 30) */
#else
3511     /* 3429/(1000 * 32) */
#endif
};
#endif

/* Equalizer output error threshold for Automatic Rate Selection */
/* Column 0 indicate the lowest data rate.
  For symbol rate other than 2400, Column 0 are assigned the same value
  as Column 1, which are the lowest data rate */
#if SUPPORT_V34FAX
CONST QWORD V34_tEQ_ARS_THRESHOLD_FAX[6][14] =
{
    /* 2400, 4800, 7200, 9600,12000,14400,16800,19200,21600,24000,26400,28800,31200,33600 */
    {6343, 3767, 2014,  998,  499,  251,  126,   62,   31,    0,    0,    0,    0,    0},
    {4659, 4659, 2565, 1398,  773,  426,  229,  126,   68,   37,   20,    0,    0,    0},
    {4826, 4826, 2701, 1494,  833,  463,  287,  141,   77,   43,   23,    0,    0,    0},
    {5275, 5275, 2959, 1725,  998,  584,  333,  205,  110,   62,   36,   20,    0,    0},
    {6343, 6343, 2982, 2014, 1188,  711,  426,  251,  150,   88,   52,   31,   18,    0},
    {6343, 6343, 3527, 2267, 1398,  882,  548,  336,  205,  125,   78,   48,   29,   17}
};
#endif

CONST QWORD V34_tEQ_ARS_THRESHOLD[6][14] =
{
    /* 2400, 4800, 7200, 9600,12000,14400,16800,19200,21600,24000,26400,28800,31200,33600 */
    {6343, 3139, 1678,  831,  400,  193,   97,   45,   22,    0,    0,    0,    0,    0},
    {3882, 3882, 2137, 1165,  618,  327,  176,   92,   45,   24,   12,    0,    0,    0},
    {4021, 4021, 2250, 1245,  666,  356,  220,  103,   51,   28,   14,    0,    0,    0},
    {4396, 4396, 2382, 1437,  798,  449,  256,  149,   73,   40,   22,   12,    0,    0},
    {5143, 5143, 2485, 1678,  950,  546,  327,  183,  100,   51,   31,   18,   12,    0},
    {5143, 5143, 2827, 1867, 1120,  660,  420,  245,  150,   91,   51,   32,   19,   10}
};

CONST QWORD V34_qARS_THRESHOLD_SCALE[14] =
{     512,  461,  461,  461,  499,  499,  501,  501,  527,  527,   586,  576,  586, 652};

CONST QWORD V34_tSys_Delay[6] =
{
    V34_SYM2400_SYS_DELAY,
    V34_SYM2743_SYS_DELAY,
    V34_SYM2800_SYS_DELAY,
    V34_SYM3000_SYS_DELAY,
    V34_SYM3200_SYS_DELAY,
    V34_SYM3429_SYS_DELAY
};

CONST UBYTE V34bis_tBit_Rate[6] =
{
    V34_BIT_21600,
    V34_BIT_26400,
    V34_BIT_26400,
    V34_BIT_28800,
    V34_BIT_31200,
    V34_BIT_33600
};

CONST UBYTE V34_tBit_Rate[6] =
{
    V34_BIT_21600,
    V34_BIT_24000,
    V34_BIT_24000,
    V34_BIT_26400,
    V34_BIT_24000,//V34_BIT_28800,
    V34_BIT_28800
};

CONST QWORD V34_tRTD_Sym[6] =
{
    1024, 1170, 1195, 1280, 1365, 1463/* 256 * (24, 27.43, 28, 30, 32, 34.29)/6 */
};

CONST UBYTE V34_tSym_Num[6] =
{
    84, 96, 98, 105, 112, 120/* 21 * (24, 27.43, 28, 30, 32, 34.29)/6 */
    /* 84 symbols is 35ms for 2400Hz symbol rate */
    /* 112 symbols is 35ms for 3200Hz symbol rate */
};
