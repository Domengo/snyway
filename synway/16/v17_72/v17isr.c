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

#include "v17ext.h"
#include "ptable.h"

void V17_isr(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    UBYTE   i, j;
#if SUPPORT_ENERGY_REPORT
    DpcsStruct *pDpcs = (DpcsStruct *)(pV17->pTable[DPCS_STRUC_IDX]);
    QDWORD qdEnergy;
#endif
#if DRAWEPG
    SWORD pt_loc;
    SWORD *pEpg = (SWORD *)(pV17->pTable[EPGDATA_IDX]);
#endif
#if WITH_DC
    DCStruct *pDC = (DCStruct *)(pV17->pTable[DC_STRUC_IDX]);

    /* Low pass DC estimator filter */
    DcEstimator(pDC, pV32Share->PCMinPtr, V32_BUF_SIZE, -9);
#endif

#if DRAWEPG
    pt_loc   = *pEpg;
    *pEpg++ += 1;
    pEpg    += (pt_loc << 1);

    if (pV17->ImageSendMode == IMAGE_SEND_MODE)
    {
        *pEpg++ = pV32Share->cqSigMapIQ.r << 1;
        *pEpg++ = pV32Share->cqSigMapIQ.i << 1;
    }
    else
    {
        *pEpg++ = pV32Share->cqRotateIQ.r << 2;
        *pEpg++ = pV32Share->cqRotateIQ.i << 2;
    }

#endif

    /* transmit and receive one buffer */
    for (i = 0; i < V32_NUM_SYM; i++)
    {
        if (pV17->ImageSendMode == IMAGE_SEND_MODE)
        {
            V17_TX_Sym(pV17);
        }
        else if (pV17->ImageSendMode == IMAGE_RECEIVE_MODE)
        {
            for (j = 0; j < V32_SYM_SIZE; j++)
            {
                pV32Share->qDemodIn[j] = pV32Share->PCMinPtr[j];
            }

            V17_RX_Sym(pV17);
        }

#if SUPPORT_V54
        else
        {
            V17_TX_Sym(pV17);

            if (pV17->ImageSendMode == DIGITAL_LOOP_BACK)
            {
                for (j = 0; j < V32_SYM_SIZE; j++)
                {
                    pV32Share->qDemodIn[j] = pV32Share->PCMoutPtr[j];
                }
            }

            V17_RX_Sym(pV17);
        }

#endif

        pV32Share->PCMinPtr  += V32_SYM_SIZE;
        pV32Share->PCMoutPtr += V32_SYM_SIZE;
    }

    /* reset buffer pointers */
    pV32Share->PCMoutPtr -= V32_BUF_SIZE;
    pV32Share->PCMinPtr  -= V32_BUF_SIZE;

#if SUPPORT_ENERGY_REPORT

    if ((pV17->Detect_FaxTone != 0) || (pV17->V21DetectNumber >= 2))
    {
        if (pV17->Detect_FaxTone == 1)
        {
            pV17->Detect_FaxTone = 2;
            SetReg(pDpcs->MFSR0, FAX_DETECT);
        }

        qdEnergy = 0;

        for (i = 0; i < V32_BUF_SIZE; i++)
        {
            qdEnergy += QQMULQD(pV32Share->PCMinPtr[i], pV32Share->PCMinPtr[i]) >> 8;
        }

        /**** CHANGE IN CARRIER LEVEL DETECTION ******/
        pV17->qdEnergyAcc += qdEnergy;
        pV17->uEnergyCount++;

        if (pV17->uEnergyCount == 4)
        {

            if ((pV17->qdEnergyMax == 0) && (pV17->qdEnergyMin == 0))
            {
                pV17->qdEnergyMax = pV17->qdEnergyAcc;
                pV17->qdEnergyMin = pV17->qdEnergyAcc;
            }
            else if (pV17->qdEnergyAcc > pV17->qdEnergyMax)
            {
                pV17->qdEnergyMax = pV17->qdEnergyAcc;
            }
            else if (pV17->qdEnergyAcc < pV17->qdEnergyMin)
            {
                pV17->qdEnergyMin = pV17->qdEnergyAcc;
            }

            pV17->qdEnergyAcc = 0;
            pV17->uEnergyCount = 0;

            if ((pV17->qdEnergyMax > ((pV17->qdEnergyMin) << 3)))
            {
                pV17->CarrLoss_Flag = 2;
                TRACE0("V17: Change of Carrier");
            }
        }

        /**** CHANGE IN CARRIER LEVEL DETECTION ******/

        qdEnergy += GetReg(pDpcs->ENERGY);
        PutReg(pDpcs->ENERGY, qdEnergy);
    }

#endif
}

void V17_TX_Sym(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    if (pV32Share->nTxDelayCnt <= 0)
    {
        /* go to next state, set up delay counter */
        pV32Share->nTxState++;
        pV17->pfTxSetUpVec = pV17->pfTxSetUpVecTab[pV32Share->nTxState];
        pV32Share->nTxDelayCnt  = pV32Share->nTxDelayCntTab[pV32Share->nTxState];
        pV17->pfTxSetUpVec(pV17);        /* init. vectors for TX */
    }

    /* call TX functions */
    pV17->pfDataInVec(pV17);
    pV32Share->pfScramVec(pV32Share);
    pV32Share->pfDiffEncVec(pV32Share);
    pV32Share->pfEncodeVec(pV32Share);
    pV32Share->pfSigMapVec(pV32Share);
    V32_Psf(pV32Share);
    V32_Mod(pV32Share);

    if (((pV32Share->nTxState == 3) && (pV32Share->nTxDelayCnt ==  80) && (pV17->ubTraintype == V17_LONG_TRAIN))
        || ((pV32Share->nTxState == 2) && (pV32Share->nTxDelayCnt == 154) && (pV17->ubTraintype == V17_SHORT_TRAIN)))
    {
        pV32Share->isTxDataMode = 1;
    }

    /* decrement TX delay counter */
    pV32Share->nTxDelayCnt--;
}

void V17_RX_Sym(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    if (pV32Share->nRxDelayCnt <= 0)
    {
        pV32Share->nRxState++;
        pV17->pfRxSetUpVec = pV17->pfRxSetUpVecTab[pV32Share->nRxState];
        pV32Share->nRxDelayCnt  = pV32Share->nRxDelayCntTab[pV32Share->nRxState];
        pV17->pfRxSetUpVec(pV17);
        TRACE1("V17: State %d", pV32Share->nRxState);
    }

    V32_Demodulate(pV32Share);

    if (pV32Share->ubSagc_Flag == 1)
    {
        V32_CalSagcScaleEnergy(pV32Share);
    }

    pV17->pfDetectVec(pV17);
    RX_TimingRecovery(&pV32Share->Poly, V32_SYM_SIZE, 3);

    while (pV32Share->Poly.ubTiming_Found)
    {
        pV32Share->pfEqVec(pV32Share);
        pV17->pfRotateVec(pV17);

#if 0

        if (DumpTone1_Idx < 500000) { DumpTone1[DumpTone1_Idx++] = pV32Share->cqRotateIQ.r; }

        if (DumpTone2_Idx < 500000) { DumpTone2[DumpTone2_Idx++] = pV32Share->cqRotateIQ.i; }

        if (DumpTone3_Idx < 500000) { DumpTone3[DumpTone3_Idx++] = pV32Share->nRxState; }

        if (DumpTone4_Idx < 500000) { DumpTone4[DumpTone4_Idx++] = pV32Share->qCarErrPhase; }

        if (DumpTone5_Idx < 500000) { DumpTone5[DumpTone5_Idx++] = pV32Share->cqEqErrorIQ.r; }

        if (DumpTone6_Idx < 500000) { DumpTone6[DumpTone6_Idx++] = pV32Share->cqEqErrorIQ.i; }

        if (DumpTone7_Idx < 500000) { DumpTone7[DumpTone7_Idx++] = pV32Share->Poly.nTimingIdx; }

        if (DumpTone8_Idx < 500000) { DumpTone8[DumpTone8_Idx++] = pV32Share->qTimingErr1; }

#endif

        pV32Share->pfTrainScramVec(pV32Share);
        V32_BypassTrainEncode(pV32Share);
        pV32Share->pfTrainSigMapVec(pV32Share);
        pV32Share->pfSliceVec(pV32Share);
        pV32Share->pfTimUpdateVec(pV32Share);
        pV32Share->pfDerotateVec(pV32Share);
        pV32Share->pfCarrierVec(pV32Share);
        pV32Share->pfEqUpdateVec(pV32Share);
        pV32Share->pfDecodeVec(pV32Share);
        pV32Share->pfDiffDecVec(pV32Share);
        pV32Share->pfDescramVec(pV32Share);
        pV17->pfDataOutVec(pV17);
        RX_TimingRecovery(&pV32Share->Poly, V32_SYM_SIZE, 3);

        /* decrement RX delay counter */
        pV32Share->nRxDelayCnt--;
    }

    if (pV32Share->isRxDataMode && pV17->CarrLoss_Flag < 2)
    {
        V17_CarrLoss_Detect(pV17);
    }
}

void V17_CarrLoss_Detect(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    QDWORD qdEgy;
    UBYTE  i;

    qdEgy = 0;

    for (i = 0; i < V32_SYM_SIZE; i++)
    {
        qdEgy += QQMULQD(pV32Share->qDemodIn[i], pV32Share->qDemodIn[i]) >> 3;
    }

    pV17->qdCarrLoss_egy += (qdEgy - pV17->qdCarrLoss_egy) >> 2;

    if (pV17->uCarrLoss_Count < 1024)
    {
        pV17->qdCarrLossEgy_Ref += (pV17->qdCarrLoss_egy >> 6);

        if (pV17->uCarrLoss_Count == 1023)
        {
            pV17->qdCarrLossEgy_Ref >>= 8;
        }

        pV17->uCarrLoss_Count++;
    }
    else
    {
#if 0

        if (DumpTone1_Idx < 500000) { DumpTone1[DumpTone1_Idx++] = pV17->qdCarrLossEgy_Ref; }

        if (DumpTone2_Idx < 500000) { DumpTone2[DumpTone2_Idx++] = pV17->qdCarrLoss_egy; }

#endif

        if (pV17->qdCarrLoss_egy < pV17->qdCarrLossEgy_Ref)
        {
            if (pV17->CarrLoss_Flag == 0)
            {
                pV17->CarrLoss_Flag  = 1;
            }
            else
            {
                pV17->qdCarrLoss_egy    = 0;
                pV17->CarrLoss_Flag     = 2;
                pV17->qdCarrLossEgy_Ref = 0;
                pV17->uCarrLoss_Count   = 0;
            }
        }
        else if (pV17->CarrLoss_Flag == 1)
        {
            pV17->CarrLoss_Flag  = 0;
        }
    }
}
