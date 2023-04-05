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

#include "v34fext.h"
#include "mhsp.h"
#include "ptable.h"

#if SUPPORT_V34FAX

void V34Fax_MPET_Handle(UBYTE **pTable)
{
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    SWORD *pPCMin = (SWORD *)pTable[PCMINDATA_IDX];
    SWORD *pPCMout = (SWORD *)pTable[PCMOUTDATA_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);
    UBYTE i;

#if DRAWEPG
    SWORD *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
    SWORD pntLoc;

    *pEpg = 0;
#endif

    pV34Tx->PCMinPtr  = pPCMin;
    pV34Tx->PCMoutPtr = pPCMout;

    for (i = 0; i < V34FAX_SYM_NUM; i++)
    {
        V34Fax_Symbol_Tx(pV34);

        V34Fax_Symbol_Rx(pV34);

        pV34Tx->PCMoutPtr += V34FAX_SYM_LEN;
        pV34Tx->PCMinPtr  += V34FAX_SYM_LEN;
    }

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        if (pAce->V34Fax_State == V34FAX_TX_ONES2)
        {
            //pV34Fax->nRxDelayCnt = 0;    /* Silence Rx */
            pAce->V34Fax_State   = V34FAX_RX_SILENCE;
            pV34Fax->silence_cnt = 0;
        }
        else if (pAce->V34Fax_State == V34FAX_RX_SILENCE)
        {
            if (pV34Fax->silence_cnt >= 8)
            {
                if (pAce->isDleEotRcved)
                {
                    pV34Fax->Disconnect_flag = TRUE;
                }
                else
                {
                    // fixing issue 80
                    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];

                    pAscCBRd->CurrLen = 0;
                    pAscCBRd->End = pAscCBRd->Start;
                    pV34Fax->V34Fax_PCR_Start_Flag = TRUE;
                }

                pV34Fax->silence_cnt = 0;
                pAce->pDceToT30->selectedV34Chnl = V34_PRIMARY_CHNL; TRACE0("V34: <DLE><PRI>");
            }
        }
    }
    else
    {
        // fixing issue 180
        if (pAce->V34Fax_State == V34FAX_TX_ONES2)
        {
            //pV34Fax->nRxDelayCnt = 0;    /* Silence Rx */
            pAce->V34Fax_State   = V34FAX_RX_SILENCE;
            pV34Fax->silence_cnt = 0;
        }
        else if (pAce->V34Fax_State == V34FAX_RX_SILENCE)
        {
            if (pV34Fax->silence_cnt >= 8)
            {
                pV34Fax->Disconnect_flag = TRUE;
            }
        }
        else if (pAce->V34Fax_State == V34FAX_RX_ONES)
        {
            if ((pV34Fax->ONEs_DetCount >= 8) || (pV34Fax->Ones_Rx_count0 >= 8))
            {
                pV34Fax->nTxDelayCnt = 0;

                pV34Fax->ONEs_DetCount  = 0;
                pV34Fax->Ones_Rx_count0 = 0;
                pAce->V34Fax_State = 255;
            }
        }
    }

    if (pV34Fax->V34Fax_PCR_Start_Flag)
    {
        /* Save Control Channel Coefs before going to primary channel */
        for (i = 0; i < V34FAX_EQ_LEN; i++)
        {
            pFaxShareMem->cEqCoef_ago[i].r = pV34Fax->pcEqCoef[i].r;
            pFaxShareMem->cEqCoef_ago[i].i = pV34Fax->pcEqCoef[i].i;
        }

        pMhsp->pfRealTime = V34Fax_PCR_Init;

        V34Fax_HW_Setup(pTable);

        pV34Fax->V34Fax_PCR_Start_Flag = FALSE;

        TRACE1("ACE: <DLE><PRI><DLE><P%d>", pAce->pDceToT30->priChnlBitRate * 24);
        TRACE0("PCR starts...");
    }

#if DRAWEPG
    pntLoc = *pEpg;

    *pEpg++ += 1;

    pEpg += (pntLoc << 1);

    *pEpg++ = pV34Fax->cqRotateIQ.r;
    *pEpg++ = pV34Fax->cqRotateIQ.i;
#endif

    pV34Fax->timeout_count++;

    if (pV34Fax->timeout_count > V34FAX_MPET_TIMEOUT)
    {
        TRACE0("V34 Time Out");
#if 1
        pV34Fax->timeout_count = 0;
        pV34Fax->Disconnect_flag = TRUE;
#else
        pV34Fax->MPET_Retrn_Type = RETRAIN_INIT;
        pV34Fax->MPET_Retrning = 0;
#endif
    }

    if (pV34Fax->silence_cnt > 300)/* LLL: DEBUG. if 100, current baseline receiver will disconnect after first page*/
    {
        pV34Fax->silence_cnt = 0;

        pV34Fax->Disconnect_flag = TRUE;

        TRACE0("No carrier");
    }

    if (pV34Fax->Disconnect_flag)
    {
        TRACE0("Line Disconnected");
        pV34Fax->Disconnect_flag = FALSE;

        pAce->V34Fax_State = V34FAX_DISCONNECT;

        Disconnect_Init(pTable);
        pAce->pDceToT30->isDleEot = TRUE; TRACE0("V34: <DLE><EOT>");
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
    }

    if (pV34Fax->MPET_Retrning == 0)
    {
        if (pV34Fax->MPET_Retrn_Type == RETRAIN_RESPOND)
        {
            V34Fax_MPET_Retrn_by_Remote(pV34Fax);
        }
        else if (pV34Fax->MPET_Retrn_Type == RETRAIN_INIT)
        {
            V34Fax_MPET_Retrn_by_Local(pV34Fax);
        }
    }
}


void V34Fax_MPET_Retrn_by_Remote(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->modem_mode == ANS_MODEM)
    {
        pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Tx_Init_VecTab;
        pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_Ans_Tx_Init_CntTab;

        pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Rx_Init_VecTab;
        pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Ans_Rx_Init_CntTab;
    }
    else
    {
        pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Tx_Init_VecTab;
        pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_Call_Tx_Init_CntTab;

        pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Rx_Init_VecTab;
        pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Call_Rx_Init_CntTab;
    }

    pV34Fax->nRxState = -1;

    pV34Fax->nTxState = 0;

    pV34Fax->nRxDelayCnt = 0;
    pV34Fax->nTxDelayCnt = 0;

    pV34Fax->MPET_Retrning = 1;
    pV34Fax->PPhDetect_Only = 1;

    TRACE0("Control Channel Retrain by Remote!");
}


void V34Fax_MPET_Retrn_by_Local(V34FaxStruct *pV34Fax)
{
    pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_MPET_Retrn_by_Local_Init_VecTab;
    pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_MPET_Retrn_by_Local_Init_CntTab;

    if (pV34Fax->modem_mode == ANS_MODEM)
    {
        pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Rx_Init_VecTab;
        pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Ans_Rx_Init_CntTab;
    }
    else
    {
        pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Rx_Init_VecTab;
        pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Call_Rx_Init_CntTab;
    }

    pV34Fax->nRxState = 1;

    pV34Fax->nTxState = -1;

    pV34Fax->nRxDelayCnt = 0;
    pV34Fax->nTxDelayCnt = 0;

    pV34Fax->MPET_Retrning = 1;
    pV34Fax->PPhDetect_Only = 1;

    TRACE0("Control Channel Retrain by local!");
}

#endif
