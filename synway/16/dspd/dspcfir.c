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

void DspcFirInit(QCFIRStruct *pcFir, CQWORD *pcCoef, CQWORD *pcDline, UWORD len)
{
    UWORD i, len2;

    pcFir->pcCoef  = pcCoef;
    /* Double buffering delayline */
    pcFir->pcDline = pcDline;
    pcFir->nTapLen = len;
    pcFir->nOffset = 0;
    len2 = len << 1;

    for (i = 0; i < len; i++)
    {
        pcFir->pcCoef[i].r = 0;
        pcFir->pcCoef[i].i = 0;
    }

    for (i = 0; i < len2; i++)
    {
        pcFir->pcDline[i].r = 0;
        pcFir->pcDline[i].i = 0;
    }
}

#if !USE_ASM
CQWORD DspcFir(QCFIRStruct *pcFir, CQWORD *pIn)
{
    CQWORD *pcHead;
    CQWORD *pcTail;
    CQWORD *pcCoef;
    QDWORD msum_r;
    QDWORD msum_i;
    CQWORD out;
    UWORD  i, Len;

    Len = pcFir->nTapLen;

    pcHead = pcFir->pcDline + pcFir->nOffset;
    pcTail = pcHead + Len;
    pcCoef = pcFir->pcCoef;

    /* Save the input sample to the delayline */
    (*pcHead).r = (*pIn).r;
    (*pcHead).i = (*pIn).i;

    (*pcTail).r = (*pIn).r;
    (*pcTail).i = (*pIn).i;

    pcHead++;
    pcTail++;

    pcFir->nOffset ++;

    if (pcFir->nOffset >= Len)
    {
        pcFir->nOffset = 0;
        pcHead = pcFir->pcDline;
    }

    msum_r = 0;
    msum_i = 0;

    for (i = 0; i < Len; i++) /* -- convolution loop -- */
    {
        msum_r  += QQMULQD(pcHead->r, pcCoef->r);
        msum_r  -= QQMULQD(pcHead->i, pcCoef->i);

        msum_i  += QQMULQD(pcHead->r, pcCoef->i);
        msum_i  += QQMULQD(pcHead->i, pcCoef->r);
        pcHead++;
        pcCoef++;
    }

    out.r = QDR15Q(msum_r);
    out.i = QDR15Q(msum_i);
    return (out);
}
#endif

void DspcFir_2T3EQInit(QCFIRStruct *pcFir, CQWORD *pcCoef, CQWORD *pcDline, UWORD len)
{
    UWORD i, len4;

    pcFir->pcCoef  = pcCoef;
    /* Double buffering delayline */
    pcFir->pcDline = pcDline;
    pcFir->nTapLen = len;
    pcFir->nOffset = 0;
    len4 = len << 2;

    for (i = 0; i < len; i++)
    {
        pcFir->pcCoef[i].r = 0;
        pcFir->pcCoef[i].i = 0;
    }

    for (i = 0; i < len4; i++)
    {
        pcFir->pcDline[i].r = 0;
        pcFir->pcDline[i].i = 0;
    }
}

#if !USE_ASM
CQWORD DspcFir_2T3EQ(QCFIRStruct *pcFir, CQWORD *pIn)
{
    CQWORD *pcHead;
    CQWORD *pcTail;
    CQWORD *pcCoef;
    QDWORD msum_r;
    QDWORD msum_i;
    CQWORD out;
    UWORD  i, Len, Len2;

    Len  = pcFir->nTapLen;
    Len2 = Len << 1;

    pcHead = pcFir->pcDline + pcFir->nOffset;
    pcTail = pcHead + Len2;
    pcCoef = pcFir->pcCoef;

    /* Save the 3 input samples to the delayline */
    for (i = 0; i < 3; i++)
    {
        (*pcHead).r = (*(pIn + i)).r;
        (*pcHead).i = (*(pIn + i)).i;

        (*pcTail).r = (*(pIn + i)).r;
        (*pcTail).i = (*(pIn + i)).i;

        pcHead++;
        pcTail++;
    }

    pcFir->nOffset += 3;

    if (pcFir->nOffset >= Len2)
    {
        pcFir->nOffset = 0;
        pcHead = pcFir->pcDline;
    }

    msum_r = 0;
    msum_i = 0;

    for (i = 0; i < Len; i++)  /* -- convolution loop -- */
    {
        msum_r += QQMULQD(pcHead->r, pcCoef->r);
        msum_r -= QQMULQD(pcHead->i, pcCoef->i);
        msum_i += QQMULQD(pcHead->r, pcCoef->i);
        msum_i += QQMULQD(pcHead->i, pcCoef->r);
        pcHead += 2;
        pcCoef++;
    }

    out.r = QDR15Q(msum_r);
    out.i = QDR15Q(msum_i);

    return (out);
}
#endif
