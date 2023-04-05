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

#ifndef _TIFFRW_H_
#define _TIFFRW_H_

#include "commtype.h"
#include "t4info.h"
#include "t4def.h"

#define USE_XOR_REVERSE                    (1)    // True to do byte reversal using XOR algorithm (good for DSPs). False to use temporary variable (good for CPUs)

// Fixed macros
#define IFD_BYTE                        (1)
#define IFD_ASCII                        (2)
#define IFD_SHORT                        (3)
#define IFD_LONG                        (4)
#define IFD_RATIONAL                    (5)
#define IFD_SRATIONAL                    (10)
#define TYPE_LITTLE_ENDIAN                (0)
#define TYPE_BIG_ENDIAN                    (1)
#define IFD_ENTRY_LENGTH                (12)
#define TIFF_NUMBER_OF_IFD_LENGTH        (2)
#define TIFF_NEXT_OFFSET_LENGTH            (4)
#define MIN_REQUIRED_IFDS                (16)
#define MIN_IFD_LENGTH                    ((MIN_REQUIRED_IFDS * IFD_ENTRY_LENGTH) + TIFF_NUMBER_OF_IFD_LENGTH + TIFF_NEXT_OFFSET_LENGTH)
#define TIFF_HEADER_LENGTH                (8)
#define WRITE_IFD_ENTRYNUM                (16)
#define WRITE_IFD_LENGTH                ((WRITE_IFD_ENTRYNUM * IFD_ENTRY_LENGTH) + TIFF_NUMBER_OF_IFD_LENGTH + TIFF_NEXT_OFFSET_LENGTH + 18) // WRITE_IFD_LENGTH: 2 + 12*WRITE_IFD_ENTRYNUM + 4 + 2(optional:for alignment) + 8(xRes) + 8(yRes)

// For writing
#define BITS_PER_SAMPLE_VALUE            (1)
#define NEWSUB_FILETYPE_VALUE            (2)    // bit1=1
#define RESOLUTION_UNIT_VALUE            (TIFF_RESOLUTION_UNIT_INCHES)
#define SAMPLES_PER_PIXEL_VALUE            (1)

// IFD Tag Offsets from IFD entry in bytes
// 16 bit TAG
// 16 bit TYPE
// 32 bit COUNT
// 32 bit VALUE
#define IFD_TAG_OFFSET                    (0)
#define IFD_TYPE_OFFSET                    (2)
#define IFD_COUNT_OFFSET                (4)
#define IFD_VALUE_OFFSET                (8)

// TIFF-F Tags
#define TAG_NEWSUBFILETYPE                (254)
#define TAG_IMAGEWIDTH                    (256)
#define TAG_IMAGELENGTH                    (257)
#define TAG_BITSPERSAMPLE                (258)
#define TAG_COMPRESSION                    (259)
#define TAG_PHOTOMETRIC                    (262)
#define TAG_FILLORDER                    (266)
#define TAG_STRIPOFFSETS                (273)
#define TAG_SAMPLESPERPIXEL                (277)
#define TAG_ROWSPERSTRIP                (278)
#define TAG_STRIPBYTECOUNTS                (279)
#define TAG_XRESOLUTION                    (282)
#define TAG_YRESOLUTION                    (283)
#define TAG_T4OPTIONS                    (292)
#define TAG_T6OPTIONS                    (293)
#define TAG_RESOLUTIONUNIT                (296)
#define TAG_PAGENUMBER                    (297)



// TIFF for JPEG
#define TAG_ORIENTATION                 (274)
#define TAG_DECODE                      (433)
#define TAG_SUBSAMPLING                 (530)
#define TAG_CHROMAPOSITION              (531)

// JPEG
#define JPEG_ORIENTATION_VALUE          (1)
#define JPEG_CHUNKY_FORMAT              (1)
#define JPEG_COMPRESSION                (7)

//JPEG
#define WRITE_IFD_ENTRYNUM_JPEG         (16)
//#define WRITE_IFD_LENGTH_JPEG            ((WRITE_IFD_ENTRYNUM_JPEG * IFD_ENTRY_LENGTH) + TIFF_NUMBER_OF_IFD_LENGTH + TIFF_NEXT_OFFSET_LENGTH + 18 + 8 + 48 + 8) // WRITE_IFD_LENGTH: 2 + 12*WRITE_IFD_ENTRYNUM + 4 + 2(optional:for alignment) + 6(BitsPerSample) + 2(alignment) + 8(xRes) + 8(yRes) + 48(Decode) + 4(subsample) + 4(page number)
//#define WRITE_IFD_LENGTH_JPEG            ((WRITE_IFD_ENTRYNUM_JPEG * IFD_ENTRY_LENGTH) + TIFF_NUMBER_OF_IFD_LENGTH + TIFF_NEXT_OFFSET_LENGTH + 18 + 8 + 4) // WRITE_IFD_LENGTH: 2 + 12*WRITE_IFD_ENTRYNUM + 4 + 2(optional:for alignment) + 6(BitsPerSample) + 2(alignment) + 8(xRes) + 8(yRes) + 4(page number)
//#define WRITE_IFD_LENGTH_JPEG1            ((WRITE_IFD_ENTRYNUM_JPEG * IFD_ENTRY_LENGTH) + TIFF_NUMBER_OF_IFD_LENGTH + TIFF_NEXT_OFFSET_LENGTH + 10 + 8 + 4) // WRITE_IFD_LENGTH: 2 + 12*WRITE_IFD_ENTRYNUM + 4 + 2(optional:for alignment) + 0(BitsPerSample) + 0(alignment) + 8(xRes) + 8(yRes) + 4(page number)
#define WRITE_IFD_LENGTH_JPEG            ((WRITE_IFD_ENTRYNUM_JPEG * IFD_ENTRY_LENGTH) + TIFF_NUMBER_OF_IFD_LENGTH + TIFF_NEXT_OFFSET_LENGTH + 18 + 8) // WRITE_IFD_LENGTH: 2 + 12*WRITE_IFD_ENTRYNUM + 4 + 2(optional:for alignment) + 6(BitsPerSample) + 2(alignment) + 8(xRes) + 8(yRes)
#define WRITE_IFD_LENGTH_JPEG1            ((WRITE_IFD_ENTRYNUM_JPEG * IFD_ENTRY_LENGTH) + TIFF_NUMBER_OF_IFD_LENGTH + TIFF_NEXT_OFFSET_LENGTH + 10 + 8) // WRITE_IFD_LENGTH: 2 + 12*WRITE_IFD_ENTRYNUM + 4 + 2(optional:for alignment) + 0(BitsPerSample) + 0(alignment) + 8(xRes) + 8(yRes)

#define JPEG_IMAGE_DEFAULT_LEN          (0x910)


#define TIFF_FLAG_NEWSUBFILETYPE        (1 << 0)
#define TIFF_FLAG_IMAGEWIDTH            (1 << 1)
#define TIFF_FLAG_IMAGELENGTH            (1 << 2)
#define TIFF_FLAG_BITSPERSAMPLE            (1 << 3)
#define TIFF_FLAG_COMPRESSION            (1 << 4)
#define TIFF_FLAG_PHOTOMETRIC            (1 << 5)
#define TIFF_FLAG_FILLORDER                (1 << 6)
#define TIFF_FLAG_STRIPOFFSETS            (1 << 7)
#define TIFF_FLAG_SAMPLESPERPIXEL        (1 << 8)
#define TIFF_FLAG_ROWSPERSTRIP            (1 << 9)
#define TIFF_FLAG_STRIPBYTECOUNTS        (1 << 10)
#define TIFF_FLAG_XRESOLUTION            (1 << 11)
#define TIFF_FLAG_YRESOLUTION            (1 << 12)
#define TIFF_FLAG_T4OPTIONS                (1 << 13)
#define TIFF_FLAG_T6OPTIONS                (1 << 14)
#define TIFF_FLAG_RESOLUTIONUNIT        (1 << 15)
#define TIFF_FLAG_PAGENUMBER            (1 << 16)


// Tage Values
#define TIFF_RESOLUTION_UNIT_INCHES        (2)
#define TIFF_PHOTOMETIC_0_IS_WHITE        (0)
#define TIFF_PHOTOMETIC_1_IS_WHITE        (1)
#define TIFF_RESOLUTION_UNIT_CENTI        (3)

// Macros for return value
#define TIFF_RD_HEADER_ERROR            (-1)
#define TIFF_NEED_MORE_DATA                (-2)
#define TIFF_RD_IFD_TAG_WRONG            (-3)
#define TIFF_RD_UNSUPPORTED                (-4)
#define TIFF_RD_MISSING_TAG                (-5)
#define TIFF_TIFF_FILE_OPERATION_ERR    (-6)



typedef struct
{
    int        startPageNumber;        // 0 is first page
    int        endPageNumber;            // 0 is end of file
    int        currentPageNumber;
    int        isReverseEndian;
    int        paddingLength;
    UDWORD    offsetIfd;
    UDWORD    offsetPage;
    UDWORD    lengthPage;

    // For Tiff Buffer:
    //   Reading: required offset and minimum buffer length for next call
    //   Writing: offset in file to write buffer to and length
    UDWORD    offset;
    UDWORD    length;
} TiffStruct;


// External API
void TIFF_init(TiffStruct *pTiffStruct, int startPageNumber, int numberOfPages);
SWORD TIFF_ReadHeader(TiffStruct *pTiffStruct, UBYTE *pTiffData, UDWORD length);
SWORD TIFF_ReadIfd(TiffStruct *pTiffStruct, T4InfoStruct *pPageInfo, UBYTE *pTiffData, UDWORD length);
void TIFF_WriteHeader(TiffStruct *pTiffStruct, UBYTE *pTiffData);
void TIFF_WriteIfd(TiffStruct *pTiffStruct, T4InfoStruct *pPageInfo, UBYTE *pIfd);
void TIFF_WriteIfd_Jpeg(TiffStruct *pTiffStruct, T4InfoStruct *pT4Info, UBYTE *pIfd);

#endif

