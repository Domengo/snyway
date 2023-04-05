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

CONST SWORD BAND_WIDTH[6] =
{
    1216, 1390, 1418, 1520, 1621, 1737
};

CONST SWORD POWER_INDEX[22] =
{
    256,  322,  406,  511,  643,  810, 1019, 1283, 1615, 2033, 2560,
    3223, 4057, 5108, 6430, 8095, 10192, 12830, 16153, 20335, 25600, 32228
};

void ClrSymRateInfo(SymRate_Info *Sinfo)
{
    Sinfo->preemphasis_idx   = 0;
    Sinfo->carrier_freq_idx  = 0;
    Sinfo->project_data_rate = 0;
}

void ChooseCarrier(SymRate_Info *Sinfo, ProbStruc *probing, UBYTE lo, UBYTE hi, UBYTE car_host)
{
    UBYTE car;

    if (lo == 0 && hi == 0)
    {
        ClrSymRateInfo(Sinfo);
    }
    else
    {
        if (lo == 1 && hi == 0)
        {
            car = 0;
        }
        else if (lo == 0 && hi == 1)
        {
            car = 1;
        }
        else if (lo == 1 && hi == 1)
        {
            car = 2;
        }
        else
        {
            car = 3;    /* exception */
        }

        if (car != car_host)
        {
            if (car == 2)
            {
                car = car_host;
            }
            else if (car_host != 2)
            {
                car = 3;
            }
        }

        Ph2_Probing_Result(Sinfo, probing, car);
    }
}

void Ph2_Create_SymRate_Table(Ph2Struct *pPH2, Info_0Struc *pI0tx, Info_0Struc *pI0rx, UBYTE SymRate)
{
    SymRate_Info *Sinfo;
    UBYTE ubMaxRate, Choose_Carrier_Host;

    /* Added 10/28/97 to check if 1664 point constellation is supported by */
    /*  both host and remote modem  ******************************/
    ubMaxRate = V34_tMAX_DRATE[pI0rx->V34bis][SymRate];

    pPH2->pSymRate_info[SymRate].symbol_rate_idx   = SymRate;
    pPH2->pSymRate_info[SymRate].max_data_rate     = ubMaxRate;
    pPH2->pSymRate_info[SymRate].project_data_rate = ubMaxRate;

#if 0

    /* Added to allow high/low freq to be disabled by host */
    if ((pI0tx->S3200_low_carrier) && (!pI0tx->S3200_high_carrier))
    {
        Choose_Carrier_Host = 0;
    }
    else if ((!pI0tx->S3200_low_carrier) && (pI0tx->S3200_high_carrier))
    {
        Choose_Carrier_Host = 1;
    }
    else if ((pI0tx->S3200_low_carrier) && (pI0tx->S3200_high_carrier))
    {
        Choose_Carrier_Host = 2;
    }
    else
    {
        Choose_Carrier_Host = 3;    /* 4 */
    }

    /********************************************/
#else

    /* Prefer low carrier for all symbol rate lower than 3200Hz */
    if (SymRate < V34_SYM_3200)
    {
        Choose_Carrier_Host = 0;
    }
    else
    {
        Choose_Carrier_Host = 2;
    }

#endif

    Sinfo = &(pPH2->pSymRate_info[SymRate]);

    if (SymRate == V34_SYM_2400)
    {
        Ph2_Probing_Result(Sinfo, &(pPH2->Probing), Choose_Carrier_Host);
    }
    else if (SymRate == V34_SYM_2743)
    {
        if (pI0rx->S2743 == 1 && pI0tx->S2743 == 1)
        {
            Ph2_Probing_Result(Sinfo, &(pPH2->Probing), Choose_Carrier_Host);
        }
        else
        {
            ClrSymRateInfo(Sinfo);
        }
    }
    else if (SymRate == V34_SYM_2800)
    {
        if (pI0rx->S2800 == 1 && pI0tx->S2800 == 1)
        {
            Ph2_Probing_Result(Sinfo, &(pPH2->Probing), Choose_Carrier_Host);
        }
        else
        {
            ClrSymRateInfo(Sinfo);
        }
    }
    else if (SymRate == V34_SYM_3000)
    {
        ChooseCarrier(Sinfo, &(pPH2->Probing), pI0rx->S3000_low_carrier, pI0rx->S3000_high_carrier, Choose_Carrier_Host);
    }
    else if (SymRate == V34_SYM_3200)
    {
        ChooseCarrier(Sinfo, &(pPH2->Probing), pI0rx->S3200_low_carrier, pI0rx->S3200_high_carrier, Choose_Carrier_Host);
    }
    else if (SymRate == V34_SYM_3429)
    {
        if (pI0rx->S3429_enable == 1 && pI0tx->S3429 == 1)
        {
            Ph2_Probing_Result(Sinfo, &(pPH2->Probing), Choose_Carrier_Host);
        }
        else
        {
            Sinfo->project_data_rate = 0;
            Sinfo->preemphasis_idx   = 0;
            Sinfo->carrier_freq_idx  = 1;    /* always single carrier frequency */
        }
    }
}

/**********************************************************/
/* Note:                                                  */
/* 1) Choose_Carrier can take one of the following values */
/*       0---require to probe the low carrier case;       */
/*       1---require to probe the high carrier case;      */
/*       2---require to probe both the carrier cases      */
/**********************************************************/
void Ph2_Probing_Result(SymRate_Info *Sinfo, ProbStruc *probing, UBYTE Choose_Carrier)
{
    UBYTE  sym_rate_idx;
    QDWORD qGain_lo;
    QDWORD qGain_hi;
    QWORD  pqPower_lo[25];
    QWORD  pqPower_hi[25];
    QDWORD qItemp;
    UBYTE  edges_perf;
    UBYTE  i;
    UWORD  j;
    QWORD  *pqL1_Power_Spectrum = &(probing->pqL1_Power_Spectrum[0]);
    QWORD  *pqPower_Spectrum    = &(probing->pqPower_Spectrum[0]);

    sym_rate_idx = Sinfo->symbol_rate_idx;

    Ph2_Preemp_Idx(sym_rate_idx, probing);

    /* adjust the probing tone pqPower according to the estimated preemphasis index */
    j = 0;

    for (i = 0; i < 25; i++)
    {
        j += PH2_CONST1;

        qItemp  = QQMULQD(V34_tONE_OVER_SYM[sym_rate_idx],         j);
        qItemp += QQMULQ15(V34_tONE_OVER_SYM_OFFSET[sym_rate_idx], j);

        qGain_lo = Ph2_Pre_Filter_Gain(qItemp, probing->lo_preemp, 0);
        qGain_hi = Ph2_Pre_Filter_Gain(qItemp, probing->hi_preemp, 0);

        if (*pqPower_Spectrum == 0)
        {
            /* Use L1 Probing to determine tone magnitude at this frequency*/
            /* L1 is 6dB higher =4times higher than L2 so shift right by two*/
            pqPower_lo[i] = (QWORD)((QDQMULQD(qGain_lo, *pqL1_Power_Spectrum)) >> 2);
            pqPower_hi[i] = (QWORD)((QDQMULQD(qGain_hi, *pqL1_Power_Spectrum)) >> 2);
        }
        else
        {
            pqPower_lo[i] = (QWORD)QDQMULQDR(qGain_lo, *pqPower_Spectrum);
            pqPower_hi[i] = (QWORD)QDQMULQDR(qGain_hi, *pqPower_Spectrum);
        }

        pqL1_Power_Spectrum++;
        pqPower_Spectrum++;
    }

    edges_perf = Ph2_Check_Edges(sym_rate_idx, pqPower_lo, pqPower_hi, probing);

    i = edges_perf & 0x3;

    /* if the low band-edge is better than the high one */
    /* if (low_edges_perf==1 && high_edges_perf == 0) */
    switch (i)
    {
        case 0:   /* bothe edges are bad */
            ClrSymRateInfo(Sinfo);   /* neither of the two carrier is ok*/
            break;
        case 1:   /* high edge is better */
            Sinfo->carrier_freq_idx = 1;
            Sinfo->preemphasis_idx  = probing->hi_preemp;

            if (Choose_Carrier == 0) /* Host Forcing */
            {
                Sinfo->carrier_freq_idx = 0;
                Sinfo->preemphasis_idx  = probing->lo_preemp;
            }

            break;
        case 2:   /* low edge is better */
            Sinfo->carrier_freq_idx = 0;
            Sinfo->preemphasis_idx  = probing->lo_preemp;

            if (Choose_Carrier == 1) /* Host Forcing */
            {
                Sinfo->carrier_freq_idx = 1;
                Sinfo->preemphasis_idx  = probing->hi_preemp;
            }

            break;
        case 3:   /* both edges are good */

            if (Choose_Carrier == 0)
            {
                Sinfo->carrier_freq_idx = 0;
                Sinfo->preemphasis_idx  = probing->lo_preemp;
            }
            else if (Choose_Carrier == 1)
            {
                Sinfo->carrier_freq_idx = 1;
                Sinfo->preemphasis_idx  = probing->hi_preemp;
            }
            else if (Choose_Carrier == 2)
                /* If the two carriers are both supported */
            {
                if ((edges_perf & 0x4) > 0)
                {
                    Sinfo->carrier_freq_idx = 1;
                    Sinfo->preemphasis_idx  = probing->hi_preemp;
                }
                else
                {
                    Sinfo->carrier_freq_idx = 0;
                    Sinfo->preemphasis_idx  = probing->lo_preemp;
                }
            }
            else
            {
                Sinfo->carrier_freq_idx  = 0;
                Sinfo->project_data_rate = 0;
            }

            break;
    }
}

#if SUPPORT_V90D
CONST UBYTE SrateIdxTab[7][4] =
{
    {3, 19,  4, 20},  /* S2400  */
    {2, 20,  3, 21},  /* S2743  */
    {2, 21,  3, 22},  /* S2800  */
    {2, 22,  3, 23},  /* S3000  */
    {1, 22,  2, 23},  /* S3200  */
    {1, 24,  1, 24},  /* S3429  */
    {1, 24,  1, 24}  /* S8000  */
};

#else /* Only look at up to 3KHz */ /* 50%_PREEMPHASIS */

CONST UBYTE SrateIdxTab[7][4] =
{
    {3, 19,  4, 20},  /* S2400  */
    {2, 20,  3, 20},  /* S2743  */
    {2, 20,  3, 20},  /* S2800  */
    {2, 20,  3, 20},  /* S3000  */
    {1, 20,  2, 20},  /* S3200  */
    {1, 20,  1, 20},  /* S3429  */
    {1, 24,  1, 24}  /* S8000  */
};
#endif

UBYTE Ph2_Is6_8_12_16(UBYTE i)
{
    return (i == 6 || i == 8 || i == 12 || i == 16);
}

void Ph2_Calc_Pre_Filter_Gain(QDWORD pqdGain[6][11][25], UBYTE sym_rate_idx)
{
    CONST  UBYTE  *pIdx;
    QDWORD qlNormFreq[25];
    UBYTE  i, j;
    QDWORD qdTemp;
    QWORD  qTemp, qSymbol_rate_r;

    qSymbol_rate_r = V34_tONE_OVER_SYM[sym_rate_idx];

    pIdx = &SrateIdxTab[sym_rate_idx][0];

    qTemp = (QWORD)UBUBMULU(pIdx[lo1], PH2_CONST1);

    for (i = pIdx[lo1]; i <= pIdx[hi2]; i++)
    {
        /* Calculate Normalized Freq based on current Tone idx (i) */
        qdTemp  =  QQMULQD(qSymbol_rate_r, qTemp);
        qdTemp += QQMULQ15(V34_tONE_OVER_SYM_OFFSET[sym_rate_idx], qTemp);

        qlNormFreq[i] = qdTemp;

        qTemp += PH2_CONST1;
    }

    for (j = 0; j < 11; j++)
    {
        for (i = pIdx[lo1]; i <= pIdx[hi2]; i++)
        {
            pqdGain[sym_rate_idx][j][i] = Ph2_Pre_Filter_Gain(qlNormFreq[i], j, 1);
        }
    }
}

void Ph2_Preemp_Idx(UBYTE sym_rate_idx, ProbStruc *probing)
{
    QDWORD qSum_low, qSum_hi;
    QDWORD qTemp_sum_low, qTemp_sum_hi;
    QDWORD qdTemp_0db;
    QDWORD qlTemp2;
    QWORD  qTemp2;
    QWORD  qTemp;
    QWORD  qPower_3db_r;
    UBYTE  i, j;
    UBYTE  logic;
    UBYTE  Selected_Idx_low, Selected_Idx_hi;
    CONST  UBYTE  *pIdx;
    QWORD  *pqL1_Power_Spectrum;
    QWORD  *pqPower_Spectrum;
    QDWORD qlTonePower[25];
    QDWORD qlTonePower_After[25];

    pIdx = &SrateIdxTab[sym_rate_idx][0];

    /* to find the maximum power tone */
    qTemp = 0;
    pqPower_Spectrum = &(probing->pqPower_Spectrum[24]);

    for (i = 0; i < 25; i++)
    {
        if (*pqPower_Spectrum >= qTemp)
        {
            qTemp = *pqPower_Spectrum;
        }

        pqPower_Spectrum--;
    }

    if (qTemp > 256)
    {
        qPower_3db_r = (QWORD)QDQDdivQD(qONE, qTemp); /* qPower_3db_r is 8.8 format */
    }
    else if (qTemp > 0)
    {
        qPower_3db_r = (QWORD)QDQDdivQD(qONE, qTemp); /* qPower_3db_r is 16.0 format */
    }
    else
    {
        probing->error = 1;
        qPower_3db_r = qONE; /* qPower_3db_r is 16.0 format */
    }

    Selected_Idx_low = 0;
    Selected_Idx_hi  = 0;

    qSum_low = Q30_MAX;
    qSum_hi  = Q30_MAX;

    pqL1_Power_Spectrum = &probing->pqL1_Power_Spectrum[pIdx[lo1] - 1];
    pqPower_Spectrum    = &probing->pqPower_Spectrum[pIdx[lo1] - 1];

    qTemp = (QWORD)UBUBMULU(pIdx[lo1], PH2_CONST1);

    for (i = pIdx[lo1]; i <= pIdx[hi2]; i++)
    {
        logic = Ph2_Is6_8_12_16(i);

        if (logic)
        {
            qTemp2 = Ph2_Tone_Power(qTemp, probing->pqPower_Spectrum);

            qlTemp2 = QQMULQD(qTemp2, qPower_3db_r);
        }
        else if (*pqPower_Spectrum == 0)
        {
            /* Use L1 to determine tone magnitude at this frequency */
            qlTemp2 = QQMULQD((*pqL1_Power_Spectrum), qPower_3db_r) >> 2;
        }
        else
        {
            qlTemp2 = QQMULQD((*pqPower_Spectrum), qPower_3db_r);
        }

        qlTonePower[i] = DSPD_Log10(qlTemp2);

        pqL1_Power_Spectrum++;
        pqPower_Spectrum++;

        qTemp += PH2_CONST1;
    }

    for (j = 0; j < 11; j++)
    {
        qTemp_sum_low = 0;
        qTemp_sum_hi  = 0;
        qdTemp_0db    = 0;

        for (i = pIdx[lo1]; i <= pIdx[hi2]; i++)
        {
            qlTonePower_After[i] = qlTonePower[i] + probing->pqdGain[sym_rate_idx][j][i];

#if 0
            if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = qlTonePower_After[i]; }
#endif
            qdTemp_0db += qlTonePower_After[i];
        }

        qdTemp_0db = QDQDdivQD(qdTemp_0db, (QWORD)(pIdx[hi2] - pIdx[lo1] + 1));

        for (i = pIdx[lo1]; i <= pIdx[hi2]; i++)
        {
            qlTemp2 = qlTonePower_After[i] - qdTemp_0db;

            if (qlTemp2 < 0)
            {
                qlTemp2 = -qlTemp2;        /* abs(log10(pqPower[i]*qGain)) */
            }

            if (i < pIdx[hi1])            /* hi_idx1) */
            {
                qTemp_sum_low += qlTemp2;
            }
            else if (i > pIdx[lo2])        /* low_idx2) */
            {
                qTemp_sum_hi += qlTemp2;
            }
            else
            {
                qTemp_sum_low += qlTemp2;
                qTemp_sum_hi  += qlTemp2;
            }
        }

        if (qTemp_sum_low < qSum_low)
        {
            qSum_low = qTemp_sum_low;
            Selected_Idx_low = j;
        }

        if (qTemp_sum_hi < qSum_hi)
        {
            qSum_hi = qTemp_sum_hi;
            Selected_Idx_hi = j;
        }
    }

    probing->lo_preemp = Selected_Idx_low;
    probing->hi_preemp = Selected_Idx_hi;
}

UBYTE Ph2_Check_Edges(UBYTE sym_rate_idx, QWORD *pqPower_lo, QWORD *pqPower_hi, ProbStruc *probing)
{
    QWORD  qLow_edge;
    QWORD  qHigh_edge;
    QWORD  qLow_carrier;
    QWORD  qHigh_carrier;
    QDWORD qdPL_low;
    QDWORD qdPH_low;
    QDWORD qdPL_high;
    QDWORD qdPH_high;
    QWORD  qMaxpower_3dbr_hi;
    QWORD  qMaxpower_3dbr_lo;
    QWORD  bandwidth;
    QWORD  qTemp;
    QWORD  qTemp1;
    UBYTE  edge_perf;
    UBYTE  i;

    qTemp = 0;
    qTemp1 = 0;

    for (i = 0; i < 25; i++)
    {
        if (pqPower_lo[i] > qTemp)
        {
            qTemp  = pqPower_lo[i];
        }

        if (pqPower_hi[i] > qTemp1)
        {
            qTemp1 = pqPower_hi[i];
        }
    }

    if (qTemp > 0)
    {
        qMaxpower_3dbr_lo = (QWORD)QDQDdivQD(qONE, qTemp);
    }
    else
    {
        probing->error = 1;
        qMaxpower_3dbr_lo = qONE; /* qMaxpower_3dbr_lo is 16.0 format */
    }

    if (qTemp1 > 0)
    {
        qMaxpower_3dbr_hi = (QWORD)QDQDdivQD(qONE, qTemp1);
    }
    else
    {
        probing->error = 1;
        qMaxpower_3dbr_hi = qONE; /* qMaxpower_3dbr_hi is 16.0 format */
    }

    qLow_carrier  = V34_tCARRIER[sym_rate_idx][0];
    qHigh_carrier = V34_tCARRIER[sym_rate_idx][1];

    bandwidth = BAND_WIDTH[sym_rate_idx];

    edge_perf = 0;

    /* Check if Low Carrier is OK */
    qLow_edge  = qLow_carrier - bandwidth;
    qHigh_edge = qLow_carrier + bandwidth;

    qTemp  = Ph2_Tone_Power(qLow_edge,  pqPower_lo);
    qTemp1 = Ph2_Tone_Power(qHigh_edge, pqPower_lo);

    qdPL_low = DSPD_Log10(QQMULQD(qTemp,  qMaxpower_3dbr_lo));
    qdPH_low = DSPD_Log10(QQMULQD(qTemp1, qMaxpower_3dbr_lo));

    if ((qdPL_low >= LOWEST_THD) && (qdPH_low >= LOWEST_THD))/*LOWEST_THD is Correct? if remove the condition, we can decide the symbol rate using ATS33=237 (3429). Otherwise, the condition will fail the connect at phase2*/
    {
        edge_perf = 2;
    }

    /* Check if High Carrier is OK */
    qLow_edge  = qHigh_carrier - bandwidth;
    qHigh_edge = qHigh_carrier + bandwidth;

    qTemp  = Ph2_Tone_Power(qLow_edge,  pqPower_hi);
    qTemp1 = Ph2_Tone_Power(qHigh_edge, pqPower_hi);

    qdPL_high = DSPD_Log10(QQMULQD(qTemp,  qMaxpower_3dbr_hi));
    qdPH_high = DSPD_Log10(QQMULQD(qTemp1, qMaxpower_3dbr_hi));

    if ((qdPH_high >= LOWEST_THD) && (qdPL_high >=  LOWEST_THD))/*LOWEST_THD is Correct? if remove the condition, we can decide the symbol rate using ATS33=237 (3429). Otherwise, the condition will fail the connect at phase2*/
    {
        edge_perf ++;
    }

    if (qdPH_high >= qdPL_low)
    {
        edge_perf |= 4;
    }

    if ((sym_rate_idx == 0) && ((edge_perf & 0x3) == 0))
    {
        edge_perf += ((edge_perf & 0x4) == 1 ? 1 : 2);
    }

    return (edge_perf);
}

QWORD Ph2_Tone_Power(QWORD qTone_freq, QWORD *pqPower_Spectrum)
{
    QDWORD qdTemp1;
    QDWORD qdTemp2;
    QWORD  qAlpha;
    QWORD  qBeta;
    QWORD  qTotal;
    QWORD  qTone_power;
    SWORD  ifreq_dn;
    SWORD  ifreq_up;
    UBYTE  logic;

    ifreq_dn = QQMULQ15(qTone_freq, q1_OVER_150);

    if (ifreq_dn < 0)
    {
        ifreq_dn = 0;
    }
    else if (ifreq_dn > 25)
    {
        ifreq_dn = 25;
    }

    logic = Ph2_Is6_8_12_16((UBYTE)ifreq_dn);

    if (logic)
    {
        ifreq_dn--;
    }

    ifreq_up = ifreq_dn + 1;

    logic = Ph2_Is6_8_12_16((UBYTE)ifreq_up);

    if (logic)
    {
        ifreq_up ++;
    }

    qAlpha = (QWORD)QQMULQD(ifreq_dn, PH2_CONST1);

    qAlpha = qTone_freq - qAlpha;

    qBeta  = (QWORD)QQMULQD(ifreq_up, PH2_CONST1);

    qBeta -= qTone_freq;

    qTotal = qAlpha + qBeta;

    if (ifreq_dn == 0)
    {
        qdTemp1 = 0;
    }
    else
    {
        qdTemp1 = QQMULQD(qBeta, pqPower_Spectrum[ifreq_dn - 1]);
    }

    if (ifreq_up > 25)
    {
        qdTemp2 = 0;
    }
    else
    {
        qdTemp2 = QQMULQD(qAlpha, pqPower_Spectrum[ifreq_up - 1]);
    }

    qdTemp1 += qdTemp2;

    qTone_power = (QWORD)QDQDdivQD(qONE, qTotal);
    qTone_power = QDQMULQDR(qdTemp1, qTone_power);

    return(qTone_power);
}

QDWORD Ph2_Pre_Filter_Gain(QDWORD qNorm_freq, SWORD preemp_idx, UBYTE ubMode)
{
    QDWORD qdGain;
    QWORD  qTemp1, qTemp2;
    QDWORD qdTemp;
    UBYTE  i;

    if (preemp_idx > 5)
    {
        if (qNorm_freq < q08)
        {
            qdGain = 0;
        }
        else
        {
            i = (preemp_idx - 6) & 0xFF;
            qdGain = qNorm_freq - q08;
            qdGain = QDQMULQDR(qdGain, V34_tPREEMP_GAMA[i]);
            qdTemp = QQMULQD(V34_tPREEMP_BETA[i], 5);
            qdGain = QDQMULQD0(qdGain, 25) + qdTemp;
        }
    }
    else
    {
        qdGain = QDQMULQDR(qNorm_freq, V34_tPREEMP_ALPHA[preemp_idx]);

        qdGain = QDQMULQD0(qdGain, 20);
    }

    /* qdGain = pow(10, 0.1 *qdGain); */
    /* modify calculation of power, Feb. 6, 1998 */

#if SUPPORT_V90D
    qdTemp = qdGain;
#else  /* 50% preemphasis */

    if (qdGain == 0)
    {
        qdTemp = 0;
    }
    else
    {
        qdTemp = qdGain << 1;
    }

#endif

    if (ubMode == 1)
    {
        qdTemp = QDQMULQD(qdTemp, q01);

        return (qdTemp);/*Logarithmic gain (dB) in Q15 format*/
    }
    else
    {
        i = 0;

        while (qdTemp > qONE)
        {
            qdTemp -= qONE;
            i++;
        }

        qTemp1 = (QWORD)qdTemp;

        qTemp2 = QQMULQ15(qTemp1, PH2_CONST2) + 7545;

        qTemp2 = QQMULQ15(qTemp1, qTemp2);

        qdTemp = (QDWORD)qONE + qTemp2;

        qdGain = QDQMULQDR8(qdTemp, POWER_INDEX[i]);

        return(qdGain);/*Linear gain in Q15 format*/
    }
}
