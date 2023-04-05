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

#if SUPPORT_T30
static UBYTE T30_MakeFrame(T30Struct *pT30, UWORD type)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    UBYTE len, i;

#if 0 // fixing issue 114

    if (type == HDLC_TYPE_CFR)
    {
        for (i = 0; i < 10; i++)
        {
            pT30->HDLC_TX_Buf[j++] = 0xFF;
            pT30->HDLC_TX_Buf[j++] = HDLC_CONTR_FIELD_LASTFRM;
            pT30->HDLC_TX_Buf[j++] = (UBYTE)(type >> 8);
        }

        len    = j;
    }
    else
#endif
    {
        pT30->HDLC_TX_Buf[0] = 0xFF;
        pT30->HDLC_TX_Buf[1] = HDLC_CONTR_FIELD_LASTFRM;
        pT30->HDLC_TX_Buf[2] = (UBYTE)(HDLC_CorrT30FCF(pT30->wasDisRcved, type) >> 8);

        len    = 3;
    }

    switch (type)
    {
        case HDLC_TYPE_NSF:
        case HDLC_TYPE_NSC:
        case HDLC_TYPE_NSS:
            pT30->HDLC_TX_Buf[1] = HDLC_CONTR_FIELD_NOTLASTFRM;

            for (i = 0; i < pDteToT30->nonStandardFrmLen; i++)
            {
                pT30->HDLC_TX_Buf[3 + i] = pDteToT30->nonStandardFrm[i];
            }

            len    += pDteToT30->nonStandardFrmLen;
            break;
        case HDLC_TYPE_PWD:
        case HDLC_TYPE_SID:
            pT30->HDLC_TX_Buf[1] = HDLC_CONTR_FIELD_NOTLASTFRM;

            for (i = 0; i < OPTFRAMELEN; i++)
            {
                if (pDteToT30->passWord[OPTFRAMELEN - 1 - i] == NUL)
                {
                    pT30->HDLC_TX_Buf[3 + i] = 0x04;
                }
                else
                {
                    pT30->HDLC_TX_Buf[3 + i] = COMM_BitReversal(pDteToT30->passWord[OPTFRAMELEN - 1 - i]);
                }
            }

            len    += OPTFRAMELEN;
            break;
        case HDLC_TYPE_SEP:
            pT30->HDLC_TX_Buf[1] = HDLC_CONTR_FIELD_NOTLASTFRM;

            for (i = 0; i < OPTFRAMELEN; i++)
            {
                if (pDteToT30->selectivePollingAddr[OPTFRAMELEN - 1 - i] == NUL)
                {
                    pT30->HDLC_TX_Buf[3 + i] = 0x04;
                }
                else
                {
                    pT30->HDLC_TX_Buf[3 + i] = COMM_BitReversal(pDteToT30->selectivePollingAddr[OPTFRAMELEN - 1 - i]);
                }
            }

            len += OPTFRAMELEN;
            break;
        case HDLC_TYPE_CSI:
        case HDLC_TYPE_TSI:
            pT30->HDLC_TX_Buf[1] = HDLC_CONTR_FIELD_NOTLASTFRM;

            for (i = 0; i < OPTFRAMELEN; i++)
            {
                if (pDteToT30->localID[OPTFRAMELEN - 1 - i] == NUL)
                {
                    pT30->HDLC_TX_Buf[3 + i] = 0x04;
                }
                else
                {
                    pT30->HDLC_TX_Buf[3 + i] = COMM_BitReversal(pDteToT30->localID[OPTFRAMELEN - 1 - i]);
                }
            }

            len    += OPTFRAMELEN;

            break;

        case HDLC_TYPE_CIG:
            pT30->HDLC_TX_Buf[1] = HDLC_CONTR_FIELD_NOTLASTFRM;

            for (i = 0; i < OPTFRAMELEN; i++)
            {
                if (pDteToT30->localPollingID[OPTFRAMELEN - 1 - i] == NUL)
                {
                    pT30->HDLC_TX_Buf[3 + i] = 0x04;
                }
                else
                {
                    pT30->HDLC_TX_Buf[3 + i] = COMM_BitReversal(pDteToT30->localPollingID[OPTFRAMELEN - 1 - i]);
                }
            }

            len    += OPTFRAMELEN;
            break;
        case HDLC_TYPE_SUB:
            pT30->HDLC_TX_Buf[1] = HDLC_CONTR_FIELD_NOTLASTFRM;

            for (i = 0; i < OPTFRAMELEN; i++)
            {
                if (pDteToT30->destSubAddr[OPTFRAMELEN - 1 - i] == NUL)
                {
                    pT30->HDLC_TX_Buf[3 + i] = 0x04;
                }
                else
                {
                    pT30->HDLC_TX_Buf[3 + i] = COMM_BitReversal(pDteToT30->destSubAddr[OPTFRAMELEN - 1 - i]);
                }
            }

            len    += OPTFRAMELEN;

            break;

        case HDLC_TYPE_DIS:
        case HDLC_TYPE_DTC:

            len = T30_MakeDIS_DTC(pT30, (UBYTE)(type == HDLC_TYPE_DIS));

            break;

        case HDLC_TYPE_DCS:

            len = T30_MakeDCS_CTC(pT30, FALSE);

            break;

#if SUPPORT_ECM_MODE
        case HDLC_TYPE_T4_PPR:

            len    += BLOCKNUM_BYTESIZE;

            break;
#endif

        default:

            break;
    }

    return  len;
}

UBYTE T30_CommandTx(T30Struct *pT30, UWORD hdlctype)
{
    UWORD len;
    UBYTE tmp;
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;

    // fixing issue 180
    if (pDteToT30->isDiscCmd && hdlctype != HDLC_TYPE_DCN)
    {
        T30_GotoPointC(pT30, HANGUP_ABORTED_FROM_FKS_OR_CAN);
        return 2;
    }

    if (!pT30->isHdlcMade)
    {
        len = T30_MakeFrame(pT30, hdlctype);

        if (len > 0)
        {
            pT30->isHdlcMade = TRUE;
#if SUPPORT_T38EP
            T30_LogHdlc(pT30->HDLC_TX_Buf, len, TRUE, TRUE);
#else
            T30_LogHdlc(pT30->HDLC_TX_Buf, len, TRUE, FALSE);
#endif
        }
    }

#if SUPPORT_V34FAX
    else if (pDceToT30->isV34Selected)
    {
        return 1;
    }

#endif
    else if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
    {
        pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
        return 1;
    }
    else
    {
        return 2;
    }

    tmp  = pT30->HDLC_TX_Buf[1];
    pT30ToDce->tempDataLen = (UBYTE)len;

    if (len != 0)
    {
        memcpy(pT30ToDce->tempData, pT30->HDLC_TX_Buf, len);
#if T30_RESPOND_BAD_CRP

        if (hdlctype == HDLC_TYPE_CRP && !pDceToT30->isV34Selected)
        {
            pT30ToDce->tempDataType = T30_DCE_HDLC_BAD_LAST;
        }
        else
#endif
        {
            pT30ToDce->tempDataType = T30_DCE_HDLC_OK;
        }
    }

    return 2;
}
#endif
