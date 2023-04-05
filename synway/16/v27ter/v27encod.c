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

/***************************************************************************/
/*  File : encoder.c                                                       */
/*  Functions :   void Encoder_init(V27Struct *pV27)                         */
/*        SWORD  Lookup_phase_4800(S16 Scrambl_bits)                       */
/*        SWORD  Lookup_phase_2400(S16 Scrambl_bits)                       */
/*  This program defines some functions related to encoder of V27 modem.   */
/*  It accepts the input from the scrambler and encoded the bits according */
/*  to the standard V27ter and determines the phase and amplitude.         */
/*  Author    : Sella Maheswaran, GAO Research & Consulting Ltd.           */
/*  Date      : February 11,1997                                           */
/*                                                                         */
/***************************************************************************/

#include "v27ext.h"

void V27_Segment4_Encoder(V27Struct *pV27)
{
    UBYTE ubScramOutbits;

    ubScramOutbits = (pV27->ubScramOutbits >> 2) & 0x01;

    pV27->sbOld_phase = (pV27->sbOld_phase + V27_qPhase_array_Segment4[ubScramOutbits]) & 0x07;

    pV27->cqSigMapIQ = tV27_Segment4_LOOKUP_IQ[pV27->sbOld_phase >> 2];    /* get desired IQ for the phase  */
}

void V27_TX_Encoder(V27Struct *pV27)
{
    if (pV27->ubBitsPerSym == 3)
    {
        pV27->sbOld_phase = (pV27->sbOld_phase + V27_qPhase_array_4800[pV27->ubScramOutbits]) & 0x07;
    }
    else
    {
        pV27->sbOld_phase = (pV27->sbOld_phase + V27_qPhase_array_2400[pV27->ubScramOutbits]) & 0x07;
    }

    pV27->cqSigMapIQ = tV27_LOOKUP_IQ[pV27->sbOld_phase];
}

void V27_RX_TrainEncoder(V27Struct *pV27)
{
    UBYTE ubTrainScramOutbits;

    ubTrainScramOutbits = (pV27->ubTrainScramOutbits >> 2) & 0x01;

    pV27->sbTrainold_phase = (pV27->sbTrainold_phase + V27_qPhase_array_Segment4[ubTrainScramOutbits]) & 0x07;

    pV27->cqTrainIQ = tV27_Segment4_LOOKUP_IQ[pV27->sbTrainold_phase >> 2]; /* get desired IQ for the phase */
}
