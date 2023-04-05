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

void DSPD_DFT_Init(DftStruct *pDft, SWORD TotalSamples, UBYTE DetTones, SWORD *pCoef, QDWORD *qdMagSq, SWORD *qQ1Q2, UBYTE BitShift)
{
    UBYTE i;

    pDft->TotalSamples       = TotalSamples;
    pDft->DetTones           = DetTones;
    pDft->pDFT_tToneGcoefs   = pCoef;
    pDft->qdMagSq            = qdMagSq;
    pDft->Current_Sample_Num = 0;
    pDft->qQ1Q2              = qQ1Q2;
    pDft->Avg_Energy         = 0;
    pDft->BitShift           = BitShift;

    for (i = 0; i < (DetTones << 1); i++)
    {
        pDft->qQ1Q2[i] = 0;
    }

    for (i = 0; i < DetTones; i++)
    {
        pDft->qdMagSq[i] = 0;
    }
}

#if !USE_ASM
QDWORD DSPD_DFT_Detect(DftStruct *pDft, SWORD *pInBuf, SWORD InSize)
{
    QDWORD  qdAvg_Energy = -1;
    UBYTE   i, j;
    SWORD   F, F1;
    SDWORD  Ans, Ans1, Ans2, Ans3, Ans4;
    SWORD   SmpCnt;
    UBYTE   BitShift = pDft->BitShift;

    /* detect frequencies */
    for (SmpCnt = 0; SmpCnt < InSize; SmpCnt++)
    {
        F  = pInBuf[SmpCnt];
        F1 = F >> BitShift;

        pDft->Avg_Energy += QQMULQD(F, F) >> 8;

        /* Basic Goertzel calculation loop for 32 samples */
        for (i = 0, j = 0; i < (pDft->DetTones << 1); i += 2, ++j)
        {
            Ans = (QQMULQD(pDft->qQ1Q2[i], pDft->pDFT_tToneGcoefs[j])) >> 14;
            Ans -= pDft->qQ1Q2[i+1] - F1;

            pDft->qQ1Q2[i+1] = pDft->qQ1Q2[i];

            if (Ans > (SDWORD)32767)
            {
                pDft->qQ1Q2[i] = 32767;
                //TRACE0("DSPD: WARNING. Overflow");
            }
            else if (Ans < (SDWORD) - 32768)
            {
                pDft->qQ1Q2[i] = -32768;
                //TRACE0("DSPD: WARNING. Underflow");
            }
            else
            {
                pDft->qQ1Q2[i] = Ans;
            }
        }
    }

    pDft->Current_Sample_Num += InSize;

    if (pDft->Current_Sample_Num >= pDft->TotalSamples)
    {
        /* The last sample final calculation, find out magnitude square */

        for (i = 0, j = 0; i < (pDft->DetTones << 1); i += 2, ++j)
        {
            Ans1 = (QQMULQD(pDft->qQ1Q2[i  ], pDft->qQ1Q2[i])) >> 2;
            Ans2 = (QQMULQD(pDft->qQ1Q2[i+1], pDft->qQ1Q2[i+1])) >> 2;
            Ans3 = (QQMULQD(pDft->qQ1Q2[i  ], pDft->qQ1Q2[i+1])) >> 2;

            /* Ans4 = (Ans3 * pDft->pDFT_tToneGcoefs[j])>>14; */
            Ans4  = QDQMULQD((Ans3 << 1), pDft->pDFT_tToneGcoefs[j]);

            pDft->qdMagSq[j] =  Ans1 + Ans2 - Ans4;
        }

        pDft->Current_Sample_Num = 0;

        for (i = 0; i < (pDft->DetTones << 1); i++)
        {
            pDft->qQ1Q2[i] = 0;
        }

        qdAvg_Energy = pDft->Avg_Energy;

        pDft->Avg_Energy = 0;
    }

    return (qdAvg_Energy);
}
#endif

#if 0
void  DSPD_DetectToneInit(DspdDetectToneStruct *pDetectTone, SWORD TotalSamples, UBYTE DetTones,
                          UWORD *puDetectTonePhase, QWORD *pqDetectToneDeltaPhase,
                          QDWORD *pqdDetectToneResult_Real, QDWORD *pqdDetectToneResult_Imag,
                          QDWORD *pqdMag, UBYTE Shift)
{
    UBYTE i;

    pDetectTone->TotalSamples = TotalSamples;
    pDetectTone->DetTones = DetTones;
    pDetectTone->puDetectTonePhase = puDetectTonePhase;
    pDetectTone->pqDetectToneDeltaPhase = pqDetectToneDeltaPhase;
    pDetectTone->pqdDetectToneResult_Real =  pqdDetectToneResult_Real;
    pDetectTone->pqdDetectToneResult_Imag =  pqdDetectToneResult_Imag;
    pDetectTone->pqdMag = pqdMag;
    pDetectTone->Shift = Shift;
    pDetectTone->Current_Sample_Num = 0;

    for (i = 0; i < DetTones; i++)
    {
        pDetectTone->pqdDetectToneResult_Real[i] = 0;
        pDetectTone->pqdDetectToneResult_Imag[i] = 0;
        pDetectTone->puDetectTonePhase[i] = 0;
    }

    pDetectTone->Avg_Energy = 0;
}

#if !USE_ASM
QDWORD DSPD_DetectTone(DspdDetectToneStruct *pDetectTone, QWORD *pInBuf, UWORD InSize)
{
    QWORD qCos;
    QWORD qSin;
    QWORD qIn, qReal, qImag;
    UWORD i;
    UBYTE j;
    QDWORD qdAvg_Energy;
    UBYTE  Shift = pDetectTone->Shift;

    qdAvg_Energy = 0;

    for (i = 0; i < InSize; i++)
    {
        qIn = pInBuf[i] >> 4;
        qdAvg_Energy += QQMULQD(qIn, qIn);

        for (j = 0; j < pDetectTone->DetTones; j++)
        {
            SinCos_Lookup_Fine(pDetectTone->puDetectTonePhase[j], &qSin, &qCos);
            Phase_Update(&(pDetectTone->puDetectTonePhase[j]), (UWORD)pDetectTone->pqDetectToneDeltaPhase[j]);

            pDetectTone->pqdDetectToneResult_Real[j] += QQMULQD(qIn, qCos) >> Shift;
            pDetectTone->pqdDetectToneResult_Imag[j] += QQMULQD(qIn, qSin) >> Shift;
        }
    }

    pDetectTone->Avg_Energy += qdAvg_Energy >> Shift;
    pDetectTone->Current_Sample_Num += InSize;

    if (pDetectTone->Current_Sample_Num >= pDetectTone->TotalSamples)
    {
        pDetectTone->Current_Sample_Num = 0;

        for (j = 0; j < pDetectTone->DetTones; j++)
        {
            qReal = QD15Q(pDetectTone->pqdDetectToneResult_Real[j]);
            qImag = QD15Q(pDetectTone->pqdDetectToneResult_Imag[j]);
            pDetectTone->pqdMag[j] = QQMULQD(qReal, qReal) + QQMULQD(qImag, qImag);

            pDetectTone->pqdDetectToneResult_Real[j] = 0;
            pDetectTone->pqdDetectToneResult_Imag[j] = 0;
            pDetectTone->puDetectTonePhase[j] = 0;
        }

        qdAvg_Energy = pDetectTone->Avg_Energy >> Shift;

        pDetectTone->Avg_Energy = 0;

        return (qdAvg_Energy);
    }
    else
    {
        return (-1);
    }
}
#endif
#endif