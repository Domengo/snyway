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

#include "v34fext.h"

#if SUPPORT_V34FAX

void V34Fax_Timing_Rec_Init(V34FaxStruct *pV34Fax)
{
    TimeCarrRecovStruc *pTCR = pV34Fax->pTCR;
    UBYTE i;

    pTCR->nTimingIdx = 17;
    pTCR->pcDline = pV34Fax->V34Fax_Delay_Line;
    pTCR->pqCoef  = (QWORD *)PolyPhaseFiltCoef14;

    for (i = 0; i < V34FAX_DLINE_DLEN; i++)
    {
        pTCR->pcDline[i].r = 0;
        pTCR->pcDline[i].i = 0;
    }

    pTCR->baud_adjust_in  = 18;
    pTCR->baud_adjust_out = 0;      /* 3 samples delay in case */

    pV34Fax->Proc_Timing_Num = 0;
    pV34Fax->Proc_Timing_In  = 0;
    pV34Fax->Proc_Timing_Out = 0;

    if (pV34Fax->FreqOffset_Enable == 0)
    {
        pV34Fax->Dline_feed_flag = 0;
        pV34Fax->PPS_detect_flag = 0;
        pV34Fax->max_R = -30000;
        pV34Fax->max_I =  30000;
        pV34Fax->min_R = -30000;
        pV34Fax->min_I =  30000;
    }
}

void V34Fax_TimingLoop_Init(V34FaxStruct *pV34Fax)
{
    TimeCarrRecovStruc *pTCR = pV34Fax->pTCR;

    pTCR->qT_Error_Out1 = 0;
    pTCR->qT_Error_Out2 = 0;

    pTCR->drift_count = 0;

    if (pV34Fax->FreqOffset_Enable == 0)
    {
        pTCR->drift_thresh    = 100;
        pTCR->drift_direction = 0;  /* Turn off at beginning */

        pV34Fax->uTimingLoopThres = V34FAX_TIMING_THRES;
        pV34Fax->uTimingLoopStep  = V34FAX_TIMING_STEP;
    }
    else
    {
        pTCR->drift_thresh    = pV34Fax->drift_thresh; /* Saved from first train */
        pTCR->drift_direction = pV34Fax->drift_direction;

        pV34Fax->uTimingLoopThres = 255;
        pV34Fax->uTimingLoopStep  = 1;
    }
}


void V34Fax_TimingLoop_Init_ForPP_and_Data(V34FaxStruct *pV34Fax)
{
    TimeCarrRecovStruc *pTCR = pV34Fax->pTCR;

    pTCR->drift_count     = 0;
    pTCR->drift_thresh    = pV34Fax->drift_thresh; /* Saved from first train */
    pTCR->drift_direction = pV34Fax->drift_direction;

    pV34Fax->uTimingLoopThres = 2000;
    pV34Fax->uTimingLoopStep  = 1;

    pV34Fax->timAdjust      = 0;
    pV34Fax->timAdjustCount = 0;
}

void V34Fax_TimingLoop(V34FaxStruct *pV34Fax, CQWORD *cqUs, CQWORD *cqD_sym)
{
    CQWORD *pcY, *pcD;
    QWORD qErr_input;
    QWORD qPLL_output1;
    QWORD qPLL_output2;
    QDWORD qdTempR, qdTempI;
    QWORD qTemp;
    TimeCarrRecovStruc *pTCR = pV34Fax->pTCR;

    pTCR->pcBaud_adjust[0] = pTCR->pcBaud_adjust[1];
    pTCR->pcBaud_adjust[1] = pTCR->pcBaud_adjust[2];
    pTCR->pcBaud_adjust[2] = *cqUs;

    pTCR->pcT3_delay[0]    = pTCR->pcT3_delay[1];
    pTCR->pcT3_delay[1]    = *cqD_sym;

    qPLL_output1           = pTCR->qT_Error_Out1;
    qPLL_output2           = pTCR->qT_Error_Out2;

    pcY = pTCR->pcBaud_adjust;
    pcD = pTCR->pcT3_delay;

    /* Input Error = Re[pcD(k-1)-pcY(k-1)] * Re[pcY(k)-pcY(k-2)] + Im[pcD(k-1)-pcY(k-1)] * Im[pcY(k)-pcY(k-2)] */
    qdTempR = (QDWORD)pcY[2].r - pcY[0].r; /* need 32bit result so that overflow can detect */
    qTemp = QSAT(qdTempR);/* Perform saturation */
    qdTempR = QQMULQD(pcD[0].r, qTemp) - QQMULQD(pcY[1].r, qTemp);

    qdTempI = (QDWORD)pcY[2].i - pcY[0].i; /* need 32bit result so that overflow can detect */
    qTemp = QSAT(qdTempI);/* Perform saturation */
    qdTempI = QQMULQD(pcD[0].i, qTemp) - QQMULQD(pcY[1].i, qTemp);

    qErr_input = QDR15Q(qdTempR + qdTempI);

    if (pTCR->drift_direction == 0)
    {
        qPLL_output1 += qErr_input;

        qTemp = QQMULQR15(qPLL_output1, 49);

        qPLL_output2 += ((qErr_input >> 2) + qTemp);
    }
    else
    {
        qPLL_output2 += qErr_input;
    }

    pV34Fax->timAdjustCount++;

    /* Adjust timing reconstruction pointer in Timing_Rec */
    if ((QDWORD)qPLL_output2 >= (QDWORD)pV34Fax->uTimingLoopThres)
    {
        pTCR->nTimingIdx += pV34Fax->uTimingLoopStep;
        qPLL_output2 = 0;

        pV34Fax->timAdjust += pV34Fax->uTimingLoopStep;
    }
    else if ((QDWORD)qPLL_output2 <= -(QDWORD)pV34Fax->uTimingLoopThres)
    {
        pTCR->nTimingIdx -= pV34Fax->uTimingLoopStep;
        qPLL_output2 = 0;

        pV34Fax->timAdjust -= pV34Fax->uTimingLoopStep;
    }

    pTCR->qT_Error_Out1 = qPLL_output1;
    pTCR->qT_Error_Out2 = qPLL_output2;

    if (pTCR->drift_direction != 0)
    {
        if (pV34Fax->FreqOffset_Enable == 0)
        {
            pTCR->drift_count += 8;
        }
        else
        {
            pTCR->drift_count ++;
        }

        if ((QDWORD)pTCR->drift_count >= (QDWORD)pTCR->drift_thresh)
        {
            pTCR->nTimingIdx  += pTCR->drift_direction;
            pTCR->drift_count -= pTCR->drift_thresh;
        }
    }
}

static CQWORD V34Fax_Timing_Interpolation(CQWORD *pcDptr, QWORD *pqCptr)
{
    UBYTE   i;
    QDWORD  qOutput_r;
    QDWORD  qOutput_i;
    CQWORD  cqOutput;

    qOutput_r = 0;
    qOutput_i = 0;

    for (i = 0; i < TIMING_TAPS14 - 1; i++)
    {
        qOutput_r += QQMULQD(pcDptr->r, *pqCptr);
        qOutput_i += QQMULQD(pcDptr->i, *pqCptr);
        pqCptr++;
        pcDptr++;
    }

    qOutput_r += QQMULQD(pcDptr->r, *pqCptr);
    cqOutput.r = QDR15Q(qOutput_r);

    qOutput_i += QQMULQD(pcDptr->i, *pqCptr);
    cqOutput.i = QDR15Q(qOutput_i);

    return(cqOutput);
}

UBYTE V34Fax_Timing_Rec(V34FaxStruct *pV34Fax, CQWORD *pqUin, CQWORD *pcUout)
{
    CQWORD *pcDptr;
    CQWORD *pcDptr1;
    QWORD *pqCptr;
    QWORD *pqCptr1;
    UBYTE  i, sym_num;
    CQWORD cqOutput, cqOutput1;
    SWORD pt1, pt2;
    TimeCarrRecovStruc  *pTCR = pV34Fax->pTCR;

    pt1 = pTCR->baud_adjust_in;
    pt2 = pt1 + V34FAX_DLINE_LEN;

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        pTCR->pcDline[pt1++] = pqUin[i];
        pTCR->pcDline[pt2++] = pqUin[i];
    }

    pTCR->baud_adjust_in += 3;

    if (pTCR->baud_adjust_in >= V34FAX_DLINE_LEN)
    {
        pTCR->baud_adjust_in = 0;
    }

    sym_num = 3;

    if (pTCR->nTimingIdx < 0)
    {
        pTCR->nTimingIdx += V34FAX_MAX_INDEX;
        sym_num = 4;
        pTCR->baud_adjust_out--;                /* repeat one */

        if (pTCR->baud_adjust_out < 0)
        {
            pTCR->baud_adjust_out = V34FAX_DLINE_LEN - 1;
        }
    }
    else if (pTCR->nTimingIdx >= V34FAX_MAX_INDEX)
    {
        pTCR->nTimingIdx -= V34FAX_MAX_INDEX;
        sym_num = 2;
        pTCR->baud_adjust_out++;                /* skip one */

        if (pTCR->baud_adjust_out >= V34FAX_DLINE_LEN)
        {
            pTCR->baud_adjust_out = 0;
        }
    }

    if ((pTCR->nTimingIdx & 0x0001) && (pTCR->nTimingIdx != (V34FAX_MAX_INDEX - 1)))
    {
        for (i = 0; i < sym_num; i++)
        {
            pcDptr = &(pTCR->pcDline[pTCR->baud_adjust_out]);
            pqCptr = pTCR->pqCoef + (pTCR->nTimingIdx >> 1) * TIMING_TAPS14;

            pqCptr1 = pqCptr + TIMING_TAPS14;
            pcDptr1 = pcDptr;

            cqOutput  = V34Fax_Timing_Interpolation(pcDptr, pqCptr);
            cqOutput1 = V34Fax_Timing_Interpolation(pcDptr, pqCptr1);

            pcUout[i].r = (QWORD)(((QDWORD)cqOutput.r + cqOutput1.r) >> 1);
            pcUout[i].i = (QWORD)(((QDWORD)cqOutput.i + cqOutput1.i) >> 1);

            pTCR->baud_adjust_out++;

            if (pTCR->baud_adjust_out >= V34FAX_DLINE_LEN)
            {
                pTCR->baud_adjust_out = 0;
            }
        }
    }
    else
    {
        for (i = 0; i < sym_num; i++)
        {
            pcDptr = &(pTCR->pcDline[pTCR->baud_adjust_out]);
            pqCptr = pTCR->pqCoef + (pTCR->nTimingIdx >> 1) * TIMING_TAPS14;

            cqOutput  = V34Fax_Timing_Interpolation(pcDptr, pqCptr);
            pcUout[i] = cqOutput;

            pTCR->baud_adjust_out++;

            if (pTCR->baud_adjust_out >= V34FAX_DLINE_LEN)
            {
                pTCR->baud_adjust_out = 0;
            }
        }
    }

    return sym_num;
}

#endif
