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
/* Phase2 Answer Modem Tx Vectors                                           */
/****************************************************************************/

#include "ph2ext.h"

void Ph2_UpdateTxDelayCnt(Ph2Struct *pPH2)
{
    pPH2->delay_counter_tx--;

    if (pPH2->delay_counter_tx <= 0)
    {
        pPH2->tx_vec_idx++;
    }
}


/*************/
/* Vector 1  */
/*************/
void Ph2_Send_Info_0A(Ph2Struct *pPH2)
{
    Ph2_DPSK_Modfilter(pPH2, pPH2->pInfo_buf[pPH2->info_idx], V34_qANS_MODEM_CARRIER);

    pPH2->info_idx++;

    if (pPH2->info_idx >= V34_INFO_0_SIZE)
    {
        if (pPH2->recovery)
        {
            pPH2->info_idx = 0;

            if (pPH2->recovery_info0_detected == 1)
            {
                pPH2->I0a.acknowledge_correct_recept = 1;

                Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->I0a), ShiftTable0);

                pPH2->recovery = 0;
            }
        }
        else
        {
            pPH2->tx_vec_idx = 2;

#if SUPPORT_V92A

            if (pPH2->Short_Phase2)
            {
                pPH2->tx_vec_idx = 4;    /* Send ToneA Continuous */
            }

#endif
            TRACE0("T1 I0a>");
            pPH2->delay_counter_tx = V34_PHASE2_160MS;
            /* 160ms for V90a to work with some ISP in Japan, at least 50ms by ITU */
            /* Notice: 160ms requirement is only our guess, this has never been confirmed
            by Baseline */
            pPH2->info_idx = 0;
        }
    }
}

/************/
/* Vector 2 */
/************/
void Ph2_Create_Tone_A_50MS(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 0, V34_qANS_MODEM_CARRIER);

    pPH2->delay_counter_tx--; /* Check from RX vectors */
}

/************/
/* Vector 3 */
/************/
void Ph2_Create_Tone_A_Phase_Change(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 1, V34_qANS_MODEM_CARRIER);

    pPH2->tx_vec_idx = 4;

#if SUPPORT_V34FAX

    if (pPH2->V34fax_Enable == 1)
    {
        pPH2->delay_counter_tx = V34_PHASE2_10MS;
    }

#endif

    pPH2->RTD_Status = 1;        /* Start counting RTD */
    pPH2->RTD_Symbol = 0;

    TRACE0("T3 A->");
}

/****************/
/* Vector 4, 14 */
/****************/
void Ph2_Create_Tone_A(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 0, V34_qANS_MODEM_CARRIER);
}

#if 0 /// LLL temp
/************/
/* Vector 5 */
/************/
void Ph2_Create_Tone_A_40MS(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 0, V34_qANS_MODEM_CARRIER);

    Ph2_UpdateTxDelayCnt(pPH2);

    if (pPH2->delay_counter_tx < 0)        /* go straight to next state */
    {
        TRACE0("T5-6>");

        Ph2_Create_Tone_A_PR_10MS(pPH2);
    }
}

/****************/
/* Vector 6, 11 */
/****************/
void Ph2_Create_Tone_A_PR_10MS(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 1, V34_qANS_MODEM_CARRIER);

    /* TRACE1("T%d>",pPH2->tx_vec_idx); */

    if (pPH2->tx_vec_idx == 11)
    {
        pPH2->rx_vec_idx = 5;
    }

    pPH2->tx_vec_idx++;

    pPH2->delay_counter_tx = V34_PHASE2_10MS;
}
#endif

/****************/
/* Vector 7, 12 */
/****************/
void Ph2_Create_Tone_A_10MS(Ph2Struct *pPH2)
{
#if SUPPORT_V92A
    Info_1aStruc *pI1a = &(pPH2->I1a);
#endif

    Ph2_DPSK_Mod(pPH2, 0, V34_qANS_MODEM_CARRIER);

    Ph2_UpdateTxDelayCnt(pPH2);

    if (pPH2->tx_vec_idx == 8)    /* this is the next state */
    {
#if SUPPORT_V92A

        if (pPH2->Short_Phase2)
        {
            pI1a->sym_rate_call_to_answer = V90_SYM_8000;

            Ph2_SetupInfo1A_V90A(pPH2);

            pPH2->info_idx   = 0;
            pPH2->tx_vec_idx = 16;
            TRACE0("T15 I1a created");
        }
        else
#endif
        {
            /* Send L1 probing tone for 160ms */
            pPH2->delay_counter_tx = V34_PHASE2_160MS;

            pPH2->L2_done = 2; /* L1/L2 echo presence */
        }
    }
}

#if 0 /// LLL temp
/****************/
/* Vector 8     */
/****************/
void Ph2_Create_L1_ANS(Ph2Struct *pPH2)
{
    Ph2_Create_Probing(pPH2, V34_qL1_AMPLITUDE, &(pPH2->L1_counter));

    if (--pPH2->delay_counter_tx <= 0)
    {
        /* Condition Transmitter to transmit L2 tones */
        pPH2->delay_counter_tx = V34_PHASE2_650MS + pPH2->RTD_Symbol - 5;    /* make it shorter than B1 detection time out */
        pPH2->delay_counter_rx = pPH2->RTD_Symbol + V34_PHASE2_SYS_DELAY;
        pPH2->tx_vec_idx = 9;  /* next state: Send L2 */
        pPH2->rx_vec_idx = 4;  /* next state: Detect TONE B */
        pPH2->Time_cnt_tone_start = pPH2->Time_cnt;   /* init time counter for detect Tone A2  */
        pPH2->toneb1_detected = 0;
        pPH2->toneA2_detected = 0;
        TRACE0("T8 L1>");
    }
}

/************/
/* Vector 9 */
/************/
void Ph2_Create_L2_ANS(Ph2Struct *pPH2)
{
    Ph2_Create_Probing(pPH2, V34_qL2_AMPLITUDE, &(pPH2->L2_counter));

    /* Send L2 for 500MS + RTD, then send TONE A for 50ms, */
    /* no matter we detect reponse tone B1 or not.         */
    --(pPH2->delay_counter_tx);

    if ((pPH2->delay_counter_tx <= 0) || (pPH2->toneb1_detected == 1))
    {
        pPH2->tx_vec_idx = 10;    /* next state: Send TONE A for 50ms */
        pPH2->delay_counter_tx = V34_PHASE2_50MS;
        pPH2->L2_done = 0; /* L2 echo not presence */
        TRACE0("T9 L2>");
    }
}

/*************/
/* Vector 10 */
/*************/
void Ph2_Create_Tone_A_After_L2(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 0, V34_qANS_MODEM_CARRIER);

    Ph2_UpdateTxDelayCnt(pPH2);
}

void SetInfoSymbolRate(Ph2Struct *pPH2, UBYTE rate)
{
    Info_1aStruc *pI1a = &(pPH2->I1a);

    pI1a->INFO1a_Bit25            = pPH2->pSymRate_info[rate].carrier_freq_idx;
    pI1a->pre_em_index            = pPH2->pSymRate_info[rate].preemphasis_idx;
    pI1a->pro_data_rate           = pPH2->pSymRate_info[rate].project_data_rate;
    pI1a->sym_rate_answer_to_call = rate;
    pI1a->sym_rate_call_to_answer = rate;
}
#endif

#if SUPPORT_V34FAX
void SetInfoHSymbolRate(Ph2Struct *pPH2, UBYTE rate)
{
    Info_hStruc *pIh = &(pPH2->Ih);

    pIh->high_carrier_freq = pPH2->pSymRate_info[rate].carrier_freq_idx;

    pIh->pre_em_index = pPH2->pSymRate_info[rate].preemphasis_idx;

    pIh->sym_rate_call_to_answer = rate;
}
#endif

UBYTE IsSymbolRateFit(Ph2Struct *pPH2, UBYTE rate)
{
    return ((pPH2->pSymRate_info[rate].project_data_rate) && (pPH2->max_symbol_rate >= rate)
            && (pPH2->min_symbol_rate <= rate));
}

#if 0 /// LLL temp
/****************/
/* Vector 15    */
/****************/
void Ph2_Create_Tone_A_Set_1A(Ph2Struct *pPH2)
{
    Info_1aStruc *pI1a = &(pPH2->I1a);
    UBYTE ubTemp;

    /* SRATE Table is a heavy calculation process, so we split      */
    /* the calculation into 6 DPSK symbol time for each symbol rate */
    /* "delay_counter_tx" is used as the symbol rate index 0-5.     */
    if (pPH2->delay_counter_tx < 0)
    {
        pPH2->delay_counter_tx = 0;
    }

    if ((pPH2->delay_counter_tx <= 30) && (pPH2->delay_counter_tx % 6) == 0)
    {
        ubTemp = (UBYTE)(QQMULQR15(pPH2->delay_counter_tx, q1_OVER_6));
        Ph2_Create_SymRate_Table(pPH2, &(pPH2->I0a), &(pPH2->I0c), ubTemp);
    }

    ++pPH2->delay_counter_tx;

    /* Continous sending Tone A unitl SRATE table is created */
    Ph2_DPSK_Mod(pPH2, 0, V34_qANS_MODEM_CARRIER);

    if (pPH2->delay_counter_tx > 35)
    {
        /* Based on the Line-Probing result, we can determine what symbol rate */
        /* we are going to use for both call modem and answer modem,          */
        /* so as the data rate.                                               */
        pI1a->min_power_reduction = 0; /* Assume for Line-Probing */
        pI1a->add_power_reduction = 0; /* Assume for Line-Probing */
        pI1a->length_MD           = 0;

        pI1a->freq_offset = -512;/* Assume for Line-Probing, ignore freq offset */

        /* Check from 3429Hz symbol rate down to 2400Hz symbol rate */
        /* No asymmerty symbol rate is supported.                   */

        if ((pPH2->pSymRate_info[V34_SYM_3429].project_data_rate) && (pPH2->I1c.pro_data_rate_S3429 > 0)
            && (pPH2->max_symbol_rate == V34_SYM_3429))
        {
            SetInfoSymbolRate(pPH2, V34_SYM_3429);
        }
        else if ((IsSymbolRateFit(pPH2, V34_SYM_3200)) && (pPH2->I1c.pro_data_rate_S3200 > 0))
        {
            SetInfoSymbolRate(pPH2, V34_SYM_3200);
        }
        else if ((IsSymbolRateFit(pPH2, V34_SYM_3000)) && (pPH2->I1c.pro_data_rate_S3000 > 0))
        {
            SetInfoSymbolRate(pPH2, V34_SYM_3000);
        }
        else if ((IsSymbolRateFit(pPH2, V34_SYM_2800)) && (pPH2->I1c.pro_data_rate_S2800 > 0))
        {
            SetInfoSymbolRate(pPH2, V34_SYM_2800);
        }
        else if ((IsSymbolRateFit(pPH2, V34_SYM_2743)) && (pPH2->I1c.pro_data_rate_S2743 > 0))
        {
            SetInfoSymbolRate(pPH2, V34_SYM_2743);
        }
        else if ((pPH2->pSymRate_info[V34_SYM_2400].project_data_rate) && (pPH2->I1c.pro_data_rate_S2400 > 0)
                 && (pPH2->min_symbol_rate == V34_SYM_2400))
        {
            SetInfoSymbolRate(pPH2, V34_SYM_2400);
        }
        else
        {
            /* assume  V34_SYM_2400 == 0 */
            pI1a->INFO1a_Bit25            = 0;
            pI1a->pre_em_index            = 0;
            pI1a->pro_data_rate           = 0;
            pI1a->sym_rate_answer_to_call = 0;
            pI1a->sym_rate_call_to_answer = 0;

            pPH2->Retrain_flag            = RETRAIN_INIT;
            pPH2->Ph2State.retrain_reason = PH2_NO_SYMBOL_RATE_SUPPORT;
        }

        /*
        * further squeeze can be done by moving the
        *    pPH2->I1c.pro_data_rate_Sxxxx > 0   into
        *    IsSymbolRateFit()
        * however this will fix the data structure (it should be fixed)
        */

#if SUPPORT_V90A

        if (pPH2->V90_Enable == 1)
        {
            Ph2_SetupInfo1A_V90A(pPH2);
        }
        else
#endif
            Ph2_CreateInfo(pPH2->pInfo_buf1A, (UBYTE *)&(pI1a->min_power_reduction), ShiftTable1a);

        pPH2->info_idx = 0;

        Ph2_NLD_Analyze(pPH2);

        pPH2->tx_vec_idx = 16;

        TRACE0("T15 I1a created");
    }
}

/* Vector 16 */
void Ph2_Send_Info_1A(Ph2Struct *pPH2)
{
    UBYTE bit;

    if (pPH2->info_idx < V34_INFO_1A_SIZE)
    {
        bit = pPH2->pInfo_buf1A[pPH2->info_idx];
    }
    else
    {
        bit = 1;
    }

    Ph2_DPSK_Modfilter(pPH2, bit, V34_qANS_MODEM_CARRIER);

    ++pPH2->info_idx;

    if (pPH2->info_idx >= (V34_INFO_1A_SIZE + 3))
    {
        pPH2->tx_vec_idx = 17;        /* next state */

#if SUPPORT_V90A

        if (pPH2->modem_mode == ANS_MODEM && pPH2->V90_Enable)
        {
            pPH2->delay_counter_tx =  6;
        }
        else
#endif
            pPH2->delay_counter_tx =  5;

        TRACE0("T16 I1a>");
    }
}
#endif

/* Vector 17 */
void Ph2_Cleanup_Tx(Ph2Struct *pPH2)
{
    Ph2_DPSK_Silence(pPH2);

    pPH2->delay_counter_tx--;

    if (pPH2->delay_counter_tx <= 0)
    {
#if SUPPORT_V34FAX

        if ((pPH2->V34fax_Enable == 0)
            || ((pPH2->V34fax_Enable == 1) && (pPH2->Silence_Detect_Count > 0)))
#endif
        {
            pPH2->Phase2_End = 1;

            pPH2->TimingDrift_Enable = 1;
        }
    }
}

#if SUPPORT_V34FAX        /* Only for answer Fax */

void Ph2_Create_Tone_A_Set_H(Ph2Struct *pPH2)
{
    Info_hStruc *pIh = &(pPH2->Ih);
    UBYTE ubTemp;

    /* SRATE Table is a heavy calculation process, so we split      */
    /* the calculation into 6 DPSK symbol time for each symbol rate */
    /* "delay_counter_tx" is used as the symbol rate index 0-5.     */
    if (pPH2->delay_counter_tx < 0)
    {
        pPH2->delay_counter_tx = 0;
    }

    if ((pPH2->delay_counter_tx <= 30) && (pPH2->delay_counter_tx % 6) == 0)
    {
        ubTemp = (UBYTE)(pPH2->delay_counter_tx / 6);

        Ph2_Create_SymRate_Table(pPH2, &(pPH2->I0a), &(pPH2->I0c), ubTemp);
    }

    ++pPH2->delay_counter_tx;

    /* Continous sending Tone A unitl SRATE table is created */
    Ph2_DPSK_Mod(pPH2, 0, V34_qANS_MODEM_CARRIER);

    if (pPH2->delay_counter_tx > 35)
    {
        /* Based on the Line-Probing result, we can determine what symbol rate */
        /* we are going to use for both call modem         */
        /* so as the data rate.                            */
        pIh->power_reduction = 0;
        pIh->trn_length      = 64; /* pPH2->Ih.trn_length = 64 * 35ms */

        /* Check from 3429Hz symbol rate down to 2400Hz symbol rate */
        /* No asymmetry symbol rate is supported.                   */

        Ph2_NLD_Analyze(pPH2);

        if ((pPH2->pSymRate_info[V34_SYM_3429].project_data_rate) && (pPH2->max_symbol_rate == V34_SYM_3429))
        {
            SetInfoHSymbolRate(pPH2, V34_SYM_3429);
        }
        else if (IsSymbolRateFit(pPH2, V34_SYM_3200))
        {
            SetInfoHSymbolRate(pPH2, V34_SYM_3200);
        }
        else if (IsSymbolRateFit(pPH2, V34_SYM_3000))
        {
            SetInfoHSymbolRate(pPH2, V34_SYM_3000);
        }
        else if (IsSymbolRateFit(pPH2, V34_SYM_2800))
        {
            SetInfoHSymbolRate(pPH2, V34_SYM_2800);
        }
        else if (IsSymbolRateFit(pPH2, V34_SYM_2743))
        {
            SetInfoHSymbolRate(pPH2, V34_SYM_2743);
        }
        else if ((pPH2->pSymRate_info[V34_SYM_2400].project_data_rate) && (pPH2->min_symbol_rate == V34_SYM_2400))
        {
            SetInfoHSymbolRate(pPH2, V34_SYM_2400);
        }
        else
        {
            /* assume  V34_SYM_2400 == 0 */
            pIh->high_carrier_freq       = 0;
            pIh->pre_em_index            = 0;
            pIh->sym_rate_call_to_answer = 0;
        }

        pIh->TRN_constellation = 1;

        /*
         * further squeeze can be done by moving the
         *    pPH2->I1c.pro_data_rate_Sxxxx > 0 into IsSymbolRateFit()
         * however this will fix the data structure (it should be fixed)
         */

        Ph2_CreateInfo(pPH2->pInfo_buf1A, (UBYTE *)&(pIh->power_reduction), ShiftTableh);

        pPH2->info_idx = 0;

        pPH2->tx_vec_idx++;
    }
}


void Ph2_Send_Info_H(Ph2Struct *pPH2)
{
    Ph2_DPSK_Modfilter(pPH2, pPH2->pInfo_buf1A[pPH2->info_idx], V34_qANS_MODEM_CARRIER);

    ++ pPH2->info_idx;

    if (pPH2->info_idx >= V34_INFO_H_SIZE)
    {
        pPH2->tx_vec_idx++;                  /* next state */

        pPH2->delay_counter_tx = 12;

        TRACE0("Sending Info h");
    }
}

#endif
