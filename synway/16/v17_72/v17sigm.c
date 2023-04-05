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

/* ----------------------------------------------------------------------- */
/* Signal Mapper                                                           */
/*                                                                         */
/* INPUT:  V32ShareStruct *pV32Share - pointer to modem data structure     */
/*                              - provides 'TEOut', which is the Trellis   */
/*                                encoded group of bits to be transmitted, */
/*                                                                         */
/* OUTPUT: V32ShareStruct *pV32Share - pointer to modem data structure     */
/*                              - writes the I-Q point for the group of    */
/*                                bits into 'SigMapIQ'                     */
/* ----------------------------------------------------------------------- */

#include "v17ext.h"

#if SUPPORT_V17/* The switch is only for compiling, cannot delete!!! */
void V17_TX_SigMap_Send_A(V32ShareStruct *pV32Share)
{
    pV32Share->cqSigMapIQ.r = -V32_SIGMAP6;
    pV32Share->cqSigMapIQ.i = -V32_SIGMAP2;
}

void V17_TX_SigMap_Send_AB(V32ShareStruct *pV32Share)
{
    if (!(pV32Share->ubCnt & 0x01))
    {
        /* sending "A" */
        pV32Share->cqSigMapIQ.r = -V32_SIGMAP6;
        pV32Share->cqSigMapIQ.i = -V32_SIGMAP2;
    }
    else
    {
        pV32Share->cqSigMapIQ.r =  V32_SIGMAP2;
        pV32Share->cqSigMapIQ.i = -V32_SIGMAP6;
    }

    pV32Share->ubCnt++;
}

void V17_RX_S_TrainSigMap(V32ShareStruct *pV32Share)
{
    /* get I-Q point */
    if (!(pV32Share->ubCnt & 0x01))
    {
        /* sending "A" */
        pV32Share->cqTrainIQ.r = -V32_SIGMAP6;
        pV32Share->cqTrainIQ.i = -V32_SIGMAP2;
    }
    else
    {
        pV32Share->cqTrainIQ.r =  V32_SIGMAP2;
        pV32Share->cqTrainIQ.i = -V32_SIGMAP6;
    }

    pV32Share->ubCnt++;
}
#endif
