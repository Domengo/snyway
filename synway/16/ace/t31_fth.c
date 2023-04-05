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

#if SUPPORT_T31
UBYTE Ace_FTH(UBYTE **pTable)
{
    ACEStruct   *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UWORD       *pClk = (UWORD *)pTable[CLOCKDATA_IDX];

    if (pAce->classState != ACE_NOF
#if SUPPORT_V34FAX
        || pAce->pDceToT30->isV34Selected
#endif
       )
    {
        TRACE0("ACE: ERROR. +FTH");
    }

    if (Ace_SetupFax(pTable) == FAIL)
    {
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
        return FAIL;
    }

    ResetReg(pDpcs->MCR0, ANSWER);           /* CALL mode */
    ResetReg(pDpcs->MFCF0, TX_RX);           /* Transmit */

    /* set class 1 state */
    Ace_ChangeState(pAce, ACE_FTH);
    pAce->isToAvtEcho = TRUE;
    pAce->currentT30DceCmd = T30_DCE_FTH;
    pAce->pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
    /* Start timer */
    pAce->StartTime = *pClk;
    return SUCCESS;
}

void Ace_FsmFTH(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    HdlcStruct *pHDLC_TX = &pAce->T30HDLC_TX;
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

#if SUPPORT_V34FAX

    if (pDceToT30->isDleEot)
    {
        Ace_ChangeState(pAce, ACE_NOF);
        return;
    }

#endif

    if (pAce->isRcvingHdlc
#if SUPPORT_V34FAX
        && !isV34Selected
#endif
       )
    {
        pAce->isRcvingHdlc = FALSE;
        PutReg(pDpcs->MCF, DISCONNECT);
    }
    else if (!pAce->isXmtingHdlc)
    {
        if (pAce->classState == ACE_FTH)
        {
#if SUPPORT_V34FAX

            if (isV34Selected)
            {
                HDLC_Init(pHDLC_TX);
                Ace_V34StartMPET(pDpcs);
                HDLC_AppendFlags(pHDLC_TX, NFLAG_EN);
                TRACE0("ACE: V34 Fax start Modem Parameter Exchange for Tx");
            }
            else
#endif
            {
                HDLC_Init(pHDLC_TX);
                ResetReg(pDpcs->MFCF0, TX_RX);
                Ace_V21StartFaxModem(pDpcs);
                TRACE0("ACE: V21StartFax V21 Tx");
                HDLC_AppendFlags(pHDLC_TX, NFLAG_BE);
                Ace_SetClass1ResultCode(pAce, RESULT_CODE_CONNECT);
            }
        }
        else if (pAce->syncCount < 20)
        {
            // 200 ms flags
            if (pAce->syncCount == 0)
            {
#if SUPPORT_V34FAX

                if (isV34Selected)
                {
                    pAce->bytesPer10ms = T30_BytesPer10ms[pDceToT30->priChnlBitRate - 1];
                }

#endif
                HDLC_Init(pHDLC_TX);
            }

            HDLC_AppendFlags(pHDLC_TX, pAce->bytesPer10ms);

            if (Ace_TxDataToModem(pAscCBRd, pHDLC_TX->RawData, pAce->bytesPer10ms) > 0)
            {
                pAce->syncCount++;
            }

            pHDLC_TX->RawDataEnd = 0;
            return;
        }
        else
        {
            pAce->syncCount = 0;
        }

        pAce->hdlcLen = 0; // fixing bug 31, need to zero the length. It's here because ATA implies FTH
        pAce->isXmtingHdlc = TRUE;
        pHDLC_TX->Status = HDLC_FRAMESTART_FOUND;
    }
    else
    {
        UWORD uLen = pT30ToDce->tempDataLen;

        pT30ToDce->tempDataLen = 0;

        if (pT30ToDce->flowCntlCmd == CAN_STOP)
        {
            pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
            PutReg(pDpcs->MCF, DISCONNECT);/* T30 Stop Fax */
            pAce->isXmtingHdlc = FALSE;
            Ace_ChangeState(pAce, ACE_NOF);
        }
        else if (pT30ToDce->tempDataType == T30_DCE_HDLC_BAD_LAST ||
                 pT30ToDce->tempDataType == T30_DCE_HDLC_BAD_OPT)
        {
            uLen = 0;
            pHDLC_TX->Status = HDLC_FRAMEEND_FOUND;
            pHDLC_TX->RawDataEnd = 0;
            pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
        }

        if (uLen > 0)
        {
            if (uLen + pAce->hdlcLen <= sizeof(pHDLC_TX->WorkingBuff))
            {
                pHDLC_TX->Status = HDLC_DATA_FOUND;
                HDLC_FillWB(pHDLC_TX, pT30ToDce->tempData, uLen);
                memcpy(&pAce->hdlcBuf[pAce->hdlcLen], pT30ToDce->tempData, uLen);
                pAce->hdlcLen += uLen;

#if SUPPORT_V34FAX

                if (pAce->classState == ACE_FTH)
                {
                    if (isV34Selected && pAce->timerSilence != 0)
                    {
                        // fixing issue 193
                        pAce->timerSilence = 0; TRACE0("ACE: V.34 HDLC TX timer reset");
                    }
                }

#endif
            }
            else
            {
                TRACE0("ACE: ERROR. HDLC buffer overflow");
                pHDLC_TX->Status = HDLC_FRAMESTART_FOUND;
                pAce->hdlcLen = 0;
            }
        }

#if SUPPORT_V34FAX
        else if (isV34Selected && pAce->classState == ACE_FTH)
        {
            // fixing issue 193
            if (pAce->timerSilence == 0)
            {
                pAce->timerSilence = 1; TRACE0("ACE: V.34 HDLC TX timer starts");
            }
            else if (pAce->timerSilence++ > TIMER_V34_TX)
            {
                pAce->timerSilence = 0; TRACE0("ACE: V.34 HDLC TX timer elapsed");
                Ace_ChangeState(pAce, ACE_FRH);
            }
        }

#endif

        if (pT30ToDce->tempDataType == T30_DCE_HDLC_OK || pT30ToDce->tempDataType == T30_DCE_HDLC_END)
        {
            if (pHDLC_TX->Status != HDLC_FRAMEEND_FOUND)
            {
                HDLC_AppendCRC(pHDLC_TX);
            }

            pAce->hdlcLen = 0;
            pHDLC_TX->CRC = 0xFFFF;
            HDLC_Encode(pHDLC_TX);
            /* at lease 1 flags at the end */
            HDLC_AppendFlags(pHDLC_TX, NFLAG_IN);

            if (pAce->classState == ACE_FTH && pAce->hdlcBuf[1] == HDLC_CONTR_FIELD_LASTFRM)
            {
                /* LAST_FRAME */
                pHDLC_TX->Status = HDLC_FRAMEEND_FOUND;
                HDLC_AppendFlags(pHDLC_TX, NFLAG_EN);
#if SUPPORT_V34FAX

                if (isV34Selected)
                {
                    Ace_ChangeState(pAce, ACE_FRH);
                    pAce->isOptFrm = FALSE;

                    switch (HDLC_GetT30Type(&pAce->hdlcBuf[2]))
                    {
                        case HDLC_TYPE_CFR:
                        case HDLC_TYPE_MCF:
                        case HDLC_TYPE_T4_ERR:
                        case HDLC_TYPE_T4_CTR:
                        case HDLC_TYPE_T4_PPR:
                            pAce->isToRcvOnes = TRUE;
                            break;
                        case HDLC_TYPE_DTC:
                            pAce->isToTurnOffCarrier = TRUE;
                            pAce->isToRcvOnes = TRUE;
                            break;
                        default:
                            pAce->isToRcvOnes = FALSE;
                            break;
                    }
                }
                else
#endif
                    if ((pHDLC_TX->RawDataEnd) & 0x7)
                    {
                        pHDLC_TX->RawDataEnd += 16;
                    }
            }
            else if (pT30ToDce->tempDataType == T30_DCE_HDLC_END)
            {
                pHDLC_TX->Status = HDLC_FRAMEEND_FOUND;
                HDLC_AppendFlags(pHDLC_TX, NFLAG_EN);
            }
            else
            {
                pHDLC_TX->Status = HDLC_FRAMESTART_FOUND;
#if SUPPORT_V34FAX

                if (!isV34Selected)
#endif
                {
                    Ace_SetClass1ResultCode(pAce, RESULT_CODE_CONNECT);
                }
            }

#if !SUPPORT_T38GW // fixing bug 8, cannot encode a frame until it's fully received
            HDLC_ResetWB(pHDLC_TX);
#endif
        }

#if SUPPORT_T38GW
        else if (uLen > 0)
        {
            HDLC_Encode(pHDLC_TX);
        }

        HDLC_ResetWB(pHDLC_TX);
#endif
        pT30ToDce->tempDataType = T30_DCE_DATA_NULL;
        pHDLC_TX->OnesCount = 0;
    }
}

#if SUPPORT_T31_PARSER
UBYTE CLASS1_FTH(UBYTE **pTable)
{
    return Ace_ParseMod(pTable, T30_DCE_FTH);
}
#endif
#endif

