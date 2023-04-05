
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

#include <string.h>
#include "imgprodte.h"
#include "dsmdef.h"
#include "t4ext.h"
#include "errorcode.h"
#if SUPPORT_TX_PURE_TEXT_FILE
#include "asciicov.h"
#endif
#if SUPPORT_JPEG_CODEC
#include "jpeglib.h"

extern CONST int YResoValue_Tiff[];
#endif
#if SUPPORT_CONVERSION_BETWEEN_T4_T6

extern UWORD ValidLenImageCB(TImageReEncodeCB *pImgReEncodeCB);
extern UWORD ReadStringFromImageCB(TImageReEncodeCB *pImgReEncodeCB, UBYTE *pDataOut, UWORD len);
extern void InitImageReEncodeCB(TImageReEncodeCB *pImgReEncodeCB);
extern SWORD GetScanLineLengthInPixels(UBYTE PaperSize, SWORD XRes);
extern UWORD IdleLenImageCB(TImageReEncodeCB *pImgReEncodeCB);
extern UWORD WriteStringToImageCB(TImageReEncodeCB *pImgReEncodeCB, UBYTE *pDataIn, UWORD len);
#endif

#if SUPPORT_JPEG_CODEC
struct jpeg_compress_struct cinfo;
struct jpeg_error_mgr jerr;
#endif

void ImageProcessInit(TImgProDteStruct *pImg)
{
    memset(pImg, 0, sizeof(TImgProDteStruct));
    pImg->Direction = MSG_DIR_IDLE;
    pImg->ImgConversion = IMG_NO_CONVERSION;
}

#if SUPPORT_CONVERSION_BETWEEN_T4_T6

SDWORD GetCompression(UBYTE Compression)
{
    SDWORD Comp;

    if (Compression == DSM_DF_T6_2D)
    {
        Comp = T6_2D_MMR;
    }
    else if (Compression == DSM_DF_T4_2D)
    {
        Comp = T4_2D_MR;
    }
    else
    {
        Comp = T4_1D_MH;
    }

    return Comp;
}


SDWORD GetMinScanLineBits(UBYTE ScanTime, UBYTE Rate, UBYTE yRes)
{
    SDWORD MinBits;

    MinBits = 12 * ((ScanTime + 1) >> 1);

    if ((yRes == VR_NORMAL_385) || (yRes == VR_100L_INCH))
    {
        if ((ScanTime & 0x01) == 0)
        {
            MinBits <<= 1;
        }
    }

    MinBits = MinBits * (Rate + 1);

    return MinBits;
}

SDWORD Get2DEncodeK(UBYTE yRes)
{
    SDWORD Rev = 2;

    switch (yRes)
    {
        case VR_NORMAL_385:
        case VR_100L_INCH:
            break;
        case VR_FINE_77:
        case VR_200L_INCH:
            Rev = 4;
            break;
        case VR_300L_INCH:
            Rev = 6;
            break;
        case VR_PERFECT_154:
        case VR_400L_INCH:
            Rev = 8;
            break;
        case VR_600L_INCH:
            Rev = 12;
            break;
        case VR_800L_INCH:
            Rev = 16;
            break;
        case VR_1200L_INCH:
            Rev = 24;
            break;
    }

    return Rev;
}

void InitImageCodec(TImgProDteStruct *pImg)
{
    TImgFormat *pImgIn = &(pImg->SrcFormat);
    SDWORD InImageW, InCompression;
    TImgFormat *pImgOut = &(pImg->DesFormat);
    SDWORD OutImageW, OutCompression;
    SDWORD minimumScanLineBits;

    memset(pImg->Bitmap, 0, sizeof(pImg->Bitmap));
    pImg->pDecoderLine = pImg->Bitmap + (0 * BITMAP_LENGTH);
    pImg->pDecoderReferenceLine = pImg->Bitmap + (1 * BITMAP_LENGTH);
    pImg->pEncoderLine = pImg->Bitmap + (2 * BITMAP_LENGTH);
    pImg->pEncoderReferenceLine = pImg->Bitmap + (3 * BITMAP_LENGTH);

    InImageW = GetScanLineLengthInPixels(pImgIn->PaperSize, pImgIn->XRes);
    InCompression = GetCompression(pImgIn->Compression);

    if (t4Decoder_init(&(pImg->t4Decoder), InImageW, InCompression, pImg->pDecoderReferenceLine) < 0)
    {
        pImg->errorCode = ERROR_IMG_INITIALIZING_DECODER;
        TRACE0("IMG: Image Decoder Init Failed.");
        return;
    }

    OutImageW = GetScanLineLengthInPixels(pImgOut->PaperSize, pImgOut->XRes);
    OutCompression = GetCompression(pImgOut->Compression);
    minimumScanLineBits = GetMinScanLineBits((UBYTE)(pImgOut->ScanTimePerLine), pImgOut->BitRate, pImgOut->YRes);

    if (t4Encoder_init(&(pImg->t4Encoder), OutImageW, (pImgIn->FillOrder == FILLORDER_LSB_FIRST), OutCompression, Get2DEncodeK(pImgOut->YRes), minimumScanLineBits, pImg->pEncoderReferenceLine) < 0)
    {
        pImg->errorCode = ERROR_IMG_INITIALIZING_ENCODER;
        TRACE0("IMG: Image Encoder Init Failed.");
        return;
    }

    if (SCALER_init(&(pImg->scaler), pImgIn, pImgOut) == SCALER_INIT_ERROR)
    {
        pImg->errorCode = ERROR_IMG_INITIALIZING_SCALER;
        TRACE0("IMG: Image Scaler Init Failed.");
        return;
    }
}
#endif

#if (SUPPORT_JPEG_CODEC && SUPPORT_TX_COVER_PAGE)
SWORD GetImageLengthInPixels(UBYTE PaperSize, SWORD YRes)
{
    SWORD Rev = 0, Temp;

    Temp = YResoValue_Tiff[YRes - 1];

    if (YRes < 4)
    {
        if (PaperSize == DSM_A4_LETTER_LEGAL)
        {
            Rev = (SWORD)(TIFF_A4_LENGTH_METRIC / 10.0 * Temp);
        }
        else if (PaperSize == DSM_B4)
        {
            Rev = (SWORD)(TIFF_B4_LENGTH_METRIC / 10.0 * Temp);
        }
        else if (PaperSize == DSM_A3)
        {
            Rev = (SWORD)(TIFF_A3_LENGTH_METRIC / 10.0 * Temp);
        }
    }
    else
    {
        if (PaperSize == DSM_A4_LETTER_LEGAL)
        {
            Rev = (SWORD)(TIFF_A4_LENGTH_METRIC / 25.4 * Temp);
        }
        else if (PaperSize == DSM_B4)
        {
            Rev = (SWORD)(TIFF_B4_LENGTH_METRIC / 25.4 * Temp);
        }
        else if (PaperSize == DSM_A3)
        {
            Rev = (SWORD)(TIFF_A3_LENGTH_METRIC / 25.4 * Temp);
        }
    }

    return Rev;
}

void JpegEncoderCoverPageInit(struct jpeg_compress_struct *pcinfo, struct jpeg_error_mgr *pjerr, TImgProDteStruct *pImg)
{

    pcinfo->err = jpeg_std_error(pjerr);
    jpeg_create_compress(pcinfo);
    jpeg_stdio_dest(pcinfo, NULL);

    cinfo.image_width = GetScanLineLengthInPixels(pImg->DesFormat.PaperSize, pImg->DesFormat.XRes);
    cinfo.image_height = GetImageLengthInPixels(pImg->DesFormat.PaperSize, pImg->DesFormat.YRes);
    cinfo.image_height = (cinfo.image_height / 16) * 16;
    cinfo.input_components = 3;
    cinfo.SpatialResolution = YResoValue_Tiff[pImg->DesFormat.YRes - 1];
    //cinfo.in_color_space = JCS_RGB;
    cinfo.in_color_space = JCS_ITULAB;
    jpeg_set_defaults(pcinfo);
    jpeg_set_quality(pcinfo, 75, TRUE);
    jpeg_start_compress(pcinfo, TRUE);
}
#endif

void ImgControl(GAO_Msg *pDsmImg, TImgProDteStruct *pImg)
{
#if (SUPPORT_TX_PURE_TEXT_FILE || SUPPORT_TX_COVER_PAGE)
    TImageReEncodeCB *pImgReEncodeCB = &(pImg->ImgReEncodeCB);
    TImgFormat *pSrc = &(pImg->SrcFormat);
#endif
#if ((SUPPORT_JPEG_CODEC && SUPPORT_TX_COVER_PAGE) || (SUPPORT_TX_PURE_TEXT_FILE) || (SUPPORT_TX_COVER_PAGE))
    TImgFormat *pDes = &(pImg->DesFormat);
#endif
#if (SUPPORT_TX_COVER_PAGE || SUPPORT_TX_PURE_TEXT_FILE)
    SDWORD imageW, functionResult;
#endif

    if ((pDsmImg->Direction == MSG_DOWN_LINK) && (pDsmImg->MsgType == MSG_TYPE_CONTROL))
    {
        if (pDsmImg->MsgCode == CTL_IMAGE_CONVERSION)
        {
            pImg->Direction = pDsmImg->Buf[0];
            pImg->ImgConversion = pDsmImg->Buf[1];
#if (SUPPORT_TX_COVER_PAGE)
            if (pImg->Direction == DSM_FAX_TRANSMISSION_WITH_COVERP)
            {
                memcpy(pDes, &(pDsmImg->Buf[2 + sizeof(TImgFormat)]), sizeof(TImgFormat));
                pImg->Direction = DSM_FAX_TRANSMISSION;
                //if((pDes->Compression == DSM_DF_T4_1D) || (pDes->Compression == DSM_DF_T4_2D) || (pDes->Compression == DSM_DF_T6_2D))
                {
                    if (pImg->ImgConversion == 1)
                    {
                        memcpy(pSrc, &(pDsmImg->Buf[2]), sizeof(TImgFormat));
                    }
                    else if ((pImg->ImgConversion == 2) || (pImg->ImgConversion == 0))
                    {
                        memcpy(pSrc, pDes, sizeof(TImgFormat));
                    }

                    InitImageCodec(pImg);
                    InitImageReEncodeCB(pImgReEncodeCB);

                    if (pDes->PaperSize == DSM_A3)
                    {
                        imageW = ImageWidth_Tiff[pDes->PaperSize - 2 ][pDes->XRes - 2];
                    }
                    else
                    {
                        imageW = ImageWidth_Tiff[pDes->PaperSize - 1 ][pDes->XRes - 2];
                    }

                    functionResult = ASCII2BMP_init(&pImg->asciiStruct, imageW, MARGIN_WIDTH, LINE_HEIGHT, 0);

                    if (functionResult == ASCII2BMP_ERROR)
                    {
                        TRACE0("IMG: Ascii2Bmp initialization ERROR!");
                    }

                    memset(pDsmImg, 0, sizeof(GAO_Msg));
#if SUPPORT_JPEG_CODEC
                    if ((pDes->Compression == DSM_DF_JPEG_MODE) && (pImg->TxCoverP == 0))
                    {
                        JpegEncoderCoverPageInit(&cinfo, &jerr, pImg);
                    }
#endif
                    pImg->TxCoverP = 1;
                    return;
                }
                //else
                //{
                //    TRACE0("IMG: Only support T4 and T6 cover page. No Cover page to be sent.");
                //}
            }

            pImg->TxCoverP = 0;
#endif

#if SUPPORT_CONVERSION_BETWEEN_T4_T6

            if (pImg->ImgConversion == 1)
            {
                memcpy(&(pImg->SrcFormat), &(pDsmImg->Buf[2]), sizeof(TImgFormat));
                memcpy(&(pImg->DesFormat), &(pDsmImg->Buf[2 + sizeof(TImgFormat)]), sizeof(TImgFormat));

                if (pImg->Direction == DSM_FAX_TRANSMISSION)
                {
                    InitImageCodec(pImg);
                }

                TRACE2("IMG: Image conversion PaperSize from %d to %d.", pImg->SrcFormat.PaperSize, pImg->DesFormat.PaperSize);
                TRACE2("IMG: Image conversion Compression from %d to %d.", pImg->SrcFormat.Compression, pImg->DesFormat.Compression);
                TRACE2("IMG: Image conversion XRes from %d to %d.", pImg->SrcFormat.XRes, pImg->DesFormat.XRes);
                TRACE2("IMG: Image conversion YRes from %d to %d.", pImg->SrcFormat.YRes, pImg->DesFormat.YRes);
            }
#if SUPPORT_TX_PURE_TEXT_FILE
            else if (pImg->ImgConversion == 2) // tx text file
            {
                memcpy(pDes, &(pDsmImg->Buf[2 + sizeof(TImgFormat)]), sizeof(TImgFormat));
                memcpy(pSrc, pDes, sizeof(TImgFormat));

                InitImageCodec(pImg);
                InitImageReEncodeCB(pImgReEncodeCB);

                if (pDes->PaperSize == DSM_A3)
                {
                    imageW = ImageWidth_Tiff[pDes->PaperSize - 2 ][pDes->XRes - 2];
                }
                else
                {
                    imageW = ImageWidth_Tiff[pDes->PaperSize - 1 ][pDes->XRes - 2];
                }

                functionResult = ASCII2BMP_init(&pImg->asciiStruct, imageW, MARGIN_WIDTH, LINE_HEIGHT, 0);

                if (functionResult == ASCII2BMP_ERROR)
                {
                    TRACE0("IMG: Ascii2Bmp initialization ERROR!");
                }
            }
#endif            
            else
            {
                pImg->ImgConversion = 0;
            }

#else
#if SUPPORT_TX_PURE_TEXT_FILE
            if (pImg->ImgConversion == 2)
            {
                TRACE0("IMG: image conversion switch should be open to transmit a text file.");
            }
#endif
            pImg->ImgConversion = 0;
#endif
            memset(pDsmImg, 0, sizeof(GAO_Msg));
        }
    }
}

#if (SUPPORT_TX_PURE_TEXT_FILE || SUPPORT_TX_COVER_PAGE)
int ASCII2BMPloadChars(ascii2BmpStruct *pAscConverter, UBYTE *pChars, int numChars)
{
    ascii2BmpStruct *pac = pAscConverter;
    UBYTE c;
    UBYTE *pLastSpace = pac->pTail;
    int hasEnter, islastline;

    hasEnter = 0;

    while ((pac->pCurrent < pac->pTail) && (numChars > 0))
    {
        c = *pChars++;
        numChars--;

        if (c == 0x0D)
        {
            hasEnter = 1;
            numChars--;

            while (pac->pCurrent < pac->pTail)
            {
                *(pac->pCurrent++) = ' ';
            }
        }
        else if (c < START_CHAR || c > END_CHAR)
        {
            *(pac->pCurrent++) = ' ';
        }
        else
        {
            if (c == ' ') { pLastSpace = pac->pCurrent; }

            *(pac->pCurrent++) = c;
        }
    }

    if (pac->pCurrent >= pac->pTail)
    {
        pac->pCurrent = pac->asciiLine;
        islastline = 0;
    }
    else
    {
        islastline = 1;

        while (pac->pCurrent < pac->pTail)
        {
            *(pac->pCurrent++) = ' ';
        }
    }

    if (!hasEnter && !islastline && (pLastSpace != pac->pTail))
    {
        if (((*(pac->pTail - 1) >= 'a') && (*(pac->pTail - 1) <= 'z')) || ((*(pac->pTail - 1) >= 'A') && (*(pac->pTail - 1) <= 'Z')))
        {
            if (((*pChars >= 'a') && (*pChars <= 'z')) || ((*pChars >= 'A') && (*pChars <= 'Z')))
            {
                numChars += pac->pTail - pLastSpace - 1;

                while (pLastSpace < pac->pTail)
                {
                    *(pLastSpace++) = ' ';
                }
            }
        }

        if (*pChars == ' ')
        {
            numChars--;
        }
    }

    return numChars;
}



void TextConversionToImageProcess(GAO_Msg *pMsgIn, TImgProDteStruct *pImg)
{
    TImageReEncodeCB *pImgReEncodeCB = &(pImg->ImgReEncodeCB);
    UBYTE ImageOut[MAX_IMAGE_LEN_AFTER_ENCODING];
    UBYTE *pDataOut = ImageOut, *ptByte1;
    SDWORD len, offset = 0, funcResult1, i, j, i1, LenOut = 0, lastColor;
    UWORD *pTemp;
    UBYTE *pDataIn = pMsgIn->Buf + 3;

    len = ((pMsgIn->Buf[1] << 8) & 0xff00) + pMsgIn->Buf[2];

    while (offset < len)
    {
        funcResult1 = ASCII2BMPloadChars(&pImg->asciiStruct, pDataIn + offset, len - offset);
        offset += len - funcResult1;

        ptByte1 = (UBYTE *)pImg->pDecoderLine;

        for (i1 = 0; i1 < pImg->asciiStruct.lineHeight; i1++)
        {
            funcResult1 = ASCII2BMP_apply(&pImg->asciiStruct, ptByte1);
            lastColor = WHITE;
            i = 0;
            j = 0;
            pImg->pEncoderLine[0] = 0;

            while (i < pImg->t4Encoder.pageWidth)
            {
                if (ptByte1[i] == lastColor)
                {
                    pImg->pEncoderLine[j]++;
                }
                else
                {
                    pImg->pEncoderLine[++j] = 1;
                }

                lastColor = ptByte1[i];
                i++;
            }

            funcResult1 = t4Encoder_apply(&pImg->t4Encoder, pImg->pEncoderLine, pImg->pEncoderReferenceLine, pDataOut + LenOut);

            if (funcResult1 < 0)
            {
                pImg->errorCode = funcResult1;
                TRACE0("IMG: img encode ERROR.");
            }
            else
            {
                LenOut += funcResult1;
            }

            pTemp = pImg->pEncoderReferenceLine;
            pImg->pEncoderReferenceLine = pImg->pEncoderLine;
            pImg->pEncoderLine = pTemp;
        }

        if (IdleLenImageCB(pImgReEncodeCB) >= LenOut)
        {
            WriteStringToImageCB(pImgReEncodeCB, ImageOut, (UWORD)LenOut);
            LenOut = 0;
        }
        else
        {
            TRACE0("IMG: ERROR, Img Circular buffer overflows.\n");
            LenOut = 0;
        }
    }

    pImg->ImgFlagBackup = pMsgIn->Buf[0];
}
#endif

#if (SUPPORT_TX_COVER_PAGE && SUPPORT_JPEG_CODEC)
void TxJpegCoverPageImgProcess(GAO_Msg *pMsgIn, TImgProDteStruct *pImg)
{
    TImageReEncodeCB *pImgReEncodeCB = &(pImg->ImgReEncodeCB);
    UBYTE ImageOut[MAX_IMAGE_LEN_AFTER_ENCODING];
    UBYTE *ptByte1 = &(ImageOut[0]);
    SDWORD len, offset = 0, funcResult1, i, i1;
    UBYTE *pTemp = (UBYTE *)(pImg->Bitmap);
    UBYTE *pDataIn = pMsgIn->Buf + 3;
    UBYTE *pInputData[1];

    len = ((pMsgIn->Buf[1] << 8) & 0xff00) + pMsgIn->Buf[2];

    while (offset < len)
    {
        funcResult1 = ASCII2BMPloadChars(&pImg->asciiStruct, pDataIn + offset, len - offset);
        offset += len - funcResult1;

        for (i1 = 0; i1 < pImg->asciiStruct.lineHeight; i1++)
        {
            funcResult1 = ASCII2BMP_apply(&pImg->asciiStruct, ptByte1);
            i = 0;

            while (i < pImg->t4Encoder.pageWidth)
            {
                if (ptByte1[i] == WHITE)
                {
                    pTemp[i*3] = IMG_LAB_WHITE_L;
                    pTemp[i*3+1] = IMG_LAB_WHITE_A;
                    pTemp[i*3+2] = IMG_LAB_WHITE_B;
                }
                else
                {
                    pTemp[i*3] = IMG_LAB_BLACK_L;
                    pTemp[i*3+1] = IMG_LAB_BLACK_A;
                    pTemp[i*3+2] = IMG_LAB_BLACK_B;
                }

                i++;
            }

            pInputData[0] = pTemp;
            jpeg_write_scanlines(&cinfo, pInputData, 1);
        }

        if (cinfo.OutputCompressedJpegData > 0)
        {
            if (IdleLenImageCB(pImgReEncodeCB) >= cinfo.OutputCompressedJpegData)
            {
                WriteStringToImageCB(pImgReEncodeCB, &(cinfo.JpegDataBufTemp[0]), (UWORD)(cinfo.OutputCompressedJpegData));
            }
            else
            {
                TRACE0("IMG: ERROR, Img Circular buffer overflows. 1\n");
            }

            cinfo.OutputCompressedJpegData = 0;
        }

        if (pMsgIn->Buf[0] == TIFF_PMC_COVER_PAGE_END)
        {
            jpeg_finish_compress(&cinfo);
        }

        if (cinfo.OutputCompressedJpegData > 0)
        {
            if (IdleLenImageCB(pImgReEncodeCB) >= cinfo.OutputCompressedJpegData)
            {
                WriteStringToImageCB(pImgReEncodeCB, &(cinfo.JpegDataBufTemp[0]), (UWORD)(cinfo.OutputCompressedJpegData));
            }
            else
            {
                TRACE0("IMG: ERROR, Img Circular buffer overflows. 1\n");
            }

            cinfo.OutputCompressedJpegData = 0;
        }


        if (pMsgIn->Buf[0] == TIFF_PMC_COVER_PAGE_END)
        {
            jpeg_abort((j_common_ptr)(&cinfo));
            jpeg_destroy_compress(&cinfo);
        }
    }

    pImg->ImgFlagBackup = pMsgIn->Buf[0];
}
#endif

void ImageConversionMain(TImgApiStruct *pApiImg, TImgProDteStruct *pImg)
{
#if SUPPORT_CONVERSION_BETWEEN_T4_T6
    TImageReEncodeCB *pImgReEncodeCB = &(pImg->ImgReEncodeCB);
    UWORD len, len1;
#endif
    GAO_Msg *pDsmImg = pApiImg->pDsmImg;
    GAO_Msg *pImgDce = pApiImg->pImgDce;
#if (SUPPORT_TX_COVER_PAGE && SUPPORT_TX_PURE_TEXT_FILE)
    SDWORD imageW, functionResult;
#endif

    if ((pImgDce->Direction == MSG_UP_LINK) && (pImgDce->MsgType == MSG_TYPE_REPORT) && (pImgDce->MsgCode == RPT_HANGUP_STATUS))
    {
#if SUPPORT_CONVERSION_BETWEEN_T4_T6
        memset(&(pImg->t4Decoder), 0, sizeof(t4DecoderStruct));
        memset(&(pImg->t4Encoder), 0, sizeof(t4EncoderStruct));
        memset(&(pImg->scaler), 0, sizeof(ScalerStruct));
        memset(&(pImg->SrcFormat), 0, sizeof(TImgFormat));
        memset(&(pImg->DesFormat), 0, sizeof(TImgFormat));
        pImg->ImgReEncodeCB.IndexIn = 0;
        pImg->ImgReEncodeCB.IndexOut = 0;
        pImg->errorCode = 0;
        pImg->errorCount = 0;
        pImg->functionResult = 0;
        pImg->Direction = 0;
        pImg->ImgFlagBackup = 0;
        pImg->ImgConversion = 0;
        pImg->XCon = 0;
        pImg->YCon = 0;
        pImg->CompCon = 0;
#else
        memset(pImg, 0, sizeof(TImgProDteStruct));
#endif
    }

#if (SUPPORT_TX_COVER_PAGE)
    if (pImg->TxCoverP != 0)
    {
        if (pImg->Direction == DSM_FAX_TRANSMISSION)
        {
            if ((pDsmImg->Direction == MSG_DOWN_LINK) && (pDsmImg->MsgType == MSG_TYPE_IMAGE_DATA) && (pDsmImg->MsgCode == IMG_DATA_AND_STATUS))
            {
                if ((pDsmImg->Buf[0] == TIFF_PMC_COVER_PAGE) || (pDsmImg->Buf[0] == TIFF_PMC_COVER_PAGE_END))
                {
                    if (ValidLenImageCB(pImgReEncodeCB) == 0)
                    {
#if SUPPORT_JPEG_CODEC                        
                        if (pImg->DesFormat.Compression == DSM_DF_JPEG_MODE)
                        {
                            TxJpegCoverPageImgProcess(pDsmImg, pImg);
                        }
                        else
#endif
                        {
                            TextConversionToImageProcess(pDsmImg, pImg);
                        }

                        if (pDsmImg->Buf[0] == TIFF_PMC_COVER_PAGE_END)
                        {
                            pImg->TxCoverP = 2;
                        }

                        memset(pDsmImg, 0, sizeof(GAO_Msg));
                    }
                }
                else
                {
                    memset(pDsmImg, 0, sizeof(GAO_Msg));
                    TRACE0("IMG: Got wrong Cover page data msg!!!\n");
                }
            }
        }

        if (pImgDce->Direction == MSG_DIR_IDLE)
        {
            len = ValidLenImageCB(pImgReEncodeCB);

            if (len > 0)
            {
                len1 = MAX_LEN_API_MSG;
                pImgDce->Direction = MSG_DOWN_LINK;
                pImgDce->MsgType = MSG_TYPE_IMAGE_DATA;
                pImgDce->MsgCode = IMG_DATA_AND_STATUS;

                if (len > len1)
                {
                    pImgDce->Buf[0] = IMG_IN_MIDDLE;
                    pImgDce->Buf[1] = (UBYTE)((len1 >> 8) & 0xff);
                    pImgDce->Buf[2] = (UBYTE)(len1 & 0xff);
                    ReadStringFromImageCB(pImgReEncodeCB, &(pImgDce->Buf[3]), len1);
                }
                else
                {
                    if (pImg->TxCoverP == 2)
                    {
                        pImgDce->Buf[0] = TIFF_PMC_NEXT_PAGE;
                    }
                    else
                    {
                        pImgDce->Buf[0] = IMG_IN_MIDDLE;
                    }

                    pImgDce->Buf[1] = (UBYTE)((len >> 8) & 0xff);
                    pImgDce->Buf[2] = (UBYTE)(len & 0xff);
                    ReadStringFromImageCB(pImgReEncodeCB, &(pImgDce->Buf[3]), len);

                    if (pImg->TxCoverP == 2)
                    {
                        pImg->TxCoverP = 0;

                        if (pImg->ImgConversion == 1)
                        {
                            if (pImg->Direction == DSM_FAX_TRANSMISSION)
                            {
                                InitImageCodec(pImg);
                                InitImageReEncodeCB(pImgReEncodeCB);
                            }

                            TRACE2("IMG: Image conversion PaperSize from %d to %d.", pImg->SrcFormat.PaperSize, pImg->DesFormat.PaperSize);
                            TRACE2("IMG: Image conversion Compression from %d to %d.", pImg->SrcFormat.Compression, pImg->DesFormat.Compression);
                            TRACE2("IMG: Image conversion XRes from %d to %d.", pImg->SrcFormat.XRes, pImg->DesFormat.XRes);
                            TRACE2("IMG: Image conversion YRes from %d to %d.", pImg->SrcFormat.YRes, pImg->DesFormat.YRes);
                        }
#if SUPPORT_TX_PURE_TEXT_FILE
                        else if (pImg->ImgConversion == 2) // start to tx text file
                        {
                            InitImageCodec(pImg);
                            InitImageReEncodeCB(pImgReEncodeCB);

                            if (pImg->DesFormat.PaperSize == DSM_A3)
                            {
                                imageW = ImageWidth_Tiff[pImg->DesFormat.PaperSize - 2 ][pImg->DesFormat.XRes - 2];
                            }
                            else
                            {
                                imageW = ImageWidth_Tiff[pImg->DesFormat.PaperSize - 1 ][pImg->DesFormat.XRes - 2];
                            }

                            functionResult = ASCII2BMP_init(&pImg->asciiStruct, imageW, MARGIN_WIDTH, LINE_HEIGHT, 0);

                            if (functionResult == ASCII2BMP_ERROR)
                            {
                                TRACE0("IMG: Ascii2Bmp initialization ERROR!");
                            }
                        }
#endif
                        else
                        {
                            pImg->ImgConversion = 0;
                        }
                    }
                }
            }
        }

        return;
    }
#endif

    if (pImg->ImgConversion == 0) // no image conversion
    {
        if (pImg->Direction == DSM_FAX_TRANSMISSION)
        {
            if ((pDsmImg->Direction == MSG_DOWN_LINK) && (pImgDce->Direction == MSG_DIR_IDLE))
            {
                memcpy(pImgDce, pDsmImg, sizeof(GAO_Msg));
                memset(pDsmImg, 0, sizeof(GAO_Msg));
            }
        }
        else if (pImg->Direction == DSM_FAX_RECEIVING)
        {
            if ((pImgDce->Direction == MSG_UP_LINK) && (pDsmImg->Direction == MSG_DIR_IDLE) && (pImgDce->MsgType == MSG_TYPE_IMAGE_DATA))
            {
                memcpy(pDsmImg, pImgDce, sizeof(GAO_Msg));
                memset(pImgDce, 0, sizeof(GAO_Msg));
            }
        }
    }

#if SUPPORT_CONVERSION_BETWEEN_T4_T6
    else if (pImg->ImgConversion == 1) // image format conversion
    {
        if (pImg->Direction == DSM_FAX_TRANSMISSION)
        {
            if ((pDsmImg->Direction == MSG_DOWN_LINK) && (pImgDce->Direction == MSG_DIR_IDLE))
            {
                if (ValidLenImageCB(pImgReEncodeCB) == 0)
                {
                    ImageConversionDTE(pImgDce, pDsmImg, pImg);
                    memset(pDsmImg, 0, sizeof(GAO_Msg));
                }
            }

            if (pImgDce->Direction == MSG_DIR_IDLE)
            {
                len = ValidLenImageCB(pImgReEncodeCB);

                if (len > 0)
                {
                    len1 = MAX_LEN_API_MSG;
                    pImgDce->Direction = MSG_DOWN_LINK;
                    pImgDce->MsgType = MSG_TYPE_IMAGE_DATA;
                    pImgDce->MsgCode = IMG_DATA_AND_STATUS;

                    if (len > len1)
                    {
                        pImgDce->Buf[0] = IMG_IN_MIDDLE;
                        pImgDce->Buf[1] = (UBYTE)((len1 >> 8) & 0xff);
                        pImgDce->Buf[2] = (UBYTE)(len1 & 0xff);
                        ReadStringFromImageCB(pImgReEncodeCB, &(pImgDce->Buf[3]), len1);
                    }
                    else
                    {
                        pImgDce->Buf[0] = pImg->ImgFlagBackup;
                        pImgDce->Buf[1] = (UBYTE)((len >> 8) & 0xff);
                        pImgDce->Buf[2] = (UBYTE)(len & 0xff);
                        ReadStringFromImageCB(pImgReEncodeCB, &(pImgDce->Buf[3]), len);
                        InitImageReEncodeCB(pImgReEncodeCB);
                        pImg->ImgFlagBackup = 0;

                        if (pImgDce->Buf[0] != IMG_IN_MIDDLE)
                        {
                            InitImageCodec(pImg);
                        }
                    }
                }
            }
        }

        /*else if (pImg->Direction == DSM_FAX_RECEIVING)
        {
            if ((pImgDce->Direction == MSG_UP_LINK) && (pDsmImg->Direction == MSG_DIR_IDLE))
            {
                ImageConversionDTE(pDsmImg, pImgDce, pImg);
                memset(pImgDce, 0, sizeof(GAO_Msg));
            }
        }*/
    }
#if (SUPPORT_TX_PURE_TEXT_FILE)
    else if (pImg->ImgConversion == 2) // tx text file
    {
        if (pImg->Direction == DSM_FAX_TRANSMISSION)
        {
            if ((pDsmImg->Direction == MSG_DOWN_LINK) && (pImgDce->Direction == MSG_DIR_IDLE))
            {
                if (ValidLenImageCB(pImgReEncodeCB) == 0)
                {
                    TextConversionToImageProcess(pDsmImg, pImg);
                    memset(pDsmImg, 0, sizeof(GAO_Msg));
                }
            }

            if (pImgDce->Direction == MSG_DIR_IDLE)
            {
                len = ValidLenImageCB(pImgReEncodeCB);

                if (len > 0)
                {
                    len1 = MAX_LEN_API_MSG;
                    pImgDce->Direction = MSG_DOWN_LINK;
                    pImgDce->MsgType = MSG_TYPE_IMAGE_DATA;
                    pImgDce->MsgCode = IMG_DATA_AND_STATUS;

                    if (len > len1)
                    {
                        pImgDce->Buf[0] = IMG_IN_MIDDLE;
                        pImgDce->Buf[1] = (UBYTE)((len1 >> 8) & 0xff);
                        pImgDce->Buf[2] = (UBYTE)(len1 & 0xff);
                        ReadStringFromImageCB(pImgReEncodeCB, &(pImgDce->Buf[3]), len1);
                    }
                    else
                    {
                        pImgDce->Buf[0] = pImg->ImgFlagBackup;
                        pImgDce->Buf[1] = (UBYTE)((len >> 8) & 0xff);
                        pImgDce->Buf[2] = (UBYTE)(len & 0xff);
                        ReadStringFromImageCB(pImgReEncodeCB, &(pImgDce->Buf[3]), len);
                        InitImageReEncodeCB(pImgReEncodeCB);
                        pImg->ImgFlagBackup = 0;

                        if (pImgDce->Buf[0] != IMG_IN_MIDDLE)
                        {
                            InitImageCodec(pImg);
                        }
                    }
                }
            }
        }
    }
#endif
#endif
}

void ImageMain(TImgApiStruct *pApiImg, TImgProDteStruct *pImg)
{
    GAO_Msg *pDsmImg = pApiImg->pDsmImg;
    //GAO_Msg *pImgDce = pApiImg->pImgDce;

    ImgControl(pDsmImg, pImg);
    ImageConversionMain(pApiImg, pImg);

}


