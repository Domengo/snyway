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

#include "v34ext.h"

#if 0 /// LLL temp
UBYTE V34_Rcv_Common(V34RxStruct *pV34Rx, CQWORD *pcSymbol, CQWORD *pcTrain_sym, CQWORD *pcErrsym,
                     UBYTE *pDbits, UBYTE update)
{
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD cqDsym[3];
    CQWORD cqTimingOut[3];
    QWORD qEQ_Beta;

    V34_Demodulate(pRx, cqDsym);

    V34_Timing_Rec(pTCR, cqDsym, cqTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return 0;
    }

    *pcSymbol = DspcFir_2T3EQ(&pTCR->eqfir, cqTimingOut);
    *pcSymbol = V34_Rotator(pcSymbol, pTCR->qS, pTCR->qC);

#if DRAWEPG
    /************* For Vxd EPG debug ********/
    pRx->pEpg[0].r = (pcSymbol->r);
    pRx->pEpg[0].i = (pcSymbol->i);
    /**************** End of EPG ************/
#endif

    *pDbits = V34_Symbol_To_Bits(*pcSymbol, pcTrain_sym, V34_4_POINT_TRN); /* always 4 points for cleardown */

    if (update)
    {
        if (update == 1)
        {
            qEQ_Beta = 1500;
        }
        else
        {
            qEQ_Beta = 350;
        }

        CQSUB((*pcTrain_sym), (*pcSymbol), (*pcErrsym));

        if (pAGC->freeze_EQ == 0)
        {
            V34_2T3_Equalizer_Update(*pcErrsym, qEQ_Beta, pTCR);
        }

        V34_TimingLoop(pTCR, pcSymbol, pcTrain_sym);
        V34_CarrierLoop(pTCR, pcSymbol, pcTrain_sym);
    }

    return 1;
}


void V34_Clrdwn_Det_S(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    CQWORD cqSymbol, cqTrain_sym, cqErrsym;
    UBYTE dbits, det_sbar;
    UWORD quadsym;
    Phase4_Info *p4 = &(pV34->p4);
    RenegoStruc *pRN = &(pRx->Renego);
    UBYTE i;

    if (V34_Rcv_Common(pV34Rx, &cqSymbol, &cqTrain_sym, &cqErrsym, &dbits, 1) == 0)
    {
        return;
    }

    /* Detect S-Sbar Transition */
    quadsym = 0;
    det_sbar = 0;

    if (cqTrain_sym.r < 0)
    {
        quadsym |= 0x01;
    }

    if (cqTrain_sym.i < 0)
    {
        quadsym |= 0x02;
    }

    if ((quadsym == pRN->quadsym2) && (quadsym != pRN->quadsym1))
    {
        ++pRN->S_DetCounter;
    }
    else if ((quadsym != pRN->quadsym2) && (pRN->S_DetCounter > 4))
    {
        det_sbar = 1;
    }
    else
    {
        pRN->S_DetCounter = 0;
    }

    pRN->quadsym2 = pRN->quadsym1;
    pRN->quadsym1 = quadsym;

    if (det_sbar)
    {
        ++p4->rx_vec_idx;

        pAGC->freeze = 0;

        for (i = 0; i < 6; i++)
        {
            pAGC->qHLK[i] = TRN_HLK_TAB[i];
        }

        if (pRx->Renego.clear_down == 0)
        {
            switch (pV34Rx->tx_symbol_rate)
            {
                case V34_SYM_2400: p4->detect_symbol_counter = 2250; break;
                case V34_SYM_2743: p4->detect_symbol_counter = 2571; break;
                case V34_SYM_2800: p4->detect_symbol_counter = 2625; break;
                case V34_SYM_3000: p4->detect_symbol_counter = 2812; break;
                case V34_SYM_3200: p4->detect_symbol_counter = 3000; break;
                case V34_SYM_3429: p4->detect_symbol_counter = 3215; break;
            }
        }

        pRN->S_DetCounter   = 0;

        pRx->qdARS_erregy   = 0;
        p4->Time_cnt_E_start = pV34->Time_cnt;
    }
}


void V34_Clrdwn_Training(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    TimeCarrRecovStruc  *pTCR = &pRx->TCR;
    CQWORD cqSymbol, cqTrain_sym, cqErrsym;
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpRx = &p4->MpRx;
    UBYTE i, ubTemp;
    UBYTE dbits;
    UBYTE diffbits;
    UBYTE pBit_stream[4];

    if (V34_Rcv_Common(pV34Rx, &cqSymbol, &cqTrain_sym, &cqErrsym, &dbits, 2) == 0)
    {
        return;
    }

    V34_DIFF_DECODE(DeCode, (dbits & 0x3), diffbits);

    pBit_stream[0] = diffbits & 0x1;
    pBit_stream[1] = (diffbits >> 1) & 0x1;
    pBit_stream[2] = (dbits >> 2) & 0x1;
    pBit_stream[3] = (dbits >> 3) & 0x1;

    (*pRx->pfDescram_bit)(pBit_stream, &(pRx->dsc), pRx->pDscram_buf, V34_RX_SYM_BIT_4POINT); /* pV34->rx_symbol_bits always 2 for cleardown */

    ubTemp = pRx->pDscram_buf [0];

    pBit_stream[0] =  ubTemp & 0x1;
    pBit_stream[1] = (ubTemp >> 1) & 0x1;
    pBit_stream[2] = (ubTemp >> 2) & 0x1;
    pBit_stream[3] = (ubTemp >> 3) & 0x1;

    p4->result_code = 0;

    for (i = 0; i < V34_RX_SYM_BIT_4POINT; i++) /* pV34->rx_symbol_bits always 2 for cleardown */
    {
        p4->result_code |= V34_Detect_Mp(pBit_stream[i], pMpRx);
    }

    if (p4->result_code)
    {
        if ((pMpRx->max_call_to_ans_rate == 0) && (pMpRx->max_ans_to_call_rate == 0))
        {
            p4->send_symbol_counter = 0;

            /* Prepare to receive MP */
            (p4->rx_vec_idx)++;

            pAGC->freeze = 0;
        }
    }

    p4->detect_symbol_counter--;

    if ((p4->detect_symbol_counter) < 1024 + 16)
    {
        V34_ARS_Calc(pRx, &cqErrsym);
        pTCR->qCarAcoef = 32368;
        pTCR->qCarBcoef = 400;
        pTCR->qB3 = 100;
    }

    if (p4->detect_symbol_counter == 16)
    {
        p4->send_symbol_counter = 0;
    }

    if (p4->detect_symbol_counter <= 0)
    {
        /* Prepare to receive MP */
        (p4->rx_vec_idx)++;

        pAGC->freeze = 0;

        /* ----------------------- Calculate ARS ---------------------- */
#if SUPPORT_V54

        if (pV34->LAL_modem)
        {
            pRx->qRTRN_Req_Thres  = QDQMULQDR8(pRx->qdARS_erregy, 896);
        }
        else
#endif
            V34_ARS_est(pV34);

        /* ------------------------------------------------------------ */
    }
}

void V34_Clrdwn_Det_MP(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    V34StateStruc *pV34State = &pRx->V34State;
    CQWORD cqSymbol, cqTrain_sym, cqErrsym;
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
    MpStruc *pMpRx = &p4->MpRx;
    UBYTE i, ubTemp;
    UBYTE dbits;
    UBYTE diffbits;
    UBYTE pBit_stream[4];

    if (V34_Rcv_Common(pV34Rx, &cqSymbol, &cqTrain_sym, &cqErrsym, &dbits, 2) == 0)
    {
        return;
    }

    V34_DIFF_DECODE(DeCode, (dbits & 0x3), diffbits);

    pBit_stream[0] = diffbits & 0x1;
    pBit_stream[1] = (diffbits >> 1) & 0x1;
    pBit_stream[2] = (dbits >> 2) & 0x1;
    pBit_stream[3] = (dbits >> 3) & 0x1;

    (*pRx->pfDescram_bit)(pBit_stream, &(pRx->dsc), pRx->pDscram_buf, V34_RX_SYM_BIT_4POINT); /* pV34->rx_symbol_bits always 2 for cleardown */

    ubTemp = pRx->pDscram_buf[0];
    pBit_stream[0] =  ubTemp & 0x1;
    pBit_stream[1] = (ubTemp >> 1) & 0x1;
    pBit_stream[2] = (ubTemp >> 2) & 0x1;
    pBit_stream[3] = (ubTemp >> 3) & 0x1;

    p4->result_code = 0;

    for (i = 0; i < V34_RX_SYM_BIT_4POINT; i++) /* pV34->rx_symbol_bits always 2 for cleardown */
    {
        p4->result_code |= V34_Detect_Mp(pBit_stream[i], pMpRx);
    }

    if (p4->result_code == 1)
    {
        /* If detect clear_down request from the other side */
        /* Go ahead and do clear down since it is default.  */
        if ((pMpRx->max_call_to_ans_rate == 0) && (pMpRx->max_ans_to_call_rate == 0))
        {
            pMpTx->max_call_to_ans_rate = 0;
            pMpTx->max_ans_to_call_rate = 0;
            pMpTx->acknowledge_bit = 1;
            V34_Create_Mp(p4->pMp_buf_pi, pMpTx);

            ++p4->tx_vec_idx;
            ++p4->rx_vec_idx;
            i = p4->tx_vec_idx;

            p4->pfTx[i++] = V34_Phase4_Finish_MP;
            p4->pfTx[i++] = V34_Phase4_Send_MP;
            p4->pfTx[i++] = V34_Phase4_Finish_MP_Pi;
            p4->pfTx[i++] = V34_Phase4_Flush_MP_Pi;
            p4->pfTx[i++] = V34_Finish_Clear_Down;
        }
        /* Not clear down request from remote, but we       */
        /* request to do clear down, should go ahead too.   */
        else if (pRx->Renego.clear_down)
        {
            ++p4->tx_vec_idx;
            ++p4->rx_vec_idx;
        }
        /* Not the above two, Rate Renegotiation needed.     */
        /* Re-setup the Rx vectors to do renegotiation.      */
        else
        {
            i = p4->rx_vec_idx;
            p4->send_symbol_counter = 0;

            p4->pfRx[i++] = V34_Phase4_Rec_MP;
            p4->pfRx[i++] = V34_Phase4_Rec_MP_Pi;
            p4->pfRx[i++] = V34_Phase4_Detect_E;
            p4->pfRx[i++] = V34_Phase4_VA_Delay;
            p4->pfRx[i++] = V34_Phase4_Rec_B1_Frame;
            p4->pfRx[i++] = V34_Phase4_Rec_Superframe;

            pRx->Renego.renego = 0;
            pRx->Renego.clear_down = 0;
            pRx->Renego.renego_generate = 0;
        }
    }

    if (p4->Time_cnt_E_start && ((pV34->Time_cnt - p4->Time_cnt_E_start) > p4->pqTime_out_value[1]))
    {
        pRx->Retrain_flag = RETRAIN_INIT;
        pV34State->retrain_reason = V34_PH4_DETECT_E_TIMEOUT;
    }
}

void V34_Clrdwn_Det_MP_PI(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    CQWORD cqSymbol, cqTrain_sym, cqErrsym;
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpRx = &p4->MpRx;
    UBYTE i, ubTemp;
    UBYTE dbits;
    UBYTE diffbits;
    UBYTE pBit_stream[4];

    if (V34_Rcv_Common(pV34Rx, &cqSymbol, &cqTrain_sym, &cqErrsym, &dbits, 2) == 0)
    {
        return;
    }

    V34_DIFF_DECODE(DeCode, (dbits & 0x3), diffbits);

    pBit_stream[0] = diffbits & 0x1;
    pBit_stream[1] = (diffbits >> 1) & 0x1;
    pBit_stream[2] = (dbits >> 2) & 0x1;
    pBit_stream[3] = (dbits >> 3) & 0x1;

    (*pRx->pfDescram_bit)(pBit_stream, &(pRx->dsc), pRx->pDscram_buf, V34_RX_SYM_BIT_4POINT); /* pV34->rx_symbol_bits always 2 for cleardown */

    ubTemp = pRx->pDscram_buf[0];
    pBit_stream[0] =  ubTemp & 0x1;
    pBit_stream[1] = (ubTemp >> 1) & 0x1;
    pBit_stream[2] = (ubTemp >> 2) & 0x1;
    pBit_stream[3] = (ubTemp >> 3) & 0x1;

    p4->result_code = 0;

    for (i = 0; i < V34_RX_SYM_BIT_4POINT; i++) /* pV34->rx_symbol_bits always 2 for cleardown */
    {
        p4->result_code |= V34_Detect_Mp(pBit_stream[i], pMpRx);
    }

    if (p4->result_code == 1)
    {
        if (pMpRx->acknowledge_bit == 1)
        {
            /* Clear down mode */
            if ((pMpRx->max_call_to_ans_rate == 0) && (pMpRx->max_ans_to_call_rate == 0))
            {
                ++p4->rx_vec_idx;

                /* Wait for RTD before hangup, make sure remote modem */
                /* will receive the MP' clear down sequence.           */
                if (pRx->Renego.clear_down)
                {
                    p4->detect_symbol_counter = 2 * pV34->fecount + 16 * p4->mp_size;
                }
                else
                {
                    p4->detect_symbol_counter = 2 * pV34->fecount;
                }
            }
            else
            {
                ++p4->rx_vec_idx;
                /* Wait for RTD before hangup, make sure remote modem */
                /* will receive the MP' clear down sequence.           */
                p4->detect_symbol_counter = 2 * pV34->fecount + 16 * p4->mp_size;
            }
        }
    }
    else if (p4->result_code == -1) /* CRC WRONG */
    {
        ++p4->rx_vec_idx;
        /* Wait for RTD before hangup, make sure remote modem */
        /* will receive the MP' clear down sequence.           */
        p4->detect_symbol_counter = (pV34->fecount << 1) + (p4->mp_size << 4);
    }
}

void V34_Clrdwn_wait(V34Struct *pV34)
{
    if ((--pV34->p4.detect_symbol_counter) <= 0)
    {
        ++pV34->p4.tx_vec_idx;         /* Let's finish up MP' then hangup */
        ++pV34->p4.rx_vec_idx;         /* Receive dummy */
    }
}
#endif
