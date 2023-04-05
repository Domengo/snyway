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

#if SUPPORT_V92A
#include "v90aext.h"
#endif

#if SUPPORT_V8BIS
void V8_Detect_CRe(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    QWORD  qC;
    QWORD  qS;
    QWORD  qIn;
    QDWORD qEgy1;
    QDWORD qEgy2;
    QDWORD qEgy3;
    UBYTE i;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        qIn = pV21->PCMinPtr[i];
#if 0 /* for to dump Answer tone data. */

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = qIn; }

#endif

        if (pV8->CRe_Count == V8BIS_DETECT_NUM)
        {
            /* 400 Hz CRe Tone */
            SinCos_Lookup_Fine(pV8->curPhase_2085, &qS, &qC);    /* 65536*2085/9600, 2PI = 65536. */
            Phase_Update(&pV8->curPhase_2085, pV8->DeltaPhase_400);
            pV8->Ansam_dft2085.r += (QQMULQD(qIn, qC) >> CRE_SHIFT);
            pV8->Ansam_dft2085.i += (QQMULQD(qIn, qS) >> CRE_SHIFT);
        }

        else
        {
            /* 2002 Hz Precedes CRe tone */
            SinCos_Lookup_Fine(pV8->curPhase_2085, &qS, &qC);  /* 65536*2100/9600, 2PI = 65536. */
            Phase_Update(&pV8->curPhase_2085, pV8->DeltaPhase_2002);
            pV8->Ansam_dft2085.r += (QQMULQD(qIn, qC) >> CRE_SHIFT);
            pV8->Ansam_dft2085.i += (QQMULQD(qIn, qS) >> CRE_SHIFT);
        }

        /* 1375 Hz Tone, Precedes CRe tone */
        SinCos_Lookup_Fine(pV8->curPhase_2100, &qS, &qC);  /* 65536*2085/9600, 2PI = 65536. */
        Phase_Update(&pV8->curPhase_2100, pV8->DeltaPhase_1375);

        pV8->Ansam_dft2100.r += (QQMULQD(qIn, qC) >> CRE_SHIFT);
        pV8->Ansam_dft2100.i += (QQMULQD(qIn, qS) >> CRE_SHIFT);

        /* Reference Tone 2100 Hz:    (could be ANSAM or ANS TONE) */
        SinCos_Lookup_Fine(pV8->curPhase_2115, &qS, &qC);  /* 65536*2115/9600, 2PI = 65536. */
        Phase_Update(&pV8->curPhase_2115, pV8->DeltaPhase_2100);
        pV8->Ansam_dft2115.r += (QQMULQD(qIn, qC) >> CRE_SHIFT);
        pV8->Ansam_dft2115.i += (QQMULQD(qIn, qS) >> CRE_SHIFT);
    }

    pV8->ANS15_Counter++;

    if (pV8->ANS15_Counter == 3)
    {
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

        pV8->ANS15_Counter = 0;

#if 0

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = qEgy1; }

        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = qEgy2; }

        if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = qEgy3; }

#endif

        if (pV8->CRe_Count == V8BIS_DETECT_NUM)
        {
            if (((qEgy1 >> 3) > (qEgy2)) && ((qEgy1 >> 3) > (qEgy2)))
            {
                /* 400 Hz Tone Detected */
                pV8->tx_vec_idx++;
                pV8->rx_vec_idx++;
                pV8->DCE_timer = V8BIS_TE;
                pV8->AnsAm_Detected = 1;
                TRACE0("V8: CRE Detect in V8 Bis");
            }
        }
        else
        {
            if (((qEgy1 >> 3) > (qEgy3 + 100)) && ((qEgy2 >> 3) > (qEgy3 + 100)))
            {
                pV8->CRe_Count++;  /* 1375Hz, 2002 Hz Tones detected */
                pV8->ANSam_Tone_Count = 0;
            }
            else if (((qEgy3 >> 3) > (qEgy1)) && ((qEgy3 >> 3) > (qEgy2)))
            {
                pV8->ANSam_Tone_Count++; /* 2100Hz Tone detected */
                pV8->CRe_Count = 0;
            }
            else
            {
                pV8->ANSam_Tone_Count = 0;
                pV8->CRe_Count = 0;
            }

            if (pV8->ANSam_Tone_Count == V8BIS_DETECT_NUM)
            {
                pV8->V8bisTimeout = 1;    /* Go to V.8 */
                TRACE0("V8: ANS Detect in V8 Bis");
            }
        }
    }
}

#endif

void V8_Detect_ANSam(V8Struct *pV8)
{
    UBYTE ansamDetected = 0;

    V21Struct *pV21 = &(pV8->v21);
    UBYTE **pTable = pV21->pTable;

    ansamDetected = V8_ANSam15_Detect(pV8);

    if (ansamDetected)
    {
        UBYTE dummy;

        V8_SetResponse(pTable, V8_OK, &dummy, 1);
        /* ANSam is detected but we need to wait for Te time to disable */
        /* the network echo suppressor */
        pV8->tx_vec_idx++; /* send Te */
        pV8->rx_vec_idx++; /* receive dummy */
        pV8->DCE_timer = V8_TE;
        pV8->AnsAm_Detected = 1;

        TRACE0("V8: ANSam detected");
    }

#if SUPPORT_V34FAX
    else
    {
        pV8->ANSam_TimeOut_Count ++;

        if ((pV8->ANSam_TimeOut_Count > V8_ANSAM_TIMEOUT_RX) && (pV8->V34fax_Enable == 1))
        {
            UBYTE ansSig = 2;
            V8_SetResponse(pTable, V8_A8A, &ansSig, 1);

            pV8->V34fax_Enable = 0;

            TRACE0("V8: V34Fax fall back at call side!");
        }
    }

#endif
}


void V8_Dummy(V8Struct *pV8)
{
}

UBYTE V8_Detect_Sub(V8Struct *pV8, UBYTE which_seq)
{
    SWORD *pString_ptr;
    UBYTE i, ubTemp;

    if (pV8->ubInternal_state != 2)
    {
        /* Use internal state to decide which string I am using */
        if (pV8->ubInternal_state == 0)
        {
            pString_ptr = pV8->pRecv_string1;
        }
        else /* if (pV8->ubInternal_state == 1) */
        {
            pString_ptr = pV8->pRecv_string2;
        }

        /* Try to receive a CM/JM string */
        ubTemp = V8_Recv_Seq(pV8, which_seq, pString_ptr);

        /* If recv_seq return 1, we have a whole string */
        /* switch to other string and break */
        if (ubTemp == 1)
        {
            pV8->ubInternal_state = (UBYTE)(1 - pV8->ubInternal_state);
        }
    }

    i = 0;
    pV8->result_code = 0;

    while (pV8->result_code == 0)
    {
        pV8->result_code |= pV8->pRecv_string1[i] ^ pV8->pRecv_string2[i];

        if (pV8->pRecv_string1[i] == 0x7E)
        {
            break;
        }

        i++;
    }

    return(i);
}

void V8_Detect_JM(V8Struct *pV8)
{
    UBYTE jmDetected = 0;
    V21Struct *pV21 = &(pV8->v21);
    UBYTE **pTable = pV21->pTable;
    UBYTE i;

    i = V8_Detect_Sub(pV8, V8_JM_TYPE);

    if ((pV8->result_code == 0) && (pV8->pRecv_string1[0] == 0x07) && (i >= 4))
    {
        pV8->ubInternal_state = 2;

        V8_Analy(pV8);
        TRACE1("V8: JM detected: V34 (162) V90 (166) avai %x", pV8->pRecv_string1[2]);

        jmDetected = 1;
    }

    if (jmDetected)
    {
        UBYTE menu[4];
        int i;

        for (i = 0; i < 4; i++)
        {
            menu[i] = COMM_BitReversal((UBYTE)(pV8->pRecv_string1[i+1]));
        }

        V8_SetResponse(pTable, V8_A8M, menu, 4);
        /* switch TX state, prepare to send CJ */
        pV8->JM_detected = 1;

        pV8->rx_vec_idx++; /* Receive silence */
    }
    else
    {
#if SUPPORT_V34FAX

        if (pV8->ANSam_TimeOut_Count++ > V8_JM_TIMEOUT_RX)
        {
            UBYTE a8m = 0;
            V8_SetResponse(pTable, V8_A8M, &a8m, 1);

            pV8->ANSam_TimeOut_Count = 0; TRACE0("V8: Rx JM timer elapsed");
            pV8->JM_detected = 1;
            pV8->V34fax_Enable = 0;
            pV8->rx_vec_idx++; /* Receive silence */
        }

#endif
    }
}

#if SUPPORT_V92A
/********************************************************/
/*  V8_Detect_QCA2d                                     */
/********************************************************/
void V8_Detect_QCA2d(V8Struct *pV8)
{
    UBYTE bit;

    bit = V8_V21Receive(pV8);

    if (pV8->DCE_timer < 0)
    {
        pV8->V8bisTimeout = 1;    // timeout go to V.8;
    }

    pV8->QCA1d_ONEs_Sync = (pV8->QCA1d_ONEs_Sync << 1) | bit;

    if ((pV8->QCA1d_ONEs_Sync & 0xFFFFF003) == 0x7E7EB003)
    {
        pV8->QCA1d_ONEs_Sync = 0;
        pV8->QCA1d_cnt = 0;
        pV8->QCA1d_detected = 2;

        if (pV8->QCA1d_detected == 2)
        {
            pV8->rx_vec_idx++;            /* detect None */
            pV8->QCA1d_cnt = 24;
            TRACE0("V8: QCA2d detected");
        }
    }
    else if ((pV8->QCA1d_ONEs_Sync & 0xFFFFF) == 0xFFC0F) /* THIS IS JM */
    {
        pV8->pfTx[1] = V8_Send_Te;      /* Tx: State 1  */
        pV8->pfTx[2] = V8_Send_CM;      /* Tx: State 2  */
        pV8->pfTx[3] = V8_Send_CJ;      /* Tx: State 3  */
        pV8->tx_vec_idx = 2;

        pV8->pfRx[0] = V8_Detect_ANSam;  /* Rx: State 0  */
        pV8->pfRx[1] = V8_Dummy;          /* Rx: State 1  */
        pV8->pfRx[2] = V8_Detect_JM;      /* Rx: State 2  */
        pV8->pfRx[3] = V8_Dummy;          /* Tx: State 3  */
        pV8->rx_vec_idx = 2;
    }
}


/********************************************************/
/*  V8_Detect_QCA1d                                     */
/********************************************************/
void V8_Detect_QCA1d(V8Struct *pV8)
{
    UBYTE bit;

    bit = V8_V21Receive(pV8);

    pV8->QCA1d_ONEs_Sync = (pV8->QCA1d_ONEs_Sync << 1) | bit;

    if ((pV8->QCA1d_ONEs_Sync & 0xFFFFF) == 0xFFD55)
    {
        pV8->QCA1d_ONEs_Sync = 0;
        pV8->QCA1d_detected++;

        if (pV8->QCA1d_detected == 2)
        {
            pV8->tx_vec_idx++;            /* send Silence */
            pV8->rx_vec_idx++;            /* detect None */
            pV8->QCA1d_cnt = 11;
            TRACE0("V8: QCA1d detected");
        }
    }
    else if ((pV8->QCA1d_ONEs_Sync & 0xFFFFF) == 0xFFC0F) /* THIS IS JM */
    {
        pV8->pfTx[1] = V8_Send_Te;      /* Tx: State 1  */
        pV8->pfTx[2] = V8_Send_CM;      /* Tx: State 2  */
        pV8->pfTx[3] = V8_Send_CJ;      /* Tx: State 3  */
        pV8->tx_vec_idx = 2;

        pV8->pfRx[0] = V8_Detect_ANSam;  /* Rx: State 0  */
        pV8->pfRx[1] = V8_Dummy;          /* Rx: State 1  */
        pV8->pfRx[2] = V8_Detect_JM;      /* Rx: State 2  */
        pV8->pfRx[3] = V8_Dummy;          /* Tx: State 3  */
        pV8->rx_vec_idx = 2;
    }
    else
    {
        if (pV8->QCA1d_detected)
        {
            pV8->QCA1d_cnt++;

            if (pV8->QCA1d_cnt == 4)
            {
                pV8->QCA1d_P = bit;
            }
            else if (pV8->QCA1d_cnt == 8)
            {
                pV8->QCA1d_LM = (bit << 1);
            }
            else if (pV8->QCA1d_cnt == 9)
            {
                pV8->QCA1d_LM |= bit;
            }
        }
    }
}

/********************************************************/
/*  V8_Detect_None                                      */
/********************************************************/
void V8_Detect_None(V8Struct *pV8)
{
    V90aDftStruct *pDFT = &(pV8->DFT);

    pV8->QCA1d_cnt--;

    if (pV8->QCA1d_cnt == 0)
    {
        pV8->rx_vec_idx++;    /* detect QTS */
        pDFT->curr_idx = 0;
        pDFT->SdTone.r = 0;
        pDFT->SdTone.i = 0;
        pDFT->SdRef.r = 0;
        pDFT->SdRef.r = 0;
        pDFT->DetTones = 0;
    }

}

/********************************************************/
/*  V8_Detect_QTS                                       */
/********************************************************/
void V8_Detect_QTS(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i, j, sym_total;
    V90aDftStruct *pDFT = &(pV8->DFT);
    CQWORD cOut[2];
    SDWORD current_angle, angle;
    QDWORD phaseAcc, phaseInc;
    UBYTE QTS_detected = 0;

    for (i = 0; i < V92A_LOOP_RATE; i++)
    {
        V90a_Tim_Rx_Buffer_Load(&(pV8->TIMRx), V92A_SAMPLES_PER_LOOP, pV21->PCMinPtr);
        sym_total = V90a_Tim_Rx(&(pV8->TIMRx), V92A_SAMPLES_PER_LOOP, pV8->Rx_sword);

        for (j = 0; j < sym_total; j++)
        {
#if PRINT_QTS

            if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV8->Rx_sword[j]; }

#endif

            if (V8_QTS_DFT_Detect(pDFT, pV8->Rx_sword[j], cOut))
            {
                if (pDFT->DetTones == 3)
                {
                    pDFT->AngleX = cOut[0].r;
                    pDFT->AngleY = cOut[0].i;
                    /****  Rotate carrier to -60 degrees to align frame ****/
                    current_angle = DSPD_Atan2(pDFT->AngleX, pDFT->AngleY);
                    phaseAcc = pV8->TIMRx.phaseAcc;
                    phaseInc = pV8->TIMRx.phaseInc << 1;

                    while (current_angle > (-qD_60))
                    {
                        current_angle = current_angle - qD_360;
                    }

                    while (current_angle < (-qD_60 << 1))
                    {
                        phaseAcc -= phaseInc;

                        while (phaseAcc < 0)
                        {
                            pV8->TIMRx.tim_pointer--;
                            phaseAcc += MAX_PHASE;
                        }

                        current_angle = current_angle + qD_60;
                    }

                    angle = current_angle + qD_60;
                    current_angle = (angle * q1_OVER_PI) >> 15;
                    angle = current_angle * 3;
                    current_angle = (phaseInc >> 15) * (angle);
                    phaseAcc += (QDWORD)current_angle;

                    while (phaseAcc < 0)
                    {
                        pV8->TIMRx.tim_pointer--;
                        phaseAcc += MAX_PHASE;
                    }

                    if (pV8->TIMRx.tim_pointer < 0)
                    {
                        pV8->TIMRx.tim_pointer += TIM_DELAYLEN_RX;
                    }

                    pV8->TIMRx.phaseAcc = phaseAcc;
                    /**** End Carrier Rotate **********/


                    QTS_detected = 1;
                    pV8->QTSbar_counter = 0;
                    TRACE0("V8: QTS detected");
                }
            }
        }
    }

    if (QTS_detected == 1)
    {
        pV8->rx_vec_idx++;        /* Go to detect QTS bar */
    }
}

/********************************************************/
/*  V8_Detect_QTS_bar                                   */
/********************************************************/
void V8_Detect_QTS_bar(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    V90aDftStruct *pDFT = &(pV8->DFT);
    UBYTE i, j, sym_total;
    CQWORD cOut[2];
    UBYTE QTS_bar_detected = 0;

    for (i = 0; i < V92A_LOOP_RATE; i++)
    {
        V90a_Tim_Rx_Buffer_Load(&(pV8->TIMRx), V92A_SAMPLES_PER_LOOP, pV21->PCMinPtr);
        sym_total = V90a_Tim_Rx(&(pV8->TIMRx), V92A_SAMPLES_PER_LOOP, pV8->Rx_sword);

        for (j = 0; j < sym_total; j++)
        {
#if PRINT_ANSPCM

            if ((pV8->QTS_bar_detected == 1) && (pV8->QTSbar_counter == 0) && ((j & 1) == 0))
            {
                if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV8->Rx_sword[j]; }
            }

#endif

#if PRINT_QTS

            if ((pV8->QTS_bar_detected == 0) || (pV8->QTSbar_counter > 0))
            {
                if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV8->Rx_sword[j]; }
            }

#endif

            if (pV8->QTS_bar_detected == 1)
            {
                if (pV8->QTSbar_counter > 0)
                {
                    --pV8->QTSbar_counter;
                }
                else if ((j & 1) == 0)
                {
                    ++pV8->ANSam_Tone_Count;    /* ANSPcm Counter */
                }
            }
            else if (V8_QTS_DFT_Detect(pDFT, pV8->Rx_sword[j], cOut))
            {
                QTS_bar_detected = V8_QTS_Angle_Detect(pDFT, cOut);

                if (QTS_bar_detected)
                {
                    pV8->QTS_bar_detected = 1;
                    pV8->QTSbar_counter = 86; /* Number of 16KHz samples left before ANSpcm */
                }
            }
        }
    }

    if ((pV8->QTS_bar_detected == 1) && (pV8->QTSbar_counter == 0))
    {
        pV8->rx_vec_idx++;         /* Go to detect ANSpcm */
        TRACE0("V8: QTS_bar detected");
    }
}

/********************************************************/
/*  V8_Detect_ANSpcm                                    */
/********************************************************/
void V8_Detect_ANSpcm(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i, j, sym_total;
    UWORD frame_idx;
    QWORD abserr;
    QWORD qC, qS;
    QDWORD angle, qR, qI;

    for (i = 0; i < V92A_LOOP_RATE; i++)
    {
        V90a_Tim_Rx_Buffer_Load(&(pV8->TIMRx), V92A_SAMPLES_PER_LOOP, pV21->PCMinPtr);
        sym_total = V90a_Tim_Rx(&(pV8->TIMRx), V92A_SAMPLES_PER_LOOP, pV8->Rx_sword);

        for (j = 0; j < sym_total; j += 2) /* Downsample from 16Khz to 8KHz */
        {
#if PRINT_ANSPCM

            if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV8->Rx_sword[j]; }

#endif

            /*** RBS DETECTION ***/
            if (pV8->ANS15_Counter == 2)
            {
                pV8->savePcm[pV8->ANSam_Tone_Count] = pV8->Rx_sword[j];
            }
            else if ((pV8->ANS15_Counter >= 3) && (pV8->ANS15_Counter <= 8))
            {
                frame_idx = (pV8->ANS15_Counter - 3) + pV8->ANSam_Tone_Count;
                frame_idx =  frame_idx % 6;
                abserr = pV8->Rx_sword[j] - pV8->savePcm[pV8->ANSam_Tone_Count];

                if (abserr  < 0)
                {
                    abserr = -abserr;
                }

                pV8->savePcm[pV8->ANSam_Tone_Count] = pV8->Rx_sword[j];
                pV8->sumDiff[frame_idx] += abserr;

                if ((pV8->ANS15_Counter == 8) && (pV8->ANSam_Tone_Count == 300))
                {
                    TRACE3("V8: ANSPcm Diffs:  %6d  %6d  %6d  ", pV8->sumDiff[0], pV8->sumDiff[1], pV8->sumDiff[2]);
                    TRACE3("V8: ANSPcm Diffs:  %6d  %6d  %6d  ", pV8->sumDiff[3], pV8->sumDiff[4], pV8->sumDiff[5]);
                }
            }

            SinCos_Lookup_Fine(pV8->curPhase_2100, &qS, &qC);  /* 65536*79/301, 2PI = 65536  */
            Phase_Update(&pV8->curPhase_2100, 17200);
            pV8->Ansam_dft2100.r  += (QQMULQD(pV8->Rx_sword[j], qC) >> 15);
            pV8->Ansam_dft2100.i  += (QQMULQD(pV8->Rx_sword[j], qS) >> 15);

            pV8->ANSam_Tone_Count++;

            if (pV8->ANSam_Tone_Count & 1)
            {
                ++pV8->curPhase_2100;    /* Delta = 17200.5 */
            }

            if (pV8->ANSam_Tone_Count == 301)
            {
                /**  Calculate Drift **/
                pV8->ANS15_Counter++;
                qR =  pV8->Ansam_dft2100.r;
                qI =  pV8->Ansam_dft2100.i;

                while ((qR < -32768) || (qR > 32768) || (qI < -32768) || (qI > 32768))
                {
                    qR = qR >> 1;
                    qI = qI >> 1;
                }

                angle = DSPD_Atan2((QWORD)qR, (QWORD)qI);

                if (pV8->ANS15_Counter == 3)
                {
                    pV8->ANSam_Egy = angle;  /* Save as first point to calculate slope */
                }
                else if (pV8->ANS15_Counter == 15)
                {
                    angle = angle - pV8->ANSam_Egy;

                    while (angle > qD_90)
                    {
                        angle = angle - qD_180;
                    }

                    while (angle < (-qD_90))
                    {
                        angle = angle + qD_180;
                    }

                    /* MAXIMUM MEASURABLE PPM OFFSET = 264PPM */
                    /* Because (qD_90 * V8_PPM_FACTOR) >> V8_PPM_SHIFT = 264 */
#if 1
                    pV8->drift = (angle * V8_PPM_FACTOR) >> (V8_PPM_SHIFT - 8);  // Q8 PPM FORMAT */
                    TRACE1("V8: Phase1: Q8 PPM OFFSET =%d", pV8->drift);
#endif
                    pV8->drift = -(angle * V8_DRIFT_FACTOR);  // Phase2 FORMAT

                    pV8->drift += V8_DRIFT_DELTA;
                    /* Setup AnsamEgy as threshold for detect silence */
                    qR =  pV8->Ansam_dft2100.r >> 12;
                    qI =  pV8->Ansam_dft2100.i >> 12;
                    pV8->ANSam_Egy = (QQMULQD(qR, qR));
                    pV8->ANSam_Egy += (QQMULQD(qI, qI));

                    /* Start Sending TONEQ, and detect silence after ANSPCM */
                    pV8->rx_vec_idx++;
                    pV8->tx_vec_idx++;
                }

                pV8->Ansam_dft2100.r = 0;
                pV8->Ansam_dft2100.i = 0;
                pV8->curPhase_2100 = 0;
                pV8->ANSam_Tone_Count = 0;
            }
        }
    }
}

/********************************************************/
/*  V8_Detect_ANSpcm                                    */
/********************************************************/
void V8_Detect_End_ANSPcm(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i, j, sym_total;
    QWORD  qC, qS;
    QDWORD qR, qI, qEgy;

    for (i = 0; i < V92A_LOOP_RATE; i++)
    {
        V90a_Tim_Rx_Buffer_Load(&(pV8->TIMRx), V92A_SAMPLES_PER_LOOP, pV21->PCMinPtr);
        sym_total = V90a_Tim_Rx(&(pV8->TIMRx), V92A_SAMPLES_PER_LOOP, pV8->Rx_sword);

        for (j = 0; j < sym_total; j += 2)    /* Downsample from 16Khz to 8KHz */
        {
#if PRINT_ANSPCM

            if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV8->Rx_sword[j]; }

#endif
            SinCos_Lookup_Fine(pV8->curPhase_2100, &qS, &qC);  /* 65536*79/301, 2PI = 65536  */
            Phase_Update(&pV8->curPhase_2100, 17200);
            pV8->Ansam_dft2100.r  += (QQMULQD(pV8->Rx_sword[j], qC) >> 15);
            pV8->Ansam_dft2100.i  += (QQMULQD(pV8->Rx_sword[j], qS) >> 15);
            pV8->ANSam_Tone_Count++;

            if (pV8->ANSam_Tone_Count & 1)
            {
                ++pV8->curPhase_2100;    /* Delta = 17200.5 */
            }

            if ((pV8->ANSam_Tone_Count & 0x0F) == 0)
            {
                qR =  pV8->Ansam_dft2100.r >> 5;
                qI =  pV8->Ansam_dft2100.i >> 5;
                qEgy = QQMULQD(qR, qR);
                qEgy += QQMULQD(qI, qI);

#if 0

                if (DumpTone3_Idx < 10000) { DumpTone3[DumpTone3_Idx++] = qEgy; }

                if (DumpTone3_Idx < 10000) { DumpTone3[DumpTone3_Idx++] = pV8->ANSam_Egy; }

#endif

                if (qEgy < pV8->ANSam_Egy)
                {
                    pV8->tx_vec_idx++; /* stop sending TONEq */
                    pV8->rx_vec_idx++;
                    pV8->DCE_timer = 10;
                    pV8->V92_Enable = 1;
                    return;
                }

                pV8->Ansam_dft2100.r = 0;
                pV8->Ansam_dft2100.i = 0;
                pV8->curPhase_2100 = 0;
                pV8->ANSam_Tone_Count = 0;
            }
        }
    }
}


/********************************************************/
/*   V8_QTS_DFT_Detect                                */
/********************************************************/
UBYTE V8_QTS_DFT_Detect(V90aDftStruct *pDFT, SWORD sample, CQWORD *out)
{
    SWORD qCos;
    SWORD qSin;
    QWORD qR;
    QWORD qRef;
    QWORD idxs, idxc;

    /* 1333.3 Hz: Delta Phase = Pi/6 */
    /* 2666.6 Hz: Delta Phase = Pi/3, (Ref) */
    /* 4000.0 Hz: Delta Phase = Pi/2 */

    idxc = (QWORD)(pDFT->curr_idx);
    idxs = idxc - 3; /* step back Pi/2 for sine idx */

    if (idxs < 0)
    {
        idxs += 12;
    }

    qCos = DSP_tCOS_TAB12[idxc];
    qSin = DSP_tCOS_TAB12[idxs];

    pDFT->SdTone.r += (QQMULQD(sample, qCos) >> 1);
    pDFT->SdTone.i += (QQMULQD(sample, qSin) >> 1);

    idxc <<= 1;

    if (idxc >= 12)
    {
        idxc -= 12;
    }

    idxs = idxc - 3;

    if (idxs < 0)
    {
        idxs += 12;
    }

    qCos = DSP_tCOS_TAB12[idxc];
    qSin = DSP_tCOS_TAB12[idxs];
    pDFT->SdRef.r += (QQMULQD(sample, qCos) >> 1);
    pDFT->SdRef.i += (QQMULQD(sample, qSin) >> 1);

    pDFT->curr_idx++;

    if (pDFT->curr_idx == 12)
    {
        pDFT->curr_idx = 0;
        out[0].r = QDR15Q(pDFT->SdRef.r);
        out[0].i = QDR15Q(pDFT->SdRef.i);
        qRef = QD16Q(CQPOW(out[0]));
        pDFT->SdRef.r = 0;
        pDFT->SdRef.i = 0;

        out[0].r = QDR15Q(pDFT->SdTone.r);
        out[0].i = QDR15Q(pDFT->SdTone.i);
        qR = QD16Q(CQPOW(out[0]));
        pDFT->SdTone.r = 0;
        pDFT->SdTone.i = 0;

        if ((qR > 5) && (qR > (10 * qRef)))
        {
            pDFT->DetTones++;

            return((UBYTE)qR); //Cast as UBYTE to prevent compile warning. This detection algorithm needs to be checked.
        }
        else
        {
            pDFT->DetTones = 0;
            return(1);
        }
    }
    else
    {
        return(0);
    }
}

/********************************************************/
/*     V8_QTS_Angle_Detect                                */
/********************************************************/
SBYTE V8_QTS_Angle_Detect(V90aDftStruct *pDFT, CQWORD *out)
{
    SDWORD previous_angle;
    SDWORD current_angle;
    SDWORD angle_differ;

    previous_angle = DSPD_Atan2(pDFT->AngleX, pDFT->AngleY);
    current_angle = DSPD_Atan2(out[0].r, out[0].i);
    pDFT->AngleX = out[0].r;
    pDFT->AngleY = out[0].i;

    angle_differ = current_angle - previous_angle;

    if (angle_differ > qD_180)
    {
        angle_differ -= qD_360;
    }
    else if (angle_differ < (-qD_180))
    {
        angle_differ += qD_360;
    }

    if (angle_differ < 0)
    {
        angle_differ = -angle_differ;
    }

    if (pDFT->S_OK > 10)
    {
        if ((angle_differ > qD_90 - 0000) && (angle_differ < qD_270 + 20000))
        {
            return(1);
        }
    }
    else
    {
        ++pDFT->S_OK;
    }

    return(0);
}

#endif
