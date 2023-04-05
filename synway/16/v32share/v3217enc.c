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
/* Encoding : Convolutional (Trellis) Encoder                              */
/*                                                                         */
/* INPUT:  'ubDiffEncOut' which is not encoded                             */
/*                                                                         */
/* OUTPUT: write encoded output into 'ubTrellisEncOut'                     */
/***************************************************************************/

#include "v3217ext.h"

/* initialize delays to zero */
void V32_EncodeInit(V32ShareStruct *pV32Share)
{
    pV32Share->pTrellisEncTab    = V32_TrellisEncTab_T;
    pV32Share->ubTrellisEncState = 0x00;
}

void V32_BypassTrellisEncoder(V32ShareStruct *pV32Share)
{
    pV32Share->ubTrellisEncOut = pV32Share->ubDiffEncOut;
}

void V32_TrellisEncode(V32ShareStruct *pV32Share)
{
    UBYTE  Y0, Y1Y2;
    UBYTE  idx;

    /* get output bit (delay elem. #3) and put in 'TEOut' */
    Y0 = pV32Share->ubTrellisEncState & 0x01;
    pV32Share->ubTrellisEncOut = (Y0 << pV32Share->ubTxBitsPerSym) | pV32Share->ubDiffEncOut;

    /* get input bits 1 & 2 for future output */
    Y1Y2 = (pV32Share->ubDiffEncOut >> (pV32Share->ubTxBitsPerSym - 2)) & 0x03;

    /* calculate lookup index */
    idx = (Y1Y2 << 3) | pV32Share->ubTrellisEncState;

    /* find encoding for next group of bits (LSB is output Y0) */
    /*     - also update previous delays                       */
    pV32Share->ubTrellisEncState = pV32Share->pTrellisEncTab[idx];
}

void V32_BypassTrainEncode(V32ShareStruct *pV32Share)
{
    pV32Share->ubTrainEncodedBits = pV32Share->ubTrainScramOutbits;
}
