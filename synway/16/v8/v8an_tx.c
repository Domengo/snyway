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

#if USE_ANS

void V8_ANS_Send_Silence(V8Struct *pV8)
{
    V8_Send_Silence(pV8);

    if (pV8->DCE_timer <= 0)
    {
        pV8->tx_vec_idx++; /* Send ANSam */
        pV8->rx_vec_idx++; /* Det CM */
    }
}

void V8_Send_ANSam(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    QWORD qP0, qP1;
    QWORD qTemp;
    UBYTE i;

#if SUPPORT_V34FAX

    if (pV8->ANSam_TimeOut_Count++ > V8_ANSAM_TIMEOUT_TX - 5 * V8_CALL_RATE)
    {
        // fixing issue 163, send silence at the end of ANSam. This shortens ANSam by 50 ms
        V8_Send_Silence(pV8);
        return;
    }

#endif

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        if ((--pV8->ANSam_timer) == 0)
        {
            pV8->uPhase0 += 32768;

            pV8->ANSam_timer = pV8->AnsamTime_Len;
        }

        Sin_Lookup_Fine(&pV8->uPhase0, pV8->DeltaPhase_2100, &qP0);
        Sin_Lookup_Fine(&pV8->uPhase1, pV8->DeltaPhase_15,   &qP1);

        qP0   = QQMULQ15(qP0, V8_qANSAM_AMPLITUDE);
        qP1   = QQMULQ15(qP1, q02);
        qTemp = QQMULQ15(qP0, qP1);
        qTemp += qP0;

        pV21->PCMoutPtr[i] = qTemp;
    }
}

void V8_Send_JM(V8Struct *pV8)
{
    if (pV8->isToSendMenu)
    {
        V8_V21Transmit(pV8, pV8->JM_Octets[pV8->cur_send_bit_index]);

        if (++pV8->cur_send_bit_index >= pV8->ubJMLen)
        {
            pV8->cur_send_bit_index = 0;
        }
    }
}

#if SUPPORT_V92D
void V8_Send_QCA1d(V8Struct *pV8)
{
    V8_V21Transmit(pV8, pV8->V8_QCA1d_Octets[pV8->cur_send_bit_index]);

    if (++pV8->cur_send_bit_index >= pV8->V8_QCA1d_Length)
    {
        pV8->DCE_timer = 13;    /* 75 ms at 8K sample rate, 8000*0.075/48 */
        pV8->QCA1d_Finish = 1;
        pV8->tx_vec_idx++;        /* send silence */
        TRACE0("V8: Send Silence");
    }
}

void V8_Send_V92_Silence(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;

    for (i = 0; i < V92D_BUF_SIZE; i++)
    {
        pV21->PCMoutPtr[i] = 0;
    }

    if (pV8->DCE_timer <= 0)
    {
        if (pV8->phase1_end)
        {
            pV8->ANSpcm_Finish = 1;
        }
        else
        {
            pV8->DCE_timer = 16;
            pV8->tx_vec_idx++;      /* Send QTS */
            TRACE0("V8: Send QTS");
        }
    }
}

void V8_Send_QTS(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;

    for (i = 0; i < V92D_BUF_SIZE; i += 6)
    {
        pV21->PCMoutPtr[i    ] = pV8->Uqts;
        pV21->PCMoutPtr[i + 1] = 0;
        pV21->PCMoutPtr[i + 2] = pV8->Uqts;
        pV21->PCMoutPtr[i + 3] = -pV8->Uqts;
        pV21->PCMoutPtr[i + 4] = 0;
        pV21->PCMoutPtr[i + 5] = -pV8->Uqts;
    }

    if (pV8->DCE_timer <= 0)
    {
        pV8->DCE_timer = 1;
        pV8->tx_vec_idx++;     /* Send QTS_bar */
        TRACE0("V8: Send QTS_bar");
    }
}

void V8_Send_QTS_bar(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;

    for (i = 0; i < V92D_BUF_SIZE; i += 6)
    {
        pV21->PCMoutPtr[i    ] = -pV8->Uqts;
        pV21->PCMoutPtr[i + 1] = 0;
        pV21->PCMoutPtr[i + 2] = -pV8->Uqts;
        pV21->PCMoutPtr[i + 3] = pV8->Uqts;
        pV21->PCMoutPtr[i + 4] = 0;
        pV21->PCMoutPtr[i + 5] = pV8->Uqts;
    }

    if (pV8->DCE_timer <= 0)
    {
        pV8->DCE_timer = 3000;
        pV8->tx_vec_idx++;   /* Send ANSpcm */
        pV8->rx_vec_idx++;    /* detect TONEq */
        pV8->ANSpcm_Sigh = 1;
        pV8->ANSpcm_Index = 0;
        pV8->ANSpcm_Reverse_Count = 0;
        TRACE0("V8: Send ANSpcm");
        TRACE1("V8: rx_vec_idx %d", pV8->rx_vec_idx);
    }
}

void V8_Send_ANSpcm(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;
    SWORD tmp;

    for (i = 0; i < V92D_BUF_SIZE; i++)
    {
        if (pV8->ANSpcm_Sigh)
        {
            pV21->PCMoutPtr[i] = ANSpcm_Table[pV8->ANSpcm_Index++];
        }
        else
        {
            pV21->PCMoutPtr[i] = -ANSpcm_Table[pV8->ANSpcm_Index++];
        }

        if (pV8->ANSpcm_Index == 301)
        {
            pV8->ANSpcm_Index = 0;
        }

        pV8->ANSpcm_Reverse_Count++;

        if (pV8->ANSpcm_Reverse_Count == 3612)
        {
            pV8->ANSpcm_Reverse_Count = 0;
            pV8->ANSpcm_Sigh = 1 - pV8->ANSpcm_Sigh;
        }
    }
}
#endif /* SUPPORT_V92D */

#endif
