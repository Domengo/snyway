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
/* Signal Mapper                                                           */
/*                                                                         */
/* INPUT:  'TrellisEncOut', which is the Trellis                           */
/*                          encoded group of bits to be transmitted,       */
/*                                                                         */
/* OUTPUT: the I-Q point for the group of bits into 'cqSigMapIQ'           */
/***************************************************************************/

#include "v3217ext.h"

void V32_SigMapResetData(V32ShareStruct *pV32Share)     /* for Send SILENCE */
{
    pV32Share->cqSigMapIQ.r = 0;
    pV32Share->cqSigMapIQ.i = 0;
}

void V32_SigMap(V32ShareStruct *pV32Share)
{
    UBYTE   idx;

    /* get I-Q point */
    idx = pV32Share->ubTrellisEncOut;

    pV32Share->cqSigMapIQ.r = pV32Share->pcSigMapTab[idx].r << V32_SIGMAP_SCALE;
    pV32Share->cqSigMapIQ.i = pV32Share->pcSigMapTab[idx].i << V32_SIGMAP_SCALE;
}

void V32_RxTrainSigMap(V32ShareStruct *pV32Share)
{
    UBYTE idx;

    /* get I-Q point */
    idx = pV32Share->ubTrainEncodedBits;

    pV32Share->cqTrainIQ.r = cV32_IQTab_4800NR[idx].r << V32_SIGMAP_SCALE;
    pV32Share->cqTrainIQ.i = cV32_IQTab_4800NR[idx].i << V32_SIGMAP_SCALE;
}
