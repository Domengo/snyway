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

/* ------------------------------------------------------------------------- */
/* Rate renegotiation routine                                              */
/* ------------------------------------------------------------------------- */

#include "v34ext.h"
#include "ptable.h"
#include "mhsp.h"
#include "hwapi.h"

#if 0 /// LLL temp
void  V34_Phase4_Renego_Init(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    ShellMapStruct *pShellMap = &(pV34->V34Tx.ShellMap);
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
    UBYTE i;

    pV34->Time_cnt = 0;
    p4->Time_cnt_E_start = 0;
    p4->tx_vec_idx = 0;
    p4->rx_vec_idx = 0;
    pAGC->freeze = 1;

    /* Set detect_symbol_counter to 128T, send_symbol_counter */
    p4->detect_symbol_counter = 0;
    p4->send_symbol_counter = 0;

    /* Current sending index of the MP bit buffer */
    p4->mp_idx = 0;
    pShellMap->scram_idx = 8;
    p4->mp_scram_size = 0;
    p4->mp_size = V34_MP_1_SIZE;

    pMpTx->type = MP_TYPE_1;

    pMpTx->aux_channel_select = 0;
    pMpTx->TrellisEncodSelect = 0; /* 0 is 16-state */
    /*   pMpTx->NonlinearEncodParam = 0; */
    pMpTx->const_shap_select_bit = 1;
    pMpTx->data_rate_cap_mask = ((UWORD)1 << V34_BIT_33600) - 2;

    /* Always use 4-Point in Rate renegotiation or Clear down */
    pV34Rx->Num_Trn_Tx     = V34_4_POINT_TRN;
    pV34Rx->Num_Trn_Rx     = V34_4_POINT_TRN;
    pV34Rx->rx_symbol_bits = V34_RX_SYM_BIT_4POINT;

    /* If clear down, set bit rate support to 0 */
    if (pRx->Renego.clear_down)
    {
        pMpTx->max_call_to_ans_rate = 0;
        pMpTx->max_ans_to_call_rate = 0;
        pMpTx->acknowledge_bit = 0;

        V34_Create_Mp(p4->pMp_buf, pMpTx);

        pMpTx->acknowledge_bit = 1;

        V34_Create_Mp(p4->pMp_buf_pi, pMpTx);
    }

#if 0 /* Create in ARS now */
    /* Else, regenerate the MP Info buffer */
    else
    {
        pMpTx->acknowledge_bit = 0;

        V34_Create_Mp(p4->pMp_buf, pMpTx);

        pMpTx->acknowledge_bit = 1;

        V34_Create_Mp(p4->pMp_buf_pi, pMpTx);
    }

#endif
    /* Reset the long word that used to store the E */
    p4->curr_word = 0;

    p4->S_point = 0;
    p4->S_point_BAR = 2;

    /* new init starts here */
    /* initialize the vector for transmit */
    i = 0;
    p4->pfTx[i++] = V34_Phase4_Send_S;
    p4->pfTx[i++] = V34_Phase4_Send_S_Bar;

    if (pRx->Renego.clear_down == 0)
    {
        p4->pfTx[i++] = V34_Phase4_Send_TRN;             /* Tx: State 2 */
        p4->pfTx[i++] = V34_Phase4_Send_TRN_2000MS;
    }

    p4->pfTx[i++] = V34_Phase4_Send_MP;
    p4->pfTx[i++] = V34_Phase4_Finish_MP;
    p4->pfTx[i++] = V34_Phase4_Send_MP;
    p4->pfTx[i++] = V34_Phase4_Finish_MP_Pi;
    p4->pfTx[i++] = V34_Phase4_Flush_MP_Pi;

    if (pRx->Renego.clear_down)
    {
        p4->pfTx[i++] = V34_Finish_Clear_Down;
    }
    else
    {
        p4->pfTx[i++] = V34_Phase4_Prepare_E;
        p4->pfTx[i++] = V34_Phase4_Send_E;
        p4->pfTx[i++] = V34_Phase4_Send_B1_Frame;
        p4->pfTx[i++] = V34_Begin_Data_Transfer;
        p4->pfTx[i++] = V34_Rate_Renego_Init;        /* for rate renegotiation  state 13 */
    }

    i = 0;

    /* Default to Clear down detection */
    if (pRx->Renego.clear_down || pRx->Renego.renego_generate)
    {
        p4->pfRx[i++] = V34_Phase4_VA_Delay;
    }

    p4->pfRx[i++] = V34_Clrdwn_Det_S;

    if (pRx->Renego.clear_down == 0)
    {
        p4->pfRx[i++] = V34_Clrdwn_Training;
    }

    p4->pfRx[i++] = V34_Clrdwn_Det_MP;
    p4->pfRx[i++] = V34_Clrdwn_Det_MP_PI;
    p4->pfRx[i++] = V34_Clrdwn_wait;
    p4->pfRx[i++] = V34_Dummy;
    p4->pfRx[i++] = V34_Dummy;
    p4->pfRx[i++] = V34_Dummy;
    p4->pfRx[i++] = V34_Dummy;
    p4->pfRx[i++] = V34_Dummy;

    pTCR->qCarAcoef = q097;
    pTCR->qCarBcoef = q003;
    pTCR->qB3       = 192;
}


/*****************************************/
/* Rate Renegotiation init                */
/*****************************************/
void V34_Rate_Renego_Init(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    VA_INFO        *dec = &(DeCode->Vdec);    /* VA INFO structure */
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info    *p4 = &(pV34->p4);
    RenegoStruc *pRN = &(pRx->Renego);
    CQWORD cqOut_sym;
    DpcsStruct *pDpcs = (DpcsStruct *)(pV34->pTable[DPCS_STRUC_IDX]);

    PutReg(pDpcs->MSR0, RRATE_DET);

    V34_Phase4_Renego_Init(pV34);

    pRN->S_DetCounter = 0;

    V34_Precoding_DataInit(&(pV34Tx->tx_precode));
    V34_Precoding_DataInit(&(pRx->rx_precode));
    V34_Precoding_DataInit(&(pRx->VA_precode));
    V34_Shell_Mapper_Init(pShellMap);
    V34_RXShell_Mapper_Init(&pRx->RxShellMap);
    V34_VA_Init(dec);

    /* --------   Initial for transmit and receive symbol buffer -------- */
    pV34Tx->tx_sym_inidx  = 0;
    pV34Tx->tx_sym_outidx = 0;
    pRx->rx_sym_inidx = 0;
    pRx->rx_sym_outidx = 0;
    pRx->rx_sym_count = 0;

    /* ------------   Initial for Convolution encoder state ------------- */
    pV34Tx->tx_COV_state = 0;

    p4->Time_cnt_SSBar_start = pV34->Time_cnt;

    p4->TimeOut = 2000 + 4 * pV34->fecount;

    p4->tx_vec_idx = 0;
    p4->rx_vec_idx = 0;
    pRx->S_trn_point = 0;

    /* send one S tone */
    cqOut_sym = V34_Create_S_or_SBar(&(p4->S_point));

    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOut_sym, (CQWORD *)&pEc->pcBulk_delay);

    ++p4->send_symbol_counter;
}
#endif

void V34_Finish_Clear_Down(V34Struct *pV34)
{
    V34RxStruct   *pV34Rx    = &(pV34->V34Rx);
    ReceiveStruc  *pRx       = &pV34Rx->RecStruc;
    V34StateStruc *pV34State = &pRx->V34State;

    pV34State->hang_up = 1;
    pV34->clear_down   = 0; /* clear down finish */
}


void V34_Dummy(V34Struct *pV34)
{
}
