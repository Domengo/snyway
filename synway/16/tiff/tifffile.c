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

#include <ctype.h>
#include <string.h>
#include "porting.h"
#include "tiffrw.h"
#include "gaomsg.h"
#include "dsmdef.h"
#include "imgprodte.h"
#include "tiffstru.h"
#if (SUPPORT_TX_PURE_TEXT_FILE || SUPPORT_TX_COVER_PAGE)
#include "asciicov.h"
#endif
#include "commmac.h"
#if (SUPPORT_TX_COVER_PAGE)
#include "coverpage.h"
#endif
#if SUPPORT_JPEG_CODEC
#include "cdjpeg.h"
#endif
#include "faxapi.h"

#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)
extern int dumpDataCnt;
extern UBYTE DiskOverflow;
#endif

extern SWORD TIFF_ReadHeader(TiffStruct *pTiffStruct, UBYTE *pTiffData, UDWORD length);
extern SWORD TIFF_ReadIfd(TiffStruct *pTiffStruct, T4InfoStruct *pPageInfo, UBYTE *pTiffData, UDWORD length);

CONST int XResoValue_Tiff[9] = {TIFF_X_40D_PER_CM_R4, TIFF_X_80D_PER_CM_R8, TIFF_X_160D_PER_CM_R16, TIFF_X_100D_PER_INCH, TIFF_X_200D_PER_INCH,
                                TIFF_X_300D_PER_INCH, TIFF_X_400D_PER_INCH, TIFF_X_600D_PER_INCH, TIFF_X_1200D_PER_INCH
                               };
CONST int YResoValue_Tiff[10] = {TIFF_Y_385L_PER_CM_NORMAL, TIFF_Y_77L_PER_CM_FINE, TIFF_Y_154L_PER_CM_PERFECT, TIFF_Y_100L_PER_INCH, TIFF_Y_200L_PER_INCH,
                                 TIFF_Y_300L_PER_INCH, TIFF_Y_400L_PER_INCH, TIFF_Y_600L_PER_INCH, TIFF_Y_800L_PER_INCH, TIFF_Y_1200L_PER_INCH
                                };
#if (SUPPORT_TX_COVER_PAGE)
const char gFrom[4] = {"FROM"};
CONST char gTo[2] = {"TO"};
CONST char gName[9] = {"Name:    "};
CONST char gFaxNum[9] = {"Fax:     "};
CONST char gPhone[9] = {"Phone:   "};
CONST char gAddress[9] = {"Address: "};
CONST char gCoverPageString[10] = {"COVER PAGE"};
#endif

#if SUPPORT_JPEG_CODEC
UBYTE BufferRGBOFromJpeg[TIFF_A4_WIDTH_R8 * 0x1000 * 5];
UWORD WbLinebuffer[TIFF_A4_WIDTH_R8 * 0x1000];
#endif

#if (SUPPORT_TX_COVER_PAGE)
SDWORD CoverPageDataTxCnt = 0;
SDWORD CoverPageDataCount = 0;
UBYTE CoverPageImageData[1024*1024];
#endif

#if SUPPORT_JPEG_CODEC
static const char *const cdjpeg_message_table[] =
{
#include "cderror.h"
    NULL
};

Txyz x_map[256], y_map[256], z_map[256];
TImgProDteStruct imgdata;
djpeg_dest_ptr gdest_mgr;
FILE *fJpegFile;
SDWORD OutCompressJpegCon;
struct jpeg_decompress_struct dinfo;
struct jpeg_error_mgr djerr;
SDWORD StartIndexTiffData = 0;
SDWORD ValidLengthTiffData = 0;
int sgTiffFileOffset = 0;

static djpeg_dest_ptr DecodeJpegFileToBiLevelBWBufferInit(FILE *fJpeg, struct jpeg_decompress_struct *pDeJpeg, struct jpeg_error_mgr *pJerr);
static void ConvertRGB2Y(struct jpeg_decompress_struct *pDeJpeg, UBYTE *rgboBuf);
static int ReadTiffFilePageAndWriteToJpegFile(FILE *fTif, int TiffFileOffset, FILE *fJpegFile);
extern SWORD GetScanLineLengthInPixels(UBYTE PaperSize, SWORD XRes);
extern SDWORD GetCompression(UBYTE Compression);
extern int t4Encoder_init(t4EncoderStruct *pT4Encoder, int pageWidth, int isLSBfirst, int compressionMode, int parameterK, int minimumScanLineBits, UWORD *pReferenceLine);
extern int t4Encoder_apply(t4EncoderStruct *pT4Encoder, UWORD *pLine, UWORD *pReferenceLine, UBYTE *pOut);
extern GLOBAL(djpeg_dest_ptr) jinit_write_jpeg2rgb(j_decompress_ptr cinfo);
extern SDWORD Get2DEncodeK(UBYTE yRes);
#endif

void TiffInit(TTiffStruct *pTiff)
{
    memset(pTiff, 0, sizeof(TTiffStruct));
}

#if (SUPPORT_TX_COVER_PAGE)
void TiffInitPartButCoverPage(TTiffStruct *pTiff)
{
    TCoverpageStruct CoverPagetemp;

    memcpy(&CoverPagetemp, &(pTiff->CoverPage), sizeof(TCoverpageStruct));
    TiffInit(pTiff);
    memcpy(&(pTiff->CoverPage), &CoverPagetemp, sizeof(TCoverpageStruct));
}
#endif

void TiffInitPart(TTiffStruct *pTiff)
{
    memset(&(pTiff->TiffCtrl), 0, sizeof(TiffStruct));
    memset(&(pTiff->pageInfo), 0, sizeof(T4InfoStruct));
#if SUPPORT_FILE_OPERATION
    memset(pTiff->TiffData, 0, sizeof(pTiff->TiffData));
#endif
}


void TiffInitPart1(TTiffStruct *pTiff)
{
#if SUPPORT_FILE_OPERATION
    FILE *fTiffFileTxPollb[MAX_NUM_TX_TIFF_FILES_POLL];
    FILE *fTiffFileTxb[MAX_NUM_TX_TIFF_FILES];
    SBYTE i;

    for (i = 0; i < MAX_NUM_TX_TIFF_FILES_POLL; i++)
    {
        fTiffFileTxPollb[i] = pTiff->fTiffFileTxPoll[i];
    }

    for (i = 0; i < MAX_NUM_TX_TIFF_FILES; i++)
    {
        fTiffFileTxb[i] = pTiff->fTiffFileTx[i];
    }

#endif
    TiffInit(pTiff);
#if SUPPORT_FILE_OPERATION

    for (i = 0; i < MAX_NUM_TX_TIFF_FILES_POLL; i++)
    {
        pTiff->fTiffFileTxPoll[i] = fTiffFileTxPollb[i];
    }

    for (i = 0; i < MAX_NUM_TX_TIFF_FILES; i++)
    {
        pTiff->fTiffFileTx[i] = fTiffFileTxb[i];
    }

#endif
}

int ZeroNumofEndofJpeg(UBYTE *pData)
{
    int i = 0;

    for (;;)
    {
        if ((*(pData - 1) == 0xff) && (*pData == 0xD9))
        {
            break;
        }

        pData--;

        i++;

        if (i > 300)
        {
            break;
        }
    }

    return i;
}


int CountJpegLength(UBYTE *pData, int len)
{
    int i, value = 0, flag = 0, pos = 0, flag1 = 0;

    for (i = 0; i < len - 6; i++)
    {
        if ((flag == 0) && (pData[i] == 0xFF) && (pData[i + 1] == 0xC0))
        {
            value = (int)((pData[i + 5] << 8) | pData[i + 6]);

            if (value == 0)
            {
                flag = 1;
                pos = i + 5;
            }
            else
            {
                break;
            }
        }

        if (flag)
        {
            if ((pData[i] == 0xFF) && (pData[i + 1] == 0xDC))
            {
                value = (int)((pData[i + 4] << 8) | pData[i + 5]);
                pData[pos] = pData[i + 4];
                pData[pos + 1] = pData[i + 5];
                flag1 = 1;
                break;
            }
        }
    }

    if ((pos != 0) && (flag1 == 0))
    {
        value = JPEG_IMAGE_DEFAULT_LEN;
        pData[pos] = (JPEG_IMAGE_DEFAULT_LEN >> 8) & 0xff;
        pData[pos + 1] = JPEG_IMAGE_DEFAULT_LEN & 0xff;
    }

    return value;
}



#if SUPPORT_FILE_OPERATION
SWORD RxTiffHeader(TTiffStruct *pTiff, FILE *fFile)
{
    SWORD Rev = 0;

    Rev = fread(pTiff->TiffData, 1, TIFF_HEADER_LENGTH, fFile);
    Rev = TIFF_ReadHeader(&(pTiff->TiffCtrl), pTiff->TiffData, (UDWORD)Rev);

    if (Rev != 0)
    {
#if (SUPPORT_TX_PURE_TEXT_FILE)
        TRACE0("TIFF: Transmit text file.");
        pTiff->TxFileType = TIFF_TX_TEXT_FILE;
        rewind(fFile);
#else
        TRACE0("TIFF: Incorrect tiff file header.");
#endif
    }
    else
    {
        TRACE0("TIFF: Transmit tiff file.");
        pTiff->TxFileType = TIFF_TX_TIFF_FILE;
    }

    return Rev;
}
#endif

SWORD RxTiffHeaderInBuffer(TTiffStruct *pTiff)
{
    SWORD Rev;
#if SUPPORT_FILE_OPERATION
    memcpy(pTiff->TiffData, pTiff->pTiffFileBuf + pTiff->Offset, TIFF_HEADER_LENGTH);
#endif

#if SUPPORT_FILE_OPERATION
    Rev = TIFF_ReadHeader(&(pTiff->TiffCtrl), pTiff->TiffData, TIFF_HEADER_LENGTH);
#else
    Rev = TIFF_ReadHeader(&(pTiff->TiffCtrl), pTiff->pTiffFileBuf + pTiff->Offset, TIFF_HEADER_LENGTH);
#endif

    if (Rev != 0)
    {
#if (SUPPORT_TX_PURE_TEXT_FILE)
        TRACE0("TIFF: Transmit text file.");
        pTiff->TxFileType = TIFF_TX_TEXT_FILE;
#else
        TRACE0("TIFF: Incorrect tiff file header.");
#endif
    }
    else
    {
        TRACE0("TIFF: Transmit tiff file.");
        pTiff->TxFileType = TIFF_TX_TIFF_FILE;
        pTiff->Offset += TIFF_HEADER_LENGTH;
    }

    return Rev;
}

#if SUPPORT_FILE_OPERATION
SWORD RxTiffPageParas(TTiffStruct *pTiff, FILE *fFile)
{
    UDWORD Len = 1;

    SWORD Rev = 1;
    UDWORD temp = 0;

    pTiff->TiffCtrl.length = TIFF_NUMBER_OF_IFD_LENGTH;

    while (Rev != 0)
    {
        Len = pTiff->TiffCtrl.length;
        temp = fread(pTiff->TiffData, 1, Len, fFile);

        if (temp < Len)
        {
            Rev = TIFF_NEED_MORE_DATA;
            return Rev;
        }

        Rev = TIFF_ReadIfd(&(pTiff->TiffCtrl), &(pTiff->pageInfo), pTiff->TiffData, Len);

        if (Rev != 0)
        {
            if (Rev == TIFF_NEED_MORE_DATA)
            {
                temp = 0 - Len;

                if (fseek(fFile, temp, SEEK_CUR) != 0)
                {
                    Rev = TIFF_TIFF_FILE_OPERATION_ERR;
                    return Rev;
                }
            }
            else
            {
                return Rev;
            }
        }
    }

    rewind(fFile);
    fseek(fFile, pTiff->TiffCtrl.offsetPage, SEEK_SET); // point to the beginning of the page data
    pTiff->TiffCtrl.length = 0;
    return Rev;
}
#endif


SWORD RxTiffPageParasInBuffer(TTiffStruct *pTiff)
{
    UDWORD Len = 1;

    SWORD Rev = 1;
    //UDWORD temp;

    pTiff->TiffCtrl.length = TIFF_NUMBER_OF_IFD_LENGTH;

    while (Rev != 0)
    {
        Len = pTiff->TiffCtrl.length;
        /*temp = fread(pTiff->TiffData, 1, Len, fFile);
        if (temp < Len)
        {
            Rev = TIFF_NEED_MORE_DATA;
            return Rev;
        }*/
#if SUPPORT_FILE_OPERATION
        memcpy(pTiff->TiffData, pTiff->pTiffFileBuf + pTiff->Offset, Len);
        pTiff->Offset += Len;
        Rev = TIFF_ReadIfd(&(pTiff->TiffCtrl), &(pTiff->pageInfo), pTiff->TiffData, Len);
#else
        Rev = TIFF_ReadIfd(&(pTiff->TiffCtrl), &(pTiff->pageInfo), pTiff->pTiffFileBuf + pTiff->Offset, Len);
        pTiff->Offset += Len;
#endif

        if (Rev != 0)
        {
            if (Rev == TIFF_NEED_MORE_DATA)
            {
                /*temp = 0 - Len;
                if (fseek(fFile, temp, SEEK_CUR) != 0)
                {
                    Rev = TIFF_TIFF_FILE_OPERATION_ERR;
                    return Rev;
                }*/
                pTiff->Offset -= Len;

                if (pTiff->Offset < 0)
                {
                    TRACE0("TIFF: Tiff file reading error!");
                    Rev = TIFF_TIFF_FILE_OPERATION_ERR;
                    return Rev;
                }
            }
            else
            {
                return Rev;
            }
        }
    }

    //rewind(fFile);
    //fseek(fFile, pTiff->TiffCtrl.offsetPage, SEEK_SET); // point to the beginning of the page data
    pTiff->Offset = 0;
    pTiff->Offset += pTiff->TiffCtrl.offsetPage;
    pTiff->TiffCtrl.length = 0;
    return Rev;
}

#if (SUPPORT_TX_PURE_TEXT_FILE)
void SendTextDataToDsm(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    T4InfoStruct *pPageInfo = &(pTiff->pageInfo);
    TiffStruct  *pTiffCtrl = &(pTiff->TiffCtrl);
    SDWORD numCharsPerLine = (pPageInfo->imageWidth - MARGIN_WIDTH - MARGIN_WIDTH) / 18;

    //SDWORD Len = MIN32((pTiff->TxFileTotalLength - pPageInfo->pageByteCount), MAX_LEN_API_MSG);
    SDWORD Len = MIN32(numCharsPerLine, MAX_LEN_API_MSG);
    SDWORD i = 0, Len1 = 0;
#if SUPPORT_FILE_OPERATION
    SDWORD EndofTextFile = 0;
#endif

    if ((pTiffCtrl->length == 0) || ((pPageInfo->imageLength - pTiffCtrl->length) <= LINE_HEIGHT))
    {
#if SUPPORT_FILE_OPERATION
        memset(pTiff->TiffData, ' ', Len);
#else
        memset(&(pTiffDsm->Buf[3]), ' ', Len);
#endif
    }
    else
    {
#if SUPPORT_FILE_OPERATION
        memset(pTiff->TiffData, ' ', Len);

        if (pTiff->FilePointer == 1)
        {
            UBYTE tdata;

            for (i = 0; i < Len; i++)
            {
                if (fread(&(pTiff->TiffData[i]), 1, 1, pTiff->fTiffFileTxCurr) == 1)
                {
                    if (pTiff->TiffData[i] == 0x0d)
                    {
                        pTiff->TiffData[i] = ' ';
                        i++;

                        if (fread(&tdata, 1, 1, pTiff->fTiffFileTxCurr) == 1)
                        {
                            if (tdata == 0x0a)
                            {
                                i++;
                            }
                            else
                            {
                                fseek(pTiff->fTiffFileTxCurr, -1, SEEK_CUR);
                            }
                        }
                        else
                        {
                            EndofTextFile = 1;
                        }

                        break;
                    }
                }
                else
                {
                    EndofTextFile = 1;
                    break;
                }
            }
        }
        else
        {
            for (i = 0; i < Len; i++)
            {
                if (pTiff->TxFileTotalLength >= (pPageInfo->pageByteCount + i + 1))
                {
                    memcpy(&(pTiff->TiffData[i]), pTiff->pTiffFileBuf + pTiff->Offset, 1);
                    pTiff->Offset++;

                    if (pTiff->TiffData[i] == 0x0d)
                    {
                        pTiff->TiffData[i] = ' ';
                        i++;

                        if (pTiff->TxFileTotalLength >= (pPageInfo->pageByteCount + i + 2))
                        {
                            if (*(pTiff->pTiffFileBuf + pTiff->Offset) == 0x0a)
                            {
                                pTiff->Offset++;
                                i++;
                            }
                        }

                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        Len1 = i;
#else
        memset(&(pTiffDsm->Buf[3]), ' ', Len);

        for (i = 0; i < Len; i++)
        {
            if (pTiff->TxFileTotalLength >= (pPageInfo->pageByteCount + i + 1))
            {
                memcpy(&(pTiffDsm->Buf[3+i]), pTiff->pTiffFileBuf + pTiff->Offset, 1);
                pTiff->Offset++;

                if (pTiffDsm->Buf[3+i] == 0x0d)
                {
                    pTiffDsm->Buf[3+i] = ' ';
                    i++;

                    if (pTiff->TxFileTotalLength >= (pPageInfo->pageByteCount + i + 2))
                    {
                        if (*(pTiff->pTiffFileBuf + pTiff->Offset) == 0x0a)
                        {
                            pTiff->Offset++;
                            i++;
                        }
                    }

                    break;
                }
            }
            else
            {
                break;
            }
        }

        Len1 = i;
#endif
    }

    pPageInfo->pageByteCount += Len1;
#if SUPPORT_FILE_OPERATION

    if (EndofTextFile == 1)
    {
        pPageInfo->isLastPage = 1;
        pTiffCtrl->length = pPageInfo->imageLength;
    }
    else
    {
        pPageInfo->isLastPage = 0;
    }

#else

    if (pTiff->TxFileTotalLength <= pPageInfo->pageByteCount)
    {
        pPageInfo->isLastPage = 1;
    }
    else
    {
        pPageInfo->isLastPage = 0;
    }

#endif

    pTiffCtrl->length += LINE_HEIGHT;
    pTiffDsm->Direction = MSG_DOWN_LINK;
    pTiffDsm->MsgType = MSG_TYPE_IMAGE_DATA;
    pTiffDsm->MsgCode = IMG_DATA_AND_STATUS;

    if (((UDWORD)pPageInfo->imageLength > pTiffCtrl->length) && (pPageInfo->isLastPage == 0))
    {
        pTiffDsm->Buf[0] = TIFF_PMC_MIDDLE_OF_PAGE;
    }
    else
    {
        if (pPageInfo->isLastPage == 1)
        {
            if (pTiff->FilePointer)
            {
#if SUPPORT_FILE_OPERATION

                if (pTiff->LastTiffFileTx == 1)
                {
                    if (pTiff->RemoteHasDocTx == 1)
                    {
                        pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOM;
                    }
                    else
                    {
                        pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOP;
                    }

                    pTiff->RemoteHasDocTx = 0;
                }
                else
                {
                    pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOM;
                }

#endif
            }
            else
            {
                pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOP;
            }
        }
        else
        {
            pTiffDsm->Buf[0] = TIFF_PMC_NEXT_PAGE;
        }
    }

    pTiffDsm->Buf[1] = (UBYTE)((Len >> 8) & 0xff);
    pTiffDsm->Buf[2] = (UBYTE)(Len & 0xff);
#if SUPPORT_FILE_OPERATION
    memcpy(&(pTiffDsm->Buf[3]), pTiff->TiffData, Len);
#endif
}
#endif

void SendImgDataToDsm(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    T4InfoStruct *pPageInfo = &(pTiff->pageInfo);
    TiffStruct  *pTiffCtrl = &(pTiff->TiffCtrl);

    UDWORD Len = MIN32((pTiffCtrl->lengthPage - pTiffCtrl->length), MAX_LEN_API_MSG);

#if SUPPORT_FILE_OPERATION

    if (pTiff->FilePointer == 1)
    {
        fread(pTiff->TiffData, 1, Len, pTiff->fTiffFileTxCurr);
    }
    else
    {
        memcpy(pTiff->TiffData, pTiff->pTiffFileBuf + pTiff->Offset, Len);
        pTiff->Offset += Len;
    }

#else
    memcpy(&(pTiffDsm->Buf[3]), pTiff->pTiffFileBuf + pTiff->Offset, Len);
    pTiff->Offset += Len;
#endif

    pTiffCtrl->length += Len;

    pTiffDsm->Direction = MSG_DOWN_LINK;
    pTiffDsm->MsgType = MSG_TYPE_IMAGE_DATA;
    pTiffDsm->MsgCode = IMG_DATA_AND_STATUS;

    if (pTiffCtrl->lengthPage > pTiffCtrl->length)
    {
        pTiffDsm->Buf[0] = TIFF_PMC_MIDDLE_OF_PAGE;
    }
    else
    {
        if (pPageInfo->isLastPage == 1)
        {
            if (pTiff->FilePointer)
            {
#if SUPPORT_FILE_OPERATION

                if (pTiff->LastTiffFileTx == 1)
                {
                    if (pTiff->RemoteHasDocTx == 1)
                    {
                        pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOM;
                    }
                    else
                    {
                        pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOP;
                    }

                    pTiff->RemoteHasDocTx = 0;
                }
                else
                {
                    pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOM;
                }

#endif
            }
            else
            {
                pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOP;
            }
        }
        else
        {
            pTiffDsm->Buf[0] = TIFF_PMC_NEXT_PAGE;
        }

        if (pTiff->FilePointer)
        {
#if SUPPORT_FILE_OPERATION
            rewind(pTiff->fTiffFileTxCurr);
            fseek(pTiff->fTiffFileTxCurr, pTiffCtrl->offsetIfd, SEEK_SET); // point to the beginning of next page parameters
#endif
        }
        else
        {
            pTiff->Offset = pTiffCtrl->offsetIfd;
        }
    }

    pTiffDsm->Buf[1] = (UBYTE)((Len >> 8) & 0xff);
    pTiffDsm->Buf[2] = (UBYTE)(Len & 0xff);
#if SUPPORT_FILE_OPERATION
    memcpy(&(pTiffDsm->Buf[3]), pTiff->TiffData, Len);
#endif
}

#if SUPPORT_JPEG_CODEC
#if SUPPORT_FILE_OPERATION
int TxTiffFileJpegConversionT4T6_Init(TTiffStruct *pTiff, SDWORD compress)
{
    int Rev = -1;
    SDWORD OutImageW, OutCompression;

    memset(&imgdata, 0, sizeof(TImgProDteStruct));

    if ((fJpegFile = fopen("image.jpg", "wb")) == 0)
    {
        TRACE0("TIF: Can not create image.jpg!\n");
        return Rev;
    }

    sgTiffFileOffset = ReadTiffFilePageAndWriteToJpegFile(pTiff->fTiffFileTxCurr, sgTiffFileOffset, fJpegFile);

    if (sgTiffFileOffset == -1)
    {
        TRACE0("TIF: wrong format of the input tif file during converting tiff to bmp.");
        fclose(fJpegFile);
        return Rev;
    }
    else if (sgTiffFileOffset == 0)
    {
        TRACE0("TIF: last page of the tif file.");
        Rev = 0;
    }
    else
    {
        TRACE0("TIF: The tif file still has next page.");
        Rev = 1;
    }

    fclose(fJpegFile);

    if ((fJpegFile = fopen("image.jpg", "rb")) == 0)
    {
        TRACE0("TIF: image.jpg missed!\n");
        return Rev;
    }

    memset(imgdata.Bitmap, 0, sizeof(imgdata.Bitmap));
    imgdata.pDecoderLine = imgdata.Bitmap + (0 * BITMAP_LENGTH);
    imgdata.pDecoderReferenceLine = imgdata.Bitmap + (1 * BITMAP_LENGTH);
    imgdata.pEncoderLine = imgdata.Bitmap + (2 * BITMAP_LENGTH);
    imgdata.pEncoderReferenceLine = imgdata.Bitmap + (3 * BITMAP_LENGTH);
    OutImageW = GetScanLineLengthInPixels((UBYTE)pTiff->PaperSize, pTiff->XRes);
    OutCompression = GetCompression((UBYTE)compress);

    if (t4Encoder_init(&(imgdata.t4Encoder), OutImageW, 1, OutCompression, Get2DEncodeK((UBYTE)pTiff->YRes), 0, imgdata.pEncoderReferenceLine) < 0)
    {
        TRACE0("TIF: Tiff encoder init failed.");
        return Rev;
    }

    gdest_mgr = DecodeJpegFileToBiLevelBWBufferInit(fJpegFile, &dinfo, &djerr);
    StartIndexTiffData = 0;
    ValidLengthTiffData = 0;
    return Rev;
}
#endif
#endif

void TiffMsgProUplink(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    T4InfoStruct *pPageInfo = &(pTiff->pageInfo);

    switch (pTiffDsm->MsgCode)
    {
        case RPT_REMOTE_POLL_IND:
            pTiff->RemoteHasDocTx = 1;
            break;
        case RPT_TIFF_FILE_RUN_MODE:

            if ((pTiffDsm->Buf[0] == DSM_RX_TIFF_FILE) || (pTiffDsm->Buf[0] == DSM_RX_TIFF_FILE_BY_POLL))
            {
                pTiff->Direction = TIFF_FILE_RECEIVE;
#if SUPPORT_FILE_OPERATION
                pTiff->fTiffFileRx = 0;
#endif
            }
            else
            {
                pTiff->Direction = TIFF_FILE_TRANSMIT;

                if (pTiffDsm->Buf[1] >= MAX_NUM_TX_TIFF_FILES)
                {
                    TRACE0("TIFF: Wrong Tx tiff file number requested by DSM!");
                }
                else
                {
                    if (pTiff->FilePointer == 1)
                    {
#if SUPPORT_FILE_OPERATION

                        if ((pTiffDsm->Buf[0] == DSM_TX_TIFF_FILE) || (pTiffDsm->Buf[0] == DSM_RE_TX_TIFF_FILE))
                        {
                            pTiff->fTiffFileTxCurr = pTiff->fTiffFileTx[pTiffDsm->Buf[1]];
                            pTiff->LastTiffFileTx = 0;

                            if (pTiffDsm->Buf[1] >= (MAX_NUM_TX_TIFF_FILES - 1))
                            {
                                pTiff->LastTiffFileTx = 1;
                            }
                            else
                            {
                                if (pTiff->fTiffFileTx[pTiffDsm->Buf[1] + 1] == NULL)
                                {
                                    pTiff->LastTiffFileTx = 1;
                                }
                            }
                        }
                        else
                        {
                            pTiff->fTiffFileTxCurr = pTiff->fTiffFileTxPoll[pTiffDsm->Buf[1]];
                            pTiff->LastTiffFileTx = 0;

                            if (pTiffDsm->Buf[1] >= (MAX_NUM_TX_TIFF_FILES_POLL - 1))
                            {
                                pTiff->LastTiffFileTx = 1;
                            }
                            else
                            {
                                if (pTiff->fTiffFileTxPoll[pTiffDsm->Buf[1] + 1] == NULL)
                                {
                                    pTiff->LastTiffFileTx = 1;
                                }
                            }
                        }

#endif
                    }
                }
            }

            break;
        case RPT_RESULT_CODE:
            pTiff->ResultCode = pTiffDsm->Buf[0];

            if ((pTiff->ResultCode == DSM_RC_OK) && (pTiff->State == TIFF_STATE_WAIT_OK))
            {
                memset(pTiffDsm, 0, sizeof(GAO_Msg));
            }

            break;
        case RPT_RECV_POST_PAGE_RESP:
            pTiff->PostMsgResp = pTiffDsm->Buf[0];
            pTiff->RxLineNumber = ((pTiffDsm->Buf[1] << 8) & 0xff00) + (pTiffDsm->Buf[2] & 0xff);
            pTiff->pageInfo.imageLength = pTiff->RxLineNumber;
            //TRACE1("APP: This Page has %d lines.", pTiff->RxLineNumber);
            break;
        case RPT_RECV_POST_PAGE_CMD:
            pTiff->PostMsgCmd = pTiffDsm->Buf[0];
            break;
        case RPT_HANGUP_STATUS:
            TiffInitPart1(pTiff);
            break;
        case RPT_NEGOTIATED_PARAS:
            pTiff->PaperSize = pTiffDsm->Buf[0];
            pTiff->XRes = pTiffDsm->Buf[1];
            pTiff->YRes = pTiffDsm->Buf[2];
            pTiff->EcmOn = pTiffDsm->Buf[5];
            pPageInfo->photometric = TIFF_PHOTOMETIC_0_IS_WHITE;
#if SUPPORT_JPEG_CODEC
#if SUPPORT_FILE_OPERATION

            if (((pPageInfo->compression == JPEG_COLOR) || (pPageInfo->compression == JPEG_GRAY)) && (pTiffDsm->Buf[3] != DSM_DF_JPEG_MODE))
            {
                if (pTiff->TxFileType == TIFF_TX_TIFF_FILE)
                {
                    pTiff->TxFileType = TIFF_TX_JPEG_CONVERSION_FILE;
                    OutCompressJpegCon = pTiffDsm->Buf[3];
                    pTiff->pageInfo.isLastPage = -1;

                    if (TxTiffFileJpegConversionT4T6_Init(pTiff, OutCompressJpegCon) < 0)
                    {
                        TRACE0("TIF: Conversion init failed from jpeg to T4/T6.");
                        break;
                    }
                }
            }

#endif
#endif
            pPageInfo->compression = pTiffDsm->Buf[3];

            if (pPageInfo->compression == DSM_DF_T6_2D)
            {
                pPageInfo->compression = T6_2D_MMR;
            }
            else if (pPageInfo->compression == DSM_DF_JPEG_MODE)
            {
                if (pTiffDsm->Buf[7] & 2) // full color
                {
                    pPageInfo->compression = JPEG_COLOR;

                    if (pTiffDsm->Buf[7] & 0x40)
                    {
                        pPageInfo->photometric = CS_CIELAB8;    // or CS_CIELAB9, need to check jpeg image data
                    }
                    else
                    {
                        pPageInfo->photometric = CS_CIELAB10;
                    }
                }
                else // gray
                {
                    pPageInfo->compression = JPEG_GRAY;
                    pPageInfo->photometric = TIFF_PHOTOMETIC_1_IS_WHITE;
                }
            }

            if ((pTiff->XRes > 0) && (pTiff->XRes < 10))
            {
                pPageInfo->xResolution = XResoValue_Tiff[pTiff->XRes - 1];

                if (pTiff->XRes < 4)
                {
                    pPageInfo->resolutionUnit = TIFF_RESOLUTION_UNIT_CENTI;
                }
                else
                {
                    pPageInfo->resolutionUnit = TIFF_RESOLUTION_UNIT_INCHES;
                }
            }
            else
            {
                pPageInfo->xResolution = XResoValue_Tiff[1];
                pPageInfo->resolutionUnit = TIFF_RESOLUTION_UNIT_CENTI;
                TRACE0("APP: x Resolution ERROR!");
            }

            if ((pTiff->YRes > 0) && (pTiff->YRes < 11))
            {
                pPageInfo->yResolution = YResoValue_Tiff[pTiff->YRes - 1];

                if (pTiff->YRes < 4)
                {
                    pPageInfo->resolutionUnit = TIFF_RESOLUTION_UNIT_CENTI;
                }
                else
                {
                    pPageInfo->resolutionUnit = TIFF_RESOLUTION_UNIT_INCHES;
                }
            }
            else
            {
                pPageInfo->yResolution = YResoValue_Tiff[0];
                pPageInfo->resolutionUnit = TIFF_RESOLUTION_UNIT_CENTI;
                TRACE0("APP: y Resolution ERROR!");
            }

            if (pTiff->PaperSize == DSM_A3)
            {
                pPageInfo->imageWidth = ImageWidth_Tiff[pTiff->PaperSize - 2 ][pTiff->XRes - 2];
            }
            else
            {
                pPageInfo->imageWidth = ImageWidth_Tiff[pTiff->PaperSize - 1 ][pTiff->XRes - 2];
            }

            pPageInfo->totalPages     = 0;
            pPageInfo->pageNumber     = 0;

            if (pTiff->Direction == TIFF_FILE_TRANSMIT)
            {
                if (pPageInfo->resolutionUnit == TIFF_RESOLUTION_UNIT_CENTI)
                {
                    if (pTiff->PaperSize == DSM_A4_LETTER_LEGAL)
                    {
                        pPageInfo->imageLength = (SDWORD)(TIFF_A4_LENGTH_METRIC / 10.0 * pPageInfo->yResolution);
                    }
                    else if (pTiff->PaperSize == DSM_B4)
                    {
                        pPageInfo->imageLength = (SDWORD)(TIFF_B4_LENGTH_METRIC / 10.0 * pPageInfo->yResolution);
                    }
                    else if (pTiff->PaperSize == DSM_A3)
                    {
                        pPageInfo->imageLength = (SDWORD)(TIFF_A3_LENGTH_METRIC / 10.0 * pPageInfo->yResolution);
                    }
                }
                else
                {
                    if (pTiff->PaperSize == DSM_A4_LETTER_LEGAL)
                    {
                        pPageInfo->imageLength = (SDWORD)(TIFF_A4_LENGTH_METRIC / 25.4 * pPageInfo->yResolution);
                    }
                    else if (pTiff->PaperSize == DSM_B4)
                    {
                        pPageInfo->imageLength = (SDWORD)(TIFF_B4_LENGTH_METRIC / 25.4 * pPageInfo->yResolution);
                    }
                    else if (pTiff->PaperSize == DSM_A3)
                    {
                        pPageInfo->imageLength = (SDWORD)(TIFF_A3_LENGTH_METRIC / 25.4 * pPageInfo->yResolution);
                    }
                }
            }

            break;
    }
}

#if SUPPORT_FILE_OPERATION
void TiffMsgCheckTifffilesToBeSent(GAO_Msg *pAppTiff, FILE **pFile, TTiffStruct *pTiff)
{
    int i = 0, j = 0;
    SBYTE value = MIN16(pAppTiff->Buf[1], MAX_NUM_TX_TIFF_FILES);

    for (; i < value; i++)
    {
        pFile[i] = (FILE *)((((pAppTiff->Buf[2 + i*4]) << 24) & 0xff000000) + (((pAppTiff->Buf[3 + i*4]) << 16) & 0x00ff0000) + (((pAppTiff->Buf[4 + i*4]) << 8) & 0x0000ff00) + ((pAppTiff->Buf[5 + i*4]) & 0xff));

        if ((pTiff->TxFileCheckResults[i] = RxTiffHeader(pTiff, pFile[i])) == 0)
        {
            pTiff->TxFileCheckResults[i] = RxTiffPageParas(pTiff, pFile[i]);
        }

#if (SUPPORT_TX_PURE_TEXT_FILE)
        pTiff->TxFileCheckResults[i] = 0; // if not tiff file, then assume that it is a text file.
#endif

        if (pTiff->TxFileCheckResults[i] == 0)
        {
            pTiff->TxFileCheckResults[i] = 1;
            pFile[j++] = pFile[i];
            rewind(pFile[i]);
        }
        else
        {
            pFile[i] = 0;
        }

        if (pTiff->State == TIFF_STATE_CHECK_PARAS)
        {
            TiffInitPart(pTiff);
        }
    }
}
#endif



void TiffMsgCheckTifffilesInBufferToBeSent(GAO_Msg *pAppTiff, TTiffStruct *pTiff)
{
    int i = 0;
    //SBYTE value = MIN16(pAppTiff->Buf[1], MAX_NUM_TX_TIFF_FILES);

    //for (; i < value; i++)
    {
        pTiff->pTiffFileBuf = (UBYTE *)((((pAppTiff->Buf[2 + i*4]) << 24) & 0xff000000) + (((pAppTiff->Buf[3 + i*4]) << 16) & 0x00ff0000) + (((pAppTiff->Buf[4 + i*4]) << 8) & 0x0000ff00) + ((pAppTiff->Buf[5 + i*4]) & 0xff));
        pTiff->TxFileTotalLength = ((pTiff->pTiffFileBuf[0] << 24) & 0xff000000) + ((pTiff->pTiffFileBuf[1] << 16) & 0x00ff0000) + ((pTiff->pTiffFileBuf[2] << 8) & 0x0000ff00) + ((pTiff->pTiffFileBuf[3] << 0) & 0xff);
        pTiff->pTiffFileBuf += 4;

        if ((pTiff->TxFileCheckResults[i] = RxTiffHeaderInBuffer(pTiff)) == 0)
        {
            pTiff->TxFileCheckResults[i] = RxTiffPageParasInBuffer(pTiff);
        }

#if (SUPPORT_TX_PURE_TEXT_FILE)
        pTiff->TxFileCheckResults[i] = 0; // if not tiff file, then assume that it is a text file.
#endif

        if (pTiff->TxFileCheckResults[i] == 0)
        {
            pTiff->TxFileCheckResults[i] = 1;
            //rewind(pFile[i]);
            pTiff->Offset = 0;
        }
        else
        {
            pTiff->pTiffFileBuf = 0;
        }

        if (pTiff->State == TIFF_STATE_CHECK_PARAS)
        {
            TiffInitPart(pTiff);
        }
    }
}

#if (SUPPORT_TX_COVER_PAGE)
void TiffMsgCoverPageCfg(TTerminalDetailsStruct *pDetail, GAO_Msg *pAppTiff)
{
    if (pAppTiff->Buf[1] == TIFF_COVER_PAGE_NAME)
    {
        if (pAppTiff->Buf[2] <= MAX_LEN_TERMINAL_NAME)
        {
            memset(pDetail->Name, 0, sizeof(pDetail->Name));
            memcpy(pDetail->Name, &(pAppTiff->Buf[3]), pAppTiff->Buf[2]);
        }
        else
        {
            TRACE0("TIFF: Cover page name too long!");
        }
    }
    else if (pAppTiff->Buf[1] == TIFF_COVER_PAGE_FAX_NUMBER)
    {
        if (pAppTiff->Buf[2] <= MAX_LEN_TERMINAKL_PHONE_NUM)
        {
            memset(pDetail->FaxNumber, 0, sizeof(pDetail->FaxNumber));
            memcpy(pDetail->FaxNumber, &(pAppTiff->Buf[3]), pAppTiff->Buf[2]);
        }
        else
        {
            TRACE0("TIFF: Cover page fax number too long!");
        }
    }
    else if (pAppTiff->Buf[1] == TIFF_COVER_PAGE_PHONE_NUMBER)
    {
        if (pAppTiff->Buf[2] <= MAX_LEN_TERMINAKL_PHONE_NUM)
        {
            memset(pDetail->PhoneNumber, 0, sizeof(pDetail->PhoneNumber));
            memcpy(pDetail->PhoneNumber, &(pAppTiff->Buf[3]), pAppTiff->Buf[2]);
        }
        else
        {
            TRACE0("TIFF: Cover page phone number too long!");
        }
    }
    else if (pAppTiff->Buf[1] == TIFF_COVER_PAGE_STREET)
    {
        if (pAppTiff->Buf[2] <= MAX_LEN_TERMINAKL_STREET)
        {
            memset(pDetail->Street, 0, sizeof(pDetail->Street));
            memcpy(pDetail->Street, &(pAppTiff->Buf[3]), pAppTiff->Buf[2]);
        }
        else
        {
            TRACE0("TIFF: Cover page street too long!");
        }
    }
    else if (pAppTiff->Buf[1] == TIFF_COVER_PAGE_CITYSTATE)
    {
        if (pAppTiff->Buf[2] <= MAX_LEN_TERMINAKL_CITY)
        {
            memset(pDetail->CityState, 0, sizeof(pDetail->CityState));
            memcpy(pDetail->CityState, &(pAppTiff->Buf[3]), pAppTiff->Buf[2]);
        }
        else
        {
            TRACE0("TIFF: Cover page city and state too long!");
        }
    }
    else if (pAppTiff->Buf[1] == TIFF_COVER_PAGE_COUNTRY_ZIP)
    {
        if (pAppTiff->Buf[2] <= MAX_LEN_TERMINAKL_CITY)
        {
            memset(pDetail->CountryZipCode, 0, sizeof(pDetail->CountryZipCode));
            memcpy(pDetail->CountryZipCode, &(pAppTiff->Buf[3]), pAppTiff->Buf[2]);
        }
        else
        {
            TRACE0("TIFF: Cover page country and postal code too long!");
        }
    }
}
#endif

void TiffMsgProDownlink(GAO_Msg *pAppTiff, TTiffStruct *pTiff)
{
    switch (pAppTiff->MsgCode)
    {
        case CFG_TIFF_FILE_PTR_RX:
            pTiff->FilePointer = pAppTiff->Buf[0];

            if (pTiff->FilePointer == 1)
            {
#if SUPPORT_FILE_OPERATION
                pTiff->fTiffFileRx = (FILE *)((((pAppTiff->Buf[1]) << 24) & 0xff000000) + (((pAppTiff->Buf[2]) << 16) & 0x00ff0000) + (((pAppTiff->Buf[3]) << 8) & 0x0000ff00) + ((pAppTiff->Buf[4]) & 0xff));
#endif
            }
            else
            {
                pTiff->pTiffFileBuf = (UBYTE *)((((pAppTiff->Buf[1]) << 24) & 0xff000000) + (((pAppTiff->Buf[2]) << 16) & 0x00ff0000) + (((pAppTiff->Buf[3]) << 8) & 0x0000ff00) + ((pAppTiff->Buf[4]) & 0xff));
                pTiff->pTiffFileBuf += 4;
            }

            pTiff->Offset = 0;
            break;
        case CFG_TIFF_FILE_PTR_TX:
            TiffInitPart(pTiff);
            pTiff->Offset = 0;
            pTiff->FilePointer = pAppTiff->Buf[0];

            if (pTiff->FilePointer == 1)
            {
#if SUPPORT_FILE_OPERATION
                memset(pTiff->fTiffFileTx, 0, sizeof(pTiff->fTiffFileTx));
                memset(pTiff->TxFileCheckResults, 0, sizeof(pTiff->TxFileCheckResults));
                TiffMsgCheckTifffilesToBeSent(pAppTiff, pTiff->fTiffFileTx, pTiff);
#endif
            }
            else
            {
                pTiff->pTiffFileBuf = 0;
                memset(pTiff->TxFileCheckResults, 0, sizeof(pTiff->TxFileCheckResults));
                TiffMsgCheckTifffilesInBufferToBeSent(pAppTiff, pTiff);
                pTiff->Offset = 0;
            }

            break;
        case CFG_TIFF_FILE_PTR_TX_BY_POLL:
#if SUPPORT_FILE_OPERATION
            memset(pTiff->fTiffFileTxPoll, 0, sizeof(pTiff->fTiffFileTxPoll));
            memset(pTiff->TxFileCheckResults, 0, sizeof(pTiff->TxFileCheckResults));
            TiffMsgCheckTifffilesToBeSent(pAppTiff, pTiff->fTiffFileTxPoll, pTiff);
#endif
            break;
#if (SUPPORT_TX_COVER_PAGE)
        case CFG_COVER_PAGE_ATTRIB:

            if (pAppTiff->Buf[0] == TIFF_COVER_PAGE_CALLER_SIDE)
            {
                if (pAppTiff->Buf[1] == 0)
                {
                    if (pAppTiff->Buf[2] <= MAX_LEN_DATE_TIME_STRING)
                    {
                        memcpy(&(pTiff->CoverPage.DateTimeStr[0]), &(pAppTiff->Buf[3]), pAppTiff->Buf[2]);
                    }
                    else
                    {
                        memcpy(&(pTiff->CoverPage.DateTimeStr[0]), &(pAppTiff->Buf[3]), MAX_LEN_DATE_TIME_STRING);
                        TRACE0("TIFF: cover page / date and time string buffer overflow!");
                    }
                }
                else
                {
                    TiffMsgCoverPageCfg(&(pTiff->CoverPage.CallerSide), pAppTiff);
                }
            }
            else if (pAppTiff->Buf[0] == TIFF_COVER_PAGE_CALLEE_SIDE)
            {
                TiffMsgCoverPageCfg(&(pTiff->CoverPage.CalleeSide), pAppTiff);
            }
            else if (pAppTiff->Buf[0] == TIFF_COVER_PAGE_SIDE_PATTERN)
            {
                pTiff->CoverPage.Pattern = pAppTiff->Buf[3];
                CoverPageDataCount = 0;
                memset(CoverPageImageData, 0, sizeof(CoverPageImageData));
            }

            break;
#endif
    }
}

#if (SUPPORT_TX_PURE_TEXT_FILE)
void TiffSendTxTextFormat(GAO_Msg *pMsg)
{
    pMsg->Direction = MSG_DOWN_LINK;
    pMsg->MsgType = MSG_TYPE_CONFIG;
    pMsg->MsgCode = CFG_TX_IMG_FORMAT;
    pMsg->Buf[3] = DSM_DF_TEXT_FILE;
}
#endif


void TiffSendTxImgFormat(GAO_Msg *pMsg, TTiffStruct *pTiff)
{
    pMsg->Direction = MSG_DOWN_LINK;
    pMsg->MsgType = MSG_TYPE_CONFIG;
    pMsg->MsgCode = CFG_TX_IMG_FORMAT;

    if (pTiff->pageInfo.imageWidth == B4_WIDTH)
    {
        pMsg->Buf[0] = DSM_B4;
    }
    else if (pTiff->pageInfo.imageWidth == A3_WIDTH)
    {
        pMsg->Buf[0] = DSM_A3;
    }
    else
    {
        pMsg->Buf[0] = DSM_A4_LETTER_LEGAL;
    }

    if (pTiff->pageInfo.resolutionUnit == TIFF_RESOLUTION_UNIT_CENTI)
    {
        if ((pTiff->pageInfo.xResolution == 40) || (pTiff->pageInfo.xResolution == 41))
        {
            pMsg->Buf[1] = R4_X_RESOLUTION;
        }
        else if ((pTiff->pageInfo.xResolution == 80) || (pTiff->pageInfo.xResolution == 81) || (pTiff->pageInfo.xResolution == 82))
        {
            pMsg->Buf[1] = R8_X_RESOLUTION;
        }

        if ((pTiff->pageInfo.xResolution >= 160) && (pTiff->pageInfo.xResolution <= 164))
        {
            pMsg->Buf[1] = R16_X_RESOLUTION;
        }

        if ((pTiff->pageInfo.yResolution == 38) || (pTiff->pageInfo.yResolution == 39))
        {
            pMsg->Buf[2] = VR_NORMAL_385;
        }
        else if ((pTiff->pageInfo.yResolution >= 76) && (pTiff->pageInfo.yResolution <= 78))
        {
            pMsg->Buf[2] = VR_FINE_77;
        }
        else if ((pTiff->pageInfo.yResolution >= 152) && (pTiff->pageInfo.yResolution <= 156))
        {
            pMsg->Buf[2] = VR_PERFECT_154;
        }
    }
    else if (pTiff->pageInfo.resolutionUnit == TIFF_RESOLUTION_UNIT_INCHES)
    {
        int i;
        pMsg->Buf[1] = 0;

        for (i = 0; i < 9; i++)
        {
            if (pTiff->pageInfo.xResolution == XResoValue_Tiff[i + 3])
            {
                pMsg->Buf[1] = i + 4;
                break;
            }
        }

        if (pMsg->Buf[1] == 0)
        {
            TRACE0("APP: X resolution ERROR. Tx Image.");
        }

        pMsg->Buf[2] = 0;

        for (i = 0; i < 10; i++)
        {
            if (pTiff->pageInfo.yResolution == YResoValue_Tiff[i + 3])
            {
                pMsg->Buf[2] = i + 4;
                break;
            }
        }

        if (pMsg->Buf[2] == 0)
        {
            TRACE0("APP: Y resolution ERROR. Tx Image.");
        }
    }
    else
    {
        TRACE0("APP: TIFF file Resolution unit ERROR.");
    }

    pMsg->Buf[4] = pTiff->pageInfo.fillOrder;

    pMsg->Buf[5] = 0;

    if (pTiff->pageInfo.compression == T4_1D_MH)
    {
        pMsg->Buf[3] = DSM_DF_T4_1D;
    }
    else if (pTiff->pageInfo.compression == T4_2D_MR)
    {
        pMsg->Buf[3] = DSM_DF_T4_2D;
    }
    else if (pTiff->pageInfo.compression == JPEG_COLOR)
    {
        pMsg->Buf[3] = DSM_DF_JPEG_MODE;
        pMsg->Buf[5] = 1;
        pMsg->Buf[5] |= 2;

        if (pTiff->pageInfo.photometric != CS_CIELAB10)
        {
            pMsg->Buf[5] |= 0x40;
        }

        if (pTiff->pageInfo.bitNum[0] == 12)
        {
            pMsg->Buf[5] |= 0x08;
        }
    }
    else if (pTiff->pageInfo.compression == JPEG_GRAY)
    {
        pMsg->Buf[3] = DSM_DF_JPEG_MODE;
        pMsg->Buf[5] = 1;

        if (pTiff->pageInfo.photometric != CS_CIELAB10)
        {
            pMsg->Buf[5] |= 0x40;
        }

        if (pTiff->pageInfo.bitNum[0] == 12)
        {
            pMsg->Buf[5] |= 0x08;
        }
    }
    else if (pTiff->pageInfo.compression == T6_2D_MMR)
    {
        pMsg->Buf[3] = DSM_DF_T6_2D;
    }
    else
    {
        TRACE0("APP: TIFF file compression format ERROR.");
    }
}

void ReportTxTiffFileCheckResults(GAO_Msg *pAppTiff, TTiffStruct *pTiff)
{
    SBYTE i;

    if (pAppTiff->Direction == MSG_DIR_IDLE)
    {
        pAppTiff->Direction = MSG_UP_LINK;
        pAppTiff->MsgType = MSG_TYPE_REPORT;
        pAppTiff->MsgCode = RPT_VALIDITY_OF_TX_TIFF_FILE;

        for (i = 0; i < MAX_NUM_TX_TIFF_FILES; i++)
        {
            if (pTiff->TxFileCheckResults[i] == 1)
            {
                pAppTiff->Buf[1+i] = 0;
                TRACE0("TIFF: Tx tiff file check OK.");
            }
            else if (pTiff->TxFileCheckResults[i] == 0)
            {
                break;
            }
            else
            {
                pAppTiff->Buf[1+i] = 0 - pTiff->TxFileCheckResults[i];
                TRACE1("TIFF: Tx tiff file check not OK, error code is %d.", pAppTiff->Buf[1+i]);
            }

            pTiff->TxFileCheckResults[i] = 0;
        }

        pAppTiff->Buf[0] = i;
    }
}

#if (SUPPORT_TX_COVER_PAGE)
void TiffSendTxCoverPageGAODefault1(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    TCoverpageStruct *pCoverPage = &(pTiff->CoverPage);
    T4InfoStruct *pPageInfo = &(pTiff->pageInfo);
    TiffStruct  *pTiffCtrl = &(pTiff->TiffCtrl);
    SDWORD numCharsPerLine = (pPageInfo->imageWidth - MARGIN_WIDTH - MARGIN_WIDTH) / 18;
    SDWORD Len = MIN32(numCharsPerLine, MAX_LEN_API_MSG);
    UDWORD CharLineNum = pPageInfo->imageLength / LINE_HEIGHT;
    UDWORD CharLineCnt = pTiffCtrl->length / LINE_HEIGHT;
    SDWORD StartIndex1 = 6, StartIndex2 = (Len - 3) / 2 + 7;
    UDWORD ToStart = CharLineNum * 2 / 3;

    pTiffDsm->Buf[3] = TIFF_VERTICAL;
    memset(&(pTiffDsm->Buf[4]), ' ', Len - 2);
    pTiffDsm->Buf[Len+2] = TIFF_VERTICAL;

    if ((CharLineCnt > 4) && (CharLineCnt < ToStart - 4))
    {
        pTiffDsm->Buf[7] = TIFF_VERTICAL;
        pTiffDsm->Buf[Len-2] = TIFF_VERTICAL;
    }

    if (CharLineCnt == 0)
    {
        pTiffDsm->Buf[3] = TIFF_LEFT_UP;
        memset(&(pTiffDsm->Buf[4]), TIFF_HORIZON, Len - 2);
        pTiffDsm->Buf[Len+2] = TIFF_UP_RIGHT;
    }

    if (CharLineCnt == 4)
    {
        pTiffDsm->Buf[7] = TIFF_LEFT_UP;
        memset(&(pTiffDsm->Buf[8]), TIFF_HORIZON, Len - 10);
        pTiffDsm->Buf[Len-2] = TIFF_UP_RIGHT;
    }
    else if (CharLineCnt == ToStart / 2)
    {
        memcpy(&(pTiffDsm->Buf[7 + (Len-2-7+1-2 - sizeof(gCoverPageString))/2]), gCoverPageString, sizeof(gCoverPageString));
    }
    else if (CharLineCnt == CharLineNum - 1)
    {
        pTiffDsm->Buf[3] = TIFF_LEFT_DOWN;
        memset(&(pTiffDsm->Buf[4]), TIFF_HORIZON, (Len - 3) / 2);
        pTiffDsm->Buf[(Len - 3)/2 + 4] = TIFF_MIDDLE_DOWN;
        memset(&(pTiffDsm->Buf[(Len - 3)/2 + 5]), TIFF_HORIZON, (Len - 3) / 2);
        pTiffDsm->Buf[Len+2] = TIFF_DOWN_RIGHT;
    }
    else if (CharLineCnt == ToStart - 4)
    {
        pTiffDsm->Buf[7] = TIFF_LEFT_DOWN;
        memset(&(pTiffDsm->Buf[8]), TIFF_HORIZON, Len - 10);
        pTiffDsm->Buf[Len-2] = TIFF_DOWN_RIGHT;
    }
    else if (CharLineCnt == ToStart - 1)
    {
        memcpy(&(pTiffDsm->Buf[StartIndex1]), pCoverPage->DateTimeStr, strlen(pCoverPage->DateTimeStr));
    }
    else if (CharLineCnt == ToStart)
    {
        pTiffDsm->Buf[3] = TIFF_LEFT_MIDDLE;
        memset(&(pTiffDsm->Buf[4]), TIFF_HORIZON, (Len - 3) / 2);
        pTiffDsm->Buf[(Len - 3)/2 + 4] = TIFF_UP_MIDDLE;
        memset(&(pTiffDsm->Buf[(Len - 3)/2 + 5]), TIFF_HORIZON, (Len - 3) / 2);
        pTiffDsm->Buf[Len+2] = TIFF_MIDDLE_RIGHT;
    }
    else if (CharLineCnt ==  ToStart + 1)
    {
        memcpy(&(pTiffDsm->Buf[StartIndex1]), gFrom, sizeof(gFrom));
        memcpy(&(pTiffDsm->Buf[StartIndex2]), gTo, sizeof(gTo));
    }
    else if (CharLineCnt ==  ToStart + 3)
    {
        memcpy(&(pTiffDsm->Buf[StartIndex1]), gName, sizeof(gName));
        memcpy(&(pTiffDsm->Buf[StartIndex1 + sizeof(gName)]), pCoverPage->CallerSide.Name, strlen(pCoverPage->CallerSide.Name));

        memcpy(&(pTiffDsm->Buf[StartIndex2]), gName, sizeof(gName));
        memcpy(&(pTiffDsm->Buf[StartIndex2 + sizeof(gName)]), pCoverPage->CalleeSide.Name, strlen(pCoverPage->CalleeSide.Name));
    }
    else if (CharLineCnt ==  ToStart + 5)
    {
        memcpy(&(pTiffDsm->Buf[StartIndex1]), gFaxNum, sizeof(gFaxNum));
        memcpy(&(pTiffDsm->Buf[StartIndex1 + sizeof(gFaxNum)]), pCoverPage->CallerSide.FaxNumber, strlen(pCoverPage->CallerSide.FaxNumber));

        memcpy(&(pTiffDsm->Buf[StartIndex2]), gFaxNum, sizeof(gFaxNum));
        memcpy(&(pTiffDsm->Buf[StartIndex2 + sizeof(gFaxNum)]), pCoverPage->CalleeSide.FaxNumber, strlen(pCoverPage->CalleeSide.FaxNumber));
    }
    else if (CharLineCnt ==  ToStart + 7)
    {
        memcpy(&(pTiffDsm->Buf[StartIndex1]), gPhone, sizeof(gPhone));
        memcpy(&(pTiffDsm->Buf[StartIndex1 + sizeof(gPhone)]), pCoverPage->CallerSide.PhoneNumber, strlen(pCoverPage->CallerSide.PhoneNumber));

        memcpy(&(pTiffDsm->Buf[StartIndex2]), gPhone, sizeof(gPhone));
        memcpy(&(pTiffDsm->Buf[StartIndex2 + sizeof(gPhone)]), pCoverPage->CalleeSide.PhoneNumber, strlen(pCoverPage->CalleeSide.PhoneNumber));
    }
    else if (CharLineCnt ==  ToStart + 9)
    {
        memcpy(&(pTiffDsm->Buf[StartIndex1]), gAddress, sizeof(gAddress));
        memcpy(&(pTiffDsm->Buf[StartIndex1 + sizeof(gAddress)]), pCoverPage->CallerSide.Street, strlen(pCoverPage->CallerSide.Street));

        memcpy(&(pTiffDsm->Buf[StartIndex2]), gAddress, sizeof(gAddress));
        memcpy(&(pTiffDsm->Buf[StartIndex2 + sizeof(gAddress)]), pCoverPage->CalleeSide.Street, strlen(pCoverPage->CalleeSide.Street));
    }
    else if (CharLineCnt ==  ToStart + 10)
    {
        memcpy(&(pTiffDsm->Buf[StartIndex1 + sizeof(gAddress)]), pCoverPage->CallerSide.CityState, strlen(pCoverPage->CallerSide.CityState));

        memcpy(&(pTiffDsm->Buf[StartIndex2 + sizeof(gAddress)]), pCoverPage->CalleeSide.CityState, strlen(pCoverPage->CalleeSide.CityState));
    }
    else if (CharLineCnt ==  ToStart + 11)
    {
        memcpy(&(pTiffDsm->Buf[StartIndex1 + sizeof(gAddress)]), pCoverPage->CallerSide.CountryZipCode, strlen(pCoverPage->CallerSide.CountryZipCode));

        memcpy(&(pTiffDsm->Buf[StartIndex2 + sizeof(gAddress)]), pCoverPage->CalleeSide.CountryZipCode, strlen(pCoverPage->CalleeSide.CountryZipCode));
    }

    if ((CharLineCnt > ToStart) && (CharLineCnt < CharLineNum - 1))
    {
        pTiffDsm->Buf[(Len - 3)/2 + 4] = TIFF_VERTICAL;
    }

    if ((CharLineCnt ==  ToStart + 2) || (CharLineCnt ==  ToStart + 4) || (CharLineCnt ==  ToStart + 6) || (CharLineCnt ==  ToStart + 8) || (CharLineCnt ==  ToStart + 12))
    {
        pTiffDsm->Buf[3] = TIFF_LEFT_MIDDLE;
        memset(&(pTiffDsm->Buf[4]), TIFF_HORIZON, (Len - 3) / 2);
        pTiffDsm->Buf[(Len - 3)/2 + 4] = TIFF_CROSS;
        memset(&(pTiffDsm->Buf[(Len - 3)/2 + 5]), TIFF_HORIZON, (Len - 3) / 2);
        pTiffDsm->Buf[Len+2] = TIFF_MIDDLE_RIGHT;
    }

    //pPageInfo->isLastPage = 0;

    pTiffCtrl->length += LINE_HEIGHT;

    pTiffDsm->Direction = MSG_DOWN_LINK;
    pTiffDsm->MsgType = MSG_TYPE_IMAGE_DATA;
    pTiffDsm->MsgCode = IMG_DATA_AND_STATUS;

    if (((UDWORD)(pPageInfo->imageLength - LINE_HEIGHT)) > pTiffCtrl->length)
    {
        pTiffDsm->Buf[0] = TIFF_PMC_COVER_PAGE;
    }
    else
    {
        pTiffDsm->Buf[0] = TIFF_PMC_COVER_PAGE_END;
    }

    pTiffDsm->Buf[1] = (UBYTE)((Len >> 8) & 0xff);
    pTiffDsm->Buf[2] = (UBYTE)(Len & 0xff);
}

void TiffSendTxCoverPageOnlyOneFaxCall(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    if (CoverPageDataCount > 0)
    {
        int len;

        pTiffDsm->Direction = MSG_DOWN_LINK;
        pTiffDsm->MsgType = MSG_TYPE_IMAGE_DATA;
        pTiffDsm->MsgCode = IMG_DATA_AND_STATUS;

        if (CoverPageDataTxCnt + MAX_LEN_API_MSG < CoverPageDataCount)
        {
            pTiffDsm->Buf[0] = TIFF_PMC_COVER_PAGE;
        }
        else
        {
            pTiffDsm->Buf[0] = TIFF_PMC_COVER_PAGE_END;
        }

        len = (pTiffDsm->Buf[0] == TIFF_PMC_COVER_PAGE) ? MAX_LEN_API_MSG : CoverPageDataCount - CoverPageDataTxCnt;

        pTiffDsm->Buf[1] = (UBYTE)((len >> 8) & 0xff);
        pTiffDsm->Buf[2] = (UBYTE)(len & 0xff);
        pTiffDsm->Buf[3] = CoverPageImageData[0];
        memcpy(&(pTiffDsm->Buf[4]), &(CoverPageImageData[1+CoverPageDataTxCnt]), len);
        CoverPageDataTxCnt += len;

        if (pTiffDsm->Buf[0] == TIFF_PMC_COVER_PAGE_END)
        {
            CoverPageDataCount = 0;
        }
    }
    else
    {
        pTiff->CoverPage.RptNoCoverPageData = 1;
    }
}

void TiffSendTxCoverPageAllLaterFaxCall(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    if (CoverPageDataCount > 0)
    {
        int len;

        pTiffDsm->Direction = MSG_DOWN_LINK;
        pTiffDsm->MsgType = MSG_TYPE_IMAGE_DATA;
        pTiffDsm->MsgCode = IMG_DATA_AND_STATUS;

        if (CoverPageDataTxCnt + MAX_LEN_API_MSG < CoverPageDataCount)
        {
            pTiffDsm->Buf[0] = TIFF_PMC_COVER_PAGE;
        }
        else
        {
            pTiffDsm->Buf[0] = TIFF_PMC_COVER_PAGE_END;
        }

        len = (pTiffDsm->Buf[0] == TIFF_PMC_COVER_PAGE) ? MAX_LEN_API_MSG : CoverPageDataCount - CoverPageDataTxCnt;

        pTiffDsm->Buf[1] = (UBYTE)((len >> 8) & 0xff);
        pTiffDsm->Buf[2] = (UBYTE)(len & 0xff);
        pTiffDsm->Buf[3] = CoverPageImageData[0];
        memcpy(&(pTiffDsm->Buf[4]), &(CoverPageImageData[1+CoverPageDataTxCnt]), len);
        CoverPageDataTxCnt += len;
    }
    else
    {
        pTiff->CoverPage.RptNoCoverPageData = 1;
    }
}
#endif

#if SUPPORT_JPEG_CODEC
void PreProcessTiffBuffer(UBYTE *bin, UWORD *bout, SDWORD width)
{
    UBYTE *q = bin;
    UBYTE threshold;
    int i, j;
    int lastcolor = 0, white = 0, black = 1;

    //threshold =(Quantum) (QuantumRange/2);
    threshold = 0x7f;

    i = 0;
    j = 0;

    while (i < width * 3)
    {
        if ((i % (width * 3)) == 0)
        {
            lastcolor = white;
            j = i / 3;
        }

        if (lastcolor == white)
        {
            if (q[i] < threshold) //black
            {
                j++;
                lastcolor = black;
            }

            bout[j] += 1;
        }
        else
        {
            if (q[i] >= threshold) //white
            {
                j++;
                lastcolor = white;
            }

            bout[j] += 1;
        }

        i += 3;
    }
}


#if SUPPORT_FILE_OPERATION

void SendconvertedImgDataToDsm(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    if (ValidLengthTiffData == 0)
    {
        SDWORD funcResult1;
        UWORD *pTemp;
        JDIMENSION num_scanlines;
        JDIMENSION col;
        UBYTE *rgboBuf = BufferRGBOFromJpeg;
        bmp_dest_ptr dest;
        UBYTE *inptr;

        StartIndexTiffData = 0;
        ValidLengthTiffData = 0;

        if (dinfo.output_scanline < dinfo.output_height)
        {
            num_scanlines = jpeg_read_scanlines(&dinfo, gdest_mgr->buffer, gdest_mgr->buffer_height);

            dest = (bmp_dest_ptr) gdest_mgr;
            inptr = dest->pub.buffer[0];

            for (col = 0; col < dinfo.output_width; col++)
            {
                rgboBuf[2] = *inptr++;
                rgboBuf[1] = *inptr++;
                rgboBuf[0] = *inptr++;
                rgboBuf += 3;
            }

            ConvertRGB2Y(&dinfo, BufferRGBOFromJpeg);
            memset(WbLinebuffer, 0, sizeof(WbLinebuffer));
            PreProcessTiffBuffer(BufferRGBOFromJpeg, WbLinebuffer, dinfo.output_width);

            memcpy(imgdata.pEncoderLine, &(WbLinebuffer[0]), dinfo.output_width * sizeof(UWORD));

            funcResult1 = t4Encoder_apply(&(imgdata.t4Encoder), imgdata.pEncoderLine, imgdata.pEncoderReferenceLine, &(pTiff->TiffData[ValidLengthTiffData]));

            if (funcResult1 < 0)
            {
                TRACE0("TIF: img encode ERROR.");
            }
            else if (funcResult1 > 0)
            {
                ValidLengthTiffData += funcResult1;
            }

            if (ValidLengthTiffData > 0x100000)
            {
                TRACE0("TIF: internal buffer overflow!");
            }

            pTemp = imgdata.pEncoderReferenceLine;
            imgdata.pEncoderReferenceLine = imgdata.pEncoderLine;
            imgdata.pEncoderLine = pTemp;
        }
    }
    else
    {
        UDWORD len;
        len = MIN32(ValidLengthTiffData, MAX_LEN_API_MSG);
        ValidLengthTiffData -= len;
        pTiffDsm->Direction = MSG_DOWN_LINK;
        pTiffDsm->MsgType = MSG_TYPE_IMAGE_DATA;
        pTiffDsm->MsgCode = IMG_DATA_AND_STATUS;

        if ((dinfo.output_scanline < dinfo.output_height) || (ValidLengthTiffData != 0))
        {
            pTiff->pageInfo.isLastPage = -1;
            pTiffDsm->Buf[0] = TIFF_PMC_MIDDLE_OF_PAGE;
        }
        else if (sgTiffFileOffset == 0)
        {
            pTiffDsm->Buf[0] = TIFF_PMC_END_OF_FILE_EOP;
            fclose(fJpegFile);
            remove("image.jpg");
            pTiff->pageInfo.isLastPage = 1;
        }
        else if (sgTiffFileOffset > 0)
        {
            pTiffDsm->Buf[0] = TIFF_PMC_NEXT_PAGE;
            fclose(fJpegFile);
            remove("image.jpg");
            pTiff->pageInfo.isLastPage = 0;

            if (TxTiffFileJpegConversionT4T6_Init(pTiff, OutCompressJpegCon) < 0)
            {
                TRACE0("TIF: Conversion init failed from jpeg to T4/T6.");
            }
        }

        pTiffDsm->Buf[1] = (UBYTE)((len >> 8) & 0xff);
        pTiffDsm->Buf[2] = (UBYTE)(len & 0xff);
        memcpy(&(pTiffDsm->Buf[3]), &(pTiff->TiffData[StartIndexTiffData]), len);
        StartIndexTiffData += len;
    }
}
#endif
#endif

void TiffDownLinkMsgSentByTiff(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    if (pTiffDsm->Direction == MSG_DIR_IDLE)
    {
        if (pTiff->TiffFileCmd == TIFF_FILE_READ_IFD)
        {
            TiffSendTxImgFormat(pTiffDsm, pTiff);
        }

#if (SUPPORT_TX_PURE_TEXT_FILE)
        else if (pTiff->TiffFileCmd == TIFF_FILE_TX_TEXT_FILE)
        {
            TiffSendTxTextFormat(pTiffDsm);
        }

#endif
        else if (pTiff->TiffFileCmd == TIFF_FILE_READ_DATA)
        {
            switch (pTiff->TxFileType)
            {
#if (SUPPORT_TX_PURE_TEXT_FILE)
                case TIFF_TX_TEXT_FILE:
                    SendTextDataToDsm(pTiffDsm, pTiff);
                    break;
#endif
                case TIFF_TX_TIFF_FILE:
                    SendImgDataToDsm(pTiffDsm, pTiff);
                    break;
#if SUPPORT_JPEG_CODEC
#if SUPPORT_FILE_OPERATION
                case TIFF_TX_JPEG_CONVERSION_FILE:
                    SendconvertedImgDataToDsm(pTiffDsm, pTiff);
                    break;
#endif
#endif
            }
        }

#if (SUPPORT_TX_COVER_PAGE)
        else if (pTiff->TiffFileCmd == TIFF_FILE_TX_COVER_PAGE)
        {
            if (pTiff->CoverPage.Pattern == TIFF_CPP_GAO_DEFAULT_1)
            {
                TiffSendTxCoverPageGAODefault1(pTiffDsm, pTiff);
            }
            else if (pTiff->CoverPage.Pattern == TIFF_CPP_CUSTOMER_ONLY_NEXT_CALL)
            {
                TiffSendTxCoverPageOnlyOneFaxCall(pTiffDsm, pTiff);
            }
            else if (pTiff->CoverPage.Pattern == TIFF_CPP_CUSTOMER_ALL_CALLS)
            {
                TiffSendTxCoverPageAllLaterFaxCall(pTiffDsm, pTiff);
            }
        }

#endif
        pTiff->TiffFileCmd = 0;
    }
}


void TiffGenrateTiffFile(GAO_Msg *pTiffDsm, TTiffStruct *pTiff)
{
    UWORD Length = 0;

    if (pTiff->TiffFileCmd == TIFF_FILE_WRITE_HEAD)
    {
        TIFF_init(&(pTiff->TiffCtrl), 0, 0);
#if SUPPORT_FILE_OPERATION
        TIFF_WriteHeader(&(pTiff->TiffCtrl), pTiff->TiffData);
#else
        TIFF_WriteHeader(&(pTiff->TiffCtrl), pTiff->pTiffFileBuf + pTiff->Offset);
#endif
#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)

        if (((dumpDataCnt + pTiff->TiffCtrl.length) < MAX_DUMP_DATA_MEMORY) && (DiskOverflow == 0))
        {
#endif

            if (pTiff->FilePointer == 1)
            {
#if SUPPORT_FILE_OPERATION
                fwrite(pTiff->TiffData, 1, pTiff->TiffCtrl.length, pTiff->fTiffFileRx);
#endif
            }
            else
            {
#if SUPPORT_FILE_OPERATION
                memcpy(pTiff->pTiffFileBuf + pTiff->Offset, pTiff->TiffData, pTiff->TiffCtrl.length);
#endif
                pTiff->Offset += pTiff->TiffCtrl.length;
            }

#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)
            dumpDataCnt = dumpDataCnt + pTiff->TiffCtrl.length;
        }
        else
        {
            DiskOverflow = 1;
        }

#endif
        pTiff->TiffFileCmd = 0;
    }
    else if (pTiff->TiffFileCmd == TIFF_FILE_WRITE_DATA)
    {
        if ((pTiffDsm->Direction == MSG_UP_LINK) && (pTiffDsm->MsgType == MSG_TYPE_IMAGE_DATA))
        {
            if (pTiffDsm->MsgCode == IMG_DATA_AND_STATUS)
            {
                Length = ((pTiffDsm->Buf[1] << 8) & 0xff00) + (pTiffDsm->Buf[2] & 0xff);
#if SUPPORT_FILE_OPERATION

                if (pTiff->pageInfo.compression == JPEG_COLOR)
                {
                    memcpy(&(pTiff->TiffData[WRITE_IFD_LENGTH_JPEG + pTiff->pageByteCnt + pTiff->padbytes]), &(pTiffDsm->Buf[3]), Length);
                }
                else if (pTiff->pageInfo.compression == JPEG_GRAY)
                {
                    memcpy(&(pTiff->TiffData[WRITE_IFD_LENGTH_JPEG1 + pTiff->pageByteCnt + pTiff->padbytes]), &(pTiffDsm->Buf[3]), Length);
                }
                else
                {
                    memcpy(&(pTiff->TiffData[WRITE_IFD_LENGTH + pTiff->pageByteCnt + pTiff->padbytes]), &(pTiffDsm->Buf[3]), Length);
                }

#else

                if (pTiff->pageInfo.compression == JPEG_COLOR)
                {
                    memcpy(&(pTiff->pTiffFileBuf[pTiff->Offset + WRITE_IFD_LENGTH_JPEG + pTiff->pageByteCnt + pTiff->padbytes]), &(pTiffDsm->Buf[3]), Length);
                }
                else if (pTiff->pageInfo.compression == JPEG_GRAY)
                {
                    memcpy(&(pTiff->pTiffFileBuf[pTiff->Offset + WRITE_IFD_LENGTH_JPEG1 + pTiff->pageByteCnt + pTiff->padbytes]), &(pTiffDsm->Buf[3]), Length);
                }
                else
                {
                    memcpy(&(pTiff->pTiffFileBuf[pTiff->Offset + WRITE_IFD_LENGTH + pTiff->pageByteCnt + pTiff->padbytes]), &(pTiffDsm->Buf[3]), Length);
                }

#endif
                pTiff->pageByteCnt += Length;
                memset(pTiffDsm, 0, sizeof(GAO_Msg));
                pTiff->TiffFileCmd = 0;
            }
        }
    }
    else if (pTiff->TiffFileCmd == TIFF_FILE_WRITE_IFD)
    {
        pTiff->pageInfo.fillOrder = FILLORDER_LSB_FIRST;
        pTiff->pageInfo.pageByteCount = pTiff->pageByteCnt;

        if (pTiff->pageInfo.compression == JPEG_COLOR)
        {
#if SUPPORT_FILE_OPERATION
            pTiff->pageInfo.imageLength = CountJpegLength(pTiff->TiffData + WRITE_IFD_LENGTH_JPEG, pTiff->pageByteCnt);
            pTiff->pageByteCnt -= ZeroNumofEndofJpeg(pTiff->TiffData + WRITE_IFD_LENGTH_JPEG - 1 + pTiff->pageByteCnt);
#else
            pTiff->pageInfo.imageLength = CountJpegLength(&(pTiff->pTiffFileBuf[pTiff->Offset + WRITE_IFD_LENGTH_JPEG]), pTiff->pageByteCnt);
            pTiff->pageByteCnt -= ZeroNumofEndofJpeg(&(pTiff->pTiffFileBuf[pTiff->Offset + WRITE_IFD_LENGTH_JPEG - 1 + pTiff->pageByteCnt]));
#endif
            pTiff->pageInfo.pageByteCount = pTiff->pageByteCnt;
#if SUPPORT_FILE_OPERATION
            TIFF_WriteIfd_Jpeg(&(pTiff->TiffCtrl), &(pTiff->pageInfo), pTiff->TiffData);
#else
            TIFF_WriteIfd_Jpeg(&(pTiff->TiffCtrl), &(pTiff->pageInfo), &(pTiff->pTiffFileBuf[pTiff->Offset]));
#endif

#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)

            if (((dumpDataCnt + pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG + pTiff->padbytes) < MAX_DUMP_DATA_MEMORY) && (DiskOverflow == 0))
            {
#endif

                if (pTiff->FilePointer == 1)
                {
#if SUPPORT_FILE_OPERATION
                    fwrite(pTiff->TiffData, 1, pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG + pTiff->padbytes, pTiff->fTiffFileRx);
#endif
                }
                else
                {
#if SUPPORT_FILE_OPERATION
                    memcpy(pTiff->pTiffFileBuf + pTiff->Offset, pTiff->TiffData, pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG + pTiff->padbytes);
#endif
                    pTiff->Offset += pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG + pTiff->padbytes;
                }

#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)
                dumpDataCnt = dumpDataCnt + pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG + pTiff->padbytes;
            }
            else
            {
                DiskOverflow = 1;
            }

#endif
        }
        else if (pTiff->pageInfo.compression == JPEG_GRAY)
        {
#if SUPPORT_FILE_OPERATION
            pTiff->pageInfo.imageLength = CountJpegLength(pTiff->TiffData + WRITE_IFD_LENGTH_JPEG1, pTiff->pageByteCnt);
            pTiff->pageByteCnt -= ZeroNumofEndofJpeg(pTiff->TiffData + WRITE_IFD_LENGTH_JPEG1 - 1 + pTiff->pageByteCnt);
#else
            pTiff->pageInfo.imageLength = CountJpegLength(&(pTiff->pTiffFileBuf[pTiff->Offset + WRITE_IFD_LENGTH_JPEG1]), pTiff->pageByteCnt);
            pTiff->pageByteCnt -= ZeroNumofEndofJpeg(&(pTiff->pTiffFileBuf[pTiff->Offset + WRITE_IFD_LENGTH_JPEG1 - 1 + pTiff->pageByteCnt]));
#endif
            pTiff->pageInfo.pageByteCount = pTiff->pageByteCnt;
#if SUPPORT_FILE_OPERATION
            TIFF_WriteIfd_Jpeg(&(pTiff->TiffCtrl), &(pTiff->pageInfo), pTiff->TiffData);
#else
            TIFF_WriteIfd_Jpeg(&(pTiff->TiffCtrl), &(pTiff->pageInfo), &(pTiff->pTiffFileBuf[pTiff->Offset]));
#endif

#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)

            if (((dumpDataCnt + pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG1 + pTiff->padbytes) < MAX_DUMP_DATA_MEMORY) && (DiskOverflow == 0))
            {
#endif

                if (pTiff->FilePointer == 1)
                {
#if SUPPORT_FILE_OPERATION
                    fwrite(pTiff->TiffData, 1, pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG1 + pTiff->padbytes, pTiff->fTiffFileRx);
#endif
                }
                else
                {
#if SUPPORT_FILE_OPERATION
                    memcpy(pTiff->pTiffFileBuf + pTiff->Offset, pTiff->TiffData, pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG1 + pTiff->padbytes);
#endif
                    pTiff->Offset += pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG1 + pTiff->padbytes;
                }

#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)
                dumpDataCnt = dumpDataCnt + pTiff->pageByteCnt + WRITE_IFD_LENGTH_JPEG1 + pTiff->padbytes;
            }
            else
            {
                DiskOverflow = 1;
            }

#endif
        }
        else
        {
#if SUPPORT_FILE_OPERATION
            TIFF_WriteIfd(&(pTiff->TiffCtrl), &(pTiff->pageInfo), pTiff->TiffData);
#else
            TIFF_WriteIfd(&(pTiff->TiffCtrl), &(pTiff->pageInfo), &(pTiff->pTiffFileBuf[pTiff->Offset]));
#endif
#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)

            if (((dumpDataCnt + pTiff->pageByteCnt + WRITE_IFD_LENGTH + pTiff->padbytes) < MAX_DUMP_DATA_MEMORY) && (DiskOverflow == 0))
            {
#endif

                if (pTiff->FilePointer == 1)
                {
#if SUPPORT_FILE_OPERATION
                    fwrite(pTiff->TiffData, 1, pTiff->pageByteCnt + WRITE_IFD_LENGTH + pTiff->padbytes, pTiff->fTiffFileRx);
#endif
                }
                else
                {
#if SUPPORT_FILE_OPERATION
                    memcpy(pTiff->pTiffFileBuf + pTiff->Offset, pTiff->TiffData, pTiff->pageByteCnt + WRITE_IFD_LENGTH + pTiff->padbytes);
#endif
                    pTiff->Offset += pTiff->pageByteCnt + WRITE_IFD_LENGTH + pTiff->padbytes;
                }

#if SUPPORT_DISK_OVER_FLOW_CHECK && (!SUPPORT_FXS)
                dumpDataCnt = dumpDataCnt + pTiff->pageByteCnt + WRITE_IFD_LENGTH + pTiff->padbytes;
            }
            else
            {
                DiskOverflow = 1;
            }

            if ((pTiff->EcmOn == 0) && (pTiff->PostMsgResp == 2)) // 2=RTN
            {
                if (pTiff->FilePointer == 1)
                {
#if SUPPORT_FILE_OPERATION
                    int offsettemp;
                    offsettemp = -(pTiff->pageByteCnt + WRITE_IFD_LENGTH + pTiff->padbytes);
                    fseek(pTiff->fTiffFileRx, offsettemp, SEEK_CUR);
#endif
                }
                else
                {
                    pTiff->Offset -= pTiff->pageByteCnt + WRITE_IFD_LENGTH + pTiff->padbytes;
                }
            }

#endif
        }

        if (!((pTiff->EcmOn == 0) && (pTiff->PostMsgResp == 2)))
        {
            pTiff->padbytes = pTiff->TiffCtrl.paddingLength;
        }

        pTiff->pageByteCnt = 0;
        pTiff->PostMsgCmd = 0;
        pTiff->PostMsgResp = 0;
        pTiff->RxLineNumber = 0;
        pTiff->TiffFileCmd = 0;

        if ((pTiff->pageInfo.isLastPage == 1) && (pTiff->FilePointer == 0))
        {
            pTiff->pTiffFileBuf -= 4;
            pTiff->pTiffFileBuf[0] = (UBYTE)((pTiff->Offset >> 24) & 0xff);
            pTiff->pTiffFileBuf[1] = (UBYTE)((pTiff->Offset >> 16) & 0xff);
            pTiff->pTiffFileBuf[2] = (UBYTE)((pTiff->Offset >> 8) & 0xff);
            pTiff->pTiffFileBuf[3] = (UBYTE)((pTiff->Offset >> 0) & 0xff);
        }
    }
}

void TiffToAppMsgFilter(GAO_Msg *pAppTiff, TTiffStruct *pTiff)
{
    if (pAppTiff->Direction == MSG_UP_LINK)
    {
        if (pAppTiff->MsgType == MSG_TYPE_REPORT)
        {
            switch (pAppTiff->MsgCode)
            {
                case RPT_RESULT_CODE:

                    if (!((pAppTiff->Buf[0] == DSM_RC_OK) || (pAppTiff->Buf[0] == DSM_RC_RING) || (pAppTiff->Buf[0] == DSM_RC_CONNECT) || (pAppTiff->Buf[0] == DSM_RC_NODIAL_TONE) || (pAppTiff->Buf[0] == DSM_RC_BUSY) || (pAppTiff->Buf[0] == DSM_RC_NO_ANSWER)))
                    {
                        memset(pAppTiff, 0, sizeof(GAO_Msg));
                    }

                    if (pAppTiff->Buf[0] == DSM_RC_CONNECT)
                    {
                        if (pTiff->pageInfo.isLastPage == 0)
                        {
                            pTiff->RptPMCTx = 1;
                        }
                        else if (pTiff->pageInfo.isLastPage == 1)
                        {
                            pTiff->RptPMCTx = 3;
                        }
                    }

                    break;
                case RPT_RECV_POST_PAGE_CMD:

                    if (!((pAppTiff->Buf[0] == DSM_PMC_MPS) || (pAppTiff->Buf[0] == DSM_PMC_EOP)))
                    {
                        memset(pAppTiff, 0, sizeof(GAO_Msg));
                    }

                    break;
                case RPT_HANGUP_STATUS:
                    pAppTiff->Buf[0] = 0;
                    break;
                case RPT_TIFF_FILE_RUN_MODE:

                    if (!((pAppTiff->Buf[0] == DSM_TX_TIFF_FILE) || (pAppTiff->Buf[0] == DSM_RX_TIFF_FILE)))
                    {
                        memset(pAppTiff, 0, sizeof(GAO_Msg));
                    }

                    break;
                case RPT_VALIDITY_OF_TX_TIFF_FILE:
                    break;
                default:

                    if (pAppTiff->MsgCode < TEST_FCLASS ||
                        pAppTiff->MsgCode > TEST_FBS)
                    {
                        memset(pAppTiff, 0, sizeof(GAO_Msg));
                    }

                    break;
            }
        }
    }
}

void TiffMsgProMain(TTiffApiStruct *pTiffApi, TTiffStruct *pTiff)
{
    GAO_Msg *pAppTiff = pTiffApi->pAppTiff;
    GAO_Msg *pTiffDsm = pTiffApi->pTiffDsm;


    if ((pAppTiff == NULL) || (pTiffDsm == NULL))
    {
        TRACE0("TIFF: msg pointer of AppTiff or TiffDsm ERROR!!");
        return;
    }

    if (pTiff->Direction == TIFF_FILE_TRANSMIT)
    {
        TiffDownLinkMsgSentByTiff(pTiffDsm, pTiff);
    }
    else if (pTiff->Direction == TIFF_FILE_RECEIVE)
    {
        TiffGenrateTiffFile(pTiffDsm, pTiff);
    }

    if ((pAppTiff->Direction == MSG_DOWN_LINK) && (pTiffDsm->Direction == MSG_DIR_IDLE))
    {
        TiffMsgProDownlink(pAppTiff, pTiff);
        memcpy(pTiffDsm, pAppTiff, sizeof(GAO_Msg));
        memset(pAppTiff, 0, sizeof(GAO_Msg));
    }
    else if (pTiff->TxFileCheckResults[0] != 0)
    {
        ReportTxTiffFileCheckResults(pAppTiff, pTiff);
    }
    else if ((pTiffDsm->Direction == MSG_UP_LINK) &&
             (pAppTiff->Direction == MSG_DIR_IDLE) &&
             (pTiffDsm->MsgCode != IMG_DATA_AND_STATUS))
    {
        TiffMsgProUplink(pTiffDsm, pTiff);
        memcpy(pAppTiff, pTiffDsm, sizeof(GAO_Msg));
        memset(pTiffDsm, 0, sizeof(GAO_Msg));
        TiffToAppMsgFilter(pAppTiff, pTiff);
    }
    else if (pTiff->RptPMCTx != 0)
    {
        if (pAppTiff->Direction == MSG_DIR_IDLE)
        {
            pAppTiff->Direction = MSG_UP_LINK;
            pAppTiff->MsgType = MSG_TYPE_REPORT;
            pAppTiff->MsgCode = RPT_RECV_POST_PAGE_CMD;
            pAppTiff->Buf[0] = pTiff->RptPMCTx;
            pTiff->RptPMCTx = 0;
        }
    }

#if (SUPPORT_TX_COVER_PAGE)
    else if (pTiff->CoverPage.RptNoCoverPageData != 0)
    {
        if (pAppTiff->Direction == MSG_DIR_IDLE)
        {
            pAppTiff->Direction = MSG_UP_LINK;
            pAppTiff->MsgType = MSG_TYPE_REPORT;
            pAppTiff->MsgCode = RPT_GAOFAXLIB_NEED_COVERPAGE_DATA;
            pTiff->CoverPage.RptNoCoverPageData = 0;
        }
    }

    if ((pAppTiff->Direction == MSG_DOWN_LINK) && (pAppTiff->MsgCode == IMG_DATA_AND_STATUS) && ((pAppTiff->Buf[0] == TIFF_PMC_COVER_PAGE) || (pAppTiff->Buf[0] == TIFF_PMC_COVER_PAGE_END)))
    {
        int len;
        len = ((pAppTiff->Buf[1] << 8) & 0xff00) + pAppTiff->Buf[2];
        CoverPageImageData[0] = pAppTiff->Buf[3];

        if (CoverPageDataCount + 1 + len > 1024 * 1024)
        {
            TRACE0("TIFF: cover page image data defined by the customer is too much!!!");
        }
        else
        {
            memcpy(&(CoverPageImageData[CoverPageDataCount+1]), &(pAppTiff->Buf[4]), len);
            CoverPageDataCount += len;
        }
    }

#endif
}



void TiffStateControlTx(TTiffStruct *pTiff)
{
#if (SUPPORT_TX_PURE_TEXT_FILE || SUPPORT_TX_COVER_PAGE)
    T4InfoStruct *pPageInfo = &(pTiff->pageInfo);
#endif
#if (SUPPORT_TX_PURE_TEXT_FILE || SUPPORT_TX_COVER_PAGE || SUPPORT_JPEG_CODEC)
    TiffStruct  *pTiffCtrl = &(pTiff->TiffCtrl);
#endif
    SWORD Rev = 0;

    switch (pTiff->State)
    {
        case TIFF_STATE_CHECK_PARAS:

            if (pTiff->FilePointer == 1)
            {
#if SUPPORT_FILE_OPERATION

                if ((Rev = RxTiffHeader(pTiff, pTiff->fTiffFileTxCurr)) == 0)
                {
                    if ((Rev = RxTiffPageParas(pTiff, pTiff->fTiffFileTxCurr)) == 0)
                    {
                        pTiff->TiffFileCmd = TIFF_FILE_READ_IFD;
#if (SUPPORT_TX_COVER_PAGE)

                        if (pTiff->CoverPage.Pattern == TIFF_CPP_NO_COVER_PAGE)
                        {
                            pTiff->State = TIFF_STATE_WAIT_OK;
                        }
                        //else if((pTiff->pageInfo.compression != T4_1D_MH) && (pTiff->pageInfo.compression != T4_2D_MR) && (pTiff->pageInfo.compression != T6_2D_MMR))
                        //    pTiff->State = TIFF_STATE_WAIT_OK;
                        else
                        {
                            pTiff->State = TIFF_STATE_WAIT_OK_COVERP;
                        }

#else
                        pTiff->State = TIFF_STATE_WAIT_OK;
#endif
                    }
                }

                if (Rev != 0)
                {
#if (SUPPORT_TX_PURE_TEXT_FILE)
                    TRACE0("TIFF: Assume there is a text file to be sent.");
                    pTiff->TiffFileCmd = TIFF_FILE_TX_TEXT_FILE;
#else
                    TRACE0("TIFF: Wrong tiff file to be sent.");
                    pTiff->ErrorCode = Rev;
#endif
#if (SUPPORT_TX_COVER_PAGE)

                    if (pTiff->CoverPage.Pattern == TIFF_CPP_NO_COVER_PAGE)
                    {
                        pTiff->State = TIFF_STATE_WAIT_CONNECT;
                    }
                    else
                    {
                        pTiff->State = TIFF_STATE_WAIT_CONNECT_COVERP;
                    }

#else
                    pTiff->State = TIFF_STATE_WAIT_CONNECT;
#endif
                }

#endif
            }
            else
            {
                if ((Rev = RxTiffHeaderInBuffer(pTiff)) == 0)
                {
                    if ((Rev = RxTiffPageParasInBuffer(pTiff)) == 0)
                    {
                        pTiff->TiffFileCmd = TIFF_FILE_READ_IFD;
#if (SUPPORT_TX_COVER_PAGE)

                        if (pTiff->CoverPage.Pattern == TIFF_CPP_NO_COVER_PAGE)
                        {
                            pTiff->State = TIFF_STATE_WAIT_OK;
                        }
                        //else if((pTiff->pageInfo.compression != T4_1D_MH) && (pTiff->pageInfo.compression != T4_2D_MR) && (pTiff->pageInfo.compression != T6_2D_MMR))
                        //    pTiff->State = TIFF_STATE_WAIT_OK;
                        else
                        {
                            pTiff->State = TIFF_STATE_WAIT_OK_COVERP;
                        }

#else
                        pTiff->State = TIFF_STATE_WAIT_OK;
#endif
                    }
                }

                if (Rev != 0)
                {
#if (SUPPORT_TX_PURE_TEXT_FILE)
                    TRACE0("TIFF: Assume there is a text file to be sent.");
                    pTiff->TiffFileCmd = TIFF_FILE_TX_TEXT_FILE;
#else
                    TRACE0("TIFF: Wrong tiff file to be sent.");
                    pTiff->ErrorCode = Rev;
#endif
#if (SUPPORT_TX_COVER_PAGE)

                    if (pTiff->CoverPage.Pattern == TIFF_CPP_NO_COVER_PAGE)
                    {
                        pTiff->State = TIFF_STATE_WAIT_CONNECT;
                    }
                    else
                    {
                        pTiff->State = TIFF_STATE_WAIT_CONNECT_COVERP;
                    }

#else
                    pTiff->State = TIFF_STATE_WAIT_CONNECT;
#endif
                }
            }

            break;
#if (SUPPORT_TX_COVER_PAGE)
        case TIFF_STATE_WAIT_OK_COVERP:

            if (pTiff->ResultCode == DSM_RC_OK)
            {
                pTiff->State = TIFF_STATE_WAIT_CONNECT_COVERP;
                pTiff->ResultCode = DSM_RC_NULL;
            }

            break;
        case TIFF_STATE_WAIT_CONNECT_COVERP:

            if (pTiff->ResultCode == DSM_RC_CONNECT)
            {
                pTiff->State = TIFF_STATE_DURING_PAGE_COVERP;
                pTiff->ResultCode = DSM_RC_NULL;
                CoverPageDataTxCnt = 0;
            }

            break;
        case TIFF_STATE_DURING_PAGE_COVERP:

            if (pTiff->CoverPage.Pattern == TIFF_CPP_GAO_DEFAULT_1)
            {
                if ((UDWORD)(pPageInfo->imageLength - LINE_HEIGHT) <= pTiffCtrl->length)
                {
                    pTiff->State = TIFF_STATE_END_OF_PAGE_COVERP;
                }
                else
                {
                    pTiff->TiffFileCmd = TIFF_FILE_TX_COVER_PAGE;
                }
            }
            else if ((pTiff->CoverPage.Pattern == TIFF_CPP_CUSTOMER_ONLY_NEXT_CALL) || (pTiff->CoverPage.Pattern == TIFF_CPP_CUSTOMER_ALL_CALLS))
            {
                if (CoverPageDataCount == 0)
                {
                    pTiff->State = TIFF_STATE_DURING_PAGE;
                }
                else
                {
                    if (CoverPageDataTxCnt >= CoverPageDataCount)
                    {
                        pTiff->State = TIFF_STATE_END_OF_PAGE_COVERP;
                    }
                    else
                    {
                        pTiff->TiffFileCmd = TIFF_FILE_TX_COVER_PAGE;
                    }
                }
            }

            break;
        case TIFF_STATE_END_OF_PAGE_COVERP:

            if (pTiff->FilePointer == 1)
            {
#if SUPPORT_FILE_OPERATION

                if (pTiff->fTiffFileTx[0] != NULL)
                {
                    pTiff->State = TIFF_STATE_WAIT_OK;
                }
                else
                {
                    pTiff->State = TIFF_STATE_END_OF_FILE;
                }

#endif
            }
            else
            {
                if (pTiff->pTiffFileBuf != 0)
                {
                    pTiff->State = TIFF_STATE_WAIT_OK;
                }
                else
                {
                    pTiff->State = TIFF_STATE_END_OF_FILE;
                }
            }

            pTiffCtrl->length = 0;
            break;
#endif
        case TIFF_STATE_WAIT_OK:

            if (pTiff->ResultCode == DSM_RC_OK)
            {
                pTiff->State = TIFF_STATE_WAIT_CONNECT;
                pTiff->ResultCode = DSM_RC_NULL;
            }

            break;
        case TIFF_STATE_WAIT_CONNECT:

            if (pTiff->ResultCode == DSM_RC_CONNECT)
            {
                pTiff->State = TIFF_STATE_DURING_PAGE;
                pTiff->ResultCode = DSM_RC_NULL;
            }

            break;
        case TIFF_STATE_DURING_PAGE:

            if (pTiff->TxFileType == TIFF_TX_TIFF_FILE)
            {
                if (pTiff->TiffCtrl.length >= pTiff->TiffCtrl.lengthPage)
                {
                    pTiff->State = TIFF_STATE_END_OF_PAGE;
                }
                else
                {
                    pTiff->TiffFileCmd = TIFF_FILE_READ_DATA;
                }
            }

#if (SUPPORT_TX_PURE_TEXT_FILE)
            else if (pTiff->TxFileType == TIFF_TX_TEXT_FILE)
            {
                if ((UDWORD)pPageInfo->imageLength <= pTiffCtrl->length)
                {
                    pTiff->State = TIFF_STATE_END_OF_PAGE;
                }
                else
                {
                    pTiff->TiffFileCmd = TIFF_FILE_READ_DATA;
                }
            }

#endif
#if SUPPORT_JPEG_CODEC
            else if (pTiff->TxFileType == TIFF_TX_JPEG_CONVERSION_FILE)
            {
                if (pTiff->pageInfo.isLastPage < 0)
                {
                    pTiff->TiffFileCmd = TIFF_FILE_READ_DATA;
                }
                else
                {
                    pTiff->pageInfo.isLastPage = -1;
                    pTiff->State = TIFF_STATE_END_OF_PAGE;
                }
            }

#endif
            break;
        case TIFF_STATE_END_OF_PAGE:

            if ((pTiff->ResultCode == DSM_RC_OK) || (pTiff->ResultCode == DSM_RC_ERROR))
            {
                pTiff->ResultCode = DSM_RC_NULL;

                if (pTiff->pageInfo.isLastPage == 1)
                {
                    pTiff->State = TIFF_STATE_END_OF_FILE;
                    pTiff->RptPMCTx = 3;
                }
                else
                {
                    pTiff->RptPMCTx = 1;
                    pTiff->State = TIFF_STATE_WAIT_CONNECT;

                    if (pTiff->TxFileType == TIFF_TX_TIFF_FILE)
                    {
                        if (pTiff->FilePointer == 1)
                        {
#if SUPPORT_FILE_OPERATION

                            if (RxTiffPageParas(pTiff, pTiff->fTiffFileTxCurr) != 0)
                            {
                                TRACE0("TIFF: Read page parameters Error!");
                            }

#endif
                        }
                        else
                        {
                            if (RxTiffPageParasInBuffer(pTiff) != 0)
                            {
                                TRACE0("TIFF: Read page parameters Error!");
                            }
                        }
                    }

#if (SUPPORT_TX_PURE_TEXT_FILE)
                    else if (pTiff->TxFileType == TIFF_TX_TEXT_FILE)
                    {
                        pTiffCtrl->length = 0;
                    }

#endif
#if SUPPORT_JPEG_CODEC
                    else if (pTiff->TxFileType == TIFF_TX_JPEG_CONVERSION_FILE)
                    {
                        pTiffCtrl->length = 0;
                    }

#endif
                }
            }

            break;
        case TIFF_STATE_END_OF_FILE:
            TiffInitPart1(pTiff);
            break;
    }

}

void TiffStateControlRx(TTiffStruct *pTiff)
{
    switch (pTiff->State)
    {
        case TIFF_STATE_INIT:

            if (pTiff->FilePointer == 1)
            {
#if SUPPORT_FILE_OPERATION

                if (pTiff->fTiffFileRx != NULL)
                {
                    pTiff->State = TIFF_STATE_FILE_HEAD;
                }

#endif
            }
            else
            {
                if (pTiff->pTiffFileBuf != 0)
                {
                    pTiff->State = TIFF_STATE_FILE_HEAD;
                }
            }

            break;
        case TIFF_STATE_FILE_HEAD:
            pTiff->TiffFileCmd = TIFF_FILE_WRITE_HEAD;
            pTiff->State = TIFF_STATE_WAIT_CONNECT;
            break;
        case TIFF_STATE_WAIT_CONNECT:

            if (pTiff->ResultCode == DSM_RC_CONNECT)
            {
                pTiff->State = TIFF_STATE_DURING_PAGE;
                pTiff->ResultCode = DSM_RC_NULL;
            }
            else if (pTiff->ResultCode == DSM_RC_HANGUPING)
            {
                pTiff->State = TIFF_STATE_END_OF_FILE;
                pTiff->ResultCode = DSM_RC_NULL;
            }

            break;
        case TIFF_STATE_DURING_PAGE:

            if (pTiff->RxLineNumber > 0)
            {
                if (pTiff->PostMsgCmd == DSM_PMC_IDL)
                {
                    pTiff->State = TIFF_STATE_DURING_PAGE;
                }
                else if (pTiff->PostMsgCmd == DSM_PMC_MPS)
                {
                    pTiff->State = TIFF_STATE_END_OF_PAGE;
                }
                else
                {
                    pTiff->State = TIFF_STATE_LAST_PAGE;
                }
            }
            else
            {
                pTiff->TiffFileCmd = TIFF_FILE_WRITE_DATA;
            }

            break;
        case TIFF_STATE_END_OF_PAGE:
            pTiff->pageInfo.isLastPage = 0;
            pTiff->TiffFileCmd = TIFF_FILE_WRITE_IFD;
            pTiff->State = TIFF_STATE_WAIT_CONNECT;
            break;
        case TIFF_STATE_LAST_PAGE:
            pTiff->pageInfo.isLastPage = 1;
            pTiff->TiffFileCmd = TIFF_FILE_WRITE_IFD;

            if ((pTiff->EcmOn == 0) && (pTiff->PostMsgResp == 2)) // 2=RTN
            {
                pTiff->State = TIFF_STATE_WAIT_CONNECT;
            }
            else
            {
                pTiff->State = TIFF_STATE_END_OF_FILE;
            }

            break;
        case TIFF_STATE_END_OF_FILE:
            TiffInitPart1(pTiff);
            break;
    }
}


void TiffStateControl(TTiffStruct *pTiff)
{
    if (pTiff == NULL)
    {
        return;
    }

    if (pTiff->Direction == TIFF_FILE_TRANSMIT)
    {
        TiffStateControlTx(pTiff);
    }
    else if (pTiff->Direction == TIFF_FILE_RECEIVE)
    {
        TiffStateControlRx(pTiff);
    }
}


void TiffMain(TTiffApiStruct *pTiffApi, TTiffStruct *pTiff)
{
    if ((pTiffApi == NULL) || (pTiff == NULL))
    {
        TRACE0("TIFF: msg pointer of pTiffApi or pTiff ERROR!!");
        return;
    }

    TiffMsgProMain(pTiffApi, pTiff);
    TiffStateControl(pTiff);
}

#if SUPPORT_JPEG_CODEC
LOCAL(cjpeg_source_ptr)
select_file_type(j_compress_ptr cinfo, FILE *infile)
{
    int c;
    boolean is_targa = FALSE;

    if (is_targa)
    {
#ifdef TARGA_SUPPORTED
        return jinit_read_targa(cinfo);
#else
        ERREXIT(cinfo, JERR_TGA_NOTCOMP);
#endif
    }

    if ((c = getc(infile)) == EOF)
    {
        ERREXIT(cinfo, JERR_INPUT_EMPTY);
    }

    if (ungetc(c, infile) == EOF)
    {
        ERREXIT(cinfo, JERR_UNGETC_FAILED);
    }

    switch (c)
    {
#ifdef BMP_SUPPORTED
        case 'B':
            return jinit_read_bmp(cinfo);
#endif
#ifdef GIF_SUPPORTED
        case 'G':
            return jinit_read_gif(cinfo);
#endif
#ifdef PPM_SUPPORTED
        case 'P':
            return jinit_read_ppm(cinfo);
#endif
#ifdef RLE_SUPPORTED
        case 'R':
            return jinit_read_rle(cinfo);
#endif
#ifdef TARGA_SUPPORTED
        case 0x00:
            return jinit_read_targa(cinfo);
#endif
        default:
            ERREXIT(cinfo, JERR_UNKNOWN_FORMAT);
            break;
    }

    return NULL;          /* suppress compiler warnings */
}

void WriteIfdEntryToFile(FILE *fTif, UWORD tag, UWORD type, UDWORD cnt, UDWORD value)
{
    fwrite(&tag, sizeof(UBYTE), sizeof(UWORD), fTif);
    fwrite(&type, sizeof(UBYTE), sizeof(UWORD), fTif);
    fwrite(&cnt, sizeof(UBYTE), sizeof(UDWORD), fTif);
    fwrite(&value, sizeof(UBYTE), sizeof(UDWORD), fTif);
}

void FileConversionFromBmpToTiff(FILE *fBmp, FILE *fTif)
{
    struct jpeg_compress_struct gcinfo;
    struct jpeg_error_mgr gjerr;
    cjpeg_source_ptr src_mgr;
    JDIMENSION num_scanlines;

    FILE *fJpegFile;
    UBYTE buf[100];
    int length;

    gcinfo.err = jpeg_std_error(&gjerr);
    jpeg_create_compress(&gcinfo);
    gjerr.addon_message_table = cdjpeg_message_table;
    gjerr.first_addon_message = JMSG_FIRSTADDONCODE;
    gjerr.last_addon_message = JMSG_LASTADDONCODE;
    gcinfo.ApplicationType = APPICATION_SEND_FAX;
    gcinfo.SpatialResolution = 200;
    gcinfo.in_color_space = JCS_RGB; // BMP uses RGB
    jpeg_set_defaults(&gcinfo);

    if ((fJpegFile = fopen("image.jpg", "wb")) == 0)
    {
        printf("Can not create image.jpg!\n");
        return;
    }

    src_mgr = select_file_type(&gcinfo, fBmp);
    src_mgr->input_file = fBmp;
    (*src_mgr->start_input)(&gcinfo, src_mgr);
    jpeg_default_colorspace(&gcinfo);
    jpeg_stdio_dest(&gcinfo, fJpegFile);
    jpeg_start_compress(&gcinfo, TRUE);

    while (gcinfo.next_scanline < gcinfo.image_height)
    {
        num_scanlines = (*src_mgr->get_pixel_rows)(&gcinfo, src_mgr);
        (void) jpeg_write_scanlines(&gcinfo, src_mgr->buffer, num_scanlines);
    }

    (*src_mgr->finish_input)(&gcinfo, src_mgr);
    jpeg_finish_compress(&gcinfo);
    //jpeg_destroy_compress(&gcinfo);
    fclose(fJpegFile);

    if ((fJpegFile = fopen("image.jpg", "rb")) == 0)
    {
        printf("image.jpg does not exist!\n");
        return;
    }

    fseek(fJpegFile, 0, SEEK_END);
    length = ftell(fJpegFile);
    fseek(fJpegFile, 0, SEEK_SET);

    buf[0] = 0x49;
    buf[1] = 0x49;
    buf[2] = 0x2A;
    buf[3] = 0x00;
    buf[4] = 0x08;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    buf[8] = WRITE_IFD_ENTRYNUM_JPEG & 0xff;
    buf[9] = (WRITE_IFD_ENTRYNUM_JPEG >> 8) & 0xff;
    fwrite(buf, 1, 10, fTif);
    WriteIfdEntryToFile(fTif, TAG_NEWSUBFILETYPE, IFD_LONG, 1, NEWSUB_FILETYPE_VALUE);
    WriteIfdEntryToFile(fTif, TAG_IMAGEWIDTH, IFD_LONG, 1, gcinfo.image_width);
    WriteIfdEntryToFile(fTif, TAG_IMAGELENGTH, IFD_LONG, 1, gcinfo.image_height);
    WriteIfdEntryToFile(fTif, TAG_BITSPERSAMPLE, IFD_SHORT, 3, 0xd0);
    WriteIfdEntryToFile(fTif, TAG_COMPRESSION, IFD_SHORT, 1, JPEG_COMPRESSION);
    WriteIfdEntryToFile(fTif, TAG_PHOTOMETRIC, IFD_SHORT, 1, CS_CIELAB10);
    WriteIfdEntryToFile(fTif, TAG_FILLORDER, IFD_SHORT, 1, FILLORDER_LSB_FIRST);
    WriteIfdEntryToFile(fTif, TAG_STRIPOFFSETS, IFD_LONG, 1, WRITE_IFD_LENGTH_JPEG1 + 16);
    WriteIfdEntryToFile(fTif, TAG_ORIENTATION, IFD_SHORT, 1, JPEG_ORIENTATION_VALUE);
    WriteIfdEntryToFile(fTif, TAG_SAMPLESPERPIXEL, IFD_LONG, 1, gcinfo.input_components);
    WriteIfdEntryToFile(fTif, TAG_ROWSPERSTRIP, IFD_LONG, 1, gcinfo.image_height);
    WriteIfdEntryToFile(fTif, TAG_STRIPBYTECOUNTS, IFD_LONG, 1, length);
    WriteIfdEntryToFile(fTif, TAG_XRESOLUTION, IFD_RATIONAL, 1, 0xd8);
    WriteIfdEntryToFile(fTif, TAG_YRESOLUTION, IFD_RATIONAL, 1, 0xe0);
    WriteIfdEntryToFile(fTif, TAG_RESOLUTIONUNIT, IFD_SHORT, 1, TIFF_RESOLUTION_UNIT_INCHES);
    WriteIfdEntryToFile(fTif, TAG_PAGENUMBER, IFD_SHORT, 2, 0);
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x08;
    buf[7] = 0x00;
    buf[8] = 0x08;
    buf[9] = 0x00;
    buf[10] = 0x08;
    buf[11] = 0x00;
    buf[12] = 0x00;
    buf[13] = 0x00;
    buf[14] = 0xc8;
    buf[15] = 0x00;
    buf[16] = 0x00;
    buf[17] = 0x00;
    buf[18] = 0x01;
    buf[19] = 0x00;
    buf[20] = 0x00;
    buf[21] = 0x00;
    buf[22] = 0xc8;
    buf[23] = 0x00;
    buf[24] = 0x00;
    buf[25] = 0x00;
    buf[26] = 0x01;
    buf[27] = 0x00;
    buf[28] = 0x00;
    buf[29] = 0x00;
    fwrite(buf, 1, 30, fTif);
    jpeg_destroy_compress(&gcinfo);

    while ((length = fread(buf, 1, 100, fJpegFile)) == 100)
    {
        fwrite(buf, 1, 100, fTif);
        length = 0;
    }

    if (length > 0)
    {
        fwrite(buf, 1, length, fTif);
    }

    fclose(fJpegFile);
    remove("image.jpg");
}

LOCAL(unsigned int)
jpeg_getc(j_decompress_ptr cinfo)
/* Read next byte */
{
    struct jpeg_source_mgr *datasrc = cinfo->src;

    if (datasrc->bytes_in_buffer == 0)
    {
        if (!(*datasrc->fill_input_buffer)(cinfo))
        {
            ERREXIT(cinfo, JERR_CANT_SUSPEND);
        }
    }

    datasrc->bytes_in_buffer--;
    return GETJOCTET(*datasrc->next_input_byte++);
}

METHODDEF(boolean)
print_text_marker(j_decompress_ptr cinfo)
{
    boolean traceit = (cinfo->err->trace_level >= 1);
    INT32 length;
    unsigned int ch;
    unsigned int lastch = 0;

    length = jpeg_getc(cinfo) << 8;
    length += jpeg_getc(cinfo);
    length -= 2;          /* discount the length word itself */

    if (traceit)
    {
        if (cinfo->unread_marker == JPEG_COM)
        {
            fprintf(stderr, "Comment, length %ld:\n", (long) length);
        }
        else            /* assume it is an APPn otherwise */
            fprintf(stderr, "APP%d, length %ld:\n",
                    cinfo->unread_marker - JPEG_APP0, (long) length);
    }

    while (--length >= 0)
    {
        ch = jpeg_getc(cinfo);

        if (traceit)
        {
            /* Emit the character in a readable form.
             * Nonprintables are converted to \nnn form,
             * while \ is converted to \\.
             * Newlines in CR, CR/LF, or LF form will be printed as one newline.
             */
            if (ch == '\r')
            {
                fprintf(stderr, "\n");
            }
            else if (ch == '\n')
            {
                if (lastch != '\r')
                {
                    fprintf(stderr, "\n");
                }
            }
            else if (ch == '\\')
            {
                fprintf(stderr, "\\\\");
            }
            else if (isprint(ch))
            {
                putc(ch, stderr);
            }
            else
            {
                fprintf(stderr, "\\%03o", ch);
            }

            lastch = ch;
        }
    }

    if (traceit)
    {
        fprintf(stderr, "\n");
    }

    return TRUE;
}

int ReadTiffFilePageAndWriteToJpegFile(FILE *fTif, int TiffFileOffset, FILE *fJpegFile)
{
    int Rev = -1, len, offset = 0, pagebytes = 0;
    UBYTE buf[20];

    fseek(fTif, offset, SEEK_SET);

    if (TiffFileOffset == 0)
    {
        if ((len = fread(buf, 1, TIFF_HEADER_LENGTH, fTif)) == TIFF_HEADER_LENGTH)
        {
            if ((buf[0] != 0x49) || (buf[1] != 0x49) || (buf[2] != 0x2a))
            {
                return Rev;
            }
            else
            {
                TiffFileOffset = 8;
            }
        }
        else
        {
            return Rev;
        }
    }

    if (TiffFileOffset > 0)
    {
        TiffFileOffset += 0x3a;
        fseek(fTif, TiffFileOffset, SEEK_SET);

        if ((len = fread(buf, 1, 4, fTif)) == 4)
        {
            if (buf[0] != JPEG_COMPRESSION)
            {
                TRACE0("TIF: not jpeg compression. 0");
                return Rev;
            }
            else
            {
                TiffFileOffset += 4;
            }
        }
        else
        {
            TRACE0("TIF: unexpected end of the tif file. 0");
            return Rev;
        }

        TiffFileOffset += 0x20;
        fseek(fTif, TiffFileOffset, SEEK_SET);

        if ((len = fread(buf, 1, 4, fTif)) == 4)
        {
            offset = ((buf[3] << 24) & 0xff000000) + ((buf[2] << 16) & 0x00ff0000) + ((buf[1] << 8) & 0x00ff00) + ((buf[0]) & 0xff);
            TiffFileOffset += 0x30;
        }
        else
        {
            TRACE0("TIF: unexpected end of the tif file.");
            return Rev;
        }

        fseek(fTif, TiffFileOffset, SEEK_SET);

        if ((len = fread(buf, 1, 4, fTif)) == 4)
        {
            pagebytes = ((buf[3] << 24) & 0xff000000) + ((buf[2] << 16) & 0x00ff0000) + ((buf[1] << 8) & 0x00ff00) + ((buf[0]) & 0xff);
            TiffFileOffset += 0x34;
        }
        else
        {
            TRACE0("TIF: unexpected end of the tif file.");
            return Rev;
        }

        fseek(fTif, TiffFileOffset, SEEK_SET);

        if ((len = fread(buf, 1, 4, fTif)) == 4)
        {
            Rev = ((buf[3] << 24) & 0xff000000) + ((buf[2] << 16) & 0x00ff0000) + ((buf[1] << 8) & 0x00ff00) + ((buf[0]) & 0xff);
        }
        else
        {
            TRACE0("TIF: unexpected end of the tif file.");
            return Rev;
        }
    }

    if (Rev >= 0)
    {
        fseek(fTif, offset, SEEK_SET);

        while ((len = fread(buf, 1, sizeof(buf), fTif)) == sizeof(buf))
        {
            fwrite(buf, 1, sizeof(buf), fJpegFile);
            len = 0;
        }

        if (len > 0)
        {
            fwrite(buf, 1, len, fJpegFile);
        }
    }

    return Rev;
}


// return value:
//  -1: wrong tif file
//   0: at the last page of the tiff file
//   1: still have next page
SWORD FileConversionFromTiffToBmp(FILE *fTif, FILE *fBmp)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    djpeg_dest_ptr dest_mgr = NULL;
    JDIMENSION num_scanlines;
    FILE *fJpegFile;
    SWORD Rev = -1;

    if ((fJpegFile = fopen("image.jpg", "wb")) == 0)
    {
        printf("Can not create image.jpg!\n");
        return Rev;
    }

    sgTiffFileOffset = ReadTiffFilePageAndWriteToJpegFile(fTif, sgTiffFileOffset, fJpegFile);

    if (sgTiffFileOffset == -1)
    {
        TRACE0("TIF: wrong format of the input tif file during converting tiff to bmp.");
        fclose(fJpegFile);
        return Rev;
    }
    else if (sgTiffFileOffset == 0)
    {
        TRACE0("TIF: last page of the tif file.");
        Rev = 0;
    }
    else
    {
        TRACE0("TIF: The tif file still has next page.");
        Rev = 1;
    }

    fclose(fJpegFile);

    if ((fJpegFile = fopen("image.jpg", "rb")) == 0)
    {
        printf("image.jpg missed!\n");
        return Rev;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jerr.addon_message_table = cdjpeg_message_table;
    jerr.first_addon_message = JMSG_FIRSTADDONCODE;
    jerr.last_addon_message = JMSG_LASTADDONCODE;

    jpeg_set_marker_processor(&cinfo, JPEG_COM, print_text_marker);
    jpeg_set_marker_processor(&cinfo, JPEG_APP0 + 12, print_text_marker);

    jpeg_stdio_src(&cinfo, fJpegFile);
    (void) jpeg_read_header(&cinfo, TRUE);
    dest_mgr = jinit_write_bmp(&cinfo, FALSE);
    dest_mgr->output_file = fBmp;
    (void) jpeg_start_decompress(&cinfo);
    (*dest_mgr->start_output)(&cinfo, dest_mgr);

    while (cinfo.output_scanline < cinfo.output_height)
    {
        num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer, dest_mgr->buffer_height);
        (*dest_mgr->put_pixel_rows)(&cinfo, dest_mgr, num_scanlines);
    }

    (*dest_mgr->finish_output)(&cinfo, dest_mgr);
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fJpegFile);
    remove("image.jpg");
    return Rev;
}


static djpeg_dest_ptr DecodeJpegFileToBiLevelBWBufferInit(FILE *fJpeg, struct jpeg_decompress_struct *pDeJpeg, struct jpeg_error_mgr *pJerr)
{
    djpeg_dest_ptr dest_mgr = NULL;
    int i;

    pDeJpeg->err = jpeg_std_error(pJerr);
    jpeg_create_decompress(pDeJpeg);

    pJerr->first_addon_message = JMSG_FIRSTADDONCODE;
    pJerr->last_addon_message = JMSG_LASTADDONCODE;

    jpeg_stdio_src(pDeJpeg, fJpeg);
    (void) jpeg_read_header(pDeJpeg, TRUE);
    dest_mgr = jinit_write_jpeg2rgb(pDeJpeg);
    (void) jpeg_start_decompress(pDeJpeg);
    (*dest_mgr->start_output)(pDeJpeg, dest_mgr);

    for (i = 0; i < 256; i++)
    {
        x_map[i].x = 0.29900f * (double) i;
        y_map[i].x = 0.58700f * (double) i;
        z_map[i].x = 0.11400f * (double) i;
        x_map[i].y = 0.29900f * (double) i;
        y_map[i].y = 0.58700f * (double) i;
        z_map[i].y = 0.11400f * (double) i;
        x_map[i].z = 0.29900f * (double) i;
        y_map[i].z = 0.58700f * (double) i;
        z_map[i].z = 0.11400f * (double) i;
    }

    return dest_mgr;
}



static void ConvertRGB2Y(struct jpeg_decompress_struct *pDeJpeg, UBYTE *rgboBuf)
{
    register TBgr *q;
    register unsigned char blue, green, red;
    TBgr pixel;
    int j;

    q = (TBgr *)rgboBuf;

    for (j = 0; j < (long) pDeJpeg->output_width; j++)
    {
        red = q->red;
        green = q->green;
        blue = q->blue;
        pixel.red = (UBYTE)(x_map[red].x + y_map[green].x + z_map[blue].x);
        pixel.green = (UBYTE)(x_map[red].y + y_map[green].y + z_map[blue].y);
        pixel.blue = (UBYTE)(x_map[red].z + y_map[green].z + z_map[blue].z);
        q->red = pixel.red;
        q->green = pixel.green;
        q->blue = pixel.blue;
        q++;
    }
}

#endif
