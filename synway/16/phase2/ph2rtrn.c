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

#include <string.h>
#include "ph2ext.h"
#include "mhsp.h"

void Ph2_RetrainParamInit(Ph2Struct *pPH2)
{
    pPH2->Ph2Timer   = V34PHASE2_TIMEOUT;
    pPH2->Time_cnt   = 0;
    pPH2->L1_counter = 0;
    pPH2->L2_counter = 0;

    pPH2->CRC = 0;

    pPH2->RTD_Symbol = 0;

    pPH2->dettone     = 0;
    pPH2->bit_0_count = 0;
    pPH2->tone_count  = 0;
    pPH2->recovery    = 0;
    pPH2->Time_cnt_tone_start  = 0;
    pPH2->Time_cnt_info1_start = 0;
    pPH2->Time_cnt_B_PR_40MS   = 0;
    pPH2->send_info1c_done     = 0;

    Ph2_Prob_Init(&(pPH2->Probing));

    Ph2_RxProbing_Init(&(pPH2->Probing));

    Ph2_DPSK_Init(&(pPH2->Dpsk), pPH2->modem_mode);

    Ph2_Detect_Info_0_Init(&(pPH2->I0check));

    Ph2_Detect_Info_1c_Init(&(pPH2->I1c));

    Ph2_Detect_Info_1a_Init(&(pPH2->I1a));
}

void Ph2_Rtrn_Init(Ph2Struct *pPH2)
{
    UBYTE i;

    pPH2->Phase2_End = 0;

    Ph2_RetrainParamInit(pPH2);

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        pPH2->PCMoutPtr[i] = 0;
    }

    pPH2->delay_counter_tx = V34_PHASE2_72MS - 13;

    pPH2->tx_vec_idx++;
}

void Ph2_Send_Rtrn_Silence(Ph2Struct *pPH2)
{
    UBYTE i;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        pPH2->PCMoutPtr[i] = 0;
    }

    pPH2->delay_counter_tx --;

    if (pPH2->delay_counter_tx <= 0)
    {
        pPH2->tx_vec_idx = 2;  /* go to send tone A or B */

        pPH2->info_idx = 0;

        if (pPH2->modem_mode == ANS_MODEM)
        {
            pPH2->delay_counter_tx = V34_PHASE2_50MS;
        }
        else
        {
            pPH2->delay_counter_tx = (V34_PHASE2_SYS_DELAY + 10);
        }
    }
}
