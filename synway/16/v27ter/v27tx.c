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

#include "v27ext.h"
#include "ptable.h"

void V27_Segment_UnMod_Carrier(V27Struct *pV27)
{
    pV27->pfDataInVec = V27_HS_Segment_UnMod_Carrier;
    pV27->pfScramVec  = V27_Dummy;
    pV27->pfEncodeVec = V27_Dummy;

    TRACE0("V27: V27_Segment_UnMod_Carrier");
}

void V27_Segment_Silent(V27Struct *pV27)
{
    pV27->pfDataInVec = V27_HS_Segment_Silent;
    pV27->pfScramVec  = V27_Dummy;
    pV27->pfEncodeVec = V27_Dummy;

    TRACE0("V27: V27_Segment_Silent");
}

void V27_Segment_180_Phase(V27Struct *pV27)
{
    pV27->pfDataInVec = V27_HS_Segment_180_Phase;
    pV27->pfScramVec  = V27_Dummy;
    pV27->pfEncodeVec = V27_TX_Encoder;

    TRACE0("V27: V27_Segment_180_Phase");
}

void V27_Segment_0_180_Phase(V27Struct *pV27)
{
    /* 0111100  initial condition for pseudo sequence, right most bit first */
    pV27->udScramSReg = 0x0000003C;
    pV27->pfDataInVec = V27_HS_Segment_0_180_Phase;
    pV27->pfScramVec  = V27_TX_Scrambler;
    pV27->pfEncodeVec = V27_Segment4_Encoder;

    pV27->ubBitsPerSym = 3;

    TRACE0("V27: V27_Segment_0_180_Phase");
}

void V27_Segment_Scrambled_One(V27Struct *pV27)
{
    if (pV27->ubRateSig == V27_4800)
    {
        pV27->ubBitsPerSym = 3;
    }
    else
    {
        pV27->ubBitsPerSym = 2;
    }

    pV27->pfDataInVec = V27_HS_Segment_Scrambled_One;
    pV27->pfScramVec  = V27_TX_Scrambler;
    pV27->pfEncodeVec = V27_TX_Encoder;

    TRACE0("V27: V27_Segment_Scrambled_One");
}

void V27_Data_Mode(V27Struct *pV27)
{
    pV27->pfDataInVec = V27_TX_Data;      /* Data Mode starts */
    pV27->pfScramVec  = V27_TX_Scrambler;
    pV27->pfEncodeVec = V27_TX_Encoder;

    TRACE0("V27: V27_Data_Mode");
}
