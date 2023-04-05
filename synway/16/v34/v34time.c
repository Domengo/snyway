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

#include "v34ext.h"

#if V34DRAWTIME
#include "graf.h"

ERRDRAW DrawTime;
UBYTE doneOnce = 0;
#endif

#if 0 /// LLL temp
void  V34_TimingLoop_Init(TimeCarrRecovStruc *pTCR)
{
    pTCR->qT_Error_Out1 = 0;
    pTCR->qT_Error_Out2 = 0;

#if V34DRAWTIME

    /* draw err init */
    if (!doneOnce)
    {
        draw_err_init(&DrawTime, 240, 155, 80);
        doneOnce = 1;
    }

#endif
}
#endif

#if !USE_ASM
void  V34_TimingLoop(TimeCarrRecovStruc *pTCR, CQWORD *cqUs, CQWORD *cqD_sym)
{
    CQWORD *pcY, *pcD;
    QWORD qErr_input;
    QWORD qPLL_output1;
    QWORD qPLL_output2;
    QDWORD qdTempR, qdTempI;
    QWORD qTemp;

    pTCR->pcBaud_adjust[0] = pTCR->pcBaud_adjust[1];
    pTCR->pcBaud_adjust[1] = pTCR->pcBaud_adjust[2];
    pTCR->pcBaud_adjust[2] = *cqUs;

    pTCR->pcT3_delay[0]     = pTCR->pcT3_delay[1];
    pTCR->pcT3_delay[1]     = *cqD_sym;

    qPLL_output1            = pTCR->qT_Error_Out1;
    qPLL_output2            = pTCR->qT_Error_Out2;

    pcY = pTCR->pcBaud_adjust;
    pcD = pTCR->pcT3_delay;

    /* Input Error = Re[pcD(k-1)-pcY(k-1)] * Re[pcY(k)-pcY(k-2)] + Im[pcD(k-1)-pcY(k-1)] * Im[pcY(k)-pcY(k-2)] */
    qdTempR = (QDWORD)pcY[2].r - pcY[0].r; /* need 32bit result so that overflow can detect */
    qTemp = (QWORD)(qdTempR >> 2);/* To avoid overflow */
    qdTempR = QQMULQD(pcD[0].r, qTemp) - QQMULQD(pcY[1].r, qTemp);

    qdTempI = (QDWORD)pcY[2].i - pcY[0].i; /* need 32bit result so that overflow can detect */
    qTemp = (QWORD)(qdTempI >> 2);/* To avoid overflow */
    qdTempI = QQMULQD(pcD[0].i, qTemp) - QQMULQD(pcY[1].i, qTemp);

    qdTempR = (qdTempR + qdTempI) >> 13; /* To Q15 */

    /* To avoid overflow */
    if (qdTempR < -32768)
    {
        qErr_input = -32768;
    }
    else if (qdTempR > 32767)
    {
        qErr_input = 32767;
    }
    else
    {
        qErr_input = (QWORD)qdTempR;
    }

    qPLL_output1 += qErr_input;

    qTemp = QQMULQR15(qPLL_output1, V34_CONST1);

    qPLL_output2 += ((qErr_input >> 2) + qTemp);

    /* Adjust timing reconstruction pointer in Timing_Rec */
    if (qPLL_output2 >= V34_qTIMING_THRES)
    {
        pTCR->nTimingIdx++;
        qPLL_output2 = 0;
    }
    else if (qPLL_output2 <= - V34_qTIMING_THRES)
    {
        pTCR->nTimingIdx--;
        qPLL_output2 = 0;
    }

#if 0 /* Timing drift click disabled */
    pTCR->drift_count++;

    if (pTCR->drift_count >= pTCR->drift_thresh)
    {
        pTCR->nTimingIdx += pTCR->drift_direction;
        pTCR->drift_count -= pTCR->drift_thresh;
    }

#endif

#if V34DRAWTIME
    /* draw coef */
    draw_err(&DrawTime, qPLL_output2 * 10.0 / 32768.0);
#endif

    pTCR->qT_Error_Out1 = qPLL_output1;
    pTCR->qT_Error_Out2 = qPLL_output2;

    if (pTCR->nTimingIdx < 0)
    {
        pTCR->sample_slip_count--;
    }
    else if (pTCR->nTimingIdx >= V34_INTER_POINT)
    {
        pTCR->sample_slip_count++;
    }
}
#endif

void  V34_Shift_Buf(CQWORD *pcBuf, UBYTE ubBuf_len, CQWORD cqNewIn)
{
    UBYTE i;

    for (i = 0; i < (ubBuf_len - 1); i++)
    {
        pcBuf[i] = pcBuf[i+1];
    }

    pcBuf[ubBuf_len - 1] = cqNewIn;
}

#if 0 /// LLL temp
void  V34_Timing_Rec_Init(TimeCarrRecovStruc *pTCR)
{
    pTCR->nTimingIdx = 0;/* It was 2. 7 is the best when the S_EQ has 8 taps. */

    /* init the interpolation Firter */
    pTCR->pcDline = pTCR->pcPolyDline;
    pTCR->pqCoef  = (QWORD *)PolyPhaseFiltCoef14;
    pTCR->offset  = 0;

    pTCR->baud_adjust_in  = 0;
    pTCR->baud_adjust_out = 15;

    pTCR->sample_slip_count = 0;
}
#endif

void  V34_Timing_Rec(TimeCarrRecovStruc *pTCR, CQWORD *pqUin, CQWORD *pcUout)
{
    CQWORD     *pcDptr, *pcDptr_end;
    CQWORD     cqSym;
    UWORD      foffset;
    CQWORD     *pcFdptr;
    QWORD      *pqFcoef_ptr1, *pqFcoef_ptr2;
    CQWORD     Temp_cqOut1, Temp_cqOut2;
    UBYTE      i;

    if (pTCR->nTimingIdx < 0)
    {
        if (pTCR->offset < 1)
        {
            pTCR->offset = TIMING_TAPS14 - 1;
        }
        else
        {
            pTCR->offset --;
        }

        if (pTCR->baud_adjust_out < 1)
        {
            pTCR->baud_adjust_out = V34_BAUD_ADJUST_LEN - 1;
        }
        else
        {
            pTCR->baud_adjust_out --;
        }

        pTCR->nTimingIdx += V34_INTER_POINT;
    }
    else if (pTCR->nTimingIdx >= V34_INTER_POINT)
    {
        cqSym = pTCR->pcBaud_adjust_Dline[pTCR->baud_adjust_out++];

        if (pTCR->baud_adjust_out >= V34_BAUD_ADJUST_LEN)
        {
            pTCR->baud_adjust_out = 0;
        }

        pTCR->pcDline[pTCR->offset] = cqSym;

        pTCR->pcDline[pTCR->offset + TIMING_TAPS14] = cqSym;

        pTCR->offset ++;

        if (pTCR->offset >= TIMING_TAPS14)
        {
            pTCR->offset = 0;
        }

        pTCR->nTimingIdx -= V34_INTER_POINT;
    }

    if (pTCR->sample_slip_count <= -3)
    {
        /* Retard, Demodulator should check slip_count <= -3, and don't */
        /* do demodulation for one buffer(3 samples) is slip_count <= -3 */
        pTCR->sample_slip_count = 0;

        /* Do one more symbol buffer since we just retard one buffer    */
        pTCR->Tcount ++;
    }
    else
    {
        /* Feed data into baud_adjust delayline */
        pcDptr = &(pTCR->pcBaud_adjust_Dline[pTCR->baud_adjust_in]);

        for (i = 0; i < 3; i++)
        {
            pcDptr->r = pqUin[i].r;
            pcDptr->i = pqUin[i].i;
            pcDptr ++;
        }

        pTCR->baud_adjust_in += 3;

        if (pTCR->baud_adjust_in >= V34_BAUD_ADJUST_LEN)
        {
            pTCR->baud_adjust_in -= V34_BAUD_ADJUST_LEN;
        }

        if (pTCR->sample_slip_count >= 3)
        {
            /* Advance, Skip the output, don't do interpolation for one   */
            /* buffer, and set Tcount - 1 (i.e. one less symbol output)   */
            pTCR->sample_slip_count = 0;

            pTCR->Tcount --;

            return;
        }
    }

    pcDptr     = &(pTCR->pcBaud_adjust_Dline[pTCR->baud_adjust_out]);
    pcDptr_end = &(pTCR->pcBaud_adjust_Dline[V34_BAUD_ADJUST_LEN - 1]);

    pTCR->baud_adjust_out += 3;

    if (pTCR->baud_adjust_out >= V34_BAUD_ADJUST_LEN)
    {
        pTCR->baud_adjust_out -= V34_BAUD_ADJUST_LEN;
    }

    /* Calculate the position of the coefficient set */
    pTCR->pqCoef = (QWORD *)PolyPhaseFiltCoef14 + SBQMULQ(TIMING_TAPS14, (pTCR->nTimingIdx >> 1));

    foffset = pTCR->offset;

    if (pTCR->nTimingIdx & 0x0001)
    {
        for (i = 0; i < 3; i++)
        {
            pcFdptr = &(pTCR->pcPolyDline[foffset]);

            pqFcoef_ptr1 = pTCR->pqCoef;
            pqFcoef_ptr2 = pTCR->pqCoef + TIMING_TAPS14;

            *pcFdptr             = *pcDptr;  /* Overwrite the oldest sample */
            pcFdptr[TIMING_TAPS14] = *pcDptr++;/* Double buffering */
            pcFdptr++;

            Temp_cqOut1 = Timing_Interpolation(pcFdptr, pqFcoef_ptr1, TIMING_TAPS14);
            Temp_cqOut2 = Timing_Interpolation(pcFdptr, pqFcoef_ptr2, TIMING_TAPS14);

            pcUout[i].r = (QWORD)(((QDWORD)Temp_cqOut1.r + Temp_cqOut2.r) >> 1);
            pcUout[i].i = (QWORD)(((QDWORD)Temp_cqOut1.i + Temp_cqOut2.i) >> 1);

            foffset++;

            if (foffset >= TIMING_TAPS14)
            {
                foffset = 0;
            }

            if (pcDptr > pcDptr_end)
            {
                pcDptr -= V34_BAUD_ADJUST_LEN;
            }
        }
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            pcFdptr = &(pTCR->pcPolyDline[foffset]);

            pqFcoef_ptr1 = pTCR->pqCoef;

            *pcFdptr             = *pcDptr;  /* Overwrite the oldest sample */
            pcFdptr[TIMING_TAPS14] = *pcDptr++;/* Double buffering         */
            pcFdptr++;

            pcUout[i] = Timing_Interpolation(pcFdptr, pqFcoef_ptr1, TIMING_TAPS14);

            foffset++;

            if (foffset >= TIMING_TAPS14)
            {
                foffset = 0;
            }

            if (pcDptr > pcDptr_end)
            {
                pcDptr -= V34_BAUD_ADJUST_LEN;
            }
        }
    }

    pTCR->offset = foffset;
}
