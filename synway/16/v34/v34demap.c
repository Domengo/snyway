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

/****************************************************************************/
/* De-Mapper - Base on the decoded symbols u(n), to find out the label Q(n) */
/*             and the rotation angle from v(n) to u(n).                    */
/*                                                                          */
/* INPUT: CQWORD u    - the decoded symbol                                  */
/*        SWORD *pRx_Q  - Q(n)                                              */
/*        SWORD *pRx_Z  - #of 90 degrees rotated, 0-3                       */
/*                                                                          */
/* OUTPUT: return rx_Q and rx_Z                                             */
/****************************************************************************/

/* All possible magnitude in the 240 points quarter super-constellation */
CONST SWORD V34_tmag[162] =
{
    2,   10,   18,   26,   34,   50,   58,   74,   82,   90,   98,  106,
    122,  130,  146,  162,  170,  178,  194,  202,  218,  226,  234,  242,
    250,  274,  290,  298,  306,  314,  338,  346,  362,  370,  386,  394,
    410,  442,  450,  458,  466,  482,  490,  514,  522,  530,  538,  554,
    562,  578,  586,  610,  626,  634,  650,  666,  674,  698,  706,  722,
    730,  738,  746,  754,  778,  794,  802,  810,  818,  842,  850,  866,
    882,  890,  898,  914,  922,  954,  962,  970,  986, 1010, 1018, 1042,
    1058, 1066, 1082, 1090, 1098, 1114, 1130, 1138, 1154, 1170, 1186, 1202,
    1210, 1226, 1234, 1250, 1258, 1274, 1282, 1306, 1314, 1322, 1346, 1354,
    1370, 1378, 1394, 1402, 1418, 1450, 1458, 1466, 1490, 1514, 1522, 1530,
    1538, 1546, 1570, 1586, 1594, 1602, 1618, 1642, 1658, 1666, 1682, 1690,
    1706, 1714, 1730, 1746, 1754, 1762, 1802, 1810, 1818, 1850, 1858, 1874,
    1882, 1898, 1906, 1922, 1930, 1954, 1962, 1970, 1994, 2018, 2026, 2034,
    2042, 2050, 2066, 2074, 2098, 2106
};

#define PACK(x,y) ( ((UWORD)(x)<<4) | (UWORD)(y) )/* High byte is start_idx, low is length */
CONST UWORD V34_tL_SC[162] =
{
    PACK(0, 1), PACK(1, 2), PACK(3, 1), PACK(4, 2), PACK(6, 2),
    PACK(8, 3), PACK(11, 2), PACK(13, 2), PACK(15, 2), PACK(17, 2),
    PACK(19, 1), PACK(20, 2), PACK(22, 2), PACK(24, 4), PACK(28, 2),
    PACK(30, 1), PACK(31, 4), PACK(35, 2), PACK(37, 2), PACK(39, 2),
    PACK(41, 2), PACK(43, 2), PACK(45, 2), PACK(47, 1), PACK(48, 4),
    PACK(52, 2), PACK(54, 4), PACK(58, 2), PACK(60, 2), PACK(62, 2),
    PACK(64, 3), PACK(67, 2), PACK(69, 2), PACK(71, 4), PACK(75, 2),
    PACK(77, 2), PACK(79, 4), PACK(83, 4), PACK(87, 3), PACK(90, 2),
    PACK(92, 2), PACK(94, 2), PACK(96, 2), PACK(98, 2), PACK(100, 2),
    PACK(102, 4), PACK(106, 2), PACK(108, 2), PACK(110, 2), PACK(112, 3),
    PACK(115, 2), PACK(117, 4), PACK(121, 2), PACK(123, 2), PACK(125, 6),
    PACK(131, 2), PACK(133, 2), PACK(135, 2), PACK(137, 2), PACK(139, 1),
    PACK(140, 4), PACK(144, 2), PACK(146, 2), PACK(148, 4), PACK(152, 2),
    PACK(154, 2), PACK(156, 2), PACK(158, 2), PACK(160, 2), PACK(162, 2),
    PACK(164, 6), PACK(170, 2), PACK(172, 1), PACK(173, 4), PACK(177, 2),
    PACK(179, 2), PACK(181, 2), PACK(183, 2), PACK(185, 4), PACK(189, 4),
    PACK(193, 4), PACK(197, 4), PACK(201, 2), PACK(203, 2), PACK(205, 1),
    PACK(206, 4), PACK(210, 2), PACK(212, 4), PACK(216, 2), PACK(218, 2),
    PACK(220, 4), PACK(224, 2), PACK(226, 2), PACK(228, 4), PACK(232, 2),
    PACK(234, 2), PACK(236, 2), PACK(238, 2), PACK(240, 2), PACK(242, 5),
    PACK(247, 4), PACK(251, 2), PACK(253, 2), PACK(255, 2), PACK(257, 2),
    PACK(259, 2), PACK(261, 2), PACK(263, 2), PACK(265, 4), PACK(269, 4),
    PACK(273, 4), PACK(277, 2), PACK(279, 2), PACK(281, 6), PACK(287, 1),
    PACK(288, 2), PACK(290, 4), PACK(294, 2), PACK(296, 2), PACK(298, 4),
    PACK(302, 2), PACK(304, 2), PACK(306, 4), PACK(310, 4), PACK(314, 2),
    PACK(316, 2), PACK(318, 2), PACK(320, 2), PACK(322, 2), PACK(324, 2),
    PACK(326, 3), PACK(329, 6), PACK(335, 2), PACK(337, 2), PACK(339, 4),
    PACK(343, 2), PACK(345, 2), PACK(347, 2), PACK(349, 4), PACK(353, 4),
    PACK(357, 2), PACK(359, 6), PACK(365, 2), PACK(367, 2), PACK(369, 2),
    PACK(371, 4), PACK(375, 2), PACK(377, 1), PACK(378, 4), PACK(382, 2),
    PACK(384, 2), PACK(386, 4), PACK(390, 2), PACK(392, 2), PACK(394, 2),
    PACK(396, 2), PACK(398, 2), PACK(400, 6), PACK(406, 2), PACK(408, 4),
    PACK(412, 2), PACK(414, 2)
};


CONST UBYTE V34_tZ_table[4][4] =
{
    {0, 1, 3, 2},
    {3, 0, 2, 1},
    {1, 2, 0, 3},
    {2, 3, 1, 0}
};

#if !USE_ASM
void V34_Demapper(CQWORD u, SWORD *pRx_Q, SWORD *pRx_Z)
{
    SWORD mag, idx, front, end, Length, x, y;
    SBYTE rr, ii;
    UBYTE quad, quad_org;
    UBYTE i;

    /* Calculate the magnitude */
    mag = u.r * u.r + u.i * u.i;

    idx = 81; /* 162/2, half the magnitude table size */
    front = 0;
    end = 161;

    while (V34_tmag[idx] != mag)
    {
        if (mag < V34_tmag[idx])
        {
            end = idx - 1;
        }
        else
        {
            front = idx;
        }

        idx = (front + end + 1) >> 1;

        if (front >= end)
        {
            break;
        }
    }

    Length = V34_tL_SC[idx] & 0xF;     /* This is the length */
    idx    = V34_tL_SC[idx] >> 4;      /* This is the start_idx */

    /* Find Z */
    /* First check which quadrant the symbol located in  */
    /*       |                                           */
    /*   2   |   0                                       */
    /* ------+-------                                    */
    /*   3   |   1                                       */
    /*       |                                           */
    quad = 0;

    if (u.r < 0)
    {
        quad = 2;
    }

    if (u.i < 0)
    {
        quad |= 1;
    }

    /* Then find the original quadrant of the symbol     */
    x = u.r;
    y = u.i;

    if (x < 0)
    {
        x = -x;
    }

    if (y < 0)
    {
        y = -y;
    }

    x &= 0x3;
    y &= 0x3;

    if (x == y)
    {
        if (x == 1)
        {
            quad_org = 0;
        }
        else
        {
            quad_org = 3;
        }
    }
    else
    {
        if (((quad == 1 || quad == 2) && x == 3) || ((quad == 0 || quad == 3) && x == 1))
        {
            quad_org = 2;
        }
        else
        {
            quad_org = 1;
        }
    }

    /* Base on the original quadrant and the current quadrant, we */
    /* know how many 90 degrees difference, which is the Z.       */
    *pRx_Z = V34_tZ_table[quad_org][quad];

    switch (*pRx_Z)
    {
        case 0:
            x = u.r;
            y = u.i;
            break; /* rotate 0 degree */
        case 1:
            x = -u.i;
            y = u.r;
            break; /* rotate 90 degree */
        case 2:
            x = -u.r;
            y = -u.i;
            break; /* rotate 180 degree */
        case 3:
            x = u.i;
            y = -u.r;
            break; /* rotate 270 degree */
    }

    for (i = 0; i < Length; i++)
    {
        rr = (SBYTE)((V34_tRI[idx] >> 8) & 0xFF);
        ii = (SBYTE)(((V34_tRI[idx] << 8) >> 8) & 0xFF);/* take out the SIGN and value in lower 8 bits */

        if ((rr == x) && (ii == y))
            break;

        idx ++;
    }

    *pRx_Q = idx;
}
#endif
