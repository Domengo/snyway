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
#include "common.h"
#include "porting.h"
#include "gstdio.h"
#include "faxapi.h"

#if TRACEON || SUPPORT_PARSER
CONST CHAR *ResponseCode[] =
{
    // 0
    "OK",
    // 1
    "CONNECT",
    // 2
    "RING",
    // 3
    "NO CARRIER",
    // 4
    "ERROR",
    // 5
    "CONNECT 1200",
    // 6
    "NO DIALTONE",
    // 7
    "BUSY",
    // 8
    "NO ANSWER",
    // 9
    "CONNECT ",
    // 10
    "(0-7F),(0-5),(0-2),(0-2),(0-3),(0-1),(0),(0-7),(0-7F)",
    // 11
    "(0-7F),(0-D),(0-2),(0-2),(0-3),(0-1),(0),(0-7),(0-7F)",
    // 12
    "(20-7E)",
    // 13
#if SUPPORT_CT && SUPPORT_CI
    "+A8E: (0-6),(0-5),(0),(0)",
#elif SUPPORT_CT
    "+A8E: (0,1,3,4,5,6),(0-5),(0),(0)",
#elif SUPPORT_CI
    "+A8E: (0,1,2,3,5,6),(0-5),(0),(0)",
#else
    "+A8E: (0,1,3,5,6),(0-5),(0),(0)",
#endif
    // 14
    "GAO RESEARCH Modem/Fax", // MI
    // 15
    __DATE__, // MM
    // 16
    __TIME__, // MR
    // 17
    "(0-1)", // BOOL type
    // 18
    "(0-2)", // flow control
    // 19
    "(0,2400,4800,9600,19200,38400,57600)", // +IPR
    // 20
#if SUPPORT_FAX
    "(0,1,2,4,8,10,18)", // +FPR
#else
    "", // +FPR
#endif
    // 21
#if SUPPORT_T31_PARSER
    "3,24,48,72,73,74,96,97,98,121,122,145,146", // T.31 MOD
#else
    "",
#endif
#if SUPPORT_T32_PARSER
    // 22
    "(0)", // 0
    // 23
    "(0-1),(0-1),(0-1)", // +FAP
    // 24
    "(0-3)", // +FBO
    // 25
    "(0),(0)", // +FCQ
    // 26
    "(0),(0),(0),(0)", // +FFC
    // 27
    "(0-5)",
    // 28
    "(0-D)",
    // 29
    "(1-3)", // +FPS
    // 30
    "(00-FF)", // +FRY
    // 31
    "(0-1),(0-1),(0-1),(0-1)", // +FNR
#else
    // 22
    "",
    // 23
    "",
    // 24
    "",
    // 25
    "",
    // 26
    "",
    // 27
    "",
    // 28
    "",
    // 29
    "",
    // 30
    "",
    // 31
    "",
#endif
};
#endif

#if SUPPORT_PARSER
UBYTE COMM_ToUpperCase(UBYTE com)
{
    if (com >= 'a' && com <= 'z')
    {
        com -= 'a' - 'A';
    }

    return com;
}

UBYTE COMM_IsNumber(UBYTE com)
{
    if (com >= '0' && com <= '9')
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UBYTE COMM_IsHexNumber(UBYTE com, UBYTE *pOutVal)
{
    if (COMM_IsNumber(com))
    {
        *pOutVal = com - '0';
        return TRUE;
    }
    else if (com >= 'a' && com <= 'f')
    {
        *pOutVal = com - 'a' + 0xa;
        return TRUE;
    }
    else if (com >= 'A' && com <= 'F')
    {
        *pOutVal = com - 'A' + 0xA;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UBYTE COMM_AsciiToUInt32(UBYTE *pInBuf, UBYTE *pIndex, UDWORD *pOutVal, UBYTE *pIsEnd, UBYTE isHex, UDWORD defVal)
{
    UDWORD theValue = 0;
    UBYTE count = 0;
    UBYTE currByte;

    if (isHex)
    {
        UBYTE temp;

        while (COMM_IsHexNumber(currByte = pInBuf[*pIndex], &temp))
        {
            theValue *= 0x10;
            theValue += temp;

            if (++count > 8)
            {
                return FALSE;
            }

            (*pIndex)++;
        }
    }
    else
    {
        while (COMM_IsNumber(currByte = pInBuf[*pIndex]))
        {
            theValue *= 10;
            theValue += currByte - '0';

            if (++count > 10 || theValue > 0x7FFFFFFF)
            {
                return FALSE;
            }

            (*pIndex)++;
        }
    }

    switch (currByte)
    {
        case ',':
        case '-':
            (*pIndex)++;
            break;
        case 0:
            *pIsEnd = TRUE;
            (*pIndex)--;
            break;
        case ')':
            break;
        case ';':
            *pIsEnd = TRUE;
            break;
        default:
            // for something like AT&K4+FCLASS=1.0
            (*pIndex)--;
            break;
    }

    if (count == 0)
    {
        theValue = defVal;
    }

    *pOutVal = theValue;

    return TRUE;
}

UBYTE COMM_AsciiToUByte(UBYTE *pInBuf, UBYTE *pIndex, UBYTE *pOutVal, UBYTE *pIsEnd, UBYTE isHex, UBYTE defVal)
{
    UDWORD temp;

    if (COMM_AsciiToUInt32(pInBuf, pIndex, &temp, pIsEnd, isHex, (UDWORD)defVal))
    {
        if (temp <= 0xFF)
        {
            *pOutVal = (UBYTE)temp;
            return TRUE;
        }
    }

    return FALSE;
}

UBYTE COMM_StringToOctets(UBYTE *pInBuf, UBYTE *pIndex, UBYTE *pOutBuf)
{
    UWORD theValue = 0;
    UBYTE count = 0;
    UBYTE currByte, temp, tempBuf[10];

    while (COMM_IsHexNumber(currByte = pInBuf[*pIndex], &temp))
    {
        theValue *= 0x10;
        theValue += temp;
        (*pIndex)++;

        if (count++ >= sizeof(tempBuf) / sizeof(UBYTE))
        {
            return 0;
        }
        else if (!(count & 0x1))
        {
            tempBuf[(count >> 1) - 1] = (UBYTE)theValue;
            theValue = 0;
        }
    }

    if (currByte != 0 && currByte != ';' || (count & 0x1))
    {
        return 0;
    }

    memcpy(pOutBuf, tempBuf, (count >> 1) * sizeof(UBYTE));

    return (count >> 1);
}

UBYTE COMM_DecToAscii(UWORD num, UBYTE *pOutBuf)
{
    // fixing bug 18, revised the whole function
    UBYTE i, len = 0;
    UWORD decimal[] = {10000, 1000, 100, 10};

    for (i = 0; i < 4; i++)
    {
        UBYTE j = 0;

        while (num >= decimal[i])
        {
            num -= decimal[i];
            j++;
        }

        if (j > 0 || len > 0)
        {
            *pOutBuf++ = j + '0';
            len++;
        }
    }

    *pOutBuf++ = num + '0';

    return (len + 1);
}

void COMM_HexToAscii(CircBuffer *pCB, UBYTE num)
{
    UBYTE byte = (num & 0xF0) >> 4;

    if (byte >= 0xA)
    {
        PutByteToCB(pCB, byte - 0xA + 'A');
    }
    else
    {
        PutByteToCB(pCB, byte + '0');
    }

    byte = num & 0x0F;

    if (byte >= 0xA)
    {
        PutByteToCB(pCB, byte - 0xA + 'A');
    }
    else
    {
        PutByteToCB(pCB, byte + '0');
    }
}

UBYTE COMM_ParseString(UBYTE *pInBuf, UBYTE *pIndex, UBYTE *pOutBuf)
{
    UBYTE len = 0;

    while (*pIndex < 255 && len < 90)
    {
        UBYTE currByte = pInBuf[(*pIndex)++];

        len++;

        if (currByte == '"')
        {
            return TRUE;
        }

        *pOutBuf++ = currByte;
    }

    return FALSE;
}

UBYTE COMM_GetRspOrInd(CircBuffer *DteRd, UBYTE *pOutBuf, UBYTE maxLen)
{
    UWORD len;
    UBYTE currByte;
    UWORD offset;

    // search for start
    while (PeekByteFromCB(DteRd, &currByte, 0))
    {
        if (currByte != T50_CR && currByte != T50_LF)
        {
            break;
        }

        GetByteFromCB(DteRd, &currByte);
    }

    offset = 0;
    len = 0;

    // search for end
    while (PeekByteFromCB(DteRd, &currByte, offset++))
    {
        if (currByte == T50_CR)
        {
            len = offset - 1;
            break;
        }
    }

    if (len > maxLen)
    {
        len = maxLen;
        TRACE0("COMM: ERROR. Buffer overflow");
    }

    // get response or indication
    if (len > 0)
    {
        int i;

        // change to upper case
        for (i = 0; i < len; i++)
        {
            GetByteFromCB(DteRd, &currByte);
            *pOutBuf++ = COMM_ToUpperCase(currByte);
        }

        // remove the trailing <CR><LF> if any
        while (PeekByteFromCB(DteRd, &currByte, 0))
        {
            if (currByte != T50_CR && currByte != T50_LF)
            {
                break;
            }

            GetByteFromCB(DteRd, &currByte);
        }
    }

    return (UBYTE)len;
}

UBYTE COMM_ParseResultCode(UBYTE *pInBuf, UBYTE len)
{
    UBYTE ret = RESULT_CODE_NULL;
#if 1
    int i;

    for (i = RESULT_CODE_OK; i <= RESULT_CODE_NOANS; i++)
    {
        if (strcmp(pInBuf, ResponseCode[i - 1]) == 0)
        {
            ret = (UBYTE)i;
            break;
        }
    }

#else

    switch (*pInBuf++)
    {
        case 'O':

            if (len == 2 && *pInBuf++ == 'K')
            {
                ret = RESULT_CODE_OK;
            }

            break;
        case 'C':

            if (len == 7 && *pInBuf++ == 'O')
                if (*pInBuf++ == 'N')
                    if (*pInBuf++ == 'N')
                        if (*pInBuf++ == 'E')
                            if (*pInBuf++ == 'C')
                                if (*pInBuf++ == 'T')
                                {
                                    ret = RESULT_CODE_CONNECT;
                                }

            break;
        case 'R':

            if (len == 4 && *pInBuf++ == 'I')
                if (*pInBuf++ == 'N')
                    if (*pInBuf++ == 'G')
                    {
                        ret = RESULT_CODE_RING;
                    }

            break;
        case 'E':

            if (len == 5 && *pInBuf++ == 'R')
                if (*pInBuf++ == 'R')
                    if (*pInBuf++ == 'O')
                        if (*pInBuf++ == 'R')
                        {
                            ret = RESULT_CODE_ERROR;
                        }

            break;
        case 'B':

            if (len == 4 && *pInBuf++ == 'U')
                if (*pInBuf++ == 'S')
                    if (*pInBuf++ == 'Y')
                    {
                        ret = RESULT_CODE_BUSY;
                    }

            break;
        case 'N':

            if (len == 10)
            {
                if (*pInBuf++ == 'O')
                    if (*pInBuf++ == ' ')
                        if (*pInBuf++ == 'C')
                            if (*pInBuf++ == 'A')
                                if (*pInBuf++ == 'R')
                                    if (*pInBuf++ == 'R')
                                        if (*pInBuf++ == 'I')
                                            if (*pInBuf++ == 'E')
                                                if (*pInBuf++ == 'R')
                                                {
                                                    ret = RESULT_CODE_NOCARRIER;
                                                }
            }
            else if (len == 11)
            {
                if (*pInBuf++ == 'O')
                    if (*pInBuf++ == ' ')
                        if (*pInBuf++ == 'D')
                            if (*pInBuf++ == 'I')
                                if (*pInBuf++ == 'A')
                                    if (*pInBuf++ == 'L')
                                        if (*pInBuf++ == 'T')
                                            if (*pInBuf++ == 'O')
                                                if (*pInBuf++ == 'N')
                                                    if (*pInBuf++ == 'E')
                                                    {
                                                        ret = RESULT_CODE_NODIALTN;
                                                    }
            }
            else if (len == 9)
            {
                if (*pInBuf++ == 'O')
                    if (*pInBuf++ == ' ')
                        if (*pInBuf++ == 'A')
                            if (*pInBuf++ == 'N')
                                if (*pInBuf++ == 'S')
                                    if (*pInBuf++ == 'W')
                                        if (*pInBuf++ == 'E')
                                            if (*pInBuf++ == 'R')
                                            {
                                                ret = RESULT_CODE_NOANS;
                                            }
            }

            break;
    }

#endif
    return ret;
}

#if SUPPORT_T32_PARSER
void COMM_ParseSubParmOut(T30SessionSubParm *pSubParm, CircBuffer *pCBOut)
{
    COMM_HexToAscii(pCBOut, (UBYTE)(pSubParm->verticalResolutionVR & 0xFF));
    PutByteToCB(pCBOut, ',');
    COMM_HexToAscii(pCBOut, pSubParm->bitRateBR);
    PutByteToCB(pCBOut, ',');
    COMM_HexToAscii(pCBOut, pSubParm->pageWidthWD);
    PutByteToCB(pCBOut, ',');
    COMM_HexToAscii(pCBOut, pSubParm->pageLengthLN);
    PutByteToCB(pCBOut, ',');
    COMM_HexToAscii(pCBOut, pSubParm->dataFormatDF);
    PutByteToCB(pCBOut, ',');
    COMM_HexToAscii(pCBOut, pSubParm->errorCorrectionEC);
    PutByteToCB(pCBOut, ',');
    COMM_HexToAscii(pCBOut, pSubParm->fileTransferBF);
    PutByteToCB(pCBOut, ',');
    COMM_HexToAscii(pCBOut, pSubParm->scanTimeST);
    PutByteToCB(pCBOut, ',');
    COMM_HexToAscii(pCBOut, pSubParm->jpegOptionJP);
}

UBYTE COMM_ParseSubParmIn(UBYTE *pBufIn, UBYTE *pIndex, T30SessionSubParm *pSubParm)
{
    T30SessionSubParm subParms;
    UBYTE t32VR;
    UBYTE isEnd = FALSE;

    if (COMM_AsciiToUByte(pBufIn, pIndex, &t32VR, &isEnd, TRUE, VR_R8_385))
    {
        subParms.verticalResolutionVR = t32VR;

        if (COMM_AsciiToUByte(pBufIn, pIndex, &subParms.bitRateBR, &isEnd, TRUE, BR_2400))
        {
            if (subParms.bitRateBR <= BR_33600)
            {
                if (COMM_AsciiToUByte(pBufIn, pIndex, &subParms.pageWidthWD, &isEnd, TRUE, WD_0))
                {
                    if (subParms.pageWidthWD <= WD_4)
                    {
                        if (COMM_AsciiToUByte(pBufIn, pIndex, &subParms.pageLengthLN, &isEnd, TRUE, LN_A4))
                        {
                            if (subParms.pageLengthLN <= LN_UNLIMITED)
                            {
                                if (COMM_AsciiToUByte(pBufIn, pIndex, &subParms.dataFormatDF, &isEnd, TRUE, DF_MH))
                                {
                                    if (subParms.dataFormatDF <= DF_MMR)
                                    {
                                        if (COMM_AsciiToUByte(pBufIn, pIndex, &subParms.errorCorrectionEC, &isEnd, TRUE, EC_NON_ECM))
                                        {
                                            if (subParms.errorCorrectionEC <= EC_ECM)
                                            {
                                                if (COMM_AsciiToUByte(pBufIn, pIndex, &subParms.fileTransferBF, &isEnd, TRUE, BF_NON_FILE_TRANSFER))
                                                {
                                                    if (subParms.fileTransferBF <= BF_NON_FILE_TRANSFER)
                                                    {
                                                        if (COMM_AsciiToUByte(pBufIn, pIndex, &subParms.scanTimeST, &isEnd, TRUE, ST_0))
                                                        {
                                                            if (subParms.scanTimeST <= ST_7)
                                                            {
                                                                if (COMM_AsciiToUByte(pBufIn, pIndex, &subParms.jpegOptionJP, &isEnd, TRUE, JP_NON_JPEG))
                                                                {
                                                                    if (subParms.jpegOptionJP <= JP_CUSTOM_GAMUT && isEnd)
                                                                    {
                                                                        memcpy(pSubParm, &subParms, sizeof(subParms));
                                                                        return TRUE;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}
#endif

#endif
