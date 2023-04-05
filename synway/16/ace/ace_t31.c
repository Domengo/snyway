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
#if SUPPORT_V34FAX
#include "v8ext.h"
#include "v34ext.h"
#endif
#include "gaoapi.h"
#include "dceext.h"
#include "hwapi.h"
#include "hdlcext.h"
#include "mhsp.h"
#include "v251.h"
#include "common.h"

#if SUPPORT_T31
UBYTE Ace_SetupFax(UBYTE **pTable)
{
    UBYTE Mod;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    Mod = pAce->pT30ToDce->tempParm[0];

    switch (Mod)
    {
        case MOD_V21_300:
            SetReg(pDpcs->MFSR0, FAX_HDLC);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);
            PutReg(pDpcs->MMR0, V21CH2);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_300);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_300);
            break;
        case MOD_V27TER_2400:
            PutReg(pDpcs->MMR0, V27TER);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_2400);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_2400);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);
            break;
        case MOD_V27TER_4800:
            PutReg(pDpcs->MMR0, V27TER);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_4800);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_4800);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);
            break;
        case MOD_V29_7200:
            PutReg(pDpcs->MMR0, V29);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_7200);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_7200);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);
            break;
        case MOD_V17_L_7200:
            PutReg(pDpcs->MMR0, V17);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_7200);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_7200);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);         /* long train */
            break;
        case MOD_V17_S_7200:
            PutReg(pDpcs->MMR0, V17);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_7200);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_7200);
            SetReg(pDpcs->MFCF0, TCF_OR_IMAGE);         /* short train */
            break;
        case MOD_V29_9600:
            PutReg(pDpcs->MMR0, V29);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_9600);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_9600);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);
            break;
        case MOD_V17_L_9600:
            PutReg(pDpcs->MMR0, V17);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_9600);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_9600);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);         /* long train */
            break;
        case MOD_V17_S_9600:
            PutReg(pDpcs->MMR0, V17);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_9600);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_9600);
            SetReg(pDpcs->MFCF0, TCF_OR_IMAGE);         /* short train */
            break;
        case MOD_V17_L_12000:
            PutReg(pDpcs->MMR0, V17);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_12000);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_12000);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);         /* long train */
            break;
        case MOD_V17_S_12000:
            PutReg(pDpcs->MMR0, V17);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_12000);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_12000);
            SetReg(pDpcs->MFCF0, TCF_OR_IMAGE);         /* short train */
            break;
        case MOD_V17_L_14400:
            PutReg(pDpcs->MMR0, V17);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_14400);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_14400);
            ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);         /* long train */
            break;
        case MOD_V17_S_14400:
            PutReg(pDpcs->MMR0, V17);
            WrReg(pDpcs->MBC0, MIN_RATE, DATA_RATE_14400);
            WrReg(pDpcs->MBC1, MAX_RATE, DATA_RATE_14400);
            SetReg(pDpcs->MFCF0, TCF_OR_IMAGE);         /* short train */
            break;
        default:
            TRACE0("ACE: ERROR. Mod Error");
            return FAIL;
    }

    pAce->bytesPer10ms = (RdReg(pDpcs->MBC1, MAX_RATE) - DATA_RATE_2400 + 1) * 3;

    SetReg(pDpcs->MCR0, FAX);        /* FAX mode */
    ResetReg(pDpcs->MFCF0, CHAR_OR_HDLC);
    ResetReg(pDpcs->MSR0, DISC_DET);

    return SUCCESS;
}

#if SUPPORT_V34FAX
UBYTE DLE_PRI(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    UWORD       *pClk = (UWORD *)pTable[CLOCKDATA_IDX];

    if (pAce->classState != ACE_FTH || !pDceToT30->isV34Selected)
    {
        TRACE0("ACE: ERROR. <DLE><PRI>");
    }

    if (pDceToT30->isV34Selected)
    {
        pAce->V34Fax_State = V34FAX_TX_ONES2;
        /* set class 1 state */
        Ace_ChangeState(pAce, ACE_TX1S);
        pAce->StartTime = *pClk;

        if (pAce->timerSilence != 0)
        {
            // fixing issue 193
            pAce->timerSilence = 0; TRACE0("ACE: V.34 HDLC TX timer reset");
        }
    }

    return SUCCESS;
}

UBYTE DLE_CTRL(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    UWORD       *pClk = (UWORD *)pTable[CLOCKDATA_IDX];

    if (pDceToT30->isV34Selected)
    {
        HdlcStruct *pHDLC_TX = &pAce->T30HDLC_TX;

        if (pHDLC_TX->Status != HDLC_FRAMEEND_FOUND)
        {
            // flags after the last RCP have not been sent out
            HDLC_AppendFlags(pHDLC_TX, NFLAG_EN);
        }

        pHDLC_TX->Status = HDLC_STREAMEND_FOUND; // fixing bug 10
        /* set class 1 state */
        Ace_ChangeState(pAce, ACE_P2S);
        pAce->StartTime = *pClk;
    }

    return SUCCESS;
}

UBYTE DLE_RTNC(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;

    if (pDceToT30->isV34Selected)
    {
        // fixing issue 173
        V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
        V34FaxStruct *pV34Fax = &(pV34->V34Fax);

        pV34Fax->MPET_Retrning = 0;
        pV34Fax->MPET_Retrn_Type = RETRAIN_INIT;
        TRACE0("ACE: <DLE><RTNC>");
    }

    return SUCCESS;
}

UBYTE DLE_EOT(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;

    if (pDceToT30->isV34Selected)
    {
        if (pDceToT30->selectedV34Chnl == V34_CONTROL_CHNL)
        {
            pAce->V34Fax_State = V34FAX_TX_ONES2;
            Ace_ChangeState(pAce, ACE_TX1S);
            pAce->isDleEotRcved = TRUE;
        }
        else
        {
            DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

            Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
            PutReg(pDpcs->MCF, DISCONNECT);/* T30 Stop Fax */
        }
    }

    return SUCCESS;
}
#endif

SWORD Ace_TxDataToModem_0(CircBuffer *pAscCBRd)
{
    SWORD sRemainLen;

    if (pAscCBRd->CurrLen == 0)
    {
        sRemainLen = -1;
    }
    else
    {
        sRemainLen = (SWORD)(pAscCBRd->Len - pAscCBRd->CurrLen);
    }

    return sRemainLen;
}

SWORD Ace_TxDataToModem(CircBuffer *pAscCBRd, UBYTE *InBuf, UWORD len)
{
    SWORD strlen, sTemp;
    UBYTE ch;

    if (len)
    {
        strlen = (SWORD)(pAscCBRd->Len - pAscCBRd->CurrLen);

        if (strlen <= len)
        {
            strlen = 0;

            //TRACE1("???loss %d char", len);
        }
        else
        {
            strlen = len;
        }

        sTemp = strlen;

        /// LLL debug
        //TRACE("ACE: Tx: ", 0);
        while (strlen --)
        {
            ch = COMM_BitReversal(*InBuf);

            /// LLL debug
            //TRACE("%02x ", ch);

            PutByteToCB(pAscCBRd, ch);

            InBuf ++;
        }

        /// LLL debug
        //TRACE0("");
    }
    else
    {
        sTemp = Ace_TxDataToModem_0(pAscCBRd);
    }

    return sTemp;
}

void Ace_V21StartFaxModem(DpcsStruct *pDpcs)
{
    PutReg(pDpcs->MCF, CONNECT);
}

void Ace_V34StartMPET(DpcsStruct *pDpcs)
{
    SetReg(pDpcs->MCR0, FAX);
    ResetReg(pDpcs->MFCF0, CHAR_OR_HDLC);
    ResetReg(pDpcs->MSR0, DISC_DET);
    SetReg(pDpcs->MFSR0, FAX_HDLC);
    ResetReg(pDpcs->MFCF0, TCF_OR_IMAGE);
}

void Ace_SetClass1ResultCode(ACEStruct *pAce, UBYTE response)
{
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;

    pAce->wasCarrLoss = FALSE;

    if (pAce->currentT30DceCmd != T30_DCE_NULL)
    {
        TRACE2("ACE: \"%s\" for %s", ResponseCode[response - 1], T31_Action_Command[pAce->currentT30DceCmd]);

        switch (response)
        {
            case RESULT_CODE_OK:
            case RESULT_CODE_ERROR:
            case RESULT_CODE_NOCARRIER:
                pAce->currentT30DceCmd = T30_DCE_NULL;

#if !SUPPORT_T31_PARSER

                if (pAce->LineState != OFF_LINE)
                {
                    Ace_ChangeLineState(pAce, ONLINE_FAX_COMMAND);
                }

                break;

            case RESULT_CODE_CONNECT:
                Ace_ChangeLineState(pAce, ONLINE_FAX_DATA);
#endif
                break;
        }
    }
    else if (response == RESULT_CODE_RING)
    {
        TRACE0("ACE: \"RING\"");
    }
    else
    {
        return;
    }

    pDceToT30->resultCodeRsp = response;
}

void Ace_GetClass1Command(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;

    if (pT30ToDce->v8OpCntl != A8E_V8O_0)
    {
        Ace_ChangeState(pAce, ACE_A8E);
    }
    else
    {
        switch (pT30ToDce->actionCmd)
        {
            case T30_DCE_ATD:
                Ace_ATD(pTable);
                break;
            case T30_DCE_ATA:
                Ace_ATA(pTable);
                break;
            case T30_DCE_ATH:
                Ace_ATH(pTable);
                break;
            case T30_DCE_ATO:
                Ace_ATO(pTable);
                break;
            case T30_DCE_FTS:
                Ace_FTS(pTable);
                break;
            case T30_DCE_FRS:
                Ace_FRS(pTable);
                break;
            case T30_DCE_FTM:
                Ace_FTM(pTable);
                break;
            case T30_DCE_FRM:
                Ace_FRM(pTable);
                break;
            case T30_DCE_FTH:
                Ace_FTH(pTable);
                break;
            case T30_DCE_FRH:
                Ace_FRH(pTable);
                break;
#if SUPPORT_V34FAX
            case T30_DCE_A8M:
                Ace_A8M(pTable);
                break;
            case T30_DCE_DLE_PRI:
                DLE_PRI(pTable);
                break;
            case T30_DCE_DLE_CTRL:
                DLE_CTRL(pTable);
                break;
            case T30_DCE_DLE_RTNC:
                DLE_RTNC(pTable);
                break;
            case T30_DCE_DLE_EOT:
                DLE_EOT(pTable);
                break;
#endif
            case T30_DCE_NULL:
                break;
            default:
                TRACE0("ACE: ERROR. pT30ToDce->actionCmd");
                break;
        }

        pT30ToDce->actionCmd = T30_DCE_NULL;
    }
}
#endif

