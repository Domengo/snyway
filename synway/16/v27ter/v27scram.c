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

#include "v27ext.h"

void V27_Scrambler_init(V27Struct *pV27)
{
    pV27->udScramSReg     = 0;
    pV27->ubScramInvt     = 0;
    pV27->ubScramRptCount = 0;
}

void V27_TX_Scrambler(V27Struct *pV27)
{
    UBYTE  i, No_bits;
    UWORD  Out = 0;
    UDWORD Out_stream = 0;
    UBYTE  x6, x7;

    No_bits = pV27->ubBitsPerSym;

#if 0

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV27->udScramSReg; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV27->ubScramInbits; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV27->ubScramInvt; }

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV27->ubScramRptCount; }

    if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = pV27->nTxState; }

#endif

    for (i = 0; i < No_bits; i++)
    {
        /* udScramSReg is LSB oriented, i.e. LSB is x^1 and MSB is x^32 */
        x6 = (UBYTE)((pV27->udScramSReg >> 5) & 0x1);   /* Retrieve x^-6 bit */
        x7 = (UBYTE)((pV27->udScramSReg >> 6) & 0x1);   /* Retrieve x^-7 bit */

        Out = (pV27->ubScramInbits ^ pV27->ubScramInvt ^ x6 ^ x7) & 0x1;
        pV27->ubScramInbits >>= 1;
        Out_stream = (Out_stream << 1) | Out;

        /* check for the repeating patterns */
        if (Out)
        {
            Out = V27_GUARD_PATTERN;
        }/* Repeated 0's or 1's mask for x^8, x^9 and x^12 */

        if ((((Out ^ pV27->udScramSReg) & V27_GUARD_PATTERN) == V27_GUARD_PATTERN) || pV27->ubScramInvt)
        {
            pV27->ubScramRptCount = 0;
            pV27->ubScramInvt     = 0;
        }
        else
        {
            pV27->ubScramRptCount++;

            if (pV27->ubScramRptCount > 32)
            {
                pV27->ubScramRptCount = 0;
                pV27->ubScramInvt     = 1;
            }
            else
            {
                pV27->ubScramInvt     = 0;
            }
        }

        if (Out)
        {
            pV27->udScramSReg = (pV27->udScramSReg << 1) | 0x1;
        }
        else
        {
            pV27->udScramSReg <<= 1;
        }
    }

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = Out_stream; }

#endif

    pV27->ubScramOutbits = (UBYTE)Out_stream ;  /* put final bits in scramble out */
}

void V27_RX_TrainScrambler(V27Struct *pV27)
{
    UBYTE bit6, bit7;
    UBYTE i, No_bits;
    UBYTE ubOut;
    UBYTE ubOut_stream = 0;

    No_bits = pV27->ubBitsPerSym;

    for (i = 0; i < No_bits; i++)
    {
        bit6 = (UBYTE)((pV27->TrainScramSReg >> 5) & 0x01);         /* put bit 6 in LSB position */
        bit7 = (UBYTE)((pV27->TrainScramSReg >> 6) & 0x01);         /* put bit 7 in LSB position */
        ubOut = 1 ^ bit6 ^ bit7;                           /* Ex-or of input,bit6 and bit7 */

        /* move shift register 1 bit and or with last bit of out */
        pV27->TrainScramSReg = (pV27->TrainScramSReg << 1) | ubOut;
        /* add only last bit of output in out put stream */
        ubOut_stream = (ubOut_stream << 1) | ubOut;
    }

    pV27->ubTrainScramOutbits = ubOut_stream;    /* put final bits in scramble bits */
}
