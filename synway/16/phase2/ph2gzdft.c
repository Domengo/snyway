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

void V34_DFT_Init(DFT_Info *pDFT, QWORD *pqQ1_buf, QWORD *pqQ2_buf, QWORD *pqCoef, QWORD *pqMag_sqr, SWORD N, UBYTE Num_Freq)
{
    UBYTE i;

    pDFT->pqQ1 = pqQ1_buf;
    pDFT->pqQ2 = pqQ2_buf;
    pDFT->pqCoef = pqCoef;
    pDFT->pqMag_sqr = pqMag_sqr;
    pDFT->N = N;                    /* 128 */
    pDFT->cur_N = N;
    pDFT->Num_Freq = Num_Freq;      /* 3 */
    pDFT->qEgy = 0;

    for (i = 0; i < Num_Freq; i++)
    {
        pDFT->pqQ1[i] = 0;
        pDFT->pqQ2[i] = 0;
    }
}

UBYTE V34_DFT(DFT_Info *pDFT, SWORD sample)
{
    QWORD  *pqQ1;
    QWORD  *pqQ2;
    QWORD  *pqCoef;
    QWORD  *pqMnsqr;
    QWORD  qX;
    QDWORD qAns;
    QDWORD qItemp;
    UBYTE  i;

    qX = sample >> 8; /* change to 9.7 format */

    pDFT->qEgy += QQMULQDR7(qX, qX);

    pqQ1   = pDFT->pqQ1;
    pqQ2   = pDFT->pqQ2;
    pqCoef = pDFT->pqCoef;

    /* Feedback Phase, n = 0,1,2,...,N-1 */
    for (i = 0; i < pDFT->Num_Freq; i++)
    {
        qItemp = QQMULQDR7(*pqQ1, *pqCoef++);

        qAns = qItemp - (*pqQ2);

        qAns  += qX;

        /* qAns = (*pqQ1) * (*pqCoef) - *pqQ2 + qX; */
        *pqQ2++ = *pqQ1;
        *pqQ1++ = (QWORD)qAns;
    }

    --pDFT->cur_N;

    if (pDFT->cur_N <= 0)
    {
        /* Feed forward Phase, n = N */
        pqQ1    = pDFT->pqQ1;
        pqQ2    = pDFT->pqQ2;
        pqCoef  = pDFT->pqCoef;
        pqMnsqr = pDFT->pqMag_sqr;

        for (i = 0; i < pDFT->Num_Freq; i++)
        {
            qItemp = QQMULQDR7(*pqQ1, *pqQ2);
            qAns   = QDQMULQDR7(qItemp, *pqCoef);

            qItemp = QQMULQDR7(*pqQ1, *pqQ1);
            qAns   = qItemp - qAns;
            qItemp = QQMULQDR7(*pqQ2, *pqQ2);
            qAns  += qItemp;

            if (qAns < 0)
            {
                qAns = -qAns;
            }

            if (qAns > 32767)
            {
                qAns = 32767;
            }

            *pqMnsqr++ = (QWORD)qAns;

            pqQ1++;
            pqQ2++;
            pqCoef++;
        }

        /* Clear up all buffer */
        pqQ1 = pDFT->pqQ1;
        pqQ2 = pDFT->pqQ2;

        while (pqQ1 < &(pDFT->pqQ1[pDFT->Num_Freq]))
        {
            *pqQ1++ = 0;
            *pqQ2++ = 0;
        }

        pDFT->cur_N = pDFT->N;

        pDFT->qEgy  = 0;

        return 1;
    }

    return(0);
}
