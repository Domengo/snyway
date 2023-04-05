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

#if !USE_ASM
void DspcFirLMS(QCFIRStruct *pcFir, QWORD Beta, CQWORD *pcErr)
{
    CQWORD *pcCoef;
    CQWORD *pcHead;
    QWORD  error_r, error_i;
    QDWORD dtemp_r, dtemp_i;
    UWORD  i, Len;

    /* Calculate real error based on beta and input real error */
    error_r = QQMULQR15(pcErr->r, Beta);

    /* Calculate imag error based on beta and input imag error */
    error_i = QQMULQR15(pcErr->i, Beta);

    Len = pcFir->nTapLen;

    /* This is the starting point of the convolution */
    pcHead = pcFir->pcDline + pcFir->nOffset;

    /* Create a pointer to the coefficient */
    pcCoef = pcFir->pcCoef;

    for (i = 0; i < Len; i++)
    {
        dtemp_r    = QQMULQD(error_r, pcHead->r);
        dtemp_r   += QQMULQD(error_i, pcHead->i);
        pcCoef->r += QDR15Q(dtemp_r);

        dtemp_i    = QQMULQD(error_i, pcHead->r);
        dtemp_i   -= QQMULQD(error_r, pcHead->i);
        pcCoef->i += QDR15Q(dtemp_i);

        pcHead++;
        pcCoef++;
    }
}

void DspcFirLMS_2T3(QCFIRStruct *pcFir, QWORD Beta, CQWORD *pcErr)
{
    CQWORD *pcCoef;
    CQWORD *pcHead;
    QWORD error_r, error_i;
    QDWORD dtemp_r, dtemp_i;
    UWORD i, Len;

    /* Calculate real error based on beta and input real error */
    error_r = QQMULQR15(pcErr->r, Beta);

    /* Calculate imag error based on beta and input imag error */
    error_i = QQMULQR15(pcErr->i, Beta);

    Len = pcFir->nTapLen;

    /* This is the starting point of the convolution */
    pcHead = pcFir->pcDline + pcFir->nOffset;

    /* Create a pointer to the coefficient */
    pcCoef = pcFir->pcCoef;

    for (i = 0; i < Len; i++)
    {
        dtemp_r    = QQMULQD(error_r, pcHead->r);
        dtemp_r   += QQMULQD(error_i, pcHead->i);
        pcCoef->r += QDR15Q(dtemp_r);

        dtemp_i    = QQMULQD(error_i, pcHead->r);
        dtemp_i   -= QQMULQD(error_r, pcHead->i);
        pcCoef->i += QDR15Q(dtemp_i);

        pcHead += 2;
        pcCoef++;
    }
}
#endif
