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

/* ----------------------------------------------------------------------- */
/* Descrambler: GPC and GPA descrambler                                    */
/*                                                                         */
/* INPUT: UBYTE *bit_in        - Input bit stream                          */
/*        UDWORD *sc           - descrambler register                      */
/*        UBYTE *out_stream    - Output descrambled byte stream            */
/*        UBYTE *outdata       - Output descrambled byte                   */
/*        UBYTE n              - number of bits to descramble              */
/*        UBYTE nbyte          - number of bytes to descramble             */
/*                                                                         */
/* OUTPUT: the out_stream is filled with descrambled bits                  */
/*                                                                         */
/* Release History:                                                        */
/*       Feb 13, 1996. Dennis first release                                */
/*       Apr 30, 1996. Jack Liu Porting to VXD lib                         */
/*       Jul 23, 1996. Benjamin Chan, Make byte descrambler                */
/* ----------------------------------------------------------------------- */

#include "shareext.h"

#if !USE_ASM

/********************************************************************/
/* Descramble up to 7 bits, 1st bit in time of sc is located in MSB */
/********************************************************************/
void  V34_Descram_Bit_GPC(UBYTE *pBit_in, UDWORD *pSc, UBYTE *pOutdata, UBYTE n)
{
    UDWORD bit5, bit23;
    UDWORD sc_out;
    UBYTE i;

    *pOutdata = 0;

    /* Answer Modem generating ploynomial   (GPA) = 1 + X(-5) + X(-23) */
    for (i = 0; i < n; i++)
    {
        bit5  = (*pSc) >> 27;
        bit23 = (*pSc) >> 9;
        sc_out = ((*pBit_in) ^ bit5 ^ bit23) & 0x1;
        (*pOutdata) |= (sc_out << i);
        (*pSc) = ((*pSc) >> 1) | ((UDWORD)(*pBit_in++) << 31);
    }
}


/*F:V34_Descram_Bit_GPA()************************************************/
/* Descramble up to 7 bits, 1st bit in time of sc is located in MSB */
/*F!*****************************************************************/
void  V34_Descram_Bit_GPA(UBYTE *pBit_in, UDWORD *pSc, UBYTE *pOutdata, UBYTE n)
{
    UDWORD bit18, bit23;
    UDWORD sc_out;
    UBYTE i;

    *pOutdata = 0;

    /* Call Modem generating ploynomial   (GPC) = 1 + X(-18) + X(-23) */
    for (i = 0; i < n; i++)
    {
        bit18 = (*pSc) >> 14;
        bit23 = (*pSc) >> 9;
        sc_out = (*pBit_in ^ bit18 ^ bit23) & 0x1;
        (*pOutdata) |= (sc_out << i);
        (*pSc) = ((*pSc) >> 1) | ((UDWORD)(*pBit_in++) << 31);
    }
}


/*************************************************************************/
/* Descramble a 8bits at a time, 1st bit in time of sc is located in MSB */
/*************************************************************************/
void  V34_Descram_Byte_GPC(UBYTE *pBit_in, UDWORD *pSc, UBYTE *pOut_stream, UBYTE nbyte)
{
    UDWORD sc_1_5, sc_16_23, sc_out;
    UBYTE  i, j;

    /*  Answer mode modem generating ploynomial   (GPA) = 1 + X(-5) + X(-23) */
    for (i = 0; i < nbyte; i++)
    {
        sc_out = 0;

        for (j = 0; j < 8; j++)
        {
            sc_out = (sc_out >> 1) | ((UDWORD)(*pBit_in++) << 7);
        }

        sc_1_5 = (*pSc) >> 19;
        sc_16_23 = (*pSc) >> 1;
        (*pSc) = (sc_out << LS) | ((*pSc) >> RS);
        (*pOut_stream++) = (UBYTE)((sc_out ^ sc_16_23 ^ sc_1_5) & 0xFF);
    }
}


/*************************************************************************/
/* Descramble a 8bits at a time, 1st bit in time of sc is located in MSB */
/*************************************************************************/
void V34_Descram_Byte_GPA(UBYTE *pBit_in, UDWORD *pSc, UBYTE *pOut_stream, UBYTE nbyte)
{
    UDWORD sc_11_18, sc_16_23, sc_out;
    UBYTE  i, j;

    /*  Call mode modem generating ploynomial   (GPC) = 1 + X(-18) + X(-23) */
    for (i = 0; i < nbyte; i++)
    {
        sc_out = 0;

        for (j = 0; j < 8; j++)
        {
            sc_out = (sc_out >> 1) | ((UDWORD)(*pBit_in++) << 7);
        }

        sc_11_18 = (*pSc) >> 14;
        sc_16_23 = (*pSc) >> 9;
        (*pSc) = (sc_out << LS) | ((*pSc) >> RS);
        (*pOut_stream++) = (UBYTE)((sc_out ^ sc_11_18 ^ sc_16_23) & 0xFF);
    }
}

#endif
