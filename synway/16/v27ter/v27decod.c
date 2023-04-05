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

/*************************************************************************/
/*  file : Decoder.c                                                     */
/*    This functions decodes the output from the slicer and receives     */
/*    back Q1,Q2,Q3 and Q4 bits to send in the descrambler.              */
/*************************************************************************/

#include "v27ext.h"

SBYTE V27_lookup_phase_Decode(V27Struct *pV27)
{
    SBYTE sbPhase = -1;
    /*  assigning proper phase value */

    if (pV27->cqSliceIQ.i == 0)
    {
        if (pV27->cqSliceIQ.r == V27_MAG0)
        {
            sbPhase = 0;
        }
        else if (pV27->cqSliceIQ.r == -V27_MAG0)
        {
            sbPhase = 4;
        }
    }
    else if (pV27->cqSliceIQ.r == 0)
    {
        if (pV27->cqSliceIQ.i == V27_MAG0)
        {
            sbPhase = 2;
        }
        else if (pV27->cqSliceIQ.i == -V27_MAG0)
        {
            sbPhase = 6;
        }
    }
    else if (pV27->cqSliceIQ.i == V27_MAG45)
    {
        if (pV27->cqSliceIQ.r == V27_MAG45)
        {
            sbPhase = 1;
        }
        else if (pV27->cqSliceIQ.r == -V27_MAG45)
        {
            sbPhase = 3;
        }
    }
    else if (pV27->cqSliceIQ.i == -V27_MAG45)
    {
        if (pV27->cqSliceIQ.r == -V27_MAG45)
        {
            sbPhase = 5;
        }
        else if (pV27->cqSliceIQ.r == V27_MAG45)
        {
            sbPhase = 7;
        }
    }

    return sbPhase;
}

void V27_RX_Decoder(V27Struct *pV27)
{
    SBYTE sbAbsolute_phase, sbPhase_change;

    sbAbsolute_phase = V27_lookup_phase_Decode(pV27);

    sbPhase_change = (sbAbsolute_phase - pV27->sbPast_phase) & 0x07;

    if (pV27->ubBitsPerSym == 2) /* baud rate 1200  */
    {
        pV27->ubDescramInbits = V27_LOOKUP_DIBITS_1200[sbPhase_change >> 1];
    }
    else
    {
        pV27->ubDescramInbits = V27_LOOKUP_DIBITS_1600[sbPhase_change];
    }

    pV27->sbPast_phase = sbAbsolute_phase;
}
