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
#ifndef _T4_EXT_H
#define _T4_EXT_H

#include "imgprodte.h"

#if SUPPORT_CONVERSION_BETWEEN_T4_T6


#include "t4stru.h"


extern const SWORD t4DecoderTreeWhite[COLOR_TREE_LENGTH];
extern const SWORD t4DecoderTreeBlack[COLOR_TREE_LENGTH];
extern const SWORD t4DecoderTreeLarge[COMBINED_TREE_LENGTH];
extern const UBYTE t4EncoderPatternWhiteSmall[ENCODER_CODEBOOK_LENGTH_SMALL];
extern const UBYTE t4EncoderShiftWhiteSmall[ENCODER_CODEBOOK_LENGTH_SMALL];
extern const UBYTE t4EncoderPatternBlackSmall[ENCODER_CODEBOOK_LENGTH_SMALL];
extern const UBYTE t4EncoderShiftBlackSmall[ENCODER_CODEBOOK_LENGTH_SMALL];
extern const UBYTE t4EncoderPatternWhiteMedium[ENCODER_CODEBOOK_LENGTH_MEDIUM];
extern const UBYTE t4EncoderShiftWhiteMedium[ENCODER_CODEBOOK_LENGTH_MEDIUM];
extern const UBYTE t4EncoderPatternBlackMedium[ENCODER_CODEBOOK_LENGTH_MEDIUM];
extern const UBYTE t4EncoderShiftBlackMedium[ENCODER_CODEBOOK_LENGTH_MEDIUM];
extern const UBYTE t4EncoderPatternLarge[ENCODER_CODEBOOK_LENGTH_LARGE];
extern const UBYTE t4EncoderShiftLarge[ENCODER_CODEBOOK_LENGTH_LARGE];
extern const UBYTE vModePattern[8];
extern const UBYTE vModeNumBits[8];

/*************************************************************************************\
*************************  T.4/T.6 Codec External API  ********************************
\*************************************************************************************/

// Encoder
int t4Encoder_init(t4EncoderStruct *pT4Encoder, int pageWidth, int isLSBfirst, int compressionMode, int parameterK, int minimumScanLineBits, UWORD *pReferenceLine);
int t4Encoder_end(t4EncoderStruct *pT4Encoder, UBYTE *pOut);
int t4Encoder_apply(t4EncoderStruct *pT4Encoder, UWORD *pLine, UWORD *pReferenceLine, UBYTE *pOut);

// Decoder
int t4Decoder_init(t4DecoderStruct *pT4Decoder, int pageWidth, int compressionMode, UWORD *pReferenceLine);
int t4Decoder_apply(t4DecoderStruct *pT4Decoder, int bit, UWORD *pLine, UWORD *pReferenceLine);


/*************************************************************************************\
*************************  T.4/T.6 Codec Internal API  ********************************
\*************************************************************************************/
void t4EncodeRun(int color, int runLength, t4OutputStruct *pT4Output);

void T4T6_reset2D(twoDStruct *p2D);
void T4T6_calculateB1B2(twoDStruct *p2D, int pageWidth);
int T4T6_detectEOL(t4DecoderStruct *pDec, int bit, int *pRunLength, int *pColor);
int T4T6_decodeEOLtag(t4DecoderStruct *pDec, int bit, int *pRunLength, int *pColor);
int T4T6_decodeLine2D(t4DecoderStruct *pDec, int bit, int *pRunLength, int *pColor);


#endif
#endif //_T4_EXT_H


