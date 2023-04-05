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

/*************************************************************************/
/* This file initialises all the vectors for V29 receiver.               */
/*************************************************************************/

#include <stdlib.h>
#include "v29ext.h"
#include "ptable.h"

/* No transmitted energy segment,silence */
void V29_Rx_Segm1_ToneDetect(V29Struct *pV29)
{
    TRACE0("V29: Segment1 ToneDetect");

    pV29->ubGuardToneCnt = 0;
    pV29->ubABtoneCnt    = 0;

    pV29->pfDetectVec      = V29_AutoCorr_Detect;
    pV29->pfTimingVec      = V29_RX_Timing_Dummy;
    pV29->pfEqVec          = V29_RX_Eq_Dummy;
    pV29->pfRotateVec      = V29_RX_Rotate_Dummy;
    pV29->pfTrainScramVec  = V29_Dummy;
    pV29->pfTrainEncodeVec = V29_Dummy;
    pV29->pfSliceVec       = V29_Dummy;
    pV29->pfDerotateVec    = V29_Dummy;
    pV29->pfCarrierVec     = V29_Dummy;
    pV29->pfTimUpdateVec   = V29_Dummy;
    pV29->pfEqUpdateVec    = V29_Dummy;
    pV29->pfDecodeVec      = V29_Dummy;
    pV29->pfDescramVec     = V29_Dummy;
    pV29->pfDataOutVec     = V29_Guard_or_ABtone_Detect;
}

void V29_Rx_Segm1_Silent(V29Struct *pV29)
{
    TRACE0("V29: Segment1 Silent");

    pV29->ubGuardToneCnt = 0;
    pV29->ubABtoneCnt    = 0;

    pV29->pfDetectVec      = V29_AutoCorr_Detect;
    pV29->pfTimingVec      = V29_RX_Timing_Dummy;
    pV29->pfEqVec          = V29_RX_Eq_Dummy;
    pV29->pfRotateVec      = V29_RX_Rotate_Dummy;
    pV29->pfTrainScramVec  = V29_Dummy;
    pV29->pfTrainEncodeVec = V29_Dummy;
    pV29->pfSliceVec       = V29_Dummy;
    pV29->pfDerotateVec    = V29_Dummy;
    pV29->pfCarrierVec     = V29_Dummy;
    pV29->pfTimUpdateVec   = V29_Dummy;
    pV29->pfEqUpdateVec    = V29_Dummy;
    pV29->pfDecodeVec      = V29_Dummy;
    pV29->pfDescramVec     = V29_Dummy;
    pV29->pfDataOutVec     = V29_AB_Detect_after_Guardtone;
}

/****************************************/
/*  alternation of two points segment   */

void V29_Rx_Segm2(V29Struct *pV29)
{
    TRACE0("V29: Segment2");

    pV29->pfDetectVec      = V29_AutoCorr_Detect;
    pV29->pfTimingVec      = V29_TimingRecovery;
    pV29->pfEqVec          = V29_RX_Eq;
    pV29->pfRotateVec      = V29_RX_Rotate_Dummy;
    pV29->pfTrainScramVec  = V29_Dummy;
    pV29->pfTrainEncodeVec = V29_Dummy;
    pV29->pfSliceVec       = V29_Dummy;
    pV29->pfDerotateVec    = V29_Dummy;
    pV29->pfCarrierVec     = V29_Dummy;
    pV29->pfTimUpdateVec   = V29_Dummy;
    pV29->pfEqUpdateVec    = V29_Dummy;
    pV29->pfDecodeVec      = V29_Dummy;
    pV29->pfDescramVec     = V29_Dummy;
    pV29->pfDataOutVec     = V29_Receive_Segment2;
}

void V29_EQ_Delay(V29Struct *pV29)
{
    pV29->pfDetectVec  = V29_Dummy;
    pV29->pfTimingVec  = V29_TimingRecovery;
    pV29->pfEqVec      = V29_RX_Eq;
    pV29->pfDataOutVec = V29_Dummy;
}

/**********************************/
/* Equalizer conditioning segment */

void V29_Rx_Segm3(V29Struct *pV29)
{
    TRACE0("V29: Segment3");
    pV29->TrainScramSReg    = (UWORD)0x54;                /* load register with 0101010 */

    pV29->pfDetectVec       = V29_Dummy;
    pV29->pfTimingVec       = V29_TimingRecovery;
    pV29->pfEqVec           = V29_RX_Eq;
    pV29->pfRotateVec       = V29_RX_Rotate;
    pV29->pfTrainScramVec   = V29_RX_TrainScrambler;
    pV29->pfTrainEncodeVec  = V29_RX_TrainEncoder;
    pV29->pfSliceVec        = V29_Dummy;
    pV29->pfDerotateVec     = V29_RX_Derotate_Get_TRN;
    pV29->pfCarrierVec      = V29_Dummy;
    pV29->pfTimUpdateVec    = V29_Dummy;

    pV29->qEqBeta           = 25000;
    pV29->pfEqUpdateVec     = V29_RX_EqUpdate;

    pV29->pfDecodeVec       = V29_Dummy;
    pV29->pfDescramVec      = V29_Dummy;
    pV29->pfDataOutVec      = V29_Receive_Segment3;
}

/**************************************/
/* all scrambled binary ONE's segment */
void V29_Rx_Segm4(V29Struct *pV29)
{
    TRACE0("V29: Segment4");

    pV29->sbPast_phase     = 0;
    pV29->DescramSReg      = 0;

    pV29->pfDetectVec      = V29_Dummy;
    pV29->pfTimingVec      = V29_TimingRecovery;
    pV29->pfEqVec          = V29_RX_Eq;
    pV29->pfRotateVec      = V29_RX_Rotate;
    pV29->pfTrainScramVec  = V29_Dummy;
    pV29->pfTrainEncodeVec = V29_Dummy;
    pV29->pfSliceVec       = V29_RX_Slicer;
    pV29->pfDerotateVec    = V29_RX_Derotate;
    pV29->pfCarrierVec     = V29_CarrierUpdate;
    pV29->pfTimUpdateVec   = V29_RX_TimingUpdate;

    pV29->qEqBeta          = 3000;
    pV29->pfEqUpdateVec    = V29_RX_EqUpdate;

    pV29->pfDecodeVec      = V29_RX_Decoder;
    pV29->pfDescramVec     = V29_RX_Descram;
    pV29->pfDataOutVec     = V29_Dummy;

    pV29->qCarAcoef    = 32368;
    pV29->qCarBcoef    = 300 ;
}

void V29_Rx_DataMode(V29Struct *pV29)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV29->pTable[DPCS_STRUC_IDX]);

    PutReg(pDpcs->MSR0, DATA);

    TRACE0("V29: Data mode");

    pV29->pfDetectVec       = V29_Dummy;
    pV29->pfTimingVec       = V29_TimingRecovery;
    pV29->pfEqVec           = V29_RX_Eq;
    pV29->pfRotateVec       = V29_RX_Rotate;
    pV29->pfTrainScramVec   = V29_Dummy;
    pV29->pfTrainEncodeVec  = V29_Dummy;
    pV29->pfSliceVec        = V29_RX_Slicer;
    pV29->pfDerotateVec     = V29_RX_Derotate;
    pV29->pfCarrierVec      = V29_CarrierUpdate;
    pV29->pfTimUpdateVec    = V29_RX_TimingUpdate;

#if 0
    pV29->qEqBeta       = V29_EQ_ADAPT_BETA;
    pV29->pfEqUpdateVec = V29_RX_EqUpdate;
#else
    pV29->pfEqUpdateVec = V29_Dummy;
#endif

    pV29->pfDecodeVec       = V29_RX_Decoder;
    pV29->pfDescramVec      = V29_RX_Descram;
    pV29->pfDataOutVec      = V29_Receive_DataMode;

#if 0 /* EQ coeff */

    for (i = 0; i < V29_EQ_LENGTH; i++)
    {
        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV29->cEqCoef[i].r; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV29->cEqCoef[i].i; }
    }

#endif
}
