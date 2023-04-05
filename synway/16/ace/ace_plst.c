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
#include "common.h"

#if SUPPORT_PARSER
UBYTE AT_plusT(UBYTE **pTable)
{
    UBYTE SerRate;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isEnd = FALSE, isErr = FALSE;

    if ((pAce->CurrCommand[pAce->ATidx] != '='))
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    pAce->ATidx++;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &SerRate, &isEnd, FALSE, 0))
    {
        if (!((SerRate >= 1) && (SerRate <= 13)))
        {
            isErr = TRUE;
        }
    }
    else
    {
        isErr = TRUE;
    }

    if (isErr)
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    pAce->ubSerPortRate = SerRate;
    AceSetResponse(pTable, ACE_OK);
    return SUCCESS;
}
#endif
