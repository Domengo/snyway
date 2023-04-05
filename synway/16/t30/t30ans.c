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
#include "faxlinecnt.h"

#if SUPPORT_T30
/*
**    there are 8 function related to 8 statuses for called side
**    (1)        T30_PhaseA_Ans()
**    (2)        T30_PhaseB_GetDCS();
**    (3)        T30_PhaseB_TCF_Rx()
**    (4)        T30_PhaseB_SendConf()
**    (5)        T30_PhaseC_ImageDataRx();
**    (6)        T30_PhaseD_GetPE()
**    (7)        T30_PhaseC_SendConf();
**    (8)        T30_PhaseE_Ans()
*/

/******************************************************
**    the function send NSF CSI and DIS Frame to remote,
**    and then change the pfT30Vec into T30_PhaseB_GetDCS
******************************************************/
void T30_PhaseA_Ans(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    SWORD re = 0;
    int i;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV); return;
    }

#endif

    if (pT30->Timer_T1_Counter > TIMER_T1_LENGTH)
    {
        pT30->Timer_T1_Counter = 0; TRACE0("T30: T1 elapsed sending DIS/DTC, send DCN and disconnect the line.(RX)");

        if (pT30->wasEOM)
        {
            T30_GotoPointC(pT30, HANGUP_T1_TIMEOUT_AFTER_EOM);
        }
        else
        {
            T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV);
        }

        return;
    }

    if (pT30->wasDisRcved)
    {
        if (pDteToT30->actionCmd == DTE_T30_FDR)
        {
            pT30->isTransmit = FALSE;
        }
        else
        {
            // need FDR to start polling
            return;
        }
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

                    if (pT30->wasDisRcved && pDteToT30->passWord[0] != NUL && RdReg(pT30->remoteFAPCap, BIT0))
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_PWD;
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

                    if (pT30->wasDisRcved && RdReg(pT30->remoteFAPCap, BIT1))
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_SEP;
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
                        ResetReg(pT30->remoteFAPCap, BIT1);
                    }

                    pT30->frmStage++;
                    break;
                case LS_FRAME_OPT3:

                    if (pT30->wasDisRcved)
                    {
                        if (pDteToT30->nonStandardFrmLen != 0)
                        {
                            pT30->hdlcTypeTx = HDLC_TYPE_NSC;
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
                    }
                    else if (pDteToT30->nonStandardFrmLen != 0)
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_NSF;
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
                case LS_FRAME_OPT4:

                    if (pT30->wasDisRcved)
                    {
                        if (pDteToT30->localPollingID[0] != NUL)
                        {
                            pT30->hdlcTypeTx = HDLC_TYPE_CIG;
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
                    }
                    else if (pDteToT30->localID[0] != NUL)
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_CSI;
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

                    if (pT30->wasDisRcved)
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_DTC;
                    }
                    else
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_DIS;
                    }

                    re = T30_CommandTx(pT30, pT30->hdlcTypeTx);

                    if (re == 1)
                    {
                        T30_StopHDLCSending(pT30);

                        pT30->pfT30Vec = T30_PhaseB_GetDCS; pT30->isDcsAsCmd = pT30->hdlcTypeTx == HDLC_TYPE_DTC ? TRUE : FALSE;
                    }

                    break;
            }

            break;
    }
}

void T30_PhaseB_GetDCS(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    UBYTE hdlcRxStatus;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        if (pT30->wasEOM)
        {
            pT30->wasEOM = FALSE;
            pT30->pfT30Vec = T30_PhaseB_AfterEOM;
            return;
        }
        else
        {
            T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV); return;
        }
    }

    // fixing issue 169
    if (isV34Selected && pDceToT30->selectedV34Chnl == V34_PRIMARY_CHNL)
    {
        pT30->pfT30Vec = T30_PhaseC_ImageDataRx_ECM; pT30->rxStage = HS_RX_STAGE_FRM;
        return;
    }

#endif

    if (pT30->isDcsAsCmd)
    {
        hdlcRxStatus = T30_GetHDLCfromRemote(pT30, TRUE);

        if (pT30->Timer_T2_Counter > TIMER_T2_LENGTH)
        {
            if (pT30->rxErrorTimes < T30_MAX_REPEAT_TIMES)
            {
                pT30->Timer_T2_Counter = 1; TRACE0("T30: T2 elapsed. Retry");
                pT30->rxErrorTimes++;
            }
            else
            {
                pT30->Timer_T2_Counter = 0; TRACE0("T30: T2 elapsed multiple times");
                pT30->rxErrorTimes = 0;
                T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV); return;
            }
        }
    }
    else
    {
        hdlcRxStatus = T30_GetHDLCfromRemote(pT30, FALSE);

        switch (hdlcRxStatus)
        {
            case HDLC_CRC_BAD:
                //pT30->Timer_3s_Counter = 0; // fix issue 212,wait for no carrior 200ms overtime, cancel 3 s overtime
                return;
            case HDLC_3S_TIMEOUT:
                T30_GotoPointC(pT30, HANGUP_RSPREC_ERROR_PHASE_B_RCV);
                return;
            case HDLC_RX_ERROR:

                if (pT30->rxErrorTimes < T30_MAX_REPEAT_TIMES)
                {
                    pT30->rxErrorTimes++;
                }
                else
                {
                    // user-defined procedure
                }

                if (pT30->Timer_T1_Counter <= TIMER_T1_LENGTH)
                {
                    if (pT30->hdlcTypeRx == HDLC_TYPE_CRP || pT30->WasTimeout
#if SUPPORT_V34FAX
                        || isV34Selected
#endif
                       )
                    {
                        // fixing issue 192, resend DIS if it's V.34 or CRP or T4 timeout
                        pT30->pfT30Vec = T30_PhaseA_Ans;
#if SUPPORT_V34FAX
                        pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                        pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                        TRACE0("T30: Resend DIS before T1 expires.");
                    }
                    else
                    {
                        //pT30->Timer_TCF = 1; TRACE0("T30: TCF timer starts"); // fixing issue 192, wait 3 seconds before resending DIS
                        // fixing issue 195, instead of starting 3-second timer, use +FRS to get the end of TCF
                        T30_SetCommand(pT30ToDce, T30_DCE_FRS);
                        pT30ToDce->tempParm[0] = 9;
                        pT30->pfT30Vec = T30_PhaseB_TCF_Rx; pT30->rxStage = HS_RX_STAGE_FRM;
                    }
                }
                else
                {
                    T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV);
                    pT30->Timer_T1_Counter = 0; TRACE0("T30: T1 elapsed during got DCS, send DCN and disconnect the line.(RX)");
                }

                return;
            case HDLC_INIT:
            case HDLC_NO_DATA:
            case HDLC_FLAG_RXED:
            case HDLC_IN_PROCESS:
                /* do nothing during collecting HDLC data */
                return;
        }
    }

    if (hdlcRxStatus == HDLC_OPTFRM_RXED)
    {
    }
    else if ((hdlcRxStatus == HDLC_LASTFRM_RXED))
    {
        pT30->Timer_T1_Counter = 0; TRACE0("T30: T1 reset");

        /* get one frame with LAST FRAME control field */
        switch (pT30->hdlcTypeRx)
        {
            case HDLC_TYPE_PWD:
            case HDLC_TYPE_SUB:
            case HDLC_TYPE_TSI:
            case HDLC_TYPE_DCS:

                if (pT30->hdlcTypeRx != HDLC_TYPE_DCS)
                {
                    T30_ProcessOptionalFrame(pT30, pT30->hdlcTypeRx);
                }
                else
                {
                    T30_AnalysisDCS_CTC(pT30, FALSE);
                }

#if SUPPORT_V34FAX

                if (isV34Selected)/* Send CFR directly for V34 Fax */
                {
                    pT30->pfT30Vec = T30_PhaseB_SendConf; pT30->txStage = LS_TX_STAGE_DATA;
                }
                else
#endif
                {
                    pT30->Timer_TCF = 1; TRACE0("T30: TCF timer starts");
                    pT30->pfT30Vec = T30_PhaseB_TCF_Rx; pT30->rxStage = HS_RX_STAGE_FRM;
                }

                // fixing issue 177 and 178
                if (pDteToT30->actionCmd != DTE_T30_FDR)
                {
                    T30_SetResponse(pT30, RESULT_CODE_OK);
                }

                break;
            case HDLC_TYPE_DTC:

                if (T30_AnalysisDIS_DTC(pT30, FALSE) && pDteToT30->isDocForPolling)
                {
#if SUPPORT_V34FAX

                    if (isV34Selected)
                    {
                        if (pT30->wasEOM)
                        {
                            pT30->wasEOM = FALSE;
                            pT30->pfT30Vec = T30_PhaseB_AfterEOM;
                            T30_SetCommand(pT30ToDce, T30_DCE_DLE_EOT);
                        }
                        else
                        {
                            pT30->pfT30Vec = T30_PhaseB_SendDCS; pT30->txStage = LS_TX_STAGE_DATA;
                        }
                    }
                    else
#endif
                    {
                        pT30->pfT30Vec = T30_PhaseB_SendDCS; pT30->txStage = LS_TX_STAGE_PREAMBLE;
                    }

                    pT30->isTransmit = TRUE;

                    // fixing issue 177 and 178
                    if (pDteToT30->actionCmd != DTE_T30_FDT)
                    {
                        T30_SetResponse(pT30, RESULT_CODE_OK);
                    }
                }
                else
                {
                    pT30->wasLastTry = TRUE; // fixing issue 181, perform an orderly disconnect
                    T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV);
                }

                break;
            case HDLC_TYPE_DIS:
                T30_AnalysisDIS_DTC(pT30, TRUE);

                if (pT30ToDte->isRmtDocToPoll && pDteToT30->isRequestingToPoll)
                {
                    // DTC lost, resend it
                    pT30->pfT30Vec = T30_PhaseA_Ans;
#if SUPPORT_V34FAX
                    pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                    pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                }
                else
                {
                    T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV);
                }

                break;
            case HDLC_TYPE_DCN:
                TRACE0("T30: DCN instead of DCS is received");
                break;
            default:
                TRACE1("T30: ERROR. Invalid message 0x%x", pT30->hdlcTypeRx);
                T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV);
                break;
        }

        T30_StopHDLCReceiving(pT30);
    }
}


static UBYTE T30_GetTCF(T30Struct *pT30)
{
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    SWORD re, i;

    re = pDceToT30->tempDataLen;

    if (re > 0)
    {
        pDceToT30->tempDataLen = 0;

        for (i = 0; i < re; i++)
        {
            if (pDceToT30->tempData[i] == 0)
            {
                pT30->TCFResult++;
            }
            else
            {
                if (pT30->TCFResult < pT30->TCFByteNum)
                {
                    if (pT30->TCFResult)
                    {
                        TRACE2("T30: Get TCF %d before reset (the threshold is %d)", pT30->TCFResult, pT30->TCFByteNum);
                    }

                    pT30->TCFResult = 0;
                }
            }
        }
    }
    else if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER)
    {
        pDceToT30->tempDataType = DCE_T30_DATA_NULL;
        pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
        /* modem detected carrier lost */
        return (0);
    }

    return (1);
}

void T30_PhaseB_TCF_Rx(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    UBYTE    ret;

    // fixing issue 194, removing the following check for FDR
    //if (pDteToT30->actionCmd != DTE_T30_FDR)
    //{
    //    return;
    //}
    if (pT30ToDce->currentT30DceCmd == T30_DCE_FRS)
    {
        // error during DCS reception
        // cannot start modem because we don't know which to start
        if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
        {
            // fixing issue 195
            pT30->pfT30Vec = T30_PhaseA_Ans; pT30->txStage = LS_TX_STAGE_PREAMBLE;
        }

        return;
    }

    switch (pT30->rxStage)
    {
        case HS_RX_STAGE_FRM:

            /* using this parameter for the threshold of TCF signal */
            if (pT30->T30Speed == DCS_V27_2400)
            {
                pT30->TCFByteNum = 390;
            }
            else
            {
                pT30->TCFByteNum = (UWORD)(T30_BytesPer10ms[pT30ToDte->faxParmStatus.bitRateBR] * 133);
            }/* 148 */ /* 1.35s - 1.65s */

            pT30->TCFResult = 0;
            T30_SetCommand(pT30ToDce, T30_DCE_FRM);
            pT30ToDce->tempParm[0] = T30_ConvertSpdToT31Mod(pT30->T30Speed, TRUE);
            TRACE1("T30: Start Fax at %02x speed to receive TCF", pT30->T30Speed);
            pT30->rxStage = HS_RX_STAGE_CONNECT;
            break;
        case HS_RX_STAGE_CONNECT:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
            {
                pT30->Timer_TCF = 0; TRACE0("T30: TCF timer reset");
                pT30->rxStage = HS_RX_STAGE_DATA;
            }
            else if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER ||
                     pDceToT30->resultCodeRsp == RESULT_CODE_OK) // fixing bug 24, looks like OK is only possible for <CAN>
            {
                pT30->Timer_TCF = 0; TRACE0("T30: TCF timer reset");
                pT30->TCFResult = T30TCFFAIL;
                pT30->pfT30Vec = T30_PhaseB_SendConf; pT30->txStage = LS_TX_STAGE_PREAMBLE;
            }
            else if (pDceToT30->resultCodeRsp == RESULT_CODE_ERROR) // fixing bug 27, for +FCERROR followed by ERROR
            {
                if (pDceToT30->infoTxtRsp == INFO_TXT_FCERROR)
                {
                    pT30->Timer_TCF = 0; TRACE0("T30: TCF timer reset");
                    pT30->TCFResult = T30TCFFAIL;
                    pDceToT30->infoTxtRsp = INFO_TXT_NULL;
                    pT30->pfT30Vec = T30_PhaseB_GetDCS; pT30->isDcsAsCmd = TRUE;
                }
                else
                {
                    TRACE0("T30: ERROR. Unexpected ERROR");
                }
            }
            else if (pT30->Timer_TCF >= TIMER_TCF_LENGTH)
            {
                pT30->Timer_TCF = 0; TRACE0("T30: TCF timer elapsed");
                pT30->TCFResult = T30TCFFAIL;
                pT30ToDce->flowCntlCmd = CAN_STOP; TRACE0("T30: <CAN> to DCE"); // fixing issue 178
            }

            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            break;
        default:
            ret = T30_GetTCF(pT30);

            if (ret == 0)
            {
                /*    modem detected no carrier */
                TRACE1("T30: TCF num is %d", pT30->TCFResult);

                if (pT30->TCFResult >= pT30->TCFByteNum)
                {
                    pT30->TCFResult = T30TCFOK;
                    TRACE0("T30: TCF OK");
                }
                else
                {
                    pT30->TCFResult = T30TCFFAIL;
                    TRACE0("T30: TCF fail");
                }

                pT30->pfT30Vec = T30_PhaseB_SendConf; pT30->txStage = LS_TX_STAGE_PREAMBLE;
            }

            break;
    }
}

void T30_PhaseB_SendConf(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    SWORD re = 0;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV); return;
    }

#endif

    // fixing issue 181
    if (pT30->wasLastTry)
    {
        T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_RCV); return;
    }

    if (pDteToT30->actionCmd != DTE_T30_FDR)
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
#if SUPPORT_V34FAX

            if (isV34Selected)
            {
                pT30->hdlcTypeTx = HDLC_TYPE_CFR;
                pT30->txStage = LS_TX_STAGE_CRC_OK;
            }
            else
#endif
                if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
                {
                    pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                    pT30->txStage = LS_TX_STAGE_CRC_OK;

                    if (pT30->TCFResult == T30TCFOK)
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_CFR;/* if pT30->HDLCType = 0xFF, remote fax will send V21 signal, not send image data */
                    }
                    else
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_FTT;
                    }
                }

            break;
        default:
            re = T30_CommandTx(pT30, pT30->hdlcTypeTx);

            if (re == 1)
            {
                pT30->TCFResult = 0;

                T30_StopHDLCSending(pT30);

                switch (pT30->hdlcTypeTx)
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

                            if (isV34Selected)
                            {
                                // fixing issue 169
                                pT30->pfT30Vec = T30_PhaseB_GetDCS; pT30->isDcsAsCmd = TRUE;
                            }
                            else
#endif
                            {
                                pT30->pfT30Vec = T30_PhaseC_ImageDataRx_ECM; pT30->rxStage = HS_RX_STAGE_FRM;
                            }
                        }
                        else
#endif
                        {
                            pT30->pfT30Vec = T30_PhaseC_ImageDataRx_NoECM; pT30->rxStage = HS_RX_STAGE_FRM;
                        }

                        break;
                    default:
                        pT30->pfT30Vec = T30_PhaseB_GetDCS; pT30->isDcsAsCmd = TRUE;
                        break;
                }
            }
    }
}

void T30_PhaseC_ImageDataRx_NoECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    UWORD len;

    if (pDteToT30->actionCmd != DTE_T30_FDR)
    {
        return;
    }

    if (pDteToT30->isDiscCmd) // fixing bug 7
    {
        T30_GotoPointB(pT30, HANGUP_ABORTED_FROM_FKS_OR_CAN); return;
    }

    switch (pT30->rxStage)
    {
        case HS_RX_STAGE_FRM:
            pT30->rxStage = HS_RX_STAGE_CONNECT;
            T30_SetCommand(pT30ToDce, T30_DCE_FRM);
            pT30ToDce->tempParm[0] = T30_ConvertSpdToT31Mod(pT30->T30Speed, FALSE);
            TRACE1("T30: Start Fax at %02x speed to receive Fax image without ECM", pT30->T30Speed);
            pT30->Timer_T2_Counter = 1; TRACE0("T30: HS T2 starts");
            break;
        case HS_RX_STAGE_CONNECT:

            switch (pDceToT30->resultCodeRsp)
            {
                case RESULT_CODE_CONNECT:
                    T30_SetResponse(pT30, RESULT_CODE_CONNECT);
                    pT30->rxStage = HS_RX_STAGE_DATA;
                    pT30->ImageState = IMAGE_STATE_DATA;
                    pT30->Timer_T2_Counter = 0; TRACE0("T30: HS T2 reset");
                    break;
                case RESULT_CODE_ERROR:
                    pT30->Timer_T2_Counter = 0; TRACE0("T30: HS T2 reset");

                    if (pDceToT30->infoTxtRsp == INFO_TXT_FCERROR)
                    {
                        pDceToT30->infoTxtRsp = INFO_TXT_NULL;

                        switch (pT30->hdlcTypeTx)
                        {
                            case HDLC_TYPE_MCF:
                                pT30->pfT30Vec = T30_PhaseD_GetPE_NoECM;
                                break;
                            default:
                                pT30->pfT30Vec = T30_PhaseB_GetDCS; pT30->isDcsAsCmd = TRUE;
                                break;
                        }
                    }
                    else
                    {
                        TRACE0("T30: ERROR. pDceToT30->infoTxtRsp");
                        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_C_RCV); return;
                    }

                    break;
            }

            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;

            if (pT30->Timer_T2_Counter > TIMER_T2_LENGTH)
            {
                pT30->Timer_T2_Counter = 0; TRACE0("T30: HS T2 elapsed, no image");
                T30_GotoPointB(pT30, HANGUP_T2_TIMEOUT_PG_NOT_RCVED); return;
            }

            break;
        default:

            if (pDteToT30->dataCmd == DC3_XOFF)
            {
                TRACE0("T30: ERROR. pDteToT30->dataCmd");
                T30_GotoPointB(pT30, HANGUP_T30_DTE_DATA_OVERFLOW); return;
            }

            // LLL DC2 checking
            len = MIN16(pDceToT30->tempDataLen, sizeof(pT30ToDte->tempData));

            if (len > 0)
            {
                UWORD i, j;

                /* there are data coming from modem */
#if SUPPORT_T38EP
                T30_UploadImgToDte(pT30, pDceToT30->tempData, len, TRUE); // fixing issue 137
#else
                T30_UploadImgToDte(pT30, pDceToT30->tempData, len, FALSE); // fixing issue 137
#endif

                for (i = len, j = 0; i < pDceToT30->tempDataLen; i++, j++)
                {
                    pDceToT30->tempData[j] = pDceToT30->tempData[i];
                }

                pDceToT30->tempDataLen -= len;
            }
            else if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER)
            {
                if (pT30->LineCount == 0)
                {
                    pT30->LineCount = 1144; // LLL debug
                }

                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30ToDte->tempParm[0] = POST_PG_RSP_MCF; // always is MCF when no ECM
                pT30ToDte->tempParm[1] = (pT30->LineCount >> 8) & 0xff;
                pT30ToDte->tempParm[2] = pT30->LineCount & 0xff;
                pT30ToDte->infoTxtRsp = INFO_TXT_FPS; TRACE2("T30: +FPS:%x,%d,0,0,0", pT30ToDte->tempParm[0], pT30->LineCount);
                pT30->LineCount = 0;
                pT30->pfT30Vec = T30_PhaseD_GetPE_NoECM;
                pT30->hdlcTypeTx = HDLC_TYPE_INIT; // fixing issue 122. MCF should be removed from here
            }
    }
}

void T30_PhaseD_GetPE_NoECM(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    UBYTE hdlcRxStatus;

    hdlcRxStatus = T30_GetHDLCfromRemote(pT30, TRUE);

    if (pT30->Timer_T2_Counter > TIMER_T2_LENGTH)
    {
        pT30->Timer_T2_Counter = 0; TRACE0("T30: T2 elapsed");
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_RCV); return;
    }

    switch (hdlcRxStatus)
    {
        case HDLC_LASTFRM_RXED:

            switch (pT30->hdlcTypeRx)
            {
                case HDLC_TYPE_EOP:
                case HDLC_TYPE_MPS:
                case HDLC_TYPE_EOM:
                case HDLC_TYPE_PRI_EOM:
                case HDLC_TYPE_PRI_MPS:
                case HDLC_TYPE_PRI_EOP:
                    pT30->hdlcTypeRx &= 0xF7FF;

                    if (pT30->hdlcTypeTx != HDLC_TYPE_MCF)
                    {
                        T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;

                        switch (pT30->hdlcTypeRx)
                        {
                            case HDLC_TYPE_EOP:
                                pT30ToDte->tempParm[0] = POST_PG_MSG_EOP;
                                break;
                            case HDLC_TYPE_MPS:
                                pT30ToDte->tempParm[0] = POST_PG_MSG_MPS;
                                break;
                            case HDLC_TYPE_EOM:
                                pT30->wasEOM = TRUE;
                                pT30ToDte->tempParm[0] = POST_PG_MSG_EOM;
                                break;
                        }

                        pT30ToDte->infoTxtRsp = INFO_TXT_FET; TRACE1("T30: +FET:%d", pT30ToDte->tempParm[0] - 1);
                        T30_SetResponse(pT30, RESULT_CODE_OK);
                    }

                    pT30->pfT30Vec = T30_PhaseC_SendConf_NoECM; pT30->txStage = LS_TX_STAGE_PREAMBLE;
                    break;
                case HDLC_TYPE_DCN:
                    T30_AssumeEopUponInvCmd(pT30);
                    TRACE0("T30: DCN instead of PE is received");
                    break;
                case HDLC_TYPE_DCS:
                    T30_AnalysisDCS_CTC(pT30, FALSE);
                    pT30->Timer_TCF = 1; TRACE0("T30: TCF timer starts");
                    pT30->pfT30Vec = T30_PhaseB_TCF_Rx; pT30->rxStage = HS_RX_STAGE_FRM;
                    break;
                default:
                    T30_AssumeEopUponInvCmd(pT30);
                    TRACE1("T30: ERROR. Invalid command 0x%x", pT30->hdlcTypeRx);
                    T30_GotoPointC(pT30, HANGUP_INVALID_CMD_PHASE_D_RCV);
                    break;
            }

            T30_StopHDLCReceiving(pT30);
            break;
    }
}

void T30_PhaseC_SendConf_NoECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    UWORD re = 0;

    if (pDteToT30->actionCmd == DTE_T30_FPS)
    {
        pT30->isFpsModified = TRUE;

        switch (pDteToT30->tempData[0])
        {
            case POST_PG_RSP_RTN:
                pT30->hdlcTypeTx = HDLC_TYPE_RTN;
                break;
            case POST_PG_RSP_RTP:
                pT30->hdlcTypeTx = HDLC_TYPE_RTP;
                break;
            default:
                pT30->hdlcTypeTx = HDLC_TYPE_MCF;
                break;
        }

        T30_SetResponse(pT30, RESULT_CODE_OK);
        return;
    }
    else if (!pT30->isFpsModified)
    {
        pT30->hdlcTypeTx = HDLC_TYPE_MCF;
    }

    if (pDteToT30->actionCmd != DTE_T30_FDR)
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

            if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_CRC_OK;
            }

            break;
        default:
            /* need add option for send RTN frame to retrain */
            re = T30_CommandTx(pT30, pT30->hdlcTypeTx);

            if (re == 1)
            {
                pT30->isFpsModified = FALSE;
                T30_StopHDLCSending(pT30);

                if (pT30->hdlcTypeRx == HDLC_TYPE_MPS)
                {
                    pT30->pfT30Vec = T30_PhaseC_ImageDataRx_NoECM; pT30->rxStage = HS_RX_STAGE_FRM;
                }
                else
                {
                    pT30->pfT30Vec = T30_PhaseE_Ans;
                }

                TRACE0("T30: Post page response is sent for answer without ECM");
            }
    }
}

void T30_PhaseE_Ans(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    UBYTE hdlcRxStatus;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

    hdlcRxStatus = T30_GetHDLCfromRemote(pT30, TRUE);

    switch (hdlcRxStatus)
    {
        case HDLC_RX_ERROR:
            T30_GotoPointB(pT30, HANGUP_NORMAL); return;
        case HDLC_LASTFRM_RXED:

            switch (pT30->hdlcTypeRx)
            {
#if SUPPORT_ECM_MODE
                case PPS_EOP:
                case EOR_EOP:
                    pT30->pfT30Vec = T30_PhaseC_SendConf_ECM;
#if SUPPORT_V34FAX
                    pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                    pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                    break;
#endif
                case HDLC_TYPE_EOP:
                    pT30->pfT30Vec = T30_PhaseC_SendConf_NoECM; pT30->txStage = LS_TX_STAGE_PREAMBLE;
                    break;
            }

            T30_StopHDLCReceiving(pT30);
            break;
    }

    if (pT30->Timer_T2_Counter >= TIMER_T2_LENGTH)
    {
        if (pT30->wasEOM)
        {
            // fixing bug 24, need to wait for OK/NO CARRIER to <CAN>
            if (pT30->Timer_T2_Counter == TIMER_T2_LENGTH)
            {
                // fixing issue 188
                pT30ToDce->flowCntlCmd = CAN_STOP; TRACE0("T30: <CAN> to DCE");
#if SUPPORT_V34FAX

                if (isV34Selected)
                {
                    pT30->txStage = LS_TX_STAGE_DATA;
                }
                else
#endif
                {
                    pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                    return;
                }
            }
            else if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER ||
                     pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_PREAMBLE;
            }
            else
            {
                return;
            }

            // fixing bug 24
            // has waited T2, and no new commands, can go to phase B again

            pT30->hdlcRxStatus = HDLC_INIT;
            T30_InitForPhaseBReEntry(pT30);
            pT30->pfT30Vec = T30_PhaseA_Ans;
        }
        else
        {
            pT30->wasLastTry = TRUE;
            T30_GotoPointB(pT30, HANGUP_NORMAL); return;
        }

        pT30->Timer_T2_Counter = 0; TRACE0("T30: T2 elapsed, no DCN.");
    }
}
#endif


