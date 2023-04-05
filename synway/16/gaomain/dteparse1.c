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

#include <stdlib.h>
#include <string.h>
#include "porting.h"

#if SUPPORT_T30 && !SUPPORT_DCE
#include "gaostru.h"
#include "faxapi.h"
#include "common.h"
#include "v251.h"
#include "dsmdef.h"

void DTE_ParseDataToClass1Dce(GAO_LibStruct *pFax)
{
    CircBuffer *DteWr = &pFax->m_BufferOut;
    TDsmStrcut *pDsm = &pFax->m_Dsm;
    DceToT30Interface *pDceToT30 = &pFax->dceToT30;
    T30ToDceInterface *pT30ToDce = &pFax->t30ToDce;
    UBYTE currByte;
    UBYTE temp[10];
    UBYTE len;

#if SUPPORT_V34FAX

    if (*pT30ToDce->pIsV34Enabled &&
        pT30ToDce->v8OpCntl != A8E_NULL &&
        pT30ToDce->currentT30DceCmd == T30_DCE_NULL)
    {
        UBYTE v8OpCntl = pT30ToDce->v8OpCntl & A8E_V8O;

        PutStringToCB(DteWr, "AT+A8E=", 7);

        if (v8OpCntl != A8E_V8O_0)
        {
            PutByteToCB(DteWr, v8OpCntl + '0');
        }
        else
        {
            v8OpCntl = pT30ToDce->v8OpCntl & A8E_V8A;

            if (v8OpCntl != A8E_V8A_0)
            {
                PutByteToCB(DteWr, ',');
                PutByteToCB(DteWr, (v8OpCntl >> 4) + '0');
            }
        }

        PutByteToCB(DteWr, '\r');
        pT30ToDce->v8OpCntl = A8E_NULL;
    }

    if (pDceToT30->isV34Selected && pT30ToDce->isReqRateReduction)
    {
        if (GetAvailLenInCB(DteWr) >= 4)
        {
            pT30ToDce->isReqRateReduction = FALSE;
            PutByteToCB(DteWr, T50_DLE);
            PutByteToCB(DteWr, T31_P24 + pT30ToDce->initV34PrimRateMax - 1);
            PutByteToCB(DteWr, T50_DLE);
            PutByteToCB(DteWr, T31_PPH);
            TRACE0("DTE: <DLE><PPH> sent out");
        }
    }

#endif

    pDceToT30->flowCntlCmd = DC1_XON;

    if (pT30ToDce->tempDataLen > 0)
    {
        int i = 0;

        while (i < pT30ToDce->tempDataLen)
        {
            if (GetAvailLenInCB(DteWr) < 2)
            {
                int j;

                for (j = 0; i < pT30ToDce->tempDataLen; i++, j++)
                {
                    pT30ToDce->tempData[j] = pT30ToDce->tempData[i];
                }

                pT30ToDce->tempDataLen = i - j;
                pDceToT30->flowCntlCmd = DC3_XOFF;
                return;
            }
            else
            {
                currByte = pT30ToDce->tempData[i++];

                if (pDceToT30->isV34Selected || pDsm->CurrT30DCECmd == T30_DCE_FTH || pDsm->CurrT30DCECmd == T30_DCE_ATA)
                {
                    currByte = COMM_BitReversal(currByte);
                }

                if (currByte == T50_DLE)
                {
                    PutByteToCB(DteWr, T50_DLE);
                }

                PutByteToCB(DteWr, currByte);
            }
        }

        pT30ToDce->tempDataLen = 0;
    }

    switch (pT30ToDce->tempDataType)
    {
        case T30_DCE_HDLC_OK:
        case T30_DCE_HDLC_END:
        case T30_DCE_HS_END:

            if (GetAvailLenInCB(DteWr) < 2)
            {
                pDceToT30->flowCntlCmd = DC3_XOFF;
                return;
            }

            PutByteToCB(DteWr, T50_DLE);
            PutByteToCB(DteWr, T50_ETX);
#if SUPPORT_V34FAX

            if (!pDceToT30->isV34Selected)
            {
                pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
            }

#endif
            break;
        default:
            break;
    }

    pT30ToDce->tempDataType = T30_DCE_DATA_NULL;

    if (GetAvailLenInCB(DteWr) >= 3)
    {
        switch (pT30ToDce->flowCntlCmd)
        {
            case DC1_XON:
                PutByteToCB(DteWr, T50_DLE);
                PutByteToCB(DteWr, T31_DC1);
                break;
            case DC3_XOFF:
                PutByteToCB(DteWr, T50_DLE);
                PutByteToCB(DteWr, T31_DC3);
                break;
            case CAN_STOP:
                PutByteToCB(DteWr, T50_CAN);
                pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
                break;
        }

        pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
    }

    switch (pT30ToDce->actionCmd)
    {
        case T30_DCE_ATD:

            if (GetAvailLenInCB(DteWr) < 5 + pT30ToDce->tempParmLen)
            {
                return;
            }

            PutStringToCB(DteWr, T31_Action_Command[pT30ToDce->actionCmd], 3);
            PutByteToCB(DteWr, 'T');
            PutStringToCB(DteWr, pT30ToDce->tempParm, pT30ToDce->tempParmLen);
            PutByteToCB(DteWr, '\r');
            pDsm->CurrT30DCECmd = pT30ToDce->actionCmd;
            break;
        case T30_DCE_ATA:
        case T30_DCE_ATH:
        case T30_DCE_ATO:

            if (GetAvailLenInCB(DteWr) < 4)
            {
                return;
            }

            PutStringToCB(DteWr, T31_Action_Command[pT30ToDce->actionCmd], 3);
            PutByteToCB(DteWr, '\r');
            pDsm->CurrT30DCECmd = pT30ToDce->actionCmd;

            if (pDsm->OperationStage != DTE_OS_COMMAND) // fixing bug 29, when ATH needs to be sent out, we should force COMMAND mode
            {
                pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
            }

            break;
        case T30_DCE_FTS:
        case T30_DCE_FRS:
        case T30_DCE_FTM:
        case T30_DCE_FRM:
        case T30_DCE_FTH:
        case T30_DCE_FRH:
            len = COMM_DecToAscii(pT30ToDce->tempParm[0], temp);

            if (GetAvailLenInCB(DteWr) < 9 + len)
            {
                return;
            }

            PutStringToCB(DteWr, T31_Action_Command[pT30ToDce->actionCmd], 6);
            PutByteToCB(DteWr, '=');
            PutStringToCB(DteWr, temp, len);
            PutByteToCB(DteWr, '\r');
            pDsm->CurrT30DCECmd = pT30ToDce->actionCmd;
            break;
#if SUPPORT_V34FAX
        case T30_DCE_A8M:

            if (GetAvailLenInCB(DteWr) < 9 + (pT30ToDce->tempParmLen << 1))
            {
                return;
            }

            PutStringToCB(DteWr, T31_Action_Command[pT30ToDce->actionCmd], 6);
            PutByteToCB(DteWr, '=');
            {
                int i;

                for (i = 0; i < pT30ToDce->tempParmLen; i++)
                {
                    COMM_HexToAscii(DteWr, pT30ToDce->tempParm[i]);
                }
            }
            PutByteToCB(DteWr, '\r');
            pDsm->CurrT30DCECmd = pT30ToDce->actionCmd;
            break;
        case T30_DCE_DLE_PRI:
        case T30_DCE_DLE_CTRL:
        case T30_DCE_DLE_RTNC:
        case T30_DCE_DLE_EOT:

            if (GetAvailLenInCB(DteWr) < 2)
            {
                return;
            }

            PutByteToCB(DteWr, T50_DLE);

            switch (pT30ToDce->actionCmd)
            {
                case T30_DCE_DLE_PRI:
                    PutByteToCB(DteWr, T31_PRI);
                    break;
                case T30_DCE_DLE_CTRL:
                    PutByteToCB(DteWr, T31_CTRL);
                    break;
                case T30_DCE_DLE_RTNC:
                    PutByteToCB(DteWr, T31_RTNC);
                    break;
                case T30_DCE_DLE_EOT:
                    PutByteToCB(DteWr, T50_EOT);
                    break;
            }

            break;
#endif
    }

    pT30ToDce->actionCmd = T30_DCE_NULL;
}

void DTE_ParseRspOrIndFromClass1Dce(GAO_LibStruct *pFax)
{
    CircBuffer *DteRd = &pFax->m_BufferIn;
    TDsmStrcut *pDsm = &pFax->m_Dsm;
    DceToT30Interface *pDceToT30 = &pFax->dceToT30;
    UBYTE buffer[255];
    UBYTE len;

    memset(buffer, 0, sizeof(buffer));
    len = COMM_GetRspOrInd(DteRd, buffer, (UBYTE)(sizeof(buffer) / sizeof(UBYTE)));

    if (len > 0)
    {
        if (buffer[0] == 'A' && buffer[1] == 'T')
        {
            // ignore echo
        }
        else if (buffer[0] == '+')
        {
            if (buffer[1] == 'F')
            {
                if (len == 8 &&
                    buffer[2] == 'C' &&
                    buffer[3] == 'E' &&
                    buffer[4] == 'R' &&
                    buffer[5] == 'R' &&
                    buffer[6] == 'O' &&
                    buffer[7] == 'R')
                {
                    pDceToT30->infoTxtRsp = INFO_TXT_FCERROR; TRACE0("DCE: +FCERROR");
                }

#if SUPPORT_V34FAX
                else if (len >= 8 &&
                         buffer[2] == '3' &&
                         buffer[3] == '4' &&
                         buffer[4] == ':')
                {
                    UBYTE *pBuf = &buffer[5];
                    UBYTE index = 0;
                    UBYTE isEnd = FALSE;

                    if (COMM_AsciiToUByte(pBuf, &index, &pDceToT30->priChnlBitRate, &isEnd, FALSE, V34_BIT_33600))
                    {
                        if (COMM_AsciiToUByte(pBuf, &index, &pDceToT30->ctrlChnlBitRate, &isEnd, FALSE, 1))
                        {
                            pDceToT30->isV34Selected = TRUE;
                            TRACE2("DCE: +F34:%d,%d", pDceToT30->priChnlBitRate, pDceToT30->ctrlChnlBitRate);
                        }
                    }
                }

#endif
            }

#if SUPPORT_V34 && SUPPORT_V251
            else if (len >= 6 &&
                     buffer[1] == 'A' &&
                     buffer[2] == '8' &&
                     buffer[4] == ':')
            {
                UBYTE *pBuf = &buffer[5];
                UBYTE index = 0;
                UBYTE isEnd = FALSE;

                switch (buffer[3])
                {
                    case 'I':

                        if (COMM_AsciiToUByte(pBuf, &index, pDceToT30->v8IndicationData, &isEnd, FALSE, 1))
                        {
                            pDceToT30->v8IndicationType = V8_A8I; TRACE1("DCE: +A8I:%02x", pDceToT30->v8IndicationData[0]);
                        }

                        break;
                    case 'C':

                        if (COMM_AsciiToUByte(pBuf, &index, pDceToT30->v8IndicationData, &isEnd, FALSE, 1))
                        {
                            pDceToT30->v8IndicationType = V8_A8C; TRACE1("DCE: +A8C:%d", pDceToT30->v8IndicationData[0]);
                        }

                        break;
                    case 'A':

                        if (COMM_AsciiToUByte(pBuf, &index, pDceToT30->v8IndicationData, &isEnd, FALSE, 1))
                        {
                            pDceToT30->v8IndicationType = V8_A8A; TRACE1("DCE: +A8A:%d", pDceToT30->v8IndicationData[0]);
                        }

                        break;
                    case 'J':

                        if (COMM_AsciiToUByte(pBuf, &index, pDceToT30->v8IndicationData, &isEnd, FALSE, 1))
                        {
                            pDceToT30->v8IndicationType = V8_A8J; TRACE1("DCE: +A8J:%d", pDceToT30->v8IndicationData[0]);
                        }

                        break;
                    case 'M':

                        if (len == 6 && COMM_IsHexNumber(pBuf[0], pDceToT30->v8IndicationData)) // fixing issue 16, for +A8X:0
                        {
                            len = 1;
                        }
                        else
                        {
                            len = COMM_StringToOctets(pBuf, &index, pDceToT30->v8IndicationData);
                        }

                        if (len > 0)
                        {
                            pDceToT30->v8IndicationType = V8_A8M;
                            pDceToT30->v8IndicationLen = len;
                            TRACE("%s", "DCE: +A8M:");
                            {
                                int i;

                                for (i = 0; i < len; i ++)
                                {
                                    TRACE("%02x", pDceToT30->v8IndicationData[i]);
                                }
                            }
                            TRACE0("");
                        }

                        break;
                }
            }

#endif
        }
        else
        {
            UBYTE resultCode = COMM_ParseResultCode(buffer, len);

            if (resultCode != RESULT_CODE_NULL)
            {
                if (resultCode == RESULT_CODE_RING)
                {
                    // fixing bug 30, for potential bug here
                    if (pDceToT30->resultCodeRsp == RESULT_CODE_NULL)
                    {
                        pDceToT30->resultCodeRsp = resultCode; TRACE1("DCE: \"%s\"", ResponseCode[resultCode - 1]);
                    }
                    else
                    {
                        TRACE1("DCE: Ignore \"RING\" because the previous \"%s\" is not processed", ResponseCode[pDceToT30->resultCodeRsp - 1]);
                    }
                }
                else if (pDsm->CurrCnfgCmd != 0)
                {
                    pDceToT30->resultCodeRsp = resultCode; TRACE2("DCE: \"%s\" for 0x%04x", ResponseCode[resultCode - 1], pDsm->CurrCnfgCmd);
                }
                else if (pDsm->CurrT30DCECmd != T30_DCE_NULL || resultCode == RESULT_CODE_RING)
                {
                    pDceToT30->resultCodeRsp = resultCode; TRACE2("DCE: \"%s\" for %s", ResponseCode[resultCode - 1], T31_Action_Command[pDsm->CurrT30DCECmd]);

                    switch (resultCode)
                    {
                        case RESULT_CODE_OK:

                            if (pDsm->WasDleEtx)
                            {
                                pDsm->WasDleEtx = FALSE;
                                pDceToT30->tempDataType = DCE_T30_HDLC_END;
                            }

                            pDsm->CurrT30DCECmd = T30_DCE_NULL;
                            break;
                        case RESULT_CODE_ERROR:

                            if (pDsm->WasDleEtx)
                            {
                                pDsm->WasDleEtx = FALSE;
                                pDceToT30->tempDataType = DCE_T30_HDLC_BAD;
                            }

                            pDsm->CurrT30DCECmd = T30_DCE_NULL;
                            break;
                        case RESULT_CODE_NOCARRIER:
                            pDsm->CurrT30DCECmd = T30_DCE_NULL;
                            break;
                        case RESULT_CODE_CONNECT:
                            pDsm->OperationStage = DTE_OS_DATA; TRACE0("DTE: ONLINE_FAX_DATA");
                            break;
                    }
                }
            }
        }
    }
}

void DTE_ParseDataFromClass1Dce(GAO_LibStruct *pFax)
{
    CircBuffer *DteRd = &pFax->m_BufferIn;
    TDsmStrcut *pDsm = &pFax->m_Dsm;
    T30ToDceInterface *pT30ToDce = &pFax->t30ToDce;
    DceToT30Interface *pDceToT30 = &pFax->dceToT30;
    UBYTE isHdlc = pDceToT30->isV34Selected || pDsm->CurrT30DCECmd == T30_DCE_FRH || pDsm->CurrT30DCECmd == T30_DCE_ATD;
    UBYTE isStop = FALSE;
    UBYTE currByte, t50Dle = isHdlc ? T50_DLE_BIT_REV : T50_DLE;

    while ((pDceToT30->tempDataLen < sizeof(pDceToT30->tempData) - 1) && !isStop)
    {
        if (!GetByteFromCB(DteRd, &currByte))
        {
            break;
        }

        if (pDsm->PrevByte == T50_DLE)
        {
            pDsm->PrevByte = 0x00;

            switch (currByte)
            {
                case T50_DLE:
                    pDceToT30->tempData[pDceToT30->tempDataLen++] = t50Dle;
                    break;
                case T50_SUB:
                    TRACE0("DTE: ERROR. Invalide <DLE><SUB>");
                    break;
                case T50_ETX:
                    TRACE0("DCE: <DLE><ETX>");
#if SUPPORT_V34FAX

                    if (pDceToT30->isV34Selected)
                    {
                        pDceToT30->tempDataType = DCE_T30_HDLC_END;
                    }
                    else
#endif
                    {
                        // don't set HDLC_END until OK or ERROR
                        pDsm->WasDleEtx = TRUE;
                        pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
                    }

                    return;
#if SUPPORT_V34FAX
                case T50_EOT:
                    TRACE0("DCE: <DLE><EOT>");
                    pDceToT30->isDleEot = TRUE;
                    pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
                    isStop = TRUE;
                    break;
                case T31_PRI:
                    TRACE0("DCE: <DLE><PRI>");
                    pDceToT30->selectedV34Chnl = V34_PRIMARY_CHNL;
                    isStop = TRUE;
                    break;
                case T31_CTRL:
                    TRACE0("DCE: <DLE><CTRL>");
                    pDceToT30->selectedV34Chnl = V34_CONTROL_CHNL;
                    isStop = TRUE;
                    break;
                case T31_RTNC:
                    TRACE0("DCE: <DLE><RTNC>");
                    break;
                case T31_C12:
                    TRACE0("DCE: <DLE><C12>");
                    pDceToT30->ctrlChnlBitRate = 1;
                    break;
                case T31_C24:
                    TRACE0("DCE: <DLE><C24>");
                    pDceToT30->ctrlChnlBitRate = 2;
                    break;
                case T31_P24:
                case T31_P48:
                case T31_P72:
                case T31_P96:
                case T31_P120:
                case T31_P144:
                case T31_P168:
                case T31_P192:
                case T31_P216:
                case T31_P240:
                case T31_P264:
                case T31_P288:
                case T31_P312:
                case T31_P336:
                    pDceToT30->priChnlBitRate = currByte - T31_P24 + 1;
                    TRACE1("DCE: <DLE><P%d>", pDceToT30->priChnlBitRate * 24);
                    break;
                case T31_DC1:

                    if (pT30ToDce->tempDataType == T30_DCE_DATA_NULL)
                    {
                        pDceToT30->tempData[pDceToT30->tempDataLen++] = T50_DC1_BIT_REV;
                    }
                    else
                    {
                        TRACE0("DCE: <DLE><DC1>");
                        pDceToT30->flowCntlCmd = DC1_XON;
                    }

                    break;
                case T31_DC3:

                    if (pT30ToDce->tempDataType == T30_DCE_DATA_NULL)
                    {
                        pDceToT30->tempData[pDceToT30->tempDataLen++] = T50_DC3_BIT_REV;
                    }
                    else
                    {
                        TRACE0("DCE: <DLE><DC3>");
                        pDceToT30->flowCntlCmd = DC3_XOFF;
                    }

                    break;
                case T31_FERR:
                    TRACE0("DCE: <DLE><FERR>");
                    pDceToT30->tempDataType = DCE_T30_HDLC_BAD;
                    break;
                case T31_RTN:
                case T31_MARK:
#endif
                default:
                    TRACE1("DTE: ERROR. Unsupproted transparent data commands %02x", currByte);
                    break;
            }
        }
        else
        {
            pDsm->PrevByte = currByte;

            if (currByte != T50_DLE)
            {
                if (isHdlc)
                {
                    currByte = COMM_BitReversal(currByte);
                }

                pDceToT30->tempData[pDceToT30->tempDataLen++] = currByte;
            }
        }
    }

    if (pDceToT30->tempDataLen > 0)
    {
        pDceToT30->tempDataType = DCE_T30_HDLC_DATA;
    }
}

#endif
