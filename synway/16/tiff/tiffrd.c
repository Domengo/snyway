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

/* TIFF-F reader */

#include "commmac.h"
#include "tiffrw.h"

UWORD ReadShort(UBYTE *ps, int isReversed)
{
    if (isReversed)
    {
        return (UWORD)((*(ps)) << 8) | (*(ps + 1));
    }
    else
    {
        return (UWORD)((*(ps + 1)) << 8) | (*(ps));
    }
}

UDWORD ReadLong(UBYTE *ps, int isReversed)
{
    UDWORD result;

    if (isReversed)
    {
        result   = ((*(ps)) << 8) | (*(ps + 1));
        result <<= 16;
        result  |= ((*(ps + 2)) << 8) | (*(ps + 3));
    }
    else
    {
        result   = ((*(ps + 3)) << 8) | (*(ps + 2));
        result <<= 16;
        result  |= ((*(ps + 1)) << 8) | (*(ps));
    }

    return result;
}


/****************************************************************************\
    TIFF_ReadHeader()

    Description:

    This function reads the first 8 bytes data of a TIFF file. The next IFD
    offset will be filled in after this function call. The file and CPU endian
    will also be checked.

    Return Value:
    0:                        Success
    TIFF_READ_HEADER_ERROR: Error
\****************************************************************************/
SWORD TIFF_ReadHeader(TiffStruct *pTiffStruct, UBYTE *pTiffData, UDWORD length)
{
    UWORD magicNumber;
    UWORD tiffId;

    if (length < TIFF_HEADER_LENGTH)
    {
        return TIFF_NEED_MORE_DATA;
    }

    tiffId = ReadShort(pTiffData + 0, 0);
    magicNumber = ReadShort(pTiffData + 2, 0);

    // Confirm File Endian and Magic Number
    if ((tiffId != 0x4949) && (tiffId != 0x4D4D))
    {
        return TIFF_RD_HEADER_ERROR;
    }

    if (magicNumber == 0x002A)
    {
        pTiffStruct->isReverseEndian = 0;
    }
    else if (magicNumber == 0x2A00)
    {
        pTiffStruct->isReverseEndian = 1;
    }
    else
    {
        return TIFF_RD_HEADER_ERROR;
    }

    pTiffStruct->offsetIfd = ReadLong(pTiffData + 4, pTiffStruct->isReverseEndian);
    pTiffStruct->offset = pTiffStruct->offsetIfd;
    pTiffStruct->length = TIFF_NUMBER_OF_IFD_LENGTH;

    return 0;
}

/*****************************************************************************
    TIFF_ReadIfd()

    Description:

    This function reads IFD information of one page image in a TIFF file.
    It needs several input information including the address of IFD data in
    memory and the number of
    data in bytes that is available in the memory for reading. Although the
    TIFF standard specifies that the IFD must begin at a WORD boudary in TIFF
    file, this function    does not require the IFD data address to be WORD aligned,
    because all the data access to the memory is by byte.

    After reading, the tag information of this IFD is stored in the buffer
    pointed by pPageInfo. All useful information except next IFD offset should
    be found in this output structure including data section offset, image
    width and length etc. The next IFD offset is stored in the structure
    pointed by pTiffReader, and it will be 0 if this is the last IFD in the TIFF
    file. This function only return offsets, rather than
    the address in memory. It is the responsbility for the upper level
    application to manage the memory mapping such as locating the data section
    address etc. Note that the returned "offset" is the offset in the whole
    TIFF file.

    Return Value:
    0:                        Success
    Other:                    Error (See below)
    TIFF_RD_IFD_DATA_SHORT:    The IFD data is not complete in the memory. Call
                            this function again after getting more data.
    TIFF_RD_IFD_TAG_WRONG:    Tag is not sorted in an ascending manner.
    TIFF_RD_UNSUPPORTED:    The tag value is unexpected. The TIFF file may not
                            be compatible with TIFF-F, or TIFF data has error.
    TIFF_RD_MISSING_TAG:    Some required tag is not found in the IFD.

*****************************************************************************/
SWORD TIFF_ReadIfd(TiffStruct *pTiffStruct, T4InfoStruct *pPageInfo, UBYTE *pTiffData, UDWORD length)
{
    UDWORD xRes[2], yRes[2];
    UDWORD requiredTags, offsetXRes = 0, offsetYRes = 0, offsetTemp, offsetbitnum = 0, bitnumCnt = 0;
    UDWORD valueLong, count;
    UWORD numIfdEntries, lastTag;
    UWORD valueShort1, valueShort2, tag, type;
    UBYTE *pData;
    int isReverseEndian = pTiffStruct->isReverseEndian;
    SBYTE errorCode = 0;
    unsigned int i;

    if (length < pTiffStruct->length)
    {
        return TIFF_NEED_MORE_DATA;
    }

    // Start with IFD offset
    pData = pTiffData + pTiffStruct->offsetIfd - pTiffStruct->offset;

    // Determine number of IFD entries
    numIfdEntries = ReadShort(pData, isReverseEndian);

    pData += TIFF_NUMBER_OF_IFD_LENGTH;
    pTiffStruct->length = (pTiffStruct->offsetIfd - pTiffStruct->offset) + (numIfdEntries * IFD_ENTRY_LENGTH)  + TIFF_NUMBER_OF_IFD_LENGTH + TIFF_NEXT_OFFSET_LENGTH;

    if (length < pTiffStruct->length)
    {
        return TIFF_NEED_MORE_DATA;
    }

    ////////////////////////////////////////////////
    // Read current IFD
    // If in range of valid pages
    //
    if (pTiffStruct->currentPageNumber >= pTiffStruct->startPageNumber)
    {
        // Initialize IDF reader and default values for optional fields
        lastTag = 0;
        requiredTags = TIFF_FLAG_IMAGEWIDTH | TIFF_FLAG_IMAGELENGTH | TIFF_FLAG_COMPRESSION | TIFF_FLAG_FILLORDER | TIFF_FLAG_STRIPOFFSETS | TIFF_FLAG_STRIPBYTECOUNTS | TIFF_FLAG_XRESOLUTION | TIFF_FLAG_YRESOLUTION;
        pPageInfo->pageNumber = 0;
        pPageInfo->photometric = TIFF_PHOTOMETIC_0_IS_WHITE;
        pPageInfo->resolutionUnit = TIFF_RESOLUTION_UNIT_INCHES;
        pPageInfo->totalPages = 0;

        // Process IFD Entries
        for (i = 0; i < numIfdEntries; i++)
        {
            // Read IFD Entry

            /* Only types of short and long or rational are required for Class F, so only
            those types are supported. Since multistrip is not supported, only single count
            entries are used */

            tag            = ReadShort(pData + IFD_TAG_OFFSET, isReverseEndian);
            type        = ReadShort(pData + IFD_TYPE_OFFSET, isReverseEndian);
            count        = ReadLong(pData + IFD_COUNT_OFFSET, isReverseEndian); // We don't support/use multiple counts for any of the tags
            valueShort1    = ReadShort(pData + IFD_VALUE_OFFSET, isReverseEndian); // First short value
            valueShort2    = ReadShort(pData + IFD_VALUE_OFFSET + 2, isReverseEndian); // Second short value
            valueLong    = ReadLong(pData + IFD_VALUE_OFFSET, isReverseEndian);
            pData += IFD_ENTRY_LENGTH;

            // Check Tag
            if (tag <= lastTag)
            {
                errorCode = TIFF_RD_IFD_TAG_WRONG;
                break;
            }

            lastTag = tag;

            switch (tag)
            {
                case TAG_IMAGEWIDTH:

                    if (type == IFD_SHORT)
                    {
                        pPageInfo->imageWidth = (int) valueShort1;
                    }
                    else
                    {
                        pPageInfo->imageWidth = (int) valueLong;
                    }

                    requiredTags &= ~ TIFF_FLAG_IMAGEWIDTH;
                    break;

                case TAG_IMAGELENGTH:

                    if (type == IFD_SHORT)
                    {
                        pPageInfo->imageLength = (int) valueShort1;
                    }
                    else
                    {
                        pPageInfo->imageLength = (int) valueLong;
                    }

                    requiredTags &= ~ TIFF_FLAG_IMAGELENGTH;
                    break;

                case TAG_BITSPERSAMPLE:
                    bitnumCnt = 0;

                    if (count == 1)
                    {
                        if ((valueShort1 != 1) && (valueShort1 != 8) && (valueShort1 != 12))
                        {
                            errorCode = TIFF_RD_UNSUPPORTED;
                        }
                        else
                        {
                            pPageInfo->bitNum[0] = valueShort1;
                        }
                    }
                    else if (count == 3)
                    {
                        offsetbitnum = valueLong;
                    }

                    bitnumCnt = count;
                    break;

                case TAG_COMPRESSION:

                    if (valueShort1 == 3)
                    {
                        pPageInfo->compression = T4_1D_MH;
                        requiredTags |= TIFF_FLAG_T4OPTIONS;
                    }
                    else if (valueShort1 == 4)
                    {
                        pPageInfo->compression = T6_2D_MMR;
                        requiredTags |= TIFF_FLAG_T6OPTIONS;
                    }
                    else if (valueShort1 == 7)
                    {
                        pPageInfo->compression = JPEG_COMPRESSION;
                    }
                    else
                    {
                        errorCode = TIFF_RD_UNSUPPORTED;
                    }

                    requiredTags &= ~ TIFF_FLAG_COMPRESSION;
                    break;

                case TAG_PHOTOMETRIC:
                    pPageInfo->photometric = (int) valueShort1;
                    break;

                case TAG_FILLORDER:
                    pPageInfo->fillOrder = (int) valueShort1;
                    requiredTags &= ~ TIFF_FLAG_FILLORDER;
                    break;

                case TAG_STRIPOFFSETS:

                    // Only support single strip data
                    if (count != 1)
                    {
                        errorCode = TIFF_RD_UNSUPPORTED;
                    }

                    if (type == IFD_SHORT)
                    {
                        pPageInfo->pageOffset = (UDWORD) valueShort1;
                    }
                    else
                    {
                        pPageInfo->pageOffset = valueLong;
                    }

                    pTiffStruct->offsetPage = pPageInfo->pageOffset;
                    requiredTags &= ~ TIFF_FLAG_STRIPOFFSETS;
                    break;

                case TAG_SAMPLESPERPIXEL:
                    pPageInfo->sampleNum = valueShort1;

                    if ((valueShort1 != 1) && (valueShort1 != 3))
                    {
                        errorCode = TIFF_RD_UNSUPPORTED;
                    }

                    break;

                case TAG_ROWSPERSTRIP:

                    // For single strip, rows per strip equals image length or 2^16-1 for short or 2^32-1 for long
                    if ((type == IFD_SHORT) && (valueShort1 < (UWORD)pPageInfo->imageLength))
                    {
                        errorCode = TIFF_RD_UNSUPPORTED;
                    }
                    else if (valueLong < (UDWORD)pPageInfo->imageLength)
                    {
                        errorCode = TIFF_RD_UNSUPPORTED;
                    }

                    break;

                case TAG_STRIPBYTECOUNTS:

                    if (type == IFD_SHORT)
                    {
                        pPageInfo->pageByteCount = (UDWORD) valueShort1;
                    }
                    else
                    {
                        pPageInfo->pageByteCount = valueLong;
                    }

                    pTiffStruct->lengthPage = pPageInfo->pageByteCount;
                    requiredTags &= ~ TIFF_FLAG_STRIPBYTECOUNTS;
                    break;

                case TAG_XRESOLUTION:
                    offsetXRes = valueLong;        // Get value of tag below
                    requiredTags &= ~ TIFF_FLAG_XRESOLUTION;
                    break;

                case TAG_YRESOLUTION:
                    offsetYRes = valueLong;        // Get value of tag below
                    requiredTags &= ~ TIFF_FLAG_YRESOLUTION;
                    break;

                case TAG_T4OPTIONS:

                    if (pPageInfo->compression == T4_1D_MH)
                    {
                        if (valueLong & 0x2)        // bit1 must be 0, uncompressed mode not supported.
                        {
                            errorCode = TIFF_RD_UNSUPPORTED;
                        }

                        if (valueLong & 0x1)
                        {
                            pPageInfo->compression = T4_2D_MR;
                        }
                    }

                    requiredTags &= ~ TIFF_FLAG_T4OPTIONS;
                    break;

                case TAG_T6OPTIONS:

                    if (pPageInfo->compression == T6_2D_MMR)
                    {
                        if (valueLong & 0x2)        // uncompressed mode not supported
                        {
                            errorCode = TIFF_RD_UNSUPPORTED;
                        }
                    }

                    requiredTags &= ~ TIFF_FLAG_T6OPTIONS;
                    break;

                case TAG_RESOLUTIONUNIT:
                    pPageInfo->resolutionUnit = valueShort1;
                    break;

                case TAG_PAGENUMBER:
                    pPageInfo->pageNumber = valueShort1;
                    pPageInfo->totalPages = valueShort2;
                    break;

                default:
                    break;
            }

            if (errorCode)
            {
                break;
            }
        }


        // Ensure that all required Tags were found
        if (requiredTags && !errorCode)
        {
            errorCode = TIFF_RD_MISSING_TAG;
        }

        // If any problems, don't read X and Y resolutions
        if (errorCode)
        {
            return errorCode;
        }

        if (bitnumCnt > 1)
        {
            offsetTemp = offsetbitnum;

            if (offsetTemp + 8 > pTiffStruct->offset + length)
            {
                pTiffStruct->length = offsetTemp + 8 - pTiffStruct->offset;
                return TIFF_NEED_MORE_DATA;
            }

            pData = pTiffData - pTiffStruct->offset;

            for (valueShort1 = 0; valueShort1 < bitnumCnt; valueShort1++)
            {
                pPageInfo->bitNum[valueShort1] = ReadShort(pData + offsetbitnum + valueShort1 * 2, isReverseEndian);
            }
        }

        // Get rational value (x,y resolution).

        // check offsets to ensure they are in range
        offsetTemp = MAX32(offsetXRes, offsetYRes);

        if (offsetTemp + 8 > pTiffStruct->offset + length)
        {
            pTiffStruct->length = offsetTemp + 8 - pTiffStruct->offset;
            return TIFF_NEED_MORE_DATA;
        }

        offsetTemp = MIN32(offsetXRes, offsetYRes);

        if (offsetTemp < pTiffStruct->offset)
        {
            pTiffStruct->length += pTiffStruct->offset - offsetTemp;
            pTiffStruct->offset = offsetTemp;
            return TIFF_NEED_MORE_DATA;
        }

        // Reset pData to beginning of TIFF File. Must then use TIFF offsets to point into buffer range
        pData = pTiffData - pTiffStruct->offset;

        xRes[0] = ReadLong(pData + offsetXRes    , isReverseEndian);
        xRes[1] = ReadLong(pData + offsetXRes + 4, isReverseEndian);
        yRes[0] = ReadLong(pData + offsetYRes    , isReverseEndian);
        yRes[1] = ReadLong(pData + offsetYRes + 4, isReverseEndian);

        pPageInfo->xResolution = xRes[0] / xRes[1];

        if (pPageInfo->xResolution *xRes[1] != xRes[0])  // If fractional result, round up.
        {
            pPageInfo->xResolution++;
        }

        pPageInfo->yResolution = yRes[0] / yRes[1];

        if (pPageInfo->yResolution *yRes[1] != yRes[0])
        {
            pPageInfo->yResolution++;
        }

        if (pPageInfo->compression == 7)
        {
            if ((pPageInfo->sampleNum == 3) && ((pPageInfo->photometric == CS_CIELAB8) || (pPageInfo->photometric == CS_CIELAB9) || (pPageInfo->photometric == CS_CIELAB10)))
            {
                pPageInfo->compression = JPEG_COLOR;
            }
            else
            {
                pPageInfo->compression = JPEG_GRAY;
            }
        }
    }


    // If any problems, don't skip to next page
    if (errorCode)
    {
        return errorCode;
    }


    ////////////////////////////////////////////////
    // Current page complete. Setup for next page
    //

    if (pTiffStruct->currentPageNumber < pTiffStruct->startPageNumber)
    {
        errorCode = TIFF_NEED_MORE_DATA;    // Flag to continue reading file at next IFD
    }

    pData = pTiffData - pTiffStruct->offset;    // Reset pData to beginning of TIFF File. Must use TIFF offsets to point into buffer range

    pTiffStruct->currentPageNumber++;
    pTiffStruct->offsetIfd = ReadLong(pData + pTiffStruct->offsetIfd + numIfdEntries * IFD_ENTRY_LENGTH + 2, isReverseEndian);

    if ((pTiffStruct->offsetIfd != 0) && (pTiffStruct->currentPageNumber != pTiffStruct->endPageNumber))    // If endPageNumber == 0, this test will always pass so file will determine last page. Check must be done after currentPageNumber increment
    {
        // Set up for next page
        pPageInfo->isLastPage = 0;
        pTiffStruct->offset = pTiffStruct->offsetIfd;
        pTiffStruct->length = TIFF_NUMBER_OF_IFD_LENGTH;
    }
    else
    {
        // Clean up after final page
        pPageInfo->isLastPage = 1;
        pTiffStruct->offset = 0;
        pTiffStruct->length = 0;
    }

    return errorCode;
}

