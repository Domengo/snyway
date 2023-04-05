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
CONST V34FnPtr V34P3TxFsmCall[] =
{
    V34_Phase3_Send_Silence,         /* Tx: State 0 */
    V34_Phase3_Send_S,           /* Tx: State 1 */
    V34_Phase3_Send_S_Bar,       /* Tx: State 2 */
    V34_Phase3_Send_PP_Call,     /* Tx: State 3 */
    V34_Phase3_Send_TRN,         /* Tx: State 4 */
    V34_Phase3_Send_J            /* Tx: State 5 */
};

CONST V34FnPtr V34P3RxFsmCall[] =
{
    V34_Phase3_Det_S,            /* Rx: State 0 */
    V34_Phase3_Rec_S_Det_S_Bar,  /* Rx: State 1 */
    V34_Phase3_Wait_S_Bar_16T,   /* Rx: State 2 */
    V34_Phase3_EQ_1,             /* Rx: State 3 */
    V34_Phase3_Rec_PP_EQ,        /* Rx: State 4 */
    V34_Phase3_Rec_TRN_EQ,       /* Rx: State 5 */
    V34_Phase3_Rec_TRN_EQ_TC,    /* Rx: State 6 */
    V34_Phase3_Det_J,            /* Rx: State 7 */
    V34_Phase3_Rx_Dummy,         /* Rx: State 8 */
    V34_Phase3_Det_S_Call,       /* Rx: State 9 */
    V34_Phase3_Dummy_Call,       /* Rx: State 10 */
    V34_Phase3_Wait_MD,          /* Rx: State 11 */
};

CONST V34FnPtr V34P3TxFsmAns[] =
{
    V34_Phase3_Send_Silence_70MS,/* Tx: State 0 */
    V34_Phase3_Send_S,           /* Tx: State 1 */
    V34_Phase3_Send_S_Bar,       /* Tx: State 2 */
    V34_Phase3_Send_PP,          /* Tx: State 3 */
    V34_Phase3_Send_TRN,         /* Tx: State 4 */
    V34_Phase3_Send_J,           /* Tx: State 5 */
    V34_Phase3_Mod_Silence,      /* Tx: State 6 */
    V34_Phase3_Mod_Silence,      /* Tx: State 7 */
    V34_Phase3_Mod_Silence       /* Tx: State 8 */
};

CONST V34FnPtr V34P3RxFsmAns[] =
{
    V34_Phase3_Rx_Dummy,         /* Rx: State 0 */
    V34_Phase3_Det_S,            /* Rx: State 1 */
    V34_Phase3_Rec_S_Det_S_Bar,  /* Rx: State 2 */
    V34_Phase3_Wait_S_Bar_16T,   /* Rx: State 3 */
    V34_Phase3_EQ_1,             /* Rx: State 4 */
    V34_Phase3_Rec_PP_EQ,        /* Rx: State 5 */
    V34_Phase3_Rec_TRN_EQ,       /* Rx: State 6 */
    V34_Phase3_Rec_TRN_EQ_TC,    /* Rx: State 7 */
    V34_Phase3_Det_J,            /* Rx: State 8 */
    V34_Phase3_Rec_J_Setup,      /* Rx: State 9 */
    V34_Phase3_Rec_J_Dummy,      /* Rx: State 10 */
    V34_Phase3_Wait_MD           /* Rx: State 11 */
};

CONST V34FnPtr V34P3FsmEc[] =
{
    V34_Phase3_Ec_Dummy,
    V34_Echo_Canceller
};

void  Phase3_Init(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase3_Info *p3 = &(pV34->p3);
    RetrainStruc *pRT = &(pRx->Rtrn);
    RenegoStruc *pRN = &(pRx->Renego);
    UBYTE  symbol_rate;
    QDWORD qdTemp;
    QWORD  qTemp;
    UBYTE  i;

    p3->TimeOut    = V34PHASE3_TIMEOUT;

    p3->K = 0;
    p3->I = 0;

    V34_S_Init(pV34);

    p3->symbol_counter = 0;
    p3->result_code = 0;

    p3->tx_vec_idx = 0;
    p3->rx_vec_idx = 0;
    p3->ec_vec_idx = 0;

    symbol_rate = pV34Rx->rx_symbol_rate;

    /* RTD factor convert ms to per interupt handler call (24 samples per call) */
    pV34->qRTD_factor = V34_tRTD_FACTOR[symbol_rate];

    qTemp  = pV34Rx->RTD_PH2_symbol + V34_PHASE2_SYS_DELAY;
    qdTemp = QQMULQD(qTemp, q5_OVER_6);

    if (pV34Rx->modem_mode == CALL_MODEM)
    {
        qTemp  = (QWORD)(qdTemp >> 13) + 2800;
        p3->pqTime_out_value[0] = QQMULQ15(pV34->qRTD_factor, qTemp);

        qTemp  = (QWORD)(qdTemp >> 14) + pV34Rx->md_length * 35;
        p3->pqTime_out_value[1] = QQMULQ15(pV34->qRTD_factor, qTemp);

        p3->pqTime_out_value[1] += 40;

        /* Time out value for call modem to detect second S tone */
        qTemp  = (QWORD)(qdTemp >> 14) + (600 - 10);
        p3->pqTime_out_value[2] = QQMULQ15(pV34->qRTD_factor, qTemp);
    }

#if USE_ANS
    else if (pV34Rx->modem_mode == ANS_MODEM)
    {
        /* time out value for detect s-to S bar transition */
        qTemp  = (QWORD)(qdTemp >> 14);
        qTemp += pV34Rx->md_length * 35 + 600;

        p3->pqTime_out_value[0]  = QQMULQ15(pV34->qRTD_factor, qTemp);
        p3->pqTime_out_value[0] += 200;

        /* time out value for J sequence */
        qTemp  = (QWORD)(qdTemp >> 13) + 2600;
        p3->pqTime_out_value[1]  = QQMULQ15(pV34->qRTD_factor, qTemp);
        p3->pqTime_out_value[1] += 200;

        /* time out value for detect S tone */
        qTemp  = (QWORD)(qdTemp >> 13) + 3800;
        p3->pqTime_out_value[2]  = QQMULQ15(pV34->qRTD_factor, qTemp);
    }

#endif

    V34_S_Detector_Init(&pRx->S_ToneDet);

    pRN->renego = 0;

    pRN->clear_down = 0;

    pRT->pqCoef[V34_RTN_TONE] = V34_tRetrain_Tone_Table[symbol_rate][pV34Rx->modem_mode].qTone2;
    pRT->pqCoef[V34_RTN_REF]  = V34_tRetrain_Tone_Table[symbol_rate][pV34Rx->modem_mode].qTone1;
    pRT->Tone_50ms    = 8;  /* 50ms retrain tone */
    pRT->ToneDet_Flag = 0;
    pRT->tone_count   = 0;

    V34_DFT_Init(&pRT->tonedet, pRT->pqQ1, pRT->pqQ2, pRT->pqCoef, pRT->pqMag_sqr, 48, 2);

    if (pV34Rx->modem_mode == CALL_MODEM)
    {
        /* initialize the vector for transmit */
        for (i = 0; i < 6; i++)
        {
            p3->pfTx[i] = (V34FnPtr)V34P3TxFsmCall[i];
        }

        /* initialize the vector for echo cancellor */
        for (i = 0; i < 2; i++)
        {
            p3->pfEc[i] = (V34FnPtr)V34P3FsmEc[i];
        }

        /* initialize the vector for receive */
        for (i = 0; i < 12; i++)
        {
            p3->pfRx[i] = (V34FnPtr)V34P3RxFsmCall[i];
        }
    }

#if USE_ANS
    else if (pV34Rx->modem_mode == ANS_MODEM)
    {
        /* Initialize the vector for transmit */
        for (i = 0; i < 9; i++)
        {
            p3->pfTx[i] = (V34FnPtr)V34P3TxFsmAns[i];
        }

        /* Initialize the vector for echo cancellor */
        for (i = 0; i < 2; i++)
        {
            p3->pfEc[i] = (V34FnPtr)V34P3FsmEc[i];
        }

        /* Initialize the vector for receive */
        for (i = 0; i < 12; i++)
        {
            p3->pfRx[i] = (V34FnPtr)V34P3RxFsmAns[i];
        }
    }

#endif
}
#endif
