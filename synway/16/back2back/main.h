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

#ifndef _MAIN_H_
#define _MAIN_H_

#include "commtype.h"

#define APP_FILE_OPERATION                 (0) // 1: file, 0:buffer

typedef struct
{
    UBYTE         FClass;
    UBYTE         isDial;
    UBYTE         isV34Enabled;
    UBYTE         isEcmEnabled;
    UBYTE         ringNum;
    UBYTE         maxBitRate;
    UBYTE         minBitRate;
    UBYTE         dialNum[21];
    UBYTE         localID[21];
    UBYTE         tiffFile[100];
} DTE_InitStruct;

#endif
