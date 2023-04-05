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
/* ------------------------------- */
/* Phase4 Initialization routine */
/* ------------------------------- */
void  Phase4_Init(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase4_Info *p4 = &(pV34->p4);
    QDWORD qdTemp;
    QWORD  qTemp;
    UBYTE i;

    pRx->Retrain_LO_BER_Counter = 0;
    pRx->Retrain_HI_BER_Counter = 0;

    p4->tx_vec_idx = 0;
    p4->ec_vec_idx = 0;
    p4->rx_vec_idx = 0;

    /* Set detect_symbol_counter to 128T, send_symbol_counter */
    p4->detect_symbol_counter = 0;
    p4->send_symbol_counter = 0;

    /* Setup two pointer to two MP structure for transmit and receive */

    /* Current sending index of the MP bit buffer */
    p4->mp_idx = 0;
    p4->mp_size = V34_MP_0_SIZE;

    qTemp  = pV34Rx->RTD_PH2_symbol + V34_PHASE2_SYS_DELAY;
    qdTemp = QQMULQD(qTemp, q5_OVER_6);

    /* time out value setting */
    if (pV34Rx->modem_mode == CALL_MODEM)
    {
        qTemp  = (QWORD)(qdTemp >> 14) + 600;
        p4->pqTime_out_value[0] = QQMULQ15(pV34->qRTD_factor, qTemp);

        qTemp  = (QWORD)(qdTemp >> 13) + 2500;
        p4->pqTime_out_value[1] = QQMULQ15(pV34->qRTD_factor, qTemp);
    }

#if USE_ANS
    else if (pV34Rx->modem_mode == ANS_MODEM)
    {
        /* time ot value for detect s-to S bar transition */
        qTemp  = (QWORD)(qdTemp >> 14) + 100;
        p4->pqTime_out_value[0] = QQMULQ15(pV34->qRTD_factor, qTemp);

        /* time ot value for J sequence */
        qTemp = SBQMULQ(q10, (pV34Rx->RTD_PH2_symbol + V34_PHASE2_SYS_DELAY)) + 2500;
        p4->pqTime_out_value[1] = QQMULQ15(pV34->qRTD_factor, qTemp);
    }

#endif

    /* Clear out the send buffer */
    for (i = 0; i < V34_MP_1_SIZE; i++)
    {
        (p4->pMp_buf)[i] = 0;
    }

    /* Reset the long word that used to store the E */
    p4->curr_word = 0;

    /* new init starts here */
    /* initialize the vector for transmit */
    i = 0;

    if (pV34Rx->modem_mode == CALL_MODEM)
    {
        p4->pfTx[i++] = V34_Phase3_Send_J;               /* Tx: State 0 */
        p4->pfTx[i++] = V34_Phase4_Send_J_Bar;           /* Tx: State 1 */
    }

#if USE_ANS
    else if (pV34Rx->modem_mode == ANS_MODEM)
    {
        p4->pfTx[i++] = V34_Phase4_Send_S;
        p4->pfTx[i++] = V34_Phase4_Send_S_Bar;
    }

#endif
    /* The rest of the Tx vectors will be the same for Call and Answer modem */
    p4->pfTx[i++] = V34_Phase4_Send_TRN;             /* Tx: State 2 */
    p4->pfTx[i++] = V34_Phase4_Send_TRN_2000MS;
    p4->pfTx[i++] = V34_Phase4_Send_MP;
    p4->pfTx[i++] = V34_Phase4_Finish_MP;
    p4->pfTx[i++] = V34_Phase4_Send_MP;              /* Tx: State 6 */
    p4->pfTx[i++] = V34_Phase4_Finish_MP_Pi;         /* Tx: State 7 */
    p4->pfTx[i++] = V34_Phase4_Flush_MP_Pi;
    p4->pfTx[i++] = V34_Phase4_Prepare_E;
    p4->pfTx[i++] = V34_Phase4_Send_E;
    p4->pfTx[i++] = V34_Phase4_Send_B1_Frame;
    p4->pfTx[i++] = V34_Begin_Data_Transfer;
    p4->pfTx[i++] = V34_Rate_Renego_Init;            /* for rate renegotiation  state 13 */

    /* Initialization for Echo cancellor vector */
    i = 0;
    p4->pfEc[i++] = V34_Echo_Canceller;
    p4->pfEc[i++] = V34_Phase3_Ec_Dummy;

    i = 0;

    if (pV34Rx->modem_mode == CALL_MODEM)
    {
        p4->pfRx[i++] = V34_Phase4_Det_S_Bar;
        p4->pfRx[i++] = V34_Phase4_Wait_S_Bar_16T;
        p4->pfRx[i++] = V34_Phase4_EQ_1;
    }

#if USE_ANS
    else if (pV34Rx->modem_mode == ANS_MODEM)
    {
        p4->pfRx[i++] = V34_Phase4_Det_J_Bar;
    }

#endif

    p4->pfRx[i++] = V34_Phase4_Det_TRN;
    p4->pfRx[i++] = V34_Phase4_Det_TRN_TC;
    p4->pfRx[i++] = V34_Phase4_Rec_MP;
    p4->pfRx[i++] = V34_Phase4_Rec_MP_Pi;
    p4->pfRx[i++] = V34_Phase4_Detect_E;
    p4->pfRx[i++] = V34_Phase4_VA_Delay;
    p4->pfRx[i++] = V34_Phase4_Rec_B1_Frame;
    p4->pfRx[i++] = V34_Phase4_Rec_Superframe;
}
#endif