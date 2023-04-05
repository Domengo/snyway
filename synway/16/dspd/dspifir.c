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

#include "dspdext.h"

/* Pulse Shape Filter Coefficients */
/* Following coefficients are for 7200 sampling rate */
/* Used by V32 and V29 */
CONST QWORD Dspi3Fir_Psf_coef[DSP_PSF_COEF_LEN] =
{
    /* reorder the coefficients for sharing with DSP */
    -59  ,  377   ,    -1008,    2178,  -4591,
    13201,  26923 ,    -6097,    2775,  -1320,
    545 ,  -139  ,     -41 ,    32767
};

void DspiFirInit(QIFIRStruct *pIFir, QWORD *pCoef, QWORD *pDline, UBYTE DlineLen, UBYTE ifactor)
{
    UBYTE i;

    /* Double buffering delayline */
    /* Setup the coefficient pointer */
    pIFir->pCoef   = pCoef;
    /* Start of the delayline, a constant during program execution */
    pIFir->pDline  = pDline;
    /* Set the coefficient blocksize */
    pIFir->blksize = DlineLen;
    /* Set interpolating factor */
    pIFir->ifactor = ifactor;
    pIFir->nOffset = 0;

    for (i = 0; i < 2 * DlineLen; i++)
    {
        pIFir->pDline[i] = 0;
    }
}

void Dspi3Fir_PsfInit(QIFIRStruct *pIFir, QWORD *pCoef, QWORD *pDline, UBYTE DlineLen, UBYTE ifactor)
{
    UBYTE i;

    /* Double buffering delayline */
    /* Setup the coefficient pointer */
    pIFir->pCoef   = pCoef;
    /* Start of the delayline, a constant during program execution */
    pIFir->pDline  = pDline;
    /* Set the coefficient blocksize */
    pIFir->blksize = DlineLen;
    /* Set interpolating factor */
    pIFir->ifactor = ifactor;
    pIFir->nOffset = 0;

#if USE_ASM

    for (i = 0; i < DlineLen; i++)
#else
    for (i = 0; i < (DlineLen << 1); i++)
#endif
        pIFir->pDline[i] = 0;
}

#if !USE_ASM
void DspiFir(QIFIRStruct *pIFir, QWORD qInSample, QWORD *pqOutSample)
{
    QWORD  *pHead, *pHeadTemp;
    QWORD  *pTail;
    QWORD  *pCoef;
    QDWORD msum;
    UBYTE  DlineLen;
    UBYTE  i, j, Interpolate_length;

    DlineLen = pIFir->blksize;
    pHead = pIFir->pDline + pIFir->nOffset;
    pTail = pHead + DlineLen;

    Interpolate_length = pIFir->ifactor;

    /* Save the input sample to the delayline */
    *pHead++ = qInSample;
    *pTail++ = qInSample;

    pIFir->nOffset ++;

    /* Reset the pointer to the beginning if necessary */
    if (pIFir->nOffset >= DlineLen)
    {
        pIFir->nOffset = 0;
        pHead = pIFir->pDline;
    }

    /* Create a pointer to the coefficient */
    pCoef = (QWORD *)pIFir->pCoef;

    for (i = 0; i < Interpolate_length; i++)
    {
        /* This is the starting point of the convolution */
        pHeadTemp = pHead;
        msum = 0;

        for (j = 0; j < DlineLen; j++)
        {
            msum += QQMULQD((*pCoef++), (*pHeadTemp++));
        }

        /* Convert format */
        *pqOutSample++ = QDR15Q(msum);
    }
}

void Dspi3Fir_Psf(QIFIRStruct *pIFir, QWORD insample, QWORD *pOutsample)
{
    QWORD *pHead;
    QWORD *pTail;
    QWORD *pCoef1;
    QWORD *pCoef2;
    QWORD *pCoef3;
    UBYTE DlineLen;
    QDWORD msum1, msum2, msum3;
    UBYTE i;

    /* Create pointers to the coefficient sets */
    DlineLen = pIFir->blksize;
    pHead = pIFir->pDline + pIFir->nOffset;
    pTail = pHead + DlineLen;

    /* Save the input sample to the delayline */
    *pHead++ = insample;
    *pTail++ = insample;

    pIFir->nOffset ++;

    /* Reset the pointer to the beginning if necessary */
    if (pIFir->nOffset >= DlineLen)
    {
        pIFir->nOffset = 0;
        pHead = pIFir->pDline;
    }

    pCoef1 = (QWORD *) pIFir->pCoef;
    pCoef2 = (QWORD *) pCoef1 + DlineLen; /* this is scale factor (usually 1/3) */
    pCoef3 = (QWORD *) pCoef2 - 1;

    /* only one non zero coefficient for second sample */
    msum2 = QQMULQD((*(pHead + (DlineLen >> 1))), (*pCoef2));

    msum1 = 0;
    msum3 = 0;

    for (i = 0; i < DlineLen; i++)
    {
        msum1 += QQMULQD((*pCoef1++), (*pHead));
        msum3 += QQMULQD((*pCoef3--), (*pHead++));
    }

    /* Convert format */
    *pOutsample++ = QDR15Q(msum1);
    *pOutsample++ = QDR15Q(msum2);
    *pOutsample   = QDR15Q(msum3);
}

#endif
