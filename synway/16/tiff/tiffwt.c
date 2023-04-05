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

void WriteShort(UBYTE *ps, UWORD value)
{
    *(ps) = (UBYTE)((value) & 0xFF);
    *(ps + 1) = (UBYTE)((value >> 8) & 0xFF);
}

void WriteLong(UBYTE *ps, UDWORD value)
{
    *(ps) = (UBYTE)((value) & 0xFF);
    *(ps + 1) = (UBYTE)((value >>  8) & 0xFF);
    *(ps + 2) = (UBYTE)((value >> 16) & 0xFF);
    *(ps + 3) = (UBYTE)((value >> 24) & 0xFF);
}


/*****************************************************************************
    TIFF_WriteHeader()

    Description:
    This function writes TIFF file header into a specified memory address.
    It should be called at first for writing a TIFF file. After calling
    this function, the procedure to write the TIFF file is
    - Copy data into data section.
    - Write the corresponding IFD into IFD section.
    - Copy next page data into next data section.
    - Write next page's IFD into next IFD section.
    - ......
    In order to do stream process, the data should be written before writing
    IFD, although the data section is behind the IFD section in memory.
    The offset of data section is
    pTiffStruct->nextIfdOffset + WRITE_IFD_LENGTH.
    The corresponding IFD section offset is
    pTiffStruct->nextIfdOffset.
    The IFD section offset will not be updated until current IFD has been
    written (IFD writing function being called).
    The exact destination address for writing of each section in memory space
    is controlled by upper level application.

    Return Value:
    Null
*****************************************************************************/
void TIFF_WriteHeader(TiffStruct *pTiffStruct, UBYTE *pTiffData)
{
    UWORD magicNumber = 0x2A;
    UWORD byteOrder;

    pTiffStruct->isReverseEndian = 0;

    if (*(UBYTE *)(&magicNumber) == 0x2A)
    {
        byteOrder = 0x4949;    // Little-endian
    }
    else
    {
        byteOrder = 0x4D4D;    // Big-endian
    }

    // Write TIFF Header
    WriteShort(pTiffData, byteOrder);

    WriteShort(pTiffData + 2, magicNumber);

    WriteLong(pTiffData + 4, TIFF_HEADER_LENGTH);

    pTiffStruct->offsetIfd = TIFF_HEADER_LENGTH;

    pTiffStruct->offsetPage = pTiffStruct->offsetIfd + WRITE_IFD_LENGTH;

    pTiffStruct->lengthPage = 0;

    pTiffStruct->offset = 0;

    pTiffStruct->length = TIFF_HEADER_LENGTH;
}


UBYTE *TIFF_WriteIfdEntry(UBYTE *pEntry, UWORD tag, UWORD type, UDWORD count, UDWORD value)
{
    WriteShort(pEntry + IFD_TAG_OFFSET, tag);

    WriteShort(pEntry + IFD_TYPE_OFFSET, type);

    WriteLong(pEntry + IFD_COUNT_OFFSET, count);

    if ((type == IFD_SHORT) && (count == 1))
    {
        WriteShort(pEntry + IFD_VALUE_OFFSET, (UWORD)value);
        WriteShort(pEntry + IFD_VALUE_OFFSET + 2, 0);
    }
    else
    {
        WriteLong(pEntry + IFD_VALUE_OFFSET, value);
    }

    return (pEntry + IFD_ENTRY_LENGTH);
}


UBYTE *TIFF_WriteIfdEntryPageNumber(UBYTE *pEntry, UWORD tag, UWORD type, UDWORD count, UWORD pageNum, UWORD totalPageNum)
{
    WriteShort(pEntry + IFD_TAG_OFFSET, tag);
    WriteShort(pEntry + IFD_TYPE_OFFSET, type);
    WriteLong(pEntry + IFD_COUNT_OFFSET, count);
    WriteShort(pEntry + IFD_VALUE_OFFSET, pageNum);
    WriteShort(pEntry + IFD_VALUE_OFFSET + 2, totalPageNum);
    return (pEntry + IFD_ENTRY_LENGTH);
}


/*****************************************************************************
    TIFF_WriteIfd()

    Description:

    This function writes TIFF file IFD into a specified memory address.
    The input to this function include parameters of the page associated with
    the IDF to be written, and the memory address from which the IFD will be
    started filling in.

    After calling this function, pTiffStruct->nextIfdOffset will be updated to
    the offset of next IFD.

    Return Value:
    Null
*****************************************************************************/
void TIFF_WriteIfd(TiffStruct *pTiffStruct, T4InfoStruct *pT4Info, UBYTE *pIfd)
{
    UDWORD offsetIfd;
    int i;

    pTiffStruct->offset = pTiffStruct->offsetIfd - pTiffStruct->paddingLength;
    pTiffStruct->length = WRITE_IFD_LENGTH + pTiffStruct->paddingLength;

    for (i = 0; i < pTiffStruct->paddingLength; i++)
    {
        *pIfd++ = 0;
    }

    // Fill in IFD entry number.
    WriteShort(pIfd, WRITE_IFD_ENTRYNUM);
    pIfd += 2;

    // Fill in IFD.
    //                                Tag                        Type        Count    Value
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_NEWSUBFILETYPE,        IFD_LONG,        1, NEWSUB_FILETYPE_VALUE);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_IMAGEWIDTH,            IFD_LONG,        1, pT4Info->imageWidth);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_IMAGELENGTH,        IFD_LONG,        1, pT4Info->imageLength);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_BITSPERSAMPLE,        IFD_SHORT,        1, BITS_PER_SAMPLE_VALUE);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_COMPRESSION,        IFD_SHORT,        1, ((pT4Info->compression == T6_2D_MMR) ? 4 : 3));
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_PHOTOMETRIC,        IFD_SHORT,        1, (UDWORD)pT4Info->photometric);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_FILLORDER,            IFD_SHORT,        1, (UDWORD)pT4Info->fillOrder);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_STRIPOFFSETS,        IFD_LONG,        1, pTiffStruct->offsetIfd + WRITE_IFD_LENGTH);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_SAMPLESPERPIXEL,    IFD_SHORT,        1, SAMPLES_PER_PIXEL_VALUE);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_ROWSPERSTRIP,        IFD_LONG,        1, pT4Info->imageLength);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_STRIPBYTECOUNTS,    IFD_LONG,        1, pT4Info->pageByteCount);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_XRESOLUTION,        IFD_RATIONAL,    1, pTiffStruct->offsetIfd + WRITE_IFD_ENTRYNUM * IFD_ENTRY_LENGTH + 8);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_YRESOLUTION,        IFD_RATIONAL,    1, pTiffStruct->offsetIfd + WRITE_IFD_ENTRYNUM * IFD_ENTRY_LENGTH + 16);

    if (pT4Info->compression == T6_2D_MMR)
    {
        pIfd = TIFF_WriteIfdEntry(pIfd, TAG_T6OPTIONS,            IFD_LONG,        1, 0);
    }
    else
    {
        pIfd = TIFF_WriteIfdEntry(pIfd, TAG_T4OPTIONS,            IFD_LONG,        1, ((pT4Info->compression == T4_2D_MR) ? 1 : 0));
    }

    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_RESOLUTIONUNIT,        IFD_SHORT,        1, pT4Info->resolutionUnit);
    pIfd = TIFF_WriteIfdEntryPageNumber(pIfd, TAG_PAGENUMBER, IFD_SHORT, 2, (UWORD)(pTiffStruct->currentPageNumber), 0);
    //pIfd = TIFF_WriteIfdEntry(pIfd, TAG_PAGENUMBER,            IFD_SHORT,        2, pTiffStruct->offsetIfd + WRITE_IFD_ENTRYNUM * IFD_ENTRY_LENGTH + 24  );


    // Fill in next IFD offset.
    pTiffStruct->lengthPage = pT4Info->pageByteCount;

    if (pT4Info->isLastPage)
    {
        offsetIfd = 0;
        pTiffStruct->offsetPage = 0;
    }
    else
    {
        offsetIfd = pTiffStruct->offsetIfd + WRITE_IFD_LENGTH + pTiffStruct->lengthPage;
        offsetIfd &= 0xFFFFFFFC;
        offsetIfd += 4;
        pTiffStruct->paddingLength = offsetIfd - (pTiffStruct->offsetIfd + WRITE_IFD_LENGTH + pTiffStruct->lengthPage);
        pTiffStruct->offsetPage = offsetIfd + WRITE_IFD_LENGTH;    // Where to start writing next page data
    }

    pTiffStruct->offsetIfd = offsetIfd;

    // Next IFD offset (0 if no more pages)
    WriteLong(pIfd, offsetIfd);
    pIfd += 4;

    // Alignment filler
    WriteShort(pIfd, 0);
    pIfd += 2;

    // Write x, y resolution.
    WriteLong(pIfd, pT4Info->xResolution);
    pIfd += 4;
    WriteLong(pIfd, 1);
    pIfd += 4;
    WriteLong(pIfd, pT4Info->yResolution);
    pIfd += 4;
    WriteLong(pIfd, 1);
    pIfd += 4;

    pTiffStruct->currentPageNumber++;
}


void TIFF_WriteIfd_Jpeg(TiffStruct *pTiffStruct, T4InfoStruct *pT4Info, UBYTE *pIfd)
{
    UDWORD offsetIfd;
    int i;
    UDWORD sampleNum, offsetOrValue, ifdJpeglength, offsettemp = 0;


    if (pT4Info->compression == JPEG_COLOR)
    {
        offsettemp = WRITE_IFD_ENTRYNUM_JPEG * IFD_ENTRY_LENGTH + 8;
        sampleNum = 3;
        offsetOrValue = pTiffStruct->offsetIfd + offsettemp;
        ifdJpeglength = WRITE_IFD_LENGTH_JPEG;
    }
    else
    {
        offsettemp = WRITE_IFD_ENTRYNUM_JPEG * IFD_ENTRY_LENGTH;
        sampleNum = 1;
        offsetOrValue = 8;
        ifdJpeglength = WRITE_IFD_LENGTH_JPEG1;
    }

    pTiffStruct->offset = pTiffStruct->offsetIfd - pTiffStruct->paddingLength;
    pTiffStruct->length = ifdJpeglength + pTiffStruct->paddingLength;

    for (i = 0; i < pTiffStruct->paddingLength; i++)
    {
        *pIfd++ = 0;
    }

    // Fill in IFD entry number.
    WriteShort(pIfd, WRITE_IFD_ENTRYNUM_JPEG);
    pIfd += 2;

    // Fill in IFD.
    //                                Tag                        Type        Count    Value
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_NEWSUBFILETYPE,        IFD_LONG,        1, NEWSUB_FILETYPE_VALUE);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_IMAGEWIDTH,            IFD_LONG,        1, pT4Info->imageWidth);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_IMAGELENGTH,        IFD_LONG,        1, pT4Info->imageLength);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_BITSPERSAMPLE,        IFD_SHORT,        sampleNum, offsetOrValue);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_COMPRESSION,        IFD_SHORT,        1, JPEG_COMPRESSION);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_PHOTOMETRIC,        IFD_SHORT,        1, pT4Info->photometric);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_FILLORDER,            IFD_SHORT,        1, (UDWORD)pT4Info->fillOrder);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_STRIPOFFSETS,        IFD_LONG,        1, pTiffStruct->offsetIfd + ifdJpeglength);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_ORIENTATION,        IFD_SHORT,        1, JPEG_ORIENTATION_VALUE);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_SAMPLESPERPIXEL,    IFD_SHORT,        1, sampleNum);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_ROWSPERSTRIP,        IFD_LONG,        1, pT4Info->imageLength);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_STRIPBYTECOUNTS,    IFD_LONG,        1, pT4Info->pageByteCount);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_XRESOLUTION,        IFD_RATIONAL,    1, pTiffStruct->offsetIfd + offsettemp + 8);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_YRESOLUTION,        IFD_RATIONAL,    1, pTiffStruct->offsetIfd + offsettemp + 16);
    pIfd = TIFF_WriteIfdEntry(pIfd, TAG_RESOLUTIONUNIT,        IFD_SHORT,        1, pT4Info->resolutionUnit);
    pIfd = TIFF_WriteIfdEntryPageNumber(pIfd, TAG_PAGENUMBER, IFD_SHORT, 2, (UWORD)(pTiffStruct->currentPageNumber), 0);
    //pIfd = TIFF_WriteIfdEntry(pIfd, TAG_PAGENUMBER,            IFD_SHORT,        2, pTiffStruct->offsetIfd + offsettemp + 24  );

    // Fill in next IFD offset.
    pTiffStruct->lengthPage = pT4Info->pageByteCount;

    if (pT4Info->isLastPage)
    {
        offsetIfd = 0;
        pTiffStruct->offsetPage = 0;
    }
    else
    {
        offsetIfd = pTiffStruct->offsetIfd + ifdJpeglength + pTiffStruct->lengthPage;
        offsetIfd &= 0xFFFFFFFC;
        offsetIfd += 4;
        pTiffStruct->paddingLength = offsetIfd - (pTiffStruct->offsetIfd + ifdJpeglength + pTiffStruct->lengthPage);
        pTiffStruct->offsetPage = offsetIfd + ifdJpeglength;    // Where to start writing next page data
    }

    pTiffStruct->offsetIfd = offsetIfd;

    // Next IFD offset (0 if no more pages)
    WriteLong(pIfd, offsetIfd);
    pIfd += 4;

    // Alignment filler
    WriteShort(pIfd, 0);
    pIfd += 2;

    if (sampleNum == 3)// Write TAG_BITSPERSAMPLE
    {
        WriteShort(pIfd, 8);
        pIfd += 2;
        WriteShort(pIfd, 8);
        pIfd += 2;
        WriteShort(pIfd, 8);
        pIfd += 2;
        WriteShort(pIfd, 0); // alignment
        pIfd += 2;
    }

    // Write x, y resolution.
    WriteLong(pIfd, pT4Info->xResolution);
    pIfd += 4;
    WriteLong(pIfd, 1);
    pIfd += 4;
    WriteLong(pIfd, pT4Info->yResolution);
    pIfd += 4;
    WriteLong(pIfd, 1);
    pIfd += 4;

    // Write page number
    /*WriteShort(pIfd, (UWORD)(pTiffStruct->currentPageNumber));
    pIfd += 2;
    WriteShort(pIfd, 0);
    pIfd += 2;    */

    // decode
    /*WriteLong(pIfd, 0);
    pIfd += 4;
    WriteLong(pIfd, 1);
    pIfd += 4;

    WriteLong(pIfd, 100);
    pIfd += 4;
    WriteLong(pIfd, 1);
    pIfd += 4;

    WriteLong(pIfd, -21760);
    pIfd += 4;
    WriteLong(pIfd, 255);
    pIfd += 4;

    WriteLong(pIfd, 21590);
    pIfd += 4;
    WriteLong(pIfd, 255);
    pIfd += 4;

    WriteLong(pIfd, -19200);
    pIfd += 4;
    WriteLong(pIfd, 255);
    pIfd += 4;

    WriteLong(pIfd, 31800);
    pIfd += 4;
    WriteLong(pIfd, 255);
    pIfd += 4;

    // subsampling
    WriteShort(pIfd, 2);
    pIfd += 2;
    WriteShort(pIfd, 2);
    pIfd += 2;
    */
    pTiffStruct->currentPageNumber++;
}
