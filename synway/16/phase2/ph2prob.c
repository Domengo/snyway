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

/************************************************************************/
/*  Line Probing Tone generation routine                                */
/*                                                                      */
/*  The tone generation program is designed based on 9.6kHz             */
/*  sampling rate.                                                      */
/*                                                                      */
/************************************************************************/

#include "ph2ext.h"

void Ph2_Prob_Init(ProbStruc *pProb)
{
    pProb->L1_counter = 0;
    pProb->L2_counter = 0;

    pProb->ptone_counter = 0;
}

/* (Freq / 9600Hz * 65536) */
CONST UWORD FreqTable[21] =
{
    1024,    /* V34_150 HZ */
    2048,    /* V34_300 HZ */
    3072,    /* V34_450 HZ */
    4096,    /* V34_600 HZ */
    5120,    /* V34_750 HZ */
    7168,    /* V34_1050HZ */
    9216,    /* V34_1350HZ */
    10240,   /* V34_1500HZ */
    11264,   /* V34_1650HZ */
    13312,   /* V34_1950HZ */
    14336,   /* V34_2100HZ */
    15360,   /* V34_2250HZ */
    17408,   /* V34_2550HZ */
    18432,   /* V34_2700HZ */
    19456,   /* V34_2850HZ */
    20480,   /* V34_3000HZ */
    21504,   /* V34_3150HZ */
    22528,   /* V34_3300HZ */
    23552,   /* V34_3450HZ */
    24576,   /* V34_3600HZ */
    25600    /* V34_3750HZ */
};

void Ph2_Create_Probing(Ph2Struct *pPH2, QWORD qAmp, SWORD *repetition_cntr)
{
    ProbStruc *pProb = &(pPH2->Probing);
    UBYTE i, j;
    QDWORD qdTemp; /* cosine value for one specific frequency */
    QDWORD qX;

    if (*repetition_cntr <= 0)
    {
        for (i = 0; i < V34_PROBING_TONES; i++)
        {
            pProb->puCurrent_phase[i] = 0;
        }

        pProb->puCurrent_phase[1 ] = PHASE_180_DEGREE;
        pProb->puCurrent_phase[8 ] = PHASE_180_DEGREE;
        pProb->puCurrent_phase[11] = PHASE_180_DEGREE;
        pProb->puCurrent_phase[13] = PHASE_180_DEGREE;
        pProb->puCurrent_phase[15] = PHASE_180_DEGREE;
        pProb->puCurrent_phase[16] = PHASE_180_DEGREE;
        pProb->puCurrent_phase[17] = PHASE_180_DEGREE;
        pProb->puCurrent_phase[18] = PHASE_180_DEGREE;

        *repetition_cntr = V34_REPETITION_RATE_INDEX - PH2_SYM_SIZE;
    }
    else
    {
        *repetition_cntr -= PH2_SYM_SIZE;
    }

    for (j = 0; j < PH2_SYM_SIZE; j++)
    {
        qdTemp = 0;

        for (i = 0; i < V34_PROBING_TONES; i++)
        {
            qX = QQMULQD(qAmp, DSP_tCOS_TABLE[(pProb->puCurrent_phase[i] >> 8)]);

            pProb->puCurrent_phase[i] = (pProb->puCurrent_phase[i] + FreqTable[i]) & 0xFFFF;

            qdTemp += qX;
        }

        pPH2->PCMoutPtr[j] = QDR15Q(qdTemp);
    }
}

/*  Using 64pt FFT to caculate Power Spectrum, remove Hilbert Transform */
void Ph2_Estimate_Power_Spectrum(Ph2Struct *pPH2)
{
    ProbStruc *probing = &(pPH2->Probing);
    CQWORD    FFT_out[64];
    cCIRC_BUF *FFT_in;
    FFT_Data  *FFT_info;
    QWORD     *pqPower_Spectrum;
    UBYTE ubLogic;
    CQWORD cqIn;
    QDWORD qdTemp;
    QDWORD qdTemp1;
    QDWORD qdTemp2;
    QDWORD dtemp;
    UBYTE  i, j;

    pqPower_Spectrum = probing->pqPower_Spectrum;

    FFT_info = &(probing->FFT32_Info);

    FFT_in = &(probing->FFT_in);

    for (j = 0; j < PH2_SYM_SIZE; j++)
    {
        cqIn.r = pPH2->PCMinPtr[j] >> 2; /* Avoid overflow */
        cqIn.i = 0;

        FFT_in->pcBuf[ FFT_in->idx++] = cqIn;
        FFT_in->idx &= 63;

        /* 64 samples collected, do FFT */
        if (FFT_in->idx == 0)
        {
            Ph2_FFT(FFT_info, FFT_in->pcBuf, FFT_out);

            probing->power_counter++;

            if (probing->power_counter <= 0)
            {
                for (i = 0; i < 25; i++)
                {
                    pqPower_Spectrum[i] = 0;
                }
            }
            else
            {
                for (i = 0; i < 25; i++)
                {
                    qdTemp   = QQMULQD(pqPower_Spectrum[i], (probing->power_counter - 1));
                    qdTemp1  = QQMULQD(FFT_out[i+1].r, FFT_out[i+1].r);
                    qdTemp1 += QQMULQD(FFT_out[i+1].i, FFT_out[i+1].i);

                    ubLogic = Ph2_Is6_8_12_16((UBYTE)(i + 1));

                    if (ubLogic)
                    {
                        qdTemp1 <<= 11;
                    }

                    qdTemp2  = QDR15Q(qdTemp1);
                    qdTemp  += qdTemp2;

                    pqPower_Spectrum[i] = (QWORD)QDQDdivQD(qdTemp, probing->power_counter);

#if SUPPORT_V34FAX

                    if (pPH2->V34fax_Enable == 1)/* Need verify for V34 Modem */
                    {
                        pPH2->pPower_Spect[i] = pqPower_Spectrum[i];
                    }

#endif

#if 0 /* To dump line probing result */
                    DumpTone2_Idx = i;
                    DumpTone2[DumpTone2_Idx++] = pqPower_Spectrum[i];
#endif
                }

                if (pPH2->TimingDrift_Enable == 1)
                {
                    /* Collect phase info of 2250Hz tone for timing/frequency offset */
                    dtemp = DSPD_Atan2(FFT_out[15].r, FFT_out[15].i);

                    if (probing->power_counter != 1)
                    {
                        while ((dtemp - pPH2->prev_angle) > qD_270)/* if ((dtemp - pPH2->prev_angle) > qD_270) */
                        {
                            dtemp -= qD_360;
                        }

                        while ((dtemp - pPH2->prev_angle) < -qD_270)/* else if ((dtemp - pPH2->prev_angle) < -qD_270) */
                        {
                            dtemp += qD_360;
                        }
                    }

                    pPH2->all_angles[probing->power_counter-1] = dtemp;
                    pPH2->prev_angle = dtemp;

                    /* Collect phase info of 1050Hz tone for timing/frequency offset */
                    dtemp = DSPD_Atan2(FFT_out[7].r, FFT_out[7].i);

                    if (probing->power_counter != 1)
                    {
                        while ((dtemp - pPH2->prev_angle1) > qD_270)
                        {
                            dtemp -= qD_360;
                        }

                        while ((dtemp - pPH2->prev_angle1) < -qD_270)
                        {
                            dtemp += qD_360;
                        }
                    }

                    pPH2->all_angles1[probing->power_counter-1] = dtemp;

                    pPH2->prev_angle1 = dtemp;
                }
            }
        }
    }
}

void Ph2_RxProbing_Init(ProbStruc *pProb)
{
    UBYTE i;

    for (i = 0; i < 32; i++)
    {
        pProb->pqL1_Power_Spectrum[i] = 0;
        pProb->pqL2_Power_Spectrum[i] = 0;
    }

    pProb->error = 0;

    Ph2_FFT_Init(&(pProb->FFT32_Info), pProb->pcFFT_w_vec, pProb->pFFT_permute_idx, 64/* 32 */, 0);

    pProb->FFT_in.pcBuf = pProb->pcFFT_inbuf;

    for (i = 0; i < 64; i++)
    {
        pProb->FFT_in.pcBuf[i].r = 0;
        pProb->FFT_in.pcBuf[i].i = 0;
    }

    pProb->FFT_in.size = 64;
    pProb->FFT_in.idx = 0;

    pProb->power_counter = -4; /* consider 4 * 16 sample points delay */
    pProb->pqPower_Spectrum = pProb->pqL1_Power_Spectrum;
}

void Ph2_NLD_Analyze(Ph2Struct *pPH2)
{
    ProbStruc *probing = &(pPH2->Probing);
    QDWORD qdSumL1, qdSumNoiseL1, qdSumNoiseL2, qdTmp;
    QWORD qAveL1, qAveNoiseL1, qAveNoiseL2;
    UBYTE i, ubLogic, ubFlag;
    QWORD Spec_l, Spec_m, Spec_h;
    UBYTE SNR;

    ubFlag = 0;
    qdSumL1 = 0;
    qdSumNoiseL1 = 0;
    qdSumNoiseL2 = 0;

    for (i = 1; i < 21; i++)
    {
#if 0 /* To dump line probing result */
        DumpTone3[DumpTone3_Idx++] = probing->pqL1_Power_Spectrum[i];
        DumpTone4[DumpTone4_Idx++] = probing->pqPower_Spectrum[i];
#endif
        ubLogic = Ph2_Is6_8_12_16((UBYTE)(i + 1));

        if (ubLogic)
        {
            if ((probing->pqL1_Power_Spectrum[i] < 0) || (probing->pqPower_Spectrum[i] < 0))
            {
                ubFlag = 1;
                TRACE0("Too much white noise for NLD analysis");

                break;
            }

            qdSumNoiseL1 += probing->pqL1_Power_Spectrum[i];
            qdSumNoiseL2 += probing->pqPower_Spectrum[i];
        }
        else
        {
            qdSumL1 += probing->pqL1_Power_Spectrum[i];
        }
    }

    pPH2->qNLDFactor = 0;

#if SUPPORT_V34FAX

    if (pPH2->V34fax_Enable == 1)/* Need verify for V34 Modem */
    {
        SNR = Ph2_SNR_calcu(pPH2);

        if (SNR <= 12)
        {
            pPH2->max_symbol_rate = V34_SYM_2400;
        }

        qdTmp = (QDWORD)pPH2->pPower_Spect[8]  + pPH2->pPower_Spect[9]  + pPH2->pPower_Spect[10] +
                pPH2->pPower_Spect[12] + pPH2->pPower_Spect[13] + pPH2->pPower_Spect[14];

        Spec_m = QDQMULQDR(qdTmp, 5120);

        Spec_l = pPH2->pPower_Spect[0];

        Spec_h = (pPH2->pPower_Spect[20] + pPH2->pPower_Spect[19]) >> 1;

        TRACE3("pPower9  =%d, pPower10=%d, pPower8 =%d", pPH2->pPower_Spect[9],  pPH2->pPower_Spect[10], pPH2->pPower_Spect[8]);
        TRACE3("pPower13 =%d, pPower14=%d, pPower12=%d", pPH2->pPower_Spect[13], pPH2->pPower_Spect[14], pPH2->pPower_Spect[12]);
        TRACE3("pPower1  =%d, pPower20=%d, pPower19=%d", pPH2->pPower_Spect[1],  pPH2->pPower_Spect[20], pPH2->pPower_Spect[19]);
        TRACE2("pPower/16=%d, pPower/8=%d", (Spec_m >> 4), (Spec_m >> 3));

        if ((Spec_l < Spec_m) || (Spec_h < Spec_m))
        {
            pPH2->V34fax_EQ_Mode = 1;

            if ((pPH2->AGC_gain > 2000) /* around level -30 to -35 */
                || (SNR <= 20))
            {
                pPH2->max_symbol_rate = V34_SYM_2400;
            }
        }
        else
        {
            pPH2->V34fax_EQ_Mode = 0;
        }

        pPH2->V34fax_EQ_Mode = 1;
        TRACE1("pPH2->V34fax_EQ_Mode=%x", pPH2->V34fax_EQ_Mode);
    }

#endif

    if (ubFlag == 0)
    {
        qAveNoiseL1 = (QWORD)(qdSumNoiseL1 >> 2);
        qAveNoiseL2 = (QWORD)(qdSumNoiseL2 >> 2);

        qAveL1 = (QWORD)(qdSumL1 >> 4);

        TRACE3("AveNoiseL1=%d, AveNoiseL2=%d, AveL1=%d", qAveNoiseL1, qAveNoiseL2, qAveL1);

        qAveL1 = QQMULQ15(qAveL1, q06);

        if (qAveNoiseL1 > qAveL1)
        {
            if (qAveNoiseL2 == 0)
            {
                qAveNoiseL2 = 1;
            }

            qdSumL1 = QDQDdivQD(((QDWORD)qAveNoiseL1 << 8), qAveNoiseL2); /* Q7.8 */

            if (qdSumL1 > 32767)
            {
                qdSumL1 = 32767;
            }

            pPH2->qNLDFactor = (QWORD)qdSumL1; /* Q7.8 */
        }
    }
}

#if SUPPORT_V34FAX

UBYTE Ph2_SNR_calcu(Ph2Struct *pPH2)
{
    QDWORD qdTemp1, qdTemp2;
    UBYTE  SNR, SignalLevel;

    if (pPH2->qdNoise_Egy > 65536)
    {
        qdTemp1 = DSPD_Log10(pPH2->qdNoise_Egy) + 147962;
    }
    else
    {
        qdTemp1 = DSPD_Log10(pPH2->qdNoise_Egy << 15);
    }

    qdTemp2 = DSPD_Log10(((QDWORD)pPH2->AGC_gain) << 15);

    SignalLevel = (UBYTE)((20 * DSPD_Log10((((QDWORD)pPH2->AGC_gain) << 15) / 40)) >> 15);

    SNR = (UBYTE)(135 - ((10 * ((qdTemp2 << 1) + qdTemp1)) >> 15));

    TRACE1("pPH2->qdNoise_Egy = %" FORMAT_DWORD_PREFIX "d", pPH2->qdNoise_Egy);
    TRACE1("SNR = %d", SNR);
    TRACE1("SignalLevel = %d", SignalLevel);

    return (SNR);
}

#endif
