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

/***********************************************
  This function converts received scraminbits to
  scram out bits by using scrambling polynomial.
************************************************/

#include "v29ext.h"

void V29_TX_Scrambler(V29Struct *pV29)
{
    UBYTE Bit18, Bit23;
    UBYTE i, No_bits;
    UBYTE Out = 0;
    UWORD Out_stream = 0;

    pV29->ubScramOutbits = 0;
    No_bits = pV29->ubBitsPerSym;

    for (i = 0; i < No_bits; i++)
    {
        Bit18 = (UBYTE)(pV29->udScramSReg >> 14);        /* put bit 18 in LSB position */
        Bit23 = (UBYTE)(pV29->udScramSReg >> 9);         /* put bit 23 in LSB position */
        Out = pV29->ubScramInbits ^ Bit18 ^ Bit23;       /* Ex-or of input,bit18 and bit23 */
        pV29->udScramSReg = (pV29->udScramSReg >> 1) | (((UDWORD)Out) << 31);  /* move shift register 1 bit and or with last bit of out */
        Out_stream += ((UWORD)(Out & 0x1)) << (No_bits - i - 1); /* add only last bit of output in out put stream */
        pV29->ubScramInbits >>= 1;                            /* move input one bit to get next bit */
    }

    pV29->ubScramOutbits = (UBYTE)Out_stream;            /* put final bits in scramble bits */
}

void V29_RX_TrainScrambler(V29Struct *pV29)
{
    UBYTE  Bit6, Bit7;
    UBYTE  Sc_out;

    pV29->ubTrainScramOutbits = 0;

    Bit6 = (UBYTE)(pV29->TrainScramSReg >> 2);       /* considering only 16 bit register */
    Bit7 = (UBYTE)(pV29->TrainScramSReg >> 1);

    Sc_out = Bit6 ^ Bit7;

    pV29->TrainScramSReg = (pV29->TrainScramSReg >> 1) | (((UWORD)Sc_out) << 7);
    pV29->ubTrainScramOutbits = Bit7 & 0x01;
}
