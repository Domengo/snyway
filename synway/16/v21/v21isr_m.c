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

#include "v21ext.h"
#include "ptable.h"
#include "mhsp.h"
#include "ioapi.h"

#if SUPPORT_V21CH1

void V21_DPCS_isr(UBYTE **pTable)
{
    UBYTE Disc, i, TxBit;
    SWORD *pPCMin   = (SWORD *)pTable[PCMINDATA_IDX];
    SWORD *pPCMout  = (SWORD *)pTable[PCMOUTDATA_IDX];
    UBYTE *pIOTxBuf = (UBYTE *)(pTable[DSPIORDBUFDATA_IDX]);
    UBYTE *pIORxBuf = (UBYTE *)(pTable[DSPIOWRBUFDATA_IDX]);
    V21Struct  *pV21  = (V21Struct *)(pTable[V21_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
#if WITH_DC
    DCStruct *pDC = (DCStruct *)(pV21->pTable[DC_STRUC_IDX]);

    /* Low pass DC estimator filter */
    DcEstimator(pDC, pPCMin, pV21->ubBufferSize, -9);
#endif

    /* V21 Rx */
    pV21->PCMinPtr = pPCMin;
    pV21->pfRxVec(pV21);

    /* If Data mode, send received bits back to IO */
    if (pV21->RxMode)
    {
        for (i = 0; i < pV21->RxNumBits; i++)
        {
            *pIORxBuf = pV21->pOutBits[i];

            IoWrProcess(pTable, 1);
        }

        IoRdProcess(pTable, 1);
        TxBit = *pIOTxBuf;
        pV21->TxBits = (UBYTE)(TxBit & 0x01);
    }
    else
    {
        pV21->TxBits = 1;
    }

    /* V21 Tx */
    pV21->PCMoutPtr = pPCMout;
    pV21->pfTxVec(pV21);

    /* Look for disconnect possibily */
    Disc = RdReg(pDpcs->MCF, DISCONNECT);

    Disc |= pV21->Disconnect_flag;

    if (Disc)
    {
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
    }
}

#endif
