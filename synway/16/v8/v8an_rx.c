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
#include "common.h"
#include "v251.h"

#if USE_ANS

void V8_Detect_CM(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE isCmDetected = FALSE;
    UBYTE **pTable = pV21->pTable;
    UBYTE i;

    if (!pV21->Detect_tone)
    {
        V21_Ch1Tone_Detect(pV21);
    }

    if (pV21->Detect_tone)
    {
        i = V8_Detect_Sub(pV8, V8_CM_TYPE);

        if ((pV8->result_code == 0) && (pV8->pRecv_string1[0] == 0x07) && (i >= 4))
        {
            TRACE1("V8: CM V34 (162) V90 (166) avai %x", pV8->pRecv_string1[2]);

            V8_Analy(pV8);
            V8_Pre_JM(pV8);

            pV8->ubInternal_state = 0;
            pV8->receive_state    = 0;

            isCmDetected = TRUE;
        }
    }

    if (isCmDetected)
    {
        UBYTE menu[4];
        int i;

        for (i = 0; i < 4; i++)
        {
            menu[i] = COMM_BitReversal((UBYTE)(pV8->pRecv_string1[i+1]));
        }

        V8_SetResponse(pTable, V8_A8M, menu, 4);

        pV8->tx_vec_idx++; /* send JM */
        pV8->rx_vec_idx++; /* detect CJ */
        pV8->DCE_timer = V8_TIME_OUT_VALUE;

        TRACE0("V8: CM detected!");
    }

#if SUPPORT_V34FAX
    else if (pV8->ANSam_TimeOut_Count > V8_ANSAM_TIMEOUT_TX && pV8->V34fax_Enable)
    {
        UBYTE a8m = 0;
        V8_SetResponse(pTable, V8_A8M, &a8m, 1);

        pV8->V34fax_Enable = FALSE;

        TRACE0("V8: V34Fax fall back 2...");
    }

#endif
}

#if SUPPORT_V92D
UBYTE V8_TONEq_Detection(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    CQDWORD TONEq_DFT980;
    QDWORD qEgy, qEgy980;
    QWORD qEgy_sqrt;
    QWORD qC, qS;
    QWORD qIn;
    UBYTE i;

    qEgy = 0;
    TONEq_DFT980.r = 0;
    TONEq_DFT980.i = 0;

    for (i = 0; i < V92D_BUF_SIZE; i++)
    {
        qIn = pV21->PCMinPtr[i];
        qEgy += (QQMULQD(qIn, qIn) >> V8_ANSAM_SHIFT);
#if 0

        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = qIn; }

#endif

        SinCos_Lookup_Fine(pV8->curPhase_980, &qS, &qC);  /* 65536*2100/9600, 2PI = 65536. */
        Phase_Update(&pV8->curPhase_980, DELTAPHASE_980);
        TONEq_DFT980.r += (QQMULQD(qIn, qC) >> V8_ANS_SHIFT);
        TONEq_DFT980.i += (QQMULQD(qIn, qS) >> V8_ANS_SHIFT);
    }

    pV8->ANSam_Egy += qEgy >> 2;

#if 0

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV8->ANSam_Egy; }

#endif

    pV8->TONEq_dft980.r += TONEq_DFT980.r >> V8_ANS_SHIFT;
    pV8->TONEq_dft980.i += TONEq_DFT980.i >> V8_ANS_SHIFT;

    pV8->TONEq_Counter++;

    if (pV8->TONEq_Counter == 20)
    {
        pV8->TONEq_Counter = 0;

        qEgy = pV8->ANSam_Egy;

        pV8->ANSam_Egy = 0;

        qC = (QWORD)(pV8->TONEq_dft980.r >> 13);
        qS = (QWORD)(pV8->TONEq_dft980.i >> 13);
        pV8->TONEq_dft980.r = 0;
        pV8->TONEq_dft980.i = 0;
        pV8->curPhase_2100 = 0;
        qEgy980  = QQMULQD(qC, qC);
        qEgy980 += QQMULQD(qS, qS);

#if 0

        if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = qEgy980; }

#endif

        if (qEgy980 > 10000)
        {
            pV8->ANSam_Tone_Count++;
        }
        else
        {
            pV8->ANSam_Tone_Count = 0;
        }

        if (pV8->ANSam_Tone_Count > 3)
        {
            return 1;
        }
    }

    return 0;
}

void V8_Detect_TONEq(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE result_code;

    result_code = V8_TONEq_Detection(pV8, pV21->PCMinPtr);

#if 0

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = result_code; }

#endif

    if (result_code == 1)
    {
        pV8->phase1_end = 1;
        pV8->DCE_timer = 3000;
        pV8->tx_vec_idx++;   /* Send Silence */
        TRACE0("V8: TONEq detected");
    }
}
#endif /* SUPPORT_V92D */
#endif
