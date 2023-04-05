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

/**********************************************************
DESCRIPTION: Designed mainly for AT+IFC.
**********************************************************/

#include "aceext.h"

#if SUPPORT_PARSER && SUPPORT_V251
UBYTE AT_plusA(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    /* Look for ID command after +A */
    if (pAce->CurrCommand[pAce->ATidx++] == '8')
    {
        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case 'E': return AT_A8E(pTable);
            case 'M': return AT_A8M(pTable);
        }
    }

    AceSetResponse(pTable, ACE_ERROR);
    return FAIL;
}

#endif
