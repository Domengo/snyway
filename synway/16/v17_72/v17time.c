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

#include "v17ext.h"

#if SUPPORT_V17/* The switch is only for compiling, cannot delete!!! */

void  V17_RX_TimeJam(V32ShareStruct *pV32Share)
{
    UBYTE   i;
    UBYTE   imin;
    CQWORD  *tIn;
    QWORD   qS;
    QWORD   qC;
    QDWORD  qTemp;
    QDWORD  qTemp1;
    QWORD   qRe;
    QWORD   qIm;
    QDWORD  qdTheta;

    tIn = pV32Share->Poly.pcqTimingDlineHead - V32_SYM_SIZE;

    imin = 0;

    /* Choose starting index: 0, 1,2 or 3 */
    if (pV32Share->sbTimeJamCnt == V17_TOTAL_SAMPLE)
    {
        imin = 0;    /* best */
    }

    for (i = imin; i < V32_SYM_SIZE; i++)
    {
        if ((pV32Share->sbTimeJamCnt--) > 0)
        {
            qC =  DSP_tCOS_TABLE[pV32Share->ubJam_cos_phase_idx];
            qS = -DSP_tCOS_TABLE[pV32Share->ubJam_sin_phase_idx];
            /* DFT of I-value of AB tone */
            pV32Share->cqAB_I.r += QQMULQR15(tIn[i].r, qC);    /* for TI code Harry */
            pV32Share->cqAB_I.i += QQMULQR15(tIn[i].r, qS);

            /* DFT of Q-value of AB tone */
            pV32Share->cqAB_Q.r += QQMULQR15(tIn[i].i, qC);    /* for TI code Harry */
            pV32Share->cqAB_Q.i += QQMULQR15(tIn[i].i, qS);

            pV32Share->ubJam_cos_phase_idx = (pV32Share->ubJam_cos_phase_idx + V17_JAM_DPH_IDX) & 0xFF;
            pV32Share->ubJam_sin_phase_idx = (pV32Share->ubJam_sin_phase_idx + V17_JAM_DPH_IDX) & 0xFF;
        }
        else
        {
            break;
        }
    }

    if (pV32Share->sbTimeJamCnt <= 0)
    {
        qTemp  = QQMULQD(pV32Share->cqAB_I.r, pV32Share->cqAB_I.r) +   /* for TI code Harry */
                 QQMULQD(pV32Share->cqAB_I.i, pV32Share->cqAB_I.i);

        qTemp1 = QQMULQD(pV32Share->cqAB_Q.r, pV32Share->cqAB_Q.r) +
                 QQMULQD(pV32Share->cqAB_Q.i, pV32Share->cqAB_Q.i);

        /* Can use either DFT result to calculate timing phase */
        /* Use larger of two results for better precision */
        if (qTemp > qTemp1)
        {
            qRe = pV32Share->cqAB_I.r;
            qIm = pV32Share->cqAB_I.i;
        }
        else
        {
            qRe = pV32Share->cqAB_Q.r;
            qIm = pV32Share->cqAB_Q.i;
        }

        qdTheta = DSPD_Atan2(qRe, qIm);

        /* 180 degrees represents a shift in timing index of one symbol */

        if (qdTheta < qD_90)
        {
            qdTheta = (qD_90  - qdTheta);
        }
        else
        {
            qdTheta = (qD_270 - qdTheta);
        }

        pV32Share->Poly.nTimingIdx = QQMULQR15((QWORD)qdTheta, V17_TIME_CONST);
        pV32Share->ubTimeJamOK     = 1;
    }
}

void V17_RX_TimeJam_Init(V32ShareStruct *pV32Share)
{
    pV32Share->ubJam_cos_phase_idx = 0;
    pV32Share->ubJam_sin_phase_idx = 192;         /* sin(x) = cos(x+(3*pi)/2)) */

    pV32Share->cqAB_I.r     = 0;
    pV32Share->cqAB_I.i     = 0;
    pV32Share->cqAB_Q.r     = 0;
    pV32Share->cqAB_Q.i     = 0;

    pV32Share->ubTimeJamOK  = 0;
    pV32Share->sbTimeJamCnt = V17_TOTAL_SAMPLE;
}

#endif
