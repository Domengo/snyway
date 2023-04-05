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

#include "v8ext.h"

SBYTE V8_V21Receive(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    SBYTE Receive_bit = -1;
    UBYTE NextRxBits_BufferOut_idx;
    UBYTE i;

    pV21->pfRxVec(pV21);/* Run V.21 Receiver */

    NextRxBits_BufferOut_idx = (UBYTE)((pV8->RxBits_BufferOut_idx + 1) & (V8_V21_RXBUFSIZE - 1));

    if (pV21->RxNumBits > 0)
    {
        for (i = 0; i < pV21->RxNumBits; i++)
        {
            pV8->RxBits_Buffer[pV8->RxBits_BufferIn_idx++] = pV21->pOutBits[i];

            pV8->RxBits_BufferIn_idx &= (V8_V21_RXBUFSIZE - 1);

#if 0

            if (DumpTone1_Idx < 10000000) { DumpTone1[DumpTone1_Idx++] = pV21->pOutBits[i]; }

            if (DumpTone2_Idx < 10000000) { DumpTone2[DumpTone2_Idx++] = pV8->RxBits_BufferIn_idx; }

#endif
        }

        Receive_bit = pV8->RxBits_Buffer[pV8->RxBits_BufferOut_idx];

        /* output bit from 16 cycles ago */
        pV8->RxBits_BufferOut_idx = NextRxBits_BufferOut_idx;
    }
    else if (NextRxBits_BufferOut_idx <= pV8->RxBits_BufferIn_idx)
    {
        Receive_bit = pV8->RxBits_Buffer[pV8->RxBits_BufferOut_idx];

        /* output bit from 16 cycles ago */
        pV8->RxBits_BufferOut_idx = NextRxBits_BufferOut_idx;
    }

#if 0
    if (DumpTone3_Idx < 10000000) { DumpTone3[DumpTone3_Idx++] = NextRxBits_BufferOut_idx; }
    if (DumpTone4_Idx < 10000000) { DumpTone4[DumpTone4_Idx++] = pV8->RxBits_BufferIn_idx; }
    if (DumpTone5_Idx < 10000000) { DumpTone5[DumpTone5_Idx++] = pV21->RxNumBits; }
#endif

    return (Receive_bit);
}

void V8_V21Transmit(V8Struct *pV8, UBYTE Bit)
{
    V21Struct *pV21 = &(pV8->v21);

    pV21->TxBits = Bit;
    pV21->pfTxVec(pV21);
}
