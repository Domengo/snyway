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

/**********************************************************************/
/* file : V27RX.C                                                     */
/* This file initilises all the vectors for V27 receiver.             */
/*                                                                    */
/* Author : Sella Maheswaran, GAO Research & Consulting Ltd.          */
/* Date   : February 17,1997                                          */
/*                                                                    */
/**********************************************************************/

#include "v27ext.h"
#include "ptable.h"

void V27_Rx_Segm1_Carrier(V27Struct *pV27)
{
    TRACE0("V27: Segment 1: ");
    pV27->DescramSReg = 0;

    pV27->pfDetectVec       = V27_Dummy;
    pV27->pfTimingVec       = V27_RX_Timing_Dummy;
    pV27->pfEqVec           = V27_EqSkipFilter;
    pV27->pfRotateVec       = V27_RX_Rotate_Dummy;
    pV27->pfTrainScramVec   = V27_Dummy;
    pV27->pfTrainEncodeVec  = V27_Dummy;
    pV27->pfSliceVec        = V27_Dummy;
    pV27->pfTimUpdateVec    = V27_Dummy;
    pV27->pfDerotateVec     = V27_Dummy;
    pV27->pfCarrierVec      = V27_Dummy;
    pV27->pfEqUpdateVec     = V27_Dummy;
    pV27->pfDecodeVec       = V27_Dummy;
    pV27->pfDescramVec      = V27_Dummy;
    pV27->pfDataOutVec      = V27_Guard_or_ABtone_Detect;
}

/* Unmodulated carrier    */
void V27_Get_Segment_UnMod_Carrier(V27Struct *pV27)
{
    pV27->DescramSReg       = 0;

    pV27->pfDetectVec       = V27_Dummy;
    pV27->pfTimingVec       = V27_RX_Timing_Dummy;
    pV27->pfEqVec           = V27_EqSkipFilter;
    pV27->pfRotateVec       = V27_RX_Rotate_Dummy;
    pV27->pfTrainScramVec   = V27_Dummy;
    pV27->pfTrainEncodeVec  = V27_Dummy;
    pV27->pfSliceVec        = V27_Dummy;
    pV27->pfTimUpdateVec    = V27_Dummy;
    pV27->pfDerotateVec     = V27_Dummy;
    pV27->pfCarrierVec      = V27_Dummy;
    pV27->pfEqUpdateVec     = V27_Dummy;
    pV27->pfDecodeVec       = V27_Dummy;
    pV27->pfDescramVec      = V27_Dummy;
    pV27->pfDataOutVec      = V27_Dummy;
}

void V27_Rx_Segm2_Silent(V27Struct *pV27)
{
    TRACE0("V27: Segment 2: ");
    pV27->DescramSReg = 0;

    pV27->pfDetectVec       = V27_Dummy;
    pV27->pfTimingVec       = V27_RX_Timing_Dummy;
    pV27->pfEqVec           = V27_EqSkipFilter;
    pV27->pfRotateVec       = V27_RX_Rotate_Dummy;
    pV27->pfTrainScramVec   = V27_Dummy;
    pV27->pfTrainEncodeVec  = V27_Dummy;
    pV27->pfSliceVec        = V27_Dummy;
    pV27->pfTimUpdateVec    = V27_Dummy;
    pV27->pfDerotateVec     = V27_Dummy;
    pV27->pfCarrierVec      = V27_Dummy;
    pV27->pfEqUpdateVec     = V27_Dummy;
    pV27->pfDecodeVec       = V27_Dummy;
    pV27->pfDescramVec      = V27_Dummy;
    pV27->pfDataOutVec      = V27_AB_Detect_after_Guardtone;
}

/* 180 Phase reversals */
void V27_Rx_Segm3_180_Phase(V27Struct *pV27)
{
    TRACE0("V27: Segment 3: ");
    pV27->DescramSReg = 0;            /* load register with 0101010 */

    pV27->pfDetectVec        = V27_AutoCorr_Detect;
    pV27->pfTimingVec        = V27_RX_Timing_Dummy;
    pV27->pfEqVec            = V27_Eq;
    pV27->pfRotateVec        = V27_RX_Rotate_Dummy;
    pV27->pfTrainScramVec    = V27_Dummy;
    pV27->pfTrainEncodeVec   = V27_Dummy;
    pV27->pfSliceVec         = V27_Dummy;
    pV27->pfTimUpdateVec     = V27_Dummy;
    pV27->pfDerotateVec      = V27_Dummy;
    pV27->pfCarrierVec       = V27_Dummy;
    pV27->pfEqUpdateVec      = V27_Dummy;
    pV27->pfDecodeVec        = V27_Dummy;
    pV27->pfDescramVec       = V27_Dummy;
    pV27->pfDataOutVec       = V27_Receive_180_Phase;
}

void V27_Segment_Eq_delay(V27Struct *pV27)
{
    pV27->pfDataOutVec       = V27_Dummy;
    pV27->pfDetectVec        = V27_Dummy;
}

/* Equalizer conditioning segment */
/****  Get TRN1 - raw training ****/
void V27_Segment_0_180_Phase_Raw(V27Struct *pV27)
{
    TRACE0("V27: Segment 4(1): ");

    pV27->TrainScramSReg = 0x0000003C;
    pV27->sbPast_phase    = 0;

    /* Setup RX functions */
    pV27->pfDetectVec        = V27_Dummy;
    pV27->pfTimingVec        = V27_TimingRecovery;
    pV27->pfEqVec            = V27_Eq;
    pV27->pfRotateVec        = V27_RX_Rotate;
    pV27->pfTrainScramVec    = V27_RX_TrainScrambler; /* CALL or ANS */
    pV27->pfTrainEncodeVec   = V27_RX_TrainEncoder;
    pV27->pfSliceVec         = V27_RX_Slicer;
    pV27->pfTimUpdateVec     = V27_RX_TimingUpdate_Train_fine;
    pV27->pfDerotateVec      = V27_RX_Derotate_Get_TRN;
    pV27->pfCarrierVec       = V27_CarUpdateUsingTrainIQ;

    pV27->qEqBeta            = V27_EQ_TRN1raw_BETA;
    pV27->pfEqUpdateVec      = V27_RX_EqUpdate;

    pV27->pfDecodeVec        = V27_RX_Decoder;
    pV27->pfDescramVec       = V27_RX_Descram;
    pV27->pfDataOutVec       = V27_Dummy;

    pV27->ubBitsPerSym = 3;
}

/***** Get TRN2 - fine training ******/
void V27_Segment_0_180_Phase_Fine(V27Struct *pV27)
{
    TRACE0("V27: Segment 4(2): ");

#if V27_SHORT_TRAIN_SUPPORT

    if (pV27->Short_Trn_Flag == 1)
    {
        pV27->TrainScramSReg = 0x3c000000;
        pV27->qEqBeta   = V27_EQ_TRN2fine_BETA;   /* fine 0.05 (1638) */
    }
    else
#endif
        pV27->qEqBeta   =  V27_EQ_SHORT_BETA;

    pV27->pfEqUpdateVec      = V27_RX_EqUpdate;

    pV27->pfDetectVec        = V27_Dummy;
    pV27->pfTimingVec        = V27_TimingRecovery;
    pV27->pfEqVec            = V27_Eq;
    pV27->pfRotateVec        = V27_RX_Rotate;
    pV27->pfTrainScramVec    = V27_RX_TrainScrambler; /* CALL or ANS */
    pV27->pfTrainEncodeVec   = V27_RX_TrainEncoder;
    pV27->pfSliceVec         = V27_RX_Slicer;
    pV27->pfTimUpdateVec     = V27_RX_TimingUpdate_Train_fine;
    pV27->pfDerotateVec      = V27_RX_Derotate_Get_TRN;
    pV27->pfCarrierVec       = V27_CarUpdateUsingTrainIQ;

    pV27->pfDecodeVec        = V27_RX_Decoder;
    pV27->pfDescramVec       = V27_RX_Descram;
    pV27->pfDataOutVec       = V27_Dummy;
}

/* all scrambled binary ONE's segment */
void V27_Get_Segment_Scrambled_One(V27Struct *pV27)
{
    if (pV27->ubRateSig == V27_4800)
    {
        pV27->ubBitsPerSym = 3;
    }
    else
    {
        pV27->ubBitsPerSym = 2;
    }

    pV27->pfDetectVec      = V27_Dummy;
    pV27->pfTimingVec      = V27_TimingRecovery;
    pV27->pfEqVec          = V27_Eq;
    pV27->pfRotateVec      = V27_RX_Rotate;
    pV27->pfTrainScramVec  = V27_Dummy;
    pV27->pfTrainEncodeVec = V27_Dummy;
    pV27->pfSliceVec       = V27_RX_Slicer;
    pV27->pfTimUpdateVec   = V27_RX_TimingUpdate;
    pV27->pfDerotateVec    = V27_RX_Derotate;
    pV27->pfCarrierVec     = V27_CarUpdateUsingSliceIQ;

    pV27->qEqBeta          = V27_EQ_ADAPT_BETA;  /* 0.075 */
    pV27->pfEqUpdateVec    = V27_RX_EqUpdate;

    pV27->pfDecodeVec      = V27_RX_Decoder;
    pV27->pfDescramVec     = V27_RX_Descram;
    pV27->pfDataOutVec     = V27_Dummy;
}

/* data mode section */
void V27_Get_Data(V27Struct *pV27)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV27->pTable[DPCS_STRUC_IDX]);

    PutReg(pDpcs->MSR0, DATA);

    pV27->pfDetectVec      = V27_Dummy;
    pV27->pfTimingVec      = V27_TimingRecovery;
    pV27->pfEqVec          = V27_Eq;
    pV27->pfRotateVec      = V27_RX_Rotate;
    pV27->pfTrainScramVec  = V27_Dummy;
    pV27->pfTrainEncodeVec = V27_Dummy;
    pV27->pfSliceVec       = V27_RX_Slicer;
    pV27->pfTimUpdateVec   = V27_RX_TimingUpdate;
    pV27->pfDerotateVec    = V27_RX_Derotate;
    pV27->pfCarrierVec     = V27_CarUpdateUsingSliceIQ;

#if 1
    pV27->pfEqUpdateVec    = V27_Dummy;
#else
    pV27->qEqBeta          = 192;
    pV27->pfEqUpdateVec    = V27_RX_EqUpdate;
#endif

    pV27->pfDecodeVec      = V27_RX_Decoder;
    pV27->pfDescramVec     = V27_RX_Descram;
    pV27->pfDataOutVec     = V27_Receive_Data;

#if 0 /* EQ Coef */

    for (i = 0; i < V27_EQ_LENGTH; i++)
    {
        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV27->cEqCoef[i].r; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV27->cEqCoef[i].i; }
    }

#endif

}
