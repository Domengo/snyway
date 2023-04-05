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

#include "shareext.h"

/* F:Scra_GPC()*****************************************************/
/* Scrambler: GPC and GPA scrambler                                */
/*                                                                 */
/* INPUT: UBYTE *bit_stream   - Input bit stream                   */
/*        UBYTE *sc           - scrambler list                     */
/*        UBYTE *out_stream   - Output scrambled bit stream        */
/*        UBYTE n             - number of bits to scramble         */
/*                                                                 */
/* OUTPUT: the out_stream is filled with scrambled bits            */
/*                                                                 */
/* Date: Feb 5, 1996.                                              */
/*       Apr 19, 1996. Change *sc to UDWORD and rewrite code (faster)   */
/*       Apr 30, 1996, Benjamin, add byte scrambler, reverse bit   */
/*                     organization of scrambler register, bit 0   */
/*                     in MSB                                      */
/*       Apr 30, 1996. Jack Liu Porting to VXD lib                 */
/*       May 14, 1996. Modify Porting.                             */
/* Author: Dennis, GAO Research & Consulting Ltd.                  */
/* F!***************************************************************/

#if !USE_ASM
/* sc is 32 bit, and bit 0 is located at the MSB */
void V34_Scram_Bit_GPC(UBYTE byte, UDWORD *sc, UBYTE *out_stream, UBYTE n)
{
    UDWORD bit18, bit23;
    UDWORD sc_out, i;

    for (i = 0; i < n; i++)
    {
        bit18 = (*sc) >> 14; // align the x[n-18] bit to LSb
        bit23 = (*sc) >> 9; // x[n-23] bit to LSb

        sc_out = (byte ^ bit18 ^ bit23); // modulo 2 sum: x[n]+x[n-18]+x[n-23]

        (*out_stream++) = sc_out & 0x1; // only the LSb

        (*sc) = ((*sc) >> 1) | (sc_out << 31); // place the bit to MSb of the 32-bit register

        byte >>= 1; // scramble the next bit
    }
}

/* sc is 32 bit, and bit 0 is located at the MSB */
/* byte_stream holds byte data, with bit0 starts in LSB */
void V34_Scram_Byte_GPC(UBYTE *byte_stream, UDWORD *sc, UBYTE *out_bit, UBYTE nbyte)
{
    UDWORD sc_11_18, sc_16_23, sc_out;
    UBYTE  i, j;

    for (i = 0; i < nbyte; i++)
    {
        sc_16_23 = (*sc) >> 9;   /* align bit 23 to LSB */
        sc_11_18 = (*sc) >> 14;  /* align bit 18 to LSB */

        sc_out = (sc_11_18 ^ (*byte_stream++) ^ sc_16_23) & 0xFF;

        (*sc) = (sc_out << LS) | ((*sc) >> RS);

        for (j = 0; j < 8; j++)
        {
            (*out_bit++) = (UBYTE)(sc_out & 0x1);
            sc_out >>= 1;
        }
    }
}

void V34_Scram_Bit_GPA(UBYTE byte, UDWORD *sc, UBYTE *out_stream, UBYTE n)
{
    UDWORD bit5, bit23;
    UDWORD sc_out, i;

    for (i = 0; i < n; i++)
    {
        bit5  = (*sc) >> 27;
        bit23 = (*sc) >> 9;

        sc_out = (byte ^ bit5 ^ bit23);

        (*out_stream++) = sc_out & 0x1;

        (*sc) = ((*sc) >> 1) | (sc_out << 31);

        byte >>= 1;
    }
}

void V34_Scram_Byte_GPA(UBYTE *byte_stream, UDWORD *sc, UBYTE *out_bit, UBYTE nbyte)
{
    UDWORD sc_1_5, sc_16_23, sc_out, temp;
    UBYTE  i, j;

    for (i = 0; i < nbyte; i++)
    {
        sc_1_5   = (*sc) >> 27;   /* Align bit 5  to LSB */
        sc_16_23 = (*sc) >> 9;    /* Align bit 23 to LSB */

        temp = (*byte_stream++) ^ sc_16_23;

        sc_out  = (temp ^ sc_1_5) & UMASK;
        sc_out |= ((temp ^(sc_out << 5)) & LMASK);

        (*sc) = (sc_out << LS) | ((*sc) >> RS);

        for (j = 0; j < 8; j++)
        {
            (*out_bit++) = (UBYTE)(sc_out & 0x1);

            sc_out >>= 1;
        }
    }
}
#endif
