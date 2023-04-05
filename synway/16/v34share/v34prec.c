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

/****************************************************************************/
/* V34_Precoder using in V.34 and V90a encoder                              */
/*                                                                          */
/* Note: The precoder is designed based on Fix-Point math, and all symbols  */
/*       points are using 9.7 fomrat except the precoder coefficients,      */
/*       using 2.14 format.                                                 */
/*                                                                          */
/* INPUT: PrecodeStruct *pPreC - PrecodeStruct structure                    */
/*        CQWORD cX - input of the precoder, x(n), 9.7 format               */
/*                                                                          */
/* Last Updated:                                                            */
/*      Apr 12, 1996 - First revised version                                */
/*      Apr 15, 1996 - Take out Step 4 of precoder sequence                 */
/*      Apr 30, 1996 - Jack Liu Porting to VXD lib                          */
/* Author:                                                                  */
/*      Benjamin Chan, GAO Research & Consulting Ltd.                       */
/****************************************************************************/

#include "shareext.h"

#if (TX_PRECODE_ENABLE + RX_PRECODE_ENABLE)

void V34_Precoding_DataInit(PrecodeStruct *pPrec)
{
    UBYTE i;

    /* init delayline */
    for (i = 0; i < 12; i++)
    {
        pPrec->pDelay[i] = 0;
    }

    /* Init other data elements but no Coef init */
    pPrec->cC.r = 0;
    pPrec->cC.i = 0;

    pPrec->cP.r = 0;
    pPrec->cP.i = 0;

    pPrec->DelayIdx = 0;
}

void  V34_Precoding_CoefInit(PrecodeStruct *pPrec, SWORD *pSrc)
{
    pPrec->pCoef[0] = pSrc[4]; /* PrecodeCoeff_h[2].r */
    pPrec->pCoef[1] = pSrc[2]; /* PrecodeCoeff_h[1].r */
    pPrec->pCoef[2] = pSrc[0]; /* PrecodeCoeff_h[0].r */
    pPrec->pCoef[3] = pSrc[5]; /* PrecodeCoeff_h[2].i */
    pPrec->pCoef[4] = pSrc[3]; /* PrecodeCoeff_h[1].i */
    pPrec->pCoef[5] = pSrc[1]; /* PrecodeCoeff_h[0].i */
}

#if !USE_ASM

void V34_Precoder(PrecodeStruct *pPrec, CQWORD *pX, SBYTE b)
{
    QDWORD   qdMacr;
    QDWORD   qdMaci;
    UBYTE    i, j;
    UWORD    Umask;
    SWORD    w;
    QWORD    P2m_r, P2m_i, C2m_r, C2m_i;
    QDWORD   qdTmp;

    pPrec->pDelay[pPrec->DelayIdx  ] = pX->r;
    pPrec->pDelay[pPrec->DelayIdx+1] = pX->i;

    /* Double buffering */
    pPrec->pDelay[pPrec->DelayIdx+6] = pX->r;
    pPrec->pDelay[pPrec->DelayIdx+7] = pX->i;

    /* Point to oldest element in delayline */
    pPrec->DelayIdx += 2;

    if (pPrec->DelayIdx >= (V34_PRECODE_COEF_LEN << 1))
    {
        pPrec->DelayIdx = 0;
    }

    /* Filtering for Real and imaginary result */
    /* Coef order: C2r C1r C0r C2i C1i C0i */
    qdMacr = 0;
    qdMaci = 0;

    for (i = 0; i < V34_PRECODE_COEF_LEN; i++)
    {
        j = pPrec->DelayIdx + (i << 1);

        qdMacr += QQMULQD(pPrec->pCoef[i  ], pPrec->pDelay[j  ]); /* Cr * Dr */
        qdMacr -= QQMULQD(pPrec->pCoef[i+3], pPrec->pDelay[j+1]); /* Ci * Di */

        qdMaci += QQMULQD(pPrec->pCoef[i+3], pPrec->pDelay[j  ]); /* Ci * Dr */
        qdMaci += QQMULQD(pPrec->pCoef[i  ], pPrec->pDelay[j+1]); /* Cr * Di */
    }

    /* If b is 0, this is special case for 1st stage of Precoder decoder */
    /* No need to calcuate the c(n) element.                             */
    if (b == 0)
    {
        /* Rounding: Round to the nearest integer mutiples of 2^-7 */
        /*       If a component falls exactly half-way between     */
        /*       two integer multiples of 2^-7, round it to the    */
        /*       one with the smaller magnitude.                   */

        pPrec->cP.r = (QWORD)ROUND14(qdMacr);

        pPrec->cP.i = (QWORD)ROUND14(qdMaci);

        return;
    }

    /* Quantize: Quantize p(n) to the nearest integer multiple */
    /*       of 2w to obtain c(n). If the component falls      */
    /*       exactly half-way between two integer multiples    */
    /*       of 2w, quantize it to the one with smaller        */
    /*       magnitude.                                        */
    if (b >= 56)
    {
        w     = 0x0100;     /* w = 2, when b>=56, in 9.7 format */
        Umask = 0xFE00;     /* 16bit dsp data type */
    }
    else
    {
        w     = 0x0080;     /* w = 1, when b<56, in 9.7 format  */
        Umask = 0xFF00;
    }

    if (qdMacr > 0)
    {
        qdTmp = ROUND14(-qdMacr);

        P2m_r = (QWORD)qdTmp;

        C2m_r = (SWORD)((P2m_r + w) & Umask) >> 7;//CANNOT!!!: (SWORD)(((P2m_r + w) & Umask) >> 7);

        pPrec->cP.r = -P2m_r;

        pPrec->cC.r = -C2m_r;
    }
    else
    {
        qdTmp = ROUND14(qdMacr);

        pPrec->cP.r = (QWORD)qdTmp;

        pPrec->cC.r = (SWORD)((pPrec->cP.r + w) & Umask) >> 7;
    }

    if (qdMaci > 0)
    {
        qdTmp = ROUND14(-qdMaci);
        P2m_i = (QWORD)qdTmp;

        C2m_i = (SWORD)((P2m_i + w) & Umask) >> 7;

        pPrec->cP.i = -P2m_i;

        pPrec->cC.i = -C2m_i;
    }
    else
    {
        qdTmp = ROUND14(qdMaci);

        pPrec->cP.i = (QWORD)qdTmp;

        pPrec->cC.i = (SWORD)((pPrec->cP.i + w) & Umask) >> 7;
    }
}
#endif

#endif
