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
#include "hdlcext.h"
#include "common.h"

#if SUPPORT_T31
UBYTE Ace_FRH(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    if ((pAce->classState != ACE_NOF &&
         pAce->classState != ACE_FRH)
#if SUPPORT_V34FAX
        || pAce->pDceToT30->isV34Selected
#endif
       )
    {
        TRACE0("ACE: ERROR. +FRH");
    }

    if (pAce->wasCarrLoss ||
        (RdReg(pDpcs->MSR0, DISC_DET) && pAce->classState == ACE_FRH)) // fixing bug 25, need to check for this race condition, DISC_DET will be reset in Ace_SetupFax
    {
        pAce->currentT30DceCmd = T30_DCE_FRH;
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_NOCARRIER);
        return SUCCESS;
    }

    if (Ace_SetupFax(pTable) == FAIL)
    {
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
        return FAIL;
    }

    SetReg(pDpcs->MCR0, ANSWER);                 /* ANS mode */
    SetReg(pDpcs->MFCF0, TX_RX);

    Ace_ChangeState(pAce, ACE_FRH);
    pAce->currentT30DceCmd = T30_DCE_FRH;
    pAce->pT30ToDce->flowCntlCmd = NULL_DATA_CMD;

    return SUCCESS;
}

static SWORD Ace_RxDataFromModem(CircBuffer *pAscCBWr, UBYTE *OutBuf, UWORD len)
{
    SWORD    strlen, sTemp;
    UBYTE    ch ;

    strlen = pAscCBWr->CurrLen;

    sTemp = 0;

    if (strlen)
    {
        if (strlen > len)
        {
            strlen = len;
        }

        sTemp = strlen;

        while (strlen --)
        {
            GetByteFromCB(pAscCBWr, &ch);
            *OutBuf++ = COMM_BitReversal(ch);
        }
    }

    return sTemp;
}

/******************************************************
* Receive raw data from Data Pump, append the data
* at the end of the buffer.
*******************************************************/
extern UDWORD DumpPCMinCount;
static SWORD Ace_Rx_From_DataPump(HdlcStruct *pHDLC_RX, CircBuffer *pAscCBWr)
{
    SWORD    byteNum;
    UBYTE    *p;
    UWORD    space_left;

    /* Need to be checked here. */
    if (((pHDLC_RX->RawDataEnd >> 3) > 100) && ((pHDLC_RX->FrameStart >> 3) > 10)) /* This is to fix the big bug. */
    {
        HDLC_CleanRD(pHDLC_RX);
    }

    space_left = (HDLCBUFLEN - (pHDLC_RX->RawDataEnd >> 3) - 1);

    /* get the last byte in raw data buffer      */
    p = pHDLC_RX->RawData + (pHDLC_RX->RawDataEnd >> 3);

    /* append data at the end of the raw data */
    byteNum = Ace_RxDataFromModem(pAscCBWr, p, space_left);

    if (byteNum > 0)/* there data coming from modem */
    {
        if (0)
        {
            int i;
            TRACE("%8d: ACE: Rx: ", DumpPCMinCount);
            for (i = 0; i < byteNum; i ++)
            {
                TRACE("%02x ", p[i]);
            }
            TRACE0("");
        }

        pHDLC_RX->RawDataEnd += byteNum << 3;
    }

    return byteNum;
}

static UBYTE Ace_HDLC_Rx(ACEStruct *pAce, HdlcStruct *pHDLC_RX)
{
    UWORD BuffLen;
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#else
    UBYTE isV34Selected = FALSE;
#endif
    UBYTE buff[256];
    UWORD uLen;
    UBYTE uRet;
    UBYTE prevStatus = pHDLC_RX->Status;
    UBYTE isToStopHdlcRx = FALSE, isErr = FALSE;

#if SUPPORT_V34FAX

    if (!isV34Selected)
#endif
    {
        if (pAce->currentT30DceCmd == T30_DCE_NULL)
        {
            return isToStopHdlcRx;
        }
        else if (pAce->isOptFrm)
        {
            pAce->isOptFrm = FALSE;
            Ace_SetClass1ResultCode(pAce, RESULT_CODE_CONNECT);
            return isToStopHdlcRx;
        }
    }

    HDLC_IdentifyFrame(pHDLC_RX, !isV34Selected);

    switch (pHDLC_RX->Status)
    {
        case HDLC_INITIALIZED:
            pDceToT30->tempDataType = DCE_T30_DATA_NULL;
            break;
        case HDLC_FRAMESTART_FOUND:
            pDceToT30->tempDataType = DCE_T30_DATA_NULL;

#if SUPPORT_V34FAX

            if (!isV34Selected)
#endif
            {
                if (prevStatus == HDLC_INITIALIZED)
                {
                    pAce->hdlcTypeRx = HDLC_TYPE_INIT; // fixing bug 34, avoid sending CONNECT twice
                    Ace_SetClass1ResultCode(pAce, RESULT_CODE_CONNECT);
                }
            }

            break;
        case HDLC_DATA_FOUND:

            // fixing bug 34, send CONNECT after a corrupted optional frame
            if (pAce->hdlcTypeRx == HDLCCFCERR)
            {
                pAce->hdlcTypeRx = HDLC_TYPE_INIT;
#if SUPPORT_V34FAX

                if (!isV34Selected)
#endif
                {
                    Ace_SetClass1ResultCode(pAce, RESULT_CODE_CONNECT);
                    break;
                }
            }
            else if (pAce->isToRcvOnes) // fixing issue 169
            {
                // The previous response is lost, so the remote side is re-sending the command
                // cannot set ERROR //Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
                // It's for V.21 //pDceToT30->infoTxtRsp = INFO_TXT_FCERROR;
                pAce->isToRcvOnes = FALSE;
                break;
            }

            uLen = 0;
            uRet = HDLC_DataFound(pHDLC_RX, buff, &uLen); // HDLC_RX_FLAG | HDLC_RX_DATA or HDLC_RX_FCS_BAD (not decodable)

            if (uRet & HDLC_RX_FCS_BAD ||
                pAce->hdlcLen + uLen > sizeof(pAce->hdlcBuf)) // fixing bug 37, avoid buffer overflow
            {
                HDLC_Init(pHDLC_RX);
                pAce->hdlcLen = 0;
                isErr = TRUE;
            }
            else if (uLen != 0)
            {
                memcpy(pDceToT30->tempData, buff, uLen);
                memcpy(&pAce->hdlcBuf[pAce->hdlcLen], buff, uLen);
                pAce->hdlcLen += uLen;
                pDceToT30->tempDataType = DCE_T30_HDLC_DATA;
                pDceToT30->tempDataLen = uLen;
            }
            else
            {
                pDceToT30->tempDataType = DCE_T30_DATA_NULL;
            }

            break;
        case HDLC_FRAMEEND_FOUND:

            if (HDLC_Decode(pHDLC_RX))
            {
                BuffLen = pHDLC_RX->WorkingBuffEnd >> 3;
                memcpy(pDceToT30->tempData, pHDLC_RX->WorkingBuff, BuffLen);
                memcpy(&pAce->hdlcBuf[pAce->hdlcLen], pHDLC_RX->WorkingBuff, BuffLen);
                pAce->hdlcLen += BuffLen;
                pDceToT30->tempDataLen = BuffLen;

                if (HDLC_CheckCRC(pHDLC_RX))
                {
                    UBYTE isLastFrm = (pAce->hdlcBuf[1] == HDLC_CONTR_FIELD_LASTFRM);
                    pAce->isOptFrm = !isLastFrm;
                    pDceToT30->tempDataType = DCE_T30_HDLC_END;
                    pAce->hdlcTypeRx = HDLC_GetT30Type(&pAce->hdlcBuf[2]);

                    if (pAce->classState == ACE_FRH)
                    {
                        // VentaFax sets last frame in the last RCP frame
#if SUPPORT_V34FAX
                        if (isV34Selected)
                        {
                            if (pAce->hdlcTypeRx == HDLC_TYPE_DCN)
                            {
                                pAce->isToTurnOffCarrier = TRUE;
                                Ace_ChangeState(pAce, ACE_NOF);
                            }
                            else
                            {
                                pAce->isToTurnOffCarrier = FALSE;

                                if (isLastFrm)
                                {
                                    Ace_ChangeState(pAce, ACE_FTH);
                                }
                            }
                        }
                        else
#endif
                        {
                            if (isLastFrm && pAce->hdlcTypeRx != HDLC_TYPE_CRP)
                            {
                                isToStopHdlcRx = TRUE;
                            }

                            Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
                        }
                    }
                }
                else
                {
                    isErr = TRUE;
                }
            }
            else
            {
                isErr = TRUE;
            }

            HDLC_CleanRD(pHDLC_RX);
            HDLC_ResetWB(pHDLC_RX);
            pHDLC_RX->CRC = 0xFFFF;
            pAce->hdlcLen = 0;
            break;
    }

    if (isErr)
    {
        pAce->hdlcTypeRx = HDLCCFCERR;
        pDceToT30->tempDataLen = 0;
        pDceToT30->tempDataType = DCE_T30_HDLC_BAD;
#if SUPPORT_V34FAX

        if (isV34Selected)
        {
            if (pAce->classState == ACE_FRH)
            {
                Ace_ChangeState(pAce, ACE_FTH);
            }
        }
        else
#endif
        {
            Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
        }
    }

    return isToStopHdlcRx;
}

void Ace_FsmFRH(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE isToStopHdlcRx = FALSE;
    HdlcStruct *pHDLC_RX = &pAce->T30HDLC_RX;
#if SUPPORT_V34FAX
    HdlcStruct *pHDLC_TX = &pAce->T30HDLC_TX;
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

    switch (pT30ToDce->flowCntlCmd)
    {
        case CAN_STOP:
            pT30ToDce->flowCntlCmd = NULL_DATA_CMD;

#if SUPPORT_V34FAX

            if (isV34Selected)
            {
                if (pAce->classState == ACE_FRH)
                {
                    Ace_ChangeState(pAce, ACE_FTH);
                }
                else
                {
                    Ace_ChangeState(pAce, ACE_NOF);
                }
            }
            else
#endif
            {
                PutReg(pDpcs->MCF, DISCONNECT);
                Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK); // fixing bug 24
                Ace_ChangeState(pAce, ACE_NOF);
            }

            isToStopHdlcRx = TRUE;
            break;
        case DC1_XON:
            break;
        case DC3_XOFF:
            isToStopHdlcRx = TRUE;
            break;
    }

    if (isToStopHdlcRx)
    {
        pAce->isRcvingHdlc = FALSE;
        return;
    }

    if (!pAce->isRcvingHdlc)
    {
#if SUPPORT_V34FAX

        if (isV34Selected)
        {
            if (pAce->classState == ACE_FRH)
            {
                if (!pAce->isXmtingHdlc)
                {
                    pAce->isXmtingHdlc = TRUE;
                    HDLC_Init(pHDLC_TX);
                }

                Ace_V34StartMPET(pDpcs);
                TRACE0("ACE: V34 Fax start Modem Parameter Exchange for Rx");
            }
            else if (pDceToT30->selectedV34Chnl == V34_CONTROL_CHNL)
            {
                return;
            }
        }
        else
#endif
        {
            if (pAce->classState == ACE_FRH)
            {
                if (pAce->isToAvtEcho)
                {
                    if (pAce->timerSilence == 0) // fixing issue 123
                    {
                        pAce->timerSilence = 1; TRACE0("ACE: Echo delay starts");
                        return;
                    }
                    else if (pAce->timerSilence <= ECHO_DELAY)
                    {
                        pAce->timerSilence++;
                        return;
                    }
                    else
                    {
                        pAce->isToAvtEcho = FALSE;
                        pAce->timerSilence = 0; TRACE0("ACE: Echo delay elapsed");
                    }
                }

                SetReg(pDpcs->MFCF0, TX_RX);
                Ace_V21StartFaxModem(pDpcs);
                TRACE0("ACE: V21StartFax V21 Rx");
            }
        }

        HDLC_Init(pHDLC_RX);
        pAce->hdlcLen = 0; // fixing bug 31, need to zero the length. It's here because ATD implies FRH
        pAce->isRcvingHdlc = TRUE;
        ClrReg(pDpcs->MSR0);
    }
    else
    {
        CircBuffer *pAscCBWr = (CircBuffer *)pTable[ASCCBWRDATA_IDX];
        UBYTE isOnesRcved = FALSE;
        SWORD byteNum = Ace_Rx_From_DataPump(pHDLC_RX, pAscCBWr);
        UBYTE mode = RdReg(pDpcs->MSR0, DISC_DET);

#if SUPPORT_V34FAX

        if (pAce->classState == ACE_FRH)
        {
            if (isV34Selected && byteNum > 0 && pAce->isToRcvOnes)
            {
                UBYTE *pRdEnd = &pHDLC_RX->RawData[pHDLC_RX->RawDataEnd >> 3];
                int i;

                for (i = 0; i < byteNum; i++)
                {
                    if (*pRdEnd-- == 0xff)
                    {
                        isOnesRcved = TRUE;
                        pAce->isToRcvOnes = FALSE;
                        pAce->isRcvingHdlc = FALSE;
                        pAce->isXmtingHdlc = FALSE;
                        Ace_ChangeState(pAce, ACE_FRH_HS);
                        pAce->V34Fax_State = V34FAX_RX_ONES;
                        break;
                    }
                }
            }
        }
        else if (pDceToT30->selectedV34Chnl == V34_CONTROL_CHNL)
        {
            Ace_ChangeState(pAce, ACE_FRH);
            pAce->isOptFrm = FALSE;
            pAce->isRcvingHdlc = FALSE;
        }

#endif

        if (!(isOnesRcved || (pHDLC_RX->WorkingBuffEnd == 0 && byteNum <= 0)))
        {
            // fixing issue 128, needs to process the trailing flags if any
            if (Ace_HDLC_Rx(pAce, pHDLC_RX))
            {
                pAce->wasCarrLoss = TRUE; // fixing bug 1, anyway the carrier is gone

                if (!mode)
                {
                    PutReg(pDpcs->MCF, DISCONNECT); // stop V.21
                }
            }
        }
        else if (mode)
        {
            ClrReg(pDpcs->MSR0);

            if (RdReg(pDpcs->MFSR0, FAX_V21))
            {
                ClrReg(pDpcs->MFSR0);
                PutReg(pDpcs->MCF, DISCONNECT);/* T30 Stop Fax */
                Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
                pDceToT30->infoTxtRsp = INFO_TXT_FCERROR; TRACE0("ACE: +FCERROR");
            }
            else
            {
                if (pAce->currentT30DceCmd == T30_DCE_NULL)
                {
                    pAce->wasCarrLoss = TRUE; // if next command is FRH, no carrier will be reported. Otherwise, it will be cleared
                }
                else
                {
                    Ace_SetClass1ResultCode(pAce, RESULT_CODE_NOCARRIER);
                }

                pAce->isOptFrm = FALSE;
                pAce->isRcvingHdlc = FALSE;
            }

            Ace_ChangeState(pAce, ACE_NOF);
        }
    }
}

#if SUPPORT_T31_PARSER
UBYTE CLASS1_FRH(UBYTE **pTable)
{
    return Ace_ParseMod(pTable, T30_DCE_FRH);
}
#endif
#endif
