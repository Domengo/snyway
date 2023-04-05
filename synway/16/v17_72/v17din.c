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

/* Input Data for Transmitter */

#include "v17ext.h"
#include "ioapi.h"
#include "ptable.h"

/* ---------- segment 2 ------------ */
void V17_TX_DataIn_Send_eq_training(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    pV32Share->uScramInbits = 0x03;
}

/* ---------- segment 3 ------------ */
void V17_TX_DataIn_bridge(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    pV32Share->uScramInbits = bit_des[pV17->ubSeg3Cnt++];

    pV17->ubSeg3Cnt &= 0x7;
}

/* ------------- TCF -------------- */
void V17_TX_DataIn_Send_TCF(V17Struct *pV17)
{
    UBYTE *pIOTxBuf = (UBYTE *)(pV17->pTable[DSPIORDBUFDATA_IDX]);
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    /* setup data mode for transmitter */
    pV32Share->nTxDelayCnt = 32000;

    IoRdProcess(pV17->pTable, pV32Share->ubTxBitsPerSym);
    pV32Share->uScramInbits = *pIOTxBuf;

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV32Share->uScramInbits; }

#endif
}
