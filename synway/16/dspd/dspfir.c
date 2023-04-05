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

/* Hilbert Coef For V.34/V29/V27 */
CONST QWORD tHilbert_Coef[(HILB_TAP_LEN + 1) >>1 ] =
{
    13,    38,    89,   179,   327,  557,  904, 1423, 2227, 3601, 6597, 20738,
    -20738, -6597, -3601, -2227, -1423, -904, -557, -327, -179,  -89,  -38,   -13
};

void DspFirInit(QFIRStruct *pFir, QWORD *pCoef, QWORD *pDline, UBYTE len)
{
    UBYTE i;

    pFir->nTapLen = len;
    pFir->pDline  = pDline;  /* Double buffering delayline */
    pFir->pCoef   = pCoef;   /* Setup the coefficient pointer */
    pFir->nOffset = 0;

    for (i = 0; i < (len << 1); i++)
    {
        pFir->pDline[i] = 0;
    }
}

#if !USE_ASM
QWORD DspFir(QFIRStruct *pFir, QWORD insample)
{
    QWORD  *pHead;
    QWORD  *pTail;
    QWORD  *pCoef;
    QDWORD msum;
    QWORD  outsample;
    UWORD  i, Len;

    Len = pFir->nTapLen;

    pHead = pFir->pDline + pFir->nOffset; /* starting point of convolution */
    pTail = pHead + Len;                  /* ending   point of convolution */

    pCoef = pFir->pCoef;                  /* Create a pointer to the coefficient */

    *pHead++ = insample;
    *pTail = insample;

    pFir->nOffset ++;

    if (pFir->nOffset >= Len)
    {
        pFir->nOffset = 0;
        pHead = pFir->pDline;
    }

    msum = 0;

    for (i = 0; i < Len; i++)
    {
        msum += QQMULQD(*pHead++, *pCoef++);
    }

    outsample = QDR15Q(msum);

    return(outsample);
}

#if 0
QWORD DspFir_Odd(QFIRStruct *pFir, QWORD insample)
{
    QWORD *pHead;
    QWORD *pTail;
    QWORD  *pCoef;
    QDWORD msum;
    QWORD outsample;
    UBYTE i, Len, Len2;

    Len      = pFir->nTapLen; /* nTapLen must be odd. */
    Len2     = Len >> 1;

    pHead = pFir->pDline + pFir->nOffset; /* starting point of convolution */
    pTail = pHead + Len;                  /* ending   point of convolution */

    pCoef = pFir->pCoef;                  /* Create a pointer to the coefficient */

    *pHead = insample;
    *pTail = insample;

    pFir->nOffset ++;

    if (pFir->nOffset >= Len)
    {
        pFir->nOffset = 0;
        pHead = pFir->pDline;
        /* This is the ending point of the convolution */
        pTail = pHead + Len;
    }
    else
    {
        pHead++;
        pTail++;
    }

    pTail--;

    msum = 0;

    for (i = 0; i < Len2; i++)
    {
        msum += QQMULQD((*pHead++) + (*pTail--)), *pCoef++);
    }

    msum += QQMULQD(*pCoef++, *pHead++);

    outsample = QDR15Q(msum);

    return(outsample);
}
#endif


/* Optimized Hilbert with 0 Coefficients removed */
QWORD DspFir_Hilbert(QFIRStruct *pFir, QWORD insample)
{
    QWORD  *pHead;
    QWORD  *pTail;
    QWORD  *pCoef;
    QDWORD msum;
    QWORD  outsample;
    UWORD  i, Len;

    Len = pFir->nTapLen;

    pHead = pFir->pDline + pFir->nOffset; /* starting point of convolution */
    pTail = pHead + Len;                  /* ending   point of convolution */

    pCoef = pFir->pCoef;                  /* Create a pointer to the coefficient */

    *pHead++ = insample;
    *pTail   = insample;

    pFir->nOffset ++;

    if (pFir->nOffset >= Len)
    {
        pFir->nOffset = 0;
        pHead = pFir->pDline;
    }

    msum = 0;
    Len = (Len + 1) >> 1;

    for (i = 0; i < Len; i++)
    {
        msum += QQMULQD(*pHead++, *pCoef++);

        pHead++;
    }

    outsample = QDR15Q(msum);

    return(outsample);
}
#endif


QWORD DspDeciFir(QFIRStruct *pFir, QWORD *pinsample)
{
    QWORD  *pHead;
    QWORD  *pTail;
    QWORD  *pCoef;
    QDWORD msum;
    QWORD  outsample;
    UWORD  i, Len;

    Len = pFir->nTapLen;

    pHead = pFir->pDline + pFir->nOffset; /* starting point of convolution */
    pTail = pHead + Len;                  /* ending   point of convolution */

    pCoef = pFir->pCoef;                  /* Create a pointer to the coefficient */

    for (i = 0; i < 2; i++) /* Decimate 2/1 */
    {
        *pHead++ = *pinsample;
        *pTail++ = *pinsample++;

        pFir->nOffset ++;

        if (pFir->nOffset >= Len)
        {
            pFir->nOffset = 0;
            pHead = pFir->pDline;
            pTail = pHead + Len;
        }
    }

    msum = 0;

    for (i = 0; i < Len; i++)
    {
        msum += QQMULQD(*pHead++, *pCoef++);
    }

    outsample = QDR15Q(msum);

    return(outsample);
}
