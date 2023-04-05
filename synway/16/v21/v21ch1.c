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

#include "ptable.h"
#include "v21ext.h"

#if SUPPORT_V21CH1

void V21_ModemCh1(V21Struct *pV21)
{
    if (pV21->Detect_tone == 1)
    {
        pV21->pfTxVec  = V21_Ch1MarkTone;
        pV21->Count_Tx = 50;
    }
}

void V21_Ch1MarkTone(V21Struct *pV21)
{
    pV21->TxBits = 0x01;

    V21_Ch1Tx(pV21);

    pV21->Count_Tx --;

    if (pV21->Count_Tx == 0)
    {
        pV21->TxMode  = 1; /* Data mode */

        pV21->pfTxVec = V21_Ch1Tx;
    }
}


void V21_Ch1Tx(V21Struct *pV21)
{
    SWORD qDeltaphase;

    if (pV21->TxBits)
    {
        qDeltaphase = pV21->qCh1_Mark_Delta_Phase;
    }
    else
    {
        qDeltaphase = pV21->qCh1_Space_Delta_Phase;
    }

    V21_Tx(pV21, qDeltaphase);
}

void V21_Ch1_Detect_tone(V21Struct *pV21)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV21->pTable[DPCS_STRUC_IDX]);

    V21_Ch1Tone_Detect(pV21);

    if (pV21->Detect_tone == 1)
    {
        pV21->pfRxVec = V21_Ch1Rx;

        pV21->RxMode = 1; /* data mode */

        PutReg(pDpcs->MSR0, DATA);
        PutReg(pDpcs->MBSC,   DATA_RATE_300);
        PutReg(pDpcs->MBSCTX, DATA_RATE_300);
    }
}


void V21_Ch1Rx(V21Struct *pV21)
{
    V21_Rx(pV21, V21_CH1);
}


void V21_Ch1Tone_Detect(V21Struct *pV21)
{
    QDWORD qdAvg_Energy0 = 0, qdAvg_Energy21 = 0;
    QWORD  qFsample = 0, qTemp;
    UBYTE  i;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        /* 1650Hz(for V21) or 2100(for V8 ans) Bandstop filtering */
        /* for filting echo to avoid echo  to effect the absolute power qdAvg_Energy0 */
        qTemp = DspFir(&(pV21->BpfLow), pV21->PCMinPtr[i]);

        /* Look for absolute power */
        qdAvg_Energy0 += QQMULQD(qTemp, qTemp) >> 8;

#if (V21_7200 && V21_9600)

        if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
        {
            qFsample = DspIIR_Cas51(&(pV21->BpfToneIIR_72), qTemp);
        }
        else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
        {
            qFsample = DspFir(&(pV21->BpfToneIIR_96), qTemp);
        }

#elif V21_7200

        qFsample = DspIIR_Cas51(&(pV21->BpfToneIIR_72), qTemp);

#elif V21_9600

        qFsample = DspFir(&(pV21->BpfToneIIR_96), qTemp);

#endif

        /* Look for absolute power */
        qdAvg_Energy21 += QQMULQD(qFsample, qFsample) >> 8;
    }

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = qdAvg_Energy0 >> 1; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = qdAvg_Energy21; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV21->qEnergy_Ref; }

#endif

    if ((qdAvg_Energy21 >= (qdAvg_Energy0 >> 1)) && (qdAvg_Energy21 > pV21->qEnergy_Ref))
    {
        pV21->DetCounter++;

        if (pV21->DetCounter > 6)
        {
            pV21->Detect_tone = 1;
            pV21->DetCounter  = 0;
            pV21->qdCarrierThreshold = 0;
            pV21->Carrier_Flag       = 0;

            /* Keep the delay line */
#if (V21_7200 && V21_9600)

            if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
            {
                pV21->BpfLow.pCoef = (QWORD *)V21_tLFilter_Coef_72;
            }
            else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
            {
                pV21->BpfLow.pCoef = (QWORD *)V21_tLFilter_Coef_96;
            }

#elif V21_7200
            pV21->BpfLow.pCoef = (QWORD *)V21_tLFilter_Coef_72;
#elif V21_9600
            pV21->BpfLow.pCoef = (QWORD *)V21_tLFilter_Coef_96;
#endif

            TRACE0("V21: Ch1 Tone Detected");

            qTemp = QDsqrt(qdAvg_Energy21);
            pV21->qAgc_gain = (QWORD)QDQDdivQD(pV21->qdModem_AGC_Ref, qTemp);
            TRACE2("V21: (1)qEgy_sqrt = %d, pV21->qAgc_gain = %d", qTemp, pV21->qAgc_gain);
        }
    }
    else
    {
        pV21->DetCounter = 0;
    }
}

#endif

void V21_Tx(V21Struct *pV21, SWORD qDeltaphase)
{
    UBYTE  i;
    UWORD  Phase;
    QWORD  qSinOut;

    Phase = pV21->Phase;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        Sin_Lookup_Fine(&Phase, qDeltaphase, &qSinOut);
        qSinOut = QQMULQ15(qSinOut, qSINGLE_TONE_12DBM0);/* -12dBm0 */

#if SUPPORT_V54

        if (pV21->modem_mode != DIGITAL_LOOP_BACK)
#endif
            qSinOut = DspFir(pV21->BpfTxPtr, qSinOut);

        *pV21->PCMoutPtr++ = qSinOut;
    }

    pV21->Phase = Phase;
}


void V21_Rx(V21Struct *pV21, UBYTE channel_mode)
{
    QFIRStruct *Bpf;
    UBYTE  i, ubSmpCnt;
    QWORD  qFsample, qInput, qTemp, qTemp1;
    QDWORD qMAC;
    QWORD  qProd, pqProd[2];
    QDWORD qdSlevel = 0;
    QWORD  qAgc_gain;
    UBYTE  pdel_delay_length;
    QWORD  modem_beta;
    UBYTE  RxNumBits = 0;

    if (channel_mode == V21_CH1)
    {
        pdel_delay_length = pV21->ubCh1_PDEL_Delay_Len;
        modem_beta        = pV21->qCh1_Modem_Beta;

        Bpf = &pV21->BpfLow;
    }
    else
    {
        pdel_delay_length = pV21->ubCh2_PDEL_Delay_Len;
        modem_beta        = pV21->qCh2_Modem_Beta;

        Bpf = &pV21->BpfHigh;
    }

    qAgc_gain = pV21->qAgc_gain;

    ubSmpCnt = 0;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
#if 0

        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV21->PCMinPtr[i]; }

#endif

        /* FSK Channel separation filter */
        qFsample = DspFir(Bpf, pV21->PCMinPtr[i]);

        /* Calculate Signal energy, for Carrier lost detection */
        if (qFsample < 0)
        {
            qInput = -qFsample;
        }
        else
        {
            qInput =  qFsample;
        }

        qdSlevel += qInput;

        /* Apply AGC gain */
        qFsample = QQMULQR8(qAgc_gain, qFsample);

#if 0

        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = qFsample; }

#endif

        /* V21 CH1 or CH2 Demodulation */
        if (pV21->sbPdelIdx == (pdel_delay_length - 1))
        {
            qTemp1 = pV21->pqPdel[0];
        }
        else
        {
            qTemp1 = pV21->pqPdel[pV21->sbPdelIdx + 1];
        }

        qTemp = QQMULQR15(pV21->pqPdel[pV21->sbPdelIdx], modem_beta);
        qMAC  = QQMULQD(qFsample, qTemp1);
        qMAC += QQMULQD(qFsample, qTemp);

        pqProd[ubSmpCnt++] = QDR15Q(qMAC);

        pV21->pqPdel[pV21->sbPdelIdx++] = qFsample;

        if (pV21->sbPdelIdx >= pdel_delay_length)
        {
            pV21->sbPdelIdx = 0;
        }

        if (ubSmpCnt >= 2)
        {
            ubSmpCnt = 0;

            /* ------ Remove high freq. components from FSK Demodulator output ---- */
            qProd = DspDeciFir(&(pV21->LpFilter), &pqProd[0]);

            if (channel_mode == V21_CH1)
            {
                qProd = - qProd;
            }

#if 0 /* V21 demodulate out */

            if (DumpTone2_Idx < 10000000) { DumpTone2[DumpTone2_Idx++] = qProd; }

#endif

            /* -------------------------- Slicer ---------------------------------- */
            if (pV21->State == 1)
            {
                if (qProd > 0)
                {
                    pV21->ubCounter++;
                }
                else                 /* Transition from +ve to -ve */
                {
                    if (pV21->ubCounter > (pV21->ubBufferSize >> 2))
                    {
                        pV21->pOutBits[RxNumBits ++] = 1;
                    }

                    pV21->ubCounter = 1;
                    pV21->State = 0;
                }
            }
            else /* ======> that is pV21->State ==0   <============= */
            {
                if (qProd < 0)
                {
                    pV21->ubCounter++;
                }
                else                 /* Transition from +ve to -ve */
                {
                    if (pV21->ubCounter > (pV21->ubBufferSize >> 2))
                    {
                        pV21->pOutBits[RxNumBits ++] = 0;
                    }

                    pV21->ubCounter = 1;
                    pV21->State = 1;
                }
            }

            if (pV21->ubCounter >= (pV21->ubBufferSize >> 1))
            {
                pV21->pOutBits[RxNumBits ++] = pV21->State;

                pV21->ubCounter = 0;
            }
        }
    }

    /* ===========>  gain/energy/loss calculation    <===================== */
    if (pV21->Carrier_Flag == 0)
    {
        if (qdSlevel < pV21->qCarrierLoss_Thres)
        {
            pV21->Disconnect_flag = 1;

            TRACE0("V21: Loss of Carrier");
        }
        else
        {
            pV21->qdCarrierThreshold = qdSlevel >> 3;
            pV21->Carrier_Flag = 1;
        }
    }

    if (qdSlevel < pV21->qdCarrierThreshold)
    {
        pV21->Silence_samples++;

        if (pV21->RxMode)/* for V21 modem or fax */
        {
            if (pV21->Silence_samples > pV21->ubMaxSilenceRef)
            {
                pV21->Detect_tone = 0;
                pV21->Disconnect_flag = 1;

                TRACE0("V21: Loss of Carrier");
            }
        }
        else if (pV21->Silence_samples > 1000 && pV21->Disconnect_flag == 0)/* for V8. 3.333s carrier loss */
        {
            pV21->Disconnect_flag = 1;
            TRACE0("V21: Long Silence");
        }
    }
    else /* ====> Get signal again <===== */
    {
        pV21->Silence_samples = 0;
    }

    pV21->RxNumBits = RxNumBits;
}
