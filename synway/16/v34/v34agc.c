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

/* ===========================Automatic Gain Control========================= *
*      Input:  COMPLEX input sample:in                                        *
*              AGC window range: low1, low2, high1, high2 and                 *
*              AGC algorithm step-size: k1, k2, defined in agc2.h/sinit3.c    *
*      Output: rx_AGC.gain , This struct is defined in V34Stru.h as AGC2      *
*      Authors:  Xian Ji and Sherry, algorithm R&D, GAO R&C Ltd.              *
*                Hanyu Liu, S/W test, verification and intergration           *
*                May 31,1996, DLB testing, pass Phase4 training               *
* =========================================================================== */

#include "v34ext.h"

void  V34_Agc_Init(V34AgcStruc *pAGC)
{
    pAGC->update_counter = V34_AGC_AVE_LEN;
    pAGC->qAve_egy       = 0;
    pAGC->freeze         = 0;
    pAGC->qGain          = pAGC->AGC_gain;
    pAGC->freeze_EQ      = 0;
}

void  V34_Agc(V34AgcStruc *pAGC, CQWORD *pcIn)
{
    SBYTE  freeze_eq = 0;
    QDWORD qError_agc, qdEnergy, qdEnergy_input;
    QDWORD qdTemp;
    QWORD  H1, H2, L1, L2, K1, K2;

    H1 = pAGC->qHLK[0];
    H2 = pAGC->qHLK[1];
    L1 = pAGC->qHLK[2];
    L2 = pAGC->qHLK[3];
    K1 = pAGC->qHLK[4];
    K2 = pAGC->qHLK[5];

    qdEnergy_input = pAGC->qAve_egy;

    /* ---------------------  Average qEnergy incoming  ---------------- */

    /* qEnergy of 2nd demodulated point */
    ++pcIn;
    qdEnergy  = QQMULQD(pcIn->r, pcIn->r) + QQMULQD(pcIn->i, pcIn->i);

    /* Multiply Energy by V34_qAGC_EN_COEF (1/16)  and do NORMRNDA */
    qdEnergy  = (qdEnergy + 0x40000) >> 19;

    qdEnergy_input += qdEnergy;

    pAGC->update_counter --;

    if (pAGC->update_counter == 0)
    {
        if (pAGC->freeze == 0)
        {
            if (qdEnergy_input > H2)
            {
                freeze_eq = 1;

                pAGC->qGain >>= 1;
            }
            else if (qdEnergy_input > H1)
            {
                freeze_eq = 0;

                qdTemp = (qdEnergy_input << 15) / H1; /* qdEnergy_input <= H2 (H2<=25031) */

                qError_agc = qdTemp - qONE;

                qdTemp = QDQMULQD(qError_agc, K2);
                qdTemp = qONE - qdTemp;

                /* qGain_alpher0*=(1-k2*qError_agc); */
                pAGC->qGain = (QWORD)QDQMULQD(qdTemp, pAGC->qGain);
            }
            else if (qdEnergy_input < L2)
            {
                freeze_eq = -1;

                pAGC->qGain <<= 1;
            }
            else if (qdEnergy_input < L1)
            {
                freeze_eq = 0;

                qdTemp = (qdEnergy_input << 15) / L1; /* qdEnergy_input < L1 (L1<=2990) */

                qError_agc = qdTemp - qONE;           /* -23540 ~ 0 */

                qdTemp = QDQMULQD(qError_agc, K1);    /* -18827 ~ 0 */
                qdTemp = qONE - qdTemp;               /*  51594 ~ 32767 */

                /* qGain_alpher0 *= (1-k1*qError_agc); */
                pAGC->qGain = (QWORD)QDQMULQD(qdTemp, pAGC->qGain); /* Output global gain factor */
            }

            if (pAGC->qGain < 0)
            {
                pAGC->qGain = -pAGC->qGain;
            }
        }
        else
        {
            freeze_eq = 0;
        }

        pAGC->freeze_EQ = freeze_eq;/* IF freeze_eq=1, disable updating the Eq coefficients */

        pAGC->update_counter = V34_AGC_AVE_LEN;
        qdEnergy_input = 0;/* Reset for the next average qEnergy compution */
    }

    pAGC->qAve_egy = (QWORD)qdEnergy_input;
}
