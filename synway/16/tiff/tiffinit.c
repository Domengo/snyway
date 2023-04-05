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

/* TIFF-F writer for Toshiba project */

#include "tiffrw.h"

void TIFF_init(TiffStruct *pTiffStruct, int startPageNumber, int numberOfPages)
{
    if (numberOfPages == 0)
    {
        pTiffStruct->endPageNumber    = 0;    // No limit
    }
    else
    {
        pTiffStruct->endPageNumber    = startPageNumber + numberOfPages;
    }

    pTiffStruct->length                = TIFF_HEADER_LENGTH;
    pTiffStruct->startPageNumber    = startPageNumber;
    pTiffStruct->currentPageNumber    = 0;
    pTiffStruct->lengthPage            = 0;
    pTiffStruct->offset                = 0;
    pTiffStruct->offsetIfd            = 0;
    pTiffStruct->offsetPage            = 0;
    pTiffStruct->isReverseEndian    = 0;
    pTiffStruct->paddingLength        = 0;
}
