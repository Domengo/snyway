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
#include "t30ext.h"
#include "common.h"

#if SUPPORT_T30
void T30_SetResponse(T30Struct *pT30, UBYTE response)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;

    if (pDteToT30->isDiscCmd)
    {
        // fixing issue 180
        if (pT30->pfT30Vec == T30_PhaseHangUp)
        {
            pDteToT30->isDiscCmd = FALSE;
            TRACE1("T30: \"%s\" for AT+FKS", ResponseCode[response - 1]);
            pT30ToDte->resultCodeRsp = response;
        }
    }
    else if (pDteToT30->currentDteT30Cmd != DTE_T30_NULL)
    {
        TRACE2("T30: \"%s\" for %s", ResponseCode[response - 1], T32_Action_Command[pDteToT30->currentDteT30Cmd]);
        pT30ToDte->resultCodeRsp = response;
    }
    else if (response == RESULT_CODE_RING)
    {
        TRACE0("T30: \"RING\"");
        pT30ToDte->resultCodeRsp = response;
    }

    if (response == RESULT_CODE_OK || response == RESULT_CODE_ERROR)
    {
        pDteToT30->actionCmd = pDteToT30->currentDteT30Cmd = DTE_T30_NULL; // fixing issue 165, clear the current command and wait for new command
    }
}

void T30_SetCommand(T30ToDceInterface *pT30ToDce, UBYTE command)
{
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
            pT30ToDce->currentT30DceCmd = command; TRACE1("T30: Command %s", T31_Action_Command[command]);
            break;
        case T30_DCE_A8M:
            pT30ToDce->currentT30DceCmd = command; TRACE("T30: Command %s=", T31_Action_Command[command]);
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

void T30_UploadPgEndToDte_ECM(T30Struct *pT30)
{
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    T4ECMstruc *pT4 = &pT30->T4ECM;

    if (pT30->LineCount == 0)
    {
        pT30->LineCount = 1144; // LLL debug
    }

    pT30ToDte->infoTxtRsp = INFO_TXT_FPS_FET;
    pT30ToDte->tempParm[0] = POST_PG_RSP_MCF;
    pT30ToDte->tempParm[1] = (pT30->LineCount >> 8) & 0xff;
    pT30ToDte->tempParm[2] = pT30->LineCount & 0xff;
    TRACE2("T30: +FPS:%x,%d,0,0,0", pT30ToDte->tempParm[0], pT30->LineCount);

    switch (pT4->FCF2)
    {
        case FCF2_MPS:
            pT30ToDte->tempParm[3] = POST_PG_MSG_MPS;
            break;
        case FCF2_EOP:
            pT30ToDte->tempParm[3] = POST_PG_MSG_EOP;
            break;
        case FCF2_EOM:
            pT30ToDte->tempParm[3] = POST_PG_MSG_EOM;
            break;
    }

    TRACE1("T30: +FET:%d", pT30ToDte->tempParm[3] - 1);
    T30_SetResponse(pT30, RESULT_CODE_OK);

    pT30->LineCount = 0;

    if (pT30ToDte->faxParmStatus.dataFormatDF == DF_MMR)
    {
        FXL_T6Init(&(pT30->T6LineStru), ImageWidth_Tiff[pT30ToDte->faxParmStatus.pageWidthWD][pT30->pgVRforWD]);
    }

    if (pT4->FCF2 == FCF2_EOM)
    {
        pT30->wasEOM = TRUE;
    }

    pT4->isPgSentToDte = TRUE;
}

void T30_AssumeEopUponInvCmd(T30Struct *pT30)
{
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;

    if (pT30ToDte->faxParmStatus.errorCorrectionEC == EC_NON_ECM)
    {
        pT30ToDte->tempParm[0] = POST_PG_MSG_EOP;
        pT30ToDte->infoTxtRsp = INFO_TXT_FET;
        TRACE0("T30: +FET:2");
        T30_SetResponse(pT30, RESULT_CODE_OK);
    }
    else if (!pT30->T4ECM.isPgSentToDte)
    {
        pT30->T4ECM.FCF2 = FCF2_EOP;
        T30_UploadPgEndToDte_ECM(pT30);
    }
}

void T30_UploadImgToDte(T30Struct *pT30, UBYTE *pSrc, UWORD len, UBYTE isBitReversed)
{
    int i;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;

    if (pT30ToDte->faxParmStatus.jpegOptionJP == JP_NON_JPEG)
    {
        UBYTE needsToBitReverse = RdReg(pT30->pDteToT30->dataBitOrder, BIT0) != 0;
        UWORD actualLen = 0;

        pT30ToDte->tempData[0] = 0x00;
        pT30ToDte->tempData[1] = 0x00;

        for (i = 0; i < len; i++)
        {
            UBYTE currByte = isBitReversed ? COMM_BitReversal(pSrc[i]) : pSrc[i];

            if (pT30->ImageState == IMAGE_STATE_RTC_EOB)
            {
                break;
            }

            FXL_CountFaxLines(pT30, &currByte);

            // fixing issue 144, temporarily
            if (pT30ToDte->faxParmStatus.errorCorrectionEC == EC_NON_ECM && pT30->LineCount == 0)
            {
                actualLen = 2;
            }
            else
            {
                // fixing issue 142, moved the counting to this loop. Byte-by-byte counting.
                if (pT30ToDte->faxParmStatus.dataFormatDF == DF_MMR)
                {
                    FXL_CountFaxLinesT6(&(pT30->T6LineStru), &currByte, 1);
                }
                else if (pT30->T4ECM.FCF1 == T4_EOR && pT30->LineCount == 0)
                {
                    // Receive some bad frames, get rid of the leading garbage data.
                    actualLen = 2;
                    continue;
                }

                pT30ToDte->tempData[actualLen++] = needsToBitReverse ? COMM_BitReversal(currByte) : currByte;
            }
        }

        pT30ToDte->tempDataLen = (UBYTE)actualLen;
    }
    else
    {
        UBYTE needsToBitReverse = (RdReg(pT30->pDteToT30->dataBitOrder, BIT0) != 0) ^ isBitReversed;

        if (needsToBitReverse)
        {
            for (i = 0; i < len; i++)
            {
                pT30ToDte->tempData[i] = COMM_BitReversal(pSrc[i]);
            }
        }
        else
        {
            memcpy(pT30ToDte->tempData, pSrc, len);
        }

        pT30ToDte->tempDataLen = (UBYTE)len;
    }
}
#endif
