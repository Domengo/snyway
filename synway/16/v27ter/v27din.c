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
/* This file defines all the vectors function for transmitter handshaking  */
/* and data mode.                                                          */
/***************************************************************************/

#include "v27ext.h"
#include "ioapi.h"
#include "ptable.h"
#include "mhsp.h"

void V27_HS_Segment_UnMod_Carrier(V27Struct *pV27)
{
    pV27->cqSigMapIQ.r = V27_MAG0;
    pV27->cqSigMapIQ.i = 0;
    /* TRACE0("DataIn Segment 1"); */
}

/* No transmitted energy segment */
void V27_HS_Segment_Silent(V27Struct *pV27)
{
    /* no transmitted energy */
    pV27->cqSigMapIQ.r = 0;
    pV27->cqSigMapIQ.i = 0;
    /* TRACE0("DataIn Segment 2"); */

}

void V27_HS_Segment_180_Phase(V27Struct *pV27)
{
    /* Continuous 180 phase reversals */
    if (pV27->ubBitsPerSym == 3)
    {
        pV27->ubScramOutbits = 0x07;
    }
    else
    {
        pV27->ubScramOutbits = 0x03;
    }

    /* TRACE0("DataIn Segment 3"); */
}

/* Equalizer Training */
void V27_HS_Segment_0_180_Phase(V27Struct *pV27)
{
    pV27->ubScramInbits = 0x07;
    /* TRACE0("DataIn Segment 4"); */
}

/* all scrambled binary ONE's */
void V27_HS_Segment_Scrambled_One(V27Struct *pV27)
{
    if (pV27->ubBitsPerSym == 3)
    {
        pV27->ubScramInbits = 0x07;
    }
    else
    {
        pV27->ubScramInbits = 0x03;
    }

    /* TRACE0("DataIn Segment 5"); */
}

void V27_TX_Data(V27Struct *pV27)
{
    UBYTE *pIOTxBuf = (UBYTE *)(pV27->pTable[DSPIORDBUFDATA_IDX]);

    pV27->nTxDelayCnt = 1000;

    IoRdProcess(pV27->pTable, pV27->ubBitsPerSym);
    pV27->ubScramInbits = *pIOTxBuf;

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV27->ubScramInbits; }

#endif
}
