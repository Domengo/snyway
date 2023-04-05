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

void V34Fax_Timing_Recovery_Init(V34FaxStruct *pV34Fax)
{
    UBYTE i;

    if (pV34Fax->FreqOffset_Enable == 0)
    {
        pV34Fax->Dline_feed_flag = 0;
        pV34Fax->PPS_detect_flag = 0;

        pV34Fax->max_R = -30000;
        pV34Fax->min_R =  30000;

        pV34Fax->max_I = -30000;
        pV34Fax->min_I =  30000;
    }

    pV34Fax->nTimIdx = 0;

    for (i = 0; i < 2 * V34FAX_TIMING_DELAY_HALF; i++)
    {
        pV34Fax->cTimDline[i].r = 0;
        pV34Fax->cTimDline[i].i = 0;
    }
}


void V34Fax_Timing_Recovery(V34FaxStruct *pV34Fax)
{
    QWORD ifac;                      /* linear interpolation factor */
    QWORD qSam1_r, qSam1_i;
    QWORD qSam2_r, qSam2_i;
    CQWORD *pcTimSam;

    pcTimSam = (pV34Fax->cTimDline + pV34Fax->ubOffset + V34FAX_TIMING_DELAY_HALF) -
               V34FAX_SYM_LEN + (pV34Fax->nTimIdx >> V34FAX_TIMELINE_RES);

    /* linear interpolation */
    ifac = (QWORD)(pV34Fax->nTimIdx & V34FAX_TIMELINE_INTERP);

    /* first point */
    qSam1_r = pcTimSam[0].r;
    qSam1_i = pcTimSam[0].i;

    qSam2_r = pcTimSam[1].r;
    qSam2_i = pcTimSam[1].i;

    pV34Fax->cTimBufIQ[0].r = (QWORD)(qSam1_r + ((((QDWORD)qSam2_r - qSam1_r) * ifac) >> V34FAX_TIMELINE_RES));
    pV34Fax->cTimBufIQ[0].i = (QWORD)(qSam1_i + ((((QDWORD)qSam2_i - qSam1_i) * ifac) >> V34FAX_TIMELINE_RES));

    /* next sample */
    pcTimSam += 6;

    /* second point */
    qSam1_r = pcTimSam[0].r;
    qSam1_i = pcTimSam[0].i;
    qSam2_r = pcTimSam[1].r;
    qSam2_i = pcTimSam[1].i;
    pV34Fax->cTimBufIQ[1].r = (QWORD)(qSam1_r + ((((QDWORD)qSam2_r - qSam1_r) * ifac) >> V34FAX_TIMELINE_RES));
    pV34Fax->cTimBufIQ[1].i = (QWORD)(qSam1_i + ((((QDWORD)qSam2_i - qSam1_i) * ifac) >> V34FAX_TIMELINE_RES));
}


void V34Fax_Timing_Update_Init(V34FaxStruct *pV34Fax)
{
    UBYTE i;

    pV34Fax->nTimError = 0;
    pV34Fax->nTimE1    = 0;

    for (i = 0; i < 2; i++)
    {
        pV34Fax->pcRotateIQ[i].r = 0;
        pV34Fax->pcRotateIQ[i].i = 0;
    }

    for (i = 0; i < 3; i++)
    {
        pV34Fax->pcSliceIQ[i].r = 0;
        pV34Fax->pcSliceIQ[i].i = 0;
    }

    pV34Fax->timing_adjust_flag = 1;
}


void V34Fax_Timing_Update(V34FaxStruct *pV34Fax)
{
    QWORD Temp1, Temp2;

    pV34Fax->pcRotateIQ[1] = pV34Fax->pcRotateIQ[0];
    pV34Fax->pcRotateIQ[0] = pV34Fax->cqRotateIQ;

    pV34Fax->pcSliceIQ[2] = pV34Fax->pcSliceIQ[1];
    pV34Fax->pcSliceIQ[1] = pV34Fax->pcSliceIQ[0];
    pV34Fax->pcSliceIQ[0] = pV34Fax->cqSliceIQ;

    Temp1 = 0;
    Temp2 = 0;

    if (pV34Fax->pcSliceIQ[0].r > pV34Fax->pcSliceIQ[2].r)
    {
        Temp1 = pV34Fax->pcSliceIQ[1].r - pV34Fax->pcRotateIQ[1].r;
    }
    else if (pV34Fax->pcSliceIQ[0].r < pV34Fax->pcSliceIQ[2].r)
    {
        Temp1 = pV34Fax->pcRotateIQ[1].r - pV34Fax->pcSliceIQ[1].r;
    }

    if (pV34Fax->pcSliceIQ[0].i > pV34Fax->pcSliceIQ[2].i)
    {
        Temp2 = pV34Fax->pcSliceIQ[1].i - pV34Fax->pcRotateIQ[1].i;
    }
    else if (pV34Fax->pcSliceIQ[0].i < pV34Fax->pcSliceIQ[2].i)
    {
        Temp2 = pV34Fax->pcRotateIQ[1].i - pV34Fax->pcSliceIQ[1].i;
    }

    Temp1 += Temp2;

    pV34Fax->nTimE1 += Temp1;

    pV34Fax->nTimError += (QWORD)((Temp1 >> 3) + (pV34Fax->nTimE1 >> 13));

    if (pV34Fax->nTimError > 1000)
    {
        pV34Fax->nTimIdx += V34FAX_TIM_STEP;
        pV34Fax->nTimError = 0;
    }

    if (pV34Fax->nTimError < -1000)
    {
        pV34Fax->nTimIdx -= V34FAX_TIM_STEP;
        pV34Fax->nTimError = 0;
    }

    if (pV34Fax->timing_adjust_flag)
    {
        if ((pV34Fax->nTimIdx < V34FAX_TIMEIDX_MAX) || (pV34Fax->nTimIdx > -1))
        {
            pV34Fax->timing_adjust_flag = 0;
        }
    }
    else
    {
        if (pV34Fax->nTimIdx < V34FAX_TIMEIDX_MAX)
        {
            pV34Fax->nTimIdx = V34FAX_TIMEIDX_MAX;
        }

        if (pV34Fax->nTimIdx > -1)
        {
            pV34Fax->nTimIdx = -1;
        }

        if ((pV34Fax->nTimIdx < V34FAX_TIMEIDX_THRES1) && (pV34Fax->nTimIdx > V34FAX_TIMEIDX_THRES2))
        {
            pV34Fax->timing_adjust_flag = 1;
        }
    }
}

#endif
