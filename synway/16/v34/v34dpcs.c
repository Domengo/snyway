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
#include "v34ext.h"
#include "mhsp.h"   /* Modem Host Signal Processor */
#include "ptable.h" /* Data memory table defines   */
#include "hwapi.h"  /* Hardware Defines            */

void V34_Disconnect(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    SWORD     *pPCMin = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD    *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
#if DRAWEPG
    SWORD *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    UBYTE i;

    ClrReg(pDpcs->MCF); /* Clear Disconnect flag */

    for (i = 0; i < pMhsp->RtBufSize; i++)
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

#if DRAWEPG
    ++pEpg;

    for (i = 0; i < 16; i++)
    {
        *pEpg++ = 0;
    }

#endif

    /************* Disable IO tx/rx *************/
    Disconnect_Init(pTable);
}

void V34_DPCS_Phase3(UBYTE **pTable)
{
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34StateStruc *pV34State = &pRx->V34State;
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    Phase3_Info *p3 = &(pV34->p3);
    UBYTE Disc;

    Disc = RdReg(pDpcs->MCF, DISCONNECT);

    if (Disc)
    {
        WrReg(pDpcs->ABORTSR, REASON, 0); /* Disconnect reason */
        pV34State->hangup_reason = V34_HOST_DISCONNECT;
    }

    if (pV34State->hang_up == 1)
    {
        WrReg(pDpcs->ABORTSR, REASON, 1); /* Disconnect reason */
        pV34State->hang_up = 0;
        Disc = 1;
    }

    if (p3->TimeOut <= 0)
    {
        WrReg(pDpcs->ABORTSR, REASON, 2); /* Disconnect reason */
        Disc = 1;
        pV34State->hangup_reason = V34_PHASE3_TIMEOUT;
    }

    if (pV34State->Retrain_Times > V34_MAX_RETRAIN) /* allow three retrain */
    {
        WrReg(pDpcs->ABORTSR, REASON, 3); /* Disconnect reason */

        pV34State->Retrain_Times = 0;

        Disc = 1;
        pV34State->hangup_reason = V34_RETRAIN_TIMEOUT;
        TRACE0("Too many retrains: Hang up");
    }

    if (Disc)
    {
        V34_Disconnect(pTable);
        PutReg(pDpcs->DEBUGSR, (0x90 + pV34State->hangup_reason));
    }
}


#if 0 /// LLL temp
void V34_DPCS_Phase4(UBYTE **pTable)
{
    MhspStruct    *pMhsp     = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    V34Struct     *pV34      = (V34Struct *)(pTable[V34_STRUC_IDX]);
    Phase4_Info   *p4        = &(pV34->p4);
    //MpStruc     *pMpTx     = &p4->MpTx;
    MpStruc       *pMpRx     = &p4->MpRx;
    V34RxStruct   *pV34Rx    = &(pV34->V34Rx);
    ReceiveStruc  *pRx       = &pV34Rx->RecStruc;
    V34StateStruc *pV34State = &pRx->V34State;
    DpcsStruct    *pDpcs     = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    UBYTE  Disc, flag;

    Disc = RdReg(pDpcs->MCF, DISCONNECT);

    if (Disc)
    {
        WrReg(pDpcs->ABORTSR, REASON, 0); /* Disconnect reason */
        pV34State->hangup_reason = V34_HOST_DISCONNECT;
    }

    if (pV34State->hang_up == 1)
    {
        if (pRx->Renego.renego)
        {
            WrReg(pDpcs->ABORTSR, REASON, 1); /* Disconnect reason */
        }
        else
        {
            WrReg(pDpcs->ABORTSR, REASON, 0); /* Disconnect reason */
        }

        pV34State->hang_up = 0;
        Disc = 1;
    }

    if (p4->TimeOut <= 0)
    {
        if (pRx->Renego.clear_down)
        {
            Disc = 1;
            WrReg(pDpcs->ABORTSR, REASON, 1); /* Disconnect reason */
            TRACE0("Retrain: Phase4 Timeout, Renego");
        }
        else
        {
            pRx->Retrain_flag = RETRAIN_INIT;
            pV34State->retrain_reason = V34_PHASE4_TIMEOUT;
            TRACE0("Retrain: Phase4 Timeout");
        }
    }

    if (pV34State->Retrain_Times > V34_MAX_RETRAIN)      /* allow three retrain */
    {
        WrReg(pDpcs->ABORTSR, REASON, 3); /* Disconnect reason */
        pV34State->Retrain_Times = 0;
        Disc = 1;
        pV34State->hangup_reason = V34_RETRAIN_TIMEOUT;
        TRACE0("Too many retrains: Hang up");
    }

    if (Disc)
    {
        V34_Disconnect(pTable);
        PutReg(pDpcs->DEBUGSR, (0x90 + pV34State->hangup_reason));
        return;
    }

    if ((p4->Phase4_TxEnd) && (p4->Phase4_RxEnd))
    {
        pV34State->Retrain_Times = 0;  /* set retrain counter back to 0 */

        p4->Phase4_TxEnd    = 0;
        p4->Phase4_RxEnd    = 0;

#if 0
        {
            UBYTE i;

            for (i = 0; i < V34_EQ_LENGTH; i++)
            {
                if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV34Rx->Eq.pcEqcoef[i].r; }
            }

            for (i = 0; i < V34_EQ_LENGTH; i++)
            {
                if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV34Rx->Eq.pcEqcoef[i].i; }
            }

            for (i = 0; i < 3; i++)
            {
                if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pMpTx->PrecodeCoeff_h[i].r; }
            }

            for (i = 0; i < 3; i++)
            {
                if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pMpTx->PrecodeCoeff_h[i].i; }
            }
        }
#endif

        if (pV34->LAL_modem == 0)
        {
            pMhsp->pfRealTime = V34_Data_Handler;
        }
        else
        {
            p4->tx_vec_idx++;
            p4->rx_vec_idx++;
        }

        /* report Status to DPCS */
        PutReg(pDpcs->MSR0, DATA);

        flag = pV34Rx->rx_bit_rate + 3;/* Report bitrate to DPCS */
        WrReg(pDpcs->MBSC, BIT_RATE, flag);

        flag = pV34Rx->tx_bit_rate + 3;/* Report bitrate to DPCS */
        WrReg(pDpcs->MBSCTX, BIT_RATE, flag);

        WrReg(pDpcs->MBSR, BAUD_RATE, pV34Rx->rx_symbol_rate);
        WrReg(pDpcs->MBSR, CAR_FREQ, (pV34Rx->rx_high_freq << 3));

        TRACE1("pV34Rx->tx_bit_rate=%d", QQMULQD(pV34Rx->tx_bit_rate, 2400));
        TRACE1("pV34Rx->rx_bit_rate=%d", QQMULQD(pV34Rx->rx_bit_rate, 2400));
        TRACE1("pMpRx->const_shap_select_bit=%d", pMpRx->const_shap_select_bit);

        /* Make sure RTRN_DET bit is cleared */
        ResetReg(pDpcs->MSR0, RTRN_DET);/* Can't use ClrReg!!! Maybe delet two lines. */
        ResetReg(pDpcs->MSR0, RRATE_DET);

        /**************** Set Circuit 106 *****************/

        /************ Turn Circuit 109 ON *****************/
    }
}


void V34_DPCS_Data(UBYTE **pTable)
{
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34StateStruc *pV34State = &pRx->V34State;
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    UBYTE flag, Disc;

    Disc = 0;

    if (pRx->ubEcDiscCount > 0)
    {
        pRx->ubEcDiscCount--;

        if (pRx->ubEcDiscCount == 3)
        {
            ClrReg(pDpcs->MCF);
            WrReg(pDpcs->ABORTSR, REASON, REASON_LOCAL_EC_HANGUP);
            TRACE0("Local EC Hangup");
            Disc = 1;
        }
    }

    if (pV34State->hang_up == 1)
    {
        WrReg(pDpcs->ABORTSR, REASON, 1); /* Disconnect reason */
        pV34State->hang_up = 0;
        Disc = 1;
    }

    flag = GetReg(pDpcs->MCF);

    if (flag & RTRN)
    {
        ClrReg(pDpcs->MCF);    /* Clear Retrain request flag */
        pRx->Retrain_flag = RETRAIN_INIT;
    }
    else if (flag & RRATE)
    {
        ClrReg(pDpcs->MCF);   /* Clear Rate renegotiation flag */
        pRx->Renego.renego_generate = 1;
    }
    else if (flag & DISCONNECT)
    {
        ClrReg(pDpcs->MCF); /* Clear Disconnect flag */
        WrReg(pDpcs->ABORTSR, REASON, 0); /* Disconnect reason */
        pV34State->hangup_reason = V34_HOST_DISCONNECT;

        if (pV34->LAL_modem)
        {
            Disc = 1;
        }
        else
        {
            pRx->Renego.clear_down = 1;    /* Do clear down when disc */
        }
    }

    flag = GetReg(pDpcs->MCF);

    if ((flag & EC_DISC) && (pRx->ubEcDiscCount == 0))
    {
        pRx->ubEcDiscCount = 50; /* 50 for about 500 msec */
    }

    flag = RdReg(pDpcs->MSR0, REMOTE_EC_DISC);

    if (flag)
    {
        ClrReg(pDpcs->MSR0); /* Clear REMOTE_EC_DISC flag */

        if (pRx->ubEcDiscCount == 0)
        {
            WrReg(pDpcs->ABORTSR, REASON, REASON_REMOTE_EC_HANGUP); /* Disconnect reason */
            TRACE0("Remote EC Hangup");
        }
        else
        {
            ClrReg(pDpcs->MCF);
            WrReg(pDpcs->ABORTSR, REASON, REASON_LOCAL_EC_HANGUP);
            TRACE0("Local EC Hangup, remote acknowledged");
        }

        Disc = 1;
    }

    if (Disc)
    {
        V34_Disconnect(pTable);
        PutReg(pDpcs->DEBUGSR, (0x90 + pV34State->hangup_reason));
    }
}
#endif