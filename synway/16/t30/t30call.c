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
/*
**    the function for T30 calling phase A
**    it starts to receive v.21 HDLC data
**    it will change the state to T30_PhaseB_SendDCS when it receives DIS
**    frame in normal case. It will deal the cases
**    with different errors, such as DCN frame is received
**    3 second timer out
*/
void T30_PhaseA_Call(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    UBYTE hdlcRxStatus;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_B_XMIT); return;
    }

#endif

    if (pT30->Timer_T1_Counter > TIMER_T1_LENGTH)
    {
        pT30->Timer_T1_Counter = 0; TRACE0("T30: T1 elapsed");
        T30_GotoPointB(pT30, HANGUP_NO_ANSWER_T1_TIMEOUT); return;
    }
    else
    {
        hdlcRxStatus = T30_GetHDLCfromRemote(pT30, TRUE);

        switch (hdlcRxStatus)
        {
            case HDLC_OPTFRM_RXED:
                break;
            case HDLC_LASTFRM_RXED:
            {
                T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;

                pT30->Timer_T1_Counter = 0; TRACE0("T30: T1 reset");

                switch (pT30->hdlcTypeRx)
                {
                    case HDLC_TYPE_NSF:
                    case HDLC_TYPE_CSI:
                    case HDLC_TYPE_DIS:
                    case HDLC_TYPE_DTC:

                        switch (pT30->hdlcTypeRx)
                        {
                            case HDLC_TYPE_DIS:
                                T30_AnalysisDIS_DTC(pT30, TRUE);
                                pT30->wasDisRcved = TRUE;
                                break;
                            case HDLC_TYPE_DTC:
                                T30_AnalysisDIS_DTC(pT30, FALSE);
                                break;
                            case HDLC_TYPE_NSF:
                            case HDLC_TYPE_CSI:
                                T30_ProcessOptionalFrame(pT30, pT30->hdlcTypeRx);
                                break;
                        }

                        if (pT30->isRemoteComptRec)
                        {
                            if (pDteToT30->currentDteT30Cmd == DTE_T30_ATD)
                            {
                                T30_SetResponse(pT30, RESULT_CODE_OK);
                            }

                            pT30->pfT30Vec = T30_PhaseB_SendDCS;
#if SUPPORT_V34FAX
                            pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                            pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                        }
                        else if (pT30ToDte->isRmtDocToPoll && pDteToT30->isRequestingToPoll)
                        {
                            // COMPT REMOTE XMTR
                            pT30->pfT30Vec = T30_PhaseA_Ans;
#if SUPPORT_V34FAX
                            pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                            pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                        }
                        else
                        {
                            T30_GotoPointC(pT30, HANGUP_REMOTE_CANNOT_RCV_OR_SEND);
                        }

                        break;
                    default:
                        TRACE1("T30: ERROR. Invalid command 0x%x", pT30->hdlcTypeRx);
                        T30_GotoPointC(pT30, HANGUP_INVALID_CMD_PHASE_B_XMIT);
                        break;
                }
            }
            T30_StopHDLCReceiving(pT30);
            break;
        }
    }
}

/*
**    the function send TSI and DCS frame to remote
**    and then change the state into T30_PhaseB_TCF_Tx
*/
void T30_PhaseB_SendDCS(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    UBYTE re;
    int i;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pT30->pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_B_XMIT); return;
    }

#endif

    if (pDteToT30->actionCmd == DTE_T30_FDR)
    {
        // FDR starts polling
        pT30->pfT30Vec = T30_PhaseA_Ans;
#if SUPPORT_V34FAX
        pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
        pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
    }
    else if (pDteToT30->actionCmd != DTE_T30_FDT)
    {
        return;
    }

    switch (pT30->txStage)
    {
        case LS_TX_STAGE_SILENCE:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_PREAMBLE;
            }

            break;
        case LS_TX_STAGE_PREAMBLE:
            T30_SetCommand(pT30ToDce, T30_DCE_FTH);
            pT30ToDce->tempParm[0] = MOD_V21_300;
            pT30->txStage = LS_TX_STAGE_DATA;
            break;
        case LS_TX_STAGE_DATA:

            if ((pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
#if SUPPORT_V34FAX
                || isV34Selected
#endif
               )
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_CRC_OK;
            }

            break;
        case LS_TX_STAGE_CRC_OK:

            switch (pT30->frmStage)
            {
                case LS_FRAME_OPT1:

                    if ((pDteToT30->passWord[0] != 0x00) && RdReg(pT30->remoteFAPCap, BIT0))
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_SID;
                        T30_CommandTx(pT30, pT30->hdlcTypeTx);

                        for (i = 0; i < T30FRAMELEN; i++)
                        {
                            pT30->HDLC_TX_Buf[i] = 0;
                        }

                        pT30->isHdlcMade = FALSE;
#if SUPPORT_V34FAX

                        if (!isV34Selected)
#endif
                        {
                            pT30->txStage = LS_TX_STAGE_DATA;
                        }
                    }
                    else
                    {
                        ResetReg(pT30->remoteFAPCap, BIT0);
                    }

                    pT30->frmStage++;
                    break;
                case LS_FRAME_OPT2:

                    if ((pDteToT30->destSubAddr[0] != 0x00) && RdReg(pT30->remoteFAPCap, BIT2))
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_SUB;
                        T30_CommandTx(pT30, pT30->hdlcTypeTx);

                        for (i = 0; i < T30FRAMELEN; i++)
                        {
                            pT30->HDLC_TX_Buf[i] = 0;
                        }

                        pT30->isHdlcMade = FALSE;
#if SUPPORT_V34FAX

                        if (!isV34Selected)
#endif
                        {
                            pT30->txStage = LS_TX_STAGE_DATA;
                        }
                    }
                    else
                    {
                        ResetReg(pT30->remoteFAPCap, BIT2);
                    }

                    pT30->frmStage++;
                    break;
                case LS_FRAME_OPT3:

                    if (pDteToT30->localID[0] != NUL)
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_TSI;
                        T30_CommandTx(pT30, pT30->hdlcTypeTx);

                        for (i = 0; i < T30FRAMELEN; i++)
                        {
                            pT30->HDLC_TX_Buf[i] = 0;
                        }

                        pT30->isHdlcMade = FALSE;
#if SUPPORT_V34FAX

                        if (!isV34Selected)
#endif
                        {
                            pT30->txStage = LS_TX_STAGE_DATA;
                        }
                    }

                    pT30->frmStage++;
                    break;
                default:
                    pT30->hdlcTypeTx = HDLC_TYPE_DCS;
                    re = T30_CommandTx(pT30, pT30->hdlcTypeTx);

                    if (re == 1)
                    {
                        T30_StopHDLCSending(pT30);
                        pT30ToDte->infoTxtRsp = INFO_TXT_FCS; TRACE("%s", "T30: +FCS:");
                        TRACE("%04x,", pT30ToDte->faxParmStatus.verticalResolutionVR);
                        TRACE("%x,", pT30ToDte->faxParmStatus.bitRateBR);
                        TRACE("%x,", pT30ToDte->faxParmStatus.pageWidthWD);
                        TRACE("%x,", pT30ToDte->faxParmStatus.pageLengthLN);
                        TRACE("%x,", pT30ToDte->faxParmStatus.dataFormatDF);
                        TRACE("%x,", pT30ToDte->faxParmStatus.errorCorrectionEC);
                        TRACE("%02x,", pT30ToDte->faxParmStatus.fileTransferBF);
                        TRACE("%x,", pT30ToDte->faxParmStatus.scanTimeST);
                        TRACE1("%02x", pT30ToDte->faxParmStatus.jpegOptionJP);
#if SUPPORT_V34FAX

                        if (isV34Selected)
                        {
                            pT30->pfT30Vec = T30_PhaseB_GetConf;
                        }
                        else
#endif
                        {
                            pT30->pfT30Vec = T30_PhaseB_TCF_Tx; pT30->txStage = HS_TX_STAGE_SILENCE;
                            T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                            pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 1; // fixing issue 187
                        }
                    }

                    break;
            }

            break;
    }
}

/*
**    the function send TCF signal to remote
**    here TCFesult is used as the timer, it set
**    as 1.5 second before calling this function.
**    the function is mainly send the data to modem
**    and check modem send all data out and then
**    return -1.
*/
static SWORD T30_SendTCF(T30Struct *pT30)
{
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    SWORD sTemp;

    /* now we only consider the modem can take all data    */
    /*    for every 10 ms, so there is no buffer full in this level */
    if (pT30->TCFResult != 0)
    {
        if (pDceToT30->flowCntlCmd != DC3_XOFF)
        {
            memset(pT30ToDce->tempData, 0, pT30->TCFByteNum);
            pT30ToDce->tempDataLen = (UBYTE)(pT30->TCFByteNum);
            pT30ToDce->tempDataType = (pT30->TCFResult == 1 ? T30_DCE_HS_END : T30_DCE_HS_DATA);
            pT30->TCFResult--;
        }

        sTemp = pT30->TCFByteNum;
    }
    else if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
    {
        pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
        /* check modem send all data out or not */
        /* if all data gone, ReVal will be -1    */
        sTemp = -1;
    }
    else
    {
        sTemp = 0;
    }

    return sTemp;
}

/*
**    The function to send TCF signal for 1.5 second
**    atfer that, change pfT30Vec to T30_PhaseB_GetConf
*/
void T30_PhaseB_TCF_Tx(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    SWORD ret;

    switch (pT30->txStage)
    {
        case HS_TX_STAGE_SILENCE:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = HS_TX_STAGE_SYNC;

                TRACE1("T30: Send TCF at speed %02x" , pT30->T30Speed);

                T30_SetCommand(pT30ToDce, T30_DCE_FTM);
                pT30ToDce->tempParm[0] = T30_ConvertSpdToT31Mod(pT30->T30Speed, TRUE);

                TRACE0("T30: mode == TX");

                pT30->TCFByteNum = T30_BytesPer10ms[pT30ToDte->faxParmStatus.bitRateBR];

                /* TCFResult is used here for 1.52 second timer */
                /* 152 is based to the func is called every 10 ms */
                pT30->TCFResult = 152;
            }

            break;
        case HS_TX_STAGE_SYNC:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = HS_TX_STAGE_DATA;
            }

            break;
        default:
            ret = T30_SendTCF(pT30);

            if (ret == -1)
            {
                pT30->pfT30Vec = T30_PhaseB_GetConf;
                TRACE0("T30: TCF is sent");
            }

            break;
    }
}

/*
**    the function waiting TCF confirmation from remote
**    if the remote send CFR, it means TCF training is OK,
**  then start to send Image data;
**  if the remote send FTT, it means TCF training is bad,
**  then it needs to go back T30_PhaseB_SendDCS to send DCS frame
**  with lower speed, update T30speed and send TCF signal again.
*/
void T30_PhaseB_GetConf(T30Struct *pT30)
{
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    UBYTE hdlcRxStatus;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_B_XMIT); return;
    }

#endif

    hdlcRxStatus = T30_GetHDLCfromRemote(pT30, FALSE);

    switch (hdlcRxStatus)
    {
        case HDLC_INIT:
        case HDLC_NO_DATA:
        case HDLC_FLAG_RXED:
        case HDLC_IN_PROCESS:
            break;
        case HDLC_3S_TIMEOUT:
            T30_GotoPointC(pT30, HANGUP_RSPREC_ERROR_PHASE_B_XMIT);
            break;
        case HDLC_RX_ERROR:

            if (pT30->rxErrorTimes < T30_MAX_REPEAT_TIMES)
            {
                pT30->pfT30Vec = T30_PhaseB_SendDCS;
#if SUPPORT_V34FAX
                pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                pT30->rxErrorTimes++;
            }
            else
            {
                T30_GotoPointC(pT30, HANGUP_DCS_SENT_3_TIMES_NO_RSP);
                TRACE0("T30: Failed to receive response three times, disconnect the line. 2");
                pT30->rxErrorTimes = 0;
            }

            break;
        case HDLC_OPTFRM_RXED:
        case HDLC_CRC_BAD:
            break;
        case HDLC_LASTFRM_RXED:

            switch (pT30->hdlcTypeRx)
            {
                case HDLC_TYPE_CFR:
                    pT30ToDte->infoTxtRsp = INFO_TXT_FCS; TRACE("%s", "T30: +FCS:");
                    TRACE("%04x,", pT30ToDte->faxParmStatus.verticalResolutionVR);
                    TRACE("%x,", pT30ToDte->faxParmStatus.bitRateBR);
                    TRACE("%x,", pT30ToDte->faxParmStatus.pageWidthWD);
                    TRACE("%x,", pT30ToDte->faxParmStatus.pageLengthLN);
                    TRACE("%x,", pT30ToDte->faxParmStatus.dataFormatDF);
                    TRACE("%x,", pT30ToDte->faxParmStatus.errorCorrectionEC);
                    TRACE("%02x,", pT30ToDte->faxParmStatus.fileTransferBF);
                    TRACE("%x,", pT30ToDte->faxParmStatus.scanTimeST);
                    TRACE1("%02x", pT30ToDte->faxParmStatus.jpegOptionJP);
#if SUPPORT_ECM_MODE

                    if (pT30ToDte->faxParmStatus.errorCorrectionEC != EC_NON_ECM)
                    {
#if SUPPORT_V34FAX

                        if (!isV34Selected)
#endif
                        {
                            T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                            pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 5; // fixing issue 187
                        }

                        pT30->pfT30Vec = T30_PhaseC_ImageDataTx_ECM; pT30->txStage = HS_TX_STAGE_SILENCE;
                    }
                    else
#endif
                    {
                        T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                        pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 5; // fixing issue 187
                        pT30->pfT30Vec = T30_PhaseC_ImageDataTx_NoECM; pT30->txStage = HS_TX_STAGE_SILENCE;
                    }

                    break;
                case HDLC_TYPE_FTT:
#if SUPPORT_V34FAX

                    if (isV34Selected)
                    {
                        T30_GotoPointC(pT30, HANGUP_INVALID_RSP_PHASE_B_XMIT);
                        TRACE0("T30: ERROR. Invalid FTT for V.34");
                    }
                    else
#endif
                        if (pT30->wasLastTry)
                        {
                            T30_GotoPointC(pT30, HANGUP_FAILURE_TO_TRAIN_AT_FMS);
                            TRACE0("T30: Cannot retrain, send DCN and disconnect the line.");
                        }
                        else
                        {
                            pT30->pfT30Vec = T30_PhaseB_SendDCS; pT30->txStage = LS_TX_STAGE_PREAMBLE;
                            pT30->isFallBackTraining = TRUE;
                            TRACE0("T30: FTT is received: TCF Tx fail");
                        }

                    break;
                case HDLC_TYPE_DIS:
                case HDLC_TYPE_DTC:

                    if (pT30->rxDisDtcTimes < T30_MAX_REPEAT_TIMES)
                    {
                        if (T30_AnalysisDIS_DTC(pT30, (UBYTE)(pT30->hdlcTypeRx == HDLC_TYPE_DIS)))
                        {
                            pT30->pfT30Vec = T30_PhaseB_SendDCS;
#if SUPPORT_V34FAX
                            pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                            pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                            pT30->rxDisDtcTimes++;
                        }
                        else
                        {
                            T30_GotoPointC(pT30, HANGUP_REMOTE_CANNOT_RCV_OR_SEND);
                        }
                    }
                    else
                    {
                        T30_GotoPointC(pT30, HANGUP_DISDTC_RCVED_3_TIMES);
                        TRACE0("T30: Received DIS/DTC more than three times, send DCN and disconnect the line.");
                    }

                    break;
                case HDLC_TYPE_DCN:
                    TRACE0("T30: DCN instead of CFR/FTT is received");
                    break;
                default:
                    /* Not appropriate response received */
                    TRACE1("T30: ERROR. Invalid response 0x%x", pT30->hdlcTypeRx);
                    T30_GotoPointC(pT30, HANGUP_INVALID_RSP_PHASE_B_XMIT);
                    break;
            }

            T30_StopHDLCReceiving(pT30);
            break;
    }
}

#define DEBUG_NECM_IMAGE (0)

static UBYTE T30_FindEOL(T30Struct *pT30, UBYTE *ch, UWORD *pMsltAccumBitNum, UBYTE *pMsltNewBitNum)
{
    UBYTE i, mask;
    UBYTE isEOL = FALSE;
    UBYTE isMR = pT30->pT30ToDte->faxParmStatus.dataFormatDF == DF_MR;

    if (pT30->ImageState == IMAGE_STATE_RTC_EOB)
    {
        *ch = 0x00;
        return FALSE;
    }

    mask = 0x01;

    for (i = 0; i < 8; i ++)
    {
        if ((mask & *ch) == 0)
        {
            pT30->ZeroCount_EOL++;
        }
        else
        {
            if (isMR && pT30->ImageState == IMAGE_STATE_EOL)
            {
                // ignore the +0/1 bit of EOL
                if (pT30->EOLCount == 6)
                {
                    pT30->ImageState = IMAGE_STATE_RTC_EOB;
                    pT30->EOLCount = 0;
                    *ch &= 0xff >> (7 - i);
                    return FALSE;
                }

                pT30->ImageState = IMAGE_STATE_DATA;
            }
            else if (pT30->ZeroCount_EOL < 11)
            {
                pT30->EOLCount = 0;
            }
            else
            {
                /* this bit is 1 and at least 11 zero bit ahead this bit. */
                isEOL = TRUE;
                pT30->ImageState = IMAGE_STATE_EOL;
                *pMsltNewBitNum = 8 - i;

                if ((pT30->EOLCount != 0) && (pT30->ZeroCount_EOL >= 12))
                {
                    pT30->EOLCount = 1;
                }
                else
                {
                    pT30->EOLCount ++;
                }

                /* RTC = 6 x EOL for MH */
                /* RTC = 6 x (EOL + 1) for MR */
                if (pT30->EOLCount > 1)
                {
                    if (!isMR && pT30->EOLCount == 6)
                    {
                        pT30->ImageState = IMAGE_STATE_RTC_EOB;
                        *ch &= 0xff >> (7 - i);
                        pT30->EOLCount = 0;
                        return FALSE;
                    }

                    *pMsltAccumBitNum = 576; // enough for 14400 and 40ms mslt
                }
                else
                {
                    *pMsltAccumBitNum += i;
                }
            }

            pT30->ZeroCount_EOL = 0;
        }

        mask <<= 1;
    }

    if (!isEOL)
    {
        *pMsltAccumBitNum += 8;
        *pMsltNewBitNum = 0;
    }

    return isEOL;
}

static void T30_SendImageData_ToModem_NoECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_ECM_MODE
    UWORD i;
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UBYTE *pBlockStart = pT4->pblock;
    UBYTE *pBlockEnd = &pT4->pblock[NECM_BLOCKSIZE];
#if SUPPORT_DCE
    UBYTE minLen = (UBYTE)pT30->TCFByteNum;
#else
    UBYTE minLen = (UBYTE)(3 * pT30->TCFByteNum);
#endif
    UWORD bufLen;
#else
    CircBuffer *pT30_TxCB = &(pT30->T30_TxCB);
    UWORD len;
    SWORD Ret;
#endif
    UWORD uImageBufLen = pDteToT30->tempDataLen;

#if SUPPORT_ECM_MODE

    if (NECM_BLOCKSIZE - 1 - pT4->leftover_len < uImageBufLen) // fixing issue 145
    {
        pT30ToDte->dataCmd = DC3_XOFF;
        uImageBufLen = 0;
    }
    else
#endif
        if (pT30ToDte->dataCmd != DC1_XON)
        {
            pT30ToDte->dataCmd = DC1_XON;
        }

    if (pT30->ImageState == IMAGE_STATE_RTC_EOB)
    {
        pDteToT30->tempDataLen = 0;
    }
    else if (uImageBufLen != 0)
    {
#if SUPPORT_ECM_MODE
        int j;
        // fixing issue 37
        UBYTE *pImageBuf = pDteToT30->tempData;
        UBYTE isFull = FALSE;

        // search
        {
#if DEBUG_NECM_IMAGE
            static unsigned int byteCount = 0;

            TRACE("T30: I 0x%05x: ", byteCount);
#endif

            for (i = 0; i < uImageBufLen; i ++)
            {
                UBYTE currByte, msltNewBitNum;
                // fixing issue 174 and 175
                UWORD prevMsltAccumBitNum;
                UWORD prevZeroCount_EOL;
                UBYTE prevEOLCount;
                UBYTE prevImageState;

                if (pT30->ImageState == IMAGE_STATE_RTC_EOB)
                {
                    // the end of image data
                    break;
                }
                else if (pT4->leftover_len == NECM_BLOCKSIZE - 1)
                {
                    // cannot hold more data
                    pDteToT30->tempDataLen = uImageBufLen - i;

                    for (j = 0; i < uImageBufLen; i ++, j ++)
                    {
                        pDteToT30->tempData[j] = pDteToT30->tempData[i];
                    }

                    isFull = TRUE;
                    break;
                }

                if (RdReg(pDteToT30->dataBitOrder, BIT0) == 0) // fixing issue 137 and 167
                {
                    // Direct bit order for Phase C
                    currByte = *pImageBuf++;
                }
                else
                {
                    // Reversed bit order for Phase C
                    currByte = COMM_BitReversal(*pImageBuf++);
                }

#if DEBUG_NECM_IMAGE
                TRACE("%02x ", currByte);
#endif
                // backup
                prevMsltAccumBitNum = pT30->msltAccumBitNum;
                prevZeroCount_EOL = pT30->ZeroCount_EOL;
                prevEOLCount = pT30->EOLCount;
                prevImageState = pT30->ImageState;

                if (T30_FindEOL(pT30, &currByte, &pT30->msltAccumBitNum, &msltNewBitNum))
                {
                    // EOL is detected in currByte
                    SWORD padBitNum = pT30->msltBitNum - pT30->msltAccumBitNum;
                    SWORD padByteNum = (padBitNum >> 3) + ((padBitNum & 0x7) == 0 ? 0 : 1);

                    if (padByteNum < 0)
                    {
                        padByteNum = 0;
                    }

                    if ((padByteNum + 1) > (NECM_BLOCKSIZE - 1 - pT4->leftover_len))
                    {
                        // cannot hold padding 0's and the currByte. Need to restore previous condition
                        // fixing issue 174 and 175. restore
                        pT30->ZeroCount_EOL = prevZeroCount_EOL;
                        pT30->EOLCount = prevEOLCount;
                        pT30->ImageState = prevImageState;
                        pT30->msltAccumBitNum = prevMsltAccumBitNum;
                        pDteToT30->tempDataLen = uImageBufLen - i;

                        for (j = 0; i < uImageBufLen; i ++, j ++)
                        {
                            pDteToT30->tempData[j] = pDteToT30->tempData[i];
                        }

                        isFull = TRUE;
                        break;
                    }

                    if (padByteNum > 0)
                    {
                        pT4->leftover_len += padByteNum;

                        for (j = 0; j < padByteNum; j ++)
                        {
                            *pT4->pWrite++ = 0;

                            if (pT4->pWrite >= pBlockEnd)
                            {
                                pT4->pWrite = pBlockStart;
                            }
                        }
                    }

                    pT30->msltAccumBitNum = msltNewBitNum; // start a new line
                    pT4->pLast_EOL = pT4->pWrite;
                }

                pT4->leftover_len += 1;

                *pT4->pWrite++ = currByte;

                if (pT4->pWrite >= pBlockEnd)
                {
                    pT4->pWrite = pBlockStart;
                }
            }

#if DEBUG_NECM_IMAGE
            TRACE0("");

            byteCount += isFull ? pDteToT30->tempDataLen : uImageBufLen;
#endif
        }

        if (isFull)
        {
            // Need more space and will do it again next time
            pT30ToDte->dataCmd = DC3_XOFF;
        }
        else
        {
            pDteToT30->tempDataLen = 0;
        }

#else

        /* move data from T30Data to T30_TxCB buffer */
        if (!PutStringToCB(pT30_TxCB, pDteToT30->tempData, uImageBufLen))
        {
            pT30ToDte->dataCmd = DC3_XOFF;
        }
        else
        {
            pT30ToDte->dataCmd = DC1_XON;
            pDteToT30->tempDataLen = 0;
        }

#endif
    }

    if (pDceToT30->flowCntlCmd != DC3_XOFF)
    {
#if SUPPORT_ECM_MODE

        if (pDteToT30->postPgMsg != POST_PG_MSG_NULL)
        {
            // Send everything left
            bufLen = pT4->leftover_len;
            minLen = (UBYTE)MIN16(bufLen, minLen);
        }
        else
        {
            // usable length in the storage buffer
            bufLen = (UWORD)((pT4->pLast_EOL + NECM_BLOCKSIZE - pT4->pRead) & (NECM_BLOCKSIZE - 1));

            if (pT4->pRead == pBlockStart && pT4->pWrite == pBlockStart && pT4->pLast_EOL == pBlockStart)
            {
                // don't send 0 before image, let datapump send 0 if it has to
                minLen = 0;
            }
        }

        // download
        if (bufLen < minLen)
        {
            for (i = 0; i < bufLen; i ++)
            {
                pT30ToDce->tempData[i] = *pT4->pRead++;

                if (pT4->pRead >= pBlockEnd)
                {
                    pT4->pRead = pBlockStart;
                }
            }

            memset(&pT30ToDce->tempData[i], 0, minLen - bufLen);
            pT4->leftover_len -= bufLen;
            pT30ToDce->tempDataLen = minLen;
            pT30ToDce->tempDataType = T30_DCE_HS_DATA;
        }
        else if (minLen != 0)
        {
            for (i = 0; i < minLen; i ++)
            {
                pT30ToDce->tempData[i] = *pT4->pRead++;

                if (pT4->pRead >= pBlockEnd)
                {
                    pT4->pRead = pBlockStart;
                }
            }

            pT4->leftover_len -= minLen;
            pT30ToDce->tempDataLen = minLen;
            pT30ToDce->tempDataType = T30_DCE_HS_DATA;
        }

#else
        /* move data from T30_TxCB to modem buffer */
        len = pT30->T30_TxCB.CurrLen;

        if (len)
        {
            if (len > T30DATATRANSFERLEN)
            {
                len = T30DATATRANSFERLEN;
            }

            /* check Modem buffer space */
            Ret = pT30->TCFByteNum;

            /* move data from T30_TxCB buf to Modem buf */
            if (Ret > 1 || Ret == -1)
            {
                UWORD i;

                if (Ret <= len && Ret != -1)
                {
                    len = Ret;
                }

                GetStringFromCB(&(pT30->T30_TxCB), pT30ToDce->tempData, len);
                pT30ToDce->tempDataLen = (UBYTE)len;
                pT30ToDce->tempDataType = T30_DCE_NON_ECM_DATA;
            }

            /* check T30_TxCB buffer space and report to upper level */
            if ((pT30->T30_TxCB.Len - pT30->T30_TxCB.CurrLen) < T30DATATRANSFERLEN)
            {
                pT30ToDte->dataCmd = DC3_XOFF;    /* ask upper level do not send data to T.30 layer */

                //TRACE0("T30 Buffer Full for image Tx");
            }
        }

#endif
        else
        {
            // nothing to send, check if it's the end
            if (pDteToT30->postPgMsg != POST_PG_MSG_NULL) // fixing issue 133
            {
                switch (pDteToT30->postPgMsg)
                {
                    case POST_PG_MSG_EOP:
                        pT30->hdlcTypeTx = HDLC_TYPE_EOP;
                        break;
                    case POST_PG_MSG_MPS:
                        pT30->hdlcTypeTx = HDLC_TYPE_MPS;
                        break;
                    case POST_PG_MSG_EOM:
                        pT30->hdlcTypeTx = HDLC_TYPE_EOM;
                        pT30->wasEOM = TRUE;
                        break;
                    default:
                        TRACE0("T30: ERROR: pDteToT30->postPgMsg");
                        T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT);
                        break;
                }

                pDteToT30->postPgMsg = POST_PG_MSG_NULL; // fixing issue 133
                pT30ToDce->tempDataLen = 0;
                pT30ToDce->tempDataType = T30_DCE_HS_END;
#if SUPPORT_ECM_MODE
                pT4->pRead = pT4->pWrite = pT4->pLast_EOL = pBlockStart;
#endif
            }

            if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->pfT30Vec = T30_PhaseD_SendPE_NoECM; pT30->txStage = LS_TX_STAGE_SILENCE;
                T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 5; // fixing issue 187
                ///pT30->wasLastTry = FALSE; // LLL, not sure why it's here
            }
        }

#if DEBUG_NECM_IMAGE

        if (pT30ToDce->tempDataLen != 0)
        {
            static unsigned int byteCount = 0;

            TRACE("T30: O 0x%05x: ", byteCount);

            for (i = 0; i < pT30ToDce->tempDataLen; i ++)
            {
                TRACE("%02x ", pT30ToDce->tempData[i]);
            }

            TRACE0("");
            byteCount += pT30ToDce->tempDataLen;
        }

#endif
    }
}
#undef DEBUG_NECM_IMAGE

/*
**    the function send image data to remote
**    first get data from T30Data Txbuf and
**    send data to modem.
*/
void T30_PhaseC_ImageDataTx_NoECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;

    if (pDteToT30->actionCmd != DTE_T30_FDT)
    {
        return;
    }

    if (pDteToT30->isDiscCmd)
    {
        pT30ToDce->tempDataLen = 0;
        pT30ToDce->tempDataType = T30_DCE_HS_END;
        T30_GotoPointC(pT30, HANGUP_ABORTED_FROM_FKS_OR_CAN); return;
    }

    switch (pT30->txStage)
    {
        case HS_TX_STAGE_SILENCE:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = HS_TX_STAGE_SYNC;
                T30_SetCommand(pT30ToDce, T30_DCE_FTM);
                pT30ToDce->tempParm[0] = T30_ConvertSpdToT31Mod(pT30->T30Speed, FALSE);
            }

            break;
        case HS_TX_STAGE_SYNC:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = HS_TX_STAGE_DATA;
                pT30->ImageState = IMAGE_STATE_DATA;
                T30_SetResponse(pT30, RESULT_CODE_CONNECT);
                pT30->TCFByteNum = T30_BytesPer10ms[pT30ToDte->faxParmStatus.bitRateBR];

                //fixing issue 166
                {
                    UWORD zerosLen = pT30->TCFByteNum * 2; // 20ms?
#if SUPPORT_ECM_MODE
                    T4ECMstruc *pT4 = &pT30->T4ECM;

                    memset(pT4->pblock, 0, zerosLen);
                    pT4->pWrite = &pT4->ubECM_Block[zerosLen];
                    pT30->T4ECM.leftover_len = zerosLen;
                    pT30->msltAccumBitNum = zerosLen * 8;
#else
                    CircBuffer *pT30_TxCB = &(pT30->T30_TxCB);
                    UBYTE zeros[zerosLen];

                    memset(zeros, 0, zerosLen);
                    PutStringToCB(pT30_TxCB, zeros, zerosLen);
#endif
                    pT30->EOLCount = 0;
                }

                TRACE0("T30: Start to send Fax image without ECM");
            }

            break;
        default:
            T30_SendImageData_ToModem_NoECM(pT30);
            break;
    }
}

/*
**    according to the flag sent by T30 module
**    send EOP or MPS frame to remote, and then
**    move to T30PHASED_CONF state
*/
void T30_PhaseD_SendPE_NoECM(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    UWORD re = 0;

#if SUPPORT_V34FAX

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT); return;
    }

#endif

    switch (pT30->txStage)
    {
        case LS_TX_STAGE_SILENCE:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_PREAMBLE;
            }

            break;
        case LS_TX_STAGE_PREAMBLE:
            T30_SetCommand(pT30ToDce, T30_DCE_FTH);
            pT30ToDce->tempParm[0] = MOD_V21_300;
            pT30->txStage = LS_TX_STAGE_DATA;
            break;
        case LS_TX_STAGE_DATA:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_CRC_OK;
            }

            break;
        default:
            re = T30_CommandTx(pT30, pT30->hdlcTypeTx);

            if (re == 1)
            {
                T30_StopHDLCSending(pT30);
                pT30->pfT30Vec  = T30_PhaseC_GetConf_NoECM;
                TRACE1("T30: Send eop/mps %x", pT30->hdlcTypeTx);
            }

            break;
    }

}

void T30_PhaseC_GetConf_NoECM(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    UBYTE hdlcRxStatus;

#if SUPPORT_V34FAX

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT); return;
    }

#endif

    hdlcRxStatus = T30_GetHDLCfromRemote(pT30, FALSE);

    switch (hdlcRxStatus)
    {
        case HDLC_INIT:
        case HDLC_NO_DATA:
        case HDLC_FLAG_RXED:
        case HDLC_IN_PROCESS:
            /* do nothing during collecting HDLC data */
            break;
        case HDLC_OPTFRM_RXED:
        case HDLC_CRC_BAD:
            break;
        case HDLC_3S_TIMEOUT:
            T30_GotoPointC(pT30, HANGUP_RSPREC_ERROR_PHASE_D_XMIT);
            break;
        case HDLC_RX_ERROR:

            if (pT30->rxErrorTimes < T30_MAX_REPEAT_TIMES)
            {
                pT30->pfT30Vec = T30_PhaseD_SendPE_NoECM; pT30->txStage = LS_TX_STAGE_PREAMBLE;
                pT30->rxErrorTimes++;
            }
            else
            {
                UBYTE hangupCode;

                switch (pT30->hdlcTypeTx)
                {
                    case HDLC_TYPE_MPS:
                        hangupCode = HANGUP_NO_RSP_TO_MPS_3_TIMES;
                        break;
                    case HDLC_TYPE_EOP:
                        hangupCode = HANGUP_NO_RSP_TO_EOP_3_TIMES;
                        break;
                    case HDLC_TYPE_EOM:
                        hangupCode = HANGUP_NO_RSP_TO_EOM_3_TIMES;
                        break;
                    default:
                        hangupCode = HANGUP_RSPREC_ERROR_PHASE_D_XMIT;
                        break;
                }

                pT30->rxErrorTimes = 0;
                T30_GotoPointC(pT30, hangupCode);
                TRACE0("T30: Failed to receive response three times, disconnect the line. 3");
            }

            break;
        case HDLC_LASTFRM_RXED:

            switch (pT30->hdlcTypeRx)
            {
                case HDLC_TYPE_MCF:

                    if (pT30->hdlcTypeTx == HDLC_TYPE_EOP)
                    {
                        T30_GotoPointC(pT30, HANGUP_NORMAL);
                        TRACE0("T30: MCF is received and go T30PHASE E");
                    }
                    else if (pT30->hdlcTypeTx == HDLC_TYPE_MPS)
                    {
                        T30_SetResponse(pT30, RESULT_CODE_OK);
                        T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                        pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 5; // fixing issue 187
                        pT30->pfT30Vec = T30_PhaseC_ImageDataTx_NoECM; pT30->txStage = HS_TX_STAGE_SILENCE;
                        TRACE0("T30: MCF is received and go T30_PhaseC_ImageDataTx_NoECM for next page");
                    }
                    else if (pT30->hdlcTypeTx == HDLC_TYPE_EOM)
                    {
                        T30_SetResponse(pT30, RESULT_CODE_OK);
                        T30_InitForPhaseBReEntry(pT30);
                        pT30->pfT30Vec = T30_PhaseA_Call;
                    }

                    break;
                case HDLC_TYPE_RTN:
                    T30_SetResponse(pT30, RESULT_CODE_ERROR);
                    TRACE0("T30: RTN is received, Retrain");

                    if (pT30->hdlcTypeTx == HDLC_TYPE_EOM)
                    {
                        T30_InitForPhaseBReEntry(pT30);
                        pT30->pfT30Vec = T30_PhaseA_Call;
                    }
                    else if (pT30->hdlcTypeTx == HDLC_TYPE_EOP)
                    {
                        T30_GotoPointC(pT30, HANGUP_NORMAL);
                    }
                    else
                    {
                        pT30->pfT30Vec = T30_PhaseB_SendDCS; pT30->txStage = LS_TX_STAGE_PREAMBLE;
                    }

                    break;
                case HDLC_TYPE_RTP:
                    T30_SetResponse(pT30, RESULT_CODE_OK);

                    if (pT30->hdlcTypeTx == HDLC_TYPE_EOP)
                    {
                        T30_GotoPointC(pT30, HANGUP_NORMAL);
                    }
                    else if (pT30->hdlcTypeTx == HDLC_TYPE_MPS)
                    {
                        pT30->pfT30Vec = T30_PhaseB_SendDCS; pT30->txStage = LS_TX_STAGE_PREAMBLE;
                    }
                    else if (pT30->hdlcTypeTx == HDLC_TYPE_EOM)
                    {
                        T30_InitForPhaseBReEntry(pT30);
                        pT30->pfT30Vec = T30_PhaseA_Call;
                    }

                    break;
                case HDLC_TYPE_DCN:
                    TRACE0("T30: DCN instead of response-to-PE is received");
                    break;
                case HDLCCFCERR:

                    if (pT30->hdlcTypeTx == HDLC_TYPE_EOP)
                    {
                        T30_GotoPointC(pT30, HANGUP_NORMAL);
                        TRACE0("T30: MCF with error is received  and go T30PHASE");
                    }
                    else if (pT30->hdlcTypeTx == HDLC_TYPE_MPS)
                    {
                        T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                        pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 5; // fixing issue 187
                        pT30->pfT30Vec = T30_PhaseC_ImageDataTx_NoECM; pT30->txStage = HS_TX_STAGE_SILENCE;
                        TRACE0("T30: MCF with error is received  and go T30_PhaseC_ImageDataTx_NoECM for next page");
                    }

                    break;
                default:
                {
                    /* error */
                    UBYTE hangupCode;
                    TRACE1("T30: ERROR. Invalid response 0x%x", pT30->hdlcTypeRx);

                    switch (pT30->hdlcTypeTx)
                    {
                        case HDLC_TYPE_MPS:
                            hangupCode = HANGUP_INVALID_RSP_TO_MPS;
                            break;
                        case HDLC_TYPE_EOP:
                            hangupCode = HANGUP_INVALID_RSP_TO_EOP;
                            break;
                        case HDLC_TYPE_EOM:
                            hangupCode = HANGUP_INVALID_RSP_TO_EOM;
                            break;
                        default:
                            hangupCode = HANGUP_UNSPECIFIED_PHASE_D_XMIT;
                            break;
                    }

                    T30_GotoPointC(pT30, hangupCode);
                    break;
                }
            }

            T30_StopHDLCReceiving(pT30);
            break;
    }
}

void T30_PhaseE_Call(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif
    UWORD re = 0;

    switch (pT30->txStage)
    {
        case LS_TX_STAGE_SILENCE:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_PREAMBLE;
            }

            break;
        case LS_TX_STAGE_PREAMBLE:
            T30_SetCommand(pT30ToDce, T30_DCE_FTH);
            pT30ToDce->tempParm[0] = MOD_V21_300;
            pT30->txStage = LS_TX_STAGE_DATA;
            break;
        case LS_TX_STAGE_DATA:

            if ((pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
#if SUPPORT_V34FAX
                || isV34Selected
#endif
               )
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_CRC_OK;
            }

            break;
        default:
            pT30->hdlcTypeTx = HDLC_TYPE_DCN;
            re = T30_CommandTx(pT30, pT30->hdlcTypeTx);

            if (re == 1)
            {
                pT30->wasLastTry = TRUE;
                T30_StopHDLCSending(pT30);
                T30_GotoPointB(pT30, pT30ToDte->hangupCode); return;
            }
    }
}
#endif
