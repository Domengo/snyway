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

#include "v29ext.h"
#include "hwapi.h"
#include "mhsp.h"
#include "ptable.h"

void V29_isr(UBYTE **pTable)
{
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);
#if DRAWEPG
    SWORD *pEpg    = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    SWORD *pPCMin  = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    V29Struct *pV29 = (V29Struct *)(pTable[V29_STRUC_IDX]);
    UBYTE i, Disc;

#if DRAWEPG
    *pEpg++ = 1;

    if (pV29->ImageSendMode == IMAGE_SEND_MODE)
    {
        *pEpg++ = pV29->cqSigMapIQ.r;
        *pEpg++ = pV29->cqSigMapIQ.i;
    }
    else
    {
        *pEpg++ = pV29->cqRotateIQ.r;
        *pEpg++ = pV29->cqRotateIQ.i;
    }

#endif

    pV29->PCMinPtr  = pPCMin;
    pV29->PCMoutPtr = pPCMout;

    for (i = 0; i < V29_BUF_SIZE; i++)
    {
        pPCMout[i] = 0;
    }

    V29_isr_main(pV29);

    Disc = RdReg(pDpcs->MCF, DISCONNECT);

    if (pV29->V21DetectNumber >= 2) /* V21 signal is detected */
    {
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
        TRACE0("V29: Disconnect after V21 detected");

        /* report V21 signal detected to T30 */
        SetReg(pDpcs->MFSR0, FAX_V21);

        return;
    }

    if (pV29->CarrLoss_Flag == 2)
    {
        Disc = 1;
        TRACE0("V29: Carrier lost Detected");
    }

    if (pV29->sTimeOutCounter > 2000)
    {
        pV29->sTimeOutCounter = 0;
        Disc = 1;
    }

    if (Disc)
    {
        /* jump to disconnect state */
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
        TRACE0("V29: Disconnect");

        pFaxShareMem->qTimingErr1  = pV29->qTimingErr1;
        pFaxShareMem->qCarErrPhase = QD16Q(pV29->qdCarErrPhase);
        pFaxShareMem->qFaxAgc      = pV29->qSagcScale;
    }
}


void V29_isr_main(V29Struct *pV29)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV29->pTable[DPCS_STRUC_IDX]);
    UBYTE i;
#if SUPPORT_ENERGY_REPORT
    QDWORD qdEnergy;
#endif
#if WITH_DC
    DCStruct *pDC = (DCStruct *)(pV29->pTable[DC_STRUC_IDX]);

    DcEstimator(pDC, pV29->PCMinPtr, V29_BUF_SIZE, -9);/* Low pass DC estimator filter */
#endif

    if (pV29->ImageSendMode == IMAGE_SEND_MODE)
    {
        for (i = 0; i < V29_NUM_SYM; i++)
        {
            V29_TX_Sym(pV29);
            pV29->PCMoutPtr += V29_SYM_SIZE;
        }

        pV29->PCMoutPtr -= V29_BUF_SIZE;
    }
    else if (pV29->ImageSendMode == IMAGE_RECEIVE_MODE)
    {
        for (i = 0; i < V29_NUM_SYM; i++)
        {
            V29_RX_Sym(pV29);
            pV29->PCMinPtr += V29_SYM_SIZE;
        }

        pV29->PCMinPtr -= V29_BUF_SIZE;/* Buffer reset */

#if SUPPORT_ENERGY_REPORT

        if ((pV29->Detect_FaxTone != 0) || (pV29->V21DetectNumber >= 2))
        {
            if (pV29->Detect_FaxTone == 1)
            {
                pV29->Detect_FaxTone = 2;

                SetReg(pDpcs->MFSR0, FAX_DETECT);
            }

            qdEnergy = GetReg(pDpcs->ENERGY);

            for (i = 0; i < V29_BUF_SIZE; i++)
            {
                qdEnergy += QQMULQD(pV29->PCMinPtr[i], pV29->PCMinPtr[i]) >> 8;
            }

            PutReg(pDpcs->ENERGY, qdEnergy);
        }

#endif
    }

#if SUPPORT_V54
    else if (pV29->ImageSendMode == DIGITAL_LOOP_BACK)
    {
        for (i = 0; i < V29_NUM_SYM; i++)
        {
            V29_TX_Sym(pV29);
            pV29->PCMoutPtr += V29_SYM_SIZE;
        }

        pV29->PCMoutPtr -= V29_BUF_SIZE;/* Buffer reset */

        for (i = 0; i < V29_BUF_SIZE; i++)
        {
            pV29->PCMinPtr[i] = pV29->PCMoutPtr[i];
        }

        for (i = 0; i < V29_NUM_SYM; i++)
        {
            V29_RX_Sym(pV29);
            pV29->PCMinPtr += V29_SYM_SIZE;
        }

        pV29->PCMinPtr -= V29_BUF_SIZE;/* Buffer reset */
    }

#endif

    if (pV29->isTxDataMode == 1)
    {
        PutReg(pDpcs->MSR0, DATA);
    }
}

void V29_TX_Sym(V29Struct *pV29)
{
    /* check for next state */
    if (pV29->nTxDelayCnt <= 0)
    {
        pV29->nTxState++;

        pV29->nTxDelayCnt = V29_TxDelayTable[pV29->nTxState];

        pV29->pfTxVecptr[pV29->nTxState](pV29);
    }

    /* call TX functions  */
    pV29->pfDataInVec(pV29);
    pV29->pfScramVec(pV29);
    pV29->pfEncodeVec(pV29);
    V29_TX_Psf(pV29);
    V29_TX_Modulate(pV29);

    if ((pV29->nTxState == 3) && (pV29->nTxDelayCnt == 360))/* Modified for TEP */
    {
        pV29->isTxDataMode = 1;
    }

    /* decrement delay counter by one symbol */
    pV29->nTxDelayCnt--;
}

void V29_RX_Sym(V29Struct *pV29)
{
    /* check for next state */
    if (pV29->nRxDelayCnt <= 0)
    {
        /* go to next state */
        pV29->nRxState++;
        pV29->nRxDelayCnt = V29_RxDelayTable[pV29->nRxState];
        TRACE2("V29: Receive change period: %d  time=%d", pV29->nRxState , pV29->nRxDelayCnt);

        pV29->pfRxVecptr[pV29->nRxState](pV29);
    }

    V29_Demodulate(pV29);

    if (pV29->ubSagc_Flag == 1)
    {
        V29_CalSagcScaleEnergy(pV29);
    }

    pV29->pfDetectVec(pV29);
    pV29->pfTimingVec(pV29);

#if 0

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV29->Poly.ubTiming_Found; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV29->nRxDelayCnt; }

#endif

    while (pV29->Poly.ubTiming_Found)
    {
#if 0

        if (DumpTone1_Idx < 500000) { DumpTone1[DumpTone1_Idx++] = pV29->cqRotateIQ.r; }

        if (DumpTone2_Idx < 500000) { DumpTone2[DumpTone2_Idx++] = pV29->cqRotateIQ.i; }

        if (DumpTone3_Idx < 500000) { DumpTone3[DumpTone3_Idx++] = pV29->nRxState; }

        if (DumpTone4_Idx < 500000) { DumpTone4[DumpTone4_Idx++] = pV29->qCarErrPhase; }

        if (DumpTone5_Idx < 500000) { DumpTone5[DumpTone5_Idx++] = pV29->cqEqErrorIQ.r; }

        if (DumpTone6_Idx < 500000) { DumpTone6[DumpTone6_Idx++] = pV29->cqEqErrorIQ.i; }

        if (DumpTone7_Idx < 500000) { DumpTone7[DumpTone7_Idx++] = pV29->Poly.nTimingIdx; }

        if (DumpTone8_Idx < 500000) { DumpTone8[DumpTone8_Idx++] = pV29->qTimingErr1; }

#endif
        pV29->pfEqVec(pV29);
        pV29->pfRotateVec(pV29);
        pV29->pfTrainScramVec(pV29);
        pV29->pfTrainEncodeVec(pV29);
        pV29->pfSliceVec(pV29);
        pV29->pfTimUpdateVec(pV29);
        pV29->pfDerotateVec(pV29);
        pV29->pfCarrierVec(pV29);
        pV29->pfEqUpdateVec(pV29);
        pV29->pfDecodeVec(pV29);
        pV29->pfDescramVec(pV29);
        pV29->pfDataOutVec(pV29);
        pV29->pfTimingVec(pV29);

        /* decrement RX delay counter */
        pV29->nRxDelayCnt--;
    }

    if (pV29->pfDataOutVec == V29_Receive_DataMode && pV29->CarrLoss_Flag < 2)
    {
        V29_CarrLoss_Detect(pV29);
    }
}
