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

/* -----------32-point one-dimensional FFT  -------------------- */
/*                    do at most 256-point FFT                   */
/*  NOTE: Do not compile this code with optimization. Why???     */
/*****************************************************************/

#include "ph2ext.h"

void Ph2_FFT(FFT_Data *I, CQWORD *pcIn, CQWORD *pcOut)
{
    CQWORD cqa;
    CQWORD cqP;
    CQWORD *pcW_vec;
    UBYTE cntr;
    UBYTE num_of_butf;
    UBYTE w_vec_idx;
    UBYTE arg;
    UBYTE i;
    UBYTE j;

    UBYTE wide_of_butf;
    UBYTE dist_to_arg;
    UBYTE num_step;
    UBYTE *pPermute_idx;
    UBYTE N;
    UBYTE first_quarter;
    UBYTE second_quarter;
    UBYTE third_quarter;
    UBYTE fourth_quarter;

    /* Adjust the seqence of stored input and send them to output buffer */

    pcW_vec = I->cqW_vec;
    pPermute_idx = I->pPermute_idx;

    for (i = 0; i < I->num_points; i++)
    {
        pcOut[pPermute_idx[i]].r = pcIn[i].r;
        pcOut[pPermute_idx[i]].i = pcIn[i].i;
    }


    N = I->num_points;
    first_quarter  = 0;
    second_quarter = N >> 2;
    third_quarter  = N >> 1;
    fourth_quarter = second_quarter + third_quarter;
    num_step = I->num_step;

    for (cntr = 1; cntr <= num_step; cntr ++)
    {
        wide_of_butf = (1 << cntr);
        dist_to_arg  = (1 << (cntr - 1));

        num_of_butf = N >> cntr;

        w_vec_idx = 0;

        for (j = 0; j < dist_to_arg; j++)
        {
            i = j;

            while (i < N)
            {
                arg = i + dist_to_arg;

                cqa = pcOut[arg];

                if (j == 0)
                {
                    cqP = cqa;
                }
                else
                {
                    if (w_vec_idx == first_quarter)
                    {
                        cqP.r = cqa.r;
                        cqP.i = cqa.i;
                    }
                    else if (w_vec_idx == second_quarter)
                    {
                        cqP.r =  cqa.i;
                        cqP.i = -cqa.r;
                    }
                    else if (w_vec_idx == third_quarter)
                    {
                        cqP.r = -cqa.r;
                        cqP.i = -cqa.i;
                    }
                    else if (w_vec_idx == fourth_quarter)
                    {
                        cqP.r = -cqa.i;
                        cqP.i =  cqa.r;
                    }
                    else
                    {
                        CQMUL(cqa, pcW_vec[w_vec_idx], cqP);
                    }
                }

                CQSUB(pcOut[i], cqP, pcOut[arg]);

                pcOut[i].r += cqP.r;
                pcOut[i].i += cqP.i;

                i += wide_of_butf;
            }

            w_vec_idx += num_of_butf;
        }
    }
}

void Ph2_FFT_Init(FFT_Data *fft, CQWORD *pcW_vec, UBYTE *pPermute_idx, UBYTE N, UBYTE FFT_type)
{
    UBYTE i, temp, temp1;

    fft->pPermute_idx = pPermute_idx;
    fft->cqW_vec      = pcW_vec;
    fft->num_step     = 0;

    while (N >> (fft->num_step)) { (fft->num_step) ++; }

    fft->num_step--;

    temp = 1 << fft->num_step;

    if ((N &(temp - 1)) != 0)        /* !! precedence of "!=" higher than "&" !! */
    {
        N = temp;
    }

    fft->num_points = N;

    /* calculate pPermute_idx and cqW_vec according to N */

    /* pcW_vec[i] = {cos(2*pi*i/N, sin(2*pi*i/N}; */
    /*  cos() and sin() can be obtained by looking up  two 256-point tables */


    temp = (256 >> fft->num_step) & 0xFF;   /* 2 * pi / N */

    for (i = 0; i < N; i++)
    {
        temp1 = UBUBMULUB(temp, i);
        pcW_vec[i].r = DSP_tCOS_TABLE[temp1];

#if 0

        if (FFT_type)
        {
            pcW_vec[i].i =  DSP_tCOS_TABLE[(temp1- 64) & COS_SAMPLE_NUM];
        }
        else
        {
            pcW_vec[i].i = -DSP_tCOS_TABLE[(temp1- 64) & COS_SAMPLE_NUM];
        }

#else
        pcW_vec[i].i =  DSP_tCOS_TABLE[(temp1- 64) & COS_SAMPLE_NUM];

        if (!FFT_type)
        {
            pcW_vec[i].i =  -pcW_vec[i].i ;
        }

#endif

        /* TRACE2("%f   %f", pcW_vec[i].r, pcW_vec[i].i); */
    }


    /* calculate pPermute_idx */   /* !! bit reversal !! */

    for (i = 0; i < N; i++)
    {
        pPermute_idx[i] = 0;
        temp1 = i;

        for (temp = 0; temp < fft->num_step; temp++)
        {
            pPermute_idx[i] <<= 1;
            pPermute_idx[i] |= (temp1 & 0x1);
            temp1 >>= 1;
        }
    }
}
