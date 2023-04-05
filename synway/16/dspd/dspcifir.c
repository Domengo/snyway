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

#if 0
/* Interpolating by 3 cFir filtering used for 16/32 Bit echo canceller */
void DspI3cFir_Adapt_EC_16_32(CQDWORD *pcCoef, CQWORD *pcCoef16, CQDWORD *pcCoefEnd, CQWORD *pcDline, CQWORD *pcAdapt, QWORD *qEchoCancel_Out, QWORD *out)
{
    QDWORD qdOut[EC_INTER_NUM];
    UBYTE  i;

    for (i = 0; i < EC_INTER_NUM; i++)
    {
        qdOut[i] = 0;
    }

    while (pcCoef <= pcCoefEnd)
    {
        for (i = 0; i < EC_INTER_NUM; i++)
        {
            qdOut[i]   += QQMULQD(pcDline->r, QD16Q(pcCoef->r));
            qdOut[i]   += QQMULQD(pcDline->i, QD16Q(pcCoef->i));

            pcCoef->r  += QQMULQD(qEchoCancel_Out[i] << 1, pcAdapt->r);
            pcCoef->i  += QQMULQD(qEchoCancel_Out[i] << 1, pcAdapt->i);

            pcCoef16->r = QD16Q(pcCoef->r);
            pcCoef16->i = QD16Q(pcCoef->i);

            ++pcCoef16;
            ++pcCoef;
        }

        pcAdapt--;
        --pcDline;
    }

    for (i = 0; i < EC_INTER_NUM; i++)
    {
        out[i] = QDR15Q(qdOut[i]);
    }
}
#endif

/* Interpolating by 3 cFir filtering used for 16/32 Bit echo canceller */
void DspI3cFir_EC_16_32(CQDWORD *pcCoef, UWORD pcCoef_Len, CQWORD *pcDline, QWORD *out)
{
    QDWORD qdOut[EC_INTER_NUM];
    UWORD  i, j;

    for (j = 0; j < EC_INTER_NUM; j++)
    {
        qdOut[j] = 0;
    }

    for (i = 0; i < pcCoef_Len; i++)
    {
        for (j = 0; j < EC_INTER_NUM; j++)
        {
            qdOut[j] += QQMULQD(pcDline->r, QD16Q(pcCoef->r));
            qdOut[j] += QQMULQD(pcDline->i, QD16Q(pcCoef->i));
            ++pcCoef;
        }

        pcDline--;
    }

    for (j = 0; j < EC_INTER_NUM; j++)
    {
        out[j] = QDR15Q(qdOut[j]);
    }
}

void DspI3cFir_EC_16_32_Update(CQDWORD *pcCoef, UWORD pcCoef_Len, CQWORD *pcAdapt, QWORD *qEchoCancel_Out)
{
    UWORD i, j;

    for (i = 0; i < pcCoef_Len; i++)
    {
        for (j = 0; j < EC_INTER_NUM; j++)
        {
            pcCoef->r += QQMULQD(qEchoCancel_Out[j] << 1, pcAdapt->r);
            pcCoef->i += QQMULQD(qEchoCancel_Out[j] << 1, pcAdapt->i);
            ++pcCoef;
        }

        pcAdapt--;
    }
}
#endif
