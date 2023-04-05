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

/**************************************************************************/
/* file : descra29.c                                                      */
/* This file initilises the descrambler and provides descrambled output   */
/* which is received.                                                     */
/*                                                                        */
/* Author  : Hiren Upadhyay, GAO Research & Consulting Ltd.               */
/* Date    : Jan 25,1997                                                  */
/*                                                                        */
/**************************************************************************/

#include "v29ext.h"

void V29_RX_Descram(V29Struct *pV29)
{
    UBYTE Bit18, Bit23;
    UBYTE i, No_bits;
    UBYTE Out = 0;
    UBYTE Bit_in;
    UDWORD Out_stream = 0;

    No_bits = pV29->ubBitsPerSym;

    for (i = 0; i < No_bits; i++)
    {
        Bit18 = (UBYTE)(pV29->DescramSReg >> 14) & 0x1;   /* put bit 18 in LSB position */
        Bit23 = (UBYTE)(pV29->DescramSReg >>  9) & 0x1;   /* put bit 23 in LSB position */
        Bit_in = (UBYTE)(pV29->ubDescramInbits >> (No_bits - i - 1)) & 0x01;
        Out = (Bit_in ^ Bit18 ^ Bit23) & 0x01;             /* Ex-or of input,bit18 and bit23 */
        pV29->DescramSReg = (pV29->DescramSReg >> 1) | (((UDWORD)Bit_in) << 31);      /* move shift register 1 bit and or with last bit of out */
        Out_stream     |= ((UDWORD)Out) << i;     /* add only last bit of output in out put stream */
    }

    pV29->ubDescramOutbits = (UBYTE)(Out_stream & 0xFF);    /* put final bits in scramble bits */
}
