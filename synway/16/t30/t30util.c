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

#include <stdio.h>
#include <string.h>
#include "t30ext.h"
#include "common.h"
#include "hdlcext.h"

#if SUPPORT_T30
static UBYTE T30_UploadOptFrmToDte(T30Struct *pT30, UBYTE *ch_add)
{
    UBYTE len, i, j;
#if SUPPORT_T38EP

    if (pT30->HDLCLen > 3)
    {
        len = pT30->HDLCLen - 3; // LLL we don't send 2-byte FCS in T38 HDLC
#else

    if (pT30->HDLCLen > 5)
    {
        len = pT30->HDLCLen - 5;
#endif

        if (len <= OPTFRAMELEN)
        {
            for (i = 0; i < len; i++)
            {
                j = len - 1 - i;

                ch_add[j] = COMM_BitReversal(pT30->HDLC_RX_Buf[3 + i]);
            }
        }
        else
        {
            TRACE1("T30: OPTFRAMELEN must change to %d to hold this frame", len);
            len = 0;
        }
    }
    else
    {
        len = 0;
    }

    return len;
}

void T30_ProcessOptionalFrame(T30Struct *pT30, UWORD type)
{
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;

    switch (type)
    {
        case HDLC_TYPE_DCS:
            TRACE0("T30: DCS is received in opt frame");
            T30_AnalysisDCS_CTC(pT30, FALSE);
            break;
        case HDLC_TYPE_SUB:
            pT30ToDte->infoTxtRsp = INFO_TXT_FSA; TRACE0("T30: +FSA");
            pT30ToDte->tempParmLen = T30_UploadOptFrmToDte(pT30, pT30ToDte->tempParm);
            break;
        case HDLC_TYPE_CIG:
            pT30ToDte->infoTxtRsp = INFO_TXT_FPI; TRACE0("T30: +FPI");
            pT30ToDte->tempParmLen = T30_UploadOptFrmToDte(pT30, pT30ToDte->tempParm);
            break;
        case HDLC_TYPE_TSI:
            pT30ToDte->infoTxtRsp = INFO_TXT_FTI; TRACE0("T30: +FTI");
            pT30ToDte->tempParmLen = T30_UploadOptFrmToDte(pT30, pT30ToDte->tempParm);
            break;
        case HDLC_TYPE_DIS:
            pT30->wasDisRcved = TRUE;
            T30_AnalysisDIS_DTC(pT30, TRUE);
            break;
        case HDLC_TYPE_NSF:
            pT30ToDte->infoTxtRsp = INFO_TXT_FNF; TRACE0("T30: +FNF");
            pT30ToDte->tempParmLen = T30_UploadOptFrmToDte(pT30, pT30ToDte->tempParm);
            break;
        case HDLC_TYPE_CSI:
            pT30ToDte->infoTxtRsp = INFO_TXT_FCI; TRACE0("T30: +FCI");
            pT30ToDte->tempParmLen = T30_UploadOptFrmToDte(pT30, pT30ToDte->tempParm);
            break;
        default:
            TRACE1("T30: ERROR. Invalid optional frame 0x%x", type);
            break;
    }

    T30_StopHDLCReceiving(pT30);
}

#if T30_RESPOND_CRP
static UBYTE T30_RespondCRP(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

    switch (pT30->txStage)
    {
        case LS_TX_STAGE_SILENCE:
            pT30->txStage = LS_TX_STAGE_PREAMBLE;
            return 0;
        case LS_TX_STAGE_PREAMBLE:
            T30_SetCommand(pT30ToDce, T30_DCE_FTH);
            pT30ToDce->tempParm[0] = MOD_V21_300;
            pT30->txStage = LS_TX_STAGE_DATA;
            return 0;
        case LS_TX_STAGE_DATA:

            if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT
#if SUPPORT_V34FAX
                || isV34Selected
#endif
               )
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_CRC_OK;
            }

            return 0;
        default:
        {
            UBYTE re;

            pT30->hdlcTypeTx = HDLC_TYPE_CRP;
            re = T30_CommandTx(pT30, pT30->hdlcTypeTx);

            if (re == 1)
            {
                T30_StopHDLCSending(pT30);
            }

            return re;
        }
    }
}
#endif

UBYTE T30_GetHDLCfromRemote(T30Struct *pT30, UBYTE isCmd)
{
    UBYTE entryHdlcRxStatus = pT30->hdlcRxStatus;
    UBYTE prevHdlcRxStatus = entryHdlcRxStatus;
    UBYTE hdlcRxStatusRet = entryHdlcRxStatus;
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

    if (entryHdlcRxStatus == HDLC_INIT)
    {
        pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
        pT30->WasTimeout = FALSE;

        if (pT30->Timer_3s_Counter != 0)
        {
            pT30->Timer_3s_Counter = 0; TRACE0("T30: 3s reset");
        }

        if (isCmd)
        {
            pT30->Timer_T2_Counter = 1; TRACE0("T30: T2 starts");
        }
        else
        {
            pT30->Timer_T4_Counter = 1; TRACE0("T30: T4 starts");
        }

        if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
        {
            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            pT30->hdlcRxStatus = HDLC_FLAG_RXED;
        }
        else
        {
#if SUPPORT_V34FAX

            if (!isV34Selected)
#endif
            {
                T30_SetCommand(pT30ToDce, T30_DCE_FRH);
                pT30ToDce->tempParm[0] = MOD_V21_300;
            }

            pT30->hdlcRxStatus = HDLC_NO_DATA;
        }
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        prevHdlcRxStatus = pT30->hdlcRxStatus;
        TRACE1("T30: hdlcRxStatus %d", prevHdlcRxStatus);
    }

    if (pT30->hdlcRxStatus == HDLC_NO_DATA)
    {
        if (!isCmd && pT30->Timer_T4_Counter > TIMER_T4_LENGTH)
        {
            pT30->Timer_T4_Counter = 0; TRACE0("T30: T4 elapsed");
            pT30->WasTimeout = TRUE;
            pT30->hdlcRxStatus = HDLC_RX_ERROR;
        }

#if SUPPORT_V34FAX
        else if (isV34Selected)
        {
            pT30->hdlcRxStatus = HDLC_FLAG_RXED;
        }

#endif
        else
        {
            switch (pDceToT30->resultCodeRsp)
            {
                case RESULT_CODE_NULL:
                    break;
                case RESULT_CODE_CONNECT:
                    pT30->hdlcRxStatus = HDLC_FLAG_RXED;
                    break;
                case RESULT_CODE_NOCARRIER:
                    pT30->hdlcRxStatus = HDLC_RX_ERROR;

                    if (isCmd)
                    {
                        pT30->WasTimeout = TRUE; // fixing bug 5
                    }

                    break;
                default:
                    TRACE0("T30: ERROR. pDceToT30->resultCodeRsp");
                    break;
            }

            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
        }

        hdlcRxStatusRet = HDLC_NO_DATA;
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        prevHdlcRxStatus = pT30->hdlcRxStatus;
        TRACE1("T30: hdlcRxStatus %d", prevHdlcRxStatus);
    }

    if (pT30->hdlcRxStatus == HDLC_FLAG_RXED)
    {
#if SUPPORT_V34FAX

        if (!isV34Selected)
#endif
        {
            if (isCmd && pT30->Timer_T2_Counter != 0)
            {
                pT30->Timer_T2_Counter = 0; TRACE0("T30: T2 reset");
            }
            else if (!isCmd && pT30->Timer_T4_Counter != 0)
            {
                pT30->Timer_T4_Counter = 0; TRACE0("T30: T4 reset");
            }

            if (pT30->Timer_3s_Counter == 0)
            {
                pT30->Timer_3s_Counter = 1; TRACE0("T30: 3s starts"); // Start 3 s timer
            }
            else if (pT30->Timer_3s_Counter > TIMER_T3S_LENGTH)
            {
                pT30->Timer_3s_Counter = 0; TRACE0("T30: 3s elapsed");
                // fixing bug 33, the following 5 lines are commented out
                //if (isCmd)
                //{
                //    pT30->hdlcRxStatus = HDLC_RX_ERROR;
                //}
                //else
                {
                    pT30->hdlcRxStatus = HDLC_3S_TIMEOUT;
                }
            }
        }

#if SUPPORT_V34FAX
        else if (isCmd)
        {
        }
        else
        {
            if (pT30->Timer_T4_Counter > TIMER_T4_LENGTH)
            {
                pT30->Timer_T4_Counter = 0; TRACE0("T30: T4 elapsed");
                pT30->hdlcRxStatus = HDLC_RX_ERROR;
                pT30->WasTimeout = TRUE;
            }
        }

#endif

        switch (pDceToT30->tempDataType)
        {
            case DCE_T30_DATA_NULL:

                switch (pDceToT30->resultCodeRsp)
                {
                    case RESULT_CODE_NULL:
                        break;
                    case RESULT_CODE_NOCARRIER:
                        pT30->hdlcRxStatus = HDLC_RX_ERROR;

                        if (isCmd)
                        {
                            pT30->WasTimeout = TRUE; // fixing bug 5
                        }

                        break;
                    default:
                        TRACE0("T30: ERROR. pDceToT30->resultCodeRsp");
                        break;
                }

                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                break;
            case DCE_T30_HDLC_DATA:
            case DCE_T30_HDLC_END:
                pT30->hdlcRxStatus = HDLC_IN_PROCESS;
                break;
            case DCE_T30_HDLC_BAD:
                pT30->hdlcRxStatus = HDLC_CRC_BAD;
                break;
        }

        hdlcRxStatusRet = HDLC_FLAG_RXED;
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        prevHdlcRxStatus = pT30->hdlcRxStatus;
        TRACE1("T30: hdlcRxStatus %d", prevHdlcRxStatus);
    }

    if (pT30->hdlcRxStatus ==  HDLC_IN_PROCESS)
    {
        if (pDceToT30->tempDataLen != 0)
        {
            if (pT30->HDLCLen + pDceToT30->tempDataLen <= sizeof(pT30->HDLC_RX_Buf))
            {
                memcpy(&pT30->HDLC_RX_Buf[pT30->HDLCLen], pDceToT30->tempData, pDceToT30->tempDataLen);
                pT30->HDLCLen += (UBYTE)pDceToT30->tempDataLen;
            }
            else
            {
                TRACE0("T30: WARNING. HDLC frame too long");
            }
        }

        switch (pDceToT30->tempDataType)
        {
            case DCE_T30_DATA_NULL:

                switch (pDceToT30->resultCodeRsp)
                {
                    case RESULT_CODE_NULL:
                        break;
                    case RESULT_CODE_NOCARRIER:
                        pT30->hdlcRxStatus = HDLC_RX_ERROR;
                        break;
                    default:
                        TRACE0("T30: ERROR. pDceToT30->resultCodeRsp");
                        break;
                }

                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                break;
            case DCE_T30_HDLC_DATA:
                break;
            case DCE_T30_HDLC_END:

                if (pT30->HDLCLen >= 3)
                {
                    UBYTE LastFrame_Flag;

#if SUPPORT_T38EP
                    T30_LogHdlc(pT30->HDLC_RX_Buf, pT30->HDLCLen, FALSE, TRUE);
#else
                    T30_LogHdlc(pT30->HDLC_RX_Buf, pT30->HDLCLen, FALSE, FALSE);
#endif
                    pT30->hdlcTypeRx = HDLC_GetT30Type(&pT30->HDLC_RX_Buf[2]);
                    LastFrame_Flag = pT30->HDLC_RX_Buf[1];

                    if (LastFrame_Flag == HDLC_CONTR_FIELD_LASTFRM)
                    {
                        if (pT30->hdlcTypeRx == HDLC_TYPE_CRP)
                        {
                            //  deal with CRP In "Response REC ? "like CRC
                            pT30->hdlcRxStatus = HDLC_CRC_BAD;
                            T30_StopHDLCReceiving(pT30);
                        }
                        else
                        {
                            pT30->hdlcRxStatus = HDLC_CRC_OK;
                        }
                    }
                    else if (LastFrame_Flag == HDLC_CONTR_FIELD_NOTLASTFRM)
                    {
                        T30_ProcessOptionalFrame(pT30, pT30->hdlcTypeRx);
                        pT30->Timer_3s_Counter = 0; TRACE0("T30: 3s reset");
                        pT30->hdlcRxStatus = HDLC_OPTFRM_RXED;
                    }
                    else
                    {
                        pT30->hdlcRxStatus = HDLC_RX_ERROR;
                        TRACE1("T30: ERROR. Invalid Control Field 0x%x", LastFrame_Flag);
                    }
                }

                break;
            case DCE_T30_HDLC_BAD:
                pT30->hdlcRxStatus = HDLC_CRC_BAD;
                break;
        }

        pDceToT30->tempDataLen = 0;

        if (isCmd && pT30->Timer_T2_Counter != 0)
        {
            pT30->Timer_T2_Counter = 0; TRACE0("T30: T2 reset");
        }
        else if (!isCmd && pT30->Timer_T4_Counter != 0)
        {
            pT30->Timer_T4_Counter = 0; TRACE0("T30: T4 reset");
        }

        if (pT30->Timer_3s_Counter == 0 && pT30->hdlcRxStatus ==  HDLC_IN_PROCESS)
        {
            pT30->Timer_3s_Counter = 1; TRACE0("T30: 3s starts"); // Starts
        }
        else if (pT30->Timer_3s_Counter > TIMER_T3S_LENGTH)
        {
            pT30->Timer_3s_Counter = 0; TRACE0("T30: 3s elapsed");
            // fixing bug 33, the following 5 lines are commented out
            //if (isCmd)
            //{
            //    pT30->hdlcRxStatus = HDLC_RX_ERROR;
            //}
            //else
            {
                pT30->hdlcRxStatus = HDLC_3S_TIMEOUT;
            }
        }

        hdlcRxStatusRet = HDLC_IN_PROCESS;
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        prevHdlcRxStatus = pT30->hdlcRxStatus;
        TRACE1("T30: hdlcRxStatus %d", prevHdlcRxStatus);
    }

    if (pT30->hdlcRxStatus == HDLC_OPTFRM_RXED)
    {
#if SUPPORT_V34FAX

        if (isV34Selected)
        {
            pT30->hdlcRxStatus = HDLC_NO_DATA;
        }
        else
#endif
        {
            if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;

                pT30->hdlcRxStatus = HDLC_NO_DATA;

                T30_SetCommand(pT30ToDce, T30_DCE_FRH);
                pT30ToDce->tempParm[0] = MOD_V21_300;
            }
        }

        hdlcRxStatusRet = HDLC_OPTFRM_RXED;
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        prevHdlcRxStatus = pT30->hdlcRxStatus;
        TRACE1("T30: hdlcRxStatus %d", prevHdlcRxStatus);
    }

    if (pT30->hdlcRxStatus == HDLC_CRC_OK)
    {
        if (isCmd && pT30->Timer_T2_Counter != 0)
        {
            pT30->Timer_T2_Counter = 0; TRACE0("T30: T2 reset");
        }
        else if (!isCmd && pT30->Timer_T4_Counter != 0)
        {
            pT30->Timer_T4_Counter = 0; TRACE0("T30: T4 reset");
        }

        if (pT30->Timer_3s_Counter != 0)
        {
            pT30->Timer_3s_Counter = 0; TRACE0("T30: 3s reset");
        }

        pT30->rxErrorTimes = 0;

        if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER
#if SUPPORT_V34FAX
            || isV34Selected
#endif
           )
        {
            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            pT30->hdlcRxStatus = HDLC_LASTFRM_RXED;
        }
        else if (entryHdlcRxStatus != HDLC_CRC_OK)
        {
            // issue only once
            T30_SetCommand(pT30ToDce, T30_DCE_FRH);
            pT30ToDce->tempParm[0] = MOD_V21_300;
        }

        hdlcRxStatusRet = HDLC_CRC_OK;
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        prevHdlcRxStatus = pT30->hdlcRxStatus;
        TRACE1("T30: hdlcRxStatus %d", prevHdlcRxStatus);
    }

    if (pT30->hdlcRxStatus == HDLC_LASTFRM_RXED)
    {
        if (pT30->hdlcTypeRx == HDLC_TYPE_DCN)
        {
            UBYTE hangupCode;

            if (pT30->pfT30Vec == T30_PhaseA_Call)
            {
                hangupCode = HANGUP_COMREC_ERROR_PHASE_B_XMIT;
            }
            else if (pT30->pfT30Vec == T30_PhaseB_GetConf)
            {
                hangupCode = HANGUP_RSPREC_ERROR_PHASE_B_XMIT;
            }
            else if (pT30->pfT30Vec == T30_PhaseE_Ans)
            {
                hangupCode = HANGUP_NORMAL;
            }
            else if (pT30->pfT30Vec == T30_PhaseD_GetPE_NoECM
#if SUPPORT_ECM_MODE
                     || pT30->pfT30Vec == T30_PhaseD_GetPE_ECM
#endif
                    )
            {
                hangupCode = HANGUP_INVALID_CMD_PHASE_D_RCV;
            }
            else if (pT30->pfT30Vec == T30_PhaseB_GetDCS)
            {
                hangupCode = (UBYTE)((isCmd) ? HANGUP_COMREC_ERROR_PHASE_B_RCV : HANGUP_RSPREC_ERROR_PHASE_B_RCV);
            }
            else if (pT30->pfT30Vec == T30_PhaseC_GetConf_NoECM
#if SUPPORT_ECM_MODE
                     || pT30->pfT30Vec == T30_PhaseC_GetConf_ECM
#endif
                    )
            {
                hangupCode = HANGUP_RSPREC_ERROR_PHASE_D_XMIT;
            }
            else
            {
                hangupCode = HANGUP_NORMAL;
                TRACE0("T30: ERROR. pT30->pfT30Vec");
            }

            T30_GotoPointB(pT30, hangupCode);
        }

        pT30->hdlcRxStatus = HDLC_INIT;
        hdlcRxStatusRet = HDLC_LASTFRM_RXED;
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        prevHdlcRxStatus = pT30->hdlcRxStatus;
        TRACE1("T30: hdlcRxStatus %d", prevHdlcRxStatus);
    }

    if (pT30->hdlcRxStatus == HDLC_CRC_BAD)
    {
        if (pDceToT30->resultCodeRsp == RESULT_CODE_ERROR ||
            pDceToT30->resultCodeRsp == RESULT_CODE_OK) // CRP
        {
            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            T30_SetCommand(pT30ToDce, T30_DCE_FRH);
            pT30ToDce->tempParm[0] = MOD_V21_300;

            if (pT30->Timer_3s_Counter != 0) // this means the end of a frame
            {
                pT30->Timer_3s_Counter = 0; TRACE0(" T30: 3s reset");
            }
        }
        else if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER)
        {
            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            pT30->hdlcRxStatus = HDLC_RX_ERROR;
            pT30->txStage = LS_TX_STAGE_PREAMBLE; // fixing issue 162
            pT30->Timer_3s_Counter = 0; TRACE0(" T30: 3s reset");

            if (pT30->Timer_3s_Counter != 0) // this means the end of a frame
            {
                pT30->Timer_3s_Counter = 0; TRACE0(" T30: 3s reset");
            }
        }
        else if (pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT) // fixing issue 35, need to handle possible CONNECT
        {
            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;

            if (pT30->Timer_3s_Counter == 0) // this means the start of another frame, just ignore it
            {
                pT30->Timer_3s_Counter = 1; TRACE0(" T30: 3s starts");
            }
        }

        // discard any data received
        pDceToT30->tempDataLen = 0;

        if (isCmd && pT30->Timer_T2_Counter != 0)
        {
            pT30->Timer_T2_Counter = 0; TRACE0("T30: T2 reset");
        }
        else if (!isCmd && pT30->Timer_T4_Counter != 0)
        {
            pT30->Timer_T4_Counter = 0; TRACE0("T30: T4 reset");
        }

#if SUPPORT_V34FAX

        if (isV34Selected) // fixing issue 130
        {
            pT30->hdlcRxStatus = HDLC_RX_ERROR;
            pT30->txStage = LS_TX_STAGE_DATA; // fixing issue 131
        }
        else
#endif
            if (pT30->Timer_3s_Counter > TIMER_T3S_LENGTH)
            {
                pT30->Timer_3s_Counter = 0;

                if (isCmd)
                {
                    UBYTE hangupCode;

                    TRACE0("T30: 3s elapsed, disconnect the line.");

                    if (pT30->pfT30Vec == T30_PhaseA_Call)
                    {
                        hangupCode = HANGUP_COMREC_ERROR_PHASE_B_XMIT;
                    }
                    else if (pT30->pfT30Vec == T30_PhaseB_GetConf)
                    {
                        hangupCode = HANGUP_RSPREC_ERROR_PHASE_B_XMIT;
                    }
                    else if (pT30->pfT30Vec == T30_PhaseE_Ans)
                    {
                        hangupCode = HANGUP_NORMAL;
                    }
                    else if (pT30->pfT30Vec == T30_PhaseD_GetPE_NoECM
#if SUPPORT_ECM_MODE
                             || pT30->pfT30Vec == T30_PhaseD_GetPE_ECM
#endif
                            )
                    {
                        hangupCode = HANGUP_INVALID_CMD_PHASE_D_RCV;
                    }
                    else if (pT30->pfT30Vec == T30_PhaseB_GetDCS)
                    {
                        hangupCode = HANGUP_COMREC_ERROR_PHASE_B_RCV;
                    }
                    else if (pT30->pfT30Vec == T30_PhaseC_GetConf_NoECM
#if SUPPORT_ECM_MODE
                             || pT30->pfT30Vec == T30_PhaseC_GetConf_ECM
#endif
                            )
                    {
                        hangupCode = HANGUP_RSPREC_ERROR_PHASE_D_XMIT;
                    }
                    else
                    {
                        hangupCode = HANGUP_NORMAL;
                        TRACE0("T30: ERROR. pT30->pfT30Vec");
                    }

                    T30_GotoPointB(pT30, hangupCode);
                }
                else
                {
                    pT30->hdlcRxStatus = HDLC_3S_TIMEOUT;
                    TRACE0("T30: 3s elapsed, send DCN and disconnect the line.");
                }
            }

        hdlcRxStatusRet = HDLC_CRC_BAD;
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        prevHdlcRxStatus = pT30->hdlcRxStatus;
        TRACE1("T30: hdlcRxStatus %d", prevHdlcRxStatus);
    }

    if (pT30->hdlcRxStatus == HDLC_RX_ERROR)
    {
        if (pT30->Timer_3s_Counter != 0)
        {
            pT30->Timer_3s_Counter = 0; TRACE0("T30: 3s reset");
        }

        if (isCmd)
        {
            if (pT30->WasTimeout)
            {
                pT30->hdlcRxStatus = HDLC_INIT;
                T30_StopHDLCReceiving(pT30);
            }

#if T30_RESPOND_CRP
            else
            {
                // issue CRP only if it's not NO CARRIER before any HDLC data
                if (T30_RespondCRP(pT30) == 1)
                {
                    // already done
                    pT30->hdlcRxStatus = HDLC_INIT;
                    T30_StopHDLCReceiving(pT30);
                }
            }

#endif
        }
        else
        {
            if (pT30->WasTimeout) // fixing issue 130
            {
#if SUPPORT_V34FAX
                if (isV34Selected)
                {
                    // issue <CAN> only if it's T4 timeout. Do Not if it's CRC/CRP/carrier loss
                    pT30ToDce->flowCntlCmd = CAN_STOP; TRACE0("T30: <CAN> to DCE"); // fixing issue 152
                }
                else
#endif
                // fixing bug 24, need to wait for the OK/NO CARRIER before proceeding
                if (entryHdlcRxStatus == HDLC_RX_ERROR)
                {
                    if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER ||
                        pDceToT30->resultCodeRsp == RESULT_CODE_OK)
                    {
                        pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                    }
                    else
                    {
                        return HDLC_CRC_BAD;
                    }
                }
                else
                {
                    // issue <CAN> only if it's T4 timeout. Do Not if it's CRC/CRP/carrier loss
                    pT30ToDce->flowCntlCmd = CAN_STOP; TRACE0("T30: <CAN> to DCE"); // fixing issue 152
                    pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                    return HDLC_CRC_BAD;
                }

                // fixing bug 24
            }

            // Stop receiving RSP
            pT30->hdlcRxStatus = HDLC_INIT;
            T30_StopHDLCReceiving(pT30);
        }

        hdlcRxStatusRet = HDLC_RX_ERROR;
#if SUPPORT_V34FAX

        if (isV34Selected && (entryHdlcRxStatus != HDLC_RX_ERROR))
        {
            // fixing issue 197, need to initiate CC retrain. Issue only once
            // later it's commented out because GAO V.34 fax has poor support of retrain
            //T30_SetCommand(pT30ToDce, T30_DCE_DLE_RTNC);
        }

#endif
    }

    if (prevHdlcRxStatus != pT30->hdlcRxStatus)
    {
        TRACE1("T30: hdlcRxStatus %d", pT30->hdlcRxStatus);
    }

    if (pT30->hdlcRxStatus == HDLC_3S_TIMEOUT)
    {
        // fixing bug 24, wait for OK/NO CARRIER to <CAN>
        if (entryHdlcRxStatus != HDLC_3S_TIMEOUT)
        {
            pT30ToDce->flowCntlCmd = CAN_STOP; TRACE0("T30: <CAN> to DCE"); // fixing issue 152
#if SUPPORT_V34FAX

            if (isV34Selected)
            {
                T30_SetCommand(pT30ToDce, T30_DCE_DLE_EOT);
            }
            else
#endif
            {
                return HDLC_CRC_BAD;
            }
        }
        else
        {
            if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER ||
                pDceToT30->resultCodeRsp == RESULT_CODE_OK)
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            }
            else
            {
                return HDLC_CRC_BAD;
            }
        }

        pT30->hdlcRxStatus = HDLC_INIT;
        hdlcRxStatusRet = HDLC_3S_TIMEOUT;
    }

    pDceToT30->tempDataType = DCE_T30_DATA_NULL;
    return hdlcRxStatusRet;
}

void T30_StopHDLCSending(T30Struct *pT30)
{
    UWORD i;

    for (i = 0; i < T30FRAMELEN; i++)
    {
        pT30->HDLC_TX_Buf[i] = 0;
    }

    pT30->isHdlcMade = FALSE;
    pT30->txStage = LS_TX_STAGE_SILENCE;
    pT30->frmStage = LS_FRAME_OPT1;
}

void T30_StopHDLCReceiving(T30Struct *pT30)
{
    UBYTE i;

    pT30->HDLCLen = 0;

    for (i = 0; i < T30FRAMELEN; i++)
    {
        pT30->HDLC_RX_Buf[i] = 0;    // should be init carefully on PPR/PPS/DIS/DTC/DCS/CTC because they have FIF bytes
    }
}

void T30_GotoPointB(T30Struct *pT30, UBYTE hangupCode)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

    TRACE1("T30: Point B with hangup code 0x%02x", hangupCode);
    pT30ToDte->hangupCode = hangupCode;
    pT30->hdlcRxStatus = HDLC_INIT;
#if SUPPORT_V34FAX

    if (isV34Selected && !pDceToT30->isDleEot)
    {
        if (pT30->isTransmit || pDteToT30->isDiscCmd || pT30->wasLastTry)
        {
            T30_SetCommand(pT30ToDce, T30_DCE_DLE_EOT);
        }
    }
    else
#endif
    {
        T30_SetCommand(pT30ToDce, T30_DCE_ATH);
        pDceToT30->resultCodeRsp = RESULT_CODE_NULL; // fixing bug 13, need to clear the result code response
        pT30ToDce->tempParm[0] = 0;
    }

    pT30->pfT30Vec = T30_PhaseHangUp;
}

void T30_GotoPointC(T30Struct *pT30, UBYTE hangupCode)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

    TRACE1("T30: Point C with hangup code 0x%02x", hangupCode);
    pT30ToDte->hangupCode = hangupCode;

    // fixing issue 180
    if (pT30->txStage == LS_TX_STAGE_CRC_OK)
    {
        // from command Tx to DCN Tx, FTH has already been issued, and CONNECT has been received.
    }
    else
    {
#if SUPPORT_V34FAX
        pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
        pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
    }

    pT30->pfT30Vec = T30_PhaseE_Call;
}

UBYTE T30_ConvertSpdToT31Mod(UBYTE T30Speed, UBYTE isLongTraining)
{
    switch (T30Speed)
    {
        case DCS_V17_14400:
            return (isLongTraining ? MOD_V17_L_14400 : MOD_V17_S_14400);
        case DCS_V17_12000:
            return (isLongTraining ? MOD_V17_L_12000 : MOD_V17_S_12000);
        case DCS_V17_9600:
            return (isLongTraining ? MOD_V17_L_9600 : MOD_V17_S_9600);
        case DCS_V17_7200:
            return (isLongTraining ? MOD_V17_L_7200 : MOD_V17_S_7200);
        case DCS_V29_9600:
            return MOD_V29_9600;
        case DCS_V29_7200:
            return MOD_V29_7200;
        case DCS_V27_4800:
            return MOD_V27TER_4800;
        default:
            return MOD_V27TER_2400;
    }
}

#if TRACEON
static void T30_DecodeHdlc(CHAR *command, UBYTE *msg, UWORD len)
{
    UWORD uTemp = ((UWORD)msg[2]) << 8;
    UWORD uTemp2 = uTemp & 0xf000;
    UWORD i;
    CHAR buf[10];
    CHAR logBuff[65];
    UBYTE isString = FALSE;

    switch (uTemp2)
    {
        case 0x6000:
        case 0x8000:
        case 0x0000: break;
        default    : uTemp |= 0x8000;
    }

    switch (uTemp)
    {
        case HDLC_TYPE_DIS    : strcpy(buf, "DIS"); break;
        case HDLC_TYPE_CSI    : strcpy(buf, "CSI"); isString = TRUE; break;
        case HDLC_TYPE_NSF    : strcpy(buf, "NSF"); break;
        case HDLC_TYPE_DTC    : strcpy(buf, "DTC"); break;
        case HDLC_TYPE_CIG    : strcpy(buf, "CIG"); isString = TRUE; break;
        case HDLC_TYPE_NSC    : strcpy(buf, "NSC"); break;
        case HDLC_TYPE_PWD    : strcpy(buf, "PWD"); break;
        case HDLC_TYPE_SEP    : strcpy(buf, "SEP"); break;
        case HDLC_TYPE_PSA    : strcpy(buf, "PSA"); break;
        case HDLC_TYPE_CIA    : strcpy(buf, "CIA"); break;
        case HDLC_TYPE_ISP    : strcpy(buf, "ISP"); break;
        case HDLC_TYPE_T4_FCD : strcpy(buf, "FCD"); break;
        case HDLC_TYPE_T4_RCP : strcpy(buf, "RCP"); break;
        case HDLC_TYPE_DCS    : strcpy(buf, "DCS"); break;
        case HDLC_TYPE_TSI    : strcpy(buf, "TSI"); isString = TRUE; break;
        case HDLC_TYPE_SUB    : strcpy(buf, "SUB"); isString = TRUE; break;
        case HDLC_TYPE_NSS    : strcpy(buf, "NSS"); break;
        case HDLC_TYPE_SID    : strcpy(buf, "SID"); break;
        case HDLC_TYPE_TSA    : strcpy(buf, "TSA"); break;
        case HDLC_TYPE_IRA    : strcpy(buf, "IRA"); break;
        case HDLC_TYPE_T4_CTC : strcpy(buf, "CTC"); break;
        case HDLC_TYPE_CFR    : strcpy(buf, "CFR"); break;
        case HDLC_TYPE_FTT    : strcpy(buf, "FTT"); break;
        case HDLC_TYPE_T4_CTR : strcpy(buf, "CTR"); break;
        case HDLC_TYPE_CSA    : strcpy(buf, "CSA"); break;
        case HDLC_TYPE_EOM    : strcpy(buf, "EOM"); break;
        case HDLC_TYPE_MPS    : strcpy(buf, "MPS"); break;
        case HDLC_TYPE_EOP    : strcpy(buf, "EOP"); break;
        case HDLC_TYPE_T4_RR  : strcpy(buf, "RR"); break;
        case HDLC_TYPE_EOS    : strcpy(buf, "EOS"); break;
        case HDLC_TYPE_PRI_EOM: strcpy(buf, "EOM"); break;
        case HDLC_TYPE_PRI_MPS: strcpy(buf, "MPS"); break;
        case HDLC_TYPE_PRI_EOP: strcpy(buf, "EOP"); break;
        case HDLC_TYPE_MCF    : strcpy(buf, "MCF"); break;
        case HDLC_TYPE_RTN    : strcpy(buf, "RTN"); break;
        case HDLC_TYPE_RTP    : strcpy(buf, "RTP"); break;
        case HDLC_TYPE_PIN    : strcpy(buf, "PIN"); break;
        case HDLC_TYPE_PIP    : strcpy(buf, "PIP"); break;
        case HDLC_TYPE_FDM    : strcpy(buf, "FDM"); break;
        case HDLC_TYPE_T4_PPR : strcpy(buf, "PPR"); break;
        case HDLC_TYPE_T4_RNR : strcpy(buf, "RNR"); break;
        case HDLC_TYPE_T4_ERR : strcpy(buf, "ERR"); break;
        case HDLC_TYPE_DCN    : strcpy(buf, "DCN"); break;
        case HDLC_TYPE_CRP    : strcpy(buf, "CRP"); break;
        case HDLC_TYPE_FNV    : strcpy(buf, "FNV"); break;
        case HDLC_TYPE_TNR    : strcpy(buf, "TNR"); break;
        case HDLC_TYPE_TR     : strcpy(buf, "TR"); break;
        case PPS_NULL         :
            strcpy(buf, "PPS");

            switch (msg[3])
            {
#if SUPPORT_ECM_MODE
                case FCF2_NULL: strcat(buf, "_NULL"); break;
                case FCF2_EOM: strcat(buf, "_EOM"); break;
                case FCF2_MPS: strcat(buf, "_MPS"); break;
                case FCF2_EOP: strcat(buf, "_EOP"); break;
                case FCF2_EOS: strcat(buf, "_EOS"); break;
                case FCF2_PRI_EOM: strcat(buf, "_PRI_EOM"); break;
                case FCF2_PRI_MPS: strcat(buf, "_PRI_MPS"); break;
                case FCF2_PRI_EOP: strcat(buf, "_PRI_EOP"); break;
#endif
                default: strcat(buf, "_unknown"); break;
            }

            break;
        case EOR_NULL         :
            strcpy(buf, "EOR");

            switch (msg[3])
            {
#if SUPPORT_ECM_MODE
                case FCF2_NULL: strcat(buf, "_NULL"); break;
                case FCF2_EOM: strcat(buf, "_EOM"); break;
                case FCF2_MPS: strcat(buf, "_MPS"); break;
                case FCF2_EOP: strcat(buf, "_EOP"); break;
                case FCF2_EOS: strcat(buf, "_EOS"); break;
                case FCF2_PRI_EOM: strcat(buf, "_PRI_EOM"); break;
                case FCF2_PRI_MPS: strcat(buf, "_PRI_MPS"); break;
                case FCF2_PRI_EOP: strcat(buf, "_PRI_EOP"); break;
#endif
                default: strcat(buf, "_unknown");
            }

            break;
        default: strcpy(buf, "Unknown");
    }

    sprintf(logBuff, "%s%10s", command, buf);
    TRACE1("%s", logBuff);

    strcpy(logBuff, "T30: DATA: ");

    for (i = 0; i < len;)
    {
        if ((i & 0xf) == 0xf)
        {
            sprintf(buf, "%02x", msg[i++]);
            strcat(logBuff, buf);
            TRACE1("%s", logBuff);
            strcpy(logBuff, "           ");
        }
        else
        {
            sprintf(buf, "%02x ", msg[i++]);
            strcat(logBuff, buf);

            if (i == len)
            {
                TRACE1("%s", logBuff);
            }
        }
    }

    if (isString)
    {
        CHAR ch;
        strcpy(logBuff, " - ");

        for (i = len - 1; i > 2 ; i --)
        {
            ch = (CHAR)COMM_BitReversal(msg[i]);
            strncat(logBuff, &ch, 1);
        }

        TRACE1("%s", logBuff);
    }
}
#endif

void T30_LogHdlc(UBYTE *msg, UWORD len, UBYTE isTransmit, UBYTE isT38)
{
#if TRACEON
    CHAR temp[15];

    if (!msg || len < 3)
    {
        return;
    }

    if (msg[2] == T4_FCD || msg[2] == T4_RCP)
    {
        return;
    }

    if (isT38)
    {
        strcpy(temp, "T30: T38 ");
    }
    else
    {
        strcpy(temp, "T30: VXX ");
    }

    if (isTransmit)
    {
        strcat(temp, "TX: ");
    }
    else
    {
        strcat(temp, "RX: ");

        if (!isT38)
        {
            len -= 2;
        }
    }

    //pFskCallBackFunction(msg, len, isTransmit);

    T30_DecodeHdlc(temp, msg, len);
#endif
}
#endif
