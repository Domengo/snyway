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

#include "imgprodte.h"
#include "t4ext.h"
#include "errorcode.h"
#include "commmac.h"

#if SUPPORT_CONVERSION_BETWEEN_T4_T6

extern void InitImageCodec(TImgProDteStruct *pImg);

UWORD ValidLenImageCB(TImageReEncodeCB *pImgReEncodeCB)
{
    return ((pImgReEncodeCB->IndexIn - pImgReEncodeCB->IndexOut + MAX_IMAGE_LEN_AFTER_ENCODING) & (MAX_IMAGE_LEN_AFTER_ENCODING - 1));
}

UWORD IdleLenImageCB(TImageReEncodeCB *pImgReEncodeCB)
{
    return (MAX_IMAGE_LEN_AFTER_ENCODING - 1 - ((pImgReEncodeCB->IndexIn - pImgReEncodeCB->IndexOut + MAX_IMAGE_LEN_AFTER_ENCODING) & (MAX_IMAGE_LEN_AFTER_ENCODING - 1)));
}

void InitImageReEncodeCB(TImageReEncodeCB *pImgReEncodeCB)
{
    memset(pImgReEncodeCB, 0 , sizeof(TImageReEncodeCB));
}

UWORD WriteStringToImageCB(TImageReEncodeCB *pImgReEncodeCB, UBYTE *pDataIn, UWORD len)
{
    UWORD length, i;
    UWORD IndexIn = pImgReEncodeCB->IndexIn;

    length = MIN16(IdleLenImageCB(pImgReEncodeCB), len);

    for (i = 0; i < length; i++)
    {
        pImgReEncodeCB->ImgDataCB[IndexIn] = *pDataIn++;
        IndexIn = ((IndexIn + 1) & (MAX_IMAGE_LEN_AFTER_ENCODING - 1));
    }

    pImgReEncodeCB->IndexIn = IndexIn;

    return length;
}

UWORD ReadStringFromImageCB(TImageReEncodeCB *pImgReEncodeCB, UBYTE *pDataOut, UWORD len)
{
    UWORD length, i;
    UWORD IndexOut = pImgReEncodeCB->IndexOut;

    length = MIN16(ValidLenImageCB(pImgReEncodeCB), len);

    for (i = 0; i < length; i++)
    {
        *pDataOut++ = pImgReEncodeCB->ImgDataCB[IndexOut];
        IndexOut = ((IndexOut + 1) & (MAX_IMAGE_LEN_AFTER_ENCODING - 1));
    }

    pImgReEncodeCB->IndexOut = IndexOut;

    return length;
}

void unpackBitsFromBytes(UBYTE *pBytes, UBYTE *pBits, int numBytes, int isLSBfirst)
{
    UBYTE uByte;

    if (isLSBfirst)
    {
        for (; numBytes > 0; numBytes--)
        {
            uByte = *pBytes++;

            *pBits++ = uByte & 0x01;    // LSB first
            *pBits++ = uByte & 0x02;
            *pBits++ = uByte & 0x04;
            *pBits++ = uByte & 0x08;
            *pBits++ = uByte & 0x10;
            *pBits++ = uByte & 0x20;
            *pBits++ = uByte & 0x40;
            *pBits++ = uByte & 0x80;
        }
    }
    else
    {
        for (; numBytes > 0; numBytes--)
        {
            uByte = *pBytes++;

            *pBits++ = uByte & 0x80;    // MSB first
            *pBits++ = uByte & 0x40;
            *pBits++ = uByte & 0x20;
            *pBits++ = uByte & 0x10;
            *pBits++ = uByte & 0x08;
            *pBits++ = uByte & 0x04;
            *pBits++ = uByte & 0x02;
            *pBits++ = uByte & 0x01;
        }
    }
}


void ImageConversionDTE(GAO_Msg *pMsgOut, GAO_Msg *pMsgIn, TImgProDteStruct *pImg)
{
    TImageReEncodeCB *pImgReEncodeCB = &(pImg->ImgReEncodeCB);
    TImgFormat *pImgIn = &(pImg->SrcFormat);
    UBYTE bitArray[8*IMAGE_BYTES_PER_FRAME];
    UBYTE ImageOut[MAX_IMAGE_LEN_AFTER_ENCODING];
    UDWORD imageFrameBytes, iBits, iLines;
    UWORD *pTemp, Temp;
    UBYTE *pDataIn = &(pMsgIn->Buf[3]);
    UBYTE *pDataOut = ImageOut;
    SDWORD functionResult, LenOut = 0;

    memcpy(pMsgOut, pMsgIn, sizeof(GAO_Msg));
    imageFrameBytes = ((pMsgIn->Buf[1] << 8) & 0xff00) + pMsgIn->Buf[2];
    memset(&(pMsgOut->Buf[1]), 0, (sizeof(pMsgOut->Buf) - 1));
    unpackBitsFromBytes(pDataIn, bitArray, imageFrameBytes, (pImgIn->FillOrder == FILLORDER_LSB_FIRST));

    for (iBits = 0; iBits < imageFrameBytes * 8; iBits++)
    {
        switch (t4Decoder_apply(&(pImg->t4Decoder), bitArray[iBits], pImg->pDecoderLine, pImg->pDecoderReferenceLine))
        {
            case T4_DECODER_ERROR:
                pImg->errorCode = ERROR_IMG_BAD_MMR_DATA;
                break;
            case T4_DECODER_EOL_BAD_LINE:
                pImg->errorCount++;
            case T4_DECODER_EOL:
                // Scaling Processing
                iLines = SCALER_apply(&(pImg->scaler), pImg->pDecoderLine, pImg->pEncoderLine);

                // Encoding Processing
                if (iLines)
                {
                    pTemp = pImg->pEncoderReferenceLine;
                }
                else
                {
                    pTemp = pImg->pEncoderLine;    // Reference line must not change when current line is dropped
                }

                for (; iLines > 0; iLines--)
                {
                    functionResult = t4Encoder_apply(&(pImg->t4Encoder), pImg->pEncoderLine, pImg->pEncoderReferenceLine, pDataOut + LenOut);

                    if (functionResult < 0)
                    {
                        pImg->errorCode = functionResult;
                        break;
                    }
                    else
                    {
                        LenOut += functionResult;
                    }

                    pImg->pEncoderReferenceLine = pImg->pEncoderLine; // Repeated line's reference is original encoded line

                    if (LenOut > MAX_IMAGE_LEN_AFTER_ENCODING - 1)
                    {
                        pImg->errorCode = ERROR_IMG_ENCODING_ERROR;
                        TRACE0("IMG: ERROR, Img inside temporary buffer overflows 1.");
                        break;
                    }
                }

                // Finish swap of encoder line and encoder reference line
                pImg->pEncoderLine = pTemp;

                // Swap Decoder Line and Decoder Reference Line
                pTemp = pImg->pDecoderReferenceLine;
                pImg->pDecoderReferenceLine = pImg->pDecoderLine;
                pImg->pDecoderLine = pTemp;
                break;
            case T4_DECODER_EOFB_BAD:
                pImg->errorCount++;
            case T4_DECODER_RTC:
            case T4_DECODER_EOFB:
                iBits = imageFrameBytes * 8;    // break from the for loop
                LenOut += t4Encoder_end(&(pImg->t4Encoder), pDataOut + LenOut);
                break;
            default:
                break;// T4_DECODER_CONTINUE
        }

        if (pImg->errorCode)
        {
            TRACE0("IMG: T4/T6 image Error!");
            break;
        }
    }

    if (pImg->errorCode)
    {
        return;
    }

    if (LenOut > MAX_LEN_API_MSG) // need to buffer the image data after re-encoding
    {
        pImg->ImgFlagBackup = pMsgOut->Buf[0];
        pMsgOut->Buf[0] = IMG_IN_MIDDLE;
        Temp = MAX_LEN_API_MSG;

        if (IdleLenImageCB(pImgReEncodeCB) >= (LenOut - Temp))
        {
            WriteStringToImageCB(pImgReEncodeCB, &(ImageOut[Temp]), (UWORD)(LenOut - Temp));
            LenOut = Temp;
        }
        else
        {
            TRACE0("IMG: ERROR, Img Circular buffer overflows.");
            return;
        }
    }
    else
    {
        InitImageReEncodeCB(pImgReEncodeCB);
    }

    pMsgOut->Buf[1] = (UBYTE)((LenOut >> 8) & 0xff);
    pMsgOut->Buf[2] = (UBYTE)(LenOut & 0xff);
    memcpy(&(pMsgOut->Buf[3]), pDataOut, LenOut);

    if (pMsgOut->Buf[0] != IMG_IN_MIDDLE)
    {
        InitImageCodec(pImg);
    }
}

#endif


