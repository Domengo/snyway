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

/**********************************************/
/* This file defines all the vectors function */
/* for transmitter handshaking and data mode. */
/**********************************************/

#include "v29ext.h"
#include "ptable.h"
#include "mhsp.h"
#include "ioapi.h"

void V29_HS_TEP(V29Struct *pV29)/* TEP Tone, unmodulated carrier. */
{
    pV29->cqSigMapIQ.r = V29_MAG0;
    pV29->cqSigMapIQ.i = 0;
}

/* No transmitted energy segment */
void V29_HS_Segment_1(V29Struct *pV29)
{
    /* no transmitted energy */
    pV29->cqSigMapIQ.i = 0;
    pV29->cqSigMapIQ.r = 0;
}

/*********************************************/
/* This function generates the pseudo-random bit to transmit  proper point
(either C or D) in the map.   Polynomial used in the function to derive
the sequence is X^(-6) + X^(-7) + 1 */

void V29_HS_Segment_3(V29Struct *pV29)
{
    UBYTE Bit6, Bit7;
    UBYTE Sc_out;

    pV29->ubScramOutbits = 0;                         /* make outbit equals to zero */

    Bit6 = (UBYTE)((pV29->udScramSReg >> 26) & 0x01); /* shift right 26 considering only 16 bit register */
    Bit7 = (UBYTE)((pV29->udScramSReg >> 25) & 0x01); /* shift right 25 considering only 16 bit register */

    Sc_out = (Bit6 ^ Bit7) & 0x01;                  /*  ex-or to get output bit  */

    pV29->udScramSReg = (pV29->udScramSReg >> 1) | (((UDWORD)Sc_out) << 31);
    pV29->ubScramOutbits = Bit7 & 0x01;               /* determine the output bit 0 or 1 */
}

/*********************************************/
/* all scrambled binary ONE's */

void V29_HS_Segment_4(V29Struct *pV29)
{
    if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
    {
        pV29->ubScramInbits = 0xf;
    }
    else
#if V29_MODEM_ENABLE
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
        {
            pV29->ubScramInbits = 0x7;
        }

#if V29_MODEM_ENABLE
        else
        {
            pV29->ubScramInbits = 0x3;
        }

#endif
}

void V29_Data_Transfer(V29Struct *pV29)
{
    UBYTE *pIOTxBuf = (UBYTE *)(pV29->pTable[DSPIORDBUFDATA_IDX]);

    IoRdProcess(pV29->pTable, pV29->ubBitsPerSym);

    pV29->ubScramInbits = *pIOTxBuf;

    pV29->nTxDelayCnt = V29_T_DATA_TIME_10000;

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV29->ubScramInbits; }

#endif
}
