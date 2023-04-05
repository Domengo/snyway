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

#if 0
CONST QWORD DSP_tCOS_TAB4[4] =
{
    32767,  0, -32767,   0
};

CONST QWORD DSP_tCOS_TAB12[12] =
{
    32767,  28377,  16384,    0, -16384, -28377,
    -32767, -28377, -16384,    0,  16384,  28377
};

CONST QWORD DSP_tCOS_TABLE16[16] =
{
    32767,
    30273,
    23170,
    12539,
    0,
    -12539,
    -23170,
    -30273,
    -32768,
    -30273,
    -23170,
    -12539,
    0,
    12539,
    23170,
    30273
};
#endif

CONST SWORD DSP_tCOS_TABLE[256] =
{
    32767,  32758,  32728,  32679,  32610,  32521,  32413,  32285,
    32138,  31971,  31785,  31581,  31357,  31114,  30852,  30572,
    30273,  29956,  29621,  29269,  28898,  28511,  28106,  27684,
    27245,  26790,  26319,  25832,  25330,  24812,  24279,  23732,
    23170,  22594,  22005,  21403,  20787,  20159,  19519,  18868,
    18204,  17530,  16846,  16151,  15446,  14732,  14010,  13278,
    12539,  11793,  11039,  10278,   9512,   8739,   7961,   7179,
    6392,   5602,   4808,   4011,   3211,   2410,   1607,    804,
    0,   -804,  -1607,  -2410,  -3211,  -4011,  -4808,  -5602,
    -6392,  -7179,  -7961,  -8739,  -9512, -10278, -11039, -11792,
    -12539, -13278, -14010, -14732, -15446, -16151, -16846, -17530,
    -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
    -23170, -23732, -24279, -24812, -25329, -25832, -26319, -26790,
    -27245, -27684, -28106, -28511, -28898, -29269, -29621, -29956,
    -30273, -30572, -30852, -31114, -31357, -31581, -31785, -31971,
    -32138, -32285, -32413, -32521, -32610, -32679, -32728, -32758,
    -32768, -32758, -32728, -32679, -32610, -32521, -32413, -32285,
    -32138, -31971, -31785, -31580, -31356, -31114, -30852, -30572,
    -30273, -29956, -29621, -29269, -28898, -28510, -28106, -27684,
    -27245, -26790, -26319, -25832, -25329, -24812, -24279, -23732,
    -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18867,
    -18204, -17530, -16846, -16151, -15446, -14732, -14009, -13278,
    -12539, -11792, -11039, -10278,  -9511,  -8739,  -7961,  -7179,
    -6392,  -5601,  -4807,  -4010,  -3211,  -2410,  -1607,   -803,
    0,    804,   1608,   2410,   3212,   4011,   4808,   5602,
    6392,   7179,   7962,   8739,   9512,  10278,  11039,  11793,
    12539,  13279,  14010,  14733,  15446,  16151,  16846,  17531,
    18205,  18868,  19520,  20160,  20787,  21403,  22005,  22595,
    23170,  23732,  24279,  24812,  25330,  25832,  26319,  26790,
    27245,  27684,  28106,  28511,  28898,  29269,  29622,  29956,
    30273,  30572,  30852,  31114,  31357,  31581,  31786,  31971,
    32138,  32285,  32413,  32521,  32610,  32679,  32728,  32758
};

CONST QWORD DSP_tCOS_OFFSET[256] =
{
    -9,   -30,
    -49,  -69,
    -89,  -108,
    -128, -147,
    -167, -186,
    -204, -224,
    -243, -262,
    -280, -299,
    -317, -335,
    -352, -371,
    -387, -405,
    -422, -439,
    -455, -471,
    -487, -502,
    -518, -533,
    -547, -562,
    -576, -589,
    -602, -616,
    -628, -640,
    -651, -664,
    -674, -684,
    -695, -705,
    -714, -722,
    -732, -739,
    -746, -754,
    -761, -766,
    -773, -778,
    -782, -787,
    -790, -794,
    -797, -800,
    -801, -803,
    -803, -803,
    -803, -803,
    -803, -801,
    -800, -797,
    -794, -790,
    -787, -782,
    -778, -773,
    -766, -761,
    -753, -747,
    -739, -732,
    -722, -714,
    -705, -695,
    -684, -674,
    -664, -651,
    -640, -628,
    -616, -602,
    -589, -576,
    -562, -547,
    -533, -517,
    -503, -487,
    -471, -455,
    -439, -422,
    -405, -387,
    -371, -352,
    -335, -317,
    -299, -280,
    -262, -243,
    -224, -204,
    -186, -167,
    -147, -128,
    -108, -89,
    -69,  -49,
    -30,  -10,
    10,   30,
    49,   69,
    89,   108,
    128,  147,
    167,  186,
    205,  224,
    242,  262,
    280,  299,
    317,  335,
    352,  371,
    388,  404,
    422,  439,
    455,  471,
    487,  503,
    517,  533,
    547,  562,
    576,  589,
    602,  616,
    628,  640,
    652,  663,
    674,  684,
    695,  705,
    714,  723,
    731,  739,
    747,  753,
    761,  767,
    772,  778,
    782,  787,
    791,  794,
    797,  799,
    801,  803,
    804,  802,
    803,  804,
    802,  802,
    799,  797,
    794,  790,
    787,  783,
    777,  773,
    766,  761,
    754,  746,
    740,  731,
    723,  713,
    705,  695,
    685,  674,
    663,  652,
    640,  627,
    616,  602,
    590,  575,
    562,  547,
    533,  518,
    502,  487,
    471,  455,
    439,  422,
    405,  387,
    371,  353,
    334,  317,
    299,  280,
    262,  243,
    224,  205,
    185,  167,
    147,  128,
    108,  89,
    69,   49,
    30,   9
};

CONST Q30 DSP_tATAN_TABLE10[64] =
{
    0,  524231, 1048207, 1571672, 2094372, 2616057, 3136477, 3655389,
    4172551, 4687728, 5200690, 5711214, 6219081, 6724082, 7226015, 7724684,
    8219903, 8711495, 9199291, 9683130, 10162862, 10638346, 11109448, 11576047,
    12038029, 12495289, 12947733, 13395274, 13837834, 14275345, 14707747, 15134988,
    15557023, 15973816, 16385337, 16791565, 17192485, 17588087, 17978369, 18363333,
    18742989, 19117351, 19486437, 19850270, 20208878, 20562293, 20910549, 21253686,
    21591746, 21924773, 22252814, 22575920, 22894142, 23207534, 23516153, 23820054,
    24119298, 24413943, 24704050, 24989681, 25270899, 25547765, 25820345, 26088700,
};

CONST QWORD DSP_tATAN_OFFSET10[64] =
{
    1023, 1023, 1022, 1020, 1018, 1016, 1013, 1010,
    1006, 1001, 997, 991, 986, 980, 973, 967,
    960, 952, 944, 936, 928, 920, 911, 902,
    893, 883, 874, 864, 854, 844, 834, 824,
    814, 803, 793, 783, 772, 762, 751, 741,
    731, 720, 710, 700, 690, 680, 670, 660,
    650, 640, 631, 621, 612, 602, 593, 584,
    575, 566, 557, 549, 540, 532, 524, 516,
};

QDWORD DSPD_Atan2(QWORD axialX, QWORD axialY)
{
    QWORD  ratio;
    QWORD tempX;
    QWORD tempY;
    QDWORD atan;
    UBYTE  index1;
    QWORD  index2;
    UBYTE  flag;

    if (axialY == 0)
    {
        if (axialX >= 0)
        {
            atan = 0;
        }
        else
        {
            atan = qD_180;
        }
    }
    else if (axialX == 0)
    {
        if (axialY > 0)
        {
            atan = qD_90;
        }
        else
        {
            atan = qD_270;
        }
    }
    else
    {
        tempX = QABS(axialX);

        tempY = QABS(axialY);

        if (tempY > tempX)
        {
            ratio = qDiva(tempX, tempY);
            flag = 1;
        }
        else
        {
            if (tempX == tempY)
            {
                ratio = 32767;
            }
            else
            {
                ratio = qDiva(tempY, tempX);
            }

            flag = 0;
        }

        /* first index - atan table */
        index1 = (UBYTE)((ratio >> 9) & 0x3F);

        /* second index - atan offset table argument for linear interpolation */
        index2 = ratio & 0x01FF;
        atan = (DSP_tATAN_TABLE10[index1] + QQMULQD(DSP_tATAN_OFFSET10[index1], index2)) >> 10;

        if (flag)
        {
            atan = qD_90 - atan;
        }

        if ((axialX > 0) && (axialY < 0))
        {
            atan = qD_360 - atan;
        }
        else if ((axialX < 0) && (axialY < 0))
        {
            atan += qD_180;
        }
        else if ((axialX < 0) && (axialY > 0))
        {
            atan = qD_180 - atan;
        }
    }

    return(atan); /* 0 <= atan < 2*pi */
}

void SinCos_Lookup_Fine(UWORD Phase, QWORD *qSin, QWORD *qCos)
{
    UBYTE idx;
    UBYTE itp;

    idx = (UBYTE)(Phase >> 8);
    itp = (UBYTE)(Phase & 0xFF);

    *qCos = DSP_tCOS_TABLE[idx] + (QWORD)(QQMULQD(DSP_tCOS_OFFSET[idx], itp) >> 8);

    idx = (idx - 64) & 0xFF;
    *qSin = DSP_tCOS_TABLE[idx] + (QWORD)(QQMULQD(DSP_tCOS_OFFSET[idx], itp) >> 8);
}

void Phase_Update(UWORD *Phase, UWORD DeltaPhase)
{
    UDWORD TempPhase;

    TempPhase = (UDWORD) * Phase + DeltaPhase;
    *Phase = (UWORD)(TempPhase & 0xFFFF);
}

void SinCos_Lookup(UWORD *phase, QWORD delta_phase, QWORD *SinOut, QWORD *CosOut)
{
    *CosOut = DSP_tCOS_TABLE[*phase];
    *SinOut = DSP_tCOS_TABLE[(*phase - 64) & 0x00FF];
    *phase += delta_phase;
    *phase &= 0x00FF;
}

#if SUPPORT_V34FAX
void Sin_Lookup_Fine(UWORD *Phase, UWORD DeltaPhase, QWORD *qSin)
{
    UBYTE idx;
    UBYTE itp;

    idx = (UBYTE)(*Phase >> 8);
    itp = (UBYTE)(*Phase & 0xFF);
    idx = (idx - 64) & 0xFF;

    *qSin = DSP_tCOS_TABLE[idx] + (QWORD)(QQMULQD(DSP_tCOS_OFFSET[idx], itp) >> 8);

    Phase_Update(Phase, DeltaPhase);
}

SDWORD Tim_drift_calc(QDWORD *all_angles, UBYTE Angle_num)
{
    QDWORD qdTemp1, qdTemp2, drift;
    UBYTE i;

    qdTemp1 = 0;
    qdTemp2 = 0;

    for (i = 0; i < Angle_num; i++)
    {
        qdTemp1 += QDQMULQD0(all_angles[i], (i + 1));
        qdTemp2 += (QDWORD)all_angles[i];

#if 0

        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = all_angles[i]; }

#endif
    }

    drift = (qdTemp1 << 1) - QDQMULQD0(qdTemp2, (Angle_num + 1));

    return (drift);
}
#endif