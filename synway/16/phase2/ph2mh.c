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

#include <string.h>
#include "ph2ext.h"

#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)
CONST SBYTE ShiftTable_MH[] = { 7, -1 };

#define TX_VEC_SEND_RT                0
#define TX_VEC_SEND_SEQUENCE        1
#define TX_VEC_SEND_SILENCE            2

CONST V34Ph2FnPtr MhTxVecTab[] =
{
    Ph2_MH_Send_RT,
    Ph2_MH_Send_Sequence,
    Ph2_MH_Send_Silence
};

#define RX_VEC_DETECT_RT            0
#define RX_VEC_DETECT_SEQUENCE        1
#define RX_VEC_DETECT_RTSILENCE        2
#define RX_VEC_DETECT_DUMMY            3
#define RX_VEC_DETECT_END_RT        4

CONST V34Ph2FnPtr MhRxVecTab[] =
{
    Ph2_MH_Detect_RT,
    Ph2_MH_Detect_Sequence,
    Ph2_MH_Detect_RTSilence,
    Ph2_MH_Detect_Dummy,
    Ph2_MH_Detect_End_RT
};

void Ph2_Init_MH_Response(Ph2Struct *pPH2)
{
    pPH2->pfTx = MhTxVecTab;
    pPH2->pfRx = MhRxVecTab;

    pPH2->tx_vec_idx = TX_VEC_SEND_SEQUENCE;
    pPH2->rx_vec_idx = RX_VEC_DETECT_SEQUENCE;

    pPH2->mH_RxByte = pPH2->I0check.mH_byte;
    Ph2_MH_Setup_Response(pPH2);

    pPH2->info_idx = 0;
    Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->mH_TxByte), ShiftTable_MH);
}

void Ph2_Init_MH_Request(Ph2Struct *pPH2)
{
    pPH2->pfTx = MhTxVecTab;
    pPH2->pfRx = MhRxVecTab;
    pPH2->tx_vec_idx = TX_VEC_SEND_RT;
    pPH2->rx_vec_idx = RX_VEC_DETECT_RT;
    pPH2->info_idx = 0;
    Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->mH_TxByte), ShiftTable_MH);
}


void Ph2_MH_Detect_Sequence(Ph2Struct *pPH2)
{
    Info_0Struc *pI0 = &(pPH2->I0check);
    SBYTE MH_result, bit;

    bit = Ph2_DPSK_Demod(pPH2);

    if (bit >= 0)
    {
        pI0->pData_sequence_0[pI0->bit_num_cntr_0] = bit;
        pI0->bit_num_cntr_0++;
        MH_result = Ph2_MH_Info_Detect(bit, pI0);
    }

    if (MH_result != 0)
    {
        Ph2_Detect_Info_0_Init(pI0);

        if ((MH_result == MH_SEQUENCE_DETECTED) && (pPH2->mH_RxByte != pPH2->I0check.mH_byte))
        {
            pPH2->mH_RxByte = pPH2->I0check.mH_byte;
            Ph2_MH_Setup_Response(pPH2);
        }
    }
}

void Ph2_MH_Detect_RTSilence(Ph2Struct *pPH2)
{
    Ph2_DetCheck_ToneAB(pPH2, 7);

    Ph2_CarrierLoss_Detect(pPH2);

    if ((pPH2->dettone == 1) || (pPH2->Silence_Detect_Count > 150)) /* (128*150 / 9600 = 2 SECONDS silence) */
    {
        //pPH2->Silence_Detect_Count = 0;

        if ((pPH2->mH_TxByte & 0x0F) == MH_CDA)
        {
            TRACE0("RT or Silence Detected. Cleardown");
            pPH2->Ph2State.hang_up = PH2_HANGUP_V92MH_CLRD;
        }
        else if ((pPH2->mH_TxByte & 0x0F) == MH_ACK)
        {
            TRACE0("Modem On Hold: Going to Send Ansam");
            pPH2->Ph2State.hang_up = PH2_RECONNECT_V92MH;
        }
    }
}

void Ph2_MH_Detect_Dummy(Ph2Struct *pPH2)
{

}

void Ph2_MH_Detect_RT(Ph2Struct *pPH2)
{
    Ph2_DetCheck_ToneAB(pPH2, 7);

    if (pPH2->dettone == 1)
    {
        TRACE0("RT Detected");
        pPH2->tx_vec_idx = TX_VEC_SEND_SEQUENCE;

        if ((pPH2->mH_TxByte & 0x0F) == MH_FRR)
        {
            pPH2->rx_vec_idx = RX_VEC_DETECT_END_RT;
            pPH2->delay_counter_rx = 0;
        }
        else
        {
            pPH2->rx_vec_idx = RX_VEC_DETECT_SEQUENCE;
        }
    }
}

void Ph2_MH_Detect_End_RT(Ph2Struct *pPH2)
{
    ++pPH2->delay_counter_rx;

    Ph2_DetCheck_ToneAB(pPH2, 2);

    if ((pPH2->delay_counter_rx & 0x3F) == 0)
    {
        if (pPH2->dettone == 0)
        {
            TRACE1("End RT Detected: %d", pPH2->delay_counter_rx);
            pPH2->Ph2State.hang_up = PH2_RECONNECT_V92MH;
        }

        pPH2->dettone = 0;
    }
}

void Ph2_MH_Send_RT(Ph2Struct *pPH2)
{
    if (pPH2->modem_mode == CALL_MODEM)
    {
        Ph2_DPSK_Mod(pPH2, 0, V34_qCALL_MODEM_CARRIER);
    }
    else
    {
        Ph2_DPSK_Mod(pPH2, 0, V34_qANS_MODEM_CARRIER);
    }
}

void Ph2_MH_Send_Sequence(Ph2Struct *pPH2)
{
    if (pPH2->modem_mode == CALL_MODEM)
    {
        Ph2_DPSK_Modfilter(pPH2, pPH2->pInfo_buf[pPH2->info_idx], V34_qCALL_MODEM_CARRIER);
    }
    else
    {
        Ph2_DPSK_Modfilter(pPH2, pPH2->pInfo_buf[pPH2->info_idx], V34_qANS_MODEM_CARRIER);
    }

    pPH2->info_idx++;

    if (pPH2->info_idx >= MH_SEQUENCE_SIZE)
    {
        pPH2->info_idx = 0;
        Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)&(pPH2->mH_TxByte), ShiftTable_MH);
    }
}

void Ph2_MH_Send_Silence(Ph2Struct *pPH2)
{
    UBYTE i;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        pPH2->PCMoutPtr[i] = 0;
    }

    --pPH2->delay_counter_tx;

    if (pPH2->delay_counter_tx <= 0)
    {
        pPH2->Ph2State.hang_up = PH2_RECONNECT_V92MH;
    }
}


void Ph2_MH_Setup_Response(Ph2Struct *pPH2)
{
    UBYTE MH_signal = pPH2->mH_RxByte & 0x0F;

    if (MH_signal == MH_CLRD)
    {
        TRACE1("Cleardown Reason: %d", (pPH2->I0check.mH_byte & 0xF0));
        pPH2->mH_TxByte = MH_CDA;
        pPH2->mH_TxByte |= MH_CDA << 4;
        pPH2->rx_vec_idx = RX_VEC_DETECT_RTSILENCE;
        pPH2->tx_vec_idx = TX_VEC_SEND_SEQUENCE;
    }

    else if (MH_signal == MH_REQ)
    {
        /* Either Send ACK or NACK */
#if 0
        pPH2->mH_TxByte =  MH_NACK;
        pPH2->mH_TxByte |=     MH_NACK << 4;
        pPH2->rx_vec_idx = RX_VEC_DETECT_SEQUENCE;
#else
        pPH2->mH_TxByte =  MH_ACK;
        pPH2->mH_TxByte |=     MH_ACK << 4;
        pPH2->rx_vec_idx = RX_VEC_DETECT_RTSILENCE;
#endif

        pPH2->tx_vec_idx = TX_VEC_SEND_SEQUENCE;
    }

    else if (MH_signal == MH_FRR)
    {
        pPH2->rx_vec_idx = RX_VEC_DETECT_DUMMY;
        pPH2->tx_vec_idx = TX_VEC_SEND_SILENCE;  /* <= 80ms silence then go to phase 1 */
        pPH2->delay_counter_tx = 10;
    }

    else if (MH_signal == MH_CDA)
    {
        TRACE0("MH_CDA Detected, going to hang up");
        pPH2->Ph2State.hang_up = PH2_HANGUP_V92MH_CLRD;
    }

    else if (MH_signal == MH_NACK)
    {
        /* Either cleardown, or fast reconnect */
#if 0
        pPH2->mH_TxByte =  MH_CDA;
        pPH2->mH_TxByte |= MH_CDA;
#else
        pPH2->mH_TxByte =  MH_FRR;
        pPH2->mH_TxByte |= MH_FRR << 4;
#endif

        pPH2->rx_vec_idx = RX_VEC_DETECT_SEQUENCE;
        pPH2->tx_vec_idx = TX_VEC_SEND_SEQUENCE;
    }

    else if (MH_signal == MH_ACK)
    {
        /* MODEM ON HOLD: GO TO PHASE 1 */
        TRACE0("MH_ACK detected.  Going to phase 1");
        pPH2->rx_vec_idx = RX_VEC_DETECT_DUMMY;
        pPH2->tx_vec_idx = TX_VEC_SEND_SILENCE;
        TRACE1("Time Out Period: %4x ", (pPH2->I0check.mH_byte & 0xF0));
        pPH2->delay_counter_tx = 8000; /* For testing */
        pPH2->Ph2Timer = pPH2->delay_counter_tx + 1000;
    }

    else
    {
        TRACE0("UNKNOWN MH MESSAGE");
    }
}

#endif /* (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH) */
