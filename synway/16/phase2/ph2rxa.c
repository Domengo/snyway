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

/***************************************************************************/
/* Phase2 Answer Modem Tx Vectors                                          */
/***************************************************************************/

#include <string.h>
#include "ph2ext.h"

/**************************************************/
/* Vector 0 for both CALL & ANS modem             */
/**************************************************/
void Ph2_Detect_Info_0(Ph2Struct *pPH2)
{
    Info_0Struc *pInfo;
    SBYTE result;
    SBYTE dbit;

    if (pPH2->modem_mode == CALL_MODEM)
    {
        pInfo = &(pPH2->I0a);
    }
    else  /* if (pPH2->modem_mode == ANS_MODEM) */
    {
        pInfo = &(pPH2->I0c);
    }

    pPH2->uINFO0_detect_Timeout_Count ++;

    pPH2->delay_counter_rx--;

    dbit = Ph2_DPSK_Demod(pPH2);

    Ph2_DetCheck_ToneAB(pPH2, 5);

    /* Haven't find frame Sync in INFO sequence but find TONE B */

    if ((pPH2->recovery == 0) && (pPH2->Dpsk.DPSK_Flag == 0) && pPH2->dettone)
    {
        TRACE0("R0t");
        pPH2->recovery      = 1;
        pPH2->init_recovery = 1;
        pPH2->tx_vec_idx    = 1;
        /* pPH2->rx_vec_idx  = 0; */  /* already in state 0 */

        pPH2->uINFO0_detect_Timeout_Count = 0;
    }
    else
    {
        if (dbit >= 0)
        {
            pPH2->SNR_test_End = 1;

#if SUPPORT_V90A

            if (pPH2->modem_mode == ANS_MODEM && pPH2->V90_Enable)
            {
                result = Ph2_Detect_Info_0d(dbit, pInfo);
            }
            else
#endif
                result = Ph2_V34Detect_Info_0(dbit, pInfo);

            if (result == 1)            /* Info 0 detected */
            {
#if SUPPORT_V92

                if ((pPH2->I0a.transmit_clock_source & 2) && (pPH2->I0c.transmit_clock_source & 1))
                {
                    pPH2->Short_Phase2   = 1;
                }

                if ((pPH2->I0a.transmit_clock_source & 1) && (pPH2->I0c.transmit_clock_source & 2))
                {
                    pPH2->V92_Capability = 1;
                }

                if ((pPH2->Short_Phase2) && (pPH2->V92_Capability))
                {
                    pPH2->V92_Enable     = 1;    /* We don't receive Info1D in a short Phase2, so enable V92 here */
                }

                TRACE2("V.92:%d, short ph2:%d", pPH2->V92_Capability, pPH2->Short_Phase2);

                if ((pPH2->Short_Phase2) && (pPH2->tx_vec_idx == 2))
                {
                    pPH2->tx_vec_idx     = 4;    /* Send ToneA Continuous */
                }

                //if (pPH2->I0c.dig_max_transmit_power < 23)
                //    pPH2->ubDownstreamPowerIdx = 3;
#endif

                if (pPH2->recovery)
                {
                    /* -- acknowledge Info 0 detection -- */
                    pPH2->recovery_info0_detected = 1;

                    if (pPH2->init_recovery || pInfo->acknowledge_correct_recept == 1)
                    {
                        pPH2->bit_0_count = 0;
                        Ph2_Detect_Info_0_Init(pInfo);
                        Ph2_Detect_Info_0_Init(&pPH2->I0check);
                        pPH2->rx_vec_idx  = 1;   /* next state */
                    }

                    TRACE0("R0r");

                    pPH2->uINFO0_detect_Timeout_Count = 0;
                }
                else
                {
                    /* -- error free detection -- */
                    pPH2->bit_0_count = 0;

                    Ph2_Detect_Info_0_Init(pInfo);

                    Ph2_Detect_Info_0_Init(&(pPH2->I0check));

                    pPH2->rx_vec_idx  = 1;    /* next state: Detect Tone B or Tone A */

                    Ph2_DPSK_Detect_Init(&pPH2->Dpsk);

                    pPH2->recovery    = 0;
                    TRACE0("R0 I0>");

                    pPH2->uINFO0_detect_Timeout_Count = 0;
                }
            }
            else if (result == -1)            /* (crc) error in Info 0 detected */
            {
                pPH2->recovery      = 1;
                pPH2->init_recovery = 1;
                pPH2->bit_0_count   = 0;
                /*              pPH2->rx_vec_idx    = 0; */ /* keep on to Detect Info0c */
                pPH2->tx_vec_idx    = 1;    /* Repeat send Info0a */

                /* Prepare to detect Info0 */
                Ph2_DPSK_Detect_Init(&pPH2->Dpsk);

                Ph2_Detect_Info_0_Init(pInfo);

                TRACE0("R0x");

                pPH2->uINFO0_detect_Timeout_Count = 0;
            }
        }
    }

    pPH2->dettone = 0;
}

void PrepareToRecoverInfo0a(Ph2Struct *pPH2)
{
    pPH2->bit_0_count = 0;

    if (pPH2->info_idx >= V34_INFO_0_SIZE)
    {
        pPH2->info_idx  = 0;
    }

    /* prepare to send info0a with ACK to 1 */
    pPH2->I0a.acknowledge_correct_recept = 1;

#if SUPPORT_V90A

    if (pPH2->modem_mode == ANS_MODEM && pPH2->V90_Enable)
    {
        pPH2->I0a.transmit_from_CME = 1;
    }

#endif

    Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->I0a), ShiftTable0);
    pPH2->tx_vec_idx = 1;  /* go to send info0a with ACK set to 1 */
    pPH2->rx_vec_idx = 1;

    pPH2->recovery      = 1;  /* go to recovery */
    pPH2->init_recovery = 0;

    pPH2->RTD_Status = 0;
    pPH2->RTD_Symbol = 0;
    pPH2->LineProbOn = 0;
}


/**********************************/
/* Vector 1                       */
/**********************************/
void Ph2_Det_Tone_B(Ph2Struct *pPH2)
{
    QDWORD qdTemp;
    QWORD  qTemp;
    SBYTE  bit;
    SBYTE  INFO0_result;

    pPH2->uToneAorB_Timeout_Count ++;

    Ph2_Biquad_Tone_Det(pPH2);

    /* feed signal into DPSK modem for correct detection */
    bit = Ph2_DPSK_Demod(pPH2);

    if (bit >= 0)
    {
#if SUPPORT_V90A

        if (pPH2->modem_mode == ANS_MODEM && pPH2->V90_Enable)
        {
            INFO0_result = Ph2_Detect_Info_0d(bit, &(pPH2->I0check));
        }
        else
#endif
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

    if ((pPH2->qdTotal_egy > V34_TONE_ENERGY) && (pPH2->qdTotal_egy > pPH2->qdTotal_egy_biqin))
    {
        pPH2->dettone++;
    }
    else
    {
        pPH2->dettone = 0;
    }

#endif

    /* for recovery only */
    if ((pPH2->dettone > 6) && (pPH2->bit_0_count > 10))
    {
        pPH2->uToneAorB_Timeout_Count = 0;

        pPH2->recovery = 0;  /* go to normal */

#if SUPPORT_V92A

        if ((pPH2->Short_Phase2) && (pPH2->bit_0_count > 15))
        {
            pPH2->rx_vec_idx = 2;  /* next state: Detect TONE B Phase Change */
            pPH2->init_recovery = FALSE;
        }

#endif

        if (pPH2->bit_0_count > 20)
        {
            /* TRACE2("pPH2->dettone=%d, pPH2->bit_0_count=%d", pPH2->dettone, pPH2->bit_0_count); */

            if ((pPH2->delay_counter_tx < 0) && (pPH2->tx_vec_idx == 2))
            {
                pPH2->bit_0_count   = 0;
                pPH2->tx_vec_idx    = 3;  /* next state: Create TONE A Phase change */
                pPH2->rx_vec_idx    = 2;  /* next state: Detect TONE B Phase Change */
                pPH2->init_recovery = 0;
                pPH2->dettone       = 0;
                TRACE0("R1 B>");

                TRACE1("pPH2->qdTotal_egy = %" FORMAT_DWORD_PREFIX "d", pPH2->qdTotal_egy);

                qTemp = (QWORD)(QDsqrt(pPH2->qdTotal_egy));
                qdTemp = QQMULQD(pPH2->AGC_gain, PH2_AGC_REFERENCE_B);
                pPH2->AGC_gain = (QWORD)QDQDdivQD(qdTemp, qTemp);
                TRACE2("Ph2_B_egy = %d, pPH2->AGC_gain = %d", qTemp, pPH2->AGC_gain);
            }
            else if (pPH2->tx_vec_idx == 1)
            {
                pPH2->recovery = 0;
            }
        }
    }

#if SUPPORT_V92A_MOH

    if (INFO0_result == MH_SEQUENCE_DETECTED)
    {
        Ph2_Init_MH_Response(pPH2);
    }

#endif

    if (INFO0_result == 1 && (!pPH2->init_recovery))
    {
        pPH2->uToneAorB_Timeout_Count = 0;

        if (pPH2->I0check.acknowledge_correct_recept == 0)
        {
            PrepareToRecoverInfo0a(pPH2);       /* !! assume RTDs also zero here !! */
            TRACE0("R1 rep I0!");
        }
        else
        {
            pPH2->recovery = 0;
            pPH2->bit_0_count = 0;
            TRACE0("R1 IO recov");
        }
    }
}


/***************/
/* Vector 2    */
/***************/
void Ph2_Det_Tone_B_PV_40MS(Ph2Struct *pPH2)
{
    SBYTE dbit, INFO0_result;
    SWORD rtde_temp;
    UBYTE phaseReverseIsDetected;

#if 0
    UBYTE i;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pPH2->PCMinPtr[i]; }
    }

#endif

    Ph2_Biquad_Tone_Det(pPH2);

    dbit = Ph2_DPSK_Demod(pPH2);

#if 0

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = dbit; }

#endif

    if (dbit >= 0)
    {
#if SUPPORT_V90A

        if (pPH2->modem_mode == ANS_MODEM && pPH2->V90_Enable)
        {
            INFO0_result = Ph2_Detect_Info_0d(dbit, &(pPH2->I0check));
        }
        else
#endif
            INFO0_result = Ph2_V34Detect_Info_0(dbit, &(pPH2->I0check));
    }
    else
    {
        INFO0_result = 0;
    }

    phaseReverseIsDetected = Ph2_DPSK_PhaseReversal_Detected(pPH2);

#if SUPPORT_V92A

    if ((pPH2->Short_Phase2) && (phaseReverseIsDetected))
    {
        pPH2->delay_counter_tx = V34_PHASE2_40MS - V34_PHASE2_SYS_DELAY;
        pPH2->tx_vec_idx = 5; /* next state: Send TONE A for 40MS at line terminal */
        pPH2->rx_vec_idx = 3; /* next state: Detect DUMMY */
    }

#endif

    if (pPH2->RTD_Status == 1)
    {
        pPH2->RTD_Symbol++;

        if ((pPH2->RTD_Symbol >= V34_PHASE2_40MS) && phaseReverseIsDetected)
        {
            rtde_temp = pPH2->RTD_Symbol - (V34_PHASE2_40MS + V34_PHASE2_SYS_DELAY);

            if (rtde_temp >= -2)
            {
#if SUPPORT_V34FAX

                if (pPH2->V34fax_Enable == 1)
                {
                    /* Receive L1 Probing tones for 160MS */
                    pPH2->delay_counter_rx = V34_PHASE2_160MS;

                    pPH2->LineProbOn = 1;
                }
                else
#endif
                {
                    pPH2->delay_counter_tx = V34_PHASE2_40MS - V34_PHASE2_SYS_DELAY;
                }

                pPH2->tx_vec_idx = 5; /* next state: Send TONE A for 40MS at line terminal */
                pPH2->rx_vec_idx = 3; /* next state: Detect DUMMY */
                /* the actual RTD is the Value minus (40MS+System Delay) */
                pPH2->RTD_Symbol = rtde_temp;

                TRACE1("pPH2->RTD_Symbol=%d", pPH2->RTD_Symbol);

                pPH2->RTD_Status = 0;
                TRACE0("R2 B->");
            }
        }
    }

    if (INFO0_result == 1)
    {
        PrepareToRecoverInfo0a(pPH2);
        TRACE0("R2 rep I0");
    }

#if SUPPORT_V92A_MOH

    if (INFO0_result == MH_SEQUENCE_DETECTED)
    {
        Ph2_Init_MH_Response(pPH2);
    }

#endif

    /* add time out test for detect tone B Phase Reversal (2000ms) Yanghua */
    if ((++pPH2->Time_cnt_B_PR_40MS) > V34_TIME_B_PR_40MS)
    {
        /* pPH2->Retrain_flag = RETRAIN_INIT; */

        pPH2->delay_counter_tx = V34_PHASE2_160MS;
        /* 160ms for V90a to work with some ISP in Japan, at least 50ms by ITU */
        /* Notice: 160ms requirement is only our guess, this has never been confirmed
        by Baseline */
        pPH2->tx_vec_idx = 2;  /* Go to send tone A */
        pPH2->rx_vec_idx = 1;  /* Detect TONE B */
        pPH2->Time_cnt_B_PR_40MS = 0;
        TRACE0("R2 rtrn ->R1");
    }
}

#if 0 /// LLL temp
/***************/
/* Vector 3    */
/***************/
void Ph2_Det_Recovery(Ph2Struct *pPH2)
{
    SBYTE dbit, INFO0_result;

    Ph2_Biquad_Tone_Det(pPH2);

    dbit = Ph2_DPSK_Demod(pPH2);

    if (dbit >= 0)
    {
#if SUPPORT_V90A

        if (pPH2->modem_mode == ANS_MODEM && pPH2->V90_Enable)
        {
            INFO0_result = Ph2_Detect_Info_0d(dbit, &(pPH2->I0check));
        }
        else
#endif
            INFO0_result = Ph2_V34Detect_Info_0(dbit, &(pPH2->I0check));

        if (INFO0_result == 1)
        {
            PrepareToRecoverInfo0a(pPH2);
            TRACE0("R3 rep I0");
        }

#if SUPPORT_V92A_MOH

        if (INFO0_result == MH_SEQUENCE_DETECTED)
        {
            Ph2_Init_MH_Response(pPH2);
        }

#endif
    }
}
#endif

/***************/
/* Vector 4    */
/***************/
void Ph2_Det_Tone_B1(Ph2Struct *pPH2)
{
    /* biquad tone detection */
    Ph2_Biquad_Tone_Det(pPH2);

    Ph2_DPSK_Demod(pPH2);

#if 0

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pPH2->qdTotal_egy; }

#endif

#if SUPPORT_V34FAX

    if (pPH2->V34fax_Enable == 1)
    {
        if (pPH2->qdTotal_egy > PH2_B1TONE_ENERGY_REF)
        {
            pPH2->dettone ++;
        }
        else
        {
            pPH2->dettone = 0;
        }

        /* skip L2 tone or A tone */
        if ((pPH2->dettone > 50) && (pPH2->rx_vec_idx == 5))
        {
            //TRACE0("B tone detected")
            pPH2->tx_vec_idx++;  /* send INFOh */
            pPH2->rx_vec_idx++;
        }
    }
    else
#endif
    {
        pPH2->delay_counter_rx --;

        if ((pPH2->delay_counter_rx <= 0) && (pPH2->qdTotal_egy > V34_TONE_ENERGY))
        {
            pPH2->dettone ++;
        }
        else
        {
            pPH2->dettone = 0;
        }

        if (pPH2->dettone > 6)
        {
            pPH2->toneb1_detected = 1;           /* inform Tx to quit state 9 */
        }
        else
        {
            pPH2->toneb1_detected = 0;
        }

        /* time out test after send L1 */
        if (pPH2->Time_cnt_tone_start)
        {
            if ((pPH2->Time_cnt - pPH2->Time_cnt_tone_start) > (V34_TONEB1_TIME + pPH2->RTD_Symbol))
            {
                pPH2->Retrain_flag = RETRAIN_INIT;
                pPH2->Ph2State.retrain_reason = PH2_DETECT_TONEB1_TIMEOUT;
                pPH2->Time_cnt_tone_start = 0;
                pPH2->Time_cnt_info1_start = pPH2->Time_cnt;  /* init the time out for detect info 1c */
                TRACE0("R4 rtrn");
            }
        }
    }
}

#if 0 /// LLL temp
/*************/
/* Vector 5  */
/*************/
void Ph2_Det_Tone_B_PV_670MS(Ph2Struct *pPH2)
{
    SBYTE bit;

    pPH2->uTimeout_Count ++;

    bit = Ph2_DPSK_Demod(pPH2);

    if ((bit == -11) || (bit == 1))
    {
        pPH2->uTimeout_Count = 0;

        /* Receive L1 Probing tones for 160MS */
        pPH2->delay_counter_rx = V34_PHASE2_160MS;

        pPH2->rx_vec_idx = 6; /* next state */

        pPH2->LineProbOn = 1;

        TRACE0("R5 L1>");
    }
}

/*************/
/*vector 11  */
/*************/
void Ph2_Det_B_Recovery(Ph2Struct *pPH2)
{
    /* feed signal into DPSK modem for correct detection */
    Ph2_DPSK_Demod(pPH2);

    Ph2_DetCheck_ToneAB(pPH2, 2);

    /* for recovery only */
    if (pPH2->dettone == 1)
    {
        pPH2->recovery = 0;    /* go to normal */
    }

    if (pPH2->delay_counter_tx <= 0)
    {
        if ((pPH2->info_idx >= V34_INFO_0_SIZE) && (pPH2->dettone == 1))
        {
            pPH2->bit_0_count = 0;
            pPH2->tx_vec_idx  = 3;  /* go to send tone A PR */
            pPH2->rx_vec_idx  = 2;  /* go to detect tone B PR */
            TRACE0("R11 B recov");
        }
    }

    pPH2->dettone = 0;
}

/*******************************************/
/* Vector 8                                */
/*******************************************/
void Ph2_Detect_Info_1c(Ph2Struct *pPH2)
{
    SBYTE info_1c_check;
    SBYTE dbit;

    /* Ph2_Biquad_Tone_Det(pPH2); */

    dbit = Ph2_DPSK_Demod(pPH2);

    /* check retrain tone */
    Ph2_DetCheck_ToneAB(pPH2, 7);

    if (pPH2->dettone == 1) /* && pPH2->qdTotal_egy > V34_TONE_ENERGY) */
    {
        pPH2->delay_counter_tx = V34_PHASE2_50MS;
        pPH2->tx_vec_idx = 2;  /* go to send tone A */
        pPH2->rx_vec_idx = 1;
        pPH2->info_idx = 0;
        pPH2->Ph2State.retrain_reason = PH2_DETECT_RETRAIN_IN_DET_INFO1C;

        /* for retrain init */
        pPH2->init_recovery = 0;

        pPH2->Time_cnt_tonea1_start = 0;

        Ph2_RetrainParamInit(pPH2);

        pPH2->Time_cnt_info1_start = 0;

        TRACE0("R8 rtrn");
        /* for retrain init */
    }

    /* end of tone check */

    if (dbit >= 0)
    {
        info_1c_check = Ph2_V34Detect_Info_1c(dbit, &(pPH2->I1c));

        if (info_1c_check == 1)  /* CRC checked */
        {
            pPH2->Time_cnt_info1_start = 0;

            pPH2->tx_vec_idx = 15;  /* next state: Setup Info1a */
            pPH2->rx_vec_idx = 9;   /* next state: Received Info1c setup */
            pPH2->delay_counter_tx = 0;
#if SUPPORT_V92A
            pPH2->PCM_support = pPH2->I1c.high_carrier_freq_S3429;

            if ((pPH2->PCM_support) && (pPH2->V92_Capability))
            {
                pPH2->V92_Enable = 1;
            }
            else
            {
                pPH2->V92_Enable = 0;
            }

            TRACE1("V92:%d", pPH2->V92_Enable);
#endif
            TRACE0("R8 I1c>");
        }
        else if (info_1c_check == -1) /* CRC is wrong */
        {
            pPH2->Time_cnt_info1_start = 0;

            pPH2->Retrain_flag = RETRAIN_INIT;

            pPH2->Ph2State.retrain_reason = PH2_DETECT_INFO1C_CRC_ERROR;
        }
    }

    if (pPH2->Time_cnt_info1_start && (pPH2->Time_cnt - pPH2->Time_cnt_info1_start) > (V34_INFO_1C_TIME + pPH2->RTD_Symbol - V34_PHASE2_SYS_DELAY + 100))
    {
        TRACE2("Detect INFO1c timeout! %d - %d > 1200", pPH2->Time_cnt, pPH2->Time_cnt_info1_start);
        TRACE1("pPH2->RTD_Symbol %d", pPH2->RTD_Symbol);
        pPH2->Retrain_flag            = RETRAIN_INIT;
        pPH2->Time_cnt_info1_start    = 0;
        pPH2->Ph2State.retrain_reason = PH2_DETECT_INFO1C_TIMEOUT;
    }
}
#endif

/*******************************************************/
/* Vector 9         also called in ph2rxc vector 11    */
/*******************************************************/
void Ph2_Receive_Info_1(Ph2Struct *pPH2)
{
#if SUPPORT_V90
    Info_1aStruc *pI1a = &(pPH2->I1a);
    UBYTE *pData, *pInfo;
#endif

#if SUPPORT_V90A

    if (pI1a->sym_rate_answer_to_call > 5)
    {
        pPH2->Ph2State.hang_up = PH2_NO_SYMBOL_RATE_SUPPORT;
    }

#endif

    if (pPH2->modem_mode == ANS_MODEM)
    {
        pPH2->rx_vec_idx = 10;    /* next state: Receive Dummy */
    }
    else
    {
        pPH2->Phase2_End = 1;

#if SUPPORT_V90A
        pPH2->TimingDrift_Enable = 1;
#endif

#if SUPPORT_V90D

        if ((pPH2->V90_Enable == 1) && (pI1a->sym_rate_call_to_answer != V90_SYM_8000))
        {
            pData = pI1a->pData_sequence_1a;
            pInfo = &pI1a->min_power_reduction;   /* beginning of Info content */
            Ph2_ShiftDataToInfo(pData, pInfo, ShiftTable1a);

            TRACE0("V90d fall back V34!");
            pPH2->V90_Enable = 0;
        }

#endif
    }
}


void Ph2_CarrierLoss_Detect(Ph2Struct *pPH2)
{
    UBYTE i;
    QDWORD egy = 0;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        egy += QQMULQD(pPH2->PCMinPtr[i], pPH2->PCMinPtr[i]);
    }

    pPH2->Silence_Count1 ++;

#if 0//DUMPDATA_FLAG

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pPH2->Silence_Egy; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pPH2->Silence_Egy_Ref; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pPH2->Silence_Detect_Count; }

#endif

    if (pPH2->Silence_Count1 <= 8) /* Setup Egy threshold */
    {
        pPH2->Silence_Egy_Ref += (egy >> 7);
    }
    else
    {
        pPH2->Silence_Count2 ++;

        pPH2->Silence_Egy += (egy >> 4);

        if (pPH2->Silence_Count2 == 8)
        {
            if (pPH2->Silence_Egy < pPH2->Silence_Egy_Ref)
            {
                pPH2->Silence_Detect_Count ++;    /* Silence Counter */
            }
            else
            {
                pPH2->Silence_Detect_Count = 0;
            }

            pPH2->Silence_Count2 = 0;
            pPH2->Silence_Egy    = 0;
        }
    }
}
