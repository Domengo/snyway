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

#include <string.h>
#include "v34ext.h"
#include "mhsp.h"
#include "ptable.h"         /* Data memory table defines   */

/*.........................................................................*/
/* Global Definition of variables that use throughout the program.         */
/*.........................................................................*/
/* Info_0Struc info0c, info0a ---- Ph2Struct.I0a, I0cn */
/* Info_1cStruc info1c ---- Ph2Struct.I1c */
/* Info_1aStruc info1a ---- Ph2Struct.I1a */

#if 0 /// LLL temp
void V34_Phase3_Init(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    Ph2Struct *pPH2 = (Ph2Struct *)(pTable[PH2_STRUC_IDX]);
    Info_1aStruc *pI1a = &(pPH2->I1a);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    QAMStruct *pQAM = &(pV34Tx->QAM);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    V34StateStruc *pV34State = &pRx->V34State;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    UBYTE  rate;
    V34SetupStruct CallData;
    V34SetupStruct AnsData;
    V34SetupStruct *pRemoteData;
    V34SetupStruct *pLocalData;
#if FREQUENCY_OFFSET
    EchoStruc *ec = &(pV34->Echo);
    UBYTE  i;
#endif
    SWORD  AGC_gain;
    UBYTE modem_mode;

    UBYTE S2743;                 /* bit 12 */
    UBYTE S2800;                 /* bit 13 */
    UBYTE S3429;                 /* bit 14 */
    UBYTE S3000_low_carrier;     /* bit 15 */
    UBYTE S3000_high_carrier;    /* bit 16 */
    UBYTE S3200_low_carrier;     /* bit 17 */
    UBYTE S3200_high_carrier;    /* bit 18 */
    UBYTE S3429_enable;          /* bit 19 */
    UBYTE reduce_transmit_power; /* bit 20 */
    UBYTE max_allowed_diff;      /* bit 21-23, value 0-5 */
    UBYTE transmit_from_CME;     /* bit 24 */
    UBYTE V34bis;                /* bit 25 */
    UBYTE V34bisL;               /* bit 25 */

    SWORD RTD_PH2_symbol;
    UBYTE Retrain_Times;

    Info_0Struc *pInfo;

    QWORD  qNLDFactor;

    QDWORD  drift ;
    UDWORD  thresh;

    modem_mode                = pPH2->modem_mode;
    /************* Copy informations of Phase2 to setup V.34 ***************/
    AnsData.bit_rate          = pI1a->pro_data_rate;
    AnsData.high_carrier_freq = pI1a->INFO1a_Bit25;
    AnsData.symbol_rate       = pI1a->sym_rate_call_to_answer;
    AnsData.preemp_idx        = pI1a->pre_em_index;
    AnsData.power_reduction   = pI1a->min_power_reduction;
    AnsData.md_length         = pI1a->length_MD;

    CallData.symbol_rate      = pI1a->sym_rate_answer_to_call;//???
    CallData.md_length        = pPH2->I1c.length_MD;
    CallData.power_reduction  = pPH2->I1c.min_power_reduction;

    /* TRACE2("AnsData.power_reduction = %d, CallData.power_reduction = %d", AnsData.power_reduction, CallData.power_reduction); */

    switch (CallData.symbol_rate)
    {
        case V34_SYM_2400:
            CallData.bit_rate          = pPH2->I1c.pro_data_rate_S2400;
            CallData.high_carrier_freq = pPH2->I1c.high_carrier_freq_S2400;
            CallData.preemp_idx        = pPH2->I1c.pre_em_index_S2400;
            break;

        case V34_SYM_2743:
            CallData.bit_rate          = pPH2->I1c.pro_data_rate_S2743;
            CallData.high_carrier_freq = pPH2->I1c.high_carrier_freq_S2743;
            CallData.preemp_idx        = pPH2->I1c.pre_em_index_S2743;
            break;

        case V34_SYM_2800:
            CallData.bit_rate          = pPH2->I1c.pro_data_rate_S2800;
            CallData.high_carrier_freq = pPH2->I1c.high_carrier_freq_S2800;
            CallData.preemp_idx        = pPH2->I1c.pre_em_index_S2800;
            break;

        case V34_SYM_3000:
            CallData.bit_rate          = pPH2->I1c.pro_data_rate_S3000;
            CallData.high_carrier_freq = pPH2->I1c.high_carrier_freq_S3000;
            CallData.preemp_idx        = pPH2->I1c.pre_em_index_S3000;
            break;

        case V34_SYM_3200:
            CallData.bit_rate          = pPH2->I1c.pro_data_rate_S3200;
            CallData.high_carrier_freq = pPH2->I1c.high_carrier_freq_S3200;
            CallData.preemp_idx        = pPH2->I1c.pre_em_index_S3200;
            break;

        case V34_SYM_3429:
            CallData.bit_rate          = pPH2->I1c.pro_data_rate_S3429;
            CallData.high_carrier_freq = pPH2->I1c.high_carrier_freq_S3429;
            CallData.preemp_idx        = pPH2->I1c.pre_em_index_S3429;
            break;
    }

    TRACE0("      Info1c          Info1a");
    TRACE2("sym    %d              %d", CallData.symbol_rate, AnsData.symbol_rate);
    TRACE2("bps    %d              %d", CallData.bit_rate, AnsData.bit_rate);
    TRACE2("freq   %d              %d", CallData.high_carrier_freq, AnsData.high_carrier_freq);
    TRACE2("preemp %d              %d", CallData.preemp_idx, AnsData.preemp_idx);
    TRACE2("FOffst %d              %d", pPH2->I1c.freq_offset, pPH2->I1a.freq_offset);

    RTD_PH2_symbol = pPH2->RTD_Symbol;
    Retrain_Times  = pPH2->Ph2State.Retrain_Times;

    AGC_gain = pPH2->AGC_gain;

    qNLDFactor     = pPH2->qNLDFactor;
    drift          = pPH2->drift;
    TRACE1("drift=%d", drift);

    if (modem_mode == CALL_MODEM)
    {
        pInfo    = &(pPH2->I0a);
        V34bisL  = pPH2->I0c.V34bis;
    }
    else
    {
        pInfo    = &(pPH2->I0c);
        V34bisL  = pPH2->I0a.V34bis;
    }

    S2743                = pInfo->S2743;
    S2800                = pInfo->S2800;
    S3429                = pInfo->S3429;
    S3000_low_carrier    = pInfo->S3000_low_carrier;
    S3000_high_carrier   = pInfo->S3000_high_carrier;
    S3200_low_carrier    = pInfo->S3200_low_carrier;
    S3200_high_carrier   = pInfo->S3200_high_carrier;
    S3429_enable         = pInfo->S3429_enable;
    reduce_transmit_power = pInfo->reduce_transmit_power;
    max_allowed_diff     = pInfo->max_allowed_diff;
    transmit_from_CME    = pInfo->transmit_from_CME;
    V34bis               = pInfo->V34bis;

    /*************** End of Copying Phase 2 data ***************/

    TRACE1("pPH2->RTD_Symbol = %d", pPH2->RTD_Symbol);
    TRACE2("AnsData.power_reduction = %d, CallData.power_reduction = %d", AnsData.power_reduction, CallData.power_reduction);

#if (!USE_ANS)

    if (modem_mode == ANS_MODEM)
    {
        Disconnect_Init(pTable);
        TRACE0("Ans modem not supported");
        return;
    }

#endif

    /* This will clear all data in Phase2 because of */
    /* data overlay between Phase2 and V.34.         */
    memset(pV34, 0, sizeof(V34Struct));

    pV34->pTable = pTable;

#if SUPPORT_V34FAX
    pRx->V34fax_Enable = 0;
#endif

    if (modem_mode == CALL_MODEM)
    {
        pRemoteData = &AnsData;
        pLocalData  = &CallData;
    }
    else
    {
        pRemoteData = &CallData;
        pLocalData  = &AnsData;
    }

    pV34Rx->rx_high_freq   = pLocalData->high_carrier_freq;
    pV34Rx->rx_bit_rate    = pLocalData->bit_rate;
    pV34Rx->rx_symbol_rate = pLocalData->symbol_rate;

    pQAM->power_reduction  = pRemoteData->power_reduction;
    pQAM->tx_preemp_idx    = pRemoteData->preemp_idx;
    pQAM->tx_high_freq     = pRemoteData->high_carrier_freq;
    pV34Rx->tx_bit_rate    = pRemoteData->bit_rate;
    pV34Rx->tx_symbol_rate = pRemoteData->symbol_rate;
    pV34Rx->md_length      = pRemoteData->md_length;

    pV34Rx->RTD_PH2_symbol = RTD_PH2_symbol;

    pV34State->hang_up     = 0;

    pV34State->Retrain_Times = Retrain_Times;

    pV34Rx->S2743                = S2743;
    pV34Rx->S2800                = S2800;
    pV34Rx->S3429                = S3429;
    pV34Rx->S3000_low_carrier    = S3000_low_carrier;
    pV34Rx->S3000_high_carrier   = S3000_high_carrier;
    pV34Rx->S3200_low_carrier    = S3200_low_carrier;
    pV34Rx->S3200_high_carrier   = S3200_high_carrier;
    pV34Rx->S3429_enable         = S3429_enable;
    pV34Rx->reduce_transmit_power = reduce_transmit_power;
    pV34Rx->max_allowed_diff     = max_allowed_diff;
    pV34Rx->transmit_from_CME    = transmit_from_CME;
    pV34Rx->V34bis               = V34bis;
    pV34Rx->V34bisL              = V34bisL;
    pV34Rx->modem_mode           = modem_mode;

    pAGC->AGC_gain = AGC_gain;

    pV34Rx->qNLDFactor           = qNLDFactor;
    pTCR->drift_count      = 0;

    if (drift < -3860)
    {
        pTCR->drift_direction = 1;
        thresh = (UDWORD)115807148L / (-drift);
    }
    else if (drift > 3860)
    {
        pTCR->drift_direction = -1;
        thresh = (UDWORD)115807148L / drift;
    }
    else
    {
        pTCR->drift_direction = 0;
        thresh = (UDWORD)30000;
    }

#if FREQUENCY_OFFSET
    drift = QDQMULQD(drift, 3018);
    drift >>= 12;

    if (thresh < 50)/* Frequency offset */
    {
        pV34Rx->frequencyOffset = (QWORD)(QDQMULQD(drift, 8676));  /* Q8 offset in Hz */

        if (thresh <= 5)   /* > 0.5Hz */
        {
            for (i = 0; i < V34_ECHO_COEF_LEN; i++)
            {
                ec->pcFe_coef[i].r = 0;
                ec->pcFe_coef[i].i = 0;
            }
        }
    }
    else
    {
        pV34Rx->frequencyOffset = (QWORD)(QDQMULQD(drift, 8000));   /* Q8 offset in Hz */
    }

#endif

    /*
    IMPORTANT NOTES:
    The timing drift_thresh calculation above is not correct for 3429 symbol rate on Hawk borad
    with ADI codec, which samples data at 10286Hz instead of 10287, introducing new clock
    difference. For other codec, no such problem.
    */

    pTCR->drift_thresh = (UWORD)thresh;

    TRACE2("drift_direction=%d; drift_thresh=%d", pTCR->drift_direction, pTCR->drift_thresh);

    /** Set modem modulation select register to V.34 **/
    PutReg(pDpcs->MMSR0, MODEM_V34);

    PutReg(pDpcs->MMTR, V34_TX_MAX_THROUGHPUT);

    PutReg(pDpcs->MSR0, TRAIN);

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

    V34_Init(pV34);

    pMhsp->pfRealTime = V34_Phase3_Handler;

    V34_HW_Setup(pTable);

    TRACE0("Phase3 Init successful");
}

void  V34_Init(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    V34AgcStruc   *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    VA_INFO        *dec = &(DeCode->Vdec);    /* VA INFO structure */
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    QAMStruct     *pQAM = &(pV34Tx->QAM);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EqStruc        *pEq = &(pV34Rx->Eq);
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    QWORD qTemp;

    V34_Carrier_Init(pV34);

    pV34->Time_cnt = 0;   /* init time out counter for phase3 and 4 */

#if 1
    pV34Rx->Num_Trn_Rx     = V34_4_POINT_TRN;/* Number points 4 points training */
    pV34Rx->Num_Trn_Tx     = V34_4_POINT_TRN;
    pV34Rx->J_point        = V34_4_POINT_J;  /* J pattern indicates 4 points training */
    pV34Rx->rx_symbol_bits = V34_RX_SYM_BIT_4POINT;
#else
    pV34Rx->Num_Trn_Rx     = V34_16_POINT_TRN;/* Number points 16 points training */
    pV34Rx->Num_Trn_Tx     = V34_16_POINT_TRN;
    pV34Rx->J_point        = V34_16_POINT_J;  /* J pattern indicates 16 points training */
    pV34Rx->rx_symbol_bits = V34_RX_SYM_BIT_16POINT;
#endif

    Phase3_Init(pV34);
    Phase4_Init(pV34);

    pV34Tx->z = 0;
    pV34Tx->Y0 = 0;

    /* -----------------   Initial data for Scrambler   ------------------ */
#if 0
    pV34->sc              = 0;
    pRx->dsc              = 0;
    pV34->train_sc        = 0;
    pShellMap->scram_idx  = 0;
    pV34->train_scram_idx = 0;
#endif

#if USE_ANS

    if (pV34Rx->modem_mode == CALL_MODEM)
#endif
    {
        pV34Rx->pfScram_byte    = V34_Scram_Byte_GPC;
        pV34Rx->pfScram_bit     = V34_Scram_Bit_GPC;
        pV34Rx->pfScramTrn_byte = V34_Scram_Byte_GPA;
        pV34Rx->pfScramTrn_bit  = V34_Scram_Bit_GPA;

        pRx->pfDescram_byte     = V34_Descram_Byte_GPC;
        pRx->pfDescram_bit      = V34_Descram_Bit_GPC;
    }

#if USE_ANS
    else
    {
        pV34Rx->pfScram_byte    = V34_Scram_Byte_GPA;
        pV34Rx->pfScram_bit     = V34_Scram_Bit_GPA;
        pV34Rx->pfScramTrn_byte = V34_Scram_Byte_GPC;
        pV34Rx->pfScramTrn_bit  = V34_Scram_Bit_GPC;

        pRx->pfDescram_byte     = V34_Descram_Byte_GPA;
        pRx->pfDescram_bit      = V34_Descram_Bit_GPA;
    }

#endif

    /* -----------------   Initial for transmit symbol buffer ------------ */
    pV34Tx->tx_sym_inidx  = 0;
    pV34Tx->tx_sym_outidx = 0;

    pRx->rx_sym_inidx  = 0;
    pRx->rx_sym_outidx = 0;
    pRx->rx_sym_count  = 0;

    /* -----------------   Initial for Convolution encoder state ---------- */
    pV34Tx->tx_COV_state = 0;

    V34_Agc_Init(pAGC);

    V34_Timing_Rec_Init(pTCR);

    V34_TimingLoop_Init(pTCR);

    V34_CarrierLoop_Init(pTCR);

#if PHASEJITTER
    V34_jt_loop_init(pJT, 1);
#endif
    V34_Modulate_Init(pV34->pTable, pQAM);
    V34_Demodulate_Init(&pRx->DeMod);

    /* V34_Equalizer_Init */
    DspcFir_2T3EQInit(&pTCR->eqfir, pEq->pcEqcoef, pEq->pcEqdline, V34_EQ_LENGTH);

#if RX_PRECODE_ENABLE
    V34_NoiseWhiten_Init(&(pV34->NoiseWhiten));
#endif

    /* V34_Equalizer_Init */
    DspcFirInit(&(pRx->S_ToneDet.EQ_S), pEq->pcEqScoef, pEq->pcEqSdline, V34_EQ_S_LEN);

    V34_Echo_Init(pV34);
    V34_VA_Init(dec);

    V34_Precoding_DataInit(&(pV34Tx->tx_precode));
    V34_Precoding_DataInit(&(pRx->rx_precode));
    V34_Precoding_DataInit(&(pRx->VA_precode));
    V34_Shell_Mapper_Init(pShellMap);
    V34_RXShell_Mapper_Init(&pRx->RxShellMap);

    pTCR->Tcount = 0;

    /* Initilize the tx scale so that an expected power level is achieved */
    /*     Note: based on the symbol rate, carrier freq, preemphasis filter */
    qTemp = V34_tPreempGain[pV34Rx->tx_symbol_rate][pQAM->tx_preemp_idx][pQAM->tx_high_freq];

    pQAM->qTxScale = QQMULQR15(qTemp, V34_tTxScaleDnFrom12dBm0[pQAM->power_reduction]);

    V34_CircFIR_Filter_Init(&(pQAM->tx_preemp), pQAM->pqTx_preemp_dline, (QDWORD *)(V34_tPREEMP_COEF[pQAM->tx_preemp_idx]), 7);

    pV34->Echo.Ec_Insert.max_bulk_len = V34_MAX_BULK_LEN;
    pV34->Echo.Ec_Insert.ne_sublen    = V34_EC_SUBLEN;
}

void  V34_Phase4_Init(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
    UBYTE  i;

    PutReg(pDpcs->MSR0, TRAIN);

    pV34->clear_down = 0;
    p4->TimeOut = V34PHASE4_TIMEOUT;

    pMhsp->pfRealTime = V34_Phase4_Handler;
    /* Create some entries of MpTx structure */

    pRx->qEq_Beta = V34_qBETA_TRN_PH4;

    if (pV34Rx->modem_mode == CALL_MODEM)
    {
        pTCR->qCarAcoef = q097;
        pTCR->qCarBcoef = q003;
        pTCR->qB3       = 192;
    }

#if 1
    p4->mp_size = V34_MP_1_SIZE;
    pMpTx->type = MP_TYPE_1;
#else
    p4->mp_size = V34_MP_0_SIZE;
    pMpTx->type = MP_TYPE_0;
#endif

    pMpTx->aux_channel_select = 0;
    pMpTx->TrellisEncodSelect = 0; /* 0 is 16-state */
    pMpTx->NonlinearEncodParam = NON_LINEAR;
    pMpTx->const_shap_select_bit = 1;
    pMpTx->acknowledge_bit = 0;
    pMpTx->asymmetric_data_sig_rate = 1;  /* 0 = symmetric bit rate */

    /* following will be moved to the end of equalizer */
    for (i = 0; i < 3; i++)
    {
        pMpTx->PrecodeCoeff_h[i].r = 0;
        pMpTx->PrecodeCoeff_h[i].i = 0;
    }

    /* -------move block end----------------------------- */
}
#endif

void  V34_RXShell_Mapper_Init(RX_ShellMapStruct *RxShellMap)
{
    memset(RxShellMap->pRx_g2, 0, sizeof(RxShellMap->pRx_g2));
    memset(RxShellMap->pRx_g4, 0, sizeof(RxShellMap->pRx_g4));
    memset(RxShellMap->pRx_g8, 0, sizeof(RxShellMap->pRx_g8));
    memset(RxShellMap->pRx_z8, 0, sizeof(RxShellMap->pRx_z8));
}

void  V34_S_Init(V34Struct *pV34)
{
    pV34->p3.S_point     = 0;
    pV34->p3.S_point_BAR = 2;

    pV34->p4.S_point     = 0;
    pV34->p4.S_point_BAR = 2;
}
