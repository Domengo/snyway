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

#include "v29ext.h"

void V29_RX_Decoder(V29Struct *pV29)
{
    SBYTE sbAbsolute_phase, sbPhase_change;

    sbAbsolute_phase = V29_tAbsolutePhase[pV29->ubDecodeOut];
    sbPhase_change   = (sbAbsolute_phase - pV29->sbPast_phase) & 0x07;

    if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
    {
        if (pV29->ubDecodeOut <= 7)      /* considering array starts from 0 not 1  */
        {
            pV29->ubDescramInbits = 0x08 | Q234_Array[sbPhase_change];
        }
        else
        {
            pV29->ubDescramInbits = Q234_Array[sbPhase_change];
        }
    }
    else
#if V29_MODEM_ENABLE
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
        {
            pV29->ubDescramInbits  = Q234_Array[sbPhase_change];
        }

#if V29_MODEM_ENABLE
        else
        {
            pV29->ubDescramInbits  = Q23_Array[sbPhase_change >> 1];
        }

#endif

    pV29->sbPast_phase = sbAbsolute_phase;
}
