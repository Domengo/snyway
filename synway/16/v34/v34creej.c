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

/************************************************************************/
/* Create the E sequence, used in end of PHASE 4.                       */
/* Put all the generated symbols in the Tx Symbol Buffer                */
/*                                                                      */
/* Function: void Create_E(INFO *V34I)                                  */
/*                                                                      */
/* INPUT : INFO *V34I - V34 Info structure pointer                      */
/*         ShellMap->scram_idx is assumed to be 0                       */
/*         All 20 scrambled binary 1s is placed in V34I->scram_buf[]    */
/*                                                                      */
/* OUTPUT: Output generated symbols to V34I->tx_sym_buf[]               */
/*                                                                      */
/* Last Updated: Mar 14, 1996.                                          */
/*               Apr 08, 1996.                                          */
/*               Apr 30, 1996. Jack Liu Porting to VXD lib              */
/* Author: Benjamin Chan, GAO Research & Consulting Ltd.                */
/************************************************************************/

#include "v34ext.h"

void V34_Create_E(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    UBYTE  dbits, q;
    UBYTE  *pBufptr;
    UBYTE  idx, symidx;

    idx = pShellMap->scram_idx;
    pBufptr = &(pShellMap->pScram_buf[idx]);
    symidx = pV34Tx->tx_sym_inidx;

    if (pV34Rx->Num_Trn_Tx == V34_4_POINT_TRN)
    {
        while (idx < 20)
        {
            dbits = pBufptr[idx] + (pBufptr[idx+1] << 1);
            idx += 2;
            pV34Tx->z = (pV34Tx->z + dbits) & 0x3;

            dbits = pV34Tx->z;

            pV34Tx->pcTx_sym[symidx].r = V34_tTRAIN_MAP_4[dbits].r;
            pV34Tx->pcTx_sym[symidx].i = V34_tTRAIN_MAP_4[dbits].i;

            ++ symidx;
        }
    }
    else
    {
        while (idx < 20)
        {
            dbits = pBufptr[idx] + (pBufptr[idx+1] << 1);
            pV34Tx->z = (pV34Tx->z + dbits) & 0x3;
            dbits = pV34Tx->z;
            q = pBufptr[idx+2] + (pBufptr[idx+3] << 1);
            idx += 4;

            pV34Tx->pcTx_sym[symidx].r = V34_tTRAIN_MAP_16[(q << 2) + dbits].r;
            pV34Tx->pcTx_sym[symidx].i = V34_tTRAIN_MAP_16[(q << 2) + dbits].i;

            symidx++;
        }
    }

    pShellMap->scram_idx = idx;
    pV34Tx->tx_sym_inidx = symidx;
}

CQWORD  V34_Create_J(V34Struct *pV34)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    UBYTE i;

    i = pShellMap->pScram_buf[pShellMap->scram_idx] + (pShellMap->pScram_buf[pShellMap->scram_idx+1] << 1);

    pShellMap->scram_idx += 2;

    pV34Tx->z = (pV34Tx->z + i) & 0x3;

    i = pV34Tx->z;

    return(V34_tTRAIN_MAP_4[i]);
}
