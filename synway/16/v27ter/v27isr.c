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

#include "v27ext.h"
#include "hwapi.h"
#include "mhsp.h"
#include "ptable.h"
#if V27_HW7200SAMPLERATE_SUPPORT
#include "smp.h"
#endif

void V27_Dpcs_Isr(UBYTE **pTable)
{
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);
#if DRAWEPG
    SWORD *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    SWORD *pPCMin = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
#if V27_HW7200SAMPLERATE_SUPPORT
    InDe_Struct *pMultiFilterOut = (InDe_Struct *)(pTable[SMPDATAOUT_IDX]);
    InDe_Struct *pMultiFilterIn  = (InDe_Struct *)(pTable[SMPDATAIN_IDX]);
    SWORD  pPCMin_temp[96], pPCMout_temp[72];
#endif
    V27Struct *pV27 = (V27Struct *)(pTable[V27_STRUC_IDX]);
#if WITH_DC
    DCStruct *pDC = (DCStruct *)(pTable[DC_STRUC_IDX]);
#endif

#if SUPPORT_ENERGY_REPORT
    QDWORD qdEnergy;
#endif
    UBYTE TimeOut_flag = 0;
    UBYTE i, Disc;

#if DRAWEPG
    /* Initial EYE pattern display symbol count */
    *pEpg = 0;
#endif

    pV27->PCMinPtr  = pPCMin;
    pV27->PCMoutPtr = pPCMout;

    for (i = 0; i < V27_BUF_SIZE; i++)
    {
        pPCMout[i] = 0;
    }

#if V27_HW7200SAMPLERATE_SUPPORT
    SampleRate_Change(pMultiFilterIn, pPCMin, pPCMin_temp);

    pPCMin = pPCMin_temp;
    pV27->PCMinPtr = pPCMin;
#endif

#if WITH_DC
    DcEstimator(pDC, pV27->PCMinPtr, V27_BUF_SIZE, -9);/* Low pass DC estimator filter */
#endif

    if (pV27->ImageSendMode == IMAGE_RECEIVE_MODE)
    {
        if (pV27->Timer >= 5600)   /* 3.5 Sec Timeout condition */
        {
            TRACE0("V27: Timeout Disconnect");
            pV27->Timer = 0;
            TimeOut_flag = 1;
        }

        V27_isrR(pV27);

#if SUPPORT_ENERGY_REPORT

        if ((pV27->Detect_FaxTone != 0) || (pV27->V21DetectNumber >= 2))
        {
            if (pV27->Detect_FaxTone == 1)
            {
                pV27->Detect_FaxTone = 2;

                SetReg(pDpcs->MFSR0, FAX_DETECT);
            }

            qdEnergy = GetReg(pDpcs->ENERGY);

            for (i = 0; i < V27_BUF_SIZE; i++)
            {
                qdEnergy += QQMULQD(pV27->PCMinPtr[i], pV27->PCMinPtr[i]) >> 8;
            }

            PutReg(pDpcs->ENERGY, qdEnergy);
        }

#endif
    }
    else if (pV27->ImageSendMode == IMAGE_SEND_MODE)
    {
        V27_isrT(pV27);
    }

#if SUPPORT_V54
    else if (pV27->ImageSendMode == DIGITAL_LOOP_BACK)
    {
        V27_isrTR(pV27);
    }

#endif

    if (pV27->isTxDataMode == 1)
    {
        PutReg(pDpcs->MSR0, DATA);
    }

    if (pV27->V21DetectNumber >= 2) /* V21 signal is detected */
    {
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
        TRACE0("V27: Disconnect after V21 detected");

        /* report V21 signal detected to T30 */
        SetReg(pDpcs->MFSR0, FAX_V21);

        return;
    }

    Disc = RdReg(pDpcs->MCF, DISCONNECT);

    if (pV27->CarrLoss_Flag == 2 || pV27->sTimeOutCounter > 200 || TimeOut_flag == 1)
    {
        Disc = 1;
    }

    if (Disc)
    {
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
        TRACE0("V27: Disconnect");

        pFaxShareMem->qTimingErr1  = pV27->qTimingErr1;
        pFaxShareMem->qCarErrPhase = pV27->qCarErrPhase;
        pFaxShareMem->qFaxAgc      = pV27->qSagcScale;
    }

#if V27_HW7200SAMPLERATE_SUPPORT
    SampleRate_Change(pMultiFilterOut, pPCMout, pPCMout_temp);

    for (i = 0; i < 72; i++)
    {
        pPCMout[i] = pPCMout_temp[i];
    }

#endif
}

void V27_isrT(V27Struct *pV27)
{
    UBYTE i;

    for (i = 0; i < pV27->ubNumSym; i++)
    {
        V27_TX_Sym(pV27);
        pV27->PCMoutPtr += pV27->ubSymBufSize;
    }

    /* Buffer reset */
    pV27->PCMoutPtr -= V27_BUF_SIZE;
}


void V27_isrR(V27Struct *pV27)
{
    UBYTE i;

    for (i = 0; i < pV27->ubNumSym; i++)
    {
        V27_RX_Sym(pV27);
        pV27->PCMinPtr += pV27->ubSymBufSize;

        if (pV27->pfDataOutVec != V27_Receive_Data)
        {
            pV27->Timer++;
        }
    }

    /* Buffer reset */
    pV27->PCMinPtr -= V27_BUF_SIZE;
}

void V27_isrTR(V27Struct *pV27)
{
    UBYTE i;

    for (i = 0; i < pV27->ubNumSym; i++)
    {
        V27_TX_Sym(pV27);

        pV27->PCMoutPtr += pV27->ubSymBufSize;
    }

    /* Buffer reset */
    pV27->PCMoutPtr -= V27_BUF_SIZE;

    for (i = 0; i < V27_BUF_SIZE; i++)
    {
        pV27->PCMinPtr[i] = pV27->PCMoutPtr[i];
    }

    /* receive one buffer at a time */
    for (i = 0; i < pV27->ubNumSym; i++)
    {
        V27_RX_Sym(pV27);
        pV27->PCMinPtr += pV27->ubSymBufSize;
    }

    /* Buffer reset */
    pV27->PCMinPtr -= V27_BUF_SIZE;
}

void V27_TX_Sym(V27Struct *pV27)
{
    /* check for next state */
    if (pV27->nTxDelayCnt <= 0)
    {
        pV27->nTxState++;

#if V27_SHORT_TRAIN_SUPPORT

        if (pV27->Short_Trn_Flag == 1)
        {
            pV27->nTxDelayCnt = V27_TxDelayTable_ShortTrain[pV27->nTxState];
        }
        else
#endif
        {
            if (pV27->ubRateSig == V27_4800)
            {
                pV27->nTxDelayCnt = V27_TxDelayTable_4800[pV27->nTxState];
            }
            else
            {
                pV27->nTxDelayCnt = V27_TxDelayTable_2400[pV27->nTxState];
            }
        }

        pV27->pfTxVecptr[pV27->nTxState](pV27);
    }

    /* call TX functions  */
    pV27->pfDataInVec(pV27);
    pV27->pfScramVec(pV27);
    pV27->pfEncodeVec(pV27);
    V27_TX_Psf(pV27);
    V27_TX_Modulate(pV27);

#if V27_SHORT_TRAIN_SUPPORT

    if (pV27->Short_Trn_Flag == 1)
    {
        if ((pV27->ubRateSig == V27_4800) && (pV27->nTxState == 0) && (pV27->nTxDelayCnt == 12))
        {
            pV27->isTxDataMode = 1;
        }

        if ((pV27->ubRateSig == V27_2400) && (pV27->nTxState == 1) && (pV27->nTxDelayCnt == 28))
        {
            pV27->isTxDataMode = 1;
        }
    }
    else
#endif
    {
        if ((pV27->ubRateSig == V27_4800) && (pV27->nTxState == 3) && (pV27->nTxDelayCnt == 120))
        {
            pV27->isTxDataMode = 1;
        }

        if ((pV27->ubRateSig == V27_2400) && (pV27->nTxState == 3) && (pV27->nTxDelayCnt == 100))
        {
            pV27->isTxDataMode = 1;
        }
    }

    /* decrement delay counter by one symbol */
    pV27->nTxDelayCnt--;
}


void V27_RX_Sym(V27Struct *pV27)
{
    /* check for next state */
    if (pV27->nRxDelayCnt <= 0)
    {
        /* go to next state */
        pV27->nRxState++;

#if V27_SHORT_TRAIN_SUPPORT

        if (pV27->Short_Trn_Flag == 1)
        {
            pV27->nRxDelayCnt = V27_RxDelayTable_ShortTrain[pV27->nRxState];
        }
        else
#endif
        {
            if (pV27->ubRateSig == V27_4800)
            {
                pV27->nRxDelayCnt = V27_RxDelayTable_4800[pV27->nRxState];
            }
            else
            {
                pV27->nRxDelayCnt = V27_RxDelayTable_2400[pV27->nRxState];
            }
        }

        TRACE2("V27: Receive change period: %d  time=%d", pV27->nRxState , pV27->nRxDelayCnt);

        pV27->pfRxVecptr[pV27->nRxState](pV27);
    }

    V27_Demodulate(pV27);

    if (pV27->ubSagc_Flag == 1)
    {
        V27_CalSagcScaleEnergy(pV27);
    }

    pV27->pfDetectVec(pV27);
    pV27->pfTimingVec(pV27);

#if 0

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV27->Poly.ubTiming_Found; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV27->nRxDelayCnt; }

#endif

    while (pV27->Poly.ubTiming_Found)
    {
#if 0

        if (DumpTone1_Idx < 500000) { DumpTone1[DumpTone1_Idx++] = pV27->nRxDelayCnt; }

        if (DumpTone2_Idx < 500000) { DumpTone2[DumpTone2_Idx++] = pV27->Poly.ubTiming_Found; }

        if (DumpTone3_Idx < 500000) { DumpTone3[DumpTone3_Idx++] = pV27->nRxState; }

        if (DumpTone4_Idx < 500000) { DumpTone4[DumpTone4_Idx++] = pV27->qCarErrPhase; }

        if (DumpTone5_Idx < 500000) { DumpTone5[DumpTone5_Idx++] = pV27->cqEqErrorIQ.r; }

        if (DumpTone6_Idx < 500000) { DumpTone6[DumpTone6_Idx++] = pV27->cqEqErrorIQ.i; }

        if (DumpTone7_Idx < 500000) { DumpTone7[DumpTone7_Idx++] = pV27->Poly.nTimingIdx; }

        if (DumpTone8_Idx < 500000) { DumpTone8[DumpTone8_Idx++] = pV27->qTimingErr1; }

#endif

        pV27->pfEqVec(pV27);
        pV27->pfRotateVec(pV27);
        pV27->pfTrainScramVec(pV27);
        pV27->pfTrainEncodeVec(pV27);
        pV27->pfSliceVec(pV27);
        pV27->pfTimUpdateVec(pV27);
        pV27->pfDerotateVec(pV27);
        pV27->pfCarrierVec(pV27);
        pV27->pfEqUpdateVec(pV27);
        pV27->pfDecodeVec(pV27);
        pV27->pfDescramVec(pV27);
        pV27->pfDataOutVec(pV27);
        pV27->pfTimingVec(pV27);

        /* decrement RX delay counter */
        pV27->nRxDelayCnt--;
    }

    if (pV27->pfDataOutVec == V27_Receive_Data && pV27->CarrLoss_Flag < 2)
    {
        V27_CarrLoss_Detect(pV27);
    }
}
