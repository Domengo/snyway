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

#if SUPPORT_V34FAX
void DSPD_DFTPure_Init(QDFTPUREStruct *pDFT, SWORD TotalSamples, SWORD DeltaPhase, UBYTE shift)
{
    pDFT->TotalSamples = TotalSamples;
    pDFT->DeltaPhase   = DeltaPhase;
    pDFT->CurPhase     = 0;
    pDFT->SmpCount     = 0;
    pDFT->TempI.r      = 0;
    pDFT->TempI.i      = 0;
    pDFT->TempQ.r      = 0;
    pDFT->TempQ.i      = 0;
    pDFT->shift        = shift;
}

UBYTE DSPD_DFTPure_Detect(QDFTPUREStruct *pDFT, UWORD InSize, CQWORD *qIn)
{
    SWORD  qCos;
    SWORD  qSin;
    QDWORD qdTemp;
    UWORD Phase;
    UWORD DeltaPhase;
    UBYTE shift;
    UWORD i;

    Phase = pDFT->CurPhase;
    DeltaPhase = pDFT->DeltaPhase;
    shift = pDFT->shift;

    for (i = 0; i < InSize; i++)
    {
        SinCos_Lookup_Fine(Phase, &qSin, &qCos);
        Phase_Update(&Phase, DeltaPhase);

        pDFT->TempI.r += (QQMULQD(qIn[i].r, qCos) >> shift);
        pDFT->TempI.i += (QQMULQD(qIn[i].r, qSin) >> shift);
        pDFT->TempQ.r += (QQMULQD(qIn[i].i, qCos) >> shift);
        pDFT->TempQ.i += (QQMULQD(qIn[i].i, qSin) >> shift);
    }

    pDFT->CurPhase = Phase;
    pDFT->SmpCount += InSize;

    if (pDFT->SmpCount == pDFT->TotalSamples)
    {
        pDFT->IResult.r = QDR15Q(pDFT->TempI.r);
        pDFT->IResult.i = QDR15Q(pDFT->TempI.i);
        pDFT->QResult.r = QDR15Q(pDFT->TempQ.r);
        pDFT->QResult.i = QDR15Q(pDFT->TempQ.i);

        qdTemp  = QQMULQD(pDFT->IResult.r, pDFT->IResult.r);
        qdTemp += QQMULQD(pDFT->IResult.i, pDFT->IResult.i);
        qdTemp += QQMULQD(pDFT->QResult.r, pDFT->QResult.r);
        qdTemp += QQMULQD(pDFT->QResult.i, pDFT->QResult.i);
        pDFT->MagResult = (QWORD)ROUND14(qdTemp);/* Multiplied by 2 and then changed to Q15 */

        pDFT->CurPhase = 0;
        pDFT->SmpCount = 0;

        pDFT->TempI.r  = 0;
        pDFT->TempI.i  = 0;
        pDFT->TempQ.r  = 0;
        pDFT->TempQ.i  = 0;

        return(1);
    }

    return(0);
}
#endif