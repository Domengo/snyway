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

#include "ptable.h"
#include "hwapi.h"
#include "v34fext.h"
#include "mhsp.h"

#if SUPPORT_V34FAX

void V34Fax_MPET_Init(UBYTE **pTable)
{
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    SWORD *pPCMin = (QWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (QWORD *)(pTable[PCMOUTDATA_IDX]);
    UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];
    CircBuffer *pAscCBWr = (CircBuffer *)pTable[ASCCBWRDATA_IDX];
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);
    UBYTE i;

    CB_InitCircBuffer(pAscCBWr, pTable[ASCTXDATA_IDX], BUFSIZE);
    CB_InitCircBuffer(pAscCBRd, pTable[ASCRXDATA_IDX], BUFSIZE);

    pV34Fax->ubMphDetect = 0;/* should check other part too */

    if (pV34Fax->Enter_CC_Order == 0)
    {
        if (pV34Fax->modem_mode == CALL_MODEM)
        {
            pV34Fax->qGain = pFaxShareMem->qFaxAgc << 1;/* need to check whether need to double */
        }
    }

    for (i = 0; i < PCM_BUFSIZE; i++) /* clear max size, not just V34FAX_BUF_LEN */
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

    pMhsp->pfRealTime = V34Fax_MPET_Handle;

    pMhsp->RtCallRate = 1;
    pMhsp->RtBufSize  = V34FAX_BUF_LEN;

    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;
    pHwApi[1] = HWAPI_7200SR;
    pHwApi[2] = pMhsp->RtCallRate * pMhsp->RtBufSize;

    pV34Fax->reRateFlag = 0;

    V34Fax_MPET_Fsm_Init(pV34Fax);

    V34Fax_MPET_Tx_Init(pV34Fax);

    V34Fax_MPET_Rx_Init(pV34Fax);

    if (pV34Fax->Enter_CC_Order != 0)
    {
        for (i = 0; i < V34FAX_EQ_LEN; i++)
        {
            pV34Fax->pcEqCoef[i] = pFaxShareMem->cEqCoef_ago[i];
        }
    }

    pV34Fax->nTxState     = 0;
    pV34Fax->pfTxSetUpVec = pV34Fax->pfTxSetUpVecTab[0];
    pV34Fax->nTxDelayCnt  = pV34Fax->nTxDelayCntTab[0];

    pV34Fax->pfTxSetUpVec(pV34Fax);

    pV34Fax->nRxState     = 0;
    pV34Fax->pfRxSetUpVec = pV34Fax->pfRxSetUpVecTab[0];
    pV34Fax->nRxDelayCnt  = pV34Fax->nRxDelayCntTab[0];

    pV34Fax->pfRxSetUpVec(pV34Fax);

    pV34Fax->V34Fax_PCR_Start_Flag = FALSE;

    pV34Fax->tx_sym_cnt      = 0;
    pV34Fax->timeout_count   = 0; TRACE0("V34: Timer init");
    pV34Fax->Disconnect_flag = FALSE;

    pV34Fax->MPET_Retrn_AC_count = 0;
}


void V34Fax_MPET_Fsm_Init(V34FaxStruct *pV34Fax)
{
    ACEStruct *pAce = (ACEStruct *)pV34Fax->pTable[ACE_STRUC_IDX];

    if (pAce->pT30ToDce->isReqRateReduction)
    {
        // fixing issue 173, should use rerate instead
        pV34Fax->reRateFlag = RERATE_INIT;
        pAce->pT30ToDce->isReqRateReduction = FALSE;
    }

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        if ((pV34Fax->Enter_CC_Order == 0) || (pV34Fax->reRateFlag == RERATE_INIT))
        {
            pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Tx_Init_VecTab;
            pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_Call_Tx_Init_CntTab;

            pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Rx_Init_VecTab;
            pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Call_Rx_Init_CntTab;
        }
        else if (pV34Fax->reRateFlag == RERATE_RESPOND)
        {
            pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Tx_Rerate_by_Remote_Init_VecTab;
            pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_Call_Tx_Rerate_by_Remote_Init_CntTab;

            /* RX is the same as initial train */
            pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Rx_Init_VecTab;
            pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Call_Rx_Init_CntTab;
        }
        else
        {
            pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Tx_NoFirstBlock_Init_VecTab;
            pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_Call_Tx_NoFirstBlock_Init_CntTab;

            pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Call_Rx_NoFirstBlock_Init_VecTab;
            pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Call_Rx_NoFirstBlock_Init_CntTab;
        }
    }
    else
    {
        if ((pV34Fax->Enter_CC_Order == 0) || (pV34Fax->reRateFlag == RERATE_RESPOND))
        {
            pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Tx_Init_VecTab;
            pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_Ans_Tx_Init_CntTab;

            pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Rx_Init_VecTab;
            pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Ans_Rx_Init_CntTab;
        }
        else if (pV34Fax->reRateFlag == RERATE_INIT)
        {
            /* RX is the same as initial train */
            pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Tx_Init_VecTab;
            pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_Ans_Tx_Init_CntTab;

            pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Rx_Rerate_by_Local_Init_VecTab;
            pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Ans_Rx_Rerate_by_Local_Init_CntTab;
        }
        else
        {
            pV34Fax->pfTxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Tx_NoFirstBlock_Init_VecTab;
            pV34Fax->nTxDelayCntTab  = (SWORD *)V34Fax_Ans_Tx_NoFirstBlock_Init_CntTab;

            pV34Fax->pfRxSetUpVecTab = (V34FaxFnptr *)V34Fax_Ans_Rx_NoFirstBlock_Init_VecTab;
            pV34Fax->nRxDelayCntTab  = (SWORD *)V34Fax_Ans_Rx_NoFirstBlock_Init_CntTab;
        }
    }
}


void V34Fax_MPET_Tx_Init(V34FaxStruct *pV34Fax)
{
    pV34Fax->ubTxBitsPerSym = V34FAX_SYM_BITS_1200;
    pV34Fax->udScramSReg = 0;

    pV34Fax->ubEncoderOldPhase = 0;

    V34Fax_PSF_Init(pV34Fax);

    V34Fax_Modulation_Init(pV34Fax);
}


void V34Fax_MPET_Rx_Init(V34FaxStruct *pV34Fax)
{
    pV34Fax->ubRxBitsPerSym = V34FAX_SYM_BITS_1200;

    pV34Fax->PPh_DetCount  = 0;
    pV34Fax->ALT_DetCount  = 0;
    pV34Fax->ONEs_DetCount = 0;

    pV34Fax->ubDecoderOldPhase = 0;

    pV34Fax->qRotateCos = 0x7FFF;
    pV34Fax->qRotateSin = 0x0000;

    pV34Fax->qEqBeta = 4096;

    pV34Fax->udDescramSReg = 0;

    V34Fax_Rx_BPF_Init(pV34Fax);
    V34Fax_Demodulation_Init(pV34Fax);
    V34Fax_Timing_Recovery_Init(pV34Fax);

    DspcFirInit(&pV34Fax->cfirEq, pV34Fax->pcEqCoef, pV34Fax->pcEqDelay, V34FAX_EQ_LEN);

    if (pV34Fax->modem_mode == ANS_MODEM)
    {
        DSPD_DFT_Init(&(pV34Fax->Dft), V34FAX_PPH_SH_DETECT_LEN, V34FAX_PPH_SH_DETECT_NUMTONES,
                      (QWORD *)qV34Fax_AnsDetectToneCoefTab,
                      pV34Fax->qdMagSq, pV34Fax->qQ1Q2, V34FAX_PPH_SH_DETECT_BITSHIFT);

        DSPD_DFT_Init(&(pV34Fax->Dft_AC), V34FAX_AC_DETECT_LEN, 3,
                      (QWORD *)qV34Fax_AnsDetectACtoneCoefTab,
                      pV34Fax->qdMagSq_AC, pV34Fax->qQ1Q2_AC, V34FAX_AC_DETECT_BITSHIFT);
    }
    else
    {
        DSPD_DFT_Init(&(pV34Fax->Dft), V34FAX_PPH_SH_DETECT_LEN, V34FAX_PPH_SH_DETECT_NUMTONES,
                      (QWORD *)qV34Fax_CallDetectToneCoefTab,
                      pV34Fax->qdMagSq, pV34Fax->qQ1Q2, V34FAX_PPH_SH_DETECT_BITSHIFT);

        DSPD_DFT_Init(&(pV34Fax->Dft_AC), V34FAX_AC_DETECT_LEN, 3,
                      (QWORD *)qV34Fax_CallDetectACtoneCoefTab,
                      pV34Fax->qdMagSq_AC, pV34Fax->qQ1Q2_AC, V34FAX_AC_DETECT_BITSHIFT);
    }

    V34Fax_Timing_Update_Init(pV34Fax);
    V34Fax_Carrier_Update_Init(pV34Fax);

    pV34Fax->MPET_Retrn_Type = 0;

    pV34Fax->MPET_Retrning = 0;

    pV34Fax->PPhDetect_Only = 0;

    pV34Fax->AC_Detect_Start_Flag = 0;
}

#endif
