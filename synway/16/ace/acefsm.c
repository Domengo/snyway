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

/*******************************************************************
* Description:   AT command processing function including class 1  *
* which can get command from keyborad / serial port                *
* Author:        Yalan Xu                                          *
*******************************************************************/

#include <string.h>
#include "aceext.h"
#include "dcestru.h"
#include "hdlcext.h"

#if SUPPORT_V34FAX
static void Ace_AppendOnes_V34Fax(HdlcStruct *pHDLC_TX, UBYTE num)
{
    UWORD word, uTemp;
    UBYTE i, byte1, byte2;

    word  = 0xFFFF >> (pHDLC_TX->RawDataEnd & 0x07);
    byte1 =    word   >> 8;
    byte2 =    word   &  0xFF;

    uTemp = pHDLC_TX->RawDataEnd >> 3;

    for (i = 0; i < num; i++)
    {
        pHDLC_TX->RawData[uTemp + i    ] |= byte1;

        pHDLC_TX->RawData[uTemp + i + 1]  = byte2;
    }

    pHDLC_TX->RawDataEnd += (num << 3);
}

static void Ace_FsmTX1(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];

    switch (pDceToT30->selectedV34Chnl)
    {
        case V34_CONTROL_CHNL:

            if (pAscCBRd->CurrLen < 2)
            {
                HdlcStruct *pHDLC_TX = &pAce->T30HDLC_TX;
                int j;

                Ace_AppendOnes_V34Fax(pHDLC_TX, 2);
                Ace_TxDataToModem(pAscCBRd, pHDLC_TX->RawData, 2);

                for (j = 0; j <= (pHDLC_TX->RawDataEnd >> 3) - 2; j++)
                {
                    pHDLC_TX->RawData[j] = pHDLC_TX->RawData[2 + j];
                }

                pHDLC_TX->RawDataEnd -= 2 << 3;
            }

            pAce->isXmtingHdlc = FALSE;
            pAce->isRcvingHdlc = FALSE;
            break;
        case V34_PRIMARY_CHNL:

            if (pAce->isDleEotRcved)
            {
                Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
                Ace_ChangeState(pAce, ACE_NOF);
                PutReg(pDpcs->MCF, DISCONNECT);
            }
            else
            {
                Ace_ChangeState(pAce, ACE_FTH_HS);
                pAce->prevBufLen = 1;
            }

            pAce->isXmtingHdlc = FALSE;
            pAce->isRcvingHdlc = FALSE;
            break;
    }
}

static void Ace_FsmP2S(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];

    if (pAscCBRd->CurrLen < 2)
    {
        pAce->V34Fax_State = V34FAX_TX_ctrl;
        Ace_ChangeState(pAce, ACE_S2C);
        pAce->isXmtingHdlc = FALSE;
    }
}

static void Ace_FsmS2C(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;

    if (pDceToT30->selectedV34Chnl == V34_CONTROL_CHNL)
    {
        Ace_ChangeState(pAce, ACE_FTH);
    }
}
#endif

void Ace_Fsm(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
#if SUPPORT_T31
    HdlcStruct *pHDLC_TX = &pAce->T30HDLC_TX;
#if SUPPORT_T38GW
    UWORD prevRdEnd = pHDLC_TX->RawDataEnd;
#endif
#endif

    switch (pAce->classState)
    {
#if SUPPORT_T31
        case ACE_FRS:       Ace_FsmFRS(pTable); break;
        case ACE_FRM:       Ace_FsmFRM(pTable); break;
        case ACE_FTM:       Ace_FsmFTM(pTable); break;
        case ACE_FTS:       Ace_FsmFTS(pTable); break;
        case ACE_FRH:
        case ACE_FRH_HS:    Ace_FsmFRH(pTable); break;
        case ACE_FTH:
        case ACE_FTH_HS:    Ace_FsmFTH(pTable); break;
#if SUPPORT_V34FAX
        case ACE_A8E:       Ace_FsmA8E(pTable); break;
        case ACE_TX1S:      Ace_FsmTX1(pTable); break;
        case ACE_P2S:       Ace_FsmP2S(pTable); break;
        case ACE_S2C:       Ace_FsmS2C(pTable); break;
#endif
#endif
        default: break;
    }

#if SUPPORT_T31

    /* insert flags when nothing to send */
    if (pAce->isXmtingHdlc)
    {
        // Still sending. Forecast errors or ending
        CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];
        DceToT30Interface *pDceToT30 = pAce->pDceToT30;
        DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
        DceStruct *pDCE = (DceStruct *)pTable[DCE_STRUC_IDX];
        UWORD rdBytes = pHDLC_TX->RawDataEnd >> 3;
        UWORD totalBytes = pAscCBRd->CurrLen + rdBytes;
        SWORD fillBits = (SWORD)pDpcs->BITSPERCYL - pDCE->numBitRd - (totalBytes << 3);
        SWORD ret;
        int i, j;
#if SUPPORT_V34FAX
        UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

        if (fillBits > 0)
        {
            UBYTE fillBytes = (UBYTE)((fillBits + 8) >> 3);

            switch (pHDLC_TX->Status)
            {
                case HDLC_INITIALIZED:
                case HDLC_FRAMESTART_FOUND:
                    HDLC_AppendFlags(pHDLC_TX, fillBytes);
                    break;
                case HDLC_DATA_FOUND:
#if SUPPORT_T38GW
                    {
                        UWORD temp = 0x7e7e;
                        UBYTE flag;

                        temp >>= prevRdEnd & 0x7;
                        flag = temp & 0xFF;

                        for (i = 0; i < fillBytes; i++)
                        {
                            Ace_TxDataToModem(pAscCBRd, &flag, 1);
                        }
                    }
#else
                    HDLC_AppendFlags(pHDLC_TX, fillBytes);
#endif
                    break;
                case HDLC_FRAMEEND_FOUND:
#if SUPPORT_V34FAX

                    if (isV34Selected)
                    {
                        HDLC_AppendFlags(pHDLC_TX, fillBytes);
                    }
                    else
#endif
                        if (totalBytes == 0)
                        {
                            Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
                            PutReg(pDpcs->MCF, DISCONNECT);/* T30 Stop Fax */
                            pAce->isXmtingHdlc = FALSE;
                            Ace_ChangeState(pAce, ACE_NOF);
                            return;
                        }

                    break;
#if SUPPORT_V34FAX
                case HDLC_STREAMEND_FOUND:
                    break;
#endif
            }

            rdBytes = pHDLC_TX->RawDataEnd >> 3;
        }

        if (rdBytes > 0)
        {
            /* send data to Data Pump */
            ret = Ace_TxDataToModem(pAscCBRd, pHDLC_TX->RawData, rdBytes);

            if ((ret > 0) && (ret <= rdBytes))
            {
                // rdBytes > 0
                /* if some more data remains to be sent */
                for (j = 0; j <= rdBytes - ret; j++)
                {
                    pHDLC_TX->RawData[j] = pHDLC_TX->RawData[ret + j];
                }

                for (i = j; i < HDLCBUFLEN; i++)
                {
                    pHDLC_TX->RawData[i] = 0;
                }

                pHDLC_TX->RawDataEnd -= ret << 3;
            }
        }

        if ((pHDLC_TX->RawDataEnd >> 3) > T30_THRESHOLD)
        {
            pDceToT30->flowCntlCmd = DC3_XOFF;
        }
        else
        {
            pDceToT30->flowCntlCmd = DC1_XON;
        }
    }

#endif
}
