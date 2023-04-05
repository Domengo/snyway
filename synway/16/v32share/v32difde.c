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

/***************************************************************************/
/* Differential Decoder                                                    */
/*                                                                         */
/* INPUT:  'Cooked_bits' which is the differentially encoded group of bits */
/*                                                                         */
/* OUTPUT: 'Cooked_bits' fully decoded group of bits                       */
/***************************************************************************/

#include "v3217ext.h"

void V32_DiffDecInit(V32ShareStruct *pV32Share)
{
    pV32Share->ubDiffDecodeTab   = ubV32_DiffDecodeTab_NR;
    pV32Share->ubDiffDecodeState = 0x00;
}


void V32_BypassDiffDec(V32ShareStruct *pV32Share)
{
    UBYTE mask = (UBYTE)(((UWORD)1 << pV32Share->ubRxBitsPerSym) - 1);

    pV32Share->ubDiffDecodeOut = pV32Share->ubTrellisDecodeOut & mask;
}


void V32_DiffDec(V32ShareStruct *pV32Share)
{
    UBYTE Q1Q2, Y1Y2;
    UBYTE idx;
    UBYTE ubShift;
    UBYTE mask;

    ubShift = pV32Share->ubRxBitsPerSym - 2;
    mask  = (UBYTE)(((UWORD)1 << ubShift) - 1);

    Q1Q2 = (pV32Share->ubTrellisDecodeOut >> ubShift) & 0x03;
    idx  = (pV32Share->ubDiffDecodeState << 2) + Q1Q2;
    Y1Y2 = pV32Share->ubDiffDecodeTab[idx];                 /* find diff decoding */

    pV32Share->ubDiffDecodeState = Q1Q2;                 /* update previous input */

    Y1Y2 <<= ubShift;
    pV32Share->ubDiffDecodeOut = Y1Y2 | (pV32Share->ubTrellisDecodeOut & mask);
}
