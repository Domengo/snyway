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

void V32_ScramMaskInit(V32ShareStruct *pV32Share)
{
    UBYTE ubMask_bit_idx;

    ubMask_bit_idx = (pV32Share->ubTxBitsPerSym - 2) << 1;

    pV32Share->ubScramRS    = pV32Share->ubTxBitsPerSym;
    pV32Share->ubScramLS    = 32 - pV32Share->ubScramRS;
    pV32Share->ubScramUMASK = ubV32ScramMaskTab[ubMask_bit_idx++]; /* initialize 5 bit lower mask */
    pV32Share->ubScramLMASK = ubV32ScramMaskTab[ubMask_bit_idx];   /* initialize mask for remaining bits */
}

/* ---------- Scrambler ---------- */

void V32_BypassScram(V32ShareStruct *pV32Share)
{
    pV32Share->uScramOutbits = pV32Share->uScramInbits;
}


void V32_ScramUsingGPC(V32ShareStruct *pV32Share)
{
    UBYTE bit_11_18, bit_16_23;
    UBYTE five_bits, six_bits;
    UBYTE xor_bits;

    bit_16_23 = (UBYTE)(pV32Share->udScramSReg >> 9);  /* Align bit 23 at LSB */
    bit_11_18 = (UBYTE)(pV32Share->udScramSReg >> 14); /* Align bit 18 at LSB */
    xor_bits  = (UBYTE)((bit_11_18 ^ pV32Share->uScramInbits ^ bit_16_23));
    five_bits = xor_bits & pV32Share->ubScramUMASK;
    six_bits  = five_bits | (xor_bits & pV32Share->ubScramLMASK);
    pV32Share->udScramSReg   = ((UDWORD)six_bits << pV32Share->ubScramLS) | (pV32Share->udScramSReg >> pV32Share->ubScramRS);
    pV32Share->uScramOutbits = ubBitReversalTab_6Bits[six_bits] >> (6 - pV32Share->ubTxBitsPerSym);
}

void V32_RxTrainScramUsingGPC(V32ShareStruct *pV32Share)
{
    UBYTE bit_11_18, bit_16_23, xor_bit, dibit;

    bit_16_23 = (UBYTE)(pV32Share->udTrainScramSReg >> 9);        /* Align bit 23 at LSB */
    bit_11_18 = (UBYTE)(pV32Share->udTrainScramSReg >> 14);       /* Align bit 18 at LSB */
    xor_bit   = (bit_11_18 ^ V32_TRN_SCRAMINBITS ^ bit_16_23);
    dibit     = xor_bit & 0x3;                      /* Mask 2 bits */
    pV32Share->udTrainScramSReg    = ((UDWORD)dibit << 30) | (pV32Share->udTrainScramSReg >> 2);
    pV32Share->ubTrainScramOutbits = ubBitReversalTab_6Bits[dibit] >> 4;
}
