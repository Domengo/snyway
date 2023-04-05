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
#if SUPPORT_DTE
#include "faxapi.h"
#include "common.h"
#include "gaostru.h"
#include "dsmdef.h"

#if SUPPORT_PARSER
void DTE_ParseRspOrIndFromDce(GAO_LibStruct *pFax)
{
    TDsmStrcut *pDsm = &(pFax->m_Dsm);
    CircBuffer *DteRd = &pFax->m_BufferIn;
    T30ToDteInterface *pT30ToDte = &pFax->t30ToDte;
    DteToT30Interface *pDteToT30 = &pFax->dteToT30;
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

#if SUPPORT_T32_PARSER
        else if (buffer[0] == '+' && buffer[1] == 'F')
        {
            UBYTE index = 2;
            int i;

            for (i = INFO_TXT_FCO; i <= INFO_TXT_FHR; i++)
            {
                if (strncmp(&buffer[2], T32_Info_Txt[i - 1], strlen(T32_Info_Txt[i - 1])) == 0)
                {
                    index += strlen(T32_Info_Txt[i - 1]);

                    switch (i)
                    {
                        case INFO_TXT_FCO:

                            if (len == strlen(T32_Info_Txt[i - 1]) + 2)
                            {
                                pT30ToDte->infoTxtRsp = INFO_TXT_FCO;
                                TRACE1("DCE: +F%s", T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]);
                            }

                            break;
                        case INFO_TXT_FCS:

                            if (COMM_ParseSubParmIn(buffer, &index, &pT30ToDte->faxParmStatus))
                            {
                                T30SessionSubParm *pStatus = &pT30ToDte->faxParmStatus;

                                pT30ToDte->infoTxtRsp = INFO_TXT_FCS;
                                TRACE("DCE: +F%s", T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]);
                                TRACE("%04x,", pStatus->verticalResolutionVR);
                                TRACE("%x,", pStatus->bitRateBR);
                                TRACE("%x,", pStatus->pageWidthWD);
                                TRACE("%x,", pStatus->pageLengthLN);
                                TRACE("%x,", pStatus->dataFormatDF);
                                TRACE("%x,", pStatus->errorCorrectionEC);
                                TRACE("%02x,", pStatus->fileTransferBF);
                                TRACE("%x,", pStatus->scanTimeST);
                                TRACE1("%02x", pStatus->jpegOptionJP);
                            }

                            break;
                        case INFO_TXT_FIS:
                        case INFO_TXT_FTC:

                            if (COMM_ParseSubParmIn(buffer, &index, &pT30ToDte->remoteSettings))
                            {
                                T30SessionSubParm *pRemote = &pT30ToDte->remoteSettings;

                                pT30ToDte->infoTxtRsp = i;
                                TRACE("DCE: +F%s", T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]);
                                TRACE("%04x,", pRemote->verticalResolutionVR);
                                TRACE("%x,", pRemote->bitRateBR);
                                TRACE("%x,", pRemote->pageWidthWD);
                                TRACE("%x,", pRemote->pageLengthLN);
                                TRACE("%x,", pRemote->dataFormatDF);
                                TRACE("%x,", pRemote->errorCorrectionEC);
                                TRACE("%02x,", pRemote->fileTransferBF);
                                TRACE("%x,", pRemote->scanTimeST);
                                TRACE1("%02x", pRemote->jpegOptionJP);
                            }

                            break;
                        case INFO_TXT_FTI:
                        case INFO_TXT_FCI:
                        case INFO_TXT_FPI:
                        case INFO_TXT_FNF:
                        case INFO_TXT_FNS:
                        case INFO_TXT_FNC:
                        case INFO_TXT_FSA:
                        case INFO_TXT_FPA:
                        case INFO_TXT_FPW:
                        {
                            UBYTE temp[NUMERIC_DIGIT_LENGTH];
                            memset(temp, 0, sizeof(temp));
                            index++; // skip the open '"'

                            if (COMM_ParseString(buffer, &index, temp))
                            {
                                memcpy(pT30ToDte->tempParm, temp, NUMERIC_DIGIT_LENGTH);
                                pT30ToDte->infoTxtRsp = i;
                                TRACE("DCE: +F%s", T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]);
                                TRACE1("\"%s\"", pT30ToDte->tempParm);
                            }
                        }
                        break;
                        case INFO_TXT_FPS:
                        {
                            UBYTE ppr = POST_PG_RSP_MCF;
                            UDWORD lc = 0, blc = 0, cblc = 0, lbc = 0;
                            UBYTE isEnd = FALSE, isOK = FALSE;

                            if (COMM_AsciiToUByte(buffer, &index, &ppr, &isEnd, TRUE, ppr))
                            {
                                if (!isEnd)
                                {
                                    // fixing bug 17, use decimal just to be the same as Multitech
                                    if (COMM_AsciiToUInt32(buffer, &index, &lc, &isEnd, FALSE, lc))
                                    {
                                        if (isEnd)
                                        {
                                            isOK = TRUE;
                                        }
                                        else if (COMM_AsciiToUInt32(buffer, &index, &blc, &isEnd, FALSE, blc))
                                        {
                                            if (isEnd)
                                            {
                                                isOK = TRUE;
                                            }
                                            else if (COMM_AsciiToUInt32(buffer, &index, &cblc, &isEnd, FALSE, cblc))
                                            {
                                                if (isEnd)
                                                {
                                                    isOK = TRUE;
                                                }
                                                else if (COMM_AsciiToUInt32(buffer, &index, &lbc, &isEnd, FALSE, lbc))
                                                {
                                                    if (isEnd)
                                                    {
                                                        isOK = TRUE;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (isOK)
                            {
                                pT30ToDte->tempParm[0] = ppr;
                                pT30ToDte->tempParm[1] = (UBYTE)((lc >> 8) & 0xFF);
                                pT30ToDte->tempParm[2] = (UBYTE)(lc & 0xFF);
                                pT30ToDte->tempParm[3] = (UBYTE)((blc >> 8) & 0xFF);
                                pT30ToDte->tempParm[4] = (UBYTE)(blc & 0xFF);
                                pT30ToDte->infoTxtRsp = INFO_TXT_FPS;
                                TRACE("DCE: +F%s", T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]);
                                TRACE3("%x,%x,%x", ppr, lc, blc);
                            }
                        }
                        break;
                        case INFO_TXT_FET:
                        {
                            UBYTE fet;
                            UBYTE isEnd = FALSE;

                            if (COMM_AsciiToUByte(buffer, &index, &fet, &isEnd, FALSE, POST_PG_MSG_MPS))
                            {
                                if (isEnd)
                                {
                                    pT30ToDte->tempParm[0] = fet + 1;
                                    pT30ToDte->infoTxtRsp = INFO_TXT_FET;
                                    TRACE("DCE: +F%s", T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]);
                                    TRACE1("%x", fet);
                                }
                            }
                        }
                        break;
                        case INFO_TXT_FHS:
                        {
                            UBYTE fhs;
                            UBYTE isEnd = FALSE;

                            if (COMM_AsciiToUByte(buffer, &index, &fhs, &isEnd, TRUE, HANGUP_NORMAL))
                            {
                                if (isEnd)
                                {
                                    pT30ToDte->hangupCode = fhs;
                                    pT30ToDte->infoTxtRsp = INFO_TXT_FHS;
                                    TRACE("DCE: +F%s", T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]);
                                    TRACE1("%x", fhs);
                                }
                            }
                        }
                        break;
                        case INFO_TXT_FPO:
                            pT30ToDte->infoTxtRsp = INFO_TXT_FPO;
                            TRACE1("DCE: +F%s", T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]);
                            break;
                        case INFO_TXT_FHT:
                        case INFO_TXT_FHR:
                            pT30ToDte->tempParmLen = COMM_StringToOctets(buffer, &index, pT30ToDte->tempParm);

                            if (i == INFO_TXT_FHT)
                            {
                                pT30ToDte->isHdlcXmitted = TRUE;
                            }
                            else
                            {
                                pT30ToDte->isHdlcRcved = TRUE;
                            }

                            break;
                    }

                    break;
                }
            }
        }

#endif
        else
        {
            UBYTE resultCode = COMM_ParseResultCode(buffer, len);

            if (resultCode == RESULT_CODE_RING)
            {
                // fixing bug 30
                if (pT30ToDte->resultCodeRsp == RESULT_CODE_NULL)
                {
                    pT30ToDte->resultCodeRsp = resultCode; TRACE1("DCE: \"%s\"", ResponseCode[resultCode - 1]);
                }
                else
                {
                    TRACE1("DCE: Ignore \"RING\" because the previous \"%s\" is not processed", ResponseCode[pT30ToDte->resultCodeRsp - 1]);
                }
            }
            else if (resultCode != RESULT_CODE_NULL)
            {
                if (pDteToT30->currentDteT30Cmd != DTE_T30_NULL)
                {
                    pT30ToDte->resultCodeRsp = resultCode; TRACE2("DCE: \"%s\" for %s", ResponseCode[resultCode - 1], T32_Action_Command[pDteToT30->currentDteT30Cmd]);

                    if (resultCode == RESULT_CODE_CONNECT)
                    {
                        pDsm->OperationStage = DTE_OS_DATA; TRACE0("DTE: ONLINE_FAX_DATA");

                        if (pDteToT30->currentDteT30Cmd == DTE_T30_FDR)
                        {
                            PutByteToCB(&pFax->m_BufferOut, T50_DC2);
                            pDsm->PrevByte = 0x00;
                        }
                    }
                    else
                    {
                        pDteToT30->currentDteT30Cmd = DTE_T30_NULL;
                    }
                }
                else if (pDsm->CurrCnfgCmd != 0)
                {
                    pT30ToDte->resultCodeRsp = resultCode; TRACE2("DCE: \"%s\" for 0x%04x", ResponseCode[resultCode - 1], pDsm->CurrCnfgCmd);

                    if (resultCode == RESULT_CODE_OK)
                    {
                        switch (pDsm->CurrCnfgCmd)
                        {
                            case CFG_SERVICE_CLASSES:
                                pDsm->ServiceClass = pDsm->CurrCnfgParm;
                                break;
                            case CFG_WRITE_FCC:
                                pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
                                break;
                        }
                    }
                }
            }
            else if (pDsm->CurrCnfgCmd != 0)
            {
                UBYTE respLen = MIN16(len, sizeof(pT30ToDte->tempParm));
                memcpy(pT30ToDte->tempParm, buffer, respLen);
                pT30ToDte->tempParmLen = respLen;
            }
        }
    }
}
#endif

#if SUPPORT_T32_PARSER
void DTE_ParseDataFromClass2Dce(GAO_LibStruct *pFax)
{
    TDsmStrcut *pDsm = &pFax->m_Dsm;
    CircBuffer *DteRd = &pFax->m_BufferIn;
    T30ToDteInterface *pT30ToDte = &pFax->t30ToDte;
    DteToT30Interface *pDteToT30 = &pFax->dteToT30;
    UBYTE currByte;

    while (pT30ToDte->tempDataLen < sizeof(pT30ToDte->tempData) - 2)
    {
        if (!GetByteFromCB(DteRd, &currByte))
        {
            break;
        }

        if (pDteToT30->currentDteT30Cmd == DTE_T30_FDT)
        {
            if (currByte == T50_DC3)
            {
                pT30ToDte->dataCmd = DC3_XOFF;
            }
            else if (currByte == T50_DC1)
            {
                pT30ToDte->dataCmd = DC1_XON;
            }
        }
        else if (pDsm->PrevByte == T50_DLE)
        {
            pDsm->PrevByte = 0x00;

            switch (currByte)
            {
                case T50_DLE:
                    pT30ToDte->tempData[pT30ToDte->tempDataLen++] = T50_DLE;
                    break;
                case T50_SUB:
                    pT30ToDte->tempData[pT30ToDte->tempDataLen++] = T50_DLE;
                    pT30ToDte->tempData[pT30ToDte->tempDataLen++] = T50_DLE;
                    break;
                case T50_ETX:
                    TRACE0("DCE: <DLE><ETX>");
                    pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
                    return;
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
                pT30ToDte->tempData[pT30ToDte->tempDataLen++] = currByte;
            }
        }
    }
}
#endif

#endif
