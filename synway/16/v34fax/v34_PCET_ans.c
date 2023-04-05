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

#if SUPPORT_V34FAX

#define EQ_TRAIN_ERROR          (0)    /* t1 , t2 */

void V34Fax_PCET_Det_S(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase3_Info *p3 = &(pV34->p3);
    UBYTE  Det_S;
    UBYTE  i;

    DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);

    Det_S = V34_S_Detector(pRx, pV34Rx->tTimingOut);

    if (Det_S)
    {
        TRACE0("S detected at PCET");

        p3->rx_vec_idx++;

        p3->detect_symbol_counter = 0;

        pRx->S_trn_point = 0;

        //pRx->trn_count = 0;
        //pRx->qTrn_avg = 0;

        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = S_HLK_TAB[i];
        }

        p3->tx_vec_idx++;
    }
}


void V34Fax_PCET_Det_S_bar(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    QWORD qTemp;
    QWORD *pqSptr = &(pRx->qEchoCancel_Out[0]);
    UBYTE i;

    DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);

    //V34_Det_S_bar_Sub(pRx);

    if (V34_Detect_S_SBAR(pRx, pV34Rx->tTimingOut, &(p3->detect_symbol_counter)))
    {
        p3->rx_vec_idx++;
        p3->detect_symbol_counter = 16;

        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = PP_HLK_TAB[i];
        }

        p3->Time_cnt_J_start = pV34->Time_cnt;

        TRACE0("S_bar detected");
    }
}


void V34Fax_PCET_Wait_S_bar(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc  *pTCR = &pRx->TCR;
    CQWORD cqSymbol;

    V34_Agc(pAGC, pV34Rx->tTimingOut);

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);

    pRx->cqEQ_symbol = cqSymbol;

    --p3->detect_symbol_counter;

    if (p3->detect_symbol_counter <= 0)
    {
        p3->rx_vec_idx++ ;

        p3->detect_symbol_counter = pV34Fax->V34FAX_EQ_Delay;

        //p3->DelayIndex = 0;
        //p3->EqDelayCount = 0;

        p3->K = 0;
        p3->I = 0;
    }
}

void V34Fax_PCET_EQ_Delay(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);
    CQWORD Train_sym;

    Train_sym.r = 0;
    Train_sym.i = 0;

    V34Fax_EQ_Train(P3_RXFUNC_EQ1, pV34, Train_sym, 0);

    if (--p3->detect_symbol_counter <= 0)
    {
        p3->rx_vec_idx++;
        p3->detect_symbol_counter = 288;
        p3->I = 0;
        p3->K = 0;
    }
}


void V34Fax_PCET_PP_EQ(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    CQWORD Train_sym;
    UBYTE  i;

    /* Create PP symbol pattern for training EQ */
    if (p3->I > 3)
    {
        p3->I = 0;
        p3->K ++;
    }

    Train_sym = V34_Create_PP(p3->K, p3->I);
    
    p3->I++;

    V34Fax_EQ_Train(P3_RXFUNC_PP_EQ, pV34, Train_sym, V34_qBETA_PP);

    if (--p3->detect_symbol_counter <= 0)
    {
        p3->rx_vec_idx++;
        p3->detect_symbol_counter = 512;

        pV34Rx->train_sc = 0;/* Initial scrambler */

        pV34Rx->train_scram_idx = 16;
        p3->pByte_stream[0] = 0xFF;
        p3->pByte_stream[1] = 0xFF;

        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = TRN_HLK_TAB[i];
        }
    }
}


void V34Fax_PCET_TRN_EQ(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info *p3 = &(pV34->p3);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD Train_sym;
    UWORD  uTemp4, uTemp16;
    QWORD  qTemp;
    QWORD  qEqBeta;
    UBYTE  dbits;

    if (pV34Rx->train_scram_idx >= 16)
    {
        (*pV34Rx->pfScramTrn_byte)(p3->pByte_stream, &(pV34Rx->train_sc), pV34Rx->pTrain_scram_buf, 2);

        pV34Rx->train_scram_idx = 0;
    }

    Train_sym = V34_Create_TRN(pV34Rx->pTrain_scram_buf, &(pV34Rx->train_scram_idx), pV34Rx->Num_Trn_Rx);

    if (p3->detect_symbol_counter < 512 && p3->detect_symbol_counter > 500)
    {
        if (pRx->cqEQ_symbol.r > 0)
        {
            qTemp = pRx->cqEQ_symbol.r - 7470;
        }
        else
        {
            qTemp = pRx->cqEQ_symbol.r + 7470;
        }

        uTemp4 = QQMULQR15(qTemp, qTemp);

        if (pRx->cqEQ_symbol.i > 0)
        {
            qTemp = pRx->cqEQ_symbol.i - 7470;
        }
        else
        {
            qTemp = pRx->cqEQ_symbol.i + 7470;
        }

        uTemp4 += QQMULQR15(qTemp, qTemp);

        if (pRx->cqEQ_symbol.r > (QWORD)6694)
        {
            qTemp = pRx->cqEQ_symbol.r - 10040;
        }
        else if (pRx->cqEQ_symbol.r > 0)
        {
            qTemp = pRx->cqEQ_symbol.r - 3347;
        }
        else if (pRx->cqEQ_symbol.r > -6694)
        {
            qTemp = pRx->cqEQ_symbol.r + 3347;
        }
        else
        {
            qTemp = pRx->cqEQ_symbol.r + 10040;
        }

        uTemp16 = QQMULQR15(qTemp, qTemp);

        if (pRx->cqEQ_symbol.i > (QWORD)6694)
        {
            qTemp = pRx->cqEQ_symbol.i - 10040;
        }
        else if (pRx->cqEQ_symbol.i > 0)
        {
            qTemp = pRx->cqEQ_symbol.i - 3347;
        }
        else if (pRx->cqEQ_symbol.i > -6694)
        {
            qTemp = pRx->cqEQ_symbol.i + 3347;
        }
        else
        {
            qTemp = pRx->cqEQ_symbol.i + 10040;
        }

        uTemp16 += QQMULQR15(qTemp, qTemp);

        pV34Fax->err_min_dist4  += uTemp4;
        pV34Fax->err_min_dist16 += uTemp16;

        if (p3->detect_symbol_counter == 501)
        {
            if (pV34Fax->err_min_dist16 < pV34Fax->err_min_dist4)
            {
                pV34Rx->Num_Trn_Rx     = V34_16_POINT_TRN;
                pV34Rx->Num_Trn_Tx     = V34_16_POINT_TRN;
                pV34Rx->J_point        = V34_16_POINT_J;
                pV34Rx->rx_symbol_bits = V34_RX_SYM_BIT_16POINT;
            }
            else
            {
                pV34Rx->Num_Trn_Rx     = V34_4_POINT_TRN;
                pV34Rx->Num_Trn_Tx     = V34_4_POINT_TRN;
                pV34Rx->J_point        = V34_4_POINT_J;
                pV34Rx->rx_symbol_bits = V34_RX_SYM_BIT_4POINT;
            }

            pV34Fax->err_min_dist4  = 0;
            pV34Fax->err_min_dist16 = 0;
        }
    }

    if (pV34Fax->EQ_Mode == 1)
    {
        qEqBeta = 4000;
    }
    else
    {
        qEqBeta = 12000;
    }

    V34Fax_EQ_Train(P3_RXFUNC_TRN_EQ, pV34, Train_sym, qEqBeta);

    Train_sym.r -= pRx->cqEQ_symbol.r;
    Train_sym.i -= pRx->cqEQ_symbol.i;

#if 0
    if (p3->detect_symbol_counter < (1024 + 16))
    {
        V34_ARS_Calc(pRx, &Train_sym);
    }

    if (p3->detect_symbol_counter == 15)
    {
        V34_ARS_est(pV34);
    }
#endif

    if (p3->detect_symbol_counter <= 0)
    {
        p3->rx_vec_idx++;

        /* LENGTH of TRN is 35ms*INFOhValue(bits15:21)    */
        /* This function takes up first 512symbols of TRN */
        /* Leftover Symbols:                              */
        /* 35*64*(symbolrate/1000) - 512 */

        switch (pV34Fax->symbol_rate)
        {
            case V34_SYM_2400:
                p3->detect_symbol_counter = 4864 - 60;
                break;
            case V34_SYM_2743:
                p3->detect_symbol_counter = 5630 - 60;
                break;
            case V34_SYM_2800:
                p3->detect_symbol_counter = 5760 - 60;
                break;
            case V34_SYM_3000:
                p3->detect_symbol_counter = 6208 - 60;
                break;
            case V34_SYM_3200:
                p3->detect_symbol_counter = 6656 - 60;
                break;
            case V34_SYM_3429:
                p3->detect_symbol_counter = 7168 - 60;
                break;
        }

        pV34Rx->sequence = 0;

        dbits = V34_Symbol_To_Bits(pRx->cqEQ_symbol, &Train_sym, pV34Rx->Num_Trn_Rx);

        DeCode->dZ = dbits;/* V34_DIFF_DECODE_INIT */

        pTCR->qCarAcoef = q095;
        pTCR->qCarBcoef = q005;

        pV34Fax->timAdjust = 0;
        pV34Fax->timAdjustCount = 0;
    }
}


void V34Fax_PCET_TRN_EQ_TC(V34Struct *pV34)
{
    V34FaxStruct    *pV34Fax = &(pV34->V34Fax);
    Phase3_Info     *p3      = &(pV34->p3);
    V34RxStruct     *pV34Rx  = &(pV34->V34Rx);
    ReceiveStruc    *pRx     = &pV34Rx->RecStruc;
    V34AgcStruc     *pAGC    = &(pRx->rx_AGC);
    DeModulateStruc *pDeMod  = &pRx->DeMod;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD cqSymbol;
    CQWORD Train_sym;
    CQWORD Errsym;
    QDWORD qdTemp;
    UWORD  i;
    QWORD  qBeta = 0;

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

    pRx->cqEQ_symbol = cqSymbol;

#if PHASEJITTER
    cqSymbol = V34_Rotator(&cqSymbol, pRx->jtvar.qS, pRx->jtvar.qC);
#endif

    V34_Symbol_To_Bits(cqSymbol, &Train_sym, pV34Rx->Num_Trn_Rx);/* always 4 points in Phase3 */

    CQSUB(Train_sym, cqSymbol, Errsym);

#if RX_PRECODE_ENABLE
    V34_NoiseWhiten(&pV34->NoiseWhiten, &Errsym);
#endif

#if EQ_TRAIN_ERROR
    if (DumpTone4_Idx < 50000) { DumpTone4[DumpTone4_Idx++] = Errsym.r; }
#endif

    if (pV34Fax->EQ_Mode == 0)
    {
        if (pV34Fax->FreqOffset_Enable == 0)
        {
            if (p3->detect_symbol_counter < 1500)
            {
                qBeta = 500;
            }
            else if (pV34Fax->timAdjustCount < 1200)
            {
                qBeta = 5000;
            }
            else
            {
                qBeta = 1000;
            }
        }
        else
        {
            if (p3->detect_symbol_counter > 3000)
            {
                qBeta = 2000;
            }
            else if (p3->detect_symbol_counter > 2000)
            {
                qBeta = 1200;
            }
            else
            {
                qBeta = 500;
            }
        }
    }
    else if (pV34Fax->EQ_Mode == 1)
    {
        if (p3->detect_symbol_counter > 3000)
        {
            qBeta = 4000;
        }
        else if (p3->detect_symbol_counter > 2000)
        {
            qBeta = 2200;
        }
        else
        {
            qBeta = 500;
        }
    }

    V34_2T3_Equalizer_Update(Errsym, qBeta, pTCR);

    V34Fax_TimingLoop(pV34Fax, &cqSymbol, &Train_sym);

    if (pV34Fax->FreqOffset_Enable == 0)
    {
        if (pV34Fax->timAdjustCount == 1200)
        {
            pV34Fax->udCarPhase = pTCR->udError_phase_out;
            pV34Fax->timAdjust  = 0;
        }
        else if (pV34Fax->timAdjustCount == 4200)
        {
            /* Save Timing Offset Estimation */
            if (pV34Fax->timAdjust != 0)
            {
                if (pV34Fax->timAdjust > 0)
                {
                    pV34Fax->drift_direction = 1;

                    pV34Fax->drift_thresh = 24000 / pV34Fax->timAdjust;
                }
                else
                {
                    pV34Fax->drift_direction = -1;

                    pV34Fax->drift_thresh = -24000 / pV34Fax->timAdjust;
                }
            }
            else if (pV34Fax->timAdjust == 0)
            {
                pV34Fax->drift_direction = 0;

                pV34Fax->drift_thresh = 32767;
            }

            /* Save Carrier Offset Estimation */
            /* 466 = ((32767*256)/(65536*3000*3)) << 15) */
            qdTemp = (QDWORD)(pTCR->udError_phase_out >> 15) - (QDWORD)(pV34Fax->udCarPhase >> 15);

            if (qdTemp > 32767)
            {
                qdTemp -= 65536;
            }
            else if (qdTemp < (-32767))
            {
                qdTemp += 65536;
            }

            pV34Fax->carAdjust = (SWORD)((qdTemp * 466) >> 15);
        }
    }

#if PHASEJITTER
    pRx->jtvar.counter++;

    V34_jt_loop(&pRx->jtvar, &cqSymbol, &Train_sym, 1);
#endif

    V34_CarrierLoop(pTCR, &cqSymbol, &Train_sym);

    if (p3->detect_symbol_counter < (1024 + 16))
    {
        V34_ARS_Calc(pRx, &Errsym);
    }

    if (p3->detect_symbol_counter == 15)
    {
        V34_ARS_est(pV34);
    }

    if (p3->detect_symbol_counter == 10)
    {
        for (i = 0; i < pV34Fax->V34fax_EQ_Length; i++)
        {
            pV34Fax->pcCoef[i] = pTCR->eqfir.pcCoef[i];
        }

        pV34Fax->qGain = pAGC->qGain;

        if (pV34Fax->FreqOffset_Enable == 0)
        {
            pV34Fax->rx_carrier_idx = pDeMod->rx_carrier_idx;

            pV34Fax->qRx_carrier_offset_idx = pDeMod->qRx_carrier_offset_idx;
        }

        pV34Fax->tx_sym_cnt = 0;
    }

    p3->detect_symbol_counter --;

    if (p3->detect_symbol_counter < 15)
    {
        V34Fax_CarrierLoss_Detect(pV34);
    }

#if RX_PRECODE_ENABLE
    if (p3->detect_symbol_counter == 3000)
    {
        pV34->NoiseWhiten.ubBetaShift = 13;
    }
    else if (p3->detect_symbol_counter == 1500)
    {
        pV34->NoiseWhiten.ubBetaShift = 15;
    }
#endif
}


void V34Fax_CarrierLoss_Detect(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34TxStruct  *pV34Tx  = &(pV34->V34Tx);
    UBYTE  i;
    QDWORD egy = 0;

    for (i = 0; i < 3; i++)
    {
        egy += QQMULQD(pV34Tx->PCMinPtr[i], pV34Tx->PCMinPtr[i]);

#if 0
        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV34Tx->PCMinPtr[i]; }
#endif
    }

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV34Fax->Silence_Egy; }
    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV34Fax->Silence_Egy_Ref; }
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV34Fax->Silence_Detect_Count; }
#endif

    if (pV34Fax->Silence_Count1 <= 8) /* Setup Egy threshold */
    {
        pV34Fax->Silence_Count1 ++;

        pV34Fax->Silence_Egy_Ref += (egy >> 6);
    }
    else
    {
        pV34Fax->Silence_Count2 ++;

        pV34Fax->Silence_Egy += (egy >> 4);

        if (pV34Fax->Silence_Count2 == 8)
        {
            if (pV34Fax->Silence_Egy < pV34Fax->Silence_Egy_Ref)
            {
                pV34Fax->Silence_Detect_Count ++;    /* Silence Counter */
            }
            else
            {
                pV34Fax->Silence_Detect_Count = 0;
            }

            pV34Fax->Silence_Count2 = 0;
            pV34Fax->Silence_Egy    = 0;
        }
    }
}

#if 0
void V34Fax_PCET_Rec_Silence(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);

    p3->detect_symbol_counter--;

    if ((pRx->cqEQ_symbol.r < 2500) && (pRx->cqEQ_symbol.r > -2500)
        && (pRx->cqEQ_symbol.i < 2500) && (pRx->cqEQ_symbol.i > -2500))
    {
        pV34Fax->PCET_silence_cnt++;
    }
    else
    {
        pV34Fax->PCET_silence_cnt = 0;
    }

    if (pV34Fax->PCET_silence_cnt >= 10)
    {
        pV34Fax->PCET_silence_cnt = 0;

        pV34Fax->PCET_RXend_Flag = 1;

        p3->detect_symbol_counter = 0;
    }
}
#endif

void V34Fax_EQ_Train(UWORD FuncCode, V34Struct *pV34, CQWORD Train_sym, QWORD qBeta)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase3_Info *p3 = &(pV34->p3);
    CQWORD      cqSymbol;
    CQWORD      Errsym;

    V34_Agc(pAGC, pV34Rx->tTimingOut);

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

    pRx->cqEQ_symbol = cqSymbol;

    if (FuncCode != P3_RXFUNC_EQ1)
    {
        CQSUB(Train_sym, cqSymbol, Errsym);

#if RX_PRECODE_ENABLE
        V34_NoiseWhiten(&pV34->NoiseWhiten, &Errsym);
#endif

#if EQ_TRAIN_ERROR
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = Errsym.r; }
        if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = Errsym.i; }
#endif

        if (pAGC->freeze_EQ == 0)
        {
            V34_2T3_Equalizer_Update(Errsym, qBeta, pTCR);
        }
    }
#if 0
    else if ((FuncCode == P3_RXFUNC_EQ1) && (++p3->EqDelayCount >= 22))
    {
        p3->EqDelayCount = 22;

        p3->DelayIndex += 3;

        if (p3->DelayIndex >= 84)
        {
            p3->DelayIndex = 0;
        }
    }
#endif

    if (FuncCode != P3_RXFUNC_TRN_EQ)
    {
        V34_Shift_Buf(pTCR->pcBaud_adjust, 3, cqSymbol);
        pTCR->pcT3_delay[0] = pTCR->pcT3_delay[1];
        pTCR->pcT3_delay[1] = Train_sym;
    }
    else if (FuncCode == P3_RXFUNC_TRN_EQ)
    {
        p3->detect_symbol_counter--;

        if (p3->detect_symbol_counter < 256)
        {
            V34Fax_TimingLoop(pV34Fax, &cqSymbol, &Train_sym);
            V34_CarrierLoop(pTCR, &cqSymbol, &Train_sym);
        }
    }
}

#endif
