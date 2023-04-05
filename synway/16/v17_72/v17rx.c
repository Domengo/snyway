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

void V17_SetUpDetToneFn(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    pV17->pfDetectVec           = V17_Dummy;
    pV32Share->pfEqVec          = V32_Eq_D;
    pV17->pfRotateVec           = V17_Dummy;
    pV32Share->pfTrainScramVec  = V32Share_Dummy;
    pV32Share->pfTrainSigMapVec = V32Share_Dummy;
    pV32Share->pfSliceVec       = V32_Slice;
    pV32Share->pfTimUpdateVec   = V32Share_Dummy;
    pV32Share->pfDerotateVec    = V32Share_Dummy;
    pV32Share->pfCarrierVec     = V32Share_Dummy;
    pV32Share->pfEqUpdateVec    = V32Share_Dummy;
    pV32Share->pfDecodeVec      = V32Share_Dummy;
    pV32Share->pfDiffDecVec     = V32_BypassDiffDec;
    pV32Share->pfDescramVec     = V32Share_Dummy;
}

void V17_SetUpTrnFn(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    pV17->pfDetectVec           = V17_Dummy;
    pV32Share->pfEqVec          = V32_Eq_D;  /* data mode equalizer */
    pV17->pfRotateVec           = V17_Rotate;
    pV32Share->pfTrainSigMapVec = V32Share_Dummy;
    pV32Share->pfSliceVec       = V32_Slice;
    pV32Share->pfTimUpdateVec   = V32_TimingUpdateBySliceIQ;
    pV32Share->pfDerotateVec    = V32_Derotate;
    pV32Share->pfCarrierVec     = V32_CarUpdateUseSliceIQ;
    pV32Share->pfEqUpdateVec    = V32Share_Dummy;
    pV32Share->pfDecodeVec      = V32Share_Dummy;
    pV32Share->pfDiffDecVec     = V32_BypassDiffDec;
}

void V17_Rx_Atone_or_ABtone(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    pV17->ubGuardToneCnt = 0;
    pV17->ubABtoneCnt    = 0;

    V17_SetUpDetToneFn(pV17);

    if (pV17->ubTraintype == V17_LONG_TRAIN)
    {
        pV17->pfDetectVec = V17_AutoCorr_Detect;
    }

    pV32Share->pfEqVec      = V32Share_Dummy;
    pV32Share->pfSliceVec   = V32Share_Dummy;
    pV32Share->pfDiffDecVec = V32Share_Dummy;
    pV17->pfDataOutVec      = V17_Guard_or_ABtone_Detect;
}

void V17_Rx_Segm1AB_after_A(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    pV17->ubGuardToneCnt = 0;
    pV17->ubABtoneCnt    = 0;

    V17_SetUpDetToneFn(pV17);

    if (pV17->ubTraintype == V17_LONG_TRAIN)
    {
        pV17->pfDetectVec = V17_AutoCorr_Detect;
    }

    pV32Share->pfSliceVec   = V32Share_Dummy;
    pV32Share->pfDiffDecVec = V32Share_Dummy;

    pV17->pfDataOutVec    = V17_AB_Detect_after_Guardtone;
}

void V17_Rx_Segm1_Long(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    V17_SetUpDetToneFn(pV17);

    pV17->pfDetectVec           = V17_AutoCorr_Detect;
    pV32Share->pfEqVec          = V32_BypassEq;

    pV17->pfRotateVec           = V17_BypassRotate;
    pV32Share->pfTrainSigMapVec = V17_RX_S_TrainSigMap;
    pV17->pfDataOutVec          = V17_RX_DataOut_Seg1L;
}

void V17_Rx_Segm1_Short(V17Struct *pV17)
{
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pV17->pTable[FAXSHAREMEM_IDX]);
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    UBYTE i;

    for (i = 0; i < V32_EQ_LENGTH; i++)
    {
        pV32Share->cEqCoef_D[i] = pFaxShareMem->cEqCoef_ago[i];
    }

    V17_SetUpDetToneFn(pV17);

    pV32Share->pfTrainSigMapVec = V17_RX_S_TrainSigMap;
    pV17->pfDataOutVec          = V17_RX_DataOut_Seg1S;
    pV32Share->pfEqVec          = V17_SyncEq;
    pV17->pfDetectVec           = V17_SyncDemod;
    pV17->sbSyncCounter            = V17_SYNC_DEMOD_SYMBOLS;

    pV32Share->ubTimErr1Freeze  = 1;
}

void V17_Rx_Segm2_Long(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    V32_DescramMask_Init(pV32Share);

    V17_SetUpDetToneFn(pV17);

    pV17->pfRotateVec          = V17_Rotate;
    pV32Share->pfDerotateVec   = V32_Derotate_Get_TRN;
    pV17->pfDataOutVec         = V17_RX_DataOut_Seg2L;
    pV32Share->ubTimErr1Freeze = 0;
    pV32Share->qTimingErr1     = 0;
}

void V17_Rx_Segm2_Short(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    V32_DescramMask_Init(pV32Share);

    V17_SetUpDetToneFn(pV17);

    pV17->pfRotateVec         = V17_Rotate;

    if (pV17->timingDelta == 0)
    {
        pV32Share->pfTimUpdateVec = V32_TimingUpdateBySliceIQ;
        pV32Share->qEqBeta        = V17_EQ_FINE_BETA;
        pV32Share->pfCarrierVec   = V32_CarUpdateUseSliceIQ;
    }
    else
    {
        /* Disable timing/carrier/EQ update to find best index */
        pV32Share->qEqBeta        = 0;
    }

    pV32Share->pfDerotateVec  = V32_Derotate;

    pV32Share->pfEqUpdateVec  = V32_EqUpdate;

    pV17->pfDataOutVec        = V17_RX_DataOut_Seg2S;
}

void V17_Rx_Segm3_Long(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    V32_DescramMask_Init(pV32Share);

    V17_SetUpTrnFn(pV17);

    pV32Share->pfTrainScramVec  = V32Share_Dummy;

    pV32Share->qEqBeta          = V17_EQ_FINE_BETA;
    pV32Share->pfEqUpdateVec    = V32_EqUpdate;

    pV32Share->pfDiffDecVec     = V32_DiffDec;
    pV17->pfDataOutVec          = V17_RX_DataOut_Seg3;
}

void V17_Rx_Segm4(V17Struct *pV17)
{
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pV17->pTable[FAXSHAREMEM_IDX]);
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    UBYTE i;

    if (pV17->ubTraintype == V17_LONG_TRAIN)
    {
        for (i = 0; i < V32_EQ_LENGTH; i++)
        {
            pFaxShareMem->cEqCoef_ago[i] = pV32Share->cEqCoef_D[i];
        }
    }

    V32_DescramMask_Init(pV32Share);

    V17_SetUpTrnFn(pV17);

    pV32Share->pfTrainScramVec   = V32Share_Dummy;

    if (pV17->ubTraintype == V17_LONG_TRAIN)
    {
        pV32Share->qEqBeta           = V17_EQ_ADAPT_BETA;

#if V32_FREQUENCY_OFFSET
        pV32Share->ubRotateCountEnable = 1;
#endif
    }
    else
    {
        pV32Share->qEqBeta          = V17_EQ_FINE_BETA;
    }

    pV32Share->pfEqUpdateVec     = V32_EqUpdate;

    pV32Share->pfDecodeVec       = V32_RX_Decode128p;
    pV32Share->pfDiffDecVec      = V32_DiffDec;
    pV17->pfDataOutVec           = V17_RX_DataOut_Seg4;
}

void V17_Rx_SegmData(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    V32_DescramMask_Init(pV32Share);

    pV32Share->ubTimErr1Freeze = 0;

    V17_SetUpTrnFn(pV17);

    pV32Share->pfTrainScramVec  = V32Share_Dummy;

    pV32Share->qEqBeta = V17_EQ_DATA_BETA;
    pV32Share->pfEqUpdateVec = V32_EqUpdate;

    pV32Share->pfDecodeVec   = V32_RX_Decode128p;
    pV32Share->pfDiffDecVec  = V32_DiffDec;
    pV32Share->pfDescramVec  = V32_DescramUsingGPC;
    pV17->pfDataOutVec       = V17_RX_DataOut_DataMode;

#if 0 /* EQ Coef */

    for (i = 0; i < V32_EQ_LENGTH; i++)
    {
        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV32Share->cEqCoef_D[i].r; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV32Share->cEqCoef_D[i].i; }
    }

#endif
}
