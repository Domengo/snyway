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

/******************************************************************/
/*       Description:    DPSK demodulation & detection            */
/*       input:   sample[] -- sample by 16,                       */
/*                CM = 1  call modem, otherwise answer modem;     */
/*       output:  1 -- info bit 1                                 */
/*                0 -- info bit 0                                 */
/*               -1 -- no bit is detected                         */
/******************************************************************/
SBYTE Ph2_DPSK_Demod(Ph2Struct *pPH2)
{
    DPSKInfo    *pDpsk = &(pPH2->Dpsk);
    QFIRStruct  *pfir  = &(pDpsk->LPfir);
    Info_0Struc *pInfo;
    QWORD  Diff; /* add for ntt6,7 DPSK symc bit detection */
    QWORD  qDp;
    UBYTE  i;
    SWORD  Threshold;

    Threshold = PH2_THRESHOLD2;

    if (pPH2->modem_mode == CALL_MODEM)
    {
        pInfo = &(pPH2->I0a);

#if SUPPORT_V34FAX

        if (pPH2->V34fax_Enable == 1)
        {
            Threshold = PH2_THRESHOLD3;
        }

#endif
    }
    else
    {
        pInfo = &(pPH2->I0c);
    }

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        qDp = QQMULQ15(pPH2->PCMinPtr[i], pDpsk->pqSdelay[i]);

        pDpsk->pqSdelay[i] = pPH2->PCMinPtr[i];

        pDpsk->pqSum_buf[i] = DspFir(pfir, qDp);

        Diff = pDpsk->pqSum_buf[i] - pPH2->PrevOut;

        pPH2->PrevOut = pDpsk->pqSum_buf[i];

#if 0

        if (pPH2->Retrain_V90 == 2)
        {
            if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pPH2->PCMinPtr[i]; }
        }

#endif

        if ((pDpsk->DPSK_Flag == 0) && (pDpsk->ubFrameSyncCount == 0))
        {
            if (pDpsk->DPSK_begin_flag == 0)
            {
                if (pDpsk->DPSK_bit_cnt < ONE_NUM)
                {
                    if (pDpsk->pqSum_buf[i] < -PH2_THRESHOLD1)
                    {
                        pDpsk->DPSK_bit_cnt++;
                        pDpsk->DPSK_bit_cnt &= 0xFF;
                    }
                    else
                    {
                        pDpsk->DPSK_bit_cnt = 0;
                    }
                }
                else
                {
                    if (pDpsk->pqSum_buf[i] > PH2_THRESHOLD4)
                    {
                        pDpsk->DPSK_bit_cnt = 1;
                        pDpsk->DPSK_begin_flag = 1;
                    }
                }
            }
            else
            {
#if 0

                if (pDpsk->pqSum_buf[i] > PH2_THRESHOLD2)
                {
                    pDpsk->DPSK_bit_cnt++;

                    if (pDpsk->DPSK_bit_cnt == 5)//7)
                    {
                        pDpsk->ubFrameSyncCount = 1;
                        pDpsk->out_offset = i;
                    }
                }

#else

                if (pDpsk->pqSum_buf[i] > PH2_THRESHOLD4)
                {
                    pDpsk->DPSK_bit_cnt++;
                    pDpsk->DPSK_bit_cnt &= 0xFF;

                    if ((Diff <= 0) && (pDpsk->DPSK_bit_cnt < 12))
                    {
                        pDpsk->ubFrameSyncCount = 1;
                        pDpsk->out_offset = i;

                        if (pDpsk->pqSum_buf[i] <= Threshold)
                        {
                            pDpsk->pqSum_buf[i] = Threshold + 1;
                        }
                    }
                }

#endif
                else
                {
                    pDpsk->DPSK_begin_flag = 0;
                    pDpsk->DPSK_bit_cnt = 0;
                }
            }
        }
    }

    if (pDpsk->DPSK_Flag == 1)
    {
#if 0

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pDpsk->out_offset; }

        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pDpsk->pqSum_buf[pDpsk->out_offset]; }

#endif

        if (pDpsk->pqSum_buf[pDpsk->out_offset] < -Threshold)
        {
            return (1);
        }
        else if (pDpsk->pqSum_buf[pDpsk->out_offset] >  Threshold)
        {
            return (0);
        }
        else
        {
            Ph2_DPSK_Detect_Init(pDpsk);

            return (-20);
        }
    }
    else
    {
        if (pDpsk->ubFrameSyncCount > 0)
        {
            if (pDpsk->pqSum_buf[pDpsk->out_offset] < -Threshold)
            {
                pDpsk->ubFrameSync = (pDpsk->ubFrameSync << 1) + 1;
            }
            else if (pDpsk->pqSum_buf[pDpsk->out_offset] >  Threshold)
            {
                pDpsk->ubFrameSync <<= 1;
            }
            else
            {
                Ph2_DPSK_Detect_Init(pDpsk);
                return (-20);
            }

            if (pDpsk->ubFrameSyncCount++ == 8)
            {
                if (pDpsk->ubFrameSync == 0x72)
                {
                    pDpsk->ubFrameSyncCount = 0;
                    pDpsk->ubFrameSync = 0;

                    Ph2_Detect_Info_0_Init(pInfo);

                    pDpsk->DPSK_Flag = 1;
                }
                else
                {
                    Ph2_DPSK_Detect_Init(pDpsk);
                }
            }
        }

        if (pDpsk->pqSum_buf[pDpsk->out_offset] < -PH2_THRESHOLD2)
        {
            return (-11);    /*For 1*/
        }
        else if (pDpsk->pqSum_buf[pDpsk->out_offset] > PH2_THRESHOLD2)
        {
            return (-10);    /*For 0*/
        }
        else
        {
            return (-20);
        }
    }
}

UBYTE Ph2_DPSK_PhaseReversal_Detected(Ph2Struct *pPH2)
{
    DPSKInfo *pDpsk = &(pPH2->Dpsk);
    UBYTE i;
    UBYTE uReverseCount = 0;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        if (pDpsk->pqSum_buf[i] < -PH2_THRESHOLD2)
        {
            ++uReverseCount;
        }
    }

    if (uReverseCount > 6)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

/***************************************************************/
/*     Description:    module for DPSK modulation;               */
/*     Input:    bit  -- bit;                                    */
/*             CM = 1  call modem, otherwise answer modem;     */
/*     Output:   sample[] -- sample by 16.                       */
/***************************************************************/
void Ph2_DPSK_Modfilter(Ph2Struct *pPH2, UBYTE bit, SWORD qModemCarrier)
{
    DPSKInfo *pDpsk = &(pPH2->Dpsk);
    QWORD  pqXs[PH2_SYM_SIZE];
    SWORD  qWc;
    UBYTE  i;

    if (bit == 1)
    {
        pDpsk->qEncode_bit = -pDpsk->qEncode_bit;
    }

#if 0

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pDpsk->qEncode_bit; }
    }

#endif

    DspiFir(&(pDpsk->dpskfir), pDpsk->qEncode_bit, pqXs);

#if 0

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pqXs[i]; }
    }

#endif

    qWc = pDpsk->qWc;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        /* Cosine Carrier */
        pPH2->PCMoutPtr[i] = QQMULQ15(pqXs[i], DSP_tCOS_TABLE[qWc]);/* modulate by cos(wc*n) */

#if 0

        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pPH2->PCMoutPtr[i]; }

#endif

        qWc = (qWc + qModemCarrier) & COS_SAMPLE_NUM;
    }

    pDpsk->qWc = qWc;
}

/* DPSK A/B Tone generator without shaping filter */
void Ph2_DPSK_Mod(Ph2Struct *pPH2, UBYTE bit, SWORD qModemCarrier)
{
    DPSKInfo *pDpsk = &(pPH2->Dpsk);
    QWORD  qWc;
    UBYTE  i;

    qWc = pDpsk->qWc;

    /* 180 degree Phase change if input bit is 1 */
    if (bit == 1)
    {
        qWc = (qWc + 128) & COS_SAMPLE_NUM;
    }

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        pPH2->PCMoutPtr[i] = QQMULQ15(DSP_tCOS_TABLE[qWc], qSINGLE_TONE_12DBM0);/* -12dBm0 */

        qWc = (qWc + qModemCarrier) & COS_SAMPLE_NUM;
    }

    pDpsk->qWc = qWc;
}


/***********************************/
/*  Output PCM buffer, 16 samples  */
/***********************************/
void Ph2_DPSK_Silence(Ph2Struct *pPH2)
{
    UBYTE i;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        pPH2->PCMoutPtr[i] = 0;
    }
}

void  Ph2_DPSK_Detect_Init(DPSKInfo *pDpsk)
{
    pDpsk->DPSK_Flag        = 0;
    pDpsk->DPSK_begin_flag  = 0;
    pDpsk->DPSK_bit_cnt     = 0;

    pDpsk->ubFrameSync      = 0;
    pDpsk->ubFrameSyncCount = 0;
}

void  Ph2_DPSK_Init(DPSKInfo *pDpsk, UBYTE modem_mode)
{
    QWORD *DPSKfir_Coef;
    QWORD *BPfir_Coef;
    SWORD BPfir_Len;
    UBYTE i;

    Ph2_DPSK_Detect_Init(pDpsk);

    pDpsk->out_offset = 0;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        pDpsk->pqSum_buf[i] = 0;
        pDpsk->pqSdelay[i]  = 0;
    }

    pDpsk->qWc = 0;/*64:init phase converse*/

    pDpsk->qEncode_bit = V34_qDPSK_MAG;

    /* Init for Demodulator */
    DspFirInit(&(pDpsk->LPfir), (QWORD *)V34_tDPSK_LP, pDpsk->pqFilter_delay_4, 48);

    if (modem_mode == CALL_MODEM)
    {
        DPSKfir_Coef = (QWORD *)V34_tDPSK_HM_Coef_Call;

        BPfir_Coef = (QWORD *)V34_tDPSK_CMCOEF;

        BPfir_Len = V34_DPSK_CALL_LEN;
    }
    else
    {
        DPSKfir_Coef = (QWORD *)V34_tDPSK_HM_Coef_Ans;

        BPfir_Coef = (QWORD *)V34_tDPSK_ANCOEF;

        BPfir_Len = V34_DPSK_ANS_LEN;
    }

    /* Init for Modulator */
    DspiFirInit(&(pDpsk->dpskfir),        /* QIFIRStruct */
                DPSKfir_Coef,            /* Filter Coef */
                pDpsk->pqFilter_delay_1, /* Delayline   */
                V34_DELAYLINE_LENGTH,    /* Delayline Length */
                16);                     /* Block size  */

    DspFirInit(&(pDpsk->BPfir), BPfir_Coef, pDpsk->pqFilter_delay_3, BPfir_Len);
    /* share the same delay line */
}
