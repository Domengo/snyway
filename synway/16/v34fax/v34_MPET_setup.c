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
#include "aceext.h"
#include "mhsp.h"
#include "ptable.h"

#if SUPPORT_V34FAX

#define V34FAX_EQ_BETA_1        (8192)
#define V34FAX_EQ_BETA_2        (7168)
#define V34FAX_EQ_BETA_3        (6144)
#define V34FAX_EQ_BETA_DATA     (1024)


void V34Fax_Dummy(V34FaxStruct *pV34Fax)
{
}


void V34Fax_Silence_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    pV34Fax->qEqBeta = V34FAX_EQ_BETA_DATA;

    pV34Fax->PPh_DetCount  = 0;
    pV34Fax->ALT_DetCount  = 0;
    pV34Fax->ONEs_DetCount = 0;

    V34Fax_Rx_SetUp(pV34Fax);

    pV34Fax->pfDataOutVec = V34Fax_Silence_Rx;

    pV34Fax->silence_cnt = 0;

    TRACE0("Rx Silence");
}


void V34Fax_PPh_Sh_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    V34Fax_Rx_SetUp(pV34Fax);

    if (pV34Fax->modem_mode == ANS_MODEM)
    {
        pV34Fax->pfDataOutVec = V34Fax_PPh_Sh_AnsRx;
    }
    else
    {
        pV34Fax->pfDataOutVec = V34Fax_PPh_Sh_CallRx;
    }

    pV34Fax->qEqBeta = V34FAX_EQ_BETA_2;

    pV34Fax->PPh_DetCount = 0;

    pV34Fax->pfDetectVec = V34Fax_PPh_Sh_Detection;  /* Detect Between Sh and PPh */

    pV34Fax->PPS_detect_flag = 1;

    pV34Fax->ShDetected  = 0;
    pV34Fax->PPhDetected = 0;
    pV34Fax->EgyDetected = 0;

    pV34Fax->pfSliceVec     = V34Fax_Dummy;
    pV34Fax->pfTimUpdateVec = V34Fax_Dummy;
    pV34Fax->pfDerotateVec  = V34Fax_Dummy;
    pV34Fax->pfCarrierVec   = V34Fax_Dummy;
    pV34Fax->pfEqUpdateVec  = V34Fax_Dummy;
    pV34Fax->pfDecodeVec    = V34Fax_Dummy;
    pV34Fax->pfDescramVec   = V34Fax_Dummy;

    pV34Fax->E_detected = 0;

    TRACE0("Rx Sh");
}


void V34Fax_ShB_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->ShDetected)
    {
        pV34Fax->qEqBeta = 4096;

        V34Fax_Rx_SetUp(pV34Fax);

        pV34Fax->pfEqVec        = V34Fax_SyncEq;
        pV34Fax->pfSliceVec     = V34Fax_Dummy;
        pV34Fax->pfTimUpdateVec = V34Fax_Dummy;
        pV34Fax->pfEqUpdateVec  = V34Fax_Dummy;
        pV34Fax->pfDataOutVec   = V34Fax_ShB_Rx_ReSync;

        V34Fax_SyncEq_Init(pV34Fax);

        TRACE0("Rx ShB");
    }
    else
    {
        pV34Fax->MPET_Retrning = 0;
        pV34Fax->MPET_Retrn_Type = RETRAIN_INIT;
        TRACE0("V34: Neither Sh nor PPh is detected");
    }
}


void V34Fax_PPh_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    pV34Fax->qEqBeta = V34FAX_EQ_BETA_3;

    V34Fax_Rx_SetUp(pV34Fax);

#if 0 /* FOR TESTING */

    pV34Fax->pfDetectVec = V34Fax_PPh_Sh_Detection;

    if (pV34Fax->modem_mode == ANS_MODEM)
    {
        pV34Fax->pfDataOutVec = V34Fax_PPh_Sh_AnsRx;
    }
    else
    {
        pV34Fax->pfDataOutVec = V34Fax_PPh_Sh_CallRx;
    }

#else

    pV34Fax->pfDetectVec   = V34Fax_PPh_Detection;
    pV34Fax->pfDataOutVec  = V34Fax_PPh_Rx;

#endif

    pV34Fax->pfSliceVec    = V34Fax_Dummy;
    pV34Fax->pfDerotateVec = V34Fax_Dummy;
    pV34Fax->pfEqUpdateVec = V34Fax_Dummy;
    pV34Fax->pfCarrierVec  = V34Fax_Dummy;
    pV34Fax->pfDecodeVec   = V34Fax_Dummy;
    pV34Fax->pfDescramVec  = V34Fax_Dummy;

    pV34Fax->E_detected = 0;

    TRACE0("Rx PPh");
}


void V34Fax_PPh_Train_SetUp(V34FaxStruct *pV34Fax)
{
    UWORD i;

    pV34Fax->qEqBeta    = V34FAX_EQ_BETA_1;
    pV34Fax->ubPPh_kref = 0;
    pV34Fax->ubPPh_Iref = 0;

    V34Fax_Rx_SetUp(pV34Fax);

    pV34Fax->pfSliceVec     = V34Fax_PPh_Slice;
    pV34Fax->pfTimUpdateVec = V34Fax_Dummy;
    pV34Fax->pfCarrierVec   = V34Fax_Dummy;
    pV34Fax->pfDataOutVec   = V34Fax_PPh_Train;

    if (pV34Fax->reRateFlag == RERATE_RESPOND)
    {
        /* Only 28 symbols of PPh in this case */
        pV34Fax->nRxDelayCnt = 26;
        pV34Fax->ubPPh_kref  = 3; /* Missed first 6 symbols of PPh */
        pV34Fax->ubPPh_Iref  = 0;

        for (i = 0; i < V34FAX_EQ_LEN; i++)
        {
            pV34Fax->pcEqCoef[i].r = 0;
            pV34Fax->pcEqCoef[i].i = 0;
        }
    }

    TRACE0("PPh Training");
}


void V34Fax_ALT_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    pV34Fax->qEqBeta = V34FAX_EQ_BETA_2;

    pV34Fax->ALT_DetCount = 0;

    V34Fax_Rx_SetUp(pV34Fax);

    pV34Fax->pfTimUpdateVec = V34Fax_Dummy;
    pV34Fax->pfCarrierVec   = V34Fax_Dummy;
    pV34Fax->pfDataOutVec   = V34Fax_ALT_Rx;

    TRACE0("Rx ALT");
}


void V34Fax_MPh_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    pV34Fax->ubMphDetect = 0;

    pV34Fax->qEqBeta = V34FAX_EQ_BETA_2;

    V34Fax_Rx_SetUp(pV34Fax);

    pV34Fax->pfDataOutVec = V34Fax_MPh_Rx;

    V34Fax_Detect_Mph_Init(&pV34Fax->MphRx);

    TRACE0("Rx MPh");
}


void V34Fax_E_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->ubMphDetect > 0 || pV34Fax->AC_Detect_Start_Flag)
    {
        ACEStruct *pAce = (ACEStruct *)pV34Fax->pTable[ACE_STRUC_IDX];
        DceToT30Interface *pDceToT30 = pAce->pDceToT30;

        pV34Fax->qEqBeta = V34FAX_EQ_BETA_2;
        pV34Fax->udEpattern = 0;

        V34Fax_Rx_SetUp(pV34Fax);

        pV34Fax->pfDataOutVec = V34Fax_E_Rx;

        TRACE0("Rx E");

        if (pDceToT30->selectedV34Chnl != V34_CONTROL_CHNL)
        {
            pDceToT30->selectedV34Chnl = V34_CONTROL_CHNL; TRACE0("V34: <DLE><CTRL>");// fixing issue 140
        }
    }
    else
    {
        // MPh detection timeout, need to initiate a control channel retrain
        pV34Fax->MPET_Retrning = 0;
        pV34Fax->MPET_Retrn_Type = RETRAIN_INIT;
        TRACE0("V34: MPh is not detected");
    }
}


void V34Fax_T30_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->E_detected)
    {
        MphStruc *pMphTx = &pV34Fax->MphTx;

        pV34Fax->qEqBeta = V34FAX_EQ_BETA_DATA;

        pV34Fax->ONEs_DetCount = 0;

        V34Fax_Rx_SetUp(pV34Fax);

        pV34Fax->pfDataOutVec = V34Fax_T30_Rx;
        pV34Fax->ubRxBitsPerSym = pMphTx->control_channel_rate == 0 ? V34FAX_SYM_BITS_1200 : V34FAX_SYM_BITS_2400;

        TRACE0("Rx T30");
    }
    else
    {
        // E detection timeout, need to initiate a control channel retrain
        pV34Fax->MPET_Retrning = 0;
        pV34Fax->MPET_Retrn_Type = RETRAIN_INIT;
        TRACE0("V34: E is not detected");
    }
}


void V34Fax_ONEs_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    pV34Fax->qEqBeta = V34FAX_EQ_BETA_DATA;

    pV34Fax->Ones_Rx_count0 = 0;

    V34Fax_Rx_SetUp(pV34Fax);

    pV34Fax->pfDataOutVec = V34Fax_ONEs_Rx;

    pV34Fax->silence_cnt = 0;

    TRACE0("Rx ONEs");
}


void V34Fax_Rx_SetUp(V34FaxStruct *pV34Fax)
{
    pV34Fax->pfDetectVec    = V34_Dummy;
    pV34Fax->pfEqVec        = V34Fax_EQ;
    pV34Fax->pfSliceVec     = V34Fax_Slice;
    pV34Fax->pfTimUpdateVec = V34Fax_Timing_Update;
    pV34Fax->pfDerotateVec  = V34Fax_Derotate;
    pV34Fax->pfCarrierVec   = V34Fax_Carrier_Update;
    pV34Fax->pfEqUpdateVec  = V34Fax_EqUpdate;
    pV34Fax->pfDecodeVec    = V34Fax_Decoder;
    pV34Fax->pfDescramVec   = V34Fax_Descramble;
    pV34Fax->pfDataOutVec   = V34Fax_Dummy;
}


void V34Fax_Silence_Tx_Setup(V34FaxStruct *pV34Fax)
{
    pV34Fax->pfDataInVec = V34Fax_Silence_Tx;
    pV34Fax->pfScramVec  = V34Fax_Dummy;
    pV34Fax->pfEncodeVec = V34Fax_Dummy;

    TRACE0("Tx Silence");
}


void V34Fax_Sh_Tx_Setup(V34FaxStruct *pV34Fax)
{
    /* V34Fax_TxSendSh will XOR this with 0x3 to toggle between point 0 and 3 */
    pV34Fax->uScramOutBits = 0x0000;

    pV34Fax->pfDataInVec = V34Fax_Sh_ShB_Tx;
    pV34Fax->pfScramVec  = V34Fax_Dummy;
    pV34Fax->pfEncodeVec = V34Fax_Dummy;

    TRACE0("Tx Sh");
}


void V34Fax_MPET_AC_Tx_Setup(V34FaxStruct *pV34Fax)
{
    pV34Fax->uScramOutBits = 0x0000;

    pV34Fax->pfDataInVec = V34Fax_MPET_AC_Tx;
    pV34Fax->pfScramVec  = V34Fax_Dummy;
    pV34Fax->pfEncodeVec = V34Fax_Dummy;

    TRACE0("Tx AC");
}

void V34Fax_ShB_Tx_Setup(V34FaxStruct *pV34Fax)
{
    /* V34Fax_TxSendSh will XOR this with 0x3 to toggle between point 1 and 2 */
    pV34Fax->uScramOutBits = 0x0002;

    pV34Fax->pfDataInVec = V34Fax_Sh_ShB_Tx;
    pV34Fax->pfScramVec  = V34Fax_Dummy;
    pV34Fax->pfEncodeVec = V34Fax_Dummy;

    TRACE0("Tx ShB");
}


void V34Fax_PPh_Tx_Setup(V34FaxStruct *pV34Fax)
{
    /* Init PPh generator index */
    pV34Fax->ubPPh_k = 0;
    pV34Fax->ubPPh_I = 0;

    pV34Fax->pfDataInVec = V34Fax_PPh_Tx;
    pV34Fax->pfScramVec  = V34Fax_Dummy;
    pV34Fax->pfEncodeVec = V34Fax_Dummy;

    TRACE0("Tx PPH");
}


void V34Fax_ALT_Tx_Setup(V34FaxStruct *pV34Fax)
{
    pV34Fax->udScramSReg  = 0;
    pV34Fax->ubEncoderOldPhase = 0;

    pV34Fax->pfDataInVec = V34Fax_ALT_Tx;
    pV34Fax->pfScramVec  = V34Fax_Scramble;
    pV34Fax->pfEncodeVec = V34Fax_Encode;

    TRACE0("Tx ALT");
}


void V34Fax_MPh_Tx_Setup(V34FaxStruct *pV34Fax)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pV34Fax->pTable[DPCS_STRUC_IDX];
    ACEStruct *pAce = (ACEStruct *)pV34Fax->pTable[ACE_STRUC_IDX];
    UBYTE Max_Rate_ofFIS;
    MphStruc *pMphTx = &pV34Fax->MphTx;
    MphStruc *pMphRx = &pV34Fax->MphRx;
    int i;

    pV34Fax->ubBitCounter = 0;

    Max_Rate_ofFIS = pAce->pT30ToDce->initV34PrimRateMax;

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
		pMphTx->type = MP_TYPE_0;

        if (Max_Rate_ofFIS < TX_RATE_LIMIT)
        {
            pMphTx->max_data_rate = Max_Rate_ofFIS;
        }
        else
        {
            pMphTx->max_data_rate = TX_RATE_LIMIT;
        }

        pMphTx->control_channel_rate  = pAce->pT30ToDce->initV34CntlRate - 1;
        pMphTx->TrellisEncodSelect    = 0;

        pMphTx->const_shap_select_bit = 1;

		pMphTx->data_rate_cap_mask = (1 << pMphTx->max_data_rate) - 2;

		if (pV34Fax->symbol_rate == V34_SYM_2400)
		{
			pMphTx->data_rate_cap_mask++;
		}

        pMphTx->asymmetric_control_rate = 0;     /* 1200bps for Tx and Rx ctrl ch. If this is changed, T30 should also be modified */
    }
    else    /* Answer rate selected in PCET */
    {
#if RX_PRECODE_ENABLE
		pMphTx->type = MP_TYPE_1;
#else
		pMphTx->type = MP_TYPE_0;
#endif

        pMphTx->control_channel_rate = pAce->pT30ToDce->initV34CntlRate - 1;
        pMphTx->TrellisEncodSelect   = 0;

        pMphTx->const_shap_select_bit   = 1;
        pMphTx->asymmetric_control_rate = 0;     /* 1200bps for Tx and Rx ctrl ch */

        if ((Max_Rate_ofFIS <= RX_RATE_LIMIT) && (Max_Rate_ofFIS < pMphRx->max_data_rate))
        {
            pMphRx->max_data_rate = Max_Rate_ofFIS;
        }
        else if ((RX_RATE_LIMIT <= Max_Rate_ofFIS) && (RX_RATE_LIMIT < pMphRx->max_data_rate))
        {
            pMphRx->max_data_rate = RX_RATE_LIMIT;
        }

        pMphTx->max_data_rate = pMphRx->max_data_rate; /* only for Answer Mph Tx */

        pMphTx->data_rate_cap_mask = (1 << pMphTx->max_data_rate) - 2;

		if (pV34Fax->symbol_rate == V34_SYM_2400)
		{
			pMphTx->data_rate_cap_mask++;
		}
    }

    pMphTx->NonlinearEncodParam = 0;

    if (pMphTx->type == MP_TYPE_0)
    {
        pV34Fax->uMPhSize = MP_TYPE_0_LEN;
    }
    else
    {
        pV34Fax->uMPhSize = MP_TYPE_1_LEN;
    }

    WrReg(pDpcs->MBC1, MAX_RATE, pMphTx->max_data_rate + 3);
    V34Fax_Create_Mph(pV34Fax->pMPhByteBuf, pMphTx);

    pV34Fax->pfDataInVec = V34Fax_MPh_Tx;
    pV34Fax->pfScramVec  = V34Fax_Scramble;
    pV34Fax->pfEncodeVec = V34Fax_Encode;

    TRACE0("Tx MPh");
}


void V34Fax_E_Tx_Setup(V34FaxStruct *pV34Fax)
{
    ACEStruct *pAce = (ACEStruct *)pV34Fax->pTable[ACE_STRUC_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
//    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;

    pV34Fax->pfDataInVec = V34Fax_E_Tx;
    pV34Fax->pfScramVec  = V34Fax_Scramble;
    pV34Fax->pfEncodeVec = V34Fax_Encode;

    TRACE0("Tx E");

//    if (pT30ToDce->isReqRateReduction)
//    {
//        CHAR temp[40];
//
//        pT30ToDce->isReqRateReduction = FALSE;
//        memset(temp, 0, sizeof(temp));
//        sprintf(temp, "<DLE><CTRL><DLE><P%d><DLE><C%d>", pDceToT30->priChnlBitRate * 24, pDceToT30->ctrlChnlBitRate * 12);
//        pDceToT30->selectedV34Chnl = V34_CONTROL_CHNL; TRACE1("V34: %s", temp);
//#if SUPPORT_T31_PARSER
//        PutStringToCB(pAce->pCBOut, temp, strlen(temp));
//#endif
//    }
//    else
        if (pDceToT30->selectedV34Chnl != V34_CONTROL_CHNL)
    {
        pDceToT30->selectedV34Chnl = V34_CONTROL_CHNL; TRACE0("V34: <DLE><CTRL>");
    }
}


void V34Fax_T30_Tx_SetUp(V34FaxStruct *pV34Fax)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pV34Fax->pTable[DPCS_STRUC_IDX];
    MhspStruct *pMhsp = (MhspStruct *)pV34Fax->pTable[MHSP_STRUC_IDX];
    MphStruc *pMphTx = &pV34Fax->MphTx;

    /* setup tx functions */
    pV34Fax->pfDataInVec = V34Fax_T30_Tx;
    pV34Fax->pfScramVec  = V34Fax_Scramble;
    pV34Fax->pfEncodeVec = V34Fax_Encode;

    pV34Fax->ubTxBitsPerSym = pMphTx->control_channel_rate == 0 ? V34FAX_SYM_BITS_1200 : V34FAX_SYM_BITS_2400;
    pDpcs->BITSPERCYL = pMhsp->RtCallRate * pV34Fax->ubTxBitsPerSym * V34FAX_SYM_NUM;

    TRACE0("Tx T30");
}

#endif
