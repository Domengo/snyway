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

#include "v3217ext.h"

/* Signal Mapping Tables */
CONST CQWORD cV32_IQTab_4800NR[4] =
{
    { -6, -2},
    { 2, -6},
    { -2,  6},
    { 6,  2}
};

CONST CQWORD cV32_IQTab_9600T[32] =
{
    { -8,  2},
    { 0, -6},
    { 0,  2},
    { 8,  2},
    { 8, -2},
    { 0,  6},
    { 0, -2},
    { -8, -2},
    { -4,  6},
    { -4, -2},
    { 4,  6},
    { 4, -2},
    { 4, -6},
    { 4,  2},
    { -4, -6},
    { -4,  2},
    { -6, -4},
    { 2, -4},
    { -6,  4},
    { 2,  4},
    { 6,  4},
    { -2,  4},
    { 6, -4},
    { -2, -4},
    { 2,  8},
    { -6,  0},
    { 2,  0},
    { 2, -8},
    { -2, -8},
    { 6,  0},
    { -2,  0},
    { -2,  8}
};

CONST CQWORD cV32_IQTab_7200T[16] =
{
    { 6, -6},
    { -2,  2},
    { -6,  6},
    { 2, -2},
    { 6,  2},
    { -2, -6},
    { -6, -2},
    { 2,  6},
    { -2,  6},
    { 6, -2},
    { 2, -6},
    { -6,  2},
    { -6, -6},
    { 2,  2},
    { 6,  6},
    { -2, -2}
};

CONST CQWORD cV32_IQTab_12000T[64] =
{
    { 7, 1},
    { 3, 5},
    { 7, -7},
    { -5, 5},
    { 3, -3},
    { -1, 1},
    { -1, -7},
    { -5, -3},
    { -7, -1},
    { -3, -5},
    { -7, 7},
    { 5, -5},
    { -3, 3},
    { 1, -1},
    { 1, 7},
    { 5, 3},
    { -1, 5},
    { -5, 1},
    { 7, 5},
    { -5, -7},
    { 3, 1},
    { -1, -3},
    { 7, -3},
    { 3, -7},
    { 1, -5},
    { 5, -1},
    { -7, -5},
    { 5, 7},
    { -3, -1},
    { 1, 3},
    { -7, 3},
    { -3, 7},
    { -5, -1},
    { -1, -5},
    { -5, 7},
    { 7, -5},
    { -1, 3},
    { 3, -1},
    { 3, 7},
    { 7, 3},
    { 5, 1},
    { 1, 5},
    { 5, -7},
    { -7, 5},
    { 1, -3},
    { -3, 1},
    { -3, -7},
    { -7, -3},
    { 1, -7},
    { 5, -3},
    { -7, -7},
    { 5, 5},
    { -3, -3},
    { 1, 1},
    { -7, 1},
    { -3, 5},
    { -1, 7},
    { -5, 3},
    { 7, 7},
    { -5, -5},
    { 3, 3},
    { -1, -1},
    { 7, -1},
    { 3, -5}
};

CONST CQWORD cV32_IQTab_14400T[128] =
{
    { -8, -3},
    { 8, -3},
    { 4, -3},
    { 4, -7},
    { -4, -3},
    { -4, -7},
    { 0, -3},
    { 0, -7},
    { -8, 1},
    { 8, 1},
    { 4, 1},
    { 4, 5},
    { -4, 1},
    { -4, 5},
    { 0, 1},
    { 0, 5},
    { 8, 3},
    { -8, 3},
    { -4, 3},
    { -4, 7},
    { 4, 3},
    { 4, 7},
    { 0, 3},
    { 0, 7},
    { 8, -1},
    { -8, -1},
    { -4, -1},
    { -4, -5},
    { 4, -1},
    { 4, -5},
    { 0, -1},
    { 0, -5},
    { 2, -9},
    { 2, 7},
    { 2, 3},
    { 6, 3},
    { 2, -5},
    { 6, -5},
    { 2, -1},
    { 6, -1},
    { -2, -9},
    { -2, 7},
    { -2, 3},
    { -6, 3},
    { -2, -5},
    { -6, -5},
    { -2, -1},
    { -6, -1},
    { -2, 9},
    { -2, -7},
    { -2, -3},
    { -6, -3},
    { -2, 5},
    { -6, 5},
    { -2, 1},
    { -6, 1},
    { 2, 9},
    { 2, -7},
    { 2, -3},
    { 6, -3},
    { 2, 5},
    { 6, 5},
    { 2, 1},
    { 6, 1},
    { 9, 2},
    { -7, 2},
    { -3, 2},
    { -3, 6},
    { 5, 2},
    { 5, 6},
    { 1, 2},
    { 1, 6},
    { 9, -2},
    { -7, -2},
    { -3, -2},
    { -3, -6},
    { 5, -2},
    { 5, -6},
    { 1, -2},
    { 1, -6},
    { -9, -2},
    { 7, -2},
    { 3, -2},
    { 3, -6},
    { -5, -2},
    { -5, -6},
    { -1, -2},
    { -1, -6},
    { -9, 2},
    { 7, 2},
    { 3, 2},
    { 3, 6},
    { -5, 2},
    { -5, 6},
    { -1, 2},
    { -1, 6},
    { -3, 8},
    { -3, -8},
    { -3, -4},
    { -7, -4},
    { -3, 4},
    { -7, 4},
    { -3, 0},
    { -7, 0},
    { 1, 8},
    { 1, -8},
    { 1, -4},
    { 5, -4},
    { 1, 4},
    { 5, 4},
    { 1, 0},
    { 5, 0},
    { 3, -8},
    { 3, 8},
    { 3, 4},
    { 7, 4},
    { 3, -4},
    { 7, -4},
    { 3, 0},
    { 7, 0},
    { -1, -8},
    { -1, 8},
    { -1, 4},
    { -5, 4},
    { -1, -4},
    { -5, -4},
    { -1, 0},
    { -5, 0}
};

/*******************************************************************/
/*               TRANSMIT GLOBAL TABLES               */
/*******************************************************************/


/* Differential Encoder Lookup Tables */

/*
bit3     bit2    bit1    bit 0
------------------------------
index : Q1n     Q2n    Y1prev    Y2prev
output:        Y1new    Y2new

*/

CONST UBYTE V32_DiffEncTab_T[16]  =
{
    0, 1, 2, 3,
    1, 0, 3, 2,
    2, 3, 1, 0,
    3, 2, 0, 1
};

CONST UBYTE V32_DiffEncTab_NR[16] =
{
    1, 3, 0, 2,
    0, 1, 2, 3,
    3, 2, 1, 0,
    2, 0, 3, 1
};

/* *****************************************************************/
/*            RECEIVE GLOBAL TABLES               */
/* *****************************************************************/

/* Slicing Tables */

/* IQ rotation and scaling */
CONST SWORD nV32_IQRotateTab_4800NR[2] =  {3277,  -1638};
CONST SWORD nV32_IQRotateTab_7200T[2]  =  {8192,      0};
CONST SWORD nV32_IQRotateTab_9600T[2]  =  {8192,  -8192};
CONST SWORD nV32_IQRotateTab_12000T[2] =  {16384,      0};
CONST SWORD nV32_IQRotateTab_14400T[2] =  {16384, -16384};

CONST UBYTE ubV32_IQtoBitsTab_4800NR[4] =
{
    0x00, 0x01,    0x02, 0x03
};

CONST UBYTE ubV32_IQtoBitsTab_7200T[16] =
{
    0x0C, 0x05, 0x0A, 0x00,
    0x06, 0x0F, 0x03, 0x09,
    0x0B, 0x01, 0x0D, 0x04,
    0x02, 0x08, 0x07, 0x0E
};

CONST UBYTE ubV32_IQtoBitsTab_9600T[36] =
{
    0x00, 0x07, 0x10, 0x0E, 0x1C, 0x1B,
    0x00, 0x19, 0x09, 0x17, 0x01, 0x1B,
    0x12, 0x0F, 0x1E, 0x06, 0x11, 0x0C,
    0x08, 0x15, 0x02, 0x1A, 0x0B, 0x16,
    0x1F, 0x05, 0x13, 0x0D, 0x1D, 0x04,
    0x1F, 0x18, 0x0A, 0x14, 0x03, 0x04
};

CONST UBYTE ubV32_IQtoBitsTab_12000T[64] =
{
    0x32, 0x13, 0x2E, 0x06, 0x30, 0x17, 0x2A, 0x02,
    0x1A, 0x3B, 0x09, 0x21, 0x18, 0x3F, 0x0B, 0x23,
    0x2F, 0x07, 0x34, 0x15, 0x2C, 0x04, 0x31, 0x16,
    0x08, 0x20, 0x1C, 0x3D, 0x0D, 0x25, 0x19, 0x3E,
    0x36, 0x11, 0x2D, 0x05, 0x35, 0x14, 0x28, 0x00,
    0x1E, 0x39, 0x0C, 0x24, 0x1D, 0x3C, 0x0F, 0x27,
    0x2B, 0x03, 0x37, 0x10, 0x29, 0x01, 0x33, 0x12,
    0x0A, 0x22, 0x1F, 0x38, 0x0E, 0x26, 0x1B, 0x3A
};

CONST UBYTE ubV32_IQtoBitsTab_14400T[144] =
{
    0x50, 0x50, 0x50, 0x00, 0x63, 0x2D, 0x55, 0x05, 0x61, 0x28, 0x28, 0x28,
    0x58, 0x19, 0x19, 0x49, 0x33, 0x7D, 0x1B, 0x4B, 0x31, 0x78, 0x78, 0x20,
    0x58, 0x08, 0x67, 0x2F, 0x54, 0x04, 0x62, 0x2C, 0x57, 0x07, 0x69, 0x20,
    0x11, 0x41, 0x37, 0x7F, 0x1A, 0x4A, 0x32, 0x7C, 0x1F, 0x4F, 0x39, 0x70,
    0x65, 0x2B, 0x5C, 0x0C, 0x66, 0x2E, 0x56, 0x06, 0x6A, 0x24, 0x53, 0x03,
    0x35, 0x7B, 0x12, 0x42, 0x36, 0x7E, 0x1E, 0x4E, 0x3A, 0x74, 0x1D, 0x4D,
    0x5D, 0x0D, 0x64, 0x2A, 0x5E, 0x0E, 0x6E, 0x26, 0x52, 0x02, 0x6B, 0x25,
    0x13, 0x43, 0x34, 0x7A, 0x16, 0x46, 0x3E, 0x76, 0x1C, 0x4C, 0x3B, 0x75,
    0x60, 0x29, 0x5F, 0x0F, 0x6C, 0x22, 0x5A, 0x0A, 0x6F, 0x27, 0x51, 0x01,
    0x30, 0x79, 0x17, 0x47, 0x3C, 0x72, 0x14, 0x44, 0x3F, 0x77, 0x18, 0x48,
    0x30, 0x68, 0x68, 0x21, 0x5B, 0x0B, 0x6D, 0x23, 0x59, 0x09, 0x09, 0x48,
    0x38, 0x38, 0x38, 0x71, 0x15, 0x45, 0x3D, 0x73, 0x10, 0x40, 0x40, 0x40
};

/* Viterbi Decoder Lookup Tables */

/*
index : the past and new delay state for Trellis Diagram
(past_delay << 3 + new_delay)

  output: the path value

    NOTE: - past_delay, new_delay, and path value
    are all 3 bits.
    - the values 0xff are not used
*/
CONST UBYTE ubV32_VitPathToStateTab[V32_VIT_ALLPATHS << V32_VIT_STATES_SHIFT] =
{
    0x00, 0xff, 0x01, 0xff, 0x02, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x02, 0xff, 0x01, 0xff, 0x00, 0xff,
    0x01, 0xff, 0x00, 0xff, 0x03, 0xff, 0x02, 0xff,
    0x02, 0xff, 0x03, 0xff, 0x00, 0xff, 0x01, 0xff,
    0xff, 0x04, 0xff, 0x05, 0xff, 0x07, 0xff, 0x06,
    0xff, 0x07, 0xff, 0x06, 0xff, 0x04, 0xff, 0x05,
    0xff, 0x06, 0xff, 0x07, 0xff, 0x05, 0xff, 0x04,
    0xff, 0x05, 0xff, 0x04, 0xff, 0x06, 0xff, 0x07
};

/*
index : the past and new delay state for Trellis Diagram
(past_delay << 3 + new_delay)

  output: the path value

    NOTE: - past_delay, new_delay, and path value
    are all 3 bits.
    - the values 0xff are not used
*/
CONST UBYTE ubV32_VitStateToPathTab[V32_VIT_STATES << V32_VIT_STATES_SHIFT] =
{
    0x00, 0xff, 0x02, 0xff, 0x03, 0xff, 0x01, 0xff,
    0x02, 0xff, 0x00, 0xff, 0x01, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x01, 0xff, 0x00, 0xff, 0x02, 0xff,
    0x01, 0xff, 0x03, 0xff, 0x02, 0xff, 0x00, 0xff,
    0xff, 0x04, 0xff, 0x07, 0xff, 0x05, 0xff, 0x06,
    0xff, 0x07, 0xff, 0x04, 0xff, 0x06, 0xff, 0x05,
    0xff, 0x06, 0xff, 0x05, 0xff, 0x07, 0xff, 0x04,
    0xff, 0x05, 0xff, 0x06, 0xff, 0x04, 0xff, 0x07
};

/* Differential Decoder Lookup Tables */

/*
bit3     bit2    bit1    bit 0
------------------------------
index : Q1n     Q2n    Y1prev    Y2prev
output:        Y1new    Y2new

*/

CONST UBYTE ubV32_DiffDecodeTab_T[16] =
{
    0, 1, 2, 3,
    1, 0, 3, 2,
    3, 2, 0, 1,
    2, 3, 1, 0
};

CONST UBYTE ubV32_DiffDecodeTab_NR[16] =
{
    1, 0, 3, 2,
    3, 1, 2, 0,
    0, 2, 1, 3,
    2, 3, 0, 1
};

CONST UBYTE V32_TrellisEncTab_T[32] =
{
    0, 4, 1, 7, 2, 6, 3, 5,
    3, 5, 2, 6, 1, 7, 0, 4,
    2, 7, 3, 4, 0, 5, 1, 6,
    1, 6, 0, 5, 3, 4, 2, 7
};

/* Scrambler/Descrambler MASK */
CONST UBYTE ubV32ScramMaskTab[10] =
{
    0x03, 0x00,         /* 2bit:UMASK, LMASK */
    0x07, 0x00,         /* 3bit:UMASK, LMASK */
    0x0F, 0x00,         /* 4bit:UMASK, LMASK */
    0x1F, 0x00,         /* 5bit:UMASK, LMASK */
    0x1F, 0x20         /* 6bit:UMASK, LMASK */
};

CONST UBYTE ubBitReversalTab_6Bits[64] =
{
    0x00, 0x20, 0x10, 0x30, 0x08, 0x28, 0x18, 0x38,
    0x04, 0x24, 0x14, 0x34, 0x0C, 0x2C, 0x1C, 0x3C,
    0x02, 0x22, 0x12, 0x32, 0x0A, 0x2A, 0x1A, 0x3A,
    0x06, 0x26, 0x16, 0x36, 0x0E, 0x2E, 0x1E, 0x3E,
    0x01, 0x21, 0x11, 0x31, 0x09, 0x29, 0x19, 0x39,
    0x05, 0x25, 0x15, 0x35, 0x0D, 0x2D, 0x1D, 0x3D,
    0x03, 0x23, 0x13, 0x33, 0x0B, 0x2B, 0x1B, 0x3B,
    0x07, 0x27, 0x17, 0x37, 0x0F, 0x2F, 0x1F, 0x3F,
};


/* --------------------------------------------------------------- */
/*                   HILBERT TRANSFORM COEFFICIENTS                */
/* --------------------------------------------------------------- */
CONST SWORD nV32_HilbCoefTab[(V32_HILB_TAP_LEN+1)>>1] =
{
#if 0   /* 49 taps hilbert filter */
    0 ,    13, 0,    38,
    0,     89, 0,   179,
    0,    327, 0,   557,
    0,    904, 0,  1423,
    0,   2227, 0,  3601,
    0,   6597, 0, 20739,
    0, -20739, 0, -6597,
    0,  -3601, 0, -2227,
    0,  -1423, 0,  -904,
    0,   -557, 0,  -327,
    0,   -179, 0,   -89,
    0,    -38, 0,   -13,
    0
#else  /* 18 taps hilbert filter */
    12   ,       67 ,     201 ,     467 ,
    943  ,     1758 ,     3205,     6331,
    20645,    -20645,    -6331,    -3205,
    -1758,     -943 ,    -467 ,     -201,
    -67  ,     -12
#endif
};
