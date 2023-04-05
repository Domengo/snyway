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

#include "v34ext.h"
#include "mhsp.h"
#include "ptable.h"

/****************************************************************************
* Phase 3 Baud ISR routine Handler
*
* INPUT : V34_DataStrucPtr *pSC
*         U8 *pGlobal
*
* OUTPUT: pPCMout buffer is filled and is send to HW
*
* Last updated:
*      Apr 03, 1996
*      Apr 09, 1996. - Benjamin, modified EC vector input parameter
*      Apr 30, 1996. - Jack Liu Porting to VXD lib
*      Jul 11, 1996. - Call parameter to all vectors now is pV34, not U8
* Author:
*      Dennis Chan, GAO Research & Consulting Ltd.
*      Benjamin Chan, GAO Research & Consulting Ltd.
****************************************************************************/

#if 0 /// LLL temp
/**************************************/
/*        Phase 3 Handler             */
/**************************************/
void V34_Phase3_Handler(UBYTE **pTable)
{
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
#if WITH_DC
    DCStruct *pDC = (DCStruct *)pTable[DC_STRUC_IDX];
#endif
#if DRAWEPG
    SWORD *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    SWORD *pPCMin  = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    Phase3_Info *p3 = &(pV34->p3);
    CQWORD tDsym[3];
    UBYTE  Num_Symbol, i;
    /* UBYTE len; */

    pV34Tx->PCMinPtr  = pPCMin;
    pV34Tx->PCMoutPtr = pPCMout;

    V34_DPCS_Phase3(pTable);

    /* Number of symbols depends on sampling rate used */
    Num_Symbol = pV34->NumSymbol;

#if WITH_DC
    /* Low pass DC estimator filter */
    DcEstimator(pDC, pPCMin, UBUBMULUB(Num_Symbol, V34_SYM_SIZE), -11);
#endif

#if DRAWEPG
    *pEpg++ = Num_Symbol;
#endif

    for (i = 0; i < Num_Symbol; i++)
    {
        p3->pfTx[p3->tx_vec_idx](pV34);

        p3->pfEc[p3->ec_vec_idx](pV34);

        /* check retrain request */
        V34_Check_Rtrn_Req(pRx);

        pTCR->Tcount ++;

        while (pTCR->Tcount > 0)
        {
            V34_Demodulate(pRx, tDsym);
            V34_Timing_Rec(pTCR, tDsym, pV34Rx->tTimingOut);

            if (pTCR->Tcount > 0)
            {
                p3->pfRx[p3->rx_vec_idx](pV34);
                pTCR->Tcount --;
            }
        }

        pTCR->Tcount = 0;

        pV34Tx->PCMinPtr  += V34_SYM_SIZE;
        pV34Tx->PCMoutPtr += V34_SYM_SIZE;

#if 0

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pRx->cqEQ_symbol.r; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pRx->cqEQ_symbol.i; }

#endif

#if DRAWEPG
        *pEpg++ = pRx->cqEQ_symbol.r;
        *pEpg++ = pRx->cqEQ_symbol.i;
#endif
    }

    if (p3->result_code == 1)
    {
        V34_Phase4_Init(pTable);
        TRACE0("Phase4 starts....");
    }

    if (pRx->Retrain_flag)
    {
        V34_Test_Retrain_State(pTable);
    }

    /* time out counter for different states  */
    pV34->Time_cnt++;

    /* Phase3 Timeout control */
    p3->TimeOut -= 4;
}

/***************************************************************************
* Phase 4 Baud ISR routine Handler
*
* INPUT : V34Struct * *pSC
*         U8 *pGlobal
*
* OUTPUT: pPCMout buffer is filled and send to HW
*
* Last updated:
*      Apr 03, 1996
*      Apr 09, 1996. - Benjamin, modified EC vector input parameter
*      Jul 11, 1996. - Call parameter to all vectors now is pV34, not U8
* Author:
*      Dennis Chan, GAO Research & Consulting Ltd.
*      Benjamin Chan, GAO Research & Consulting Ltd.
****************************************************************************/

/*********************/
/*  Phase 4 Handler  */
/*********************/
void V34_Phase4_Handler(UBYTE **pTable)
{
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
#if WITH_DC
    DCStruct *pDC = (DCStruct *)pTable[DC_STRUC_IDX];
#endif
#if DRAWEPG
    SWORD *pEpg  = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    SWORD  *pPCMin  = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD  *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    Phase4_Info *p4 = &(pV34->p4);
    UBYTE i, Num_Symbol;

    pV34Tx->PCMinPtr  = pPCMin;
    pV34Tx->PCMoutPtr = pPCMout;

    V34_DPCS_Phase4(pTable);

    /* Number of symbols depends on sampling rate used */
    Num_Symbol = pV34->NumSymbol;

#if WITH_DC
    /* Low pass DC estimator filter */
    DcEstimator(pDC, pPCMin, UBUBMULUB(Num_Symbol, V34_SYM_SIZE), -11);
#endif

#if DRAWEPG
    *pEpg ++ = Num_Symbol;
#endif

    for (i = 0; i < Num_Symbol; i++)
    {
        p4->pfTx[p4->tx_vec_idx](pV34);
        p4->pfEc[p4->ec_vec_idx](pV34);

        /* check retrain request */
        V34_Check_Rtrn_Req(pRx);

        pTCR->Tcount ++;

        while (pTCR->Tcount > 0)
        {
            p4->pfRx[p4->rx_vec_idx](pV34);
            pTCR->Tcount --;
        }

        pTCR->Tcount = 0;

        pV34Tx->PCMinPtr  += V34_SYM_SIZE;
        pV34Tx->PCMoutPtr += V34_SYM_SIZE;

#if DRAWEPG
        *pEpg++ = pRx->pEpg[0].r;
        *pEpg++ = pRx->pEpg[0].i;
#endif
    }

    /* time out counter for different states  */
    pV34->Time_cnt++;

    /* Phase4 Timeout control */
    p4->TimeOut -= 4;

    if (pRx->Retrain_flag)
    {
        V34_Test_Retrain_State(pTable);
    }
}


/***********************/
/* Data Modem Handler  */
/***********************/
void V34_Data_Handler(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
#if WITH_DC
    DCStruct *pDC = (DCStruct *)pTable[DC_STRUC_IDX];
#endif
#if DRAWEPG
    SWORD *pEpg     = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    Phase4_Info *pPh4 = &(pV34->p4);
    SWORD *pPCMin  = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    UBYTE i, Num_Symbol;

    pV34Tx->PCMinPtr  = pPCMin;
    pV34Tx->PCMoutPtr = pPCMout;

    V34_DPCS_Data(pTable);

#if 0/* For dump opposite modem send signal to meansure input signal power for geting AGC. */
    /* First the AGC of V8 and Phase2 and V34 must be removed. */
    for (i = 0; i < 96; i++)
    {
        if (DumpIdx2 < 400000) { DumpData2[DumpIdx2++] = pPCMin[i]; }
    }

#endif

    /* Number of symbols depends on sampling rate used */
    Num_Symbol = pV34->NumSymbol;

#if WITH_DC
    /* Low pass DC estimator filter */
    DcEstimator(pDC, pPCMin, UBUBMULUB(Num_Symbol, V34_SYM_SIZE), -11);
#endif

#if DRAWEPG
    *pEpg++ = Num_Symbol;
#endif

    for (i = 0; i < Num_Symbol; i++)
    {
        V34_Transmiter(pV34);

        V34_Echo_Canceller(pV34);

        /* check retrain request */
        V34_Check_Rtrn_Req(pRx);

        pTCR->Tcount ++;

        while (pTCR->Tcount > 0)
        {
            V34_Receiver(pRx, pTable, 1);
            pTCR->Tcount --;
        }

        pTCR->Tcount = 0;

        pV34Tx->PCMinPtr  += V34_SYM_SIZE;
        pV34Tx->PCMoutPtr += V34_SYM_SIZE;

#if DRAWEPG
        *pEpg++ = pRx->pEpg[0].r;
        *pEpg++ = pRx->pEpg[0].i;
#endif
    }

    if (pRx->ubEcDiscCount > 0)
    {
        return;
    }

    if ((pRx->Renego.renego == 1) || (pRx->Renego.clear_down == 1) || (pRx->Renego.renego_generate == 1))
    {
        if (pRx->Renego.renego == 1)
        {
            PutReg(pDpcs->DEBUGSR, (0x72));
        }
        else if (pRx->Renego.clear_down == 1)
        {
            PutReg(pDpcs->DEBUGSR, (0x71));
        }
        else if (pRx->Renego.renego_generate == 1)
        {
            PutReg(pDpcs->DEBUGSR, (0x73));
        }

        pMhsp->pfRealTime = V34_Phase4_Handler;
        pPh4->Phase4_TxEnd = 0;
        pPh4->Phase4_RxEnd = 0;

        PutReg(pDpcs->MSR0, RRATE_DET);

        TRACE0("Renegotiation");

        pPh4->tx_vec_idx = 13;   /* renegotiation init */
        pPh4->rx_vec_idx = 0;
    }
    else if (pRx->Retrain_flag)
    {
        V34_Test_Retrain_State(pTable);
    }
}
#endif
