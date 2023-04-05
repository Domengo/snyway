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


#ifndef _T4INFO_H_
#define _T4INFO_H_

#include <stdio.h>
#include <string.h>
#include "commtype.h"


#define T4INFO_MAX_STRING_LENGTH        20
#define T4INFO_FILE_IO_ERROR            (-1)

#define T4INFO_STRING_COMPRESSION        "COMPRESSION"
#define T4INFO_STRING_FILLORDER            "FILLORDER"
#define T4INFO_STRING_IMAGEWIDTH        "IMAGEWIDTH"
#define T4INFO_STRING_IMAGELENGTH        "IMAGELENGTH"
#define T4INFO_STRING_PAGENUMBER        "PAGENUMBER"
#define T4INFO_STRING_TOTALPAGES        "TOTALPAGES"
#define T4INFO_STRING_PHOTOMETRIC        "PHOTOMETRIC"
#define T4INFO_STRING_RESOLUTIONUNIT    "RESOLUTIONUNIT"
#define T4INFO_STRING_XRESOLUTION        "XRESOLUTION"
#define T4INFO_STRING_YRESOLUTION        "YRESOLUTION"
#define T4INFO_STRING_ISLASTPAGE        "ISLASTPAGE"
#define T4INFO_STRING_PAGEOFFSET        "PAGEOFFSET"
#define T4INFO_STRING_PAGEBYTECOUNT        "PAGEBYTECOUNT"


// Page sizes
#define A4_WIDTH                            1728
#define A4_HEIGHT                            1120

#define B4_WIDTH                            2048
#define B4_HEIGHT                            1328

#define A3_WIDTH                            2432
#define A3_HEIGHT                            1576


// Page resolutions
#define NORMAL_RESOLUTION_98                98
#define NORMAL_RESOLUTION_100                100
#define NORMAL_RESOLUTION                    NORMAL_RESOLUTION_98
#define FINE_RESOLUTION_196                    196
#define FINE_RESOLUTION_200                    200
#define FINE_RESOLUTION                        FINE_RESOLUTION_196
#define NORMAL_RESOL_METRIC_39              39
#define FINE_RESOL_METRIC_77                77


// Fill Order
#define FILLORDER_MSB_FIRST                    1
#define FILLORDER_LSB_FIRST                    2




typedef struct
{
    int compression;    // return macros defined for write struct
    int fillOrder;        // return macros defined for write struct
    int imageWidth;
    int imageLength;
    int pageNumber;
    int totalPages;
    int photometric;
    int resolutionUnit;
    int xResolution;
    int yResolution;
    int isLastPage;        // 1:last page, 0:not last page
    int    minimumScanLineBits;
    int sampleNum;
    int bitNum[3];
    UDWORD pageOffset;
    UDWORD pageByteCount;
} T4InfoStruct;


//void T4INFO_initSessionParams(T4InfoStruct *pT4InfoStruct, int compression, int imageWidth, int fillOrder);
//void T4INFO_copyFileParams(T4InfoStruct *pSource, T4InfoStruct *pDest);
//int T4INFO_isNormalResolution(int yResolution);
//int T4INFO_imageHeight(int imageWidth);


#endif

