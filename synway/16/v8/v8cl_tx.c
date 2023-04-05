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

#if SUPPORT_CT

void V8_Send_CallTone(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;
    QWORD qTone;

    pV8->CtCiTimer ++;

    if (pV8->CtCiState == 1)  /* Send tone if State is 1 */
    {
        for (i = 0; i < pV21->ubBufferSize; i++)
        {
            qTone = DSP_tCOS_TABLE[pV8->CTCurPhase >> 8];
            pV21->PCMoutPtr[i] = QQMULQ15(qTone, qSINGLE_TONE_12DBM0);/* -12dBm0 */
            pV8->CTCurPhase += pV8->DeltaPhase_1300;
        }

        if (pV8->CtCiTimer >= 150)     /* 0.5 sec on time */
        {
            pV8->CtCiTimer = 0;
            pV8->CtCiState = 0;
        }
    }
    else        /* Send silence if State is 0 */
    {
        V8_Send_Silence(pV8);

        if (pV8->CtCiTimer >= 450)    /* 1.5 sec off time */
        {
            pV8->CtCiTimer = 0;
            pV8->CtCiState = 1;
        }
    }
}
#endif

#if SUPPORT_CI
void V8_Send_CI(V8Struct *pV8)
{
    if (pV8->CtCiState == 0)
    {
        // send ON
        UBYTE currBit = (pV8->CI_Octets[pV8->cur_send_bit_index >> 3] >> (7 - (pV8->cur_send_bit_index & 0x7))) & 0x1;

        V8_V21Transmit(pV8, currBit);

        if (++pV8->cur_send_bit_index >= 30)
        {
            if (++pV8->CtCiTimer > 10) // 10 * 30 bits, or 1s
            {
                pV8->CtCiTimer = 0;
                pV8->CtCiState = 1;
            }

            pV8->cur_send_bit_index = 0;
        }
    }
    else
    {
        // OFF
        V8_Send_Silence(pV8);

        if (++pV8->CtCiTimer >= 300)    /* 1 sec off time */
        {
            pV8->CtCiTimer = 0;
            pV8->CtCiState = 0;
        }
    }
}
#endif

void V8_Send_Silence(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        pV21->PCMoutPtr[i] = 0;
    }
}

void V8_Send_Te(V8Struct *pV8)
{
    V8_Send_Silence(pV8);

    if (pV8->isToSendMenu && pV8->DCE_timer < 0)
    {
        pV8->tx_vec_idx++; /* send CM */

        pV8->rx_vec_idx++; /* detect JM */

        pV8->DCE_timer = 0;

        TRACE0("V8: TX: Send CM RX: Detect JM");
#if SUPPORT_V34FAX
        pV8->ANSam_TimeOut_Count = 0; TRACE0("V8: Rx JM timer starts");
#endif

#if SUPPORT_V92A
        pV8->DCE_timer = 600; /* Timeout QCA2d detection */
        TRACE0("V8: Send QC1a, Detect QCA1d");
#endif
    }
}


void V8_Send_CM(V8Struct *pV8)
{
    if (pV8->isToSendMenu)
    {
        V8_V21Transmit(pV8, pV8->CM_Octets[pV8->cur_send_bit_index]);

        if (++pV8->cur_send_bit_index >= pV8->ubCMLen)
        {
            pV8->cur_send_bit_index = 0;

#if SUPPORT_V92A

            /* no use here because of long CM frame */
            if (pV8->QCA1d_detected == 2)
            {
                pV8->QCA1d_detected = 3;
                pV8->tx_vec_idx++;  /* send Silence */
                pV8->rx_vec_idx++;  /* detect None */
                pV8->DCE_timer = 3400; /* Make sure at least 30 sent considering xmit delay */

                TRACE0("V8: Send None");
            }

#endif
        }
    }

    if (pV8->cur_send_bit_index == 0)
    {
        if (pV8->JM_detected)
        {
            pV8->tx_vec_idx++;

            pV8->DCE_timer = 34; /* Make sure at least 30 sent considering xmit delay */

            TRACE0("V8: TX: Send CJ   RX: JM Found");
        }
    }
}


void V8_Send_CJ(V8Struct *pV8)
{
    V8_V21Transmit(pV8, V8_CJ_OCTETS[pV8->cur_send_bit_index]);

    if (++pV8->cur_send_bit_index >= V8_CJ_LENGTH)
    {
        pV8->cur_send_bit_index = 0;
    }

    if (pV8->DCE_timer <= 0)
    {
        UBYTE a8j = 1;

        V8_SetResponse(pV8->v21.pTable, V8_A8J, &a8j, 1);
        pV8->phase1_end = 1; /* call modem phase1 end */

#if SUPPORT_V34FAX

        if (!pV8->JM_Octets[38])
        {
            pV8->V34fax_Enable = 0;
        }

#endif
        TRACE0("V8: TX: CJ Finish");
    }
}

#if SUPPORT_V92A

/********************************************************/
/*   V8_Send_QC2a                                       */
/********************************************************/
void V8_Send_QC2a(V8Struct *pV8)
{
    V8_V21Transmit(pV8, pV8->V8_QC1a_Octets[pV8->cur_send_bit_index]);

    if (++pV8->cur_send_bit_index >= pV8->ubQC1aLen)
    {
        pV8->cur_send_bit_index = 0;

        pV8->tx_vec_idx++;

        TRACE0("V8: Send None");
    }
}

/********************************************************/
/*   V8_Send_QC1a                                       */
/********************************************************/
void V8_Send_QC1a(V8Struct *pV8)
{
    V8_V21Transmit(pV8, pV8->V8_QC1a_Octets[pV8->cur_send_bit_index]);

    if (++pV8->cur_send_bit_index >= pV8->ubQC1aLen)
    {
        pV8->cur_send_bit_index = 0;

        pV8->tx_vec_idx++;

        TRACE0("V8: Send CM");
    }
}

/********************************************************/
/*   V8_Send_None                                       */
/********************************************************/
void V8_Send_None(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        pV21->PCMoutPtr[i] = 0;
    }
}

#define DELTAPHASE_980                6690   /* 65536*980/9600 */
#define V8_TONEq_AMPLITUDE            6000

/********************************************************/
/*   V8_Send_TONEq                                      */
/********************************************************/
void V8_Send_TONEq(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;
    QWORD qP0;

    for (i = 0; i < V92A_BUFSIZE; i++)
    {
        Sin_Lookup_Fine(&pV8->uPhase0, DELTAPHASE_980, &qP0);

        pV21->PCMoutPtr[i] = QQMULQ15(qP0, V8_TONEq_AMPLITUDE);
    }
}

/********************************************************/
/*   V8_Send_TONEq                                      */
/********************************************************/
void V8_Send_TONEq_End(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;

    for (i = 0; i < V92A_BUFSIZE; i++)
    {
        pV21->PCMoutPtr[i] = 0;
    }

    if (pV8->DCE_timer <= 0)
    {
        pV8->TONEq_finish = 1;
        TRACE0("V8: Enter Ph2");
    }

}

/********************************************************/
/*   V8_Send_Dummy                                      */
/********************************************************/
void V8_Send_Dummy(V8Struct *pV8)
{
    V21Struct *pV21 = &(pV8->v21);
    UBYTE i;

    for (i = 0; i < V92A_BUFSIZE; i++)
    {
        pV21->PCMoutPtr[i] = 0;
    }
}
#endif

