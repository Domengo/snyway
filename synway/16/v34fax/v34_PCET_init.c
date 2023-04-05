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
#include "hwapi.h"
#include "v34fext.h"
#include "mhsp.h"

#if SUPPORT_V34FAX

#define V34FAX_IMAGE_PAGES       1

void V34Fax_PCET_Init(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    SWORD *pPCMin  = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    Ph2Struct *pPH2 = (Ph2Struct *)(pTable[PH2_STRUC_IDX]);
    Info_hStruc *pIh = &(pPH2->Ih);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
	MphStruc *pMphRx = &pV34Fax->MphRx;
    Phase3_Info *p3 = &(pV34->p3);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    QAMStruct *pQAM = &(pV34Tx->QAM);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    EqStruc *pEq = &(pV34Rx->Eq);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    V34StateStruc *pV34State = &pRx->V34State;
    DeCodeStruc *DeCode = &pRx->DeCode;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    UBYTE  rate;
    QDWORD qdTemp;
    QWORD  qTemp;
    UBYTE  i;
    QDWORD drift, drift1, thresh;
    QDWORD qdNoise_Egy;
    SWORD  AGC_gain;
    UBYTE modem_mode;
    UBYTE EQ_Mode;
    UBYTE FreqOffset_Enable;

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

    UBYTE Retrain_Times;

    Info_0Struc *pInfo;
    UBYTE power_reduction;
    UBYTE trn_length;
    UBYTE high_carrier_freq;
    UBYTE preemp_idx;
    UBYTE symbol_rate;
    UBYTE TRN_constellation_point;

    modem_mode = pPH2->modem_mode;   /* 0: Answer, 1: Call */

    EQ_Mode = pPH2->V34fax_EQ_Mode;

    FreqOffset_Enable = pPH2->FreqOffset_Enable;

    Retrain_Times = pPH2->Ph2State.Retrain_Times;

    AGC_gain = pPH2->AGC_gain;

    drift  = pPH2->drift;
    drift1 = pPH2->drift1;

    qdNoise_Egy = pPH2->qdNoise_Egy;

    TRACE1("V34: pIh->power_reduction=%d", pIh->power_reduction);

    /* Copy informations of Phase2 INFOh */
    power_reduction         = pIh->power_reduction;
    trn_length              = pIh->trn_length;
    high_carrier_freq       = pIh->high_carrier_freq;
    //pIh->pre_em_index     = 0;
    preemp_idx              = pIh->pre_em_index;
    symbol_rate             = pIh->sym_rate_call_to_answer;
    TRN_constellation_point = pIh->TRN_constellation;

    if (modem_mode == CALL_MODEM)
    {
        pInfo   = &(pPH2->I0a);
        V34bisL = pPH2->I0c.V34bis;
    }
    else
    {
        pInfo   = &(pPH2->I0c);
        V34bisL = pPH2->I0a.V34bis;
    }

    S2743                 = pInfo->S2743;
    S2800                 = pInfo->S2800;
    S3429                 = pInfo->S3429;
    S3000_low_carrier     = pInfo->S3000_low_carrier;
    S3000_high_carrier    = pInfo->S3000_high_carrier;
    S3200_low_carrier     = pInfo->S3200_low_carrier;
    S3200_high_carrier    = pInfo->S3200_high_carrier;
    S3429_enable          = pInfo->S3429_enable;
    reduce_transmit_power = pInfo->reduce_transmit_power;
    max_allowed_diff      = pInfo->max_allowed_diff;
    transmit_from_CME     = pInfo->transmit_from_CME;
    V34bis                = pInfo->V34bis;

    /* This will clear all data in Phase2 */
    memset(pV34, 0, sizeof(V34Struct));

    pV34Fax->FreqOffset_Enable = FreqOffset_Enable;

    if (pV34Fax->FreqOffset_Enable == 1)
    {
        qdTemp = drift - drift1;

        if (qdTemp < -3860)
        {
            /* (30856 * (freq_tone_diff) * 2 * pi * 32768 ) / TIMING_INDEX_1SYMBOL  = 115807385 */
            /* TIMING_INDEX_1SYMBOL => interpolation points in 1 symbol == 384 */
            /* 30856   = [(1*2)+(2*3)+(3*4)+ ... +((ANGLE_N-1) * ANGLE_N]/2 */
            /* freq_tone = 7 for drift1, 15 for drift, difference is 8 */

            pV34Fax->drift_direction = 1;

            thresh = (UDWORD)132351294L / (-qdTemp);
        }
        else if (qdTemp > 3860)
        {
            pV34Fax->drift_direction = -1;

            thresh = (UDWORD)132351294L / (qdTemp);
        }
        else
        {
            pV34Fax->drift_direction = 0;

            thresh = (UDWORD)30000;
        }

        pV34Fax->drift_thresh = (UWORD)thresh;

#if FREQUENCY_OFFSET
        qdTemp = (qdTemp * 7) >> 3;
        qdTemp = drift1 - qdTemp;

        /* 6490 = 9600*256 / 30856*2*pi*64 */
        /* 9600 == sampling rate during phase 2 */
        /* 256 == (1<<8) for Q8 Format */
        /* 30856   = [(1*2)+(2*3)+(3*4)+ ... +((ANGLE_N-1) * ANGLE_N]/2 */
        /* 64  == Number of points in FFT during phase 2 */
        pV34Rx->frequencyOffset = (SWORD)(QDQMULQD((qdTemp >> 15), 6490)); /* 0FFSET in Q8 Hz */

        pV34Fax->frequencyOffset = pV34Rx->frequencyOffset;

        TRACE1("pV34Fax->frequencyOffset=%d", pV34Fax->frequencyOffset);
#endif
    }

    pV34->pTable = pTable;

    for (i = 0; i < PCM_BUFSIZE; i++) /* clear max size, not just V34FAX_BUF_LEN */
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

    pV34State->hang_up = FALSE;
    pRx->V34fax_Enable = 1;

    pV34Fax->pTable = pTable;

    pV34Fax->power_reduction = power_reduction;
    pV34Fax->trn_length = trn_length;
    pV34Fax->high_carrier_freq = high_carrier_freq;
    pV34Fax->preemp_idx = preemp_idx;

    pV34Fax->TRN_constellation_point = TRN_constellation_point;

    pV34Fax->Enter_CC_Order = 0;
    pV34Fax->tx_sym_cnt = 0;

    p3 = &(pV34->p3);

    pV34State->Retrain_Times = Retrain_Times;

    pV34Rx->S2743                 = S2743;
    pV34Rx->S2800                 = S2800;
    pV34Rx->S3429                 = S3429;
    pV34Rx->S3000_low_carrier     = S3000_low_carrier;
    pV34Rx->S3000_high_carrier    = S3000_high_carrier;
    pV34Rx->S3200_low_carrier     = S3200_low_carrier;
    pV34Rx->S3200_high_carrier    = S3200_high_carrier;
    pV34Rx->S3429_enable          = S3429_enable;
    pV34Rx->reduce_transmit_power = reduce_transmit_power;
    pV34Rx->max_allowed_diff      = max_allowed_diff;
    pV34Rx->transmit_from_CME     = transmit_from_CME;
    pV34Rx->V34bis                = V34bis;
    pV34Rx->V34bisL               = V34bisL;

    pV34Rx->modem_mode            = modem_mode;
    pV34Fax->modem_mode           = modem_mode;

    pV34Rx->tx_symbol_rate        = symbol_rate;
    pV34Fax->symbol_rate          = symbol_rate;

    pAGC->AGC_gain = AGC_gain;

    pTCR->Tcount = 0;

    p3->tx_vec_idx = 0;
    p3->rx_vec_idx = 0;

    p3->K = 0;
    p3->I = 0;

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        pV34Fax->PCET_TXend_Flag = 0;

        p3->symbol_counter = 0;

        for (i = 0; i < 5; i++)
        {
            p3->pfTx[i] = (V34FnPtr)V34Fax_PCET_Tx_Fsm[i];
        }

        pQAM->power_reduction = power_reduction;
        pQAM->tx_preemp_idx   = preemp_idx;
        pQAM->tx_high_freq    = pV34Fax->high_carrier_freq;

        pV34Rx->md_length = 0;

        pV34Rx->pfScram_byte   = V34_Scram_Byte_GPC;
        pV34Rx->pfScram_bit    = V34_Scram_Bit_GPC;
        pV34Rx->pfScramTrn_byte = V34_Scram_Byte_GPA;
        pV34Rx->pfScramTrn_bit = V34_Scram_Bit_GPA;
        pRx->pfDescram_byte    = V34_Descram_Byte_GPC;
        pRx->pfDescram_bit     = V34_Descram_Bit_GPC;

        pQAM->tx_carrier_freq    = V34_tCarrier_tbl[symbol_rate][pV34Fax->high_carrier_freq];
        pQAM->qTx_carrier_offset = V34_tCarrier_offset[symbol_rate][pV34Fax->high_carrier_freq];

        V34_S_Init(pV34);

        pV34Tx->z = 0;
        pV34Tx->Y0 = 0;

        pV34Tx->tx_sym_inidx  = 0;
        pV34Tx->tx_sym_outidx = 0;
        pV34Tx->tx_COV_state  = 0;

        V34_Modulate_Init(pV34->pTable, pQAM);

#if TX_PRECODE_ENABLE
        V34_Precoding_DataInit(&(pV34Tx->tx_precode));
#endif

        /* Initilize the tx scale so that an expected power level is achieved */
        /* Note: based on the symbol rate, carrier freq, preemphasis filter   */
        qTemp = V34_tPreempGain[symbol_rate][preemp_idx][pV34Fax->high_carrier_freq];

        pQAM->qTxScale = QQMULQR15(qTemp, V34_tTxScaleDnFrom12dBm0[pQAM->power_reduction]);

        V34_CircFIR_Filter_Init(&(pQAM->tx_preemp), pQAM->pqTx_preemp_dline, (QDWORD *)(V34_tPREEMP_COEF[preemp_idx]), 7);
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            p3->pfRx[i] = (V34FnPtr)V34Fax_PCET_Rx_Fsm[i];
        }

        pV34Rx->rx_high_freq = pV34Fax->high_carrier_freq;

        pV34Rx->rx_symbol_rate = symbol_rate;

        pV34Rx->pfScram_byte    = V34_Scram_Byte_GPA;
        pV34Rx->pfScram_bit     = V34_Scram_Bit_GPA;
        pV34Rx->pfScramTrn_byte = V34_Scram_Byte_GPC;
        pV34Rx->pfScramTrn_bit  = V34_Scram_Bit_GPC;

        pRx->pfDescram_byte     = V34_Descram_Byte_GPA;
        pRx->pfDescram_bit      = V34_Descram_Bit_GPA;

        pV34Fax->EQ_Mode = EQ_Mode;

        if (EQ_Mode == 0)
        {
            pV34Fax->V34fax_EQ_Length = V34FAX_EQ_LENGTH_27;
            pV34Fax->V34FAX_EQ_Delay  = V34FAX_EQ_DELAY_9;
        }
        else
        {
            pV34Fax->V34fax_EQ_Length = V34FAX_EQ_LENGTH_78;
            pV34Fax->V34FAX_EQ_Delay  = V34FAX_EQ_DELAY_36;
        }

        if (pV34Fax->FreqOffset_Enable == 0)
        {
            pRx->rx_carrier_freq    = V34_tCarrier_tbl[symbol_rate][pV34Fax->high_carrier_freq];
            pRx->qRx_carrier_offset = V34_tCarrier_offset[symbol_rate][pV34Fax->high_carrier_freq];
        }
        else
        {
            V34_Carrier_Init(pV34);
        }

        if (pV34Fax->TRN_constellation_point == 0)
        {
            pV34Rx->Num_Trn_Rx     = V34_4_POINT_TRN;/* Number points 4 points training */
            pV34Rx->Num_Trn_Tx     = V34_4_POINT_TRN;
            pV34Rx->J_point        = V34_4_POINT_J;  /* J pattern indicates 4 points training */
            pV34Rx->rx_symbol_bits = V34_RX_SYM_BIT_4POINT;
        }
        else
        {
            pV34Rx->Num_Trn_Rx     = V34_16_POINT_TRN;
            pV34Rx->Num_Trn_Tx     = V34_16_POINT_TRN;
            pV34Rx->J_point        = V34_16_POINT_J;
            pV34Rx->rx_symbol_bits = V34_RX_SYM_BIT_16POINT;
        }

        pV34Fax->pTCR = &pRx->TCR;

        pRx->rx_sym_inidx  = 0;
        pRx->rx_sym_outidx = 0;
        pRx->rx_sym_count  = 0;

        V34_VA_Init(&DeCode->Vdec);

#if RX_PRECODE_ENABLE
        V34_Precoding_DataInit(&(pRx->rx_precode));
        V34_Precoding_DataInit(&(pRx->VA_precode));
#endif
        V34_RXShell_Mapper_Init(&pRx->RxShellMap);

        V34_Agc_Init(&pRx->rx_AGC);

        V34_S_Detector_Init(&pRx->S_ToneDet);

        V34_Demodulate_Init(&pRx->DeMod);

        V34_CarrierLoop_Init(pTCR);

        V34Fax_Timing_Rec_Init(pV34Fax);
        V34Fax_TimingLoop_Init(pV34Fax);

#if PHASEJITTER
        V34_jt_loop_init(&pRx->jtvar, 1);
#endif

#if RX_PRECODE_ENABLE
        V34_NoiseWhiten_Init(&(pV34->NoiseWhiten));
#endif

        DspcFirInit(&(pRx->S_ToneDet.EQ_S), pEq->pcEqScoef, pEq->pcEqSdline, V34_EQ_S_LEN);
        DspcFir_2T3EQInit(&pTCR->eqfir, pEq->pcEqcoef, pEq->pcEqdline, pV34Fax->V34fax_EQ_Length);

        pV34Fax->qdNoise_Egy = qdNoise_Egy;
    }

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

    /* RTD factor convert ms to per interupt handler call (24 samples per call) */
    pV34->qRTD_factor = V34_tRTD_FACTOR[symbol_rate];

    qTemp  = pV34Rx->RTD_PH2_symbol + V34_PHASE2_SYS_DELAY;
    qdTemp = QQMULQD(qTemp, q5_OVER_6);

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        qTemp  = (QWORD)(qdTemp >> 13) + 2800;
        p3->pqTime_out_value[0] = QQMULQ15(pV34->qRTD_factor, qTemp);

        qTemp  = (QWORD)(qdTemp >> 14) + pV34Rx->md_length * 35;
        p3->pqTime_out_value[1] = QQMULQ15(pV34->qRTD_factor, qTemp) + 40;

        /* for call modem to detect S tone */
    }
    else
    {
        /* time out value for detect s-to S bar transition */
        qTemp  = (QWORD)(qdTemp >> 14);
        qTemp += pV34Rx->md_length * 35 + 600;

        p3->pqTime_out_value[0] = QQMULQ15(pV34->qRTD_factor, qTemp) + 200;

        /* time out value for J sequence */
        qTemp  = (QWORD)(qdTemp >> 13) + 2600;

        p3->pqTime_out_value[1]  = QQMULQ15(pV34->qRTD_factor, qTemp) + 200;

        /* time out value for detect S tone */
        qTemp  = (QWORD)(qdTemp >> 13) + 3800;

        p3->pqTime_out_value[2]  = QQMULQ15(pV34->qRTD_factor, qTemp);
    }

    pMhsp->pfRealTime = V34Fax_PCET_Handler;
    V34Fax_HW_Setup(pTable);

    pV34Fax->timeout_count = 0; TRACE0("V34: Timer init");

    pMphRx->pre_max_data_rate = V34_BIT_33600;
}


void V34Fax_HW_Setup(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    UBYTE *pHwApi = (UBYTE *)(pTable[HWAPIDATA_IDX]);
    SWORD *pPCMin = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    UBYTE i;

    for (i = 0; i < pMhsp->RtBufSize; i++)
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

    pMhsp->RtCallRate = 1;

    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;

    switch (pV34Fax->symbol_rate)
    {
        case V34_SYM_2400:
            pHwApi[1] = HWAPI_7200SR;/* 10ms <-> 72 */

            pMhsp->RtBufSize = 72;
            pV34->NumSymbol  = 24;
            break;

        case V34_SYM_2743:
            pHwApi[1] = HWAPI_8229SR;/* 10.2ms <-> 84 */

#if SAMCONV
            pMhsp->RtBufSize = 72;
            pV34->NumSymbol  = 24;
#else
            pMhsp->RtBufSize = 84;
            pV34->NumSymbol  = 28;
#endif
            break;

        case V34_SYM_2800:
            pHwApi[1] = HWAPI_8400SR;/* 10ms <-> 84 */

            pMhsp->RtBufSize = 84;
            pV34->NumSymbol  = 28;
            break;

        case V34_SYM_3000:
            pHwApi[1] = HWAPI_9000SR;/* 10ms <-> 90 */

            pMhsp->RtBufSize = 90;
            pV34->NumSymbol  = 30;
            break;

        case V34_SYM_3200:
            pHwApi[1] = HWAPI_9600SR;/* 10ms <-> 96 */

            pMhsp->RtBufSize = 96;
            pV34->NumSymbol  = 32;
            break;

        case V34_SYM_3429:
            pHwApi[1] = HWAPI_10287SR;/* 9.3ms <-> 96 */
#if SAMCONV
            pMhsp->RtBufSize = 90;
            pV34->NumSymbol  = 30;
#else
            pMhsp->RtBufSize = 96;
            pV34->NumSymbol  = 32;
#endif
            break;
    }

    pHwApi[2] = pMhsp->RtCallRate * pMhsp->RtBufSize;
}

#endif
