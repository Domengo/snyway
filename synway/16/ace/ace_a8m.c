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
#include "v8ext.h"
#include "common.h"

#if SUPPORT_V251 && SUPPORT_V8
UBYTE Ace_A8M(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    V8Struct *pV8   = (V8Struct *)(pTable[V8_STRUC_IDX]);
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
    UWORD *pClk = (UWORD *)pTable[CLOCKDATA_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE *pMenuSrc = pT30ToDce->tempParm;
    UBYTE *pMenuDst;
    int i, j;

    pV8->isToSendMenu = TRUE;

    if (RdReg(pDpcs->MCR0, ANSWER))
    {
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
        pMenuDst = &pV8->JM_Octets[20];
    }
    else
    {
        pMenuDst = &pV8->CM_Octets[20];
    }

    for (i = 0; i < pT30ToDce->tempParmLen; i ++)
    {
        for (j = 0; j < 8; j ++)
        {
            pMenuDst[j + 1] = (*pMenuSrc & (1 << j)) ? 1 : 0;
        }

        pMenuDst += 10;
        pMenuSrc ++;
    }

    pT30ToDce->tempParmLen = 0;
    /* set class 1 state */
    Ace_ChangeState(pAce, ACE_A8M);
    pAce->currentT30DceCmd = T30_DCE_A8M;
    pAce->StartTime = *pClk;
    return SUCCESS;
}

#if SUPPORT_PARSER
UBYTE AT_A8M(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (pAce->CurrCommand[pAce->ATidx++] == '=')
    {
#if SUPPORT_T31_PARSER
        T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;

        if (COMM_StringToOctets((UBYTE *)pAce->CurrCommand, &pAce->ATidx, pT30ToDce->tempParm))
        {
            Ace_SetT31Command(pAce, T30_DCE_A8M);
            return SUCCESS;
        }

#endif
    }

    AceSetResponse(pTable, ACE_ERROR);
    return FAIL;
}
#endif
#endif
