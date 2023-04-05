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
/* Phase2 Call Modem Rx Vectors                                             */
/*                                                                          */
/* Functions:                                                               */
/*   void Ph2_Biquad_Tone_Det(V34Struct *, SWORD *)                         */
/*   void Ph2_Det_Tone_A_PV_40MS(V34Struct *,SWORD *)                       */
/*   void Ph2_Detect_Tone_A2( V34Struct *, SWORD *)                         */
/****************************************************************************/

#include <string.h>
#include "ph2ext.h"

void Ph2_Check_Tone_N(Ph2Struct *pPH2, UBYTE N)
{
    /* directly compare Mag1 with Mag0 and Mag2 */

    if (pPH2->pqMag_sqr[0] < 5)
    {
        pPH2->pqMag_sqr[0] = 5;    /* avoid small signal */
    }

    if (pPH2->pqMag_sqr[2] < 5)
    {
        pPH2->pqMag_sqr[2] = 5;    /* avoid small signal */
    }

    if ((pPH2->pqMag_sqr[1] > pPH2->pqMag_sqr[0] * 5) && (pPH2->pqMag_sqr[1] > pPH2->pqMag_sqr[2] * 5))
    {
        pPH2->tone_count++;

        if (pPH2->tone_count > N)
        {
            pPH2->dettone = 1;
            pPH2->tone_count = 0;
            TRACE0("tone");
        }
    }
    else
    {
        pPH2->tone_count = 0;
    }
}


void Ph2_DetCheck_ToneAB(Ph2Struct *pPH2, UBYTE N)
{
    UBYTE i, result = 0;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        result |= V34_DFT(&pPH2->tonedet, pPH2->PCMinPtr[i]);
    }

    if (result == 1)
    {
        Ph2_Check_Tone_N(pPH2, N);
    }
}

void PrepareToRecoverInfo0c(Ph2Struct *pPH2)
{
    SWORD info_size;

    pPH2->bit_0_count = 0;

#if SUPPORT_V90D

    if ((pPH2->modem_mode == CALL_MODEM) && pPH2->V90_Enable)
    {
        info_size = V34_INFO_0d_SIZE;
    }
    else
#endif
        info_size = V34_INFO_0_SIZE;

    if (pPH2->info_idx >= info_size)
    {
        pPH2->info_idx  = 0;
    }

    /* prepare to send info0c with ACK to 1 */
    pPH2->I0c.acknowledge_correct_recept = 1;

#if SUPPORT_V90D

    if ((pPH2->modem_mode == CALL_MODEM) && pPH2->V90_Enable)
    {
        Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->I0c), ShiftTable0d);
    }
    else
#endif
        Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->I0c), ShiftTable0);

    pPH2->tx_vec_idx = 1;     /* go to send info0c with ACK set to 1 */
    pPH2->rx_vec_idx = 1;

    pPH2->recovery      = 1;  /* go to recovery */
    pPH2->init_recovery = 0;
    pPH2->LineProbOn    = 0;
}

/*************/
/* Vector 1  */
/*************/
void Ph2_Detect_Tone_A(Ph2Struct *pPH2)
{
    QDWORD qdTemp;
    QWORD  qTemp;
    SBYTE  bit;
    SBYTE  INFO0_result;
    UBYTE  ubLogic = 0;

    pPH2->uToneAorB_Timeout_Count ++;

    /* biquad tone detection */
    Ph2_Biquad_Tone_Det(pPH2);

    bit = Ph2_DPSK_Demod(pPH2);

    if (bit >= 0)
    {
        INFO0_result = Ph2_V34Detect_Info_0(bit, &(pPH2->I0check));
    }
    else
    {
        INFO0_result = 0;
    }

    if ((bit == -10) || (bit == 0))
    {
        pPH2->bit_0_count++;
    }
    else
    {
        pPH2->bit_0_count = 0;
    }

#if 0
    Ph2_DetCheck_ToneAB(pPH2, 2);
#else

#if 0

    if (DumpTone1_Idx < 10000000) { DumpTone1[DumpTone1_Idx++] = pPH2->qdTotal_egy; }

    if (DumpTone2_Idx < 10000000) { DumpTone2[DumpTone2_Idx++] = pPH2->qdTotal_egy_biqin; }

#endif

    if ((pPH2->qdTotal_egy > V34_TONE_ENERGY1) && (pPH2->qdTotal_egy > pPH2->qdTotal_egy_biqin))
    {
        pPH2->dettone++;
    }
    else
    {
        pPH2->dettone = 0;
    }

#endif

    /* for recovery only */
    if ((pPH2->dettone > 6) && (pPH2->bit_0_count > 6))
    {
        pPH2->uToneAorB_Timeout_Count = 0;

        pPH2->recovery = 0;  /* go to normal */

        ubLogic = (pPH2->tx_vec_idx == 2) && (pPH2->delay_counter_tx >= (V34_PHASE2_SYS_DELAY + 10));

        if (ubLogic)
        {
            pPH2->bit_0_count = 0;

            pPH2->rx_vec_idx = 2;      /* next state: Detect TONE A Phase reversal */

            TRACE0("R1 A>");

            TRACE1("pPH2->qdTotal_egy = %" FORMAT_DWORD_PREFIX "d", pPH2->qdTotal_egy);

            qTemp = (QWORD)(QDsqrt(pPH2->qdTotal_egy));

            qdTemp = QQMULQD(pPH2->AGC_gain, PH2_AGC_REFERENCE_A);

            pPH2->AGC_gain = (QWORD)QDQDdivQD(qdTemp, qTemp);

            TRACE2("Ph2_A_egy = %d, pPH2->AGC_gain = %d", qTemp, pPH2->AGC_gain);

            pPH2->init_recovery = 0;
            pPH2->dettone = 0;
        }
    }

    if (INFO0_result == 1 && pPH2->init_recovery == 0)
    {
        pPH2->uToneAorB_Timeout_Count = 0;

        if (pPH2->I0check.acknowledge_correct_recept == 0)
        {
            PrepareToRecoverInfo0c(pPH2);       /* !! assume rx_vec_idx is 1 here */

            TRACE0("R1 rep I0! ");
        }
        else
        {
            pPH2->bit_0_count = 0;
            pPH2->recovery    = 0;
            TRACE0("R1 I0 recov");
        }
    }

#if SUPPORT_V92D

    if (INFO0_result == MH_SEQUENCE_DETECTED)
    {
        Ph2_Init_MH_Response(pPH2);
    }

#endif
}

/****************/
/* Vector 2, 7  */
/****************/
void Ph2_Det_Tone_A_PV_40MS(Ph2Struct *pPH2)
{
    SBYTE dbit;
    SBYTE INFO0_result;

    pPH2->uTimeout_Count ++;

    dbit = Ph2_DPSK_Demod(pPH2);

    if (dbit >= 0)
    {
        INFO0_result = Ph2_V34Detect_Info_0(dbit, &(pPH2->I0check));
    }
    else
    {
        INFO0_result = 0;
    }

    pPH2->delay_counter_rx--;

    if ((dbit == -11) || (dbit == 1))
    {
        pPH2->delay_counter_tx = V34_PHASE2_40MS - V34_PHASE2_SYS_DELAY;

        if ((pPH2->rx_vec_idx == 2) || (pPH2->delay_counter_rx <= 0))
        {
            /* ^^for state 7^^ */
            pPH2->uTimeout_Count = 0;

            TRACE1("R%d A->", pPH2->rx_vec_idx);
            pPH2->tx_vec_idx ++;         /* both advance to next state */
            pPH2->rx_vec_idx ++;
            pPH2->delay_counter_rx = 0;
        }
    }
    else if (INFO0_result == 1)
    {
        pPH2->uTimeout_Count = 0;

        PrepareToRecoverInfo0c(pPH2);

        pPH2->recovery_info0_detected = 0;

        TRACE1("R%d rep I0", pPH2->rx_vec_idx);
    }

#if SUPPORT_V92D

    if (INFO0_result == MH_SEQUENCE_DETECTED)
    {
        Ph2_Init_MH_Response(pPH2);
    }

#endif

}

#if 0 /// LLL temp
/*************/
/* Vector 3  */
/*************/
void Ph2_Det_Tone_A_PV_RTD(Ph2Struct *pPH2)
{
    SBYTE dbit;
    SBYTE INFO0_result;
    SWORD rtde_temp;

    pPH2->uTimeout_Count ++;

    dbit = Ph2_DPSK_Demod(pPH2);

    if (dbit >= 0)
    {
        INFO0_result = Ph2_V34Detect_Info_0(dbit, &(pPH2->I0check));
    }
    else
    {
        INFO0_result = 0;
    }

    if (pPH2->RTD_Status == 1)
    {
        pPH2->RTD_Symbol++;

        if ((pPH2->RTD_Symbol >= V34_PHASE2_40MS) && ((dbit == -11) || (dbit == 1)))
        {
            pPH2->delay_counter_rx = V34_PHASE2_160MS;
            rtde_temp = pPH2->RTD_Symbol - (V34_PHASE2_40MS + V34_PHASE2_SYS_DELAY);

            if (rtde_temp >= -2)
            {
                pPH2->uTimeout_Count = 0;

                pPH2->RTD_Symbol = rtde_temp;

                pPH2->rx_vec_idx = 4;          /* next state */
                pPH2->RTD_Status = 0;
                pPH2->LineProbOn = 1;

                TRACE0("R3>");
            }
        }
    }

    if (INFO0_result == 1)
    {
        pPH2->uTimeout_Count = 0;

        PrepareToRecoverInfo0c(pPH2);
        pPH2->recovery_info0_detected = 0;
        pPH2->RTD_Status = 0;
        pPH2->RTD_Symbol = 0;
        TRACE0("R3 rep I0");
    }

#if SUPPORT_V92D

    if (INFO0_result == MH_SEQUENCE_DETECTED)
    {
        Ph2_Init_MH_Response(pPH2);
    }

#endif

}
#endif

/***************************/
/* CALL MODEM:    Vector 4 */
/* ANSWER MODEM:  Vector 6 */
/***************************/
void Ph2_Detect_L1(Ph2Struct *pPH2)
{
    ProbStruc *probing = &(pPH2->Probing);
    SBYTE dbit, INFO0_result = 0;

    pPH2->TimingDrift_Enable = 0;

    if (pPH2->modem_mode == CALL_MODEM)
    {
        dbit = Ph2_DPSK_Demod(pPH2);

        if (dbit >= 0)
        {
            INFO0_result = Ph2_V34Detect_Info_0(dbit, &(pPH2->I0check));
        }
        else
        {
            INFO0_result = 0;
        }
    }

    Ph2_Estimate_Power_Spectrum(pPH2);

    pPH2->delay_counter_rx--;

    if (pPH2->delay_counter_rx <= 0)
    {
        /* Prepare to receive L2 signal */
        probing->power_counter = -3;
        probing->pqPower_Spectrum = probing->pqL2_Power_Spectrum;

        pPH2->delay_counter_rx = V34_PHASE2_400MS;/* V34_PHASE2_300MS; */

        /* TRACE1("R%d L1>",pPH2->rx_vec_idx); */
        pPH2->rx_vec_idx++;
        memset(pPH2->biq_dline, 0, sizeof(pPH2->biq_dline));
    }

    if ((pPH2->modem_mode == CALL_MODEM) && (INFO0_result == 1))
    {
        PrepareToRecoverInfo0c(pPH2);
        pPH2->RTD_Status = 0;
        pPH2->RTD_Symbol = 0;
        TRACE1("R%d rep I0", pPH2->rx_vec_idx);
    }
}

/****************************/
/* CALL MODEM:    Vector 5  */
/* ANSWER MODEM:  Vector 7  */
/****************************/
void Ph2_Detect_L2(Ph2Struct *pPH2)
{
    /* Ph2_Biquad_Tone_Det(pPH2); */

    pPH2->TimingDrift_Enable = 1;

    Ph2_Estimate_Power_Spectrum(pPH2);

    pPH2->delay_counter_rx--;

    if (pPH2->delay_counter_rx <= 0)
    {
        /* Prepare to receive A tone at the call modem   */
        /* Prepare to receive info1c at the answer modem */
        if (pPH2->modem_mode == CALL_MODEM)
        {
            pPH2->tx_vec_idx = 7;  /* next state: Send TONE B */
            pPH2->delay_counter_tx = 0;
            /* pPH2->rx_vec_idx = 6; */  /* next state: Detect TONE A */

            pPH2->Time_cnt_tonea1_start = pPH2->Time_cnt;
            pPH2->delay_counter_rx = V34_PHASE2_SYS_DELAY;
        }
        else if (pPH2->modem_mode == ANS_MODEM)
        {
            Ph2_DPSK_Init(&(pPH2->Dpsk), ANS_MODEM);

#if SUPPORT_V34FAX

            if (pPH2->V34fax_Enable == 1)
            {
                pPH2->tx_vec_idx++;
            }
            else
#endif
            {
                pPH2->tx_vec_idx = 14; /* Send TONE A */
            }

            pPH2->Time_cnt_info1_start = pPH2->Time_cnt;

            /* pPH2->rx_vec_idx = 8; */  /* next state: Detect INFO1c */
        }

        TRACE1("R%d L2>", pPH2->rx_vec_idx);
        pPH2->rx_vec_idx++;

        pPH2->LineProbOn = 0;
    }
}

#if 0 /// LLL temp
/*************/
/* Vector 6  */
/*************/
void Ph2_Detect_Tone_A_After_L2(Ph2Struct *pPH2)
{
    Ph2_Biquad_Tone_Det(pPH2);

    Ph2_DPSK_Demod(pPH2);

    if (pPH2->qdTotal_egy > V34_TONE_ENERGY)
    {
        pPH2->dettone++;
    }
    else
    {
        pPH2->dettone = 0;
    }

    pPH2->delay_counter_rx--;

    if ((pPH2->delay_counter_rx <= 0) && (pPH2->dettone > 6))
    {
        pPH2->bit_0_count = 0;
        pPH2->rx_vec_idx = 7; /* next state: Detect TONE A Phase reversal */
        TRACE0("R6 A>");
        pPH2->delay_counter_rx = 4; /* delay for 4 symbols detecting Phase change */
        pPH2->tone_count = 0;
        pPH2->dettone = 0;
    }

    if (pPH2->Time_cnt_tonea1_start && (pPH2->Time_cnt - pPH2->Time_cnt_tonea1_start) > (V34_TONE_A1_PR_TIME + pPH2->RTD_Symbol - V34_PHASE2_SYS_DELAY))
    {
        pPH2->bit_0_count = 0;
        pPH2->tx_vec_idx = 8;  /* Send TONE B 40MS */
        pPH2->rx_vec_idx = 8;  /* Detect DUMMY */
        pPH2->delay_counter_tx = V34_PHASE2_40MS;
        pPH2->Time_cnt_tonea1_start = 0;
        TRACE0("R6->8");
    }
}
#endif

/*************/
/* Vector 9  */
/*************/
void Ph2_Detect_Tone_A2(Ph2Struct *pPH2)
{
    /* biquad tone detection */
    Ph2_Biquad_Tone_Det(pPH2);

    pPH2->uTimeout_Count ++;

    pPH2->delay_counter_rx--;

    if ((pPH2->delay_counter_rx <= 0) && (pPH2->qdTotal_egy > V34_TONE_ENERGY))
    {
        pPH2->toneA2_detected ++;
    }
    else
    {
        pPH2->toneA2_detected = 0;
    }

    if (pPH2->L2_done == 1)
    {
        pPH2->uTimeout_Count = 0;

#if SUPPORT_V34FAX

        if (pPH2->V34fax_Enable == 1)
        {
            pPH2->tx_vec_idx = 8;     /* next state: Send B tone */
            pPH2->rx_vec_idx = 5;     /* next state: Ph2_Detect_Info_h */
        }
        else
#endif
        {
            pPH2->tx_vec_idx = 13;     /* next state: Setup INFO1c */
            pPH2->rx_vec_idx = 10;     /* next state: Detect TONE A DUMMY */
        }

        TRACE0("R9 A2>");
        pPH2->L2_done = 0;

        /* Use delay_counter_tx as a Symbol Rate index for Info1c_setup */
        pPH2->delay_counter_tx = 0;
    }
}

/**************/
/* Vector 10  */
/**************/
void Ph2_Dummy(Ph2Struct *pPH2)
{
}

#if 0 /// LLL temp
/***************/
/* Vector 11   */
/***************/
void Ph2_Detect_Info_1a(Ph2Struct *pPH2)
{
    Info_1aStruc *pI1a = &(pPH2->I1a);
    SBYTE info_1a_check;
    SBYTE dbit;

    dbit = Ph2_DPSK_Demod(pPH2);

    /* check retrain tone */
    if (pPH2->send_info1c_done)
    {
        Ph2_DetCheck_ToneAB(pPH2, 7);

        if (pPH2->dettone)
        {
            TRACE0("R11 rtrn");

            pPH2->Retrain_flag = RETRAIN_RESPOND;

            pPH2->Ph2State.retrain_reason = PH2_DETECT_RETRAIN_IN_DET_INFO1A;

            pPH2->Time_cnt_info1_start = 0;
        }
    }

    if (dbit >= 0)
    {
#if SUPPORT_V90D

        if (pPH2->modem_mode == CALL_MODEM && pPH2->V90_Enable)
        {
            info_1a_check = Ph2_V90dDetect_Info_1a(dbit, pI1a);
        }
        else
#endif
            info_1a_check = Ph2_V34Detect_Info_1a(dbit, pI1a);

        if (info_1a_check == 1)  /* CRC checked */
        {
            pPH2->Time_cnt_info1_start = 0;

            Ph2_Receive_Info_1(pPH2);

#if SUPPORT_V92D
            /* bit 12-13, value 0-3 */
            pPH2->filter_sections = pI1a->min_power_reduction & 3;
            /* bit 14 */
            pPH2->max_total_coefficients = (pI1a->min_power_reduction & 4) >> 2;
            /* bit 15, value 0-3 */
            pPH2->max_total_coefficients |= ((pI1a->add_power_reduction & 1) << 1);
            /* bit 16-17, value 0-3 */
            pPH2->max_each_coefficients = ((pI1a->add_power_reduction >> 1) & 3);

            pPH2->max_total_coefficients = 192 + (pPH2->max_total_coefficients << 6);
            pPH2->max_each_coefficients = 128 + (pPH2->max_each_coefficients << 6);

            TRACE1("filter:%d", pPH2->filter_sections);
            TRACE1("total max:%d", pPH2->max_total_coefficients);
            TRACE1("each max:%d", pPH2->max_each_coefficients);
            TRACE1("V90 rate:%d", pI1a->sym_rate_call_to_answer);
#endif
            pPH2->rx_vec_idx = 12; /* Final Setup from Info1a infomation */

            TRACE1("R11 1a>.    pPH2->RTD_Symbol=%d", pPH2->RTD_Symbol);
        }
        else if (info_1a_check == -1)
        {
            pPH2->Time_cnt_info1_start = 0;

            pPH2->Retrain_flag = RETRAIN_INIT;

            pPH2->Ph2State.retrain_reason = PH2_DETECT_INFO1A_CRC_ERROR;
        }
    }

    if (pPH2->Time_cnt_info1_start && (pPH2->Time_cnt - pPH2->Time_cnt_info1_start) > (V34_INFO_1A_TIME + pPH2->RTD_Symbol - V34_PHASE2_SYS_DELAY + 100))
    {
        TRACE2("R11 timeout! %d - %d > 420", pPH2->Time_cnt, pPH2->Time_cnt_info1_start);
        TRACE1("pPH2->RTD_Symbol %d", pPH2->RTD_Symbol);
        pPH2->send_info1c_done        = 1;
        pPH2->Retrain_flag            = RETRAIN_INIT;
        pPH2->Time_cnt_info1_start    = 0;
        pPH2->Ph2State.retrain_reason = PH2_DETECT_INFO1A_TIMEOUT;
    }

    pPH2->dettone = 0;
}
#endif

#if SUPPORT_V34FAX
void Ph2_Detect_Info_h(Ph2Struct *pPH2)
{
    Info_hStruc *pIh = &(pPH2->Ih);
    SBYTE info_h_check;
    SBYTE dbit;

    dbit = Ph2_DPSK_Demod(pPH2);

    pPH2->uTimeout_Count ++;

    if (dbit >= 0)
    {
        info_h_check = Ph2_V34Detect_Info_h(dbit, pIh);

        if (info_h_check == 1)  /* CRC checked */
        {
            pPH2->uTimeout_Count = 0;
            TRACE0("Info h Detected");

            Ph2_Receive_Info_1(pPH2);

            pPH2->rx_vec_idx = 6;
        }
    }
    else
    {
        Ph2_Detect_Info_h_Init(pIh);
    }

    pPH2->dettone = 0;
}
#endif
