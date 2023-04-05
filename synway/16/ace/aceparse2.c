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
#include "aceext.h"
#include "common.h"
#include "gaoapi.h"

#if SUPPORT_T32_PARSER
void Ace_SetT32Command(ACEStruct *pAce, UBYTE command)
{
    DteToT30Interface *pDteToT30 = pAce->pDteToT30;

    pDteToT30->actionCmd = pDteToT30->currentDteT30Cmd = command;

    switch (command)
    {
        case DTE_T30_NULL:
            break;
        case DTE_T30_ATD:
        case DTE_T30_ATA:
        case DTE_T30_FDT:
        case DTE_T30_FDR:
        case DTE_T30_FIP:
        case DTE_T30_FPS:
            pAce->Mandate = TRUE;
            TRACE1("DTE: Command %s", T32_Action_Command[command]);
            break;
        default:
            TRACE1("DTE: ERROR. Invalid command 0x%x", command);
            break;
    }
}

UBYTE Ace_ParseClass2Capability(UBYTE **pTable, UBYTE isFcc)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isErr = FALSE;
    T30SessionSubParm *pSubParm;

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        pSubParm = isFcc ? &pAce->pDteToT30->faxParmCapability : &pAce->pDteToT30->faxParmSettings;

        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case '?':
            {
                CircBuffer *DteRd = pAce->pCBOut;
                UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                Ace_ParseSubParmToClass2Dte(pTable, pSubParm);

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
            }
            break;
            case '=':

                if (pAce->CurrCommand[pAce->ATidx] == '?')
                {
                    pAce->ATidx++;
#if SUPPORT_V34FAX

                    if (pAce->FaxClassType == FCLASS21)
                    {
                        AceSetResponse(pTable, ACE_FCC_V34FAX);
                    }
                    else
#endif
                    {
                        AceSetResponse(pTable, ACE_FCC);
                    }
                }
                else
                {
                    if (Ace_ParseSubParmFromClass2Dte(pAce, pSubParm))
                    {
                        if (isFcc)
                        {
                            memcpy(&pAce->pDteToT30->faxParmSettings, &pAce->pDteToT30->faxParmCapability, sizeof(pAce->pDteToT30->faxParmSettings));
                        }

#if SUPPORT_V34FAX
                        pAce->pT30ToDce->initV34PrimRateMax = pAce->pDteToT30->faxParmSettings.bitRateBR + 1;
#endif
                    }
                    else
                    {
                        isErr = TRUE;
                    }
                }
        }
    }
    else
    {
        isErr = TRUE;
    }

    if (isErr)
    {
        return FAIL;
    }
    else
    {
        return SUCCESS;
    }
}

void Ace_ParseDataFromClass2Dte(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (pAce->LineState == ONLINE_FAX_DATA)
    {
        T30ToDteInterface *pT30ToDte = pAce->pT30ToDte;
        DteToT30Interface *pDteToT30 = pAce->pDteToT30;
        CircBuffer *DteWr = pAce->pCBIn;
        UBYTE ch;

        if (pDteToT30->currentDteT30Cmd == DTE_T30_FDR)
        {
            while (PeekByteFromCB(DteWr, &ch, 0))
            {
                switch (ch)
                {
                    case T50_DC1:
                        pDteToT30->dataCmd = DC1_XON;
                        break;
                    case T50_DC2:
                        pDteToT30->dataCmd = DC2_READY;
                        break;
                    case T50_DC3:
                        pDteToT30->dataCmd = DC3_XOFF;
                        break;
                    case T50_CAN:
                        pDteToT30->isDiscCmd = TRUE;
                        Ace_ChangeLineState(pAce, ONLINE_FAX_COMMAND);
                        break;
                    default:
                        // although T.32 8.3.4 specifies all other characters should be discarded, some DTEs do send AT
                        pDteToT30->isDiscCmd = TRUE;
                        Ace_ChangeLineState(pAce, ONLINE_FAX_COMMAND);
                        return;
                }

                GetByteFromCB(DteWr, &ch);
            }
        }
        else
        {
            if (pT30ToDte->dataCmd == DC3_XOFF || pDteToT30->tempDataLen > 0)
            {
                return;
            }

            while (pDteToT30->tempDataLen < sizeof(pDteToT30->tempData) - 1)
            {
                if (!GetByteFromCB(DteWr, &ch))
                {
                    break;
                }

                if (pAce->wasDle)
                {
                    pAce->wasDle = FALSE;

                    switch (ch)
                    {
                        case T50_DLE:
                            pDteToT30->tempData[pDteToT30->tempDataLen++] = T50_DLE;
                            break;
                        case T50_SUB:
                            pDteToT30->tempData[pDteToT30->tempDataLen++] = T50_DLE;
                            pDteToT30->tempData[pDteToT30->tempDataLen++] = T50_DLE;
                            break;
                        case T32_MPS:
                            pDteToT30->postPgMsg = POST_PG_MSG_MPS;
                            return;
                        case T32_EOM:
                            pDteToT30->postPgMsg = POST_PG_MSG_EOM;
                            return;
                        case T32_EOP:
                            pDteToT30->postPgMsg = POST_PG_MSG_EOP;
                            return;
                        case T50_ETX:
                            pDteToT30->isDiscCmd = TRUE;
                            Ace_ChangeLineState(pAce, ONLINE_FAX_COMMAND);
                            return;
                        default:
                            TRACE1("ACE: ERROR. Invalid transparent data commands %02x", ch);
                            break;
                    }
                }
                else
                {
                    if (ch == T50_DLE)
                    {
                        pAce->wasDle = TRUE;
                    }
                    else
                    {
                        pAce->wasDle = FALSE;
                        pDteToT30->tempData[pDteToT30->tempDataLen++] = ch;
                    }
                }
            }
        }
    }
}

void Ace_ParseDataToClass2Dte(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDteInterface *pT30ToDte = pAce->pT30ToDte;
    DteToT30Interface *pDteToT30 = pAce->pDteToT30;
    CircBuffer *DteRd = pAce->pCBOut;

    pDteToT30->dataCmd = DC1_XON; /// fixing bug 3, set DC3 if buffer is full, otherwise DC1

    if (pT30ToDte->tempDataLen > 0)
    {
        UBYTE i;

        for (i = 0; i < pT30ToDte->tempDataLen; i++)
        {
            if (GetAvailLenInCB(DteRd) < 2)
            {
                pDteToT30->dataCmd = DC3_XOFF;

                if (i > 0)
                {
                    UBYTE j;

                    for (j = 0; i < pT30ToDte->tempDataLen; i++, j++)
                    {
                        pT30ToDte->tempData[j] = pT30ToDte->tempData[i];
                    }

                    pT30ToDte->tempDataLen = j;
                }

                return;
            }
            else
            {
                UBYTE ch = pT30ToDte->tempData[i];

                if (ch == T50_DLE)
                {
                    if (pAce->wasDle)
                    {
                        pAce->wasDle = FALSE;
                        PutByteToCB(DteRd, T50_SUB);
                    }
                    else
                    {
                        pAce->wasDle = TRUE;
                        PutByteToCB(DteRd, T50_DLE);
                    }
                }
                else
                {
                    if (pAce->wasDle)
                    {
                        PutByteToCB(DteRd, T50_DLE);
                        pAce->wasDle = FALSE;
                    }

                    PutByteToCB(DteRd, ch);
                }
            }
        }

        pT30ToDte->tempDataLen = 0;
    }

    if (pAce->prevDataCmd != pT30ToDte->dataCmd)
    {
        pAce->prevDataCmd = pT30ToDte->dataCmd;

        switch (pT30ToDte->dataCmd)
        {
            case NULL_DATA_CMD:
                break;
            case DC1_XON:
                //if (pT30ToDte->faxParmStatus.errorCorrectionEC != EC_NON_ECM)
            {
                PutByteToCB(DteRd, T50_DC1);
            }
            break;;
            case DC3_XOFF:
                //if (pT30ToDte->faxParmStatus.errorCorrectionEC != EC_NON_ECM)
            {
                PutByteToCB(DteRd, T50_DC3);
            }
            break;
            case CAN_STOP:
                PutByteToCB(DteRd, T50_CAN);
                break;
            default:
                TRACE1("ACE: ERROR. Invalid data command %d", pT30ToDte->dataCmd);
                break;
        }
    }
}

void Ace_ParseRespToClass2Dte(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    T30ToDteInterface *pT30ToDte = pAce->pT30ToDte;

    if (GetAvailLenInCB(DteRd) < (DteRd->Len >> 1)) // fixing bug 6
    {
        return;
    }

    if (pT30ToDte->infoTxtRsp == INFO_TXT_FPS || pT30ToDte->infoTxtRsp == INFO_TXT_FPS_FET)
    {
        if (pAce->wasDle)
        {
            PutByteToCB(DteRd, T50_DLE);
            pAce->wasDle = FALSE;
        }

        PutByteToCB(DteRd, T50_DLE);
        PutByteToCB(DteRd, T50_ETX);

        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
        PutStringToCB(DteRd, (UBYTE *)PLUS_F, (UWORD)strlen(PLUS_F));
        PutStringToCB(DteRd, (UBYTE *)T32_Info_Txt[pT30ToDte->infoTxtRsp - 1], (UWORD)strlen(T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]));

        PutByteToCB(DteRd, pT30ToDte->tempParm[0] + '0');
        PutByteToCB(DteRd, ',');
        //fixing bug 17, use decimal just to be the same as Multitech
        //COMM_HexToAscii(DteRd, pT30ToDte->tempParm[1]);
        //COMM_HexToAscii(DteRd, pT30ToDte->tempParm[2]);
        {
            UBYTE temp[5];
            UWORD lineCount = (pT30ToDte->tempParm[1] << 8) + pT30ToDte->tempParm[2];
            UBYTE len = COMM_DecToAscii(lineCount, temp);
            PutStringToCB(DteRd, temp, len);
        }
        PutStringToCB(DteRd, ",0,0,0", 6);

        if (pT30ToDte->infoTxtRsp == INFO_TXT_FPS_FET)
        {
            PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
            PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

            PutStringToCB(DteRd, (UBYTE *)PLUS_F, (UWORD)strlen(PLUS_F));
            PutStringToCB(DteRd, (UBYTE *)T32_Info_Txt[INFO_TXT_FET - 1], (UWORD)strlen(T32_Info_Txt[INFO_TXT_FET - 1]));
            PutByteToCB(DteRd, (pT30ToDte->tempParm[3] - 1) + '0');
        }

        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

        pT30ToDte->infoTxtRsp = INFO_TXT_NULL;
    }
    else if (pT30ToDte->infoTxtRsp != INFO_TXT_NULL)
    {
        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

        PutStringToCB(DteRd, (UBYTE *)PLUS_F, (UWORD)strlen(PLUS_F));
        PutStringToCB(DteRd, (UBYTE *)T32_Info_Txt[pT30ToDte->infoTxtRsp - 1], (UWORD)strlen(T32_Info_Txt[pT30ToDte->infoTxtRsp - 1]));

        switch (pT30ToDte->infoTxtRsp)
        {
            case INFO_TXT_FCO:
                break;
            case INFO_TXT_FCS:
                Ace_ParseSubParmToClass2Dte(pTable, &pT30ToDte->faxParmStatus);
                break;
            case INFO_TXT_FIS:
            case INFO_TXT_FTC:
                Ace_ParseSubParmToClass2Dte(pTable, &pT30ToDte->remoteSettings);
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
                Ace_ParseStringToClass2Dte(pTable, pT30ToDte->tempParm);
                break;
            case INFO_TXT_FET:
                PutByteToCB(DteRd, (pT30ToDte->tempParm[0] - 1) + '0');
                break;
            case INFO_TXT_FHS:
                COMM_HexToAscii(DteRd, pT30ToDte->hangupCode);
                pAce->FaxClassType = FCLASS0;
                break;
            default:
                TRACE1("ACE: ERROR. Invalid info txt response %x", pT30ToDte->infoTxtRsp);
                break;
        }

        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

        pT30ToDte->infoTxtRsp = INFO_TXT_NULL;
    }

    if (pT30ToDte->isRmtDocToPoll)
    {
        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
        PutStringToCB(DteRd, "+FPO", 4);
        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
        pT30ToDte->isRmtDocToPoll = FALSE;
    }

    if (pT30ToDte->resultCodeRsp != RESULT_CODE_NULL)
    {
        if (pT30ToDte->tempDataLen == 0)
        {
            AceSetResponse(pTable, pT30ToDte->resultCodeRsp - 1);
            pT30ToDte->resultCodeRsp = RESULT_CODE_NULL;
        }
    }
}

void Ace_ParseStringToClass2Dte(UBYTE **pTable, UBYTE *pString)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;

    PutByteToCB(DteRd, '"');
    PutStringToCB(DteRd, pString, (UWORD)strlen(pString));
    PutByteToCB(DteRd, '"');
}

UBYTE Ace_ParseStringFromClass2Dte(UBYTE **pTable, UBYTE *pString, UBYTE len)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE temp[91];

    switch (pAce->CurrCommand[pAce->ATidx++])
    {
        case '=':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case '?':
                    AceSetResponse(pTable, ACE_CHAR_RANGE);
                    return SUCCESS;
                case '"':
                    memset(temp, 0, sizeof(temp));

                    if (COMM_ParseString((UBYTE *)pAce->CurrCommand, &pAce->ATidx, temp))
                    {
                        memcpy(pString, temp, len);
                        return SUCCESS;
                    }

                    break;
            }

            break;
        case '?':
        {
            CircBuffer *DteRd = pAce->pCBOut;
            UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

            PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
            PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

            Ace_ParseStringToClass2Dte(pTable, pString);

            PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
            PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
        }

        return SUCCESS;
    }

    return FAIL;
}

void Ace_ParseSubParmToClass2Dte(UBYTE **pTable, T30SessionSubParm *pSubParm)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;

    COMM_HexToAscii(DteRd, (UBYTE)(pSubParm->verticalResolutionVR & 0xFF));
    PutByteToCB(DteRd, ',');
    COMM_HexToAscii(DteRd, pSubParm->bitRateBR);
    PutByteToCB(DteRd, ',');
    COMM_HexToAscii(DteRd, pSubParm->pageWidthWD);
    PutByteToCB(DteRd, ',');
    COMM_HexToAscii(DteRd, pSubParm->pageLengthLN);
    PutByteToCB(DteRd, ',');
    COMM_HexToAscii(DteRd, pSubParm->dataFormatDF);
    PutByteToCB(DteRd, ',');
    COMM_HexToAscii(DteRd, pSubParm->errorCorrectionEC);
    PutByteToCB(DteRd, ',');
    COMM_HexToAscii(DteRd, pSubParm->fileTransferBF);
    PutByteToCB(DteRd, ',');
    COMM_HexToAscii(DteRd, pSubParm->scanTimeST);
    PutByteToCB(DteRd, ',');
    COMM_HexToAscii(DteRd, pSubParm->jpegOptionJP);
}

UBYTE Ace_ParseSubParmFromClass2Dte(ACEStruct *pAce, T30SessionSubParm *pSubParm)
{
    T30SessionSubParm subParms;
    UBYTE t32VR;
    UBYTE isEnd = FALSE;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &t32VR, &isEnd, TRUE, VR_R8_385))
    {
        subParms.verticalResolutionVR = t32VR;

        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &subParms.bitRateBR, &isEnd, TRUE, BR_2400))
        {
            if (subParms.bitRateBR <= BR_33600)
            {
                if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &subParms.pageWidthWD, &isEnd, TRUE, WD_0))
                {
                    if (subParms.pageWidthWD <= WD_4)
                    {
                        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &subParms.pageLengthLN, &isEnd, TRUE, LN_A4))
                        {
                            if (subParms.pageLengthLN <= LN_UNLIMITED)
                            {
                                if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &subParms.dataFormatDF, &isEnd, TRUE, DF_MH))
                                {
                                    if (subParms.dataFormatDF <= DF_MMR)
                                    {
                                        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &subParms.errorCorrectionEC, &isEnd, TRUE, EC_NON_ECM))
                                        {
                                            if (subParms.errorCorrectionEC <= EC_ECM)
                                            {
                                                if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &subParms.fileTransferBF, &isEnd, TRUE, BF_NON_FILE_TRANSFER))
                                                {
                                                    if (subParms.fileTransferBF <= BF_NON_FILE_TRANSFER)
                                                    {
                                                        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &subParms.scanTimeST, &isEnd, TRUE, ST_0))
                                                        {
                                                            if (subParms.scanTimeST <= ST_7)
                                                            {
                                                                if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &subParms.jpegOptionJP, &isEnd, TRUE, JP_NON_JPEG))
                                                                {
                                                                    if (isEnd &&
                                                                        (subParms.jpegOptionJP <= (JP_ENABLE_JPEG |
                                                                                                   JP_FULL_COLOR |
                                                                                                   JP_ENABLE_HUFFMAN |
                                                                                                   JP_12_BITS_PEL_COMP |
                                                                                                   JP_NO_SUBSAMPLING |
                                                                                                   JP_CUSTOM_ILLUMINANT |
                                                                                                   JP_CUSTOM_GAMUT)))
                                                                    {
                                                                        memcpy(pSubParm, &subParms, sizeof(subParms));
                                                                        return SUCCESS;
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

    return FAIL;
}

#endif
