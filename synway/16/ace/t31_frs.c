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
UBYTE Ace_FRS(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (pAce->classState != ACE_NOF
#if SUPPORT_V34FAX
        || pAce->pDceToT30->isV34Selected
#endif
       )
    {
        TRACE0("ACE: ERROR. +FRS");
    }

    pAce->classWaitTime = pAce->pT30ToDce->tempParm[0];
    Ace_ChangeState(pAce, ACE_FRS);
    pAce->currentT30DceCmd = T30_DCE_FRS;

    return SUCCESS;
}

void Ace_FsmFRS(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    // fixing issue 195
    if (pAce->timerSilence > pAce->classWaitTime ||
        pAce->pT30ToDce->flowCntlCmd == CAN_STOP)
    {
        pAce->timerSilence = 0;
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
        Ace_ChangeState(pAce, ACE_NOF);
    }
}

#if SUPPORT_T31_PARSER
UBYTE CLASS1_FRS(UBYTE **pTable)
{
    return Ace_ParseSilence(pTable, T30_DCE_FRS);
}
#endif
#endif