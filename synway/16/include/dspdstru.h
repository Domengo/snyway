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

#ifndef _DSPDSTRU_H
#define _DSPDSTRU_H

#include "dspddef.h"

/* ---------- FIR filtering ---------- */
typedef struct
{
    QWORD *pDline;    /* pointer to delayline in filter */
    /* Double the filter length */
    QWORD  *pCoef;    /* pointer to coefficients of filter */ /* Constant during filtering */
    UBYTE  nTapLen;   /* size of filter */
    UBYTE  nOffset;   /* Delayline pointer offset, init to 0 1st */
} QFIRStruct;

typedef struct
{
    /* Constant during filtering */
    QWORD  *pDline;
    QWORD  *pCoef;
    /* Small coefficient block size */
    UBYTE  blksize;
    UBYTE  nOffset;   /* Delayline pointer offset, init to 0 1st */
    /* Interpolating factor */
    UBYTE ifactor;
} QIFIRStruct;

typedef struct
{
    CQWORD *pcDline;
    CQWORD *pcCoef;
    UWORD  nTapLen;
    UWORD  nOffset;
} QCFIRStruct;


/* This is for Fixed point IIR, double precision delayline */
/* ---------- Bi-Quad IIR filtering ---------- */
typedef struct
{
    UBYTE numbiquad; /* Number of biquad               */
    SWORD *pCoef;    /* Shift count, B0, B1, B2, -A1, -A2 */
    SWORD *pDline;   /* Delayline */
} IIR_Cas5Struct;

typedef struct
{
    SWORD    TotalSamples;
    SWORD    SmpCount;
    CQDWORD  TempI;
    CQDWORD  TempQ;
    CQWORD   IResult;
    CQWORD   QResult;
    QWORD    MagResult;
    UWORD    CurPhase;
    UWORD    DeltaPhase;
    UBYTE    shift; /* this is for fixed point */
} QDFTPUREStruct;

typedef struct
{
    SWORD    TotalSamples;
    SWORD    Current_Sample_Num;
    UBYTE    DetTones;
    QWORD    *qQ1Q2;
    QWORD    *pDFT_tToneGcoefs;
    QDWORD   *qdMagSq;
    QDWORD   Avg_Energy;
    UBYTE    BitShift;
} DftStruct;

typedef struct
{
    SWORD    TotalSamples;
    SWORD    Current_Sample_Num;
    UBYTE    DetTones;
    UWORD    *puDetectTonePhase;
    QWORD    *pqDetectToneDeltaPhase;
    QDWORD   *pqdDetectToneResult_Real;
    QDWORD   *pqdDetectToneResult_Imag;
    QDWORD   *pqdMag;
    QDWORD   Avg_Energy;
    UBYTE    Shift;
} DspdDetectToneStruct;

typedef struct
{
    QWORD  *pqDline;  /* point to the begining of delay line buffer */
    QDWORD  *pqCoef;   /* pointer to coefficients of filter */

    UBYTE tap_length;      /* size of filter */
    UBYTE offset;          /* delayline pointer offset */
} CircFilter;

#if WITH_DC
typedef struct
{
    /* DC adaptation filter component */
    QDWORD qdLpY;
    QDWORD qdDcY;

    SWORD  DC_Count;
    UBYTE  DC_Start;
} DCStruct;
#endif

typedef struct
{
    CQWORD *pcqTimingDlineHead;
    SWORD  nTimingIdx;
    CQWORD cqTimingIQ[3];/* V32 & V17 & V29: 3;  V27, V22: 2 */
    UBYTE  ubTimingCnt;
    UBYTE  ubTiming_Found;
} PolyFilterStruct;

#endif
