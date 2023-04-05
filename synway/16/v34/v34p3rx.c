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

/****************************************************************************
* Phase3 Tx vectors
*
* Functions:
*      V34_Phase3_Det_S(V34Struct * pV34)
*      V34_Phase3_Rec_S_Det_S_Bar(V34Struct * pV34)
*      V34_Phase3_EQ_1(V34Struct * pV34)
*      V34_Phase3_Rec_PP_EQ(V34Struct * pV34)
*      V34_Phase3_Rec_TRN_EQ(V34Struct * pV34)
*      V34_Phase3_Rec_TRN_EQ_TC(V34Struct * pV34)
*      V34_Phase3_Det_J(V34Struct * pV34)
*      V34_Phase3_Det_S_Call(V34Struct * pV34)
*      V34_Phase3_Dummy_Call(V34Struct * pV34)
*      V34_Phase3_Rx_Dummy(V34Struct * pV34)
*      V34_Phase3_Rec_J_Setup(V34Struct * pV34)
*      V34_Phase3_Rec_J_Dummy(V34Struct * pV34)
*      V34_Phase3_Wait_MD(V34Struct * pV34)
*
* Lasted Updated:
*      Apr 03, 1996.
*      Jul 11, 1996. Benjamin, change input parameter to V34Struct *
* Author:
*      Dennis Chan, GAO Research & Consulting Ltd.
*      Benjamin Chan, GAO Research & Consulting Ltd.
****************************************************************************/

#include "v34ext.h"

#if 0 /// LLL temp
void V34_MD_Setup(UBYTE md_length, UBYTE tx_symbol_rate, Phase3_Info *p3)
{
    if (md_length > 0)
    {
        /* 112 symbols is 35ms for 3200Hz symbol rate */
        /* 84 symbols is 35ms for 2400Hz symbol rate */
        p3->detect_symbol_counter = (SWORD)(UBUBMULU(md_length, V34_tSym_Num[tx_symbol_rate])  + 16 + 64);

        p3->rx_vec_idx = 11;
    }
}

void V34_Phase3_Det_S(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    V34StateStruc *pV34State = &pRx->V34State;
    Phase3_Info *p3 = &(pV34->p3);
    UBYTE Det_S;
    UBYTE i;

    Det_S = V34_S_Detector(pRx, pV34Rx->tTimingOut);

    if (Det_S)
    {
        p3->rx_vec_idx++;
        p3->detect_symbol_counter = 0;
        pRx->S_trn_point = 0;

        pRx->trn_count = 0;
        pRx->qTrn_avg  = 0;

        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = S_HLK_TAB[i];
        }

        TRACE0("V34_qS_K1");

        if (pV34Rx->modem_mode == CALL_MODEM)
        {
            V34_MD_Setup(pV34Rx->md_length, pV34Rx->tx_symbol_rate, p3);
        }

#if USE_ANS
        else if (pV34Rx->modem_mode == ANS_MODEM)
        {
            /* For Answer Modem:                      */
            /* when detect S_Bar, Tx state goes from  */
            /* sending J to send silence              */
            p3->tx_vec_idx++;

            V34_MD_Setup(pV34Rx->md_length, pV34Rx->tx_symbol_rate, p3);
        }

#endif
    }

    /* for time out test */
    if (((pV34Rx->modem_mode == CALL_MODEM) && (pV34->Time_cnt > p3->pqTime_out_value[1]))
#if USE_ANS
        || ((pV34Rx->modem_mode == ANS_MODEM)  && (pV34->Time_cnt > p3->pqTime_out_value[2]))
#endif
       )
    {
        pRx->Retrain_flag         = RETRAIN_INIT;
        pV34State->retrain_reason = V34_ANS_DETECT_S_TIMEOUT;
    }
}

void V34_Det_S_bar_Sub(ReceiveStruc *pRx)
{
    QWORD qTemp = 0;
    UBYTE i;

    if (pRx->trn_count < 96)
    {
        for (i = 0; i < EC_INTER_NUM; i++)
        {
            qTemp += QQMULQ15(pRx->qEchoCancel_Out[i], pRx->qEchoCancel_Out[i]);
        }

        qTemp = QQMULQR15(qTemp, q1_OVER_96);

        pRx->qTrn_avg  += qTemp;
        pRx->trn_count += 3;
    }
}


void V34_Phase3_Rec_S_Det_S_Bar(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    V34StateStruc *pV34State = &pRx->V34State;
    Phase3_Info *p3 = &(pV34->p3);
    UBYTE i;

    /* V34_Agc(pAGC, pV34->tTimingOut); */

    V34_Det_S_bar_Sub(pRx);

    if (V34_Detect_S_SBAR(pRx, pV34Rx->tTimingOut, &(p3->detect_symbol_counter)))
    {
        p3->rx_vec_idx++;
        p3->detect_symbol_counter = 16;

        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = PP_HLK_TAB[i];
        }

        TRACE0("PP_HLK_TAB");

#if USE_ANS

        /* init the time cnt for answer modem detect J sequence */
        if (pV34Rx->modem_mode == ANS_MODEM)
        {
            p3->Time_cnt_J_start = pV34->Time_cnt;
        }

#endif
    }

#if USE_ANS

    /* time out test for s to s bar transition,
    when add renegotiation, may change the time out value */
    if ((pV34Rx->modem_mode == ANS_MODEM) && p3->Time_cnt_SSBar_start &&
        ((pV34->Time_cnt - p3->Time_cnt_SSBar_start) > p3->pqTime_out_value[0]))
    {
        pRx->Retrain_flag         = RETRAIN_INIT;
        pV34State->retrain_reason = V34_ANS_DETECT_SSBAR_TIMEOUT;
    }

#endif
}

void V34_Phase3_Wait_S_Bar_16T(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase3_Info *p3 = &(pV34->p3);
    CQWORD cqSymbol;

    V34_Agc(pAGC, pV34Rx->tTimingOut);

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);

    pRx->cqEQ_symbol = cqSymbol;

    p3->detect_symbol_counter --;

    if (p3->detect_symbol_counter <= 0)
    {
        p3->rx_vec_idx++ ;
        p3->detect_symbol_counter = V34_EQ_DELAY;
        p3->DelayIndex = 0;
        p3->EqDelayCount = 0;

        p3->K = 0;
        p3->I = 0;
    }
}

void V34_P3_RxFunc(UWORD FuncCode, V34Struct *pV34, CQWORD Train_sym, QWORD qBeta)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD      cqSymbol;
    CQWORD      Errsym;
    Phase3_Info *p3 = &(pV34->p3);

    V34_Agc(pAGC, pV34Rx->tTimingOut);

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

    pRx->cqEQ_symbol = cqSymbol;

    if (FuncCode != P3_RXFUNC_EQ1)
    {
        CQSUB(Train_sym, cqSymbol, Errsym);

        if (pAGC->freeze_EQ == 0)
        {
            V34_2T3_Equalizer_Update(Errsym, qBeta, pTCR);
        }
    }
    else if ((FuncCode == P3_RXFUNC_EQ1) && (++p3->EqDelayCount >= 22))
    {
        p3->EqDelayCount = 22;

        p3->DelayIndex += 3;

        if (p3->DelayIndex >= 84)
        {
            p3->DelayIndex = 0;
        }
    }

    if (FuncCode != P3_RXFUNC_TRN_EQ)
    {
        V34_Shift_Buf(pTCR->pcBaud_adjust, 3, cqSymbol);
        pTCR->pcT3_delay[0] = pTCR->pcT3_delay[1];
        pTCR->pcT3_delay[1] = Train_sym;
    }
    else if (FuncCode == P3_RXFUNC_TRN_EQ)
    {
        p3->detect_symbol_counter --;

        if (p3->detect_symbol_counter < 256)
        {
            V34_TimingLoop(pTCR, &cqSymbol, &Train_sym);
            V34_CarrierLoop(pTCR, &cqSymbol, &Train_sym);
        }
    }
}

/*****************************************************************/
/* Delay for training equalizer since output of Eq is delay      */
/*****************************************************************/
void V34_Phase3_EQ_1(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);
    CQWORD Train_sym;

    Train_sym.r = 0;
    Train_sym.i = 0;

    V34_P3_RxFunc(P3_RXFUNC_EQ1, pV34, Train_sym, 0);

    if (--p3->detect_symbol_counter <= 0)
    {
        p3->rx_vec_idx++;
        p3->detect_symbol_counter = 288;
        p3->I = 0;
        p3->K = 0;
    }
}

/****************************************/
/* Receive PP and train the Equalizer   */
/****************************************/
void V34_Phase3_Rec_PP_EQ(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    Phase3_Info *p3 = &(pV34->p3);
    CQWORD Train_sym;
    UBYTE  i;

    /* Create PP symbol pattern for training EQ */
    if (p3->I > 3)
    {
        p3->I = 0;
        p3->K ++;
    }

    Train_sym = V34_Create_PP(p3->K, p3->I);
    (p3->I)++;

    V34_P3_RxFunc(P3_RXFUNC_PP_EQ, pV34, Train_sym, V34_qBETA_PP);

    if (--p3->detect_symbol_counter <= 0)
    {
        p3->rx_vec_idx++;
        p3->detect_symbol_counter = 512;

        pV34Rx->train_sc = 0;
        pV34Rx->train_scram_idx = 16;
        p3->pByte_stream[0] = 0xFF;
        p3->pByte_stream[1] = 0xFF;

        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = TRN_HLK_TAB[i];
        }
    }
}

/************************************************************/
/* Receive TRN for at least 512 symbols, fine train the Eq. */
/************************************************************/
void V34_Phase3_Rec_TRN_EQ(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase3_Info *p3 = &(pV34->p3);
    CQWORD Train_sym;
    UBYTE  dbits;

    if (pV34Rx->train_scram_idx >= 16)
    {
        (*pV34Rx->pfScramTrn_byte)(p3->pByte_stream, &(pV34Rx->train_sc), pV34Rx->pTrain_scram_buf, 2);
        pV34Rx->train_scram_idx = 0;
    }

    Train_sym = V34_Create_TRN(pV34Rx->pTrain_scram_buf, &(pV34Rx->train_scram_idx), V34_4_POINT_TRN);

    V34_P3_RxFunc(P3_RXFUNC_TRN_EQ, pV34, Train_sym, V34_qBETA_TRN);

    if (p3->detect_symbol_counter <= 0)
    {
        p3->rx_vec_idx++;

        switch (pV34Rx->tx_symbol_rate)
        {
            case V34_SYM_2400:
                p3->detect_symbol_counter = pV34->fecount + 2250;
                break;
            case V34_SYM_2743:
                p3->detect_symbol_counter = pV34->fecount + 2571;
                break;
            case V34_SYM_2800:
                p3->detect_symbol_counter = pV34->fecount + 2625;
                break;
            case V34_SYM_3000:
                p3->detect_symbol_counter = pV34->fecount + 2812;
                break;
            case V34_SYM_3200:
                p3->detect_symbol_counter = pV34->fecount + 3000;
                break;
            case V34_SYM_3429:
                p3->detect_symbol_counter = pV34->fecount + 3214;
                break;
        }

        pV34Rx->sequence = 0;

        dbits = V34_Symbol_To_Bits(pRx->cqEQ_symbol, &Train_sym, V34_4_POINT_TRN);

        DeCode->dZ = (SBYTE)dbits;

        pTCR->qCarAcoef = q097;
        pTCR->qCarBcoef = q003;
        pRx->qEq_Beta   = V34_qBETA_TRN_MIDDLE;
    }
}

void V34_P3_RxBasic(V34Struct *pV34, QWORD qBeta)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    V34AgcStruc   *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    CQWORD cqSymbol;
    CQWORD Train_sym;
    CQWORD Errsym;
    UBYTE  dbits, diffbits;
    UBYTE  bit_stream[2];

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);
    pRx->cqEQ_symbol = cqSymbol;

#if PHASEJITTER
    cqSymbol = V34_Rotator(&cqSymbol, pJT->qS, pJT->qC);
#endif

    dbits = V34_Symbol_To_Bits(cqSymbol, &Train_sym, V34_4_POINT_TRN); /* always 4 points in Phase3 */

    CQSUB(Train_sym, cqSymbol, Errsym);

    if (pAGC->freeze_EQ == 0)
    {
        V34_2T3_Equalizer_Update(Errsym, qBeta, pTCR);
    }

    V34_TimingLoop(pTCR, &cqSymbol, &Train_sym);

#if PHASEJITTER
    pJT->counter++;
    V34_jt_loop(pJT, &cqSymbol, &Train_sym, 1);
#endif
    V34_CarrierLoop(pTCR, &cqSymbol, &Train_sym);

    V34_DIFF_DECODE(DeCode, dbits, diffbits);

    bit_stream[0] = diffbits & 0x1;
    bit_stream[1] = (diffbits >> 1) & 0x1;

    (*pRx->pfDescram_bit)(bit_stream, &(pRx->dsc), pRx->pDscram_buf, 2);
}


void V34_Phase3_Rec_TRN_EQ_TC(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase3_Info *p3 = &(pV34->p3);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;

    if (p3->detect_symbol_counter <= 1000)
    {
        pRx->qEq_Beta = V34_qBETA_TRN_FINE;
    }
    else if (p3->detect_symbol_counter == 2000)
    {
        pRx->qEq_Beta   = V34_qBETA_J;

        pTCR->qCarAcoef = 32368;
        pTCR->qCarBcoef = 400;
        pTCR->qB3 = 100;
    }

    V34_P3_RxBasic(pV34, pRx->qEq_Beta);

    pV34Rx->Num_Trn_Tx = V34_Detect_J_Sequence(pRx->pDscram_buf[0], pV34);

    if ((pV34Rx->Num_Trn_Tx == V34_4_POINT_TRN) || (pV34Rx->Num_Trn_Tx == V34_16_POINT_TRN) || (--p3->detect_symbol_counter <= 0))
    {
        p3->rx_vec_idx++;

        if (p3->detect_symbol_counter > 0)
        {
            switch (pV34Rx->rx_symbol_rate)
            {
                case V34_SYM_2400: p3->detect_symbol_counter = 1200; break;
                case V34_SYM_2743: p3->detect_symbol_counter = 1371; break;
                case V34_SYM_2800: p3->detect_symbol_counter = 1400; break;
                case V34_SYM_3000: p3->detect_symbol_counter = 1500; break;
                case V34_SYM_3200: p3->detect_symbol_counter = 1600; break;
                case V34_SYM_3429: p3->detect_symbol_counter = 1714; break;
            }
        }

        p3->detJ_yet = 0;
    }
}

/************************/
/* Detect J sequence    */
/************************/
void V34_Phase3_Det_J(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34StateStruc *pV34State = &pRx->V34State;
    Phase3_Info *p3 = &(pV34->p3);

    V34_P3_RxBasic(pV34, V34_qBETA_TRN_FINE);

    pV34Rx->Num_Trn_Tx = V34_Detect_J_Sequence(pRx->pDscram_buf[0], pV34);

    if (--p3->detect_symbol_counter <= 0)
    {
        if (p3->detJ_yet)
        {
            if ((pV34Rx->Num_Trn_Tx == V34_4_POINT_TRN) || (pV34Rx->Num_Trn_Tx == V34_16_POINT_TRN))
            {
                (p3->tx_vec_idx) ++; /* goto send S */
                (p3->rx_vec_idx) ++; /* goto dummy */
                V34_S_Detector_Init(&pRx->S_ToneDet);
            }
            else
            {
                p3->detJ_bit_count -= 2;

                if (p3->detJ_bit_count == 0)
                {
                    p3->detJ_yet = 0;
                }
            }
        }
        else
        {
            if (pV34Rx->Num_Trn_Tx == V34_4_POINT_TRN)
            {
                p3->detJ_yet = V34_4_POINT_TRN;
                p3->detJ_bit_count = 16;
            }
            else if (pV34Rx->Num_Trn_Tx == V34_16_POINT_TRN)
            {
                p3->detJ_yet = V34_16_POINT_TRN;
                p3->detJ_bit_count = 16;
            }
        }
    }

#if USE_ANS

    /* Answer modem detect J sequence time out test */
    if ((pV34Rx->modem_mode == ANS_MODEM) && (p3->Time_cnt_J_start) &&
        ((pV34->Time_cnt - p3->Time_cnt_J_start) > p3->pqTime_out_value[1]))
    {
        pRx->Retrain_flag         = RETRAIN_INIT;
        pV34State->retrain_reason = V34_DETECT_J_TIMEOUT;
    }
    else
#endif
        if ((pV34Rx->modem_mode == CALL_MODEM) && (pV34->Time_cnt > p3->pqTime_out_value[0]))
        {
            pRx->Retrain_flag         = RETRAIN_INIT;
            pV34State->retrain_reason = V34_DETECT_J_TIMEOUT;
        }
}

void V34_Phase3_Det_S_Call(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    V34StateStruc *pV34State = &pRx->V34State;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase3_Info *p3 = &(pV34->p3);
    Phase4_Info *p4 = &(pV34->p4);
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    CQWORD cqSymbol;
    UBYTE Det_S;
    UBYTE i;

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
    pRx->cqEQ_symbol = cqSymbol;

    Det_S = V34_S_Detector(pRx, pV34Rx->tTimingOut);

    if (Det_S)
    {
        p4->detect_symbol_counter = 0;

        pRx->S_trn_point = 0;

        V34_Cleanup_Equalizer(&pRx->S_ToneDet.EQ_S);

#if PHASEJITTER
        V34_jt_loop_init(pJT, 0);
#endif
        V34_CarrierLoop_Init(pTCR);

        V34_TimingLoop_Init(pTCR);

        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = S_HLK_TAB[i];
        }

        TRACE0("S_HLK_TAB");

        p3->rx_vec_idx ++;  /* goto Phase3_Dummy_Call */
        p3->result_code = 1; /* Phase 3 is DONE, indicate to goto Phase 4 */

        /* Restore trained AGC gain after 2nd S detection */
        pAGC->qGain = pAGC->freeze;

        TRACE1("pAGC->qGain=%d", pAGC->qGain);
        pAGC->freeze = 0;
    }

    if ((p4->Time_cnt_SSBar_start) && ((pV34->Time_cnt - p4->Time_cnt_SSBar_start) > p3->pqTime_out_value[2]))
    {
        pRx->Retrain_flag         = RETRAIN_INIT;
        pV34State->retrain_reason = V34_PH4_DETECT_SSBAR_TIMEOUT;
    }
}

void V34_Phase3_Dummy_Call(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase4_Info *p4 = &(pV34->p4);

    V34_Detect_S_SBAR(pRx, pV34Rx->tTimingOut, &(p4->detect_symbol_counter));
}


void V34_Phase3_Rx_Dummy(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;

    /* Ensure continous data flow into Equalizer */
    DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);

    pRx->cqEQ_symbol = pV34Rx->tTimingOut[1];
}

void V34_Phase3_Rec_J_Setup(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);

    V34_P3_RxBasic(pV34, V34_qBETA_TRN_FINE);

    ++p3->rx_vec_idx;
    p3->result_code = 1;                    /* End of Phase 3 */
}


void V34_Phase3_Rec_J_Dummy(V34Struct *pV34)
{
    V34_P3_RxBasic(pV34, V34_qBETA_TRN_FINE);
}

void V34_Phase3_Wait_MD(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase3_Info *p3 = &(pV34->p3);

    if (--p3->detect_symbol_counter <= 0)
    {
        pRx->S_trn_point = 0;

        /* prepare for S tone detect */
        V34_S_Detector_Init(&pRx->S_ToneDet);

        pV34Rx->md_length = 0;

        if (pV34Rx->modem_mode == CALL_MODEM)
        {
            p3->rx_vec_idx = 0;
        }

#if USE_ANS
        else if (pV34Rx->modem_mode == ANS_MODEM)
        {
            p3->rx_vec_idx = 1;
        }

#endif
    }
}
#endif
