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
#include "hdlcext.h"
#include "faxlinecnt.h"

#if SUPPORT_T30 && SUPPORT_ECM_MODE

static SWORD T30_Rx_From_DataPump_ECM(T30Struct *pT30)
{
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    HdlcStruct *pHDLC_RX = &pT30->T30HDLC;
    SWORD ByteReceived;
    UBYTE *p;
    UWORD space_left;
    UBYTE doNotDec;

#if SUPPORT_T38EP
    doNotDec = TRUE;
#elif SUPPORT_V34FAX
    doNotDec = pDceToT30->isV34Selected;
#else
    doNotDec = FALSE;
#endif

#if !SUPPORT_T38EP

    if ((pHDLC_RX->RawDataEnd > 1760) && (pHDLC_RX->FrameStart > 320))/* Need to be considered again */
    {
        HDLC_CleanRD(pHDLC_RX);
    }

#endif

    if (doNotDec)
    {
        space_left = HDLCBUFLEN - (pHDLC_RX->RawDataEnd >> 3) - 1;
    }
    else
    {
        space_left = MIN16(HDLCBUFLEN - (pHDLC_RX->RawDataEnd >> 3) - 1, 128);
    }

    /* get the last byte in raw data buffer      */
    p = pHDLC_RX->RawData + (pHDLC_RX->RawDataEnd >> 3);

    /* append data at the end of the raw data */
    //ByteReceived = T30_RxDataFromModem(pT30, p, space_left);
    if (pDceToT30->tempDataLen == 0)
    {
        if (pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER)
        {
            if (!pT30->T4ECM.isRcpExchanged)
            {
                pT30->T4ECM.isRcpExchanged = TRUE;
                TRACE0("T30: Carrier lost without RCP frames");
            }

            return -1;
        }

        return 0;
    }
    else if (space_left == 0)
    {
        // fixing issue 132
        TRACE0("T30: WARNING. Lots of garbage data");
#if !SUPPORT_T38EP
        HDLC_CleanRD(pHDLC_RX);
#endif
        p = pHDLC_RX->RawData;
        ByteReceived = pDceToT30->tempDataLen = 0; // fixing issue 172, discard everything received
    }
    else if (pDceToT30->tempDataLen > space_left)
    {
        ByteReceived = space_left;
    }
    else
    {
        ByteReceived = pDceToT30->tempDataLen;
    }

    if (doNotDec)
    {
        memcpy(p, pDceToT30->tempData, ByteReceived);
        pDceToT30->tempDataLen = 0;
    }
    else if (ByteReceived > 0)
    {
        UWORD i, j = 0;

        //TRACE("T30: Rx: ", 0);
        for (i = 0; i < ByteReceived; i++)
        {
            p[i] = COMM_BitReversal(pDceToT30->tempData[i]);
            //TRACE("%02x ", p[i]);
        }

        //TRACE0("");

        for (; i < pDceToT30->tempDataLen; i++, j++)
        {
            pDceToT30->tempData[j] = pDceToT30->tempData[i];
        }

        pDceToT30->tempDataLen = j;
    }

    pHDLC_RX->RawDataEnd += ByteReceived << 3;

    return ByteReceived;
}

static UBYTE T30_HDLC_Rx_ECM(T30Struct *pT30, UBYTE *pBuff, UWORD *Len)
{
    HdlcStruct *pHDLC_RX = &pT30->T30HDLC;
    UWORD BuffLen;
    UBYTE ret = HDLC_RX_NULL;
    UBYTE doNotDec;

#if SUPPORT_T38EP
    doNotDec = TRUE;
#elif SUPPORT_V34FAX
    doNotDec = pT30->pDceToT30->isV34Selected;
#else
    doNotDec = FALSE;
#endif

    if (doNotDec)
    {
        DceToT30Interface *pDceToT30 = pT30->pDceToT30;
        *Len = BuffLen = pHDLC_RX->RawDataEnd >> 3;

        if (BuffLen > 0)
        {
            memcpy(pBuff, pHDLC_RX->RawData, BuffLen);
            pHDLC_RX->RawDataEnd = 0;
        }

        switch (pDceToT30->tempDataType)
        {
            case DCE_T30_HDLC_DATA:
                pT30->hdlcRxStatus = HDLC_IN_PROCESS;
                ret = HDLC_RX_DATA | HDLC_RX_FLAG;
                break;
            case DCE_T30_HDLC_END:
                pT30->hdlcRxStatus = HDLC_IN_PROCESS;
                ret = HDLC_RX_FCS_OK | HDLC_RX_DATA | HDLC_RX_FLAG;
                break;
            case DCE_T30_HDLC_BAD:
                pT30->hdlcRxStatus = HDLC_IN_PROCESS;
                ret = HDLC_RX_FCS_BAD;
                break;
            case DCE_T30_DATA_NULL:

                if (pT30->hdlcRxStatus == HDLC_IN_PROCESS) // fixing issue 156
                {
                    ret = HDLC_RX_DATA | HDLC_RX_FLAG;
                }

                break;
        }

        pDceToT30->tempDataType = DCE_T30_DATA_NULL;
    }

#if !SUPPORT_T38EP
    else
    {
        HDLC_IdentifyFrame(pHDLC_RX, FALSE);

        switch (pHDLC_RX->Status)
        {
            case HDLC_INITIALIZED:
                ret = HDLC_RX_NULL;
                break;
            case HDLC_FRAMESTART_FOUND:

                ret = HDLC_RX_FLAG;

                break;

            case HDLC_DATA_FOUND:

                ret = HDLC_DataFound(pHDLC_RX, pBuff, Len);

                break;

            case HDLC_FRAMEEND_FOUND:

                if (HDLC_Decode(pHDLC_RX))
                {
                    if (HDLC_CheckCRC(pHDLC_RX))
                    {
                        int i;

                        BuffLen = pHDLC_RX->WorkingBuffEnd >> 3;

                        for (i = 0; i < BuffLen; i++)
                        {
                            pBuff[i] = pHDLC_RX->WorkingBuff[i];
                        }

                        *Len = BuffLen & 0x1FF;

                        ret = HDLC_RX_FCS_OK | HDLC_RX_DATA | HDLC_RX_FLAG;
                    }
                    else
                    {
                        ret = HDLC_RX_FCS_BAD;
                    }
                }
                else
                {
                    ret = HDLC_RX_FCS_BAD;
                }

                HDLC_CleanRD(pHDLC_RX);

                HDLC_ResetWB(pHDLC_RX);

                break;
        }
    }

#endif
    return ret;
}

static UWORD T30_GetHDLCfromRemote_ECM(T30Struct *pT30)
{
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UWORD Len, i;
    UBYTE re;
    UBYTE pBuf[512];
    UBYTE *ch_add, LastFrame_Flag;
    UWORD ReVal = T30MESNULL;
    SWORD ByteReceived = T30_Rx_From_DataPump_ECM(pT30);

    if (ByteReceived == -1)
    {
        re = HDLC_RX_NO_CARRIER;/* no carrier */
        Len = 0;
    }
    else
    {
        re = T30_HDLC_Rx_ECM(pT30, pBuf, &Len);
    }

    if (pT30->hdlcRxStatus == HDLC_INIT)
    {
        if (re & HDLC_RX_FLAG)
        {
            pT30->hdlcRxStatus = HDLC_FLAG_RXED;
            pT30->hdlcTypeRx = HDLC_TYPE_INIT;
        }
        else if (re == HDLC_RX_NO_CARRIER)
        {
            pT30->hdlcRxStatus = HDLC_RX_ERROR;
        }
    }

    if (pT30->hdlcRxStatus == HDLC_FLAG_RXED)
    {
        if (pT30->Timer_T2_Counter != 0)
        {
            pT30->Timer_T2_Counter = 0; TRACE0("T30: HS T2 reset");
        }

        if (pT30->Timer_T1_Counter == 0)
        {
            pT30->Timer_T1_Counter = 1; TRACE0("T30: HS T1 starts");
        }
        else if (pT30->Timer_T1_Counter > TIMER_T1_LENGTH)
        {
            pT30->Timer_T1_Counter = 0; TRACE0("T30: HS T1 elapsed");
            pT30->wasLastTry = TRUE;
            T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_C_RCV); return T30TIMEOUT;
        }

        if ((re == HDLC_RX_FCS_BAD) || (re == HDLC_RX_NO_CARRIER))
        {
            pT30->hdlcRxStatus = HDLC_RX_ERROR;
        }
        else if (re & HDLC_RX_DATA)
        {
            pT30->hdlcRxStatus = HDLC_IN_PROCESS;
        }
    }

    if (pT30->hdlcRxStatus == HDLC_IN_PROCESS)
    {
        if (pT30->Timer_T1_Counter != 0)
        {
            pT30->Timer_T1_Counter = 0; TRACE0("T30: HS T1 reset");
        }

        if (re & HDLC_RX_DATA)
        {
            if ((re & 0x0C) == 0x00)
            {
                if ((pT4->leftover_len + Len) > (FRAMESIZE + 6))
                {
                    /* check buf full? */
                    pT30->hdlcRxStatus = HDLC_RX_ERROR;
                }
                else
                {
                    /* receiving HDLC data */
                    for (i = 0; i < Len; i++)
                    {
                        pT4->leftover_buf[pT4->leftover_len + i] = pBuf[i];
                    }

                    pT4->leftover_len += Len;
                }
            }
            else
            {
                /* HDLC data end */
                if ((re & HDLC_RX_FCS_OK) && (pT4->leftover_len + Len) <= (FRAMESIZE + 6)) /* check buf full? */
                {
                    for (i = 0; i < Len; i++)
                    {
                        pT4->leftover_buf[pT4->leftover_len + i] = pBuf[i];
                    }

                    pT4->leftover_len += Len;

                    ch_add = &pT4->leftover_buf[2];

                    pT30->hdlcTypeRx = HDLC_GetT30Type(ch_add);

                    LastFrame_Flag = pT4->leftover_buf[1];

                    if (LastFrame_Flag == HDLC_CONTR_FIELD_LASTFRM)
                    {
                        pT30->hdlcRxStatus = HDLC_RX_ERROR;
                    }
                    else if (LastFrame_Flag == HDLC_CONTR_FIELD_NOTLASTFRM)
                    {
                        pT30->Timer_T2_Counter = 1; TRACE0("T30: HS T2 starts");
                        pT30->hdlcRxStatus = HDLC_INIT;
                        ReVal = pT30->hdlcTypeRx;
                    }
                }
                else/* CRC error */
                {
                    pT30->hdlcRxStatus = HDLC_RX_ERROR;
                }
            }
        }
        else
        {
            pT30->hdlcRxStatus = HDLC_RX_ERROR;
        }
    }

    if (pT30->hdlcRxStatus == HDLC_RX_ERROR)
    {
        pT30->hdlcRxStatus = HDLC_INIT;
        /* have error, go back to receive HDLC again */
        ReVal = T30_FRAME_ERR;
    }

    return ReVal;
}

static void T30_SendImageDataBlock_ToDTE_ECM(T30Struct *pT30)
{
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UDWORD udTemp;
    UWORD len;

    if (pDteToT30->dataCmd == DC3_XOFF)
    {
        //pT30ToDte->tempDataLen = 0; // fixing bug 12, cannot zero the length because the remaining bytes haven't been received by DTE
        return;
    }

    udTemp = (((UDWORD)pT4->frmNumUploaded + 2) << pT4->frmSizeBitNum);

    len = (UWORD)MIN16((udTemp - pT4->udImageDataByteNum), sizeof(pT30ToDte->tempData));

    T30_UploadImgToDte(pT30, &pT4->pblock[pT4->udImageDataByteNum], len, TRUE); // fixing issue 137

    pT4->udImageDataByteNum += len;

    if (udTemp == pT4->udImageDataByteNum)
    {
        pT4->frmNumUploaded++;
        TRACE1("T30: ECM frame %d uploaded", pT4->frmNumUploaded);
    }
}

/* T30_SendImageData To DTE by UART for ECM */
static void T30_SendImageData_ToDTE_ECM(T30Struct *pT30)
{
    T4ECMstruc *pT4 = &pT30->T4ECM;

    if (ReadBit(pT4->status[(pT4->frmNumUploaded+1) >> 3], 7 - ((pT4->frmNumUploaded + 1) & 0x7)) == 0)
    {
        // the frame is received
        if (pT4->udImageDataByteNum < (((UDWORD)pT4->frmNumUploaded + 2) << pT4->frmSizeBitNum))
        {
            // haven't sent all the bytes
            T30_SendImageDataBlock_ToDTE_ECM(pT30);
        }
    }

    if (pT4->frmNumUploaded >= pT4->frmNumInBlock)
    {
        pT4->isBlkSentToDte = TRUE;

        if (!pT4->isPgSentToDte && // the page end is not sent
            pT4->FCF2 != FCF2_INIT && // not before the first PPS_any
            pT4->FCF2 != FCF2_NULL) // not PPS_NULL
        {
            T30_UploadPgEndToDte_ECM(pT30);
        }
    }
}

void T30_T4InitLeftoverBuf(T4ECMstruc *pT4)
{
    UWORD i;

    pT4->leftover_len = 0;

    for (i = 0; i < LEFTOVER_BUFSIZE; i++)
    {
        pT4->leftover_buf[i] = 0;
    }
}

/* [4]
** T30_PhaseC_ImageDataRx in ECM mode: (MODEM speed HIGH:T4)
** In this phase receive image data in FCD frames, until RCP comes.
*/
void T30_PhaseC_ImageDataRx_ECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    T4ECMstruc *pT4   = &pT30->T4ECM;
    UWORD ReVal = T30MESNULL, i;
    UBYTE ubFrameIdx;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_C_RCV); return;
    }
    else
#endif
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
#if SUPPORT_V34FAX

            if (!isV34Selected)
#endif
            {
                T30_SetCommand(pT30ToDce, T30_DCE_FRM);

                if (pT4->FCF1 == T4_CTC) // fixing issue 111
                {
                    pT30ToDce->tempParm[0] = T30_ConvertSpdToT31Mod(pT30->T30Speed, TRUE);
                }
                else
                {
                    pT30ToDce->tempParm[0] = T30_ConvertSpdToT31Mod(pT30->T30Speed, FALSE);
                }

                TRACE1("T30: Start Fax at %02x speed to receive Fax image with ECM", pT30->T30Speed);
            }

            pT30->Timer_T2_Counter = 1; TRACE0("T30: HS T2 starts");

            break;

        case HS_RX_STAGE_CONNECT:

            if ((pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
#if SUPPORT_V34FAX
                || (isV34Selected && (pDceToT30->selectedV34Chnl == V34_PRIMARY_CHNL))
#endif
               )
            {
                pT30->rxErrorTimes = 0;
                pT30->hdlcRxStatus = HDLC_INIT;
                HDLC_Init(&pT30->T30HDLC);

#if SUPPORT_V34FAX

                if (isV34Selected)
                {
                    pT30ToDte->faxParmStatus.bitRateBR = pDceToT30->priChnlBitRate - 1;

                    if ((pT4->PPR_count & 0x1) != 0x0)
                    {
                        pT30ToDce->initV34PrimRateMax = pDceToT30->priChnlBitRate - 1;
                        pT30ToDce->isReqRateReduction = TRUE; TRACE1("T30: Command <DLE><P%d><DLE><PPH>", pT30ToDce->initV34PrimRateMax * 24);
                    }
                    else
                    {
                        pT30ToDce->isReqRateReduction = FALSE;
                    }
                }

#endif

                /* do some initialization on T4Struc */
                if (pT4->PPR_count == 0) /* the first time */
                {
                    pT4->udImageDataByteNum = 0;

                    for (i = 0; i < BLOCKNUM_BYTESIZE; i++)
                    {
                        pT4->status[i] = 0xff;    /* receive all 256 bits frame */
                    }

                    T30_T4InitLeftoverBuf(pT4);

                    pT4->frmNumUploaded = -1;
                    pT4->frmNumInBlock = BLOCKNUM - 1; /* 256 frames */
                    pT4->areAllFrmRcved = FALSE;
                    pT4->isBlkSentToDte = FALSE;
                    pT4->isPgSentToDte = FALSE;

                    if (pT4->FCF2 != FCF2_NULL) // fixing issue 158
                    {
                        // init/mps/eop/eom
                        T30_SetResponse(pT30, RESULT_CODE_CONNECT);
                    }

                    pT4->FCF1 = 0;
                    pT4->FCF2 = FCF2_INIT; // fixing issue 158
                }

                pT4->isRcpExchanged = FALSE;

                TRACE0("T30: Start receive Image data with ECM mode");
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->rxStage = HS_RX_STAGE_DATA;
                pT30->ImageState = IMAGE_STATE_DATA;
            }
            else if (pDceToT30->resultCodeRsp == RESULT_CODE_ERROR)
            {
                pT30->Timer_T2_Counter = 0; TRACE0("T30: HS T2 reset");
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;

                if (pDceToT30->infoTxtRsp == INFO_TXT_FCERROR)
                {
                    pDceToT30->infoTxtRsp = INFO_TXT_NULL;

                    switch (pT30->hdlcTypeTx)
                    {
                        case HDLC_TYPE_T4_PPR:
                        case HDLC_TYPE_MCF:
                        case HDLC_TYPE_T4_RNR:
                        case HDLC_TYPE_T4_CTR:
                        case HDLC_TYPE_T4_ERR:
                            pT30->pfT30Vec = T30_PhaseD_GetPE_ECM;
                            break;
                        default:
                            pT30->pfT30Vec = T30_PhaseB_GetDCS; pT30->isDcsAsCmd = TRUE;
                    }

                    break;
                }
                else
                {
                    TRACE0("T30: ERROR. pDceToT30->infoTxtRsp");
                    T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_C_RCV); return;
                }
            }

            break;
        default:

            if (!pT4->isRcpExchanged)/* still in the middle of image receiving? */
            {
                /* receive HDLC image data */
                ReVal = T30_GetHDLCfromRemote_ECM(pT30);

                switch (ReVal)
                {
                    case HDLC_TYPE_T4_FCD:
                        ubFrameIdx = COMM_BitReversal(pT4->leftover_buf[3]);
                        TRACE1("T30: HDLC FCD received, frame index=%d", ubFrameIdx);

                        if (ubFrameIdx == 0)
                        {
                            UBYTE thresh = 68;
                            UBYTE doNotDec;

#if SUPPORT_T38EP
                            doNotDec = TRUE;
#elif SUPPORT_V34FAX
                            doNotDec = pDceToT30->isV34Selected;
#else
                            doNotDec = FALSE;
#endif

                            // fixing the issue that actual frame size is different from DCS
                            if (!doNotDec)
                            {
                                thresh += 2;
                            }

                            if (pT4->leftover_len > thresh)
                            {
                                pT30ToDte->ecmFrmSize = ECM_SIZE_256;
                                pT4->frmSizeBitNum = 8;
                            }
                            else if (pT4->frmSizeBitNum == 8)
                            {
                                pT30ToDte->ecmFrmSize = ECM_SIZE_64;
                                pT4->frmSizeBitNum = 6;
                            }
                        }

                        for (i = 0; i < (1 << pT4->frmSizeBitNum); i ++)
                        {
                            pT4->pblock[(ubFrameIdx << pT4->frmSizeBitNum) + i] = pT4->leftover_buf[4 + i];
                        }

                        ResetBit(pT4->status[ubFrameIdx >> 3], 7 - (ubFrameIdx & 0x7));

                        T30_T4InitLeftoverBuf(pT4);
                        break;
                    case HDLC_TYPE_T4_RCP:
                        pT4->isRcpExchanged = TRUE;
                        TRACE0("T30: HDLC RCP received");
                        T30_T4InitLeftoverBuf(pT4);
                        break;
                    case T30_FRAME_ERR:
                        T30_T4InitLeftoverBuf(pT4);
                        break;
                    case T30TIMEOUT:
                        return;
                }

#if SUPPORT_V34FAX

                if (isV34Selected && (pDceToT30->selectedV34Chnl == V34_CONTROL_CHNL)) /* Fax in V34 MPET stage, send confirmation */
                {
                    pT30->pfT30Vec = T30_PhaseD_GetPE_ECM;
                    pT30->hdlcTypeTx = HDLC_TYPE_INIT; // fixing issue 122. MCF should be removed from here
                    pT30->hdlcRxStatus = HDLC_INIT;
                    pT30->Timer_T2_Counter = 0; TRACE0("T30: HS T2 reset");
                    TRACE0("T30: Phase C MODEM carrier lost, we move to next phase");
                }

#endif

                /* while data available, pass it to DTE */
                if (!pT4->isBlkSentToDte)
                {
                    T30_SendImageData_ToDTE_ECM(pT30);
                }
            }
            else
            {
                /* wait until modem carrier lost detected, then move on to next phase */
                pDceToT30->tempDataLen = 0; // fixing bug 26, discard anything else after RCP frames is received

                if ((pDceToT30->resultCodeRsp == RESULT_CODE_NOCARRIER)
#if SUPPORT_V34FAX
                    || (isV34Selected && (pDceToT30->selectedV34Chnl == V34_CONTROL_CHNL))
#endif
                   )
                {
                    pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                    pT30->pfT30Vec = T30_PhaseD_GetPE_ECM;

                    pDceToT30->tempDataType = DCE_T30_DATA_NULL;
                    pT30->hdlcTypeTx = HDLC_TYPE_INIT; // fixing issue 122. MCF should be removed from here
                    pT30->hdlcRxStatus = HDLC_INIT;
                    pT30->Timer_T2_Counter = 0; TRACE0("T30: HS T2 reset");
                    TRACE0("T30: Phase C carrier lost, move to next phase");
                }
            }

            break;
    }

    if (pT30->Timer_T2_Counter > TIMER_T2_LENGTH)
    {
        if (pT30->rxErrorTimes < T30_MAX_REPEAT_TIMES
#if SUPPORT_V34FAX
            || isV34Selected
#endif
           )
        {
            pT30->Timer_T2_Counter = 1; TRACE0("T30: HS T2 elapsed. Retry");
            pT30->rxErrorTimes++;
        }
        else
        {
            pT30->rxErrorTimes = 0;
            pT30->Timer_T2_Counter = 0; TRACE0("T30: HS T2 elapsed multiple times, no image");
            pT30->wasLastTry = TRUE;
            T30_GotoPointB(pT30, HANGUP_T2_TIMEOUT_PG_NOT_RCVED); return;
        }
    }
}


/* [5]
** T30_PhaseD_GetPE in ECM mode: (MODEM speed LOW:T30)
** wait for PPS, EOR, RR from calling
*/
void T30_PhaseD_GetPE_ECM(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    //T30ToDceInterface *pT30ToDce = &pT30->t30ToDce;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UBYTE hdlcRxStatus;
    int i;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_RCV); return;
    }

    // fixing issue 169
    if (isV34Selected && pDceToT30->selectedV34Chnl == V34_PRIMARY_CHNL)
    {
        pT30->pfT30Vec = T30_PhaseC_ImageDataRx_ECM; pT30->rxStage = HS_RX_STAGE_FRM;
        return;
    }

#endif

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
            pT30->wasLastTry = TRUE;
            T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_RCV); return;
        }
    }

    /* while data available, pass it to DTE */
    if (!pT4->isBlkSentToDte)
    {
        T30_SendImageData_ToDTE_ECM(pT30);
    }

    switch (hdlcRxStatus)
    {
        case HDLC_LASTFRM_RXED:

            switch (pT30->hdlcTypeRx)
            {
                case PPS_NULL:
                case PPS_MPS:
                case PPS_EOP:
                case PPS_EOM:
                case PPS_PRI_EOM:
                case PPS_PRI_MPS:
                case PPS_PRI_EOP:
                    pT4->FCF1 = T4_PPS;
                    pT4->FCF2 = pT30->hdlcTypeRx & 0xF7;
                    pT30->hdlcTypeRx &= 0xFFF7;

                    if ((pT4->PPR_count == 0) && (pT30->hdlcTypeTx != HDLC_TYPE_MCF))//if (pT4->PPR_count == 0)
                    {
                        pT4->pgNum  = COMM_BitReversal(pT30->HDLC_RX_Buf[4]);
                        pT4->blkNum = COMM_BitReversal(pT30->HDLC_RX_Buf[5]);
                        pT4->frmNumInBlock = COMM_BitReversal(pT30->HDLC_RX_Buf[6]);
                    }

                    pT30->pfT30Vec = T30_PhaseC_SendConf_ECM;
#if SUPPORT_V34FAX
                    pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                    pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                    break;
                case EOR_NULL:
                case EOR_MPS:
                case EOR_EOP:
                case EOR_EOM:
                case EOR_PRI_EOM:
                case EOR_PRI_MPS:
                case EOR_PRI_EOP:
                    pT4->FCF1 = T4_EOR;
                    pT4->FCF2 = pT30->hdlcTypeRx & 0xF7;
                    pT30->hdlcTypeRx &= 0xFFF7;

                    /* set all bits in status to 0 to send everything to DTE */
                    for (i = 0; i <= BLOCKNUM_BYTESIZE; i++)
                    {
                        pT4->status[i] = 0; // pretent all frames are received
                    }

                    pT30->pfT30Vec = T30_PhaseC_SendConf_ECM;
#if SUPPORT_V34FAX
                    pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                    pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                    break;
                case HDLC_TYPE_T4_RR:
                    pT30->pfT30Vec = T30_PhaseC_SendConf_ECM;
#if SUPPORT_V34FAX
                    pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                    pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                    break;
                case HDLC_TYPE_T4_CTC:
                    pT4->FCF1 = T4_CTC;

                    if (T30_AnalysisDCS_CTC(pT30, TRUE))
                    {
                        pT30->pfT30Vec = T30_PhaseC_SendConf_ECM;
#if SUPPORT_V34FAX
                        pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                        pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                    }
                    else
                    {
                        T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_D_RCV);
                    }

                    break;
                case HDLC_TYPE_DCS:   // fix issue 221 and 222, to solve ft send DCS after first image
                    pT4->FCF1 = 0;
                    T30_AnalysisDCS_CTC(pT30, FALSE);
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

                    break;
                case HDLC_TYPE_DCN:
                    pT4->FCF1 = 0;
                    T30_AssumeEopUponInvCmd(pT30);
                    TRACE0("T30: DCN instead of PE is received");
                    break;
                default:
                    pT4->FCF1 = 0;
                    T30_AssumeEopUponInvCmd(pT30);
                    TRACE1("T30: ERROR. Invalid message 0x%x", pT30->hdlcTypeRx);
                    T30_GotoPointC(pT30, HANGUP_INVALID_CMD_PHASE_D_RCV);
                    break;
            }

            if (pT4->FCF2 != FCF2_NULL && // fixing issue 189, it's not PPS_NULL
                pT4->isBlkSentToDte &&
                !pT4->isPgSentToDte) // all blocks are uploaded
            {
                T30_UploadPgEndToDte_ECM(pT30);
            }

            T30_StopHDLCReceiving(pT30);
            break;
    }
}


/* [6]
** T30_PhaseC_SendConf in ECM mode: (MODEM speed LOW:T30)
** send PPR, MCF, RNR to calling
*/
void T30_PhaseC_SendConf_ECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UWORD re = 0, i;
    UBYTE allok;

    /* while data available, pass it to DTE */
    if (!pT4->isBlkSentToDte)
    {
        T30_SendImageData_ToDTE_ECM(pT30);
    }

#if SUPPORT_V34FAX

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_RCV); return;
    }
    else
#endif
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

            if ((pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
#if SUPPORT_V34FAX
                || isV34Selected
#endif
               )
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_CRC_OK;
                allok = TRUE;

                if (!pT4->areAllFrmRcved && (pT4->FCF1 != T4_EOR))
                {
                    for (i = 0; i <= pT4->frmNumInBlock; i++)
                    {
                        if (ReadBit(pT4->status[i>>3], 7 - (i & 0x7)))
                        {
                            // not all frames are received
                            allok = FALSE;
                            break;
                        }
                    }

                    pT4->areAllFrmRcved = allok;
                }

                // LLL fixing issue 10
                //else if (!pT30->Block_Receive)
                if (!allok)
                {
                    if (pT4->FCF1 == T4_CTC) // fixing issue 111
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_T4_CTR;
                    }
                    else
                    {
                        //if (isV34Selected)
                        //{
                        //    //fixing issue 173, however faxlab doesn't support CC retrain between PPS and PPR
                        //    if ((pT4->PPR_count & 0x3) == 0x3)
                        //    {
                        //        T30_SetCommand(pT30ToDce, T30_DCE_DLE_RTNC);
                        //        pT30ToDce->initV34PrimRateMax = pDceToT30->priChnlBitRate - 1;
                        //    }
                        //}

                        pT4->PPR_count++;

                        for (i = 0; i < BLOCKNUM_BYTESIZE; i++)
                        {
                            pT30->HDLC_TX_Buf[3 + i] = pT4->status[i];
                        }

                        pT30->hdlcTypeTx = HDLC_TYPE_T4_PPR;
                    }
                }
                else if (pT4->isBlkSentToDte)
                {
                    pT4->PPR_count = 0;

                    if (pT4->FCF1 == T4_EOR)
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_T4_ERR;
                    }
                    else
                    {
                        pT30->hdlcTypeTx = HDLC_TYPE_MCF;
                    }
                }
                else
                {
                    pT30->hdlcTypeTx = HDLC_TYPE_T4_RNR;
                }
            }

            break;
        default:
            re = T30_CommandTx(pT30, pT30->hdlcTypeTx);

            if (re == 1)
            {
                T30_StopHDLCSending(pT30);

                switch (pT30->hdlcTypeTx)
                {
                    case HDLC_TYPE_T4_PPR:
                    case HDLC_TYPE_T4_CTR:
#if SUPPORT_V34FAX

                        if (isV34Selected)
                        {
                            pT30->pfT30Vec = T30_PhaseD_GetPE_ECM; // fixing issue 169, return to GetPE
                        }
                        else
#endif
                        {
                            pT30->pfT30Vec = T30_PhaseC_ImageDataRx_ECM; pT30->rxStage = HS_RX_STAGE_FRM;
                        }

                        break;
                    case HDLC_TYPE_MCF:
                    case HDLC_TYPE_T4_ERR:

                        if (pT4->FCF2 == FCF2_EOP || pT4->FCF2 == FCF2_EOM)
                        {
                            pT30->pfT30Vec = T30_PhaseE_Ans;
                        }

#if SUPPORT_V34FAX
                        else if (isV34Selected)
                        {
                            pT30->pfT30Vec = T30_PhaseD_GetPE_ECM; // fixing issue 169, return to GetPE
                        }

#endif
                        else
                        {
                            pT30->pfT30Vec = T30_PhaseC_ImageDataRx_ECM; pT30->rxStage = HS_RX_STAGE_FRM;
                        }

                        break;
                    case HDLC_TYPE_T4_RNR:
                        pT30->pfT30Vec = T30_PhaseD_GetPE_ECM;
                        break;
                    default:
                        TRACE1("T30: ERROR. Invalid HDLC type 0x%x", pT30->hdlcTypeTx);
                        pT30->wasLastTry = TRUE;
                        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_RCV); return;
                }
            }
    }
}

#endif
