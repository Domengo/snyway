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

#include "ph2ext.h"

#if 0 /// LLL temp
CONST V34Ph2FnPtr CallTxVecTab[] =
{
    Ph2_Send_Silence_75MS,                /* Tx: State 0  */
    Ph2_Send_Info_0c,                    /* Tx: State 1  */
    Ph2_Create_Tone_B,                    /* Tx: State 2  */
    Ph2_Create_Tone_B_40MS,                /* Tx: State 3  */
    Ph2_Tone_B_Phase_Rev_RTD,            /* Tx: State 4  */
    Ph2_Create_Tone_B_10MS,                /* Tx: State 5  */
    Ph2_Send_Silence,                    /* Tx: State 6  */
    Ph2_Create_Tone_B,                    /* Tx: State 7  */
    Ph2_Create_Tone_B_40MS,                /* Tx: State 8  */
    Ph2_Tone_B_Phase_Rev,                /* Tx: State 9  */
    Ph2_Create_Tone_B_10MS_Before_L1,    /* Tx: State 10 */
    Ph2_Create_L1,                        /* Tx: State 11 */
    Ph2_Create_L2,                        /* Tx: State 12 */
    Ph2_Setup_Info1c,                    /* Tx: State 13 */
    Ph2_Send_Info_1c,                    /* Tx: State 14 */
    Ph2_Send_Silence,                    /* Tx: State 15 */

    Ph2_Rtrn_Init,                        /* Tx: State 16 */
    Ph2_Send_Rtrn_Silence                /* Tx: State 17 */
};

CONST V34Ph2FnPtr CallRxVecTab[] =
{
    Ph2_Detect_Info_0,                    /* Rx: State 0  */
    Ph2_Detect_Tone_A,                    /* Rx: State 1  */
    Ph2_Det_Tone_A_PV_40MS,                /* Rx: State 2  */
    Ph2_Det_Tone_A_PV_RTD,                /* Rx: State 3  */
    Ph2_Detect_L1,                        /* Rx: State 4  */
    Ph2_Detect_L2,                        /* Rx: State 5  */
    Ph2_Detect_Tone_A_After_L2,            /* Rx: State 6  */
    Ph2_Det_Tone_A_PV_40MS,                /* Rx: State 7  */
    Ph2_Biquad_Tone_Det,                /* Rx: State 8  */
    Ph2_Detect_Tone_A2,                    /* Rx: State 9  */
    Ph2_Dummy,                            /* Rx: State 10 */
    Ph2_Detect_Info_1a,                    /* Rx: State 11 */
    Ph2_Biquad_Tone_Det                    /* Rx: State 12 */
};


CONST V34Ph2FnPtr AnsTxVecTab[] =
{
    Ph2_Send_Silence_75MS,                /* Tx: State 0  */
    Ph2_Send_Info_0A,                    /* Tx: State 1  */
    Ph2_Create_Tone_A_50MS,                /* Tx: State 2  */
    Ph2_Create_Tone_A_Phase_Change,        /* Tx: State 3  */
    Ph2_Create_Tone_A,                    /* Tx: State 4  */
    Ph2_Create_Tone_A_40MS,                /* Tx: State 5  */
    Ph2_Create_Tone_A_PR_10MS,            /* Tx: State 6  */
    Ph2_Create_Tone_A_10MS,                /* Tx: State 7  */
    Ph2_Create_L1_ANS,                    /* Tx: State 8  */
    Ph2_Create_L2_ANS,                    /* Tx: State 9  */
    Ph2_Create_Tone_A_After_L2,            /* Tx: State 10 */
    Ph2_Create_Tone_A_PR_10MS,            /* Tx: State 11 */
    Ph2_Create_Tone_A_10MS,                /* Tx: State 12 */
    Ph2_Send_Silence,                    /* Tx: State 13 */
    Ph2_Create_Tone_A,                    /* Tx: State 14 */
    Ph2_Create_Tone_A_Set_1A,            /* Tx: State 15 */
    Ph2_Send_Info_1A,                    /* Tx: State 16 */
    Ph2_Cleanup_Tx,                        /* Tx: State 17 */

    Ph2_Rtrn_Init,                        /* Tx: State 18 */
    Ph2_Send_Rtrn_Silence                /* Tx: State 19 */
};


CONST V34Ph2FnPtr AnsRxVecTab[] =
{
    Ph2_Detect_Info_0,                    /* Rx: State 0  */
    Ph2_Det_Tone_B,                        /* Rx: State 1  */
    Ph2_Det_Tone_B_PV_40MS,                /* Rx: State 2  */
    Ph2_Det_Recovery,                    /* Rx: State 3  */
    Ph2_Det_Tone_B1,                    /* Rx: State 4  */
    Ph2_Det_Tone_B_PV_670MS,            /* Rx: State 5  */
    Ph2_Detect_L1,                        /* Rx: State 6  */
    Ph2_Detect_L2,                        /* Rx: State 7  */
    Ph2_Detect_Info_1c,                    /* Rx: State 8  */
    Ph2_Receive_Info_1,                    /* Rx: State 9  */
    Ph2_Biquad_Tone_Det,                /* Rx: State 10 */
    Ph2_Det_B_Recovery                    /* Rx: 11 switch line  */
};

#elif SUPPORT_V34FAX
CONST V34Ph2FnPtr CallTxVecTab_V34fax[] =
{
    Ph2_Send_Silence_75MS,            /* Tx: State 0  */
    Ph2_Send_Info_0c,                 /* Tx: State 1  */
    Ph2_Create_Tone_B,                /* Tx: State 2  */
    Ph2_Create_Tone_B_40MS,           /* Tx: State 3  */
    Ph2_Tone_B_Phase_Rev_RTD,         /* Tx: State 4  */

    Ph2_Create_Tone_B_10MS_Before_L1, /* Tx: State 5  */
    Ph2_Create_L1,                    /* Tx: State 6  */
    Ph2_Create_L2,                    /* Tx: State 7  */
    Ph2_Create_Tone_B,                /* Tx: State 8  */

    Ph2_Rtrn_Init,                      /* Tx: State 9  */
    Ph2_Send_Rtrn_Silence              /* Tx: State 10 */
};

CONST V34Ph2FnPtr CallRxVecTab_V34fax[] =
{
    Ph2_Detect_Info_0,             /* Rx: State 0  */
    Ph2_Detect_Tone_A,             /* Rx: State 1  */
    Ph2_Det_Tone_A_PV_40MS,        /* Rx: State 2  */

    Ph2_Dummy,                     /* Rx: State 3  */
    Ph2_Detect_Tone_A2,            /* Rx: State 4  */
    Ph2_Detect_Info_h,             /* Rx: State 5  */
    Ph2_Dummy,                     /* Rx: State 6  */
};

CONST V34Ph2FnPtr AnsTxVecTab_V34fax[] =
{
    Ph2_Send_Silence_75MS,         /* Tx: State 0  */
    Ph2_Send_Info_0A,              /* Tx: State 1  */
    Ph2_Create_Tone_A_50MS,        /* Tx: State 2  */
    Ph2_Create_Tone_A_Phase_Change,/* Tx: State 3  */

    Ph2_Create_Tone_A_10MS,        /* Tx: State 4  */
    Ph2_Send_Silence,              /* Tx: State 5  */
    Ph2_Create_Tone_A,             /* Tx: State 6  */
    Ph2_Create_Tone_A_Set_H,       /* Tx: State 7  */
    Ph2_Send_Info_H,               /* Tx: State 8  */
    Ph2_Cleanup_Tx,                /* Tx: State 9  */

    Ph2_Rtrn_Init,                   /* Tx: State 10 */
    Ph2_Send_Rtrn_Silence           /* Tx: State 11 */
};

CONST V34Ph2FnPtr AnsRxVecTab_V34fax[] =
{
    Ph2_Detect_Info_0,         /* Rx: State 0  */
    Ph2_Det_Tone_B,            /* Rx: State 1  */
    Ph2_Det_Tone_B_PV_40MS,    /* Rx: State 2  */

    Ph2_Detect_L1,             /* Rx: State 3  */
    Ph2_Detect_L2,             /* Rx: State 4  */
    Ph2_Det_Tone_B1,           /* Rx: State 5  */
    Ph2_CarrierLoss_Detect,    /* Rx: State 6  */
    Ph2_Det_Tone_B1,           /* Rx: State 7  */
};

#endif

/* init the call_modem_vector_table for phase 2 */
void Ph2_StateInit(Ph2Struct *pPH2)
{
    QWORD *pqDline;

    pPH2->delay_counter_tx = V34_PHASE2_72MS - 1;    /* Phase2_DPCS_Init use 1 SYMBOL */
    /* wait 70ms before training AGC in detect info0 otherwise we will use v8JM ehco and 75 silence to train AGC */
    /* For MIPs reason, preemphasis gains are pre-calculated during this period, */
    /* so delay_counter_tx should not be less than 30 symbols */

    pPH2->delay_counter_rx = 36;

    pqDline  = pPH2->biq_dline;

    if (pPH2->modem_mode == CALL_MODEM)
    {
#if SUPPORT_V34FAX

        if (pPH2->V34fax_Enable == 1)
        {
            pPH2->pfTx = CallTxVecTab_V34fax;
            pPH2->pfRx = CallRxVecTab_V34fax;
        }
        else
#endif
        {
#if 0 /// LLL temp
            pPH2->pfTx = CallTxVecTab;
            pPH2->pfRx = CallRxVecTab;
#endif
        }

        /* Init for Tone A detection */
        pPH2->pqCoef[0] = V34_qTONEAL;
        pPH2->pqCoef[1] = V34_qTONEA;
        pPH2->pqCoef[2] = V34_qTONEAR;

        DspIIR_Cas51_Init(&(pPH2->biq), (QWORD *)BIQ_COEF_CALL, pqDline, 3);
    }
    else if (pPH2->modem_mode == ANS_MODEM) /* only 2 modes in phase 2 */
    {
#if SUPPORT_V34FAX

        if (pPH2->V34fax_Enable == 1)
        {
            pPH2->pfTx = AnsTxVecTab_V34fax;
            pPH2->pfRx = AnsRxVecTab_V34fax;
        }
        else
#endif
        {
#if 0 /// LLL temp
            pPH2->pfTx = AnsTxVecTab;
            pPH2->pfRx = AnsRxVecTab;
#endif
        }

        /* Init for Tone B detection */
        pPH2->pqCoef[0] = V34_qTONEB1L;
        pPH2->pqCoef[1] = V34_qTONEB1;
        pPH2->pqCoef[2] = V34_qTONEB1R;

        DspIIR_Cas51_Init(&(pPH2->biq), (QWORD *)BIQ_COEF_ANS, pqDline, 3);
    }

    V34_DFT_Init(&pPH2->tonedet, pPH2->pqQ1, pPH2->pqQ2, pPH2->pqCoef, pPH2->pqMag_sqr, 128, 3);
}
