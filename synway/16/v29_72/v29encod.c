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

/*****************************************************************************
File : encode29.c
Functions :   void Encoder_init(TX Struct *pV29)
S16  Lookup_phase_9600(S16 Scrambl_bits)
S16  Lookup_phase_7200(S16 Scrambl_bits)

  This program defines some functions related to encoder of V29 modem.
  It accepts the input from the scrambler and encoded the bits according
  to the standard V29 and determines the phase and amplitude.

    Author    : Hiren Upadhyay, GAO Research & Consulting Ltd.

      Date      : December 27,1996

*****************************************************************************/

#include "v29ext.h"

void V29_Segment2_Encode(V29Struct *pV29)
{
    SWORD i;
    SWORD nMapper, n3Mapper;

    if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
    {
        nMapper = V29_MAPPER_9600;
    }
    else
#if V29_MODEM_ENABLE
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
        {
            nMapper = V29_MAPPER_7200;
        }

#if V29_MODEM_ENABLE
        else
        {
            nMapper = V29_MAPPER_4800;
        }

#endif

    i = pV29->nTxDelayCnt;

    n3Mapper = SBQMULQ(3, nMapper);

    if ((i & 0x01) == 0)                   /* either send point A or point B */
    {
        pV29->sbOld_phase  = 4;
        pV29->cqSigMapIQ.r = - n3Mapper;
        pV29->cqSigMapIQ.i = 0;
    }
    else
    {
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
        {
            pV29->sbOld_phase  = 7;
            pV29->cqSigMapIQ.r =  n3Mapper;
            pV29->cqSigMapIQ.i = -n3Mapper;
        }
        else
#if V29_MODEM_ENABLE
            if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
            {
                pV29->sbOld_phase  = 7;
                pV29->cqSigMapIQ.r =   nMapper;
                pV29->cqSigMapIQ.i = - nMapper;
            }

#if V29_MODEM_ENABLE
            else
            {
                pV29->sbOld_phase  = 6;
                pV29->cqSigMapIQ.r = 0;
                pV29->cqSigMapIQ.i = -n3Mapper;
            }

#endif
    }
}

/*******************************************************************/
/* This function determine proper point for transmission(either C or D)
in segment3 of Handshaking. */

void V29_Segment3_Encode(V29Struct *pV29)
{
    UWORD Bit;
    SWORD nMapper, n3Mapper;

    if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
    {
        nMapper = V29_MAPPER_9600;
    }
    else
#if V29_MODEM_ENABLE
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
        {
            nMapper = V29_MAPPER_7200;
        }

#if V29_MODEM_ENABLE
        else
        {
            nMapper = V29_MAPPER_4800;
        }

#endif

    Bit = pV29->ubScramOutbits;

    n3Mapper = SBQMULQ(3, nMapper);

    if (Bit == 0)                       /* send point C if it is zero */
    {
        pV29->sbOld_phase  = 0;
        pV29->cqSigMapIQ.r = n3Mapper;
        pV29->cqSigMapIQ.i = 0;
    }
    else                                /* send point D if it is 1 */
    {
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
        {
            pV29->sbOld_phase  = 3;
            pV29->cqSigMapIQ.r = -n3Mapper;
            pV29->cqSigMapIQ.i =  n3Mapper;
        }
        else
#if V29_MODEM_ENABLE
            if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
            {
                pV29->sbOld_phase  = 3;
                pV29->cqSigMapIQ.r = - nMapper ;
                pV29->cqSigMapIQ.i =   nMapper;
            }

#if V29_MODEM_ENABLE
            else
            {
                pV29->sbOld_phase  = 2;
                pV29->cqSigMapIQ.r = 0 ;
                pV29->cqSigMapIQ.i = n3Mapper;
            }

#endif
    }
}

CQWORD V29_Lookup_iq(V29Struct *pV29)
{
    CQWORD cqIq = {0, 0};
    SWORD Q1_bit;
    SWORD nMapper, n3Mapper, n5Mapper;

    if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
    {
        nMapper = V29_MAPPER_9600;
    }
    else
#if V29_MODEM_ENABLE
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
        {
            nMapper = V29_MAPPER_7200;
        }

#if V29_MODEM_ENABLE
        else
        {
            nMapper = V29_MAPPER_4800;
        }

#endif

    /* get Q1 to determine amplitude, refer V29 standard  */
    Q1_bit = (pV29->ubScramOutbits >> 3) & 0x1;

    n3Mapper = SBQMULQ(3, nMapper);

    if (Q1_bit == 1)/* Must use table */
    {
        n5Mapper = SBQMULQ(5, nMapper);

        if (pV29->sbOld_phase ==  0)
        {
            cqIq.r = n5Mapper;
            cqIq.i = 0;
        }
        else if (pV29->sbOld_phase == 2)
        {
            cqIq.r = 0;
            cqIq.i = n5Mapper;
        }
        else if (pV29->sbOld_phase == 4)
        {
            cqIq.r = -n5Mapper;
            cqIq.i = 0;
        }
        else if (pV29->sbOld_phase == 6)
        {
            cqIq.r = 0;
            cqIq.i = -n5Mapper;
        }
        else if (pV29->sbOld_phase == 1)
        {
            cqIq.r = cqIq.i = n3Mapper;
        }
        else if (pV29->sbOld_phase == 3)
        {
            cqIq.r = -n3Mapper;
            cqIq.i =  n3Mapper;
        }
        else if (pV29->sbOld_phase == 5)
        {
            cqIq.r = cqIq.i = -n3Mapper;
        }
        else if (pV29->sbOld_phase == 7)
        {
            cqIq.r =  n3Mapper;
            cqIq.i = -n3Mapper;
        }
    }
    else
    {
        if (pV29->sbOld_phase ==  0)
        {
            cqIq.r = n3Mapper;
            cqIq.i = 0;
        }
        else if (pV29->sbOld_phase == 2)
        {
            cqIq.r = 0;
            cqIq.i = n3Mapper;
        }
        else if (pV29->sbOld_phase == 4)
        {
            cqIq.r = -n3Mapper;
            cqIq.i = 0;
        }
        else if (pV29->sbOld_phase == 6)
        {
            cqIq.r = 0;
            cqIq.i = -n3Mapper;
        }
        else if (pV29->sbOld_phase == 1)
        {
            cqIq.r = cqIq.i = nMapper;
        }
        else if (pV29->sbOld_phase == 3)
        {
            cqIq.r = -nMapper;
            cqIq.i = -cqIq.r;
        }
        else if (pV29->sbOld_phase == 5)
        {
            cqIq.r = cqIq.i = -nMapper;
        }
        else if (pV29->sbOld_phase == 7)
        {
            cqIq.r = nMapper;
            cqIq.i = -cqIq.r;
        }
    }

    return cqIq;
}

void V29_TX_Encoder(V29Struct *pV29)
{
    if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
    {
        pV29->sbOld_phase = (pV29->sbOld_phase + Phase_array[pV29->ubScramOutbits & 0x7]) & 0x07;
    }
    else
#if V29_MODEM_ENABLE
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
        {
            pV29->sbOld_phase = (pV29->sbOld_phase + Phase_array[pV29->ubScramOutbits]) & 0x07;
        }

#if V29_MODEM_ENABLE
        else /* if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_4800) */
        {
            pV29->sbOld_phase = (pV29->sbOld_phase + Phase4800_array[pV29->ubScramOutbits]) & 0x07;
        }

#endif

    pV29->cqSigMapIQ = V29_Lookup_iq(pV29);
}

void V29_RX_TrainEncoder(V29Struct *pV29)
{
    UBYTE ubBit;
    SWORD nMapper, n3Mapper;

    if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
    {
        nMapper = V29_MAPPER_9600;
    }
    else
#if V29_MODEM_ENABLE
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
        {
            nMapper = V29_MAPPER_7200;
        }

#if V29_MODEM_ENABLE
        else
        {
            nMapper = V29_MAPPER_4800;
        }

#endif

    ubBit = pV29->ubTrainScramOutbits;

    n3Mapper = SBQMULQ(3, nMapper);

    if (ubBit == 0)
    {
        pV29->cqTrainIQ.r = n3Mapper;
        pV29->cqTrainIQ.i = 0;
    }
    else
    {
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
        {
            pV29->cqTrainIQ.r = - n3Mapper;
            pV29->cqTrainIQ.i =   n3Mapper;
        }
        else
#if V29_MODEM_ENABLE
            if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
            {
                pV29->cqTrainIQ.r = - nMapper;
                pV29->cqTrainIQ.i =   nMapper;
            }

#if V29_MODEM_ENABLE
            else /* if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_4800) */
            {
                pV29->cqTrainIQ.r = 0;
                pV29->cqTrainIQ.i = n3Mapper;
            }

#endif
    }

    /* to assign trainIQ values to slicerIQ during timing update in training */
    pV29->cqSliceIQ = pV29->cqTrainIQ;
}
