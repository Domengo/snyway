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

#include "v34fext.h"

#if SUPPORT_V34FAX

CONST QWORD V34Fax_PSF_Coeff[V34FAX_PSF_LEN] =
{
    -1126, 25763, -1126,     0, -1128, 25157,  -769,    30,
    -924, 23406,    94,    48,  -641, 20702,  1582,    64,
    -367, 17333,  3754,    70,  -155, 13636,  6587,    49,
    -19,  9954,  9954,   -19,    49,  6587, 13636,  -155,
    70,  3754, 17333,  -367,    64,  1582, 20702,  -641,
    48,    94, 23406,  -924,    30,  -769, 25157, -1128
};


CONST QWORD V34Fax_1200COS[V34FAX_SYM_LEN] =
{
    32767, 16384, -16384, -32767, -16384, 16384,
    32767, 16384, -16384, -32767, -16384, 16384
}; // 7200 sample rate, 1200Hz has 6 sample points, 600 symbol/s has 12 symbols


CONST QWORD V34Fax_1200SIN[V34FAX_SYM_LEN] =
{
    0, 28377, 28377, 0, -28377, -28377,
    0, 28377, 28377, 0, -28377, -28377
};


CONST QWORD V34Fax_2400COS[V34FAX_SYM_LEN] =
{
    -32767, 16384, 16384, -32767, 16384, 16384,
    -32767, 16384, 16384, -32767, 16384, 16384
}; // 7200 sample rate, 2400Hz has 3 sample points (pi, pi/3, pi*2/3), 600 symbol/s has 12 symbols


CONST QWORD V34Fax_2400SIN[V34FAX_SYM_LEN] =
{
    0, -28377, 28377, 0, -28377, 28377,
    0, -28377, 28377, 0, -28377, 28377
};


CONST CQWORD V34FAX_MAP_TAB[4] =
{
    /* Consider using V.34 HS constellation */
    {4096, 4096}, {4096, -4096}, { -4096, -4096}, { -4096, 4096}

    //{8192, 8192}, { 8192,-8192}, {-8192,-8192}, {-8192, 8192}
};


CONST QWORD V34Fax_Hilbert_Coeff[(HILBERT_TAP_LEN+1)/2] =
{
    46,   205,   583,  1345,  2817,  6053,  20545,
    -20545, -6053, -2817, -1345,  -583,  -205,    -46
};


/* order is 16, stop band is */

CONST QWORD V34Fax_HPF_Coeff[V34FAX_BIQUAD_NUM * 6] =
{
    0,
    4640,
    -7167,
    4640,
    -23405,
    -19422,
    1,
    7496,
    14762,
    7496,
    -19899,
    -11001,
    0,
    13531,
    -5968,
    13531,
    -11941,
    -24504,
    1,
    11768,
    21792,
    11768,
    -25179,
    -13901,
    0,
    23156,
    -1804,
    23156,
    -6825,
    -29278,
    1,
    14548,
    26023,
    14548,
    -27394,
    -15464,
    0,
    27396,
    676,
    27396,
    -5089,
    -31865,
    1,
    15950,
    28187,
    15950,
    -28250,
    -16157
};

CONST QWORD V34Fax_LPF_Coeff[V34FAX_BIQUAD_NUM * 6] =
{
    0,
    4640,
    7167,
    4640,
    23405,
    -19422,
    1,
    7496,
    -14762,
    7496,
    19899,
    -11001,
    0,
    13531,
    5968,
    13531,
    11941,
    -24504,
    1,
    11768,
    -21792,
    11768,
    25179,
    -13901,
    0,
    23156,
    1804,
    23156,
    6825,
    -29278,
    1,
    14548,
    -26023,
    14548,
    27394,
    -15464,
    0,
    27396,
    -676,
    27396,
    5089,
    -31865,
    1,
    15950,
    -28187,
    15950,
    28250,
    -16157
};

#if 0
CONST QWORD V34Fax_HPF_Coeff[V34FAX_BIQUAD_NUM * 6] =
{
    1,      /* shift count for section  1 values  */
    1040,   /* section  1 coefficient B0 */
    -1857,  /* section  1 coefficient B1 */
    1040,   /* section  1 coefficient B2 */
    -18994, /* section  1 coefficient A1 */
    -6549,  /* section  1 coefficient A2 */
    0,      /* shift count for section  2 values */
    8376,   /* section  2 coefficient B0 */
    -7040,  /* section  2 coefficient B1 */
    8376,   /* section  2 coefficient B2 */
    -28630, /* section  2 coefficient A1 */
    -19688, /* section  2 coefficient A2 */
    0,      /* shift count for section  3 values */
    17178,  /* section  3 coefficient B0 */
    -4267,  /* section  3 coefficient B1 */
    17178,  /* section  3 coefficient B2 */
    -19912, /* section  3 coefficient A1 */
    -26335, /* section  3 coefficient A2 */
    0,      /* shift count for section  4 values */
    23140,  /* section  4 coefficient B0 */
    -566,   /* section  4 coefficient B1 */
    23140,  /* section  4 coefficient B2 */
    -15926, /* section  4 coefficient A1 */
    -30888  /* section  4 coefficient A2 */
};


CONST QWORD V34Fax_LPF_Coeff[V34FAX_BIQUAD_NUM * 6] =
{
    0,      /* shift count for section  1 values */
    4205,   /* section  1 coefficient B0 */
    6429,   /* section  1 coefficient B1 */
    4205,   /* section  1 coefficient B2 */
    19175,  /* section  1 coefficient A1 */
    -22547, /* section  1 coefficient A2 */
    1,      /* shift count for section  2 values */
    8254,   /* section  2 coefficient B0 */
    -16149, /* section  2 coefficient B1 */
    8254,   /* section  2 coefficient B2 */
    20420,  /* section  2 coefficient A1 */
    -12390, /* section  2 coefficient A2 */
    0,      /* shift count for section  3 values */
    13150,  /* section  3 coefficient B0 */
    8074,   /* section  3 coefficient B1 */
    13150,  /* section  3 coefficient B2  */
    5093,   /* section  3 coefficient A1 */
    -29201, /* section  3 coefficient A2 */
    1,      /* shift count for section  4 values */
    14041,  /* section  4 coefficient B0 */
    -25732, /* section  4 coefficient B1 */
    14041,  /* section  4 coefficient B2 */
    26108,  /* section  4 coefficient A1 */
    -15343  /* section  4 coefficient A2 */
};
#endif

CONST QWORD qV34Fax_AnsDetectACtoneCoefTab[3] =
{
    23170,
     8481,
    16384,
};

CONST QWORD qV34Fax_CallDetectACtoneCoefTab[3] =
{
     -8481,
    -23170,
    -16384,
};

CONST QWORD qV34Fax_AnsDetectToneCoefTab[V34FAX_PPH_SH_DETECT_NUMTONES] =
{
    23170,  /*  900 Hz */
    19948,  /* 1050 Hz Reference */
    12540,  /* 1350 Hz Reference */
     8481,  /* 1500 Hz */
    30792,  /*  400 Hz for Reference */
};


CONST QWORD qV34Fax_CallDetectToneCoefTab[V34FAX_PPH_SH_DETECT_NUMTONES] =
{
     -8481, /* 2100 Hz */
    -12540, /* 2250 Hz Reference */
    -19948, /* 2550 Hz Reference */
    -23170, /* 2700 Hz */
     30792, /*  400 Hz for Reference */
};


CONST UBYTE V34fax_2BitReversal[4]  = {0, 1, 2, 3};//???Why {0, 2, 1, 3} cannot work???

CONST UBYTE V34fax_4BitReversal[16] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};

#endif
