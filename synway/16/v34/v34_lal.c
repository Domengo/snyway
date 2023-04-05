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

/*********************************************************************
  This file implemented V.34 modem local analog loopback

    V34_LAL_Startup(UBYTE **pTable)  called by background to start LAL
    V34_LAL_Handler(UBYTE **pTable) handle HW interrupt
    V34_Digital_Loop_Back_Init(UBYTE **pTable) initiate LAL and LDL

      Following RX vector only for LAL,
      V34LAL_Det_S(V34Struct * pV34)
      V34LAL_Det_S_Bar(V34Struct * pV34)
      V34LAL_Look_For_E(V34Struct * pV34)
********************************************************************/

#include <string.h>
#include "v34ext.h"
#include "mhsp.h"   /* Modem Host Signal Processor */
#include "ptable.h" /* Data memory table defines   */
#include "hwapi.h"  /* Hardware Defines            */
#include "gaoapi.h"

#if SUPPORT_V54

CONST V34FnPtr V34LalTxFsm[] =
{
    V34_Phase4_Send_S,                /* Tx: State 0 */
    V34_Phase4_Send_S_Bar,            /* Tx: State 1 */
    V34_Phase4_Send_TRN,              /* Tx: State 2 */
    V34_Phase4_Send_TRN_2000MS,       /* Tx: State 3 */
    V34_Phase4_Send_MP,               /* Tx: State 4 */
    V34_Phase4_Finish_MP,             /* Tx: State 5 */
    V34_Phase4_Send_MP,               /* Tx: State 6 */
    V34_Phase4_Finish_MP_Pi,          /* Tx: State 7 */
    V34_Phase4_Flush_MP_Pi,           /* Tx: State 8 */
    V34_Phase4_Prepare_E,             /* Tx: State 9 */
    V34_Phase4_Send_E,                /* Tx: State 10 */
    V34_Phase4_Send_B1_Frame,         /* Tx: State 11 */
    V34_Transmiter                    /* Tx: State 12 */
};

CONST V34FnPtr V34LalRxFsm[] =
{
    V34_LAL_Det_S,                 /* Rx: State 0 */
    V34_LAL_Det_S_Bar,             /* Rx: State 1 */
    V34_Phase4_Wait_S_Bar_16T,     /* Rx: State 2 */
    V34_Phase4_EQ_1,               /* Rx: State 3 */
    V34_Phase4_Det_TRN,            /* Rx: State 4 */
    V34_Phase4_Det_TRN_TC,         /* Rx: State 5 */
    V34_Phase4_Rec_MP,             /* Rx: State 6 */
    V34_Phase4_Rec_MP_Pi,          /* Rx: State 7 */
    V34_LAL_Look_For_E,            /* Rx: State 8 */
    V34_Phase4_VA_Delay,           /* Rx: State 9 */
    V34_Phase4_Rec_B1_Frame        /* Rx: State 10 */
};

CONST V34FnPtr V34LalEcFsm[] =
{
    V34_Phase3_Ec_Dummy,           /* Ec: State 0 */
    V34_Phase3_Ec_Dummy            /* Ec: State 1 */
};

void V34_LAL_Codec_Setup(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    UBYTE *pHwApi = (UBYTE *)(pTable[HWAPIDATA_IDX]);

    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;

    switch (pV34Rx->tx_symbol_rate)
    {
        case V34_SYM_2400:
            pHwApi[1] = HWAPI_7200SR;/* 10ms <-> 72 */
            pHwApi[2] = 72;
            pMhsp->RtBufSize = 72;
            pV34->NumSymbol = 24;
            break;

        case V34_SYM_2743:
            pHwApi[1] = HWAPI_8229SR;/* 10.2ms <-> 84 */
            pHwApi[2] = 84;
            pMhsp->RtBufSize  = 84;
            pV34->NumSymbol = 28;
            break;

        case V34_SYM_2800:
            pHwApi[1] = HWAPI_8400SR;/* 10ms <-> 84 */
            pHwApi[2] = 84;
            pMhsp->RtBufSize  = 84;
            pV34->NumSymbol = 28;
            break;

        case V34_SYM_3000:
            pHwApi[1] = HWAPI_9000SR;/* 10ms <-> 90 */
            pHwApi[2] = 90;
            pMhsp->RtBufSize  = 90;
            pV34->NumSymbol = 30;
            break;

        case V34_SYM_3200:
            pHwApi[1] = HWAPI_9600SR;/* 10ms <-> 96 */
            pHwApi[2] = 96;
            pMhsp->RtBufSize  = 96;
            pV34->NumSymbol = 32;
            break;

        case V34_SYM_3429:
            pHwApi[1] = HWAPI_10287SR;/* 9.3ms <-> 96 */
            pHwApi[2] = 96;
            pMhsp->RtBufSize  = 96;
            pV34->NumSymbol = 32;
            break;
    }

    pMhsp->RtCallRate = 1;
}


/* V.34 modem LAL train start up */
void V34_LAL_Startup(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);

    V34_Digital_Loop_Back_Init(pTable);

    /* HW init */
    /* Codec Setup */
    V34_LAL_Codec_Setup(pTable);

    /* Setup MHSP pointer */
    pMhsp->pfRealTime = V34_LAL_Handler;
}

/*******************/
/* LAL modem Handler */
/*******************/
void V34_LAL_Handler(UBYTE **pTable)
{
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    SWORD *pPCMin  = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    Phase4_Info *p4 = &(pV34->p4);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &(pRx->TCR);
    UBYTE Num_Symbol, i;
#if WITH_DC
    DCStruct *pDC = (DCStruct *)pTable[DC_STRUC_IDX];
#endif
#if DRAWEPG
    SWORD *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
#endif

    pV34Tx->PCMinPtr  = pPCMin;
    pV34Tx->PCMoutPtr = pPCMout;

#if WITH_DC
    /* Low pass DC estimator filter */
    DcEstimator(pDC, pPCMin, UBUBMULUB(pV34->NumSymbol, V34_SYM_SIZE), -11);
#endif

    V34_DPCS_Data(pTable);

    Num_Symbol = pV34->NumSymbol;
#if DRAWEPG
    *pEpg++ = Num_Symbol;
#endif

    for (i = 0; i < Num_Symbol; i++)
    {
        if (p4->Phase4_TxEnd == 1)
        {
            V34_Transmiter(pV34);
        }
        else
        {
            p4->pfTx[p4->tx_vec_idx](pV34);
        }

        p4->pfEc[p4->ec_vec_idx](pV34);

        pTCR->Tcount++;

        while (pTCR->Tcount > 0)
        {
            if (p4->Phase4_RxEnd == 1)
            {
                V34_Receiver(pRx, pTable, 1);
            }
            else
            {
                p4->pfRx[p4->rx_vec_idx](pV34);
            }

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

    if ((p4->Phase4_TxEnd == 1) && (p4->Phase4_RxEnd == 1) && (pDpcs->MSR0 != DISC_DET))
    {
        PutReg(pDpcs->MSR0, DATA);
    }

    /* Phase4 Timeout control */
    if ((p4->tx_vec_idx == 12) && (p4->rx_vec_idx == 11))
    {
        pV34->LAL_modem = 2;
    }
    else
    {
        p4->TimeOut --;
    }
}

/* SC_Digital_Loop_Back_Init()*****************/
void V34_Digital_Loop_Back_Init(UBYTE **pTable)
{
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    VA_INFO        *dec = &(DeCode->Vdec);    /* VA INFO structure */
    V34StateStruc *pV34State = &pRx->V34State;
    RetrainStruc *pRT = &(pRx->Rtrn);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    EqStruc *pEq = &(pV34Rx->Eq);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    QAMStruct *pQAM = &(pV34Tx->QAM);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
    Phase3_Info *p3 = &(pV34->p3);
    UBYTE sym_rate;
    UBYTE  i, rate;
    QWORD  itemp;
    QWORD  qTemp;

    memset(pV34, 0, sizeof(V34Struct));
    pV34->pTable = pTable;

    pV34->LAL_modem = 1;
    pV34Tx->Min_Exp = 1;                /* Tx maximum shaping */
    pV34Rx->rx_me   = 1;                /* Rx maximum shaping */

    /** Set modem modulation select register to V.34 **/
    PutReg(pDpcs->MMSR0, MODEM_V34);

    PutReg(pDpcs->MSR0, TRAIN);

    /* get maximum symbol rate from host */
    pV34Rx->tx_symbol_rate = RdReg(pDpcs->MBR, MAX_BAUD_RATE);
    pV34Rx->rx_symbol_rate = pV34Rx->tx_symbol_rate;

    pV34Rx->tx_bit_rate = V34bis_tBit_Rate[pV34Rx->tx_symbol_rate];
    pV34Rx->rx_bit_rate = pV34Rx->tx_bit_rate;

    /* get data from DPCS */
    rate = RdReg(pDpcs->MBC1, MAX_RATE);

    if ((rate <= 4) || (rate > 17))
    {
        pV34Rx->Host_maxbitrate = V34_BIT_33600;
    }
    else
    {
        pV34Rx->Host_maxbitrate = rate - 3;    /* Convert Value to Bitrate */
    }

    TRACE1("Host max bitrate:%d", pV34Rx->Host_maxbitrate);

    if (pV34Rx->Host_maxbitrate < pV34Rx->tx_bit_rate)
    {
        pV34Rx->tx_bit_rate = pV34Rx->Host_maxbitrate;
    }

    if (pV34Rx->Host_maxbitrate < pV34Rx->rx_bit_rate)
    {
        pV34Rx->rx_bit_rate = pV34Rx->Host_maxbitrate;
    }

    /* get maximum freq support from host */
    rate = RdReg(pDpcs->MBR, MAX_CAR_FREQ);
    pQAM->tx_high_freq = rate >> 6;
    pV34Rx->rx_high_freq = pQAM->tx_high_freq;
    pV34Rx->modem_mode    = CALL_MODEM;
    pV34Tx->Aux.Aux_Flag = 0;
    pV34State->hang_up = 0;

    pV34Rx->Num_Trn_Rx     = V34_4_POINT_TRN;/* Number points 4 or 16 points training */
    pV34Rx->Num_Trn_Tx     = V34_4_POINT_TRN;
    pV34Rx->J_point        = V34_4_POINT_J;  /* J pattern indicates 4 points training */
    pV34Rx->rx_symbol_bits = V34_RX_SYM_BIT_4POINT;

    pV34Tx->z = 0;

    V34_S_Init(pV34);
    pRx->S_trn_point = 0;

    p4->tx_vec_idx = 0;
    p4->rx_vec_idx = 0;
    p4->ec_vec_idx = 0;

    p4->result_code           = 0;
    p4->detect_symbol_counter = 0;
    p4->send_symbol_counter   = 0;

    p3->detect_symbol_counter = 0;
    p3->symbol_counter        = 0;

    V34_S_Detector_Init(&pRx->S_ToneDet);

    sym_rate = pV34Rx->rx_symbol_rate;

    pRT->pqCoef[V34_RTN_TONE] = V34_tRetrain_Tone_Table[sym_rate][pV34Rx->modem_mode].qTone2;
    pRT->pqCoef[V34_RTN_REF]  = V34_tRetrain_Tone_Table[sym_rate][pV34Rx->modem_mode].qTone1;
    pRT->Tone_50ms    = 8;  /* 50ms retrain tone */
    pRT->ToneDet_Flag = 0;
    pRT->tone_count   = 0;

    V34_DFT_Init(&pRT->tonedet, pRT->pqQ1, pRT->pqQ2, pRT->pqCoef, pRT->pqMag_sqr, 48, 2);

    itemp = (pV34Rx->RTD_PH2_symbol + (18 << 1)) + 600;
    p4->pqTime_out_value[0] = QQMULQ15(pV34->qRTD_factor, itemp);

    itemp = ((pV34Rx->RTD_PH2_symbol + (18 << 1)) << 1) + 2500;
    p4->pqTime_out_value[1] = QQMULQ15(pV34->qRTD_factor, itemp);

    /* Clear out the send buffer */

    for (i = 0; i < V34_MP_1_SIZE; i++)
    {
        (p4->pMp_buf)[i] = 0;
    }

    /* Reset the long word that used to store the E */
    p4->curr_word = 0;

    /* Initialize the vector for transmit */
    for (i = 0; i < 13; i++)
    {
        p4->pfTx[i] = (V34FnPtr)V34LalTxFsm[i];
    }

    /* Initialize the vector for receive */
    for (i = 0; i < 11; i++)
    {
        p4->pfRx[i] = (V34FnPtr)V34LalRxFsm[i];
    }

    /* Initialize the vector for Ec */
    for (i = 0; i < 2; i++)
    {
        p4->pfEc[i] = (V34FnPtr)V34LalEcFsm[i];
    }

    /* -----------------   Initial data for Scrambler   ------------------ */
    pV34Rx->sc = 0;

    pRx->dsc = 0; /* V34_Descrambler_Init */

    pV34Rx->train_sc = 0;

    pV34Rx->pfScram_byte = V34_Scram_Byte_GPC;
    pV34Rx->pfScram_bit  = V34_Scram_Bit_GPC;
    pV34Rx->pfScramTrn_byte = V34_Scram_Byte_GPC;
    pV34Rx->pfScramTrn_bit = V34_Scram_Bit_GPC;
    pRx->pfDescram_byte = V34_Descram_Byte_GPA;
    pRx->pfDescram_bit  = V34_Descram_Bit_GPA;

    pShellMap->scram_idx = 0;
    pV34Rx->train_scram_idx = 0;

    /* -----------------   Initial Transmit filter ----------------------- */
    pRx->DeMod.rx_carrier_idx = 0;
    V34_Carrier_Init(pV34);

    /* -----------------   Initial for transmit symbol buffer ------------ */
    pV34Tx->tx_sym_inidx  = 0;
    pV34Tx->tx_sym_outidx = 0;
    pRx->rx_sym_inidx = 0;
    pRx->rx_sym_outidx = 0;
    pRx->rx_sym_count = 0;

    /* -----------------   Initial for Convolution encoder state ---------- */
    pV34Tx->tx_COV_state = 0;

    V34_Agc_Init(&pRx->rx_AGC);
    pRx->rx_AGC.qGain  = 127;  /* for LAL only */
    V34_Timing_Rec_Init(pTCR);
    V34_TimingLoop_Init(pTCR);
    V34_CarrierLoop_Init(pTCR);
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

    V34_Precoding_DataInit(&(pV34Tx->tx_precode));
    V34_Precoding_DataInit(&(pRx->rx_precode));
    V34_Precoding_DataInit(&(pRx->VA_precode));
    V34_Shell_Mapper_Init(pShellMap);
    V34_RXShell_Mapper_Init(&pRx->RxShellMap);
    V34_VA_Init(dec);

    /* -----------------   Initilize preemphasis filter ------------------- */
    pQAM->tx_preemp_idx = 0;
    qTemp = V34_tPreempGain[pV34Rx->tx_symbol_rate][pQAM->tx_preemp_idx][pQAM->tx_high_freq];

    pQAM->qTxScale = QQMULQ15(qTemp, V34_tTxScaleDnFrom12dBm0[6]);

    V34_CircFIR_Filter_Init(&(pQAM->tx_preemp), pQAM->pqTx_preemp_dline, (QDWORD *)(V34_tPREEMP_COEF[pQAM->tx_preemp_idx]), 7);
    V34_Parameter_Function_Init_Tx(pV34);
    V34_Parameter_Function_Init_Rx(pV34);

    if (pShellMap->M > 0)
    {
        V34_Calc_g2(pShellMap->M, pShellMap->pG2);
        V34_Calc_g4(pShellMap->M, pShellMap->pG2, pShellMap->pG4);
    }
    else
    {
        TRACE0("M=0");
    }

    /*******************************************************************/

    /* ------------------------------------------------------------ */
    /* initialize those information which was supposed to be the  */
    /* information receive from MP                                */
    pV34Tx->conv_state = 0; /* 16 state */

    /* Create some entries of MpTx structure */
    p4->mp_size = V34_MP_1_SIZE;

    pMpTx->max_call_to_ans_rate = pV34Rx->tx_bit_rate;
    pMpTx->max_ans_to_call_rate = pV34Rx->rx_bit_rate;

    pMpTx->type = MP_TYPE_1;

    pMpTx->aux_channel_select    = 0;
    pMpTx->TrellisEncodSelect    = 0; /* 0 is 16-state */
    pMpTx->NonlinearEncodParam   = 0;
    pMpTx->const_shap_select_bit = 1;
    pMpTx->acknowledge_bit       = 0;

    if (pV34Rx->tx_bit_rate >= pV34Rx->rx_bit_rate)
    {
        pMpTx->data_rate_cap_mask = ((UWORD)1 << pV34Rx->tx_bit_rate) - 1;
    }
    else
    {
        pMpTx->data_rate_cap_mask = ((UWORD)1 << pV34Rx->rx_bit_rate) - 1;
    }

    pMpTx->asymmetric_data_sig_rate = 0;

    /* following will be moved to the end of equalizer */
    for (i = 0; i < 3; i++)
    {
        pMpTx->PrecodeCoeff_h[i].r = 0;
        pMpTx->PrecodeCoeff_h[i].i = 0;
    }

    V34_Create_Mp(p4->pMp_buf, pMpTx);

    pMpTx->acknowledge_bit = 1;
    V34_Create_Mp(p4->pMp_buf_pi, pMpTx);

    pTCR->drift_thresh    = 32767;
    pTCR->drift_direction = 1;

    /* init time out */
    pV34->Time_cnt = 0;

    /* Phase4 Timeout control */
    p4->TimeOut = V34PHASE4_TIMEOUT;

    pV34->Echo.Ec_Insert.max_bulk_len = V34_MAX_BULK_LEN;
    pV34->Echo.Ec_Insert.ne_sublen    = V34_EC_SUBLEN;
}

void V34_LAL_Det_S(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD pcDsym[3];
    UBYTE  Det_S;
    UBYTE  i;

    Det_S = 0;

    V34_Demodulate(pRx, pcDsym);
    V34_Timing_Rec(pTCR, pcDsym, pV34Rx->tTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    Det_S = V34_S_Detector(pRx, pV34Rx->tTimingOut);

    if (Det_S)
    {
        p4->detect_symbol_counter = 0;
        pRx->S_trn_point         = 0;

        /** Add on Jan 15,1997, Benjamin **/
        pRx->trn_count = 0;
        pRx->qTrn_avg  = 0;

        for (i = 0; i < 6; i++)
        {
            pRx->rx_AGC.qHLK[i] = S_HLK_TAB[i];
        }

        ++(p4->rx_vec_idx);
    }
}


void V34_LAL_Det_S_Bar(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD pcDsym[3];
    Phase4_Info *p4 = &(pV34->p4);

    V34_Demodulate(pRx, pcDsym);
    V34_Timing_Rec(pTCR, pcDsym, pV34Rx->tTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    V34_Det_S_bar_Sub(pRx);

    if (V34_Detect_S_SBAR(pRx, pV34Rx->tTimingOut, &(p4->detect_symbol_counter)))
    {
        /* Prepare next receive state */
        (p4->rx_vec_idx)++;

        /* Prepare to wait for 16 symbols for start of TRN */
        p4->detect_symbol_counter = 16;
    }
}

void V34_LAL_Look_For_E(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    VA_INFO        *dec = &(DeCode->Vdec);    /* VA INFO structure */
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase4_Info     *p4 = &(pV34->p4);
    CQWORD cqSymbol, cqTrain_sym, pcDsym[3];
    QDWORD qdTemp;
    UBYTE pBit_stream[4];
    UBYTE dbits, diffbits;
    UBYTE i;

    V34_Demodulate(pRx, pcDsym);
    V34_Timing_Rec(pTCR, pcDsym, pV34Rx->tTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if DRAWEPG
    /************* For Vxd Epg debug ********/
    pRx->pEpg[0].r = cqSymbol.r;
    pRx->pEpg[0].i = cqSymbol.i;
    /**************** End of pEpg ************/
#endif

    dbits = V34_Symbol_To_Bits(cqSymbol, &cqTrain_sym, pV34Rx->Num_Trn_Rx);

    V34_Shift_Buf(pTCR->pcBaud_adjust, 3, cqSymbol);
    pTCR->pcT3_delay[0] = pTCR->pcT3_delay[1];
    pTCR->pcT3_delay[1] = cqTrain_sym;

    V34_TimingLoop(pTCR, &cqSymbol, &cqTrain_sym);
    V34_CarrierLoop(pTCR, &cqSymbol, &cqTrain_sym);

    V34_DIFF_DECODE(DeCode, (dbits & 0x3), diffbits);

    pBit_stream[0] = diffbits & 0x1;
    pBit_stream[1] = (diffbits >> 1) & 0x1;
    pBit_stream[2] = (dbits >> 2) & 0x1;
    pBit_stream[3] = (dbits >> 3) & 0x1;

    (*pRx->pfDescram_bit)(pBit_stream, &(pRx->dsc), pRx->pDscram_buf, pV34Rx->rx_symbol_bits);

    for (i = 0; i < pV34Rx->rx_symbol_bits; i++)
    {
        p4->curr_word = (p4->curr_word << 1) & 0x000FFFFFL;
        p4->curr_word |= (pRx->pDscram_buf[0] & 0x1);
        pRx->pDscram_buf[0] >>= 1;

        if (p4->curr_word == 0x000fffffL)
        {
            break;
        }
    }

    if (p4->curr_word == 0x000fffffL)
    {
        TRACE0("LAL: E detected");

        /* goto receive B1 frame */
        (p4->rx_vec_idx)++;

        /* Initial Descrambler and Differential decoder */
        pRx->dsc = 0; /* V34_Descrambler_Init */
        DeCode->dZ = 0;

        if (RxMod->rx_M > 0)
        {
            V34_Calc_z8(RxMod->rx_M, pRx->RxShellMap.pRx_g8, pRx->RxShellMap. pRx_z8);
        }
        else
        {
            TRACE0("M=0");
        }

        /* Since last data frame will used the last two bits of */
        /* the bit inversion pattern, which is bit 1 and bit 0  */
        /* we setup the variable to be 1.                       */
        dec->current_rx_inv_bit = 0x1;

        /* That is the last data frame */
        dec->current_rx_J = RxMod->rx_Js - 1;
        pRx->current_rx_J = dec->current_rx_J;

        V34_Decoder_Init(DeCode);

        pRx->qdEq_scale = V34_tBASIC_SCALE[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate-1];
        pRx->qEq_nl_scale = V34_tOFFSET_NL_SCALE0[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate-1];

        if (pV34Rx->rx_me)
        {
            qdTemp = QDQMULQD(pRx->qdEq_scale, V34_tOFFSET_SCALE[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate - 1]);
            pRx->qdEq_scale += qdTemp;
            pRx->qEq_nl_scale = V34_tOFFSET_NL_SCALE1[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate-1];
        }

        pV34Rx->qRx_avg_egy = V34_EXPECT_POWER_LEVEL;

        pRx->qRx_nl_scale   = Q30_MAX / pRx->qEq_nl_scale;
        pRx->qdRx_scale      = Q30_MAX / pRx->qdEq_scale;

        pRx->rx_AGC.freeze = 0;

        for (i = 0; i < 6; i++)
        {
            pRx->rx_AGC.qHLK[i] = AGC_HLK_TAB[i];
        }
    }
}

#endif
