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
#include "porting.h"

#if SUPPORT_DTE
#include "gaostru.h"
#include "faxapi.h"
#include "dsmdef.h"
#include "common.h"
#if SUPPORT_T32_PARSER
#include "circbuff.h"
#endif
#if SUPPORT_T31_PARSER
#include "v251.h"
#endif
#if SUPPORT_T38
#include "t38ext.h"
#endif

void FaxlibVersionResp(GAO_Msg *pMsg)
{
    pMsg->Direction = MSG_UP_LINK;
    pMsg->MsgType = MSG_TYPE_MAINTENANCE;
    pMsg->MsgCode = MTN_RESP_FAXLIB_VERSION;
    pMsg->Buf[0] = VERSION_NUMBER;
}

void FaxlibMaintenance(GAO_Msg *pMsg)
{
    if ((pMsg->Direction == MSG_DOWN_LINK) && (pMsg->MsgType == MSG_TYPE_MAINTENANCE))
    {
        switch (pMsg->MsgCode)
        {
            case MTN_REQUEST_FAXLIB_VERSION:
                FaxlibVersionResp(pMsg);
                break;
        }
    }
}

void T30DownLinkInterfaceCtrl(GAO_Msg *pMsg, GAO_LibStruct *pFax)
{
    DteToT30Interface *pDteToT30 = &pFax->dteToT30;

    switch (pMsg->MsgCode)
    {
        case CTL_FAX_CALL_ANS:
            pDteToT30->currentDteT30Cmd = pDteToT30->actionCmd = DTE_T30_ATA; TRACE1("GAOMAIN: Command %s", T32_Action_Command[pDteToT30->actionCmd]);
            break;
        case CTL_FAX_CALL_ORG:
            pDteToT30->currentDteT30Cmd = pDteToT30->actionCmd = DTE_T30_ATD; TRACE1("GAOMAIN: Command %s", T32_Action_Command[pDteToT30->actionCmd]);
            pDteToT30->tempData[0] = pMsg->Buf[0]; // 1: pulse or 2: tone
            pDteToT30->tempDataLen = pMsg->Buf[1]; // length of phone number
            memcpy(&(pDteToT30->tempData[1]), &(pMsg->Buf[2]), pDteToT30->tempDataLen);// dialed telephone number
            break;
        case CTL_RECEIVE_PAGE:
            pDteToT30->currentDteT30Cmd = pDteToT30->actionCmd = DTE_T30_FDR; TRACE1("GAOMAIN: Command %s", T32_Action_Command[pDteToT30->actionCmd]);
            break;
        case CTL_TRANSMIT_PAGE:
            pDteToT30->currentDteT30Cmd = pDteToT30->actionCmd = DTE_T30_FDT; TRACE1("GAOMAIN: Command %s", T32_Action_Command[pDteToT30->actionCmd]);
            break;
        default:
            return;
    }

#if SUPPORT_T32_PARSER

    if (RdReg(pFax->m_Dsm.ServiceClass, FCLASS_MAJ) == FCLASS2)
    {
        PutStringToCB(&pFax->m_BufferOut, T32_Action_Command[pDteToT30->actionCmd], strlen(T32_Action_Command[pDteToT30->actionCmd]));

        if (pDteToT30->actionCmd == DTE_T30_ATD)
        {
            PutByteToCB(&pFax->m_BufferOut, pDteToT30->tempData[0] == 1 ? 'P' : 'T');
            PutStringToCB(&pFax->m_BufferOut, &pDteToT30->tempData[1], pDteToT30->tempDataLen);
        }

        PutByteToCB(&pFax->m_BufferOut, T50_CR);
        pDteToT30->tempDataLen = 0;
    }

#endif
}

UBYTE T30DownLinkInterfaceCfg(GAO_Msg *pMsg, GAO_LibStruct *pFax)
{
    TDsmStrcut *pDsm = &(pFax->m_Dsm);
    DteToT30Interface *pDteToT30 = &pFax->dteToT30;
    T30ToDteInterface *pT30ToDte = &pFax->t30ToDte;
    T30SessionSubParm *pSettings = &(pDteToT30->faxParmSettings);
#if SUPPORT_T32_PARSER
    CircBuffer *DteWr = &pFax->m_BufferOut;
    CONST CHAR *pCommand = NULL;
    CHAR temp[60];

    memset(temp, 0, sizeof(temp));
#endif

    switch (pMsg->MsgCode)
    {
        case CFG_COPY_QUALITY_CHECK:
            break;
        case CFG_RECV_QUALITY_THRESHOLD:
            break;
        case CFG_RE_TRANSMIT_CAPA_NON_ECM:
            break;
        case CFG_TIFF_FILE_PTR_TX:
            pDteToT30->isTransmit = TRUE;
            break;
        case CFG_LOCAL_POLL_IND:
            pDteToT30->isDocForPolling = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                sprintf(temp, "%s=1", T32_Parm_Command[DTE_T30_FLP - 1]);
                pCommand = temp;
            }

#endif
            break;
        case CFG_LOCAL_POLL_REQ:
            pDteToT30->isRequestingToPoll = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                sprintf(temp, "%s=1", T32_Parm_Command[DTE_T30_FSP - 1]);
                pCommand = temp;
            }

#endif
            break;
        case CFG_SERVICE_CLASSES:
#if SUPPORT_PARSER
            pDsm->CurrCnfgParm = pMsg->Buf[0];
            sprintf(temp, "%s=%d", T32_Parm_Command[DTE_T30_FCLASS - 1], RdRegShft(pDsm->CurrCnfgParm, FCLASS_MAJ));

            if (RdReg(pDsm->CurrCnfgParm, FCLASS_REV) == FCLASS_REV_0)
            {
                strcat(temp, ".0");
            }
            else if (RdReg(pDsm->CurrCnfgParm, FCLASS_REV) == FCLASS_REV_1)
            {
                strcat(temp, ".1");
            }

            pCommand = temp;
#else
            pDsm->ServiceClass = pMsg->Buf[0];
#endif
            break;
        case CFG_BIT_ORDER:
            pDteToT30->dataBitOrder = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                sprintf(temp, "%s=%1d", T32_Parm_Command[DTE_T30_FBO - 1], pDteToT30->dataBitOrder);
                pCommand = temp;
            }

#endif
            break;
        case CFG_LOCAL_RECV_CAPA:
            pDteToT30->isCapToRcv = pMsg->Buf[0] == 0 ? FALSE : TRUE;
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                sprintf(temp, "%s=1", T32_Parm_Command[DTE_T30_FCR - 1]);
                pCommand = temp;
            }

#endif
            break;
        case CFG_NEGOTIATION_PARA_REPORT:
            pDteToT30->negoReporting = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                sprintf(temp, "%s=%1d,%1d,%1d,%1d",
                        T32_Parm_Command[DTE_T30_FNR - 1],
                        RdRegShft(pDteToT30->negoReporting, FNR_RPR),
                        RdRegShft(pDteToT30->negoReporting, FNR_TPR),
                        RdRegShft(pDteToT30->negoReporting, FNR_IDR),
                        RdRegShft(pDteToT30->negoReporting, FNR_NSR));
                pCommand = temp;
            }

#endif
            break;
        case CFG_MIN_PHASE_C_RATE:
            pDteToT30->minPhaseCSpd = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                sprintf(temp, "%s=%1X", T32_Parm_Command[DTE_T30_FMS - 1], pDteToT30->minPhaseCSpd);
                pCommand = temp;
            }

#endif
            break;
        case CFG_LOCAL_ID:
            memcpy(pDteToT30->localID, &(pMsg->Buf[0]), ID_STRING_LEN_MAX);
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                sprintf(temp, "%s=\"%s\"", T32_Parm_Command[DTE_T30_FLI - 1], pDteToT30->localID);
                pCommand = temp;
            }

#endif
            break;
        case CFG_TX_IMG_FORMAT:
#if SUPPORT_TX_PURE_TEXT_FILE

            if (pMsg->Buf[3] == DSM_DF_TEXT_FILE)
            {
                pSettings->jpegOptionJP = 0;
                //pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
                break;
            }

#endif

            if (pMsg->Buf[0] == DSM_A4_LETTER_LEGAL)
            {
                pSettings->pageWidthWD = WD_0;
            }
            else if (pMsg->Buf[0] == DSM_B4)
            {
                pSettings->pageWidthWD = WD_1;
            }
            else if (pMsg->Buf[0] == DSM_A3)
            {
                pSettings->pageWidthWD = WD_2;
            }

            pSettings->jpegOptionJP = 0;

            if (pMsg->Buf[3] == DSM_DF_T4_1D)
            {
                pSettings->dataFormatDF = DF_MH;
            }
            else if (pMsg->Buf[3] == DSM_DF_T4_2D)
            {
                pSettings->dataFormatDF = DF_MR;
            }
            else if (pMsg->Buf[3] == DSM_DF_T6_2D)
            {
                pSettings->dataFormatDF = DF_MMR;
            }
            else if (pMsg->Buf[3] == DSM_DF_JPEG_MODE)
            {
                pSettings->jpegOptionJP = pMsg->Buf[5];
            }
            else
            {
                TRACE0("GAOMAIN: TIFF file compression format ERROR.");
            }

            if (pMsg->Buf[2] == VR_NORMAL_385)
            {
                pSettings->verticalResolutionVR = VR_R8_385;
            }
            else if (pMsg->Buf[2] == VR_FINE_77)
            {
                pSettings->verticalResolutionVR = VR_R8_77;
            }
            else if (pMsg->Buf[2] == VR_PERFECT_154)
            {
                if (pMsg->Buf[1] == R8_X_RESOLUTION)
                {
                    pSettings->verticalResolutionVR = VR_R8_154;
                }
                else
                {
                    pSettings->verticalResolutionVR = VR_R16_154;
                }
            }
            else if (pMsg->Buf[2] == VR_100L_INCH)
            {
                if (pMsg->Buf[1] == X_RESOLUTION_100DPI)
                {
                    pSettings->verticalResolutionVR = VR_100_100;
                }
                else
                {
                    pSettings->verticalResolutionVR = VR_200_100;
                }
            }
            else if (pMsg->Buf[2] == VR_200L_INCH)
            {
                pSettings->verticalResolutionVR = VR_200_200;
            }
            else if (pMsg->Buf[2] == VR_300L_INCH)
            {
                if (pSettings->jpegOptionJP & 0x02)
                {
                    pSettings->verticalResolutionVR = VR_300_300;
                }
                else
                {
                    pSettings->verticalResolutionVR = VR_300_300;
                }
            }
            else if (pMsg->Buf[2] == VR_400L_INCH)
            {
                pSettings->verticalResolutionVR = VR_400_400;
            }
            else if (pMsg->Buf[2] == VR_600L_INCH)
            {
                if (pSettings->jpegOptionJP & 0x02)
                {
                    pSettings->verticalResolutionVR = VR_600_600;
                }
                else
                {
                    if (pMsg->Buf[1] == X_RESOLUTION_300DPI)
                    {
                        pSettings->verticalResolutionVR = VR_300_600;
                    }
                    else
                    {
                        pSettings->verticalResolutionVR = VR_600_600;
                    }
                }
            }
            else if (pMsg->Buf[2] == VR_800L_INCH)
            {
                pSettings->verticalResolutionVR = VR_400_800;
            }
            else if (pMsg->Buf[2] == VR_1200L_INCH)
            {
                if (pSettings->jpegOptionJP & 0x02)
                {
                    pSettings->verticalResolutionVR = VR_1200_1200;
                }
                else
                {
                    if (pMsg->Buf[1] == X_RESOLUTION_600DPI)
                    {
                        pSettings->verticalResolutionVR = VR_600_1200;
                    }
                    else
                    {
                        pSettings->verticalResolutionVR = VR_1200_1200;
                    }
                }
            }

#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pDsm->CurrCnfgCmd = pMsg->MsgCode;
                sprintf(temp, "%s=", T32_Parm_Command[DTE_T30_FIS - 1]);
                PutStringToCB(DteWr, AT_PLUS_F, strlen(AT_PLUS_F));
                PutStringToCB(DteWr, temp, strlen(temp));
                COMM_ParseSubParmOut(pSettings, DteWr);
                PutByteToCB(DteWr, T50_CR);
                return TRUE;
            }

#endif
            break;
        case CFG_ECM_ATTEMP_BLOCK_RETRY_NUM:

            if (pMsg->Buf[0] > 0)
            {
                pDteToT30->ecmRetryCnt = pMsg->Buf[0];
            }

#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                sprintf(temp, "%s=%X", T32_Parm_Command[DTE_T30_FRY - 1], pDteToT30->ecmRetryCnt);
                pCommand = temp;
            }

#endif
            break;
        case CFG_DCE_CAPA_JP:
            pSettings->jpegOptionJP = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
        case CFG_DCE_CAPA_ST:
            pSettings->scanTimeST = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
        case CFG_DCE_CAPA_BF:
            pSettings->fileTransferBF = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
        case CFG_DCE_CAPA_DF:
            pSettings->dataFormatDF = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
        case CFG_DCE_CAPA_LN:
            pSettings->pageLengthLN = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
        case CFG_DCE_CAPA_WD:
            pSettings->pageWidthWD = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
        case CFG_DCE_CAPA_VR:
            pDteToT30->faxParmSettings.verticalResolutionVR = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
        case CFG_DCE_CAPA_EC:

            if (pMsg->Buf[0] && pT30ToDte->isV34Capable)
            {
                pDteToT30->isV34Enabled = TRUE;
            }
            else
            {
                pDteToT30->isV34Enabled = FALSE;
            }

            pSettings->errorCorrectionEC = pMsg->Buf[0];
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
        case CFG_DCE_CAPA_BR:

            if (pMsg->Buf[0] && pT30ToDte->isV34Capable)
            {
                pDteToT30->isV34Enabled = TRUE;
            }
            else
            {
                pDteToT30->isV34Enabled = FALSE;
            }

            if (pDteToT30->faxParmCapability.bitRateBR > pMsg->Buf[1])
            {
                pSettings->bitRateBR = pMsg->Buf[1];
            }

#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
            }

#endif
            break;
#if SUPPORT_T38
        case CFG_INIT_T38:

            if (T38_Init(&pFax->m_T38, (T38initParamStruct *)&pMsg->Buf[0]) < 0)
            {
                TRACE0("DTE: ERROR. Cannot initialize T.38");
            }

            pFax->m_T38.pDceToT30 = &(pFax->dceToT30);
            pFax->m_T38.pT30ToDce = &(pFax->t30ToDce);
            break;
#endif
#if SUPPORT_PARSER
        case CFG_RING_NUMBER_FOR_AUTO_ANS:
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pDsm->CurrCnfgCmd = pMsg->MsgCode;
                PutStringToCB(DteWr, "ATS0=0", 6);
                PutByteToCB(DteWr, T50_CR);
                return TRUE;
            }

#endif
            break;
        case CFG_INIT_DCE_CUST:

            if (pDsm->CurrCnfgCmd == CFG_INIT_DCE_DEF)
            {
                if (pT30ToDte->resultCodeRsp == RESULT_CODE_OK)
                {
                    if (strlen(pMsg->Buf) > 0)
                    {
                        pDsm->CurrCnfgCmd = pMsg->MsgCode;
                        PutStringToCB(DteWr, pMsg->Buf, strlen(pMsg->Buf));
                        PutByteToCB(DteWr, T50_CR);

                        pT30ToDte->resultCodeRsp = RESULT_CODE_NULL;
                    }

                    pDsm->CurrCnfgCmd = 0;
                    return TRUE;
                }
            }
            else
            {
                pDsm->CurrCnfgCmd = CFG_INIT_DCE_DEF;
                PutStringToCB(DteWr, DTE_DCE_INIT_PARM, strlen(DTE_DCE_INIT_PARM));
                PutByteToCB(DteWr, T50_CR);
            }

            return FALSE;
        case CFG_WRITE_FCC:
#if SUPPORT_T31_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS1)
            {
                T30ToDceInterface *pT30ToDce = &pFax->t30ToDce;
                DceToT30Interface *pDceToT30 = &pFax->dceToT30;
                UWORD len;
                UBYTE command = 0; // NULL, +A8E, +FCL, or +F34

#if SUPPORT_V34FAX

                if (pDsm->CurrCnfgCmd == CFG_V8_OPR_CTRL)
                {
                    switch (pDceToT30->resultCodeRsp)
                    {
                        case RESULT_CODE_NULL:
                            break;
                        case RESULT_CODE_OK:
                            command = 3;
                            break;
                        case RESULT_CODE_ERROR:
                            // DCE doesn't support V.34
                            pDteToT30->isV34Enabled = FALSE;
                            command = 2;
                            break;
                        default:
                            TRACE1("DTE: ERROR. Invalid result code response %d", pDceToT30->resultCodeRsp);
                            break;
                    }

                    pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                }
                else
#endif
                    if (pDsm->CurrCnfgCmd == CFG_FCLASS1_LAST)
                    {
                        // +FCL or +F34
                        switch (pDceToT30->resultCodeRsp)
                        {
                            case RESULT_CODE_NULL:
                                break;
                            case RESULT_CODE_OK:
                                pDsm->CurrCnfgCmd = 0;
                                pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
                                break;
                            case RESULT_CODE_ERROR:
#if SUPPORT_V34FAX

                                if (pDteToT30->isV34Enabled)
                                {
                                    TRACE0("DTE: ERROR. DCE doesn't support AT+F34");
                                }
                                else
#endif
                                {
                                    pDsm->CurrCnfgCmd = 0;
                                    pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
                                }

                                break;
                        }

                        pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                    }

#if SUPPORT_V34FAX
                    else if (pDteToT30->isV34Enabled)
                    {
                        command = 1;
                    }

#endif
                    else
                    {
                        // +FCL
                        command = 2;
                    }

                switch (command)
                {
#if SUPPORT_V34FAX
                    case 1: // +A8E
                        pDsm->CurrCnfgCmd = CFG_V8_OPR_CTRL;
                        pT30ToDte->resultCodeRsp = RESULT_CODE_NULL;

                        PutStringToCB(DteWr, "AT+A8E=", 7);
                        PutByteToCB(DteWr, (pT30ToDce->v8OpCntl & A8E_V8O) + '0');
                        PutByteToCB(DteWr, ',');
                        PutByteToCB(DteWr, ((pT30ToDce->v8OpCntl & A8E_V8A) >> 4) + '0');
                        PutByteToCB(DteWr, T50_CR);
                        pT30ToDce->v8OpCntl = A8E_NULL;
                        break;
                    case 3:
                        // +F34
                        pDsm->CurrCnfgCmd = CFG_FCLASS1_LAST;
                        pT30ToDte->resultCodeRsp = RESULT_CODE_NULL;

                        T30_UpdateT31Parms(&pFax->m_T30);

                        PutStringToCB(DteWr, "AT+F34=", 7);
                        len = COMM_DecToAscii(pT30ToDce->initV34PrimRateMax, temp);
                        PutStringToCB(DteWr, temp, len);
                        PutByteToCB(DteWr, ',');
                        len = COMM_DecToAscii(pT30ToDce->initV34PrimRateMin, temp);
                        PutStringToCB(DteWr, temp, len);
                        PutByteToCB(DteWr, ',');
                        len = COMM_DecToAscii(pT30ToDce->initV34CntlRate, temp);
                        PutStringToCB(DteWr, temp, len);
                        PutByteToCB(DteWr, T50_CR);
                        break;
#endif
                    case 2:
                        // +FCL
                        pDsm->CurrCnfgCmd = CFG_FCLASS1_LAST;
                        pT30ToDte->resultCodeRsp = RESULT_CODE_NULL;

                        PutStringToCB(DteWr, "AT+FCL=", 7);
                        len = COMM_DecToAscii(pT30ToDce->carrierLossTimeout, temp);
                        PutStringToCB(DteWr, temp, len);
                        PutByteToCB(DteWr, T50_CR);
                        break;
                }

                if (pDsm->OperationStage != DTE_OS_COMMAND)
                {
                    return FALSE;
                }
            }

#endif
#if SUPPORT_T32_PARSER

            if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
            {
                pDsm->CurrCnfgCmd = pMsg->MsgCode;
                sprintf(temp, "%s=", T32_Parm_Command[DTE_T30_FCC - 1]);
                PutStringToCB(DteWr, AT_PLUS_F, strlen(AT_PLUS_F));
                PutStringToCB(DteWr, temp, strlen(temp));
                COMM_ParseSubParmOut(pSettings, DteWr);
                PutByteToCB(DteWr, T50_CR);
                return TRUE;
            }

#endif
#endif
            break;
        default:
            return TRUE;
    }

#if SUPPORT_T32_PARSER

    if (pCommand != NULL)
    {
        pDsm->CurrCnfgCmd = pMsg->MsgCode;
        PutStringToCB(DteWr, AT_PLUS_F, strlen(AT_PLUS_F));
        PutStringToCB(DteWr, pCommand, strlen(pCommand));
        PutByteToCB(DteWr, T50_CR);
    }
    else
#endif
        if (pMsg->MsgCode != CFG_TIFF_FILE_PTR_TX)
        {
            pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
        }

    return TRUE;
}


void T30DownLinkInterfaceImg(GAO_Msg *pMsg, GAO_LibStruct *pFax)
{
#if SUPPORT_T32_PARSER
    TDsmStrcut *pDsm = &(pFax->m_Dsm);
#endif
    DteToT30Interface *pDteToT30 = &pFax->dteToT30;
    T30ToDteInterface *pT30ToDte = &pFax->t30ToDte;
    UBYTE Leng;

    if ((pMsg->Direction == MSG_DOWN_LINK) && (pMsg->MsgType == MSG_TYPE_IMAGE_DATA))
    {
        if (pT30ToDte->hangupCode != HANGUP_NORMAL)
        {
            memset(pMsg, 0, sizeof(GAO_Msg));
        }
        else if ((pMsg->MsgCode == IMG_DATA_AND_STATUS) &&
                 (pT30ToDte->dataCmd == DC1_XON) &&
                 (pDteToT30->tempDataLen == 0))
        {
            Leng = ((pMsg->Buf[1] << 8) & 0xff00) + pMsg->Buf[2];
            memcpy(pDteToT30->tempData, &(pMsg->Buf[3]), Leng);
            pDteToT30->tempDataLen = Leng;

            if ((pMsg->Buf[0] >= TIFF_PMC_NEXT_PAGE) && (pMsg->Buf[0] <= TIFF_PMC_END_OF_FILE_EOP))
            {
                pDteToT30->postPgMsg = pMsg->Buf[0] - 1;
            }

            memset(pMsg, 0, sizeof(GAO_Msg));
        }

#if SUPPORT_T32_PARSER

        if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
        {
            CircBuffer *DteWr = &pFax->m_BufferOut;

            if (pDteToT30->tempDataLen > 0)
            {
                int i = 0;
                UBYTE needsBitRev = RdReg(pDteToT30->dataBitOrder, BIT0) != 0;
                UBYTE currByte;

                while (i < pDteToT30->tempDataLen)
                {
                    if (GetAvailLenInCB(DteWr) < 2)
                    {
                        int j;

                        for (j = 0; i < pDteToT30->tempDataLen; i++, j++)
                        {
                            pDteToT30->tempData[j] = pDteToT30->tempData[i];
                        }

                        pDteToT30->tempDataLen = i - j;
                        pT30ToDte->dataCmd = DC3_XOFF;
                        return;
                    }
                    else
                    {
                        currByte = pDteToT30->tempData[i++];

                        if (needsBitRev)
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

                pDteToT30->tempDataLen = 0;
            }

            if (pDteToT30->postPgMsg != POST_PG_MSG_NULL)
            {
                if (GetAvailLenInCB(DteWr) < 2)
                {
                    pT30ToDte->dataCmd = DC3_XOFF;
                    return;
                }
                else
                {
                    UBYTE ppm;

                    PutByteToCB(DteWr, T50_DLE);

                    switch (pDteToT30->postPgMsg)
                    {
                        case POST_PG_MSG_EOP:
                            ppm = T32_EOP;
                            break;
                        case POST_PG_MSG_EOM:
                            ppm = T32_EOM;
                            break;
                        default:
                            ppm = T32_MPS;
                            break;
                    }

                    PutByteToCB(DteWr, ppm);
                    pDteToT30->postPgMsg = POST_PG_MSG_NULL;
                    pDsm->OperationStage = DTE_OS_COMMAND; TRACE0("DTE: ONLINE_FAX_COMMAND");
                }
            }

            if (pT30ToDte->dataCmd == NULL_DATA_CMD)
            {
                pT30ToDte->dataCmd = DC1_XON;
            }
        }

#endif
    }
}

#if SUPPORT_PARSER
void T30DownLinkInterfaceTest(GAO_Msg *pMsg, GAO_LibStruct *pFax)
{
    TDsmStrcut *pDsm = &(pFax->m_Dsm);
    CONST CHAR *pCommand;

    if (pMsg->MsgCode >= TEST_FDT && pMsg->MsgCode <= TEST_FIP)
    {
        pCommand = T32_Action_Command[pMsg->MsgCode - TEST_FDT + DTE_T30_FDT];
        PutStringToCB(&pFax->m_BufferOut, pCommand, strlen(pCommand));
    }
    else if (pMsg->MsgCode >= TEST_FCLASS && pMsg->MsgCode <= TEST_FBS)
    {
        pCommand = T32_Parm_Command[pMsg->MsgCode - TEST_FCLASS];
        PutStringToCB(&pFax->m_BufferOut, AT_PLUS_F, strlen(AT_PLUS_F));
        PutStringToCB(&pFax->m_BufferOut, pCommand, strlen(pCommand));
        pDsm->CurrCnfgCmd = pMsg->MsgCode;
    }
    else
    {
        return;
    }

    PutStringToCB(&pFax->m_BufferOut, "=?", 2);

    if (pMsg->MsgCode == TEST_FCLASS)
    {
        // LLL, temporary
        PutStringToCB(&pFax->m_BufferOut, ";I0", 3);
    }

    PutByteToCB(&pFax->m_BufferOut, T50_CR);
}

void T30DownLinkInterfaceRead(GAO_Msg *pMsg, GAO_LibStruct *pFax)
{
    CONST CHAR *pCommand;

    if (pMsg->MsgCode >= TEST_FCLASS && pMsg->MsgCode <= TEST_FBS)
    {
        pCommand = T32_Parm_Command[pMsg->MsgCode - TEST_FCLASS + DTE_T30_FCLASS];
        PutStringToCB(&pFax->m_BufferOut, AT_PLUS_F, strlen(AT_PLUS_F));
        PutStringToCB(&pFax->m_BufferOut, pCommand, strlen(pCommand));
    }
    else
    {
        return;
    }

    PutByteToCB(&pFax->m_BufferOut, '?');
    PutByteToCB(&pFax->m_BufferOut, T50_CR);
}
#endif

UBYTE T30DownLinkInterface(GAO_Msg *pMsg, GAO_LibStruct *pFax)
{
    switch (pMsg->MsgType)
    {
        case MSG_TYPE_CONTROL:
            T30DownLinkInterfaceCtrl(pMsg, pFax);
            break;
        case MSG_TYPE_CONFIG:
            return T30DownLinkInterfaceCfg(pMsg, pFax);
        case MSG_TYPE_IMAGE_DATA:
            T30DownLinkInterfaceImg(pMsg, pFax);
            break;
#if SUPPORT_T32_PARSER
        case MSG_TYPE_TEST:
            T30DownLinkInterfaceTest(pMsg, pFax);
            break;
        case MSG_TYPE_READ:
            T30DownLinkInterfaceRead(pMsg, pFax);
            break;
#endif
    }

    return TRUE;
}

void SendReportMsgSub(GAO_Msg *pMsg, UWORD MsgCode)
{
    pMsg->Direction = MSG_UP_LINK;
    pMsg->MsgType = MSG_TYPE_REPORT;
    pMsg->MsgCode = MsgCode;
}


void SendULImageMsgSub(GAO_Msg *pMsg, UWORD MsgCode)
{
    pMsg->Direction = MSG_UP_LINK;
    pMsg->MsgType = MSG_TYPE_IMAGE_DATA;
    pMsg->MsgCode = MsgCode;
}


void T30UpLinkInterface(GAO_Msg *pMsg, GAO_LibStruct *pFax)
{
#if SUPPORT_PARSER
    TDsmStrcut *pDsm = &(pFax->m_Dsm);
#endif
    T30ToDteInterface *pT30ToDte = &pFax->t30ToDte;
    T30SessionSubParm *pFcs = &(pT30ToDte->faxParmStatus);

    if (pMsg->Direction == MSG_DIR_IDLE)
    {
        if (pT30ToDte->tempDataLen > 0 &&
            pT30ToDte->resultCodeRsp != RESULT_CODE_CONNECT) // fixing issue 22, do NOT process image data untill after CONNECT
        {
            SendULImageMsgSub(pMsg, IMG_DATA_AND_STATUS);
            pMsg->Buf[0] = 0;
            pMsg->Buf[1] = 0;
            pMsg->Buf[2] = pT30ToDte->tempDataLen;

            if (RdReg(pFax->dteToT30.dataBitOrder, BIT0) == 0)
            {
                memcpy(&(pMsg->Buf[3]), pT30ToDte->tempData, pT30ToDte->tempDataLen);
            }
            else
            {
                UBYTE i;

                for (i = 0; i < pT30ToDte->tempDataLen; i++)
                {
                    pMsg->Buf[i + 3] = COMM_BitReversal(pT30ToDte->tempData[i]);
                }
            }

            pT30ToDte->tempDataLen = 0;
        }
        else if (pT30ToDte->isRmtDocToPoll)
        {
            pT30ToDte->isRmtDocToPoll = FALSE;
            SendReportMsgSub(pMsg, RPT_REMOTE_POLL_IND);
        }
        else if (pT30ToDte->infoTxtRsp != INFO_TXT_NULL)
        {
            UBYTE infoTxtRsp = pT30ToDte->infoTxtRsp;

            pT30ToDte->infoTxtRsp = INFO_TXT_NULL;

            switch (infoTxtRsp)
            {
                case INFO_TXT_FPS_FET:
                    // Need to first send FPS, and then FET
                    SendReportMsgSub(pMsg, RPT_RECV_POST_PAGE_RESP);
                    pMsg->Buf[0] = pT30ToDte->tempParm[0]; // page recv response
                    pMsg->Buf[1] = pT30ToDte->tempParm[1];
                    pMsg->Buf[2] = pT30ToDte->tempParm[2];
                    pT30ToDte->infoTxtRsp = INFO_TXT_FET;
                    pT30ToDte->tempParm[0] = pT30ToDte->tempParm[3];
                    break;
                case INFO_TXT_FTC:
                    SendReportMsgSub(pMsg, RPT_FTC_OF_REMOTE_SIDE);
                    break;
                case INFO_TXT_FET:
                    SendReportMsgSub(pMsg, RPT_RECV_POST_PAGE_CMD);
                    pMsg->Buf[0] = pT30ToDte->tempParm[0];
                    break;
                case INFO_TXT_FPS:
                    SendReportMsgSub(pMsg, RPT_RECV_POST_PAGE_RESP);
                    pMsg->Buf[0] = pT30ToDte->tempParm[0]; // page recv response
                    pMsg->Buf[1] = pT30ToDte->tempParm[1];
                    pMsg->Buf[2] = pT30ToDte->tempParm[2];
                    break;
                case INFO_TXT_FCS:
                    SendReportMsgSub(pMsg, RPT_NEGOTIATED_PARAS);
                    pMsg->Buf[0] = (1 << pFcs->pageWidthWD);

                    if (pFcs->verticalResolutionVR == VR_R8_385)
                    {
                        pMsg->Buf[1] = R8_X_RESOLUTION; // R8
                        pMsg->Buf[2] = VR_NORMAL_385; //
                    }
                    else if (pFcs->verticalResolutionVR == VR_R8_77)
                    {
                        pMsg->Buf[1] = R8_X_RESOLUTION; // R8
                        pMsg->Buf[2] = VR_FINE_77; //
                    }
                    else if (pFcs->verticalResolutionVR == VR_R8_154)
                    {
                        pMsg->Buf[1] = R8_X_RESOLUTION; // R8
                        pMsg->Buf[2] = VR_PERFECT_154; //
                    }
                    else if (pFcs->verticalResolutionVR == VR_R16_154)
                    {
                        pMsg->Buf[1] = R16_X_RESOLUTION;
                        pMsg->Buf[2] = VR_PERFECT_154;
                    }
                    else if (pFcs->verticalResolutionVR == VR_200_100)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_200DPI;
                        pMsg->Buf[2] = VR_100L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_100_100)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_100DPI;
                        pMsg->Buf[2] = VR_100L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_200_200)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_200DPI;
                        pMsg->Buf[2] = VR_200L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_200_400)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_200DPI;
                        pMsg->Buf[2] = VR_400L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_300_300)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_300DPI;
                        pMsg->Buf[2] = VR_300L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_400_400)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_400DPI;
                        pMsg->Buf[2] = VR_400L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_600_600)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_600DPI;
                        pMsg->Buf[2] = VR_600L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_1200_1200)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_1200DPI;
                        pMsg->Buf[2] = VR_1200L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_300_600)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_300DPI;
                        pMsg->Buf[2] = VR_600L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_400_800)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_400DPI;
                        pMsg->Buf[2] = VR_800L_INCH;
                    }
                    else if (pFcs->verticalResolutionVR == VR_600_1200)
                    {
                        pMsg->Buf[1] = X_RESOLUTION_600DPI;
                        pMsg->Buf[2] = VR_1200L_INCH;
                    }
                    else
                    {
                        TRACE0("GAOMAIN: FCS report Error: VR.");
                    }

                    if (pFcs->jpegOptionJP == JP_NON_JPEG)
                    {
                        pMsg->Buf[7] = 0;

                        if (pFcs->dataFormatDF == DF_MH)
                        {
                            pMsg->Buf[3] = DSM_DF_T4_1D;
                        }
                        else if (pFcs->dataFormatDF == DF_MR)
                        {
                            pMsg->Buf[3] = DSM_DF_T4_2D;
                        }
                        else if (pFcs->dataFormatDF == DF_MMR)
                        {
                            pMsg->Buf[3] = DSM_DF_T6_2D;
                        }
                        else
                        {
                            TRACE0("GAOMAIN: FCS report Error: Compression(B&W).");
                        }
                    }
                    else if (pFcs->jpegOptionJP & JP_ENABLE_JPEG)
                    {
                        pMsg->Buf[3] = DSM_DF_JPEG_MODE;
                        pMsg->Buf[7] = pFcs->jpegOptionJP;

                        if (pMsg->Buf[1] < X_RESOLUTION_100DPI)
                        {
                            TRACE0("GAOMAIN: Resolution unit should be inch in DCS.");

                            if (pMsg->Buf[1] == R8_X_RESOLUTION)
                            {
                                pMsg->Buf[1] = X_RESOLUTION_200DPI;
                            }
                            else if (pMsg->Buf[1] == R16_X_RESOLUTION)
                            {
                                pMsg->Buf[1] = X_RESOLUTION_400DPI;
                            }

                            if (pMsg->Buf[2] == VR_FINE_77)
                            {
                                pMsg->Buf[2] = VR_200L_INCH;
                            }
                            else if (pMsg->Buf[2] == VR_PERFECT_154)
                            {
                                pMsg->Buf[2] = VR_400L_INCH;
                            }
                        }
                    }
                    else
                    {
                        TRACE0("GAOMAIN: FCS report Error: Compression(Jpeg).");
                    }

                    pMsg->Buf[4] = pFcs->bitRateBR;
                    pMsg->Buf[5] = pFcs->errorCorrectionEC;
                    pMsg->Buf[6] = pFcs->scanTimeST;
                    pMsg->Buf[8] = pT30ToDte->faxProtocol;
                    break;
                case INFO_TXT_FHS:
                    SendReportMsgSub(pMsg, RPT_HANGUP_STATUS);
                    pMsg->Buf[0] = pT30ToDte->hangupCode;
#if SUPPORT_PARSER
                    pDsm->OperationStage = DTE_OS_CONFIG; TRACE0("DTE: ONLINE_FAX_CONFIG");
#endif
                    break;
            }
        }
        else if (pT30ToDte->resultCodeRsp != RESULT_CODE_NULL)
        {
            switch (pT30ToDte->resultCodeRsp)
            {
                case RESULT_CODE_OK:
                    pMsg->Buf[0] = DSM_RC_OK;
                    break;
                case RESULT_CODE_CONNECT:
                    pMsg->Buf[0] = DSM_RC_CONNECT;
                    break;
                case RESULT_CODE_ERROR:  // fixing issue 147
                    pMsg->Buf[0] = DSM_RC_ERROR;
                    break;
                case RESULT_CODE_NODIALTN:
                    pMsg->Buf[0] = DSM_RC_NODIAL_TONE;
                    break;
                case RESULT_CODE_BUSY:
                    pMsg->Buf[0] = DSM_RC_BUSY;
                    break;
                case RESULT_CODE_NOANS:
                    pMsg->Buf[0] = DSM_RC_NO_ANSWER;
                    break;
                case RESULT_CODE_RING:
                    pMsg->Buf[0] = DSM_RC_RING;
                    break;
            }

            SendReportMsgSub(pMsg, RPT_RESULT_CODE);
            pT30ToDte->resultCodeRsp = RESULT_CODE_NULL;
        }

#if SUPPORT_PARSER
        else if (pT30ToDte->tempParmLen > 0 && pDsm->CurrCnfgCmd != 0)
        {
            SendReportMsgSub(pMsg, pDsm->CurrCnfgCmd);
            memcpy(pMsg->Buf, pT30ToDte->tempParm, pT30ToDte->tempParmLen);
            pT30ToDte->tempParmLen = 0;
        }

#endif
    }
}


void DsmImgT30InterfaceMain(GAO_LibStruct *pFax)
{
    GAO_Msg *pMsg = &(pFax->m_ImgDce);

    // from pMsg to pDteToT30
    if (pMsg->Direction == MSG_DOWN_LINK)
    {
        if (T30DownLinkInterface(pMsg, pFax))
        {
            if ((pMsg->MsgType != MSG_TYPE_IMAGE_DATA) ||
                (pMsg->MsgCode != IMG_DATA_AND_STATUS)) //do not reset msg buffer for image data message
            {
                memset(pMsg, 0, sizeof(GAO_Msg));
            }
        }
    }
    else if (pMsg->Direction == MSG_DIR_IDLE) // from pT30ToDte to pMsg
    {
        T30UpLinkInterface(pMsg, pFax);
    }
}

#endif
