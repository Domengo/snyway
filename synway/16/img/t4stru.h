/*****************************************************************************
*             Copyright (c) 1995 - 2007 by GAO Research Inc.                 *
*                          All rights reserved.                              *
*                                                                            *
*  This program is a confidential and an unpublished work of GAO Research    *
*  Inc. that is protected under international and Canadian copyright laws.   *
*  This program is a trade secret and constitutes valuable property of GAO   *
*  Research Inc. All use, reproduction and/or disclosure in whole or in      *
*  part of this program is strictly prohibited unless expressly authorized   *
*  in writing by GAO Research Inc.                                           *
*****************************************************************************/
#ifndef _T4_STRU_H
#define _T4_STRU_H

#include "imgprodte.h"

#if SUPPORT_CONVERSION_BETWEEN_T4_T6


#include "t4def.h"

typedef struct
{
    UBYTE *pOut;
    UBYTE partialByte;
    int numBitsFree;
    int numBitsOutput;
} t4OutputStruct;


typedef struct
{
    int a0;
    int a0Color;
    int b1;
    int b1Color;
    int b1Index;
    int b2;
    int vx;
    int Hrun;
    int numCodeword;
    UWORD *pReferenceLine;
} twoDStruct;

/*****************************\
    Decoder Structure
\*****************************/
struct t4decstruct
{
    const SWORD *tree;
    int runLengthAccumulated;
    int treeIndex;
    int color;
    int numZeros;
    int compressionMode;
    int numEOL;
    int runIndex;
    int lineLengthCurrent;
    int isLineError;
    int isFirstEOL;

    // 2D decoder
    twoDStruct twoD;
    int (*pFnProcess)(struct t4decstruct *, int, int *, int *);
    int pageWidth;
};
typedef struct t4decstruct t4DecoderStruct;

/*****************************\
       Encoder Structure
\*****************************/
typedef struct
{
    int pageWidth;
    int isLSBfirst;
    int compressionMode;
    int minimumScanLineBits;
    int    K;
    int lineIdx;
    twoDStruct twoD;
    t4OutputStruct t4Output;
} t4EncoderStruct;

#endif
#endif //T4_STRU_H

