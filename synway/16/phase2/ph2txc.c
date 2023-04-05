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
/* Phase2 Call Modem Tx Vectors                                            */
/***************************************************************************/

#include <string.h>
#include "ph2ext.h"

/*******************************/
/* Vector 0                    */
/* for both CAll and ANS modem */
/*******************************/
void Ph2_Send_Silence_75MS(Ph2Struct *pPH2)
{
    UBYTE i;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        pPH2->PCMoutPtr[i] = 0;
    }

    /* Counter is first initialize in ph2_init.c, 75ms */
    Ph2_UpdateTxDelayCnt(pPH2);      /* next to send Info_0 */

    /* Ph2_Calc_Pre_Filter_Gain for SRATE Table is a heavy calculation */
    /* process, so we pre-calculate the gain value and also split     */
    /* the calculation into 6 DPSK symbol time for each symbol rate */
    /* "delay_counter_tx" is used as the symbol rate index 5-0.     */

    /* pPH2->delay_counter_tx from 30 to 0 inclusive */
    if ((pPH2->delay_counter_tx <= 30) && (pPH2->delay_counter_tx % 6) == 0)
    {
        i = (UBYTE)(QQMULQR15(pPH2->delay_counter_tx, q1_OVER_6));

        Ph2_Calc_Pre_Filter_Gain(pPH2->Probing.pqdGain, i);
    }
}

/**************/
/* Vector 1   */
/**************/
void Ph2_Send_Info_0c(Ph2Struct *pPH2)
{
    SWORD info_size;

#if 0 /* To dump Info0 */
    if (DumpTone1_Idx < MAX_DUMPDATA_SIZE) { DumpTone1[DumpTone1_Idx++] = pPH2->pInfo_buf[pPH2->info_idx]; }
#endif

    Ph2_DPSK_Modfilter(pPH2, pPH2->pInfo_buf[pPH2->info_idx++], V34_qCALL_MODEM_CARRIER);

#if SUPPORT_V90D

    if (pPH2->modem_mode == CALL_MODEM && pPH2->V90_Enable)
    {
        info_size = V34_INFO_0d_SIZE;
    }
    else
#endif
        info_size = V34_INFO_0_SIZE;

    if (pPH2->info_idx >= info_size)
    {
        if (pPH2->recovery)
        {
            /* TRACE1("T1 %d I0c",++jjjj); */
            pPH2->info_idx = 0;

            if (pPH2->recovery_info0_detected == 1)
            {
                pPH2->I0c.acknowledge_correct_recept = 1;

#if SUPPORT_V90D

                if (pPH2->modem_mode == CALL_MODEM && pPH2->V90_Enable)
                {
                    Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->I0c), ShiftTable0d);
                }
                else
#endif
                    Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->I0c), ShiftTable0);

                pPH2->recovery = 0;
                TRACE0("T1 recov>");
            }
        }
        else
        {
            pPH2->info_idx = 0;
            pPH2->tx_vec_idx = 2;  /* No error recovery */

            pPH2->delay_counter_tx = 0;
            TRACE0("T1 I0c>");
        }
    }
}

/****************/
/* Vector 2, 7  */
/****************/
void Ph2_Create_Tone_B(Ph2Struct *pPH2)
{
    UBYTE ubTemp;

    Ph2_DPSK_Mod(pPH2, 0, V34_qCALL_MODEM_CARRIER);

    /* SRATE Table is a heavy calculation process, so we split      */
    /* the calculation into 6 DPSK symbol time for each symbol rate */
    /* "delay_counter_tx" is used as the symbol rate index 1-4.     */
    if ((pPH2->tx_vec_idx == 7) && ((pPH2->delay_counter_tx & 0x7) == 0))
    {
        ubTemp = (pPH2->delay_counter_tx >> 3) & 0xFF;

        if (ubTemp >= 1 && ubTemp <= 4)/* For symbol rate 1 to 4 */
        {
            Ph2_Create_SymRate_Table(pPH2, &(pPH2->I0c), &(pPH2->I0a), ubTemp);
        }
    }

    pPH2->delay_counter_tx ++;
}

/*
* Call modem Tx Vector 3, 8
* Keep on sending Tone B
* switch to Send Tone B phase reversal after 40ms
*/
void Ph2_Create_Tone_B_40MS(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 0, V34_qCALL_MODEM_CARRIER);

    Ph2_UpdateTxDelayCnt(pPH2);

    if (pPH2->delay_counter_tx < 0)    /* do next state function immediately */
    {
        TRACE0("T3-4-5 ");
        pPH2->pfTx[pPH2->tx_vec_idx](pPH2);
    }
}

/*
* Call modem Tx Vector 4
* Send Tone B phase reversal
* Set RTD counter to 0
*/
void Ph2_Tone_B_Phase_Rev_RTD(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 1, V34_qCALL_MODEM_CARRIER);

    pPH2->delay_counter_tx = V34_PHASE2_10MS;
    pPH2->tx_vec_idx = 5;

    pPH2->RTD_Status = 1;        /* Start counting RTD */
    pPH2->RTD_Symbol = 0;
}

#if 0 /// LLL temp
/************/
/* Vector 5 */
/************/
void Ph2_Create_Tone_B_10MS(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 0, V34_qCALL_MODEM_CARRIER);

    Ph2_UpdateTxDelayCnt(pPH2);
}
#endif

/**************************************/
/* Vector 6, 13           not 15      */
/**************************************/
void Ph2_Send_Silence(Ph2Struct *pPH2)
{
    Ph2_DPSK_Silence(pPH2);
}

#if 0 /// LLL temp
/*************/
/* Vector 9  */
/*************/
void Ph2_Tone_B_Phase_Rev(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 1, V34_qCALL_MODEM_CARRIER);

    /* SRATE Table is a heavy calculation process, so we split      */
    /* the calculation into 6 DPSK symbol time for each symbol rate */

    /* For symbol rate 0 and 5 */
    Ph2_Create_SymRate_Table(pPH2, &(pPH2->I0c), &(pPH2->I0a), 0);
    Ph2_Create_SymRate_Table(pPH2, &(pPH2->I0c), &(pPH2->I0a), 5);

    pPH2->delay_counter_tx = V34_PHASE2_10MS;
    pPH2->tx_vec_idx       = 10;
}
#endif

/*************/
/* Vector 10 */
/*************/
void Ph2_Create_Tone_B_10MS_Before_L1(Ph2Struct *pPH2)
{
    Ph2_DPSK_Mod(pPH2, 0, V34_qCALL_MODEM_CARRIER);

    pPH2->delay_counter_tx--;

    if (pPH2->delay_counter_tx <= 0)
    {
#if SUPPORT_V34FAX

        if (pPH2->V34fax_Enable == 1)
        {
            pPH2->tx_vec_idx++;
        }
        else
#endif
            pPH2->tx_vec_idx = 11;                        /* Send L1 Probing Tone next */

        pPH2->delay_counter_tx = V34_PHASE2_160MS;    /* Time for sending L1 */
        pPH2->L2_done = 2;                            /* L1/L2 echo presence */
        pPH2->L1_counter = 0;
        TRACE0("T10 B>");
    }
}

/***************************/
/* Call modem Tx Vector 11 */
/*      send L1 for 160ms  */
/***************************/
void Ph2_Create_L1(Ph2Struct *pPH2)
{
    Ph2_Create_Probing(pPH2, V34_qL1_AMPLITUDE, &(pPH2->L1_counter));

    if (--pPH2->delay_counter_tx <= 0)
    {
        /* Condition Transmitter to transmit L2 tones */
        pPH2->delay_counter_tx = V34_PHASE2_650MS + pPH2->RTD_Symbol;

        pPH2->delay_counter_rx = pPH2->RTD_Symbol + V34_PHASE2_SYS_DELAY;

#if SUPPORT_V34FAX

        if (pPH2->V34fax_Enable == 1)
        {
            pPH2->tx_vec_idx++;
            pPH2->rx_vec_idx++;
        }
        else
#endif
        {
            pPH2->tx_vec_idx = 12; /* next state: Send L2 */
            pPH2->rx_vec_idx = 9;  /* next state: Detect TONE A2 */
        }

        pPH2->Time_cnt_tone_start = pPH2->Time_cnt; /* init time counter for detect Tone A2  */

        pPH2->toneb1_detected = 0;

        pPH2->toneA2_detected = 0;

        pPH2->L2_counter = 0;
        TRACE0("T11 L1>");
    }
}

/**************/
/* Vector 12  */
/**************/
void Ph2_Create_L2(Ph2Struct *pPH2)
{
    Ph2_Create_Probing(pPH2, V34_qL2_AMPLITUDE, &(pPH2->L2_counter));

    /* Send L2 for 500MS + RTD, then send INFO1c, */
    /* no matter we detect reponse tone A2 or not.*/
#if 1  /* for retrain test */
    pPH2->delay_counter_tx --;

    if ((pPH2->delay_counter_tx <= 0) || (pPH2->toneA2_detected > 6))
    {
        pPH2->L2_done = 1;
    }

#endif
}

#if 0 /// LLL temp
/*************/
/* Vector 13 */
/*************/
void Ph2_Setup_Info1c(Ph2Struct *pPH2)
{
    UBYTE *pB;

    /* While calculating SRATE table, continue to send L2 signal */
    Ph2_Create_Probing(pPH2, V34_qL2_AMPLITUDE, &(pPH2->L2_counter));

    if (pPH2->I0a.reduce_transmit_power == 1)
    {
        pPH2->I1c.min_power_reduction = 0;    /* for testing, Probing */
        pPH2->I1c.add_power_reduction = 0;    /* exact values to be calculated */
    }
    else
    {
        pPH2->I1c.min_power_reduction = 0;
        pPH2->I1c.add_power_reduction = 0;
    }

    pPH2->I1c.length_MD = 0;                /* MD is not generated */

    /* Symbol rate 2400Hz probing result */
    pPH2->I1c.high_carrier_freq_S2400 = pPH2->pSymRate_info[0].carrier_freq_idx;
    pPH2->I1c.pre_em_index_S2400      = pPH2->pSymRate_info[0].preemphasis_idx;

    if (pPH2->min_symbol_rate > V34_SYM_2400)
    {
        pPH2->I1c.pro_data_rate_S2400 = 0;
    }
    else
    {
        pPH2->I1c.pro_data_rate_S2400 = pPH2->pSymRate_info[0].project_data_rate;
    }

    /* Symbol rate 2743Hz probing result */
    pPH2->I1c.high_carrier_freq_S2743 = pPH2->pSymRate_info[V34_SYM_2743].carrier_freq_idx;
    pPH2->I1c.pre_em_index_S2743      = pPH2->pSymRate_info[V34_SYM_2743].preemphasis_idx;

    if ((pPH2->min_symbol_rate <= V34_SYM_2743) && (pPH2->max_symbol_rate >= V34_SYM_2743))
    {
        pPH2->I1c.pro_data_rate_S2743 = pPH2->pSymRate_info[V34_SYM_2743].project_data_rate;
    }
    else
    {
        pPH2->I1c.pro_data_rate_S2743 = 0;
    }

    /* Symbol rate 2800Hz probing result */
    pPH2->I1c.high_carrier_freq_S2800 = pPH2->pSymRate_info[V34_SYM_2800].carrier_freq_idx;
    pPH2->I1c.pre_em_index_S2800      = pPH2->pSymRate_info[V34_SYM_2800].preemphasis_idx;

    if ((pPH2->min_symbol_rate <= V34_SYM_2800) && (pPH2->max_symbol_rate >= V34_SYM_2800))
    {
        pPH2->I1c.pro_data_rate_S2800 = pPH2->pSymRate_info[V34_SYM_2800].project_data_rate;
    }
    else
    {
        pPH2->I1c.pro_data_rate_S2800 = 0;
    }

    /* Symbol rate 3000Hz probing result */
    pPH2->I1c.high_carrier_freq_S3000 = pPH2->pSymRate_info[V34_SYM_3000].carrier_freq_idx;
    pPH2->I1c.pre_em_index_S3000      = pPH2->pSymRate_info[V34_SYM_3000].preemphasis_idx;

    if ((pPH2->min_symbol_rate <= V34_SYM_3000) && (pPH2->max_symbol_rate >= V34_SYM_3000))
    {
        pPH2->I1c.pro_data_rate_S3000 = pPH2->pSymRate_info[V34_SYM_3000].project_data_rate;
    }
    else
    {
        pPH2->I1c.pro_data_rate_S3000 = 0;
    }

    /* Symbol rate 3200Hz probing result */
    pPH2->I1c.high_carrier_freq_S3200 = pPH2->pSymRate_info[V34_SYM_3200].carrier_freq_idx;
    pPH2->I1c.pre_em_index_S3200      = pPH2->pSymRate_info[V34_SYM_3200].preemphasis_idx;

    if ((pPH2->min_symbol_rate <= V34_SYM_3200) && (pPH2->max_symbol_rate >= V34_SYM_3200))
    {
        pPH2->I1c.pro_data_rate_S3200 = pPH2->pSymRate_info[V34_SYM_3200].project_data_rate;
    }
    else
    {
        pPH2->I1c.pro_data_rate_S3200 = 0;
    }

    /* Symbol rate 3429Hz probing result */
    pPH2->I1c.high_carrier_freq_S3429 = pPH2->pSymRate_info[V34_SYM_3429].carrier_freq_idx;
    pPH2->I1c.high_carrier_freq_S3429 = 1;    //???

    pPH2->I1c.pre_em_index_S3429      = pPH2->pSymRate_info[V34_SYM_3429].preemphasis_idx;

    if ((pPH2->min_symbol_rate <= V34_SYM_3429) && (pPH2->max_symbol_rate >= V34_SYM_3429))
    {
        pPH2->I1c.pro_data_rate_S3429 = pPH2->pSymRate_info[V34_SYM_3429].project_data_rate;
    }
    else
    {
        pPH2->I1c.pro_data_rate_S3429 = 0;
    }

    pPH2->I1c.freq_offset = -512;        /* ignore freq offset */

    pB = pPH2->pInfo_buf1;
    /* !! somehow Info_1c might not be detected */
    /**pB++ = 1; */        /* !! if these 2 are not included, however, */
    /**pB++ = 1; */        /* !! the trailing fill bits will be        */
    /* !! reduced by 2 then !                   */

    Ph2_CreateInfo(pB, (UBYTE *)&(pPH2->I1c.min_power_reduction), ShiftTable1c);
    pPH2->info_idx = 0;

    Ph2_NLD_Analyze(pPH2);

    Ph2_DPSK_Init(&(pPH2->Dpsk), pPH2->modem_mode); /* Prepare to send Info1c */

    pPH2->tx_vec_idx = 14;        /* next state: go on to send Info 1c */
}

/**************/
/* Vector 14  */
/**************/
void Ph2_Send_Info_1c(Ph2Struct *pPH2)
{
    UBYTE bit;

    if (pPH2->info_idx < V34_INFO_1C_SIZE)
    {
        bit = pPH2->pInfo_buf1[pPH2->info_idx];

#if 0 /* To dump Info1c */

        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = bit; }

#endif
    }
    else
    {
        bit = 1;
    }

    Ph2_DPSK_Modfilter(pPH2, bit, V34_qCALL_MODEM_CARRIER);

    if (++pPH2->info_idx >= (V34_INFO_1C_SIZE + 3))
    {
        pPH2->tx_vec_idx = 15;     /* next state: Send Silence */
        pPH2->Time_cnt_info1_start = pPH2->Time_cnt; /* init time counter for detect Tone info 1a  */
        TRACE1("T14 t=%d", pPH2->Time_cnt);

        /* Prepare to detect Info1a */
        Ph2_DPSK_Detect_Init(&pPH2->Dpsk);

        pPH2->rx_vec_idx = 11;     /* next state: Phase2_Detect_INFO_1A */
    }
}
#endif

