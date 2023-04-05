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

#if SUPPORT_V34FAX

#define EQ_COEFF_DUMP           0       /* t3 & t4 */

void V34Fax_PCR_Init(UBYTE **pTable)
{
    MhspStruct *pMH = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    QWORD *pPCMout = (QWORD *)(pTable[PCMOUTDATA_IDX]);
    UWORD *pClk = (UWORD *)(pTable[CLOCKDATA_IDX]);
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    MphStruc *pMphRx = &pV34Fax->MphRx;
    MphStruc *pMphTx = &pV34Fax->MphTx;
    Phase3_Info *p3 = &(pV34->p3);
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    QAMStruct *pQAM = &(pV34Tx->QAM);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    EqStruc *pEq = &(pV34Rx->Eq);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    DeModulateStruc *pDeMod = &pRx->DeMod;
    CircBuffer *pAscCBWr = (CircBuffer *)pTable[ASCCBWRDATA_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    UBYTE  flag;

    ModemData *pM;
    UWORD i;
    QWORD qTemp;

    pV34Fax->Enter_CC_Order++;

    pV34Fax->silence_cnt = 0;

    for (i = 0; i < 96; i++)
    {
        pPCMout[i] = 0;
    }

    /* not use timeout logic, think later */
    *pClk = pAce->StartTime;

    p3->TimeOut    = 30000; //V34PHASE3_TIMEOUT;
    pV34->Time_cnt = 0;   /* init time out counter for phase3 and 4 */

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        V34Fax_PCR_Call_Init(pTable);

        for (i = 0; i < 7; i++)
        {
            p3->pfTx[i] = (V34FnPtr)V34Fax_PCR_Tx_Fsm[i];
        }

        /* V34_Carrier_Init(pV34); */  /* Be care for FREQUENCY_OFFSET??? */
        pQAM->tx_carrier_freq    = V34_tCarrier_tbl[pV34Fax->symbol_rate][pV34Fax->high_carrier_freq];
        pQAM->qTx_carrier_offset = V34_tCarrier_offset[pV34Fax->symbol_rate][pV34Fax->high_carrier_freq];

        pV34Rx->tx_bit_rate = pMphRx->max_data_rate;
        pV34Tx->conv_state  = pMphRx->TrellisEncodSelect;
        pV34Tx->nlinear     = pMphRx->NonlinearEncodParam;
        pV34Tx->Min_Exp     = pMphRx->const_shap_select_bit;

        pMpTx->type = pMphRx->type;

#if TX_PRECODE_ENABLE
        if (pMpTx->type == MP_TYPE_1)
        {
            V34_Precoding_DataInit(&(pV34Tx->tx_precode));      /* just clear everything */

            V34_Precoding_CoefInit(&(pV34Tx->tx_precode), &(pMphRx->PrecodeCoeff_h[0].r)); // fixing bug 32
        }
#endif

        pM = (ModemData *)&V34_tINIT_PARA_DATA[pV34Fax->symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400];

        pV34Tx->b       = pM->b;
        pV34Tx->Aux.Ws  = pM->Ws;
        pV34Tx->Js      = pM->Js;
        pV34Tx->P       = pM->P;
        pShellMap->K    = pM->K;
        pShellMap->M    = V34_tMAP_M[pV34Fax->symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400][pV34Tx->Min_Exp];
        pV34Tx->SWP     = pM->SWP;
        pV34Tx->Aux.AMP = pM->AMP;
        pV34Tx->nbyte   = pM->nbyte;
        pV34Tx->nbits   = pM->nbits;

        /* Determine the Superframe sychronization bit pattern */
        if (pV34Tx->Js == 8)
        {
            pV34Tx->bit_inv_pat = 0x5FEE;
        }
        else
        {
            pV34Tx->bit_inv_pat = 0x1FEE;
        }

        pV34Tx->current_inv_bit = pV34Tx->bit_inv_pat;
        pV34Tx->current_SWP_bit = pV34Tx->SWP;
        pV34Tx->Aux.current_AMP_bit = pV34Tx->Aux.AMP;
        pV34Tx->current_J       = 0;
        pV34Tx->current_P       = 0;

        if (pV34Tx->b > 12)
        {
            pV34Tx->q = (pV34Tx->b - 12 - pShellMap->K) >> 3;
        }
        else
        {
            pV34Tx->q = 0;
        }

        V34_Shell_Mapper_Init(pShellMap);

        pShellMap->scram_idx = 0;
        pV34Tx->tx_COV_state = 0;

        if (pShellMap->M > 0)
        {
            V34_Calc_g2(pShellMap->M, &(pShellMap->pG2[0]));
            V34_Calc_g4(pShellMap->M, &(pShellMap->pG2[0]), &(pShellMap->pG4[0]));

            V34_Calc_g8(pShellMap->M, &(pShellMap->pG4[0]), &(pShellMap->pG8[0]));
            V34_Calc_z8(pShellMap->M, &(pShellMap->pG8[0]), &(pShellMap->pZ8[0]));
        }
        else
        {
            TRACE0("M=0");
        }

        V34_Modulate_Init(pV34->pTable, pQAM);

        /* Initilize the tx scale so that an expected power level is achieved */
        /* Note: based on the symbol rate, carrier freq, preemphasis filter */
        qTemp = V34_tPreempGain[pV34Fax->symbol_rate][pV34Fax->preemp_idx][pV34Fax->high_carrier_freq];

        pQAM->qTxScale = QQMULQR15(qTemp, V34_tTxScaleDnFrom12dBm0[pV34Fax->power_reduction]);

        V34_CircFIR_Filter_Init(&(pQAM->tx_preemp), pQAM->pqTx_preemp_dline, (QDWORD *)(V34_tPREEMP_COEF[pV34Fax->preemp_idx]), 7);

        pV34Fax->tx_sym_cnt = 0;

        pDpcs->BITSPERCYL = pMphTx->max_data_rate * 24;
    }
    else
    {
        pV34Fax->PCR_RXend_Flag = 0;

        for (i = 0; i < 7; i++)//9
        {
            p3->pfRx[i] = (V34FnPtr)V34Fax_PCR_Rx_Fsm[i];
        }

        /* The following should be initiated per image page */
        pRx->rx_sym_inidx  = 0;
        pRx->rx_sym_outidx = 0;
        pRx->rx_sym_count  = 0;

        pV34Rx->rx_bit_rate = pMphRx->max_data_rate;

        flag = pV34Rx->rx_bit_rate + 3;
        WrReg(pDpcs->MBSC, BIT_RATE, flag);

        pV34Rx->rx_me = pMphTx->const_shap_select_bit;

        /* recovery every preious parameters */
        DspcFir_2T3EQInit(&pTCR->eqfir, pEq->pcEqcoef, pEq->pcEqdline, pV34Fax->V34fax_EQ_Length);

        for (i = 0; i < pV34Fax->V34fax_EQ_Length; i++)
        {
#if EQ_COEFF_DUMP
            DumpTone3[DumpTone3_Idx++] = pV34Fax->pcCoef[i].r;
            DumpTone4[DumpTone4_Idx++] = pV34Fax->pcCoef[i].i;
#endif
            pTCR->eqfir.pcCoef[i] = pV34Fax->pcCoef[i];
        }

        if (pV34Fax->FreqOffset_Enable == 1)
        {
            pV34Rx->frequencyOffset = pV34Fax->frequencyOffset;

            V34_CarrierLoop_Init(pTCR);

            SinCos_Lookup_Fine(0, &pTCR->qS, &pTCR->qC);

            V34_Carrier_Init(pV34);

            V34_Demodulate_Init(&pRx->DeMod);
        }
        else
        {
            V34_CarrierLoop_Init(pTCR);

            SinCos_Lookup_Fine(0, &pTCR->qS, &pTCR->qC);

            pRx->rx_carrier_freq = V34_tCarrier_tbl[pV34Fax->symbol_rate][pV34Fax->high_carrier_freq];

            pRx->qRx_carrier_offset = V34_tCarrier_offset[pV34Fax->symbol_rate][pV34Fax->high_carrier_freq];
            pRx->qRx_carrier_offset += pV34Fax->carAdjust;
            TRACE1("Carrier Offset: %d", pV34Fax->carAdjust);

            V34_Demodulate_Init(&pRx->DeMod);

            pDeMod->rx_carrier_idx = pV34Fax->rx_carrier_idx;

            pDeMod->qRx_carrier_offset_idx = pV34Fax->qRx_carrier_offset_idx;
        }

        pAGC->qGain = pV34Fax->qGain;

        p3->tx_vec_idx = 0;
        p3->rx_vec_idx = 0;

        /* just clear structure */
        V34_VA_Init(&DeCode->Vdec);

#if RX_PRECODE_ENABLE
        V34_Precoding_DataInit(&(pRx->rx_precode));
        V34_Precoding_DataInit(&(pRx->VA_precode));
#endif
        V34_RXShell_Mapper_Init(&pRx->RxShellMap);

        V34_CarrierLoop_Init(pTCR);

        V34Fax_Timing_Rec_Init(pV34Fax);
        V34Fax_TimingLoop_Init(pV34Fax);

        pTCR->Tcount = 0;

#if PHASEJITTER
        pRx->jtvar.enable = 0;
#endif

        pV34Fax->HS_Data_flag = 0;
        pV34Fax->tx_sym_cnt   = 0;

        CB_InitCircBuffer(pAscCBWr, pTable[ASCTXDATA_IDX], BUFSIZE);
    }

    pMH->pfRealTime = V34Fax_PCR_Handler;

    pV34Fax->qdNoise_Egy_Ref = (pV34Fax->qdNoise_Egy >> 5) * pV34->NumSymbol;/* pV34Fax->qdNoise_Egy * 2 */

    pV34Fax->timeout_count = 0; TRACE0("V34: Timer init");

#if V34FAX_DEBUG_ERROR
    pV34Fax->current_error   = 0;
    pV34Fax->EQ_change_count = 0;
#endif
}


void V34Fax_PCR_Call_Init(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info *p3 = &(pV34->p3);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    QAMStruct *pQAM = &(pV34Tx->QAM);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    UBYTE  rate;
    QDWORD qdTemp;
    QWORD  qTemp;
    UBYTE symbol_rate;

    pQAM->power_reduction  = pV34Fax->power_reduction;
    pQAM->tx_preemp_idx    = pV34Fax->preemp_idx;
    pQAM->tx_high_freq     = pV34Fax->high_carrier_freq;

    symbol_rate = pV34Fax->symbol_rate;

    pV34Rx->tx_symbol_rate = symbol_rate;

    pV34Rx->md_length      = 0;

    pV34Rx->pfScram_byte   = V34_Scram_Byte_GPC;
    pV34Rx->pfScram_bit    = V34_Scram_Bit_GPC;
    pV34Rx->pfScramTrn_byte = V34_Scram_Byte_GPA;
    pV34Rx->pfScramTrn_bit = V34_Scram_Bit_GPA;
    pRx->pfDescram_byte    = V34_Descram_Byte_GPC;
    pRx->pfDescram_bit     = V34_Descram_Bit_GPC;

    pQAM->tx_carrier_freq    = V34_tCarrier_tbl[symbol_rate][pV34Fax->high_carrier_freq];
    pQAM->qTx_carrier_offset = V34_tCarrier_offset[symbol_rate][pV34Fax->high_carrier_freq];

    p3->K = 0;
    p3->I = 0;

    V34_S_Init(pV34);

    pV34Tx->z  = 0;
    pV34Tx->Y0 = 0;

    pV34Tx->tx_sym_inidx  = 0;
    pV34Tx->tx_sym_outidx = 0;
    pV34Tx->tx_COV_state  = 0;

    V34_Modulate_Init(pV34->pTable, pQAM);

#if TX_PRECODE_ENABLE
    V34_Precoding_DataInit(&(pV34Tx->tx_precode));
#endif

    pTCR->Tcount = 0;

    /* used in PSF in V34 Fax */
    //pV34->Echo.Ec_Insert.max_bulk_len = V34_MAX_BULK_LEN;
    //pV34->Echo.Ec_Insert.ne_sublen    = V34_EC_SUBLEN;

    /* Initilize the tx scale so that an expected power level is achieved */
    /*     Note: based on the symbol rate, carrier freq, preemphasis filter */
    qTemp = V34_tPreempGain[symbol_rate][pV34Fax->preemp_idx][pV34Fax->high_carrier_freq];

    pQAM->qTxScale = QQMULQR15(qTemp, V34_tTxScaleDnFrom12dBm0[pV34Fax->power_reduction]);

    V34_CircFIR_Filter_Init(&(pQAM->tx_preemp), pQAM->pqTx_preemp_dline, (QDWORD *)(V34_tPREEMP_COEF[pV34Fax->preemp_idx]), 7);

    /** Set modem modulation select register to V.34 **/
    PutReg(pDpcs->MMSR0, MODEM_V34);

    SetReg(pDpcs->MSR0, TRAIN);

    /* get data from DPCS */
    rate = RdReg(pDpcs->MBC1, MAX_RATE);

    if ((rate < DATA_RATE_2400) || (rate > DATA_RATE_33600))
    {
        pV34Rx->Host_maxbitrate = V34_BIT_33600;
    }
    else
    {
        pV34Rx->Host_maxbitrate = rate - 3;    /* Convert Value to Bitrate */
    }

    rate = RdReg(pDpcs->MBC0, MIN_RATE);

    if ((rate < DATA_RATE_2400) || (rate > DATA_RATE_33600))
    {
        pV34Rx->Host_minbitrate = V34_BIT_2400;
    }
    else
    {
        pV34Rx->Host_minbitrate = rate - 3;    /* Convert Value to Bitrate */
    }

    p3->TimeOut    = V34PHASE3_TIMEOUT;
    pV34->Time_cnt = 0;   /* init time out counter for phase3 and 4 */

    p3->symbol_counter = 0;

    pV34Fax->PCR_TXend_Flag = 0;

    p3->tx_vec_idx = 0;
    p3->rx_vec_idx = 0;

    /* RTD factor convert ms to per interupt handler call (24 samples per call) */
    pV34->qRTD_factor = V34_tRTD_FACTOR[symbol_rate];

    qTemp  = pV34Rx->RTD_PH2_symbol + V34_PHASE2_SYS_DELAY;
    qdTemp = QQMULQD(qTemp, q5_OVER_6);

    qTemp  = (QWORD)(qdTemp >> 13) + 2800;
    p3->pqTime_out_value[0] = QQMULQ15(pV34->qRTD_factor, qTemp);

    qTemp  = (QWORD)(qdTemp >> 14) + pV34Rx->md_length * 35;
    p3->pqTime_out_value[1] = QQMULQ15(pV34->qRTD_factor, qTemp);

    p3->pqTime_out_value[1] += 40;
}

#endif
