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
/* DESCRA32.C                                                              */
/***************************************************************************/

#include "v3217ext.h"

void V32_DescramMask_Init(V32ShareStruct *pV32Share)
{
    UBYTE idx;

    idx = (pV32Share->ubRxBitsPerSym - 2) << 1;

    pV32Share->ubDescramRS    = pV32Share->ubRxBitsPerSym;
    pV32Share->ubDescramLS    = 32 - pV32Share->ubDescramRS;
    pV32Share->ubDescramUMASK = ubV32ScramMaskTab[idx++]; /* initialize 5 bit lower mask */
    pV32Share->ubDescramLMASK = ubV32ScramMaskTab[idx];   /* initialize mask for remaining bits */
}

/* ---------- Descrambler ---------- */
void V32_DescramUsingGPC(V32ShareStruct *pV32Share)
{
    UBYTE bit_11_18, bit_16_23, out, xor_bit, reversed_data;

    reversed_data = ubBitReversalTab_6Bits[pV32Share->ubDiffDecodeOut] >> (6 - pV32Share->ubRxBitsPerSym);

    bit_16_23 = (UBYTE)(pV32Share->udDescramSReg >>  9);   /* Align bit 23 at LSB */

    bit_11_18 = (UBYTE)(pV32Share->udDescramSReg >> 14);   /* Align bit 18 at LSB */

    xor_bit   = (bit_11_18 ^ reversed_data ^ bit_16_23);

    out = xor_bit & pV32Share->ubDescramUMASK;

    out = out | (xor_bit & pV32Share->ubDescramLMASK);

    pV32Share->udDescramSReg = ((UDWORD)reversed_data << pV32Share->ubDescramLS) | (pV32Share->udDescramSReg >> pV32Share->ubDescramRS);

    pV32Share->ubDescramOutbits = (UBYTE)(out & 0xff);
}
