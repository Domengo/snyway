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

#if SUPPORT_T31
UBYTE Ace_FTM(UBYTE **pTable)
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
        TRACE0("ACE: ERROR. +FTM");
    }

    /* set modem connect flag */
    if (Ace_SetupFax(pTable) == FAIL)
    {
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
        return FAIL;
    }

    ResetReg(pDpcs->MCR0, RECEIVE_IMAGE);
    ResetReg(pDpcs->MFCF0, TX_RX);      /* Transmit */
    PutReg(pDpcs->MCF, CONNECT);
    pAce->StartTime = *pClk;

    /* set class 1 state */
    Ace_ChangeState(pAce, ACE_FTM);
    pAce->isToAvtEcho = FALSE;
    pAce->prevBufLen = 1;
    pAce->currentT30DceCmd = T30_DCE_FTM;
    pAce->pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
    Ace_SetClass1ResultCode(pAce, RESULT_CODE_CONNECT);

    return SUCCESS;
}

void Ace_FsmFTM(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];
    UWORD uLen = pT30ToDce->tempDataLen;

    if (uLen != 0)
    {
        if (!PutStringToCB(pAscCBRd, pT30ToDce->tempData, uLen))
        {
            pDceToT30->flowCntlCmd = DC3_XOFF;
        }
        else
        {
            /*
            int i;

            TRACE("%s", "ACE: Tx: ");
            for (i = 0; i < uLen; i++)
            {
                TRACE("%02x ", pT30ToDce->tempData[i]);
            }
            TRACE0("");
            */
            pDceToT30->flowCntlCmd = DC1_XON;
            pT30ToDce->tempDataLen = 0;
        }
    }
    else if (pT30ToDce->tempDataType == T30_DCE_HS_END) // cannot NULL the tempDataType until DISCONNECT
    {
        if (pAscCBRd->CurrLen == 0)
        {
            pDceToT30->flowCntlCmd = DC1_XON;
            pT30ToDce->tempDataType = T30_DCE_DATA_NULL;
            Ace_ChangeState(pAce, ACE_NOF);
            PutReg(pDpcs->MCF, DISCONNECT);/* T30 Stop Fax */
            Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
        }
        else
        {
            pDceToT30->flowCntlCmd = DC3_XOFF;
        }
    }
}

#if SUPPORT_T31_PARSER
UBYTE CLASS1_FTM(UBYTE **pTable)
{
    return Ace_ParseMod(pTable, T30_DCE_FTM);
}
#endif
#endif
