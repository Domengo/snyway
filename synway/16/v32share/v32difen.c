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
/* Differential Encoder                                                    */
/*                                                                         */
/* INPUT:  'scramOutbits' which is the non-encoded group of bits           */
/*                                                                         */
/* OUTPUT:  'ubDiffEncOut'-  the differentially encoded group of bits      */
/***************************************************************************/

#include "v3217ext.h"

void V32_DiffEncInit(V32ShareStruct *pV32Share)
{
    pV32Share->pDiffEncTab    = V32_DiffEncTab_NR;
    pV32Share->ubDiffEncState = 0x00;
}


void V32_ByPassDiffEnc(V32ShareStruct *pV32Share)
{
    pV32Share->ubDiffEncOut = pV32Share->uScramOutbits & (((UBYTE)1 << pV32Share->ubTxBitsPerSym) - 1);
}

/* this routine contains mainly bit manipulation, */
/* so all variables are default unsigned          */
void V32_DiffEnc(V32ShareStruct *pV32Share)
{
    UBYTE Q1Q2, Y1Y2;
    UBYTE index;
    UBYTE ubShift;
    UBYTE mask;

    ubShift = pV32Share->ubTxBitsPerSym - 2;

    /* get input bits 1 & 2  */
    Q1Q2 = (pV32Share->uScramOutbits >> ubShift) & 0x03;

    /* find diff encoding */
    index = (Q1Q2 << 2) + (pV32Share->ubDiffEncState & 0x03);
    Y1Y2  = pV32Share->pDiffEncTab[index];

    pV32Share->ubDiffEncState = Y1Y2;                       /* update previous output */

    /* store output bits */
    Y1Y2 <<= ubShift;
    mask   = (1 << ubShift) - 1;
    pV32Share->ubDiffEncOut = Y1Y2 | (pV32Share->uScramOutbits & mask);
}
