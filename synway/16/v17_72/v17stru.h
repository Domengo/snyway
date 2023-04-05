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

#ifndef _V17STRU_H
#define _V17STRU_H

#include "v3217str.h"
#include "v17def.h"

typedef struct _v17_struct V17Struct;
typedef void (*V17Fnptr)(V17Struct *);

struct _v17_struct
{
    /* pointer to host-allocated memory */
    UBYTE **pTable;

    UBYTE ImageSendMode;

    V17Fnptr pfDetectVec;
    V17Fnptr pfDataInVec;      /* input data */
    V17Fnptr pfTxSetUpVec;     /* vector for 1 symbol transmit */
    V17Fnptr *pfTxSetUpVecTab; /* Table of Setup vectors */

    V17Fnptr pfRxSetUpVec;     /* vector for 1 symbol receive  */
    V17Fnptr *pfRxSetUpVecTab; /* table of setup vector  */

    /* receive vectors (pointer to functions) */
    V17Fnptr pfRotateVec;      /* rotater */
    V17Fnptr pfDataOutVec;     /* output data */

    V32ShareStruct V32Share;

    UBYTE  ubTraintype;

    UBYTE  ubGuardToneCnt;
    UBYTE  ubABtoneCnt;

    UBYTE  ubSeg3Cnt;

    SWORD  time_index;

    QDWORD qdCarrLoss_egy;
    UBYTE  CarrLoss_Flag;
    QDWORD qdCarrLossEgy_Ref;
    UWORD  uCarrLoss_Count;

    UBYTE ubTxRateIdx;
    UBYTE ubRxRateIdx;

    /* correlation */
    QDWORD qdCorrelationI;
    QDWORD qdCorrelationQ;
    QDWORD qdCorrelation;
    QDWORD qdCorrThres;
    UBYTE  ubCorrIdx;
    CQWORD cqCorrDline[V17_CORR_NUM_SAMPLES];
    CQDWORD cqdDemodABTone[V17_SYNC_SAMPLES];
    SBYTE  sbSyncCounter;

    QDWORD qdMinPhaseErr0;
    QDWORD qdMinPhaseErr1;
    QDWORD qdMaxPhaseErr;
    SWORD  timeIndex0;
    SWORD  timeIndex1;
    SWORD  timingDelta;

    SWORD  sTimeOutCounter;

    UBYTE  V21DetectNumber;
    UWORD  V21DetectDelayCnt;
    UBYTE  SilenceCount;

#if SUPPORT_ENERGY_REPORT
    UBYTE  Detect_FaxTone;
    QDWORD qdEnergyAcc;
    QDWORD qdEnergyMax;
    QDWORD qdEnergyMin;
    UWORD  uEnergyCount;
#endif
};

#endif
