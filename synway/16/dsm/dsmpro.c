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
#include "dsmstru.h"
#include "dsmdef.h"
#include "t4info.h"
#include "imgprodte.h"

extern void SendReportMsgSub(GAO_Msg *pMsg, UWORD MsgCode);

void DsmSendCtlMsg(GAO_Msg *pMsg, UWORD MsgCode)
{
    pMsg->Direction = MSG_DOWN_LINK;
    pMsg->MsgType = MSG_TYPE_CONTROL;
    pMsg->MsgCode = MsgCode;
}

void DsmReportMsgUL(GAO_Msg *pRptUL, TDsmStrcut *pDsm)
{
    TImgFormat *pTif = &(pDsm->TxTifFormat);
    TImgFormat *pDcs = &(pDsm->DcsFormat);

    switch (pRptUL->MsgCode)
    {
        case RPT_REMOTE_POLL_IND:
            pDsm->RemoteHasDocTx = 1;
            break;
        case RPT_RESULT_CODE:
            pDsm->RespCode = pRptUL->Buf[0];

            if ((pDsm->RespCode == DSM_RC_RING) && (pDsm->RingNumber > 0))
            {
                pDsm->RingCount++;
            }

            if ((pDsm->RespCode == DSM_RC_OK) && (pDsm->GotImgFormat == 1))
            {
                pDsm->GotImgFormat = 2; // Dsm Got OK response to TxImgFormat message from DCE
                TRACE0("DSM: got image format of the tiff file to be sent.");
            }

            break;
        case RPT_NEGOTIATED_PARAS:
            pDcs->PaperSize = pRptUL->Buf[0];
            pDcs->XRes = pRptUL->Buf[1];
            pDcs->YRes = pRptUL->Buf[2];
            pDcs->Compression = pRptUL->Buf[3];
            pDcs->FillOrder = FILLORDER_LSB_FIRST;
            pDcs->BitRate = pRptUL->Buf[4];
            pDcs->ScanTimePerLine = pRptUL->Buf[6];

            if (pDsm->CallType == FAX_CALL_ANS)
            {
                pDsm->Direction = DSM_FAX_RECEIVING;
            }

            if (pDsm->Direction == DSM_FAX_TRANSMISSION)
            {
#if SUPPORT_TX_PURE_TEXT_FILE
                if (pTif->Compression == DSM_DF_TEXT_FILE)
                {
                    pDsm->SendImgConCmd = 3; // tx a text file
                }
                else if (pDcs->Compression != DSM_DF_JPEG_MODE)
#else
                if (pDcs->Compression != DSM_DF_JPEG_MODE)
#endif                
                {
                    if (pTif->Compression == DSM_DF_JPEG_MODE)
                    {
                        pDsm->SendImgConCmd = 2;    // just inform the direction
                    }
                    else if ((pDcs->PaperSize != pTif->PaperSize) || (pDcs->XRes != pTif->XRes) ||
                             (pDcs->YRes != pTif->YRes) || (pDcs->Compression != pTif->Compression))
                    {
                        pDsm->SendImgConCmd = 1;    // the format of dcs and tif is different, so need to convert
                    }
                    else
                    {
                        pDsm->SendImgConCmd = 2;    // just inform the direction
                    }

#if !SUPPORT_CONVERSION_BETWEEN_T4_T6

                    if (pDsm->SendImgConCmd == 1)
                    {
                        TRACE0("DSM: DCS and Tif have different image format, but image conversion is off. ERROR.");
                    }

#endif
                }
                else
                {
                    pDsm->SendImgConCmd = 2;    // just inform the direction
                }
            }
            else
            {
                pDsm->SendImgConCmd = 2;    // just inform the direction
            }

            pDsm->FcsFtcRpt = RPT_NEGOTIATED_PARAS;
            break;
        case RPT_FTC_OF_REMOTE_SIDE:
            pDsm->FcsFtcRpt = RPT_FTC_OF_REMOTE_SIDE;
            break;
        case RPT_HANGUP_STATUS:
            pDsm->RespCode = DSM_RC_HANGUPING;
            pDsm->RingCount = 0;
            break;
        case RPT_RECV_POST_PAGE_CMD:
            pDsm->PostMsgCmd = pRptUL->Buf[0];
            break;
    }
}

void DsmMsgProUplink(GAO_Msg *pMsgUL, TDsmStrcut *pDsm)// the uplink messages DSM needs to process
{
    if (pMsgUL->MsgType == MSG_TYPE_REPORT)
    {
        DsmReportMsgUL(pMsgUL, pDsm);
    }
    else if (pMsgUL->MsgType == MSG_TYPE_IMAGE_DATA)
    {
        return;
    }
    else
    {
        memset(pMsgUL, 0, sizeof(GAO_Msg));
    }
}

void DsmControlMsgDL(GAO_Msg *pCtrlDL, TDsmStrcut *pDsm)
{
    switch (pCtrlDL->MsgCode)
    {
        case CTL_FAX_CALL_ORG:
            pDsm->CallType = FAX_CALL_ORG;
            break;
        case CTL_FAX_CALL_ANS:
            pDsm->CallType = FAX_CALL_ANS;
            break;
        case CTL_ACTION_AFTER_PRI:

            if (pCtrlDL->Buf[0] == 1)
            {
                pDsm->CallType = FAX_CALL_ORG;
            }
            else if (pCtrlDL->Buf[0] == 2)
            {
                pDsm->CallType = FAX_CALL_ANS;
            }
            else if (pCtrlDL->Buf[0] == 3)
            {
                pDsm->ActionAfterPRI = HANGUP_AFTER_PRI;
            }

            break;
    }
}

void DsmConfigureMsgDL(GAO_Msg *pCfgDL, TDsmStrcut *pDsm)
{
    TImgFormat *pTif = &(pDsm->TxTifFormat);

    switch (pCfgDL->MsgCode)
    {
        case CFG_TX_IMG_FORMAT:
            pTif->Compression = pCfgDL->Buf[3];
#if SUPPORT_TX_PURE_TEXT_FILE
            if (pTif->Compression == DSM_DF_TEXT_FILE)
            {
                pDsm->GotImgFormat = 2;
                //TRACE0("DSM: do not need to pass the msg img format of a text file.");
                //memset(pCfgDL, 0, sizeof(GAO_Msg));
            }
            else
                //if(pTif->Compression != DSM_DF_TEXT_FILE)
#endif
            {
                pDsm->GotImgFormat = 1; // DSM got tx img format message and wait for OK response from DCE
                pTif->PaperSize = pCfgDL->Buf[0];
                pTif->XRes = pCfgDL->Buf[1];
                pTif->YRes = pCfgDL->Buf[2];
                pTif->FillOrder = pCfgDL->Buf[4];
            }

            break;
        case CFG_LOCAL_POLL_REQ:
            pDsm->RxPolledDocCapa = pCfgDL->Buf[0];
            break;
        case CFG_LOCAL_POLL_IND:

            if (pCfgDL->Buf[0] == 0)
            {
                pDsm->DocNumForPoll = pCfgDL->Buf[0];
            }

            break;
        case CFG_TIFF_FILE_PTR_TX:
            pDsm->DocNumForTx = pCfgDL->Buf[1];
            //memset(pCfgDL, 0, sizeof(GAO_Msg));
            break;
        case CFG_TIFF_FILE_PTR_RX:
            pDsm->RxTiffPtrReady = 1;
            TRACE0("DSM: Rx Tiff Ready.");
            break;
        case CFG_TIFF_FILE_PTR_TX_BY_POLL:
            pDsm->DocNumForPoll = pCfgDL->Buf[0];
            memset(pCfgDL, 0, sizeof(GAO_Msg));
            break;
        case CFG_RE_TRANSMIT_CAPA_NON_ECM:
            pDsm->ReTxCapaNoECM = pCfgDL->Buf[0];
            break;
        case CFG_RING_NUMBER_FOR_AUTO_ANS:
            pDsm->RingNumber = pCfgDL->Buf[0];
            break;
#if (SUPPORT_TX_COVER_PAGE)
        case CFG_COVER_PAGE_ATTRIB:

            if (pCfgDL->Buf[0] == TIFF_COVER_PAGE_SIDE_PATTERN)
            {
                pDsm->TxCoverP = pCfgDL->Buf[3];
            }

            break;
#endif
        case CFG_COPY_QUALITY_CHECK:
            pDsm->DceRxImageQualityCheckCapa = pCfgDL->Buf[0];
            pDsm->DceTxImageQualityCheckCapa = pCfgDL->Buf[1];
            break;
    }
}



/*
void DsmTiffFileDataMsgDL(GAO_Msg *pTifFileDL, TDsmStrcut *pDsm)// Interface E
{

}
*/

void DsmMsgProDownlink(GAO_Msg *pMsgDL, TDsmStrcut *pDsm) // the downlink messages DSM needs to process
{
    switch (pMsgDL->MsgType)
    {
        case MSG_TYPE_CONTROL:
            DsmControlMsgDL(pMsgDL, pDsm);
            break;
        case MSG_TYPE_CONFIG:
            DsmConfigureMsgDL(pMsgDL, pDsm);
            break;
        case MSG_TYPE_IMAGE_DATA:

            if (pMsgDL->MsgCode == IMG_DATA_AND_STATUS)
            {
                pDsm->PostMsgCmd = pMsgDL->Buf[0] - 1;
#if (SUPPORT_TX_COVER_PAGE)
                if (pDsm->PostMsgCmd == (TIFF_PMC_COVER_PAGE_END - 1))
                {
                    if (pDsm->DocNumForTx != 0)
                    {
                        pDsm->PostMsgCmd = DSM_PMC_MPS;
                    }
                    else
                    {
                        pDsm->PostMsgCmd = DSM_PMC_EOP;
                    }
                }
                else if (pDsm->PostMsgCmd == (TIFF_PMC_COVER_PAGE - 1))
                {
                    pDsm->PostMsgCmd = DSM_PMC_IDL;
                }
#endif
            }

            break;
        case MSG_TYPE_TEST:
        case MSG_TYPE_READ:
            break;
        default:
            memset(pMsgDL, 0, sizeof(GAO_Msg));
            break;
    }
}




void DsmMsgProMain(TDsmApiStruct *pApi, TDsmStrcut *pDsm)
{
    GAO_Msg *pTiffDsm = pApi->pTiffDsm;
    GAO_Msg *pDsmDce = pApi->pDsmDce;
    GAO_Msg *pDsmImg = pApi->pDsmImg;

    if ((pApi == NULL) || (pDsm == NULL))
    {
        return;
    }

    if (pTiffDsm->Direction == MSG_DOWN_LINK)
    {
        if ((pTiffDsm->MsgType == MSG_TYPE_IMAGE_DATA) && (pDsmImg->Direction == MSG_DIR_IDLE))
        {
            DsmMsgProDownlink(pTiffDsm, pDsm);
            memcpy(pDsmImg, pTiffDsm, sizeof(GAO_Msg));
            memset(pTiffDsm, 0, sizeof(GAO_Msg));
        }
        else if ((pTiffDsm->MsgType != MSG_TYPE_IMAGE_DATA) && (pDsmDce->Direction == MSG_DIR_IDLE))
        {
            DsmMsgProDownlink(pTiffDsm, pDsm);
            memcpy(pDsmDce, pTiffDsm, sizeof(GAO_Msg));
            memset(pTiffDsm, 0, sizeof(GAO_Msg));
        }
        else if ((pDsmImg->Direction == MSG_DIR_IDLE) && (pDsmDce->Direction == MSG_DIR_IDLE)) // invalid messages
        {
            memset(pTiffDsm, 0, sizeof(GAO_Msg));
        }

    }

    if (pDsm->RptTiffFileMode != 0)
    {
        if (pTiffDsm->Direction == MSG_DIR_IDLE)
        {
            pTiffDsm->Direction = MSG_UP_LINK;
            pTiffDsm->MsgType = MSG_TYPE_REPORT;
            pTiffDsm->MsgCode = RPT_TIFF_FILE_RUN_MODE;
            pTiffDsm->Buf[0] = pDsm->RptTiffFileMode;
            pTiffDsm->Buf[1] = pDsm->NumOfTiffFile;

            if (!((pDsm->RptTiffFileMode == DSM_RE_TX_TIFF_FILE) || (pDsm->RptTiffFileMode == DSM_RE_TX_TIFF_FILE_BY_POLL)))
            {
                pDsm->NumOfTiffFile++;
            }

            pDsm->RptTiffFileMode = 0;
        }
    }
    else if (pDsm->ActionCmd != 0)
    {
        if (pDsmDce->Direction == MSG_DIR_IDLE)
        {
            DsmSendCtlMsg(pDsmDce, pDsm->ActionCmd);
            pDsm->ActionCmd = 0;
        }
    }
    else if (pDsm->ResetDte != 0)
    {
        if (pTiffDsm->Direction == MSG_DIR_IDLE)
        {
            SendReportMsgSub(pTiffDsm, RPT_DCE_DISC);
            pDsm->ResetDte = 0;
        }
    }

    if (pDsmImg->Direction == MSG_UP_LINK)
    {
        if (pDsmImg->MsgType == MSG_TYPE_IMAGE_DATA)
        {
            if (pTiffDsm->Direction == MSG_DIR_IDLE)
            {
                memcpy(pTiffDsm, pDsmImg, sizeof(GAO_Msg));
                memset(pDsmImg, 0, sizeof(GAO_Msg));
            }
        }
    }

    if (pDsmDce->Direction == MSG_UP_LINK)
    {
        if (pDsmDce->MsgType != MSG_TYPE_IMAGE_DATA)
        {
            if (pTiffDsm->Direction == MSG_DIR_IDLE)
            {
                DsmMsgProUplink(pDsmDce, pDsm);
                memcpy(pTiffDsm, pDsmDce, sizeof(GAO_Msg));
                memset(pDsmDce, 0, sizeof(GAO_Msg));
            }
        }
    }

    if (pDsmDce->Direction == MSG_DIR_IDLE)
    {
        if (pDsm->SendATA == 1)
        {
            DsmSendCtlMsg(pDsmDce, CTL_FAX_CALL_ANS);
            pDsm->SendATA = 0;
        }
    }
}


void DsmSessionStateControl(TDsmApiStruct *pApi, TDsmStrcut *pDsm)
{
    GAO_Msg *pTiffDsm = pApi->pTiffDsm;
    UBYTE CallType = pDsm->CallType;

    if (pDsm->RespTimer)
    {
        pDsm->RespTimer++;
    }

    switch (pDsm->State)
    {
        case DSM_STATE_INIT:
            pDsm->PostMsgCmd = DSM_PMC_IDL;
            pDsm->RespCode = DSM_RC_NULL;
            pDsm->ActionCmd = 0;
            pDsm->NumOfTiffFile = 0;

            if (pDsm->RingNumber > 0)
            {
                if (pDsm->RingCount >= pDsm->RingNumber)
                {
                    pDsm->CallType = FAX_CALL_ANS;
                    pDsm->State = DSM_STATE_ATD_ATA;
                    pDsm->RespTimer = 1;
                    pDsm->RingCount = 0;
                    pDsm->SendATA = 1;
                }
            }
            else
            {
                if (pDsm->ActionAfterPRI == HANGUP_AFTER_PRI)
                {
                    pDsm->State = DSM_STATE_FKS;
                    pDsm->RespTimer = 1;
                }
                else if (CallType != FAX_CALL_IDL)
                {
                    pDsm->State = DSM_STATE_ATD_ATA;
                    pDsm->RespTimer = 1;
                }
            }

            break;
        case DSM_STATE_ATD_ATA:

            if (pDsm->RespCode == DSM_RC_OK)
            {
                pDsm->RespTimer = 0;
                pDsm->RespCode = DSM_RC_NULL;
                pDsm->RxNextDoc = 0;

                if (CallType == FAX_CALL_ORG)
                {
                    if (pDsm->DocNumForTx > 0)
                    {
                        pDsm->State = DSM_STATE_FDT;
                        pDsm->RptTiffFileMode = DSM_TX_TIFF_FILE;
                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                        pDsm->RespTimer = 1;
                    }
                    else if (pDsm->RemoteHasDocTx == 1)
                    {
                        if (pDsm->RxPolledDocCapa == 1)
                        {
                            TRACE0("DSM: This calling side creates a call to receive a polled doc.");
                            pDsm->State = DSM_STATE_FDR;
                            pDsm->RptTiffFileMode = DSM_RX_TIFF_FILE_BY_POLL;
                            pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                            pDsm->RxTiffPtrReady = 0;
                            pDsm->RemoteHasDocTx = 0;
                            pDsm->RespTimer = 1;
                        }
                        else
                        {
                            TRACE0("DSM: Remote has a doc to be polled, but local has no capability to receive a polled document!");
                            pDsm->RespTimer = DSM_RESP_TIMER_LEN + 1; // reset DTE
                        }
                    }
                    else
                    {
                        TRACE0("DSM: For this fax call, there is nothing to send and nothing to receive!");
                        pDsm->RespTimer = DSM_RESP_TIMER_LEN + 1; // reset DTE
                    }
                }
                else
                {
                    if (pDsm->FcsFtcRpt == RPT_FTC_OF_REMOTE_SIDE)
                    {
                        if (pDsm->DocNumForPoll > 0)
                        {
                            pDsm->State = DSM_STATE_FDT;
                            pDsm->RptTiffFileMode = DSM_TX_TIFF_FILE_BY_POLL;
                            pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                            pDsm->RespTimer = 1;
                        }
                        else
                        {
                            TRACE0("DSM: Local should have a doc to be ready to be polled!");
                        }
                    }
                    else if (pDsm->FcsFtcRpt == RPT_NEGOTIATED_PARAS)
                    {
                        pDsm->State = DSM_STATE_FDR;
                        pDsm->RptTiffFileMode = DSM_RX_TIFF_FILE;
                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                        pDsm->RxTiffPtrReady = 0;
                        pDsm->RespTimer = 1;
                    }
                    else
                    {
                        TRACE0("DSM: should have FCS or FTC report before here!");
                    }

                    pDsm->FcsFtcRpt = 0;
                }
            }
            //else if (pDsm->RespCode == DSM_RC_HANGUPING)
            else if (pDsm->RespCode != DSM_RC_NULL)
            {
                pDsm->State = DSM_STATE_INIT;
                pDsm->CallType = FAX_CALL_IDL;
                pDsm->RespCode = DSM_RC_NULL;
                pDsm->PostMsgCmd = DSM_PMC_IDL;
                pDsm->RespTimer = DSM_RESP_TIMER_LEN + 1;
            }

            break;
        case DSM_STATE_FDT:

            if (pDsm->GotImgFormat == 2)// Dsm Got OK response to TxImgFormat message from DCE
            {
                pDsm->GotImgFormat = 0;
                pDsm->Direction = DSM_FAX_TRANSMISSION;
                pDsm->ActionCmd = CTL_TRANSMIT_PAGE;
                pDsm->State = DSM_STATE_CONNECT;
                pDsm->FcsFtcRpt = 0;
                pDsm->RespCode = DSM_RC_NULL;
                pDsm->RespTimer = 1;
            }

            break;
        case DSM_STATE_FDR:

            if (pDsm->RxTiffPtrReady == 1)
            {
                pDsm->RxTiffPtrReady = 0;
                pDsm->ActionCmd = CTL_RECEIVE_PAGE;
                pDsm->Direction = DSM_FAX_RECEIVING;
                pDsm->FcsFtcRpt = 0;
                pDsm->State = DSM_STATE_CONNECT;
                pDsm->RespTimer = 1;
            }

            break;
        case DSM_STATE_CONNECT:

            if ((pDsm->Direction == DSM_FAX_RECEIVING) && (pDsm->FcsFtcRpt == RPT_NEGOTIATED_PARAS) && (pDsm->PostMsgCmd == DSM_PMC_EOM))
            {
                pDsm->RptTiffFileMode = DSM_RX_TIFF_FILE;
                pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                pDsm->PostMsgCmd = DSM_PMC_IDL;
                pDsm->RxTiffPtrReady = 0;
                pDsm->RxNextDoc = 1;
            }

            if (pDsm->RespCode == DSM_RC_CONNECT)
            {
                if (pDsm->RxNextDoc == 1)
                {
                    if (pDsm->RxTiffPtrReady == 1)
                    {
                        pDsm->RespTimer = 0;
                        pDsm->State = DSM_STATE_IMAGE;
                        pDsm->RespCode = DSM_RC_NULL;
                        pDsm->ActionCmd = CTL_DTE_READY_RECV;
                        pDsm->RxTiffPtrReady = 0;
                        pDsm->RxNextDoc = 0;
                    }
                    else
                    {
                        TRACE0("DSM: Waiting for Rx Tiff Ready!");
                    }
                }
                else  // fax send or receive the first doc
                {
                    pDsm->RespTimer = 0;
                    pDsm->State = DSM_STATE_IMAGE;
                    pDsm->RespCode = DSM_RC_NULL;

                    if (pDsm->Direction == DSM_FAX_RECEIVING)
                    {
                        pDsm->ActionCmd = CTL_DTE_READY_RECV;
                    }
                }
            }
            else if (pDsm->RespCode == DSM_RC_HANGUPING) // last +FDR for fax receiving
            {
                pDsm->State = DSM_STATE_GOING_HANGUP;
                pDsm->RespCode = DSM_RC_NULL;
                pDsm->PostMsgCmd = DSM_PMC_IDL;
                pDsm->RespTimer = 1;
            }
            else if (pDsm->RespCode == DSM_RC_OK)  // last +FDR before changing to Tx from Rx
            {
                pDsm->State = DSM_STATE_IMAGE;
                pDsm->PostMsgCmd = 0;
                pDsm->RespTimer = 0;
            }

            break;
        case DSM_STATE_IMAGE:

            switch (pDsm->RespCode)
            {
                case DSM_RC_HANGUPING:
                    pDsm->State = DSM_STATE_GOING_HANGUP;
                    pDsm->RespCode = DSM_RC_NULL;
                    pDsm->PostMsgCmd = DSM_PMC_IDL;
                    pDsm->RespTimer = 1;
                    break;
                case DSM_RC_ERROR:
                case DSM_RC_OK:
                    pDsm->RespTimer = 0;

                    if (pDsm->PRI_Voice == 1) // During PRI
                    {
                        pDsm->State = DSM_STATE_INIT;
                        pDsm->CallType = FAX_CALL_IDL;
                        pDsm->PRI_Voice = 0;
                    }
                    else
                    {
                        if (pDsm->Direction == DSM_FAX_TRANSMISSION)
                        {
                            if (pDsm->PostMsgCmd == DSM_PMC_EOP) // only for non-ecm ERROR result code
                            {
                                if ((pDsm->RespCode == DSM_RC_ERROR) && (pDsm->ReTxCapaNoECM == 1))
                                {
                                    pDsm->State = DSM_STATE_FDT;
                                    pDsm->RptTiffFileMode = DSM_RE_TX_TIFF_FILE;
                                    pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                                    pDsm->NumOfTiffFile--;
                                    pDsm->GotImgFormat = 0;
                                    pDsm->RespTimer = 1;
                                }
                                else
                                {
                                    pDsm->State = DSM_STATE_INIT;
                                    pDsm->RespCode = DSM_RC_NULL;
                                    pDsm->PostMsgCmd = DSM_PMC_IDL;
                                    pDsm->CallType = FAX_CALL_IDL;
                                    pDsm->RespTimer = 0;
                                }
                            }
                            else if (pDsm->PostMsgCmd == DSM_PMC_EOM)
                            {
                                if ((pDsm->TiffFileMode == DSM_TX_TIFF_FILE_BY_POLL) || (pDsm->TiffFileMode == DSM_RE_TX_TIFF_FILE_BY_POLL))
                                {
                                    if ((pDsm->RespCode == DSM_RC_ERROR) && (pDsm->ReTxCapaNoECM == 1))
                                    {
                                        pDsm->State = DSM_STATE_FDT;
                                        pDsm->RptTiffFileMode = DSM_RE_TX_TIFF_FILE_BY_POLL;
                                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                                        pDsm->NumOfTiffFile--;
                                        pDsm->GotImgFormat = 0;
                                        pDsm->RespTimer = 1;
                                    }
                                    else if (pDsm->NumOfTiffFile < pDsm->DocNumForPoll)
                                    {
                                        pDsm->State = DSM_STATE_FDT;
                                        pDsm->RptTiffFileMode = DSM_TX_TIFF_FILE_BY_POLL;
                                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                                        pDsm->GotImgFormat = 0;
                                        pDsm->RespTimer = 1;
                                    }
                                    else if ((pDsm->RemoteHasDocTx == 1) && (pDsm->RxPolledDocCapa == 1))
                                    {
                                        pDsm->NumOfTiffFile = 0;
                                        pDsm->State = DSM_STATE_FDR;
                                        pDsm->RptTiffFileMode = DSM_RX_TIFF_FILE_BY_POLL;
                                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                                        pDsm->RxTiffPtrReady = 0;
                                        pDsm->RespTimer = 1;
                                        pDsm->RemoteHasDocTx = 0;
                                    }
                                    else
                                    {
                                        pDsm->State = DSM_STATE_INIT;
                                        pDsm->RespCode = DSM_RC_NULL;
                                        pDsm->PostMsgCmd = DSM_PMC_IDL;
                                        pDsm->CallType = FAX_CALL_IDL;
                                        pDsm->RespTimer = 0;
                                    }
                                }
                                else
                                {
                                    if ((pDsm->RespCode == DSM_RC_ERROR) && (pDsm->ReTxCapaNoECM == 1))
                                    {
                                        pDsm->State = DSM_STATE_FDT;
                                        pDsm->RptTiffFileMode = DSM_TX_TIFF_FILE;
                                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                                        pDsm->GotImgFormat = 0;
                                        pDsm->RespTimer = 1;
                                    }
                                    else if (pDsm->NumOfTiffFile < pDsm->DocNumForTx)
                                    {
                                        pDsm->State = DSM_STATE_FDT;
                                        pDsm->RptTiffFileMode = DSM_TX_TIFF_FILE;
                                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                                        pDsm->GotImgFormat = 0;
                                        pDsm->RespTimer = 1;
                                    }
                                    else if ((pDsm->RemoteHasDocTx == 1) && (pDsm->RxPolledDocCapa == 1))
                                    {
                                        pDsm->NumOfTiffFile = 0;
                                        pDsm->State = DSM_STATE_FDR;
                                        pDsm->RptTiffFileMode = DSM_RX_TIFF_FILE_BY_POLL;
                                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                                        pDsm->RxTiffPtrReady = 0;
                                        pDsm->RemoteHasDocTx = 0;
                                        pDsm->RespTimer = 1;
                                    }
                                    else
                                    {
                                        pDsm->State = DSM_STATE_INIT;
                                        pDsm->RespCode = DSM_RC_NULL;
                                        pDsm->PostMsgCmd = DSM_PMC_IDL;
                                        pDsm->CallType = FAX_CALL_IDL;
                                        pDsm->RespTimer = 0;
                                    }
                                }
                            }
                            else if (pDsm->PostMsgCmd == DSM_PMC_MPS)
                            {
                                pDsm->State = DSM_STATE_FDT;
                                pDsm->GotImgFormat = 2;  // between pages does not need to get the image format again.
                            }

                            pDsm->PostMsgCmd = 0;
                        }
                        else // receive
                        {
                            if (pDsm->PostMsgCmd == DSM_PMC_IDL)
                            {
                                if (pDsm->FcsFtcRpt == RPT_FTC_OF_REMOTE_SIDE)
                                {
                                    if (pDsm->DocNumForPoll > 0)
                                    {
                                        pDsm->NumOfTiffFile = 0;
                                        pDsm->State = DSM_STATE_FDT;
                                        pDsm->RptTiffFileMode = DSM_TX_TIFF_FILE_BY_POLL;
                                        pDsm->TiffFileMode = pDsm->RptTiffFileMode;
                                        pDsm->GotImgFormat = 0;
                                        pDsm->RespTimer = 1;
                                    }
                                    else
                                    {
                                        TRACE0("DSM: Got remote poll request, but local does not have doc to be polled!");
                                    }
                                }
                                else
                                {
                                    TRACE0("DSM: Got remote poll request, but local does not get FTC from the remote!");
                                }

                                pDsm->FcsFtcRpt = 0;
                            }
                            else
                            {
                                pDsm->State = DSM_STATE_FDR;
                                pDsm->RxTiffPtrReady = 1;   // different pages in the same doc or can not decide if it is receive multiple docs or change to Tx from Rx
                            }
                        }
                    }

                    pDsm->RespCode = DSM_RC_NULL;
                    break;
            }

            break;
        case DSM_STATE_FKS:

            if (pDsm->RespCode == DSM_RC_HANGUPING)
            {
                pDsm->State = DSM_STATE_GOING_HANGUP;
                pDsm->RespCode = DSM_RC_NULL;
                pDsm->PostMsgCmd = DSM_PMC_IDL;
                pDsm->RespTimer = 1;
            }

            break;
        case DSM_STATE_GOING_HANGUP:
            //if (pDsm->RespCode == DSM_RC_OK)
        {
            pDsm->State = DSM_STATE_INIT;
            pDsm->RespCode = DSM_RC_NULL;
            pDsm->CallType = FAX_CALL_IDL;
            pDsm->Direction = FAX_CALL_IDL;
            pDsm->RespTimer = 0;
            pDsm->RingCount = 0;
            memset(pDsm, 0, sizeof(TDsmStrcut));
        }
        break;
    }

    // judge the timer expiring for the response from DCE
    if (pDsm->RespTimer > DSM_RESP_TIMER_LEN)
    {
        if (pTiffDsm->Direction != MSG_DIR_IDLE)
        {
            TRACE0("DSM: Clear TiffDsm message buffer in order to report DCE disconnection message to APP!!");
        }

        pDsm->ResetDte = 1;
        pDsm->RespTimer = 0;
        pDsm->State = DSM_STATE_INIT;
        pDsm->CallType = FAX_CALL_IDL;
    }
}

void DsmInit(TDsmStrcut *pDsm)
{
    memset(pDsm, 0, sizeof(TDsmStrcut));
}

void DsmMain(TDsmApiStruct *pApi, TDsmStrcut *pDsm)
{
    DsmMsgProMain(pApi, pDsm);
    DsmSessionStateControl(pApi, pDsm);
}

