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


#ifndef _IMG_PRO_DTE_H_
#define _IMG_PRO_DTE_H_

#define SUPPORT_CONVERSION_BETWEEN_T4_T6             (1) // 1: support image conversion between T4 and T6, 0: no image conversion to have a lower MIPS


#include "gstdio.h"
#include "commtype.h"
#include "gaomsg.h"
#include "dsmstru.h"

#include "imgprodef.h"
#include "t4def.h"
#include "t4stru.h"
#include "scaling.h"
#if (SUPPORT_TX_PURE_TEXT_FILE || SUPPORT_TX_COVER_PAGE)
#include "asciicov.h"
#endif
#include "faxapi.h"


typedef struct
{
    GAO_Msg *pDsmImg;
    GAO_Msg *pImgDce;
} TImgApiStruct;

#if SUPPORT_CONVERSION_BETWEEN_T4_T6
typedef struct
{
    UWORD IndexIn;
    UWORD IndexOut;
    UBYTE ImgDataCB[MAX_IMAGE_LEN_AFTER_ENCODING];
} TImageReEncodeCB;
#endif


typedef struct
{
#if (SUPPORT_TX_PURE_TEXT_FILE || SUPPORT_TX_COVER_PAGE)
    ascii2BmpStruct asciiStruct;
#endif
    UBYTE Direction;//MSG_DIR_IDLE or MSG_DOWN_LINK or MSG_UP_LINK
    UBYTE ImgFlagBackup;
#if (SUPPORT_TX_COVER_PAGE)
    UBYTE TxCoverP;
#endif

    TImgFormat SrcFormat; // format before conversion
    TImgFormat DesFormat;    // format after conversion
    UBYTE ImgConversion;

#if SUPPORT_CONVERSION_BETWEEN_T4_T6
    UWORD Bitmap[BITMAP_BUFFER_LENGTH];
    UWORD *pDecoderLine;
    UWORD *pDecoderReferenceLine;
    UWORD *pEncoderLine;
    UWORD *pEncoderReferenceLine;

    t4DecoderStruct t4Decoder;
    t4EncoderStruct t4Encoder;
    ScalerStruct scaler;

    SDWORD errorCode;
    SDWORD errorCount;
    SDWORD functionResult;

    TImageReEncodeCB ImgReEncodeCB;
#endif

    UBYTE XCon;
    UBYTE YCon;
    UBYTE CompCon;

} TImgProDteStruct;


void ImageConversionDTE(GAO_Msg *pMsgOut, GAO_Msg *pMsgIn, TImgProDteStruct *pImg);

#endif


