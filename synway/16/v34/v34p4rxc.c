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

/****************************************************************************/
/* Phase4 Rx vectors                                                        */
/****************************************************************************/

#include <string.h>
#include "v34ext.h"

CONST QWORD S_HLK_TAB[6] =
{
    V34_qS_H1,
    V34_qS_H2,
    V34_qS_L1,
    V34_qS_L2,
    V34_qS_K1,
    V34_qS_K2
};

CONST QWORD PP_HLK_TAB[6] =
{
    V34_qPP_H1,
    V34_qPP_H2,
    V34_qPP_L1,
    V34_qPP_L2,
    V34_qPP_K1,
    V34_qPP_K2
};

CONST QWORD TRN_HLK_TAB[6] =
{
    V34_qTRN_H1,
    V34_qTRN_H2,
    V34_qTRN_L1,
    V34_qTRN_L2,
    V34_qTRN_K1,
    V34_qTRN_K2
};

CONST QWORD AGC_HLK_TAB[6] =
{
    V34_qAGC_H1,/* 0.191882955; */
    V34_qAGC_H2,/* 0.763899804; */
    V34_qAGC_L1,/* 0.046769378; */
    V34_qAGC_L2,/* 0.011747936; */
    V34_qAGC_K1,
    V34_qAGC_K2
};


#if 0 /// LLL temp
/*******************************************/
/* Detect S bar , and turn on circuit 107  */
/*******************************************/
void V34_Phase4_Det_S_Bar(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    V34StateStruc *pV34State = &pRx->V34State;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD pcDsym[3], pcTimingOut[3];
    Phase4_Info *p4 = &(pV34->p4);
    UBYTE  i;

    V34_Demodulate(pRx, pcDsym);

    V34_Timing_Rec(pTCR, pcDsym, pcTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    if (V34_Detect_S_SBAR(pRx, pcTimingOut, &(p4->detect_symbol_counter)))
    {
        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = TRN_HLK_TAB[i];
        }

        if ((pRx->Renego.renego == 0) && (pRx->Renego.clear_down == 0))
        {
            /* Prepare next receive state */
            p4->rx_vec_idx++;

            /* goto send J' */
            p4->tx_vec_idx++;

            /* In total we should send 8 2-bit symbol out */
            /* Set 16 symbol for J to finish.             */
            p4->send_symbol_counter = 16;

            /* Prepare to wait for 16 symbols for start of TRN */
        }

        p4->detect_symbol_counter = 16;
    }

    /* Turn on circuit 107 */
    if ((pV34Rx->modem_mode == CALL_MODEM) && (p4->Time_cnt_SSBar_start) && ((pV34->Time_cnt - p4->Time_cnt_SSBar_start) > p4->pqTime_out_value[0]))
    {
        pRx->Retrain_flag         = RETRAIN_INIT;
        pV34State->retrain_reason = V34_PH4_DETECT_SSBAR_TIMEOUT;
    }
}


/*******************************************************/
/* suppose SBAR will last for 16T, wait for 16T here   */
/*******************************************************/
void V34_Phase4_Wait_S_Bar_16T(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD pcDsym[3];
    CQWORD pcTimingOut[3];
    CQWORD cqSymbol;            /* keep it!! */
    Phase4_Info *p4 = &(pV34->p4);

    V34_Demodulate(pRx, pcDsym);

    V34_Timing_Rec(pTCR, pcDsym, pcTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    /* Continue insert Symbols into Eq delayline */
    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pcTimingOut);

#if 0
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = cqSymbol.r; }
    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = cqSymbol.i; }
#endif

#if DRAWEPG
    /************* For Vxd EPG debug ********/
    pRx->pEpg[0] = cqSymbol;
    /**************** End of EPG ************/
#endif

    p4->detect_symbol_counter--;

    if (p4->detect_symbol_counter <= 0)
    {
        p4->detect_symbol_counter = V34_EQ_DELAY;

        (p4->rx_vec_idx)++; /* goto detect TRN */

        pV34Rx->train_sc = 0;

        pV34Rx->train_scram_idx = 16;

        if (pRx->Renego.renego || pRx->Renego.clear_down)
        {
            p4->Time_cnt_E_start = pV34->Time_cnt;
        }
    }
}


/************************************************************/
/* Delay for training equalizer since output of Eq is delay */
/************************************************************/
void V34_Phase4_EQ_1(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase4_Info *p4 = &(pV34->p4);
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    CQWORD cqSymbol;
    CQWORD cqErrsym;
    CQWORD cqTrain_sym;
    CQWORD pcDsym[3], pcTimingOut[3];

    cqTrain_sym.r = 0;
    cqTrain_sym.i = 0;

    V34_Demodulate(pRx, pcDsym);

    V34_Timing_Rec(pTCR, pcDsym, pcTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pcTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if DRAWEPG
    pRx->cqEQ_symbol = cqSymbol;

    pRx->pEpg[0] = cqSymbol;
#endif

    V34_Shift_Buf(pTCR->pcBaud_adjust, 3, cqSymbol);
    pTCR->pcT3_delay[0] = pTCR->pcT3_delay[1];
    pTCR->pcT3_delay[1] = cqTrain_sym;

    CQSUB(cqTrain_sym, cqSymbol, cqErrsym);

    if (pV34->LAL_modem == 1 && pAGC->freeze_EQ == 0)
    {
        V34_2T3_Equalizer_Update(cqErrsym, V34_qBETA_TRN, pTCR);
    }

    /* DO NOT Train EQ at all in TRN part of Phase 4 */

    if (p4->detect_symbol_counter <= 1)
    {
        p4->detect_symbol_counter = 512;

        p4->rx_vec_idx++; /* goto detect TRN */

#if PHASEJITTER
        V34_jt_decision(pJT);
#endif
    }
    else
    {
        p4->detect_symbol_counter --;
    }
}


CONST SWORD DetTrnSymCntTab[] =
{
    3120,
    3566,
    3937,
    4200,
    4500,
    4822,
};


/************************************************/
/* condition receiver to detect TRN             */
/* First 512 symbols of TRN, reference training.*/
/* This is the minimum training time.           */
/************************************************/
void V34_Phase4_Det_TRN(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpRx = &p4->MpRx;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    CQWORD pcDsym[3], pcTimingOut[3];
    CQWORD cqSymbol, cqTrain_sym, cqErrsym;
    UBYTE dbits;
    UBYTE byte_stream[2];

    V34_Demodulate(pRx, pcDsym);

    V34_Timing_Rec(pTCR, pcDsym, pcTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    if (pV34->LAL_modem == 1)
    {
        /* Receive TRN and train the equalizer */
        if (pV34Rx->train_scram_idx >= 16)
        {
            byte_stream[0] = 0xFF;
            byte_stream[1] = 0xFF;

            (*pV34Rx->pfScramTrn_byte)(byte_stream, &(pV34Rx->train_sc), pV34Rx->pTrain_scram_buf, 2);

            pV34Rx->train_scram_idx = 0;
        }

        V34_Agc(pAGC, pcTimingOut);
    }

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pcTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if 0

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = cqSymbol.r; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = cqSymbol.i; }

#endif

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        cqSymbol = V34_Rotator(&cqSymbol, pJT->qS, pJT->qC);
    }

#endif

    if (pV34->LAL_modem == 1)
    {
        cqTrain_sym = V34_Create_TRN(pV34Rx->pTrain_scram_buf, &(pV34Rx->train_scram_idx), pV34Rx->Num_Trn_Rx);
    }
    else
    {
        dbits = V34_Symbol_To_Bits(cqSymbol, &cqTrain_sym, pV34Rx->Num_Trn_Rx);
    }

#if DRAWEPG
    pRx->pEpg[0] = cqSymbol;
#endif

    /* DO NOT Train EQ at all in TRN part of Phase 4 except Loopback */
    CQSUB(cqTrain_sym, cqSymbol, cqErrsym);

#if RX_PRECODE_ENABLE
    V34_NoiseWhiten(&(pV34->NoiseWhiten), &cqErrsym);
#endif

    if (pV34->LAL_modem == 1 && pAGC->freeze_EQ == 0)
    {
        V34_2T3_Equalizer_Update(cqErrsym, V34_qBETA_TCR, pTCR);
    }

    /* This is for the checking of enough training on our */
    /* side, if it is less than zero, 512T has past */
    p4->detect_symbol_counter--;

    V34_TimingLoop(pTCR, &cqSymbol, &cqTrain_sym);

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        pJT->counter++;

        V34_jt_loop(pJT, &cqSymbol, &cqTrain_sym, 0);
    }

#endif

    V34_CarrierLoop(pTCR, &cqSymbol, &cqTrain_sym);

    /* If send_state is 3, we already send TRN for >= 512T */
    /* and 2000 ms to go in send_state 3 for extra training */
    /* Condition the receiver to look for MP until our eq */
    /* train adequately */
    if (p4->detect_symbol_counter <= 0)
    {
        p4->detect_symbol_counter = DetTrnSymCntTab[pV34Rx->tx_symbol_rate];

        /* Timing Drift Calculation */

        p4->rx_vec_idx++;

        pTCR->qCarAcoef = 32368;
        pTCR->qCarBcoef = 400;
        pTCR->qB3 = 100;

        /* Initialize differential decoder to the final */
        /* TRN symbol to decode MP sequence */
        dbits = V34_Symbol_To_Bits(cqSymbol, &cqTrain_sym, pV34Rx->Num_Trn_Rx);

        DeCode->dZ = (SBYTE)(dbits & 0x3);

        V34_Detect_Mp_Init(pMpRx);

        pRx->dsc = 0; /* V34_Descrambler_Init */

        pRx->qdARS_erregy = 0;

#if RX_PRECODE_ENABLE
        pV34->NoiseWhiten.ubBetaShift = 0;
#endif
    }
}


UBYTE V34_P4Rxc_Common(V34Struct *pV34, CQWORD *pcqErrsym, QWORD qEQ_Beta)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    CQWORD cqSymbol, cqTrain_sym;
    CQWORD cqDsym[3];
    CQWORD pcTimingOut[3];
    UBYTE  bit_stream[4];
    UBYTE  dbits, diffbits;

    V34_Demodulate(pRx, cqDsym);

    V34_Timing_Rec(pTCR, cqDsym, pcTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return 0;
    }

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pcTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cqSymbol.r; }
    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = cqSymbol.i; }
#endif

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        cqSymbol = V34_Rotator(&cqSymbol, pJT->qS, pJT->qC);
    }

#endif

#if DRAWEPG
    /************* For Vxd EPG debug ********/
    pRx->pEpg[0].r = cqSymbol.r;
    pRx->pEpg[0].i = cqSymbol.i;
    /**************** End of EPG ************/
#endif

    dbits = V34_Symbol_To_Bits(cqSymbol, &cqTrain_sym, pV34Rx->Num_Trn_Rx);
    CQSUB(cqTrain_sym, cqSymbol, (*pcqErrsym));

#if RX_PRECODE_ENABLE
    V34_NoiseWhiten(&pV34->NoiseWhiten, pcqErrsym);
#endif

    if (pAGC->freeze_EQ == 0)
    {
        V34_2T3_Equalizer_Update(*pcqErrsym, qEQ_Beta, pTCR);
    }

    V34_TimingLoop(pTCR, &cqSymbol, &cqTrain_sym);

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        pJT->counter++;

        V34_jt_loop(pJT, &cqSymbol, &cqTrain_sym, 0);
    }

#endif

    V34_CarrierLoop(pTCR, &cqSymbol, &cqTrain_sym);

    V34_DIFF_DECODE(DeCode, (dbits & 0x3), diffbits);

    bit_stream[0] = diffbits & 0x1;
    bit_stream[1] = (diffbits >> 1) & 0x1;
    bit_stream[2] = (dbits >> 2) & 0x1;
    bit_stream[3] = (dbits >> 3) & 0x1;

    (*pRx->pfDescram_bit)(bit_stream, &(pRx->dsc), pRx->pDscram_buf, pV34Rx->rx_symbol_bits);

    return 1;
}


/**************************************/
/* condition receiver to detect TRN   */
/**************************************/
void V34_Phase4_Det_TRN_TC(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    CQWORD cqErrsym;
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpRx = &p4->MpRx;
    UBYTE i;

    if (V34_P4Rxc_Common(pV34, &cqErrsym, pRx->qEq_Beta) == 0)
    {
        return;
    }

    for (i = 0; i < pV34Rx->rx_symbol_bits; i++)
    {
        V34_Detect_Mp((UBYTE)((pRx->pDscram_buf[0] >> i) & 0x1), pMpRx);
    }

    /* This is for the checking of enough training on our */
    /* side, if it is less than zero, 512T has past */
    p4->detect_symbol_counter--;

#if RX_PRECODE_ENABLE

    if (p4->detect_symbol_counter == 3000)
    {
        pV34->NoiseWhiten.ubBetaShift = 13;
    }
    else if (p4->detect_symbol_counter == 1500)
    {
        pV34->NoiseWhiten.ubBetaShift = 15;
    }

#endif

    if (p4->detect_symbol_counter < (1024 + 16))
    {
        V34_ARS_Calc(pRx, &cqErrsym);
    }

#if 0

    /* the condition was added by someone at 28.001.017,                                       */
    /* then was asked to change to the condition by someone at 28.001.031                      */
    /* in fact, for the condition, the result is the same with 28.001.016, mean it is unuseful */
    if (p4->detect_symbol_counter == 515)
    {
        pRx->qEq_Beta = V34_qBETA_TRN_PH4;
    }

#endif

    if (p4->detect_symbol_counter == 100)
    {
        pRx->qEq_Beta = 0;
    }

    /* Bug in Finite State Machine for Phase 4 */
    /* Need to ensure that we have begun sending MP */
    /* before MP has been detected, otherwise FSM fails  */
    if (p4->detect_symbol_counter == 15)
    {
        /* ARS / 1024 */
        if (pV34->LAL_modem)
        {
            pRx->qRTRN_Req_Thres = 32767;
        }
        else
        {
            V34_ARS_est(pV34);
        }

        /* MP not to be sent if retrain set by V34_ARS_est() */
        if (pRx->Retrain_flag != RETRAIN_INIT)
        {
            p4->send_symbol_counter = 0;
        }
    }

    /* If send_state is 3, we already send TRN for >= 512T */
    /* and 2000 ms to go in send_state 3 for extra training */
    /* Condition the receiver to look for MP until our eq */
    /* train adequately */
    if (p4->detect_symbol_counter <= 0)
    {
        /* Prepare to receive MP */
        p4->rx_vec_idx++;

        pAGC->freeze = 0;
    }
}

/**************************************/
/*  Receive MP from the answer side   */
/**************************************/
void V34_Phase4_Rec_MP(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    VA_INFO        *dec = &(DeCode->Vdec);    /* VA INFO structure */
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    V34StateStruc *pV34State = &pRx->V34State;
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
    MpStruc *pMpRx = &p4->MpRx;
    CQWORD cqErrsym;
    UBYTE i;

    if (V34_P4Rxc_Common(pV34, &cqErrsym, pRx->qEq_Beta) == 0)
    {
        return;
    }

    p4->result_code = 0;

    for (i = 0; i < pV34Rx->rx_symbol_bits; i++)
    {
        p4->result_code = V34_Detect_Mp((UBYTE)((pRx->pDscram_buf[0] >> i) & 0x1), pMpRx);

        if (p4->result_code)
        {
            break;
        }
    }

    if (p4->result_code == 1)
    {
        /* So, MP is detected, complete the current MP */
        /* and send out MP' */
        /* first check if the two modem have common bit rate */
        if ((pMpRx->data_rate_cap_mask & pMpTx->data_rate_cap_mask) == 0)
        {
            pV34State->hang_up       = 1;
            pV34State->hangup_reason = V34_NO_COMMON_BIT_RATE;
            return;
        }

        p4->tx_vec_idx++;

        /* goto receive MP' */
        p4->rx_vec_idx++;

        TRACE2("REC MP: %d  %d", p4->rx_vec_idx, p4->tx_vec_idx);

        if (pV34->LAL_modem == 0)
        {
#if TX_PRECODE_ENABLE
            /* And also prepare to Initalize all data in data mode */
            if (pMpRx->type == MP_TYPE_1)
            {
                V34_Precoding_CoefInit(&(pV34Tx->tx_precode), &(pMpRx->PrecodeCoeff_h[0].r));

                TRACE2("RX-PRECODE (1) %6d  %6d", pMpRx->PrecodeCoeff_h[0].r, pMpRx->PrecodeCoeff_h[0].i);
                TRACE2("           (2) %6d  %6d", pMpRx->PrecodeCoeff_h[1].r, pMpRx->PrecodeCoeff_h[1].i);
                TRACE2("           (3) %6d  %6d", pMpRx->PrecodeCoeff_h[2].r, pMpRx->PrecodeCoeff_h[2].i);
            }
#endif

            /* Setting up Tx bit rate and Rx bit rate */
            if (pV34Rx->modem_mode == CALL_MODEM)
            {
                if (pMpRx->max_call_to_ans_rate <= pMpTx->max_call_to_ans_rate)
                {
                    pV34Rx->tx_bit_rate = pMpRx->max_call_to_ans_rate;
                }
                else
                {
                    pV34Rx->tx_bit_rate = pMpTx->max_call_to_ans_rate;
                }

                if (pMpRx->max_ans_to_call_rate <= pMpTx->max_ans_to_call_rate)
                {
                    pV34Rx->rx_bit_rate = pMpRx->max_ans_to_call_rate;
                }
                else
                {
                    pV34Rx->rx_bit_rate = pMpTx->max_ans_to_call_rate;
                }
            }
            else
            {
#if 0
                TRACE2("%d   %d", pMpRx->max_ans_to_call_rate, pMpTx->max_ans_to_call_rate);
                TRACE2("%d   %d", pMpRx->max_call_to_ans_rate, pMpTx->max_call_to_ans_rate);
#endif

                if (pMpRx->max_ans_to_call_rate <= pMpTx->max_ans_to_call_rate)
                {
                    pV34Rx->tx_bit_rate = pMpRx->max_ans_to_call_rate;
                }
                else
                {
                    pV34Rx->tx_bit_rate = pMpTx->max_ans_to_call_rate;
                }

                if (pMpRx->max_call_to_ans_rate <= pMpTx->max_call_to_ans_rate)
                {
                    pV34Rx->rx_bit_rate = pMpRx->max_call_to_ans_rate;
                }
                else
                {
                    pV34Rx->rx_bit_rate = pMpTx->max_call_to_ans_rate;
                }
            }

            /* Check remote modem MP mask bit for transmit bit rate */
            /* Gao's MP mask is always (1<<pV34->tx_bit_rate)-2, continuous, no gap */
            while (0 == (pMpRx->data_rate_cap_mask >> (pV34Rx->tx_bit_rate - V34_BIT_2400)))
            {
                pV34Rx->tx_bit_rate--;
            }

            while (0 == (pMpRx->data_rate_cap_mask >> (pV34Rx->rx_bit_rate - V34_BIT_2400)))
            {
                pV34Rx->rx_bit_rate--;
            }

            TRACE2("ans to call %d   %d", pMpRx->max_ans_to_call_rate, pMpTx->max_ans_to_call_rate);
            TRACE2("call to ans %d   %d", pMpRx->max_call_to_ans_rate, pMpTx->max_call_to_ans_rate);

            if ((pMpRx->asymmetric_data_sig_rate == 0) || (pMpTx->asymmetric_data_sig_rate == 0))
            {
                if (pV34Rx->tx_bit_rate < pV34Rx->rx_bit_rate)
                {
                    pV34Rx->rx_bit_rate = pV34Rx->tx_bit_rate;
                }
                else
                {
                    pV34Rx->tx_bit_rate = pV34Rx->rx_bit_rate;
                }
            }

            pV34Tx->Min_Exp = pMpRx->const_shap_select_bit;

            V34_Parameter_Function_Init_Tx(pV34);

            pV34Rx->rx_me = pMpTx->const_shap_select_bit;

            V34_Parameter_Function_Init_Rx(pV34);
        }

        pV34Tx->conv_state = pMpRx->TrellisEncodSelect;

        pV34Tx->nlinear = pMpRx->NonlinearEncodParam;

        if (pShellMap->M > 1)
        {
            V34_Calc_g2(pShellMap->M, pShellMap->pG2);
            V34_Calc_g4(pShellMap->M, pShellMap->pG2, pShellMap->pG4);
        }

        dec->rx_COV_type = pMpTx->TrellisEncodSelect;

        pRx->rx_nlinear  = pMpTx->NonlinearEncodParam;

        if (RxMod->rx_M > 1)
        {
            V34_Calc_g2(RxMod->rx_M, pRx->RxShellMap.pRx_g2);
            V34_Calc_g4(RxMod->rx_M, pRx->RxShellMap.pRx_g2, pRx->RxShellMap.pRx_g4);
            V34_Calc_g8(RxMod->rx_M, pRx->RxShellMap.pRx_g4, pRx->RxShellMap.pRx_g8);
        }

        p4->curr_word = 0;
        /* For detect E sequence, that could be in MP_PI */
#if 0
        TRACE2("NL:TX-%d  RX-%d", pRx->rx_nlinear, pV34Tx->nlinear);
        TRACE2("BitRate: Tx:%d  Rx:%d", pV34->tx_bit_rate, pV34->rx_bit_rate);
#endif
    }
}
#endif

void V34_Phase4_ReceiverSetup(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    VA_INFO        *dec = &(DeCode->Vdec);    /* VA INFO structure */
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
    QWORD qTemp;
    UBYTE  i;

    /* Initial Descrambler and Differential decoder */
    pRx->dsc = 0; /* V34_Descrambler_Init */

    DeCode->dZ = 0;

#if RX_PRECODE_ENABLE
    if (pMpTx->type == MP_TYPE_1)
    {
        V34_Precoding_CoefInit(&(pRx->rx_precode), &(pMpTx->PrecodeCoeff_h[0].r));
        V34_Precoding_CoefInit(&(pRx->VA_precode), &(pMpTx->PrecodeCoeff_h[0].r));
    }
#endif

    /* This is a quite time consuming operation */
    if (RxMod->rx_M > 1)
    {
        V34_Calc_z8(RxMod->rx_M, pRx->RxShellMap.pRx_g8, pRx->RxShellMap.pRx_z8);
    }

    /* Since last data frame will used the last two bits of */
    /* the bit inversion pattern, which is bit 1 and bit 0  */
    /* we setup the variable to be 1.                       */
    dec->current_rx_inv_bit = 0x1;

    /* That is the last data frame */
    dec->current_rx_J = RxMod->rx_Js - 1;
    pRx->current_rx_J = dec->current_rx_J;

    V34_Decoder_Init(DeCode);

    pRx->qdEq_scale = V34_tBASIC_SCALE[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate - V34_BIT_2400];

    if (pV34Rx->rx_me)
    {
        qTemp = (QWORD)QDQMULQD(pRx->qdEq_scale, V34_tOFFSET_SCALE[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate - V34_BIT_2400]);

        pRx->qdEq_scale += qTemp;
        pRx->qEq_nl_scale = V34_tOFFSET_NL_SCALE1[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate - V34_BIT_2400];
    }
    else
    {
        pRx->qEq_nl_scale = V34_tOFFSET_NL_SCALE0[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate - V34_BIT_2400];
    }

    pV34Rx->qRx_avg_egy = V34_EXPECT_POWER_LEVEL;

    pRx->qRx_nl_scale = Q30_MAX / pRx->qEq_nl_scale;
    pRx->qdRx_scale   = Q30_MAX / pRx->qdEq_scale;

    pRx->qRTRN_Req_Thres = QQMULQR8(V34_tEQ_ARS_THRESHOLD[pV34Rx->tx_symbol_rate][pV34Rx->rx_bit_rate - V34_BIT_2400], V34_qARS_THRESHOLD_SCALE[pV34Rx->rx_bit_rate - V34_BIT_2400]);

    pAGC->freeze = 0;

    for (i = 0; i < 6; i++)
    {
        pAGC->qHLK[i] = AGC_HLK_TAB[i];
    }
}

#if 0 /// LLL temp
/********************************************/
/*  Receive MP' from the answer side        */
/********************************************/
void V34_Phase4_Rec_MP_Pi(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpRx = &p4->MpRx;
    CQWORD cqErrsym;
    UBYTE i;
    UBYTE bit_i;

    if (V34_P4Rxc_Common(pV34, &cqErrsym, pRx->qEq_Beta) == 0)
    {
        return;
    }

    p4->result_code = 0;

    for (i = 0; i < pV34Rx->rx_symbol_bits; i++)
    {
        p4->curr_word  <<= 1;

        bit_i = (pRx->pDscram_buf[0] >> i) & 0x1;

        p4->curr_word   |= bit_i;

        p4->result_code |= V34_Detect_Mp(bit_i, pMpRx);
    }

    p4->curr_word &= 0x000FFFFFL;

    if ((p4->tx_vec_idx == 6) && (p4->result_code == 1) && (pMpRx->acknowledge_bit == 1))
    {
        /* MP' has begun to be sent and MP' is detected, complete the current MP' and send out E */
        p4->tx_vec_idx++;

        /* goto detect E */
        p4->rx_vec_idx++;

        TRACE2("REC MP PI: %d  %d", p4->rx_vec_idx, p4->tx_vec_idx);
    }
    else if (p4->curr_word == 0x000fffffL)
    {
        /* MP' is detected, complete the current MP' and send out E */
        p4->tx_vec_idx++;

        p4->rx_vec_idx += 2;

        TRACE2("REC MP PI and E: %d  %d", p4->rx_vec_idx, p4->tx_vec_idx);

        V34_Phase4_ReceiverSetup(pV34);
    }
}

/*************/
/* Detect E  */
/*************/
void V34_Phase4_Detect_E(V34Struct *pV34)
{
    V34RxStruct      *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc        *pRx = &pV34Rx->RecStruc;
    V34StateStruc *pV34State = &pRx->V34State;
    Phase4_Info          *p4 = &(pV34->p4);
    CQWORD cqErrsym;
    UBYTE i;

    if (V34_P4Rxc_Common(pV34, &cqErrsym, pRx->qEq_Beta) == 0)
    {
        return;
    }

    for (i = 0; i < pV34Rx->rx_symbol_bits; i++)
    {
        p4->curr_word  = (p4->curr_word << 1) & 0x000FFFFFL;
        p4->curr_word |= (pRx->pDscram_buf[0] & 0x1);

        pRx->pDscram_buf[0] >>= 1;

        if (p4->curr_word == 0x000fffffL)
        {
            break;
        }
    }

    if (p4->curr_word == 0x000fffffL)
    {
        /* goto receive B1 frame */
        p4->rx_vec_idx++;

        pRx->ubB1_Err_Count = 0;

        TRACE2("REC E: %d  %d", p4->rx_vec_idx, p4->tx_vec_idx);

        V34_Phase4_ReceiverSetup(pV34);
    }

    /* test time out for detecting E */
    if (p4->Time_cnt_E_start && ((pV34->Time_cnt - p4->Time_cnt_E_start) > p4->pqTime_out_value[1]))
    {
        pRx->Retrain_flag         = RETRAIN_INIT;
        pV34State->retrain_reason = V34_PH4_DETECT_E_TIMEOUT;
    }
}

/**************************/
/* receive B1 data frame  */
/**************************/
void V34_Phase4_Rec_B1_Frame(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase4_Info *p4 = &(pV34->p4);

    V34_Receiver(pRx, pV34->pTable, 0);

    if (pRx->current_rx_J == 0)
    {
        p4->Phase4_RxEnd = 1; /* phase4 Rx flag */

        /* goto receive Superframe */
        (p4->rx_vec_idx)++;

#if 0//PHASEJITTER
        {
            UBYTE i;

            for (i = 0; i < V34_JTTAPS; i++)
            {
                DumpTone6[DumpTone6_Idx++] = pJT->cf[i];
            }
        }
#endif
    }
}


/********************************/
/* receive start of superframe  */
/********************************/
void V34_Phase4_Rec_Superframe(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;

    V34_Receiver(pRx, pV34->pTable, 0);
}

/**************/
/* VA delay   */
/**************/
void V34_Phase4_VA_Delay(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase4_Info *p4 = &(pV34->p4);
    V34AgcStruc  *pAGC   = &(pRx->rx_AGC);
    DeCodeStruc  *DeCode = &(pRx->DeCode);
    VA_INFO      *dec    = &(DeCode->Vdec);    /* VA INFO structure */
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
#if RX_PRECODE_ENABLE
    PrecodeStruct *precode_ptr = &(pRx->rx_precode);
#endif
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    CQWORD cqSymbol, cqErrsym, cqUs;
    CQWORD cqD_sym;
    CQWORD pcDsym[3], pcTimingOut[3];
    CQWORD pcYout[2];
    CQWORD cX, cY, cU;
    CQWORD cSlice;
    UBYTE  i;

    V34_Demodulate(pRx, pcDsym);

    V34_Timing_Rec(pTCR, pcDsym, pcTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pcTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        cqSymbol = V34_Rotator(&cqSymbol, pJT->qS, pJT->qC);
    }

#endif

#if DRAWEPG
    pRx->pEpg[0] = cqSymbol;
#endif

    V34_4Point_Slicer(&pRx->Renego, &cqSymbol);

    cqUs = cqSymbol;


    if (pRx->rx_nlinear)
    {
        V34_Non_Linear_Decoder(pRx->qRx_nl_scale, &cqSymbol);
    }

    /* scale down for different data rate */
    cqSymbol.r = (QWORD)QDQMULQD(pRx->qdRx_scale, cqSymbol.r);
    cqSymbol.i = (QWORD)QDQMULQD(pRx->qdRx_scale, cqSymbol.i);

    /* Change to 9.7 format */
    cX.r = cqSymbol.r >> 2;
    cX.i = cqSymbol.i >> 2;

    /* Precoder to get p(n), y(n) = x(n) + p(n) */
    cY = cX;

#if RX_PRECODE_ENABLE
    cY.r += pRx->VA_precode.cP.r;
    cY.i += pRx->VA_precode.cP.i;
#endif

    /* Slicer for instance Symbol error calculation */
    cSlice = V34_Slicer(&cY);

    /* Find the Decision point for Loop updated, Eq update */
    cqD_sym.r = cSlice.r << 2;
    cqD_sym.i = cSlice.i << 2;

#if RX_PRECODE_ENABLE
    cqD_sym.r -= (pRx->VA_precode.cP.r << 2);
    cqD_sym.i -= (pRx->VA_precode.cP.i << 2);
#endif

    cqD_sym.r = QDQMULQDR(pRx->qdEq_scale, cqD_sym.r);
    cqD_sym.i = QDQMULQDR(pRx->qdEq_scale, cqD_sym.i);

    if (pRx->rx_nlinear)
    {
        V34_Non_Linear_Encoder(pRx->qEq_nl_scale, &cqD_sym);
    }

    if (pRx->Renego.clear_down || pRx->Renego.renego_generate)
    {
        if (pRx->Renego.S_DetCounter > 76)
        {
            p4->rx_vec_idx ++;
        }

        return;
    }

    CQSUB(cqD_sym, cqUs, cqErrsym);

    if (pAGC->freeze_EQ == 0)
    {
        V34_2T3_Equalizer_Update(cqErrsym, pRx->qEq_Beta, pTCR);
    }

    V34_TimingLoop(pTCR, &cqUs, &cqD_sym);

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        pJT->counter++;

        V34_jt_loop(pJT, &cqSymbol, &cqD_sym, 0);
    }

#endif

    V34_CarrierLoop(pTCR, &cqUs, &cqD_sym);

#if RX_PRECODE_ENABLE
    V34_Precoder(&pRx->VA_precode, &cX, 0);
#endif

    dec->pcVA_sym[dec->VA_sym_inidx] = cY;

    dec->VA_sym_inidx ++;
    dec->VA_sym_inidx &= V34_VA_BUF_MASK;

#if 0
    if (DumpTone7_Idx < 5000000) { DumpTone7[DumpTone7_Idx++] = cY.r; }
    if (DumpTone8_Idx < 5000000) { DumpTone8[DumpTone8_Idx++] = cY.i; }
#endif

    dec->VA_sym_count ++;
    if (dec->VA_sym_count >= 2)
    {
		dec->VA_sym_count = 0;

        V34_VA_Decode(DeCode, pcYout);

#if 0
        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pcYout[0].r; }
        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pcYout[0].i; }
        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pcYout[1].r; }
        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pcYout[1].i; }
#endif

        if (pcYout[0].r != 0)
        {
            V34_Precode_Sub(pRx, pcYout);

            ++p4->rx_vec_idx;

            p4->detect_symbol_counter = 0;

            pRx->qdARS_erregy            = 0;
            pRx->Retrain_Request_Counter = 0;
            pRx->Retrain_LO_BER_Counter  = 0;
            pRx->Retrain_HI_BER_Counter  = 0;
            pRx->qPrev_erregy            = 32767;

            pAGC->qHLK[4] = AGC_HLK_TAB[4];  /* K1 */
            pAGC->qHLK[5] = AGC_HLK_TAB[5];  /* K2 */
        }
    }
}
#endif