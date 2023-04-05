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

/************************************************************************/
/* file : descram.c                                                     */
/* This file initilises the descrambler and provides descrambled output */
/* which is received.                                                   */
/************************************************************************/

#include "v27ext.h"

void V27_Descrambler_init(V27Struct *pV27)
{
    pV27->DescramSReg       = 0;
    pV27->ubDescramInvt     = 0;
    pV27->ubDescramRptCount = 0;
}

void V27_RX_Descram(V27Struct *pV27)
{
    UBYTE i, No_bits;
    UDWORD out = 0;
    UDWORD bit_in, out_stream = 0;
    UBYTE Inbits = pV27->ubDescramInbits;
    UBYTE x6, x7;

    No_bits = pV27->ubBitsPerSym;

    for (i = 0; i < No_bits; i++)
    {
        x6 = (UBYTE)((pV27->DescramSReg >> 5) & 0x1);
        x7 = (UBYTE)((pV27->DescramSReg >> 6) & 0x1);
        bit_in = (Inbits >> (No_bits - i - 1)) & 0x01;
        out = (bit_in ^ pV27->ubDescramInvt ^ x6 ^ x7) & 0x01;

        out_stream |= (UDWORD)out << i;

        if (bit_in)
        {
            bit_in = V27_GUARD_PATTERN;
        }

        if ((((bit_in ^ pV27->DescramSReg) & V27_GUARD_PATTERN) == V27_GUARD_PATTERN) || pV27->ubDescramInvt)
        {
            pV27->ubDescramRptCount = 0;
            pV27->ubDescramInvt     = 0;
        }
        else
        {
            ++pV27->ubDescramRptCount;

            if (pV27->ubDescramRptCount > 32)
            {
                pV27->ubDescramRptCount = 0;
                pV27->ubDescramInvt   = 1;
            }
            else
            {
                pV27->ubDescramInvt   = 0;
            }
        }

        if (bit_in)
        {
            pV27->DescramSReg = (pV27->DescramSReg << 1) | 0x1;
        }
        else
        {
            pV27->DescramSReg <<= 1;
        }
    }

    pV27->ubDescramOutbits = (UBYTE)out_stream ; /* put final bits in scramble bits */
}
