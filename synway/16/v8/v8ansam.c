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

#include "v8ext.h"
#include "v251.h"

/***************************************************************************/
/* Function :                                                              */
/*              SWORD ANSam_gen(void)                                      */
/*              UBYTE V8_ANSam15_Detect(V8Struct *pV8)                     */
/* Description :                                                           */
/*              ANSam_gen will generate modified Answer Tone ANSam         */
/*              ANSam_detect will detect modified Answer Tone ANSam        */
/*                      from input samples                                 */
/* Input :                                                                 */
/*              An 16-bit signed integer represented amplitude (0.. 32767) */
/*              Don't use 32767, it may saturate output sample             */
/*              A pointer point to 96 samples buffer                       */
/* Output :                                                                */
/*              Consecutive time sample will be generated for each call    */
/*              1 if ANSam is detected                                     */
/*              0 if ANSam is not detected                                 */
/***************************************************************************/

UBYTE V8_ANSam15_Detect(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    CQDWORD   ANSam_DFT2085, ANSam_DFT2100, ANSam_DFT2115;
    QDWORD    qEgy, qEgy1, qEgy2, qEgy3, qEgy_sqrt;
    QWORD     qC, qS, qIn;
    UBYTE     i;

    qEgy            = 0;

    ANSam_DFT2085.r = 0;
    ANSam_DFT2085.i = 0;

    ANSam_DFT2100.r = 0;
    ANSam_DFT2100.i = 0;

    ANSam_DFT2115.r = 0;
    ANSam_DFT2115.i = 0;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
#if 0 /* for to dump Answer tone data. */

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV21->PCMinPtr[i]; }

#endif

        qIn = DspFir(&(pV8->V8AnsamToneH), pV21->PCMinPtr[i]);

        qEgy += (QQMULQD(qIn, qIn) >> V8_ANSAM_SHIFT);

        SinCos_Lookup_Fine(pV8->curPhase_2085, &qS, &qC);
        Phase_Update(&pV8->curPhase_2085, pV8->DeltaPhase_2085);

        ANSam_DFT2085.r += (QQMULQD(qIn, qC) >> V8_ANSAM_SHIFT);
        ANSam_DFT2085.i += (QQMULQD(qIn, qS) >> V8_ANSAM_SHIFT);

        SinCos_Lookup_Fine(pV8->curPhase_2100, &qS, &qC);
        Phase_Update(&pV8->curPhase_2100, pV8->DeltaPhase_2100);

        ANSam_DFT2100.r += (QQMULQD(qIn, qC) >> V8_ANS_SHIFT);
        ANSam_DFT2100.i += (QQMULQD(qIn, qS) >> V8_ANS_SHIFT);

        SinCos_Lookup_Fine(pV8->curPhase_2115, &qS, &qC);
        Phase_Update(&pV8->curPhase_2115, pV8->DeltaPhase_2115);

        ANSam_DFT2115.r += (QQMULQD(qIn, qC) >> V8_ANSAM_SHIFT);
        ANSam_DFT2115.i += (QQMULQD(qIn, qS) >> V8_ANSAM_SHIFT);
    }

    pV8->ANSam_Egy += qEgy >> 2;

    pV8->Ansam_dft2085.r += ANSam_DFT2085.r >> V8_ANSAM_SHIFT;
    pV8->Ansam_dft2085.i += ANSam_DFT2085.i >> V8_ANSAM_SHIFT;

    pV8->Ansam_dft2100.r += ANSam_DFT2100.r >> V8_ANS_SHIFT;
    pV8->Ansam_dft2100.i += ANSam_DFT2100.i >> V8_ANS_SHIFT;

    pV8->Ansam_dft2115.r += ANSam_DFT2115.r >> V8_ANSAM_SHIFT;
    pV8->Ansam_dft2115.i += ANSam_DFT2115.i >> V8_ANSAM_SHIFT;

    pV8->ANS15_Counter++;

    if (pV8->ANS15_Counter == 20)
    {
        qEgy = pV8->ANSam_Egy;

        pV8->ANSam_Egy = 0;

        pV8->ANS15_Counter = 0;

        qC = (QWORD)(pV8->Ansam_dft2085.r >> 13);
        qS = (QWORD)(pV8->Ansam_dft2085.i >> 13);

        pV8->Ansam_dft2085.r = 0;
        pV8->Ansam_dft2085.i = 0;

        pV8->curPhase_2085 = 0;

        qEgy1  = QQMULQD(qC, qC);
        qEgy1 += QQMULQD(qS, qS);

        qC = (QWORD)(pV8->Ansam_dft2100.r >> 13);
        qS = (QWORD)(pV8->Ansam_dft2100.i >> 13);

        pV8->Ansam_dft2100.r = 0;
        pV8->Ansam_dft2100.i = 0;

        pV8->curPhase_2100 = 0;

        qEgy2  = QQMULQD(qC, qC);
        qEgy2 += QQMULQD(qS, qS);

        qC = (QWORD)(pV8->Ansam_dft2115.r >> 13);
        qS = (QWORD)(pV8->Ansam_dft2115.i >> 13);

        pV8->Ansam_dft2115.r = 0;
        pV8->Ansam_dft2115.i = 0;

        pV8->curPhase_2115 = 0;

        qEgy3  = QQMULQD(qC, qC);
        qEgy3 += QQMULQD(qS, qS);

        qEgy1 = (qEgy1 >> 1) + (qEgy3 >> 1) ;

        /* For TIA3700 channel(B4a2, B4b2, B4c2, B4d2/loop6) to pass, the reference of qEgy2 change from 10000 to 5000 */
        if ((qEgy2 > 10000) && (qEgy1 > (qEgy2 >> 8)) && (qEgy1 < (qEgy2 >> 1)) && (qEgy2 > (qEgy >> 2)))
        {
            TRACE2("V8: [%d]: Egy=%" FORMAT_DWORD_PREFIX "d", pV8->ANSam_Tone_Count, qEgy);
            pV8->ANSam_Tone_Count++;
        }
        else
        {
            pV8->ANSam_Tone_Count = 0;
        }

        if (pV8->ANSam_Tone_Count > 3)
        {
            qEgy_sqrt = QDsqrt(qEgy);   /* qEgy_sqrt = sqrt(qEgy2); */

            pV21->qAgc_gain = (QWORD)QDQDdivQD(pV8->qdAnsam_AGC_Ref, qEgy_sqrt);

            TRACE2("V8: qEgy_sqrt = %" FORMAT_DWORD_PREFIX "d, pV21->qAgc_gain = %d", qEgy_sqrt, pV21->qAgc_gain);

            return 1;
        }
        else if (pV8->ANSam_Tone_Count == 1)
        {
            UBYTE ansSig = 1;
            V8_SetResponse(pV21->pTable, V8_A8A, &ansSig, 1);
        }
    }

    return 0;
}
