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
#include "gaoapi.h"
#include "common.h"
#include "v251.h"

#if SUPPORT_T31_PARSER
void Ace_SetT31Command(ACEStruct *pAce, UBYTE command)
{
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;

    pT30ToDce->actionCmd = command;

    switch (command)
    {
        case T30_DCE_NULL:
            break;
        case T30_DCE_ATD:
        case T30_DCE_ATA:
        case T30_DCE_ATH:
        case T30_DCE_ATO:
        case T30_DCE_FTS:
        case T30_DCE_FRS:
        case T30_DCE_FTM:
        case T30_DCE_FRM:
        case T30_DCE_FTH:
        case T30_DCE_FRH:
            pAce->Mandate = TRUE;
            TRACE1("T30: Command %s", T31_Action_Command[command]);
            break;
        case T30_DCE_A8M:
            pAce->Mandate = TRUE;
            TRACE("T30: Command %s=", T31_Action_Command[command]);
            break;
        case T30_DCE_DLE_PRI:
            TRACE0("T30: Command <DLE><PRI>");
            break;
        case T30_DCE_DLE_CTRL:
            TRACE0("T30: Command <DLE><CTRL>");
            break;
        case T30_DCE_DLE_RTNC:
            TRACE0("T30: Command <DLE><RTNC>");
            break;
        case T30_DCE_DLE_EOT:
            TRACE0("T30: Command <DLE><EOT>");
            break;
        default:
            TRACE1("T30: ERROR. Invalid command 0x%x", command);
            break;
    }
}

void Ace_ParseDataFromClass1Dte(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;

    if (pAce->currentT30DceCmd != T30_DCE_NULL)
    {
        DceToT30Interface *pDceToT30 = pAce->pDceToT30;
        CircBuffer *DteWr = pAce->pCBIn;
        UBYTE ch;
        UBYTE isHdlc = pAce->classState != ACE_FTM;
        UBYTE t50Dle;

#if SUPPORT_V34FAX

        if (pDceToT30->isV34Selected)
        {
            if (pAce->classState == ACE_FRH &&
                DteWr->CurrLen != 0)
            {
                TRACE0("ACE: HDLC receiving cancelled");
                pT30ToDce->flowCntlCmd = CAN_STOP;
                return;
            }
        }
        else
#endif
        {
            if ((pAce->classState == ACE_FTH && pAce->hdlcLen == 0) ||
                pAce->classState == ACE_FRH ||
                pAce->classState == ACE_FRM)
            {
                if (PeekByteFromCB(DteWr, &ch, 0))
                {
                    if (ch != 0xff && ch != T50_DLE)
                    {
                        if (pAce->classState == ACE_FTH)
                        {
                            TRACE0("ACE: Transmitting cancelled");
                        }
                        else
                        {
                            TRACE0("ACE: Receiving cancelled");
                        }

                        pDceToT30->tempDataType = DCE_T30_DATA_NULL; // fixing bug 29, the data type doesn't mean anything here
                        pDceToT30->tempDataLen = 0;
                        Ace_ChangeLineState(pAce, ONLINE_FAX_COMMAND);
                        pT30ToDce->flowCntlCmd = CAN_STOP;
                        return;
                    }
                }
            }
        }

        if (pAce->LineState == ONLINE_FAX_DATA)
        {
            if (pDceToT30->flowCntlCmd == DC3_XOFF || pT30ToDce->tempDataLen > 0)
            {
                return;
            }

            t50Dle = isHdlc ? T50_DLE_BIT_REV : T50_DLE;

            while (pT30ToDce->tempDataLen < sizeof(pT30ToDce->tempData) - 1)
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
                            pT30ToDce->tempData[pT30ToDce->tempDataLen++] = t50Dle;
                            break;
                        case T50_SUB:

                            if (pAce->ClassFDD)
                            {
                                pT30ToDce->tempData[pT30ToDce->tempDataLen++] = t50Dle;
                                pT30ToDce->tempData[pT30ToDce->tempDataLen++] = t50Dle;
                            }
                            else
                            {
                                TRACE0("ACE: ERROR. Invalide <DLE><SUB>");
                            }

                            break;
                        case T50_ETX:

                            switch (pAce->classState)
                            {
                                case ACE_NOF:
                                case ACE_FTH:
                                case ACE_FTH_HS:
                                case ACE_TX1S:
                                    pT30ToDce->tempDataType = T30_DCE_HDLC_OK;
                                    break;
                                case ACE_FTM:
                                    pT30ToDce->tempDataType = T30_DCE_HS_END;
                                    break;
#if SUPPORT_V34FAX
                                case ACE_FRH:
                                    Ace_ChangeState(pAce, ACE_FTH);
                                    pT30ToDce->tempDataType = T30_DCE_HDLC_OK;
                                    break;
#endif
                                default:
                                    TRACE0("ACE: ERROR. Invalid <DLE><ETX>");
                                    break;
                            }

                            return;
#if SUPPORT_V34FAX
                        case T50_EOT:
                            Ace_SetT31Command(pAce, T30_DCE_DLE_EOT);
                            return;
                        case T31_PRI:
                            Ace_SetT31Command(pAce, T30_DCE_DLE_PRI);
                            return;
                        case T31_CTRL:
                            Ace_SetT31Command(pAce, T30_DCE_DLE_CTRL);
                            return;
                        case T31_PPH:
                            pT30ToDce->isReqRateReduction = TRUE; TRACE0("ACE: <DLE><PPH> received");
                            break;
                        case T31_RTNC:
                            Ace_SetT31Command(pAce, T30_DCE_DLE_RTNC);
                            {
                                CircBuffer *DteRd = pAce->pCBOut;

                                PutByteToCB(DteRd, T50_DLE);
                                PutByteToCB(DteRd, T31_RTNC);
                            }
                            return;
                        case T31_C12:
                            pT30ToDce->initV34CntlRate = 1;
                            break;
                        case T31_C24:
                            pT30ToDce->initV34CntlRate = 2;
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
                            pT30ToDce->initV34PrimRateMax = ch - T31_P24 + 1;
                            break;
                        case T31_DC1:
                        case T31_DC3:
                        case T31_FERR:
                        case T31_RTN:
                        case T31_MARK:
                        default:
                            TRACE1("ACE: ERROR. Unsupproted transparent data commands %02x", ch);
                            break;
#endif
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

                        if (isHdlc)
                        {
                            ch = COMM_BitReversal(ch);
                        }

                        pT30ToDce->tempData[pT30ToDce->tempDataLen++] = ch;
                    }
                }
            }
        }
    }
}

void Ace_ParseDataToClass1Dte(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    CircBuffer *DteRd = pAce->pCBOut;
    int i;

#if SUPPORT_V34FAX

    if (pAce->selectedV34Chnl != pDceToT30->selectedV34Chnl)
    {
        PutByteToCB(DteRd, T50_DLE);

        pAce->selectedV34Chnl = pDceToT30->selectedV34Chnl;

        switch (pAce->selectedV34Chnl)
        {
            case V34_CONTROL_CHNL:
                PutByteToCB(DteRd, T31_CTRL);
                PutByteToCB(DteRd, T50_DLE);
                PutByteToCB(DteRd, pDceToT30->priChnlBitRate - 1 + T31_P24);
                PutByteToCB(DteRd, T50_DLE);
                PutByteToCB(DteRd, pDceToT30->ctrlChnlBitRate - 1 + T31_C12);
                break;
            case V34_PRIMARY_CHNL:
                PutByteToCB(DteRd, T31_PRI);
                PutByteToCB(DteRd, T50_DLE);
                PutByteToCB(DteRd, pDceToT30->priChnlBitRate - 1 + T31_P24);
                break;
            default:
                TRACE1("ACE: ERROR. Invalid channel selected %d", pAce->selectedV34Chnl);
                break;
        }
    }

#endif

    if (pDceToT30->tempDataLen > 0)
    {
        UBYTE isHdlc = !(pDceToT30->tempDataType == DCE_T30_HS_DATA || pDceToT30->tempDataType == DCE_T30_HS_END);

        pT30ToDce->flowCntlCmd = DC1_XON;

        for (i = 0; i < pDceToT30->tempDataLen; i++)
        {
            if (GetAvailLenInCB(DteRd) < 2)
            {
                pT30ToDce->flowCntlCmd = DC3_XOFF;

                if (i > 0)
                {
                    UWORD j;

                    for (j = 0; i < pDceToT30->tempDataLen; i++, j++)
                    {
                        pDceToT30->tempData[j] = pDceToT30->tempData[i];
                    }

                    pDceToT30->tempDataLen = j;
                }

                return;
            }
            else
            {
                UBYTE ch = pDceToT30->tempData[i];

                if (isHdlc)
                {
                    ch = COMM_BitReversal(ch);
                }

                if (ch == T50_DLE)
                {
                    if (pAce->wasDle)
                    {
                        if (pAce->ClassFDD)
                        {
                            PutByteToCB(DteRd, T50_SUB);
                            pAce->wasDle = FALSE;
                        }
                        else
                        {
                            PutByteToCB(DteRd, T50_DLE);
                            PutByteToCB(DteRd, T50_DLE);
                            pAce->wasDle = TRUE;
                        }
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
                    }

                    PutByteToCB(DteRd, ch);
                    pAce->wasDle = FALSE;
                }
            }
        }

        pDceToT30->tempDataLen = 0;
    }

    switch (pDceToT30->tempDataType)
    {
        case DCE_T30_HDLC_BAD:
        case DCE_T30_HDLC_END:
        case DCE_T30_HS_END:

            if (GetAvailLenInCB(DteRd) > 2)
            {
                if (pAce->wasDle)
                {
                    PutByteToCB(DteRd, T50_DLE);
                    pAce->wasDle = FALSE;
                }

                PutByteToCB(DteRd, T50_DLE);
#if SUPPORT_V34FAX

                if (pDceToT30->isV34Selected)
                {
                    if (pDceToT30->tempDataType == DCE_T30_HDLC_BAD)
                    {
                        PutByteToCB(DteRd, T31_FERR);
                    }
                    else
                    {
                        PutByteToCB(DteRd, T50_ETX);
                    }
                }
                else
#endif
                {
                    PutByteToCB(DteRd, T50_ETX);
                }

                pAce->WasETX = TRUE;
                pDceToT30->tempDataType = DCE_T30_DATA_NULL;
            }

            break;
    }

#if SUPPORT_V34FAX

    if (pDceToT30->isDleEot)
    {
        pDceToT30->isDleEot = FALSE;
        PutByteToCB(DteRd, T50_DLE);
        PutByteToCB(DteRd, T50_EOT);
    }

#endif
}

void Ace_ParseRespToClass1Dte(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    int i;

    if (pDceToT30->v8IndicationType != V8_NULL)
    {
        UBYTE temp[10];
        UBYTE len;

        switch (pDceToT30->v8IndicationType)
        {
            case V8_A8A:
                PutStringToCB(DteRd, (UBYTE *)"+A8A:", 5);
                len = COMM_DecToAscii(pDceToT30->v8IndicationData[0], temp);
                PutStringToCB(DteRd, temp, len);
                break;
            case V8_A8J:
                PutStringToCB(DteRd, (UBYTE *)"+A8J:", 5);
                len = COMM_DecToAscii(pDceToT30->v8IndicationData[0], temp);
                PutStringToCB(DteRd, temp, len);
                //fixing bug 14, don't inject ATO when reporting +A8J:1
                //if (pDceToT30->v8IndicationData[0] != 0)
                //{
                //    CircBuffer *DteWr = pAce->pCBIn;

                //    PutByteToCB(DteWr, pSreg[CR_CHARACTER]);
                //    PutByteToCB(DteWr, pSreg[LF_CHARACTER]);
                //    PutStringToCB(DteWr, (UBYTE *)"ATO", 3);
                //    PutByteToCB(DteWr, pSreg[CR_CHARACTER]);
                //    PutByteToCB(DteWr, pSreg[LF_CHARACTER]);
                //}
                break;
            case V8_A8C:
                PutStringToCB(DteRd, (UBYTE *)"+A8C:", 5);
                len = COMM_DecToAscii(pDceToT30->v8IndicationData[0], temp);
                PutStringToCB(DteRd, temp, len);
                break;
            case V8_A8I:
                PutStringToCB(DteRd, (UBYTE *)"+A8I:", 5);
                len = COMM_DecToAscii(pDceToT30->v8IndicationData[0], temp);
                PutStringToCB(DteRd, temp, len);
                break;
            case V8_A8M:
                PutStringToCB(DteRd, (UBYTE *)"+A8M:", 5);

                if (pDceToT30->v8IndicationLen == 1)
                {
                    //fixing bug 16, don't inject AT+FTH or AT+FRH, the state is already set to FTH/FRH in V8_Handler()
                    //DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
                    //CircBuffer *DteWr = pAce->pCBIn;

                    //PutByteToCB(DteWr, pSreg[CR_CHARACTER]);
                    //PutByteToCB(DteWr, pSreg[LF_CHARACTER]);
                    //PutStringToCB(DteWr, (UBYTE *)"AT+F", 4);
                    //if (RdReg(pDpcs->MCR0, ANSWER))
                    //{
                    //    PutByteToCB(DteWr, 'T');
                    //}
                    //else
                    //{
                    //    PutByteToCB(DteWr, 'R');
                    //}
                    //PutStringToCB(DteWr, (UBYTE *)"H=3;", 4);
                    //PutByteToCB(DteWr, pSreg[CR_CHARACTER]);
                    //PutByteToCB(DteWr, pSreg[LF_CHARACTER]);

                    len = COMM_DecToAscii(pDceToT30->v8IndicationData[0], temp);
                    PutStringToCB(DteRd, temp, len);
                }
                else
                {
                    for (i = 0; i < pDceToT30->v8IndicationLen; i++)
                    {
                        COMM_HexToAscii(DteRd, pDceToT30->v8IndicationData[i]);
                    }
                }

                break;
            case V8_OK:
                break;
            default:
                TRACE1("ACE: ERROR. Invalid V.8 indicator %x", pDceToT30->v8IndicationType);
                break;
        }

        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        //PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

        pDceToT30->v8IndicationType = V8_NULL;
    }

    if (pDceToT30->infoTxtRsp != INFO_TXT_NULL)
    {
        UBYTE temp[4];
        UBYTE len;

        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

        switch (pDceToT30->infoTxtRsp)
        {
            case INFO_TXT_F34:
                PutStringToCB(DteRd, (UBYTE *)"+F34:", 5);
                len = COMM_DecToAscii(pDceToT30->priChnlBitRate, temp);
                PutStringToCB(DteRd, temp, len);
                PutByteToCB(DteRd, ',');
                len = COMM_DecToAscii(pDceToT30->ctrlChnlBitRate, temp);
                PutStringToCB(DteRd, temp, len);
                break;
            case INFO_TXT_FCERROR:

                if (pAce->Verbose)
                {
                    PutStringToCB(DteRd, (UBYTE *)"+FCERROR", 8);
                }
                else
                {
                    PutStringToCB(DteRd, (UBYTE *)"+F4", 3);
                }

                break;
            default:
                TRACE1("ACE: ERROR. Invalid infor txt response %x", pDceToT30->infoTxtRsp);
                break;
        }

        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

        pDceToT30->infoTxtRsp = INFO_TXT_NULL;
    }

    if (pDceToT30->resultCodeRsp != RESULT_CODE_NULL)
    {
        if (pDceToT30->tempDataType == DCE_T30_DATA_NULL && pDceToT30->tempDataLen == 0)
        {
            if (GetAvailLenInCB(DteRd) > 30)
            {
                if (!pAce->WasETX && // send <DLE><ETX> if it's not sent
                    pAce->LineState == ONLINE_FAX_DATA)
                {
                    if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER ||
                        pDceToT30->resultCodeRsp == RESULT_CODE_ERROR)
                    {
                        if (pAce->wasDle)
                        {
                            PutByteToCB(DteRd, T50_DLE);
                            pAce->wasDle = FALSE;
                        }

                        PutByteToCB(DteRd, T50_DLE);
                        PutByteToCB(DteRd, T50_ETX);
                    }
                }

                AceSetResponse(pTable, pDceToT30->resultCodeRsp - 1);
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            }
        }
    }
}

UBYTE Ace_ParseMod(UBYTE **pTable, UBYTE command)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isEnd = FALSE, isErr = FALSE;

    if (pAce->CurrCommand[pAce->ATidx++] != '=')
    {
        isErr = TRUE;
    }
    else
    {
        /* check for report modulation parameters command */
        if (pAce->CurrCommand[pAce->ATidx] == '?')
        {
            AceSetResponse(pTable, ACE_MOD_CAP);
            AceSetResponse(pTable, ACE_OK);
            return SUCCESS;
        }

        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, pAce->pT30ToDce->tempParm, &isEnd, FALSE, MOD_NULL))
        {
            if (!isEnd)
            {
                isErr = TRUE;
            }
        }
        else
        {
            isErr = TRUE;
        }
    }

    if (isErr)
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
    else
    {

        Ace_SetT31Command(pAce, command);
        return SUCCESS;
    }
}

UBYTE Ace_ParseSilence(UBYTE **pTable, UBYTE command)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isErr = FALSE, isEnd = FALSE;

    if (pAce->CurrCommand[pAce->ATidx++] != '=')
    {
        isErr = TRUE;
    }
    else
    {
        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, pAce->pT30ToDce->tempParm, &isEnd, FALSE, 0))
        {
            if (!isEnd)
            {
                isErr = TRUE;
            }
        }
        else
        {
            isErr = TRUE;
        }
    }

    if (isErr)
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
    else
    {
        Ace_SetT31Command(pAce, command);
        return SUCCESS;
    }
}

#endif
