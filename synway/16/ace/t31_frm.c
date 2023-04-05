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

#include "aceext.h"
#include "gaoapi.h"

#if SUPPORT_T31
UBYTE Ace_FRM(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UWORD *pClk = (UWORD *)pTable[CLOCKDATA_IDX];

    if (pAce->classState != ACE_NOF
#if SUPPORT_V34FAX
        || pAce->pDceToT30->isV34Selected
#endif
       )
    {
        TRACE0("ACE: ERROR. +FRM");
    }

    if (Ace_SetupFax(pTable) == FAIL)
    {
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
        return FAIL;
    }

    SetReg(pDpcs->MCR0, RECEIVE_IMAGE);
    SetReg(pDpcs->MFCF0, TX_RX);       /* receive */
    PutReg(pDpcs->MCF, CONNECT);
    ResetReg(pDpcs->MFCF0, CARRIER_GOT);

    /* set class 1 state */
    Ace_ChangeState(pAce, ACE_FRM);
    pAce->isToAvtEcho = FALSE;
    pAce->currentT30DceCmd = T30_DCE_FRM;
    pAce->pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
    pAce->StartTime = *pClk;

    return SUCCESS;
}

void Ace_FsmFRM(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    if (pAce->LineState == ONLINE_FAX_DATA)
    {
        CircBuffer *pAscCBWr = (CircBuffer *)pTable[ASCCBWRDATA_IDX];
        UWORD uLen = pAscCBWr->CurrLen;

        if (pT30ToDce->flowCntlCmd == DC3_XOFF)
        {
            TRACE0("ACE: WARNING. T30 indicates <DC3>");
        }
        else if (uLen > 0)
        {
            if (uLen > sizeof(pDceToT30->tempData))
            {
                uLen = sizeof(pDceToT30->tempData);
            }

            pDceToT30->tempDataLen = (UBYTE)(GetStringFromCB(pAscCBWr, pDceToT30->tempData, uLen));
            pDceToT30->tempDataType = DCE_T30_HS_DATA;
        }
    }
    else if (RdReg(pDpcs->MFCF0, CARRIER_GOT))
    {
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_CONNECT);
    }

    if (pT30ToDce->flowCntlCmd == CAN_STOP) // fixing issue 178
    {
        pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK); // fixing bug 24
        Ace_ChangeState(pAce, ACE_NOF);
        PutReg(pDpcs->MCF, DISCONNECT);
    }
    else if (RdReg(pDpcs->MSR0, DISC_DET))
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
            if (pAce->LineState == ONLINE_FAX_DATA) // fixing bug 28, cannot set type when it's not CONNECTed
            {
                pDceToT30->tempDataType = DCE_T30_HS_END;
            }
            else
            {
                pDceToT30->tempDataType = DCE_T30_DATA_NULL;
            }

            Ace_SetClass1ResultCode(pAce, RESULT_CODE_NOCARRIER);
        }

        Ace_ChangeState(pAce, ACE_NOF);
    }
}

#if SUPPORT_T31_PARSER
UBYTE CLASS1_FRM(UBYTE **pTable)
{
    return Ace_ParseMod(pTable, T30_DCE_FRM);
}
#endif
#endif

