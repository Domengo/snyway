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

#if SUPPORT_PARSER && SUPPORT_FAX

UBYTE AT_plusI(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    /* Look for ID command after +C */
    if ((pAce->CurrCommand[pAce->ATidx  ] != 'F')
        || (pAce->CurrCommand[pAce->ATidx+1] != 'C'))
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    /* Look for "=" or "?" */
    pAce->ATidx += 2;

    if (pAce->CurrCommand[pAce->ATidx] == '?')
    {
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }
    else if (pAce->CurrCommand[pAce->ATidx] != '=')
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    pAce->ATidx += 4;

    /* set result code: "OK") */
    AceSetResponse(pTable, ACE_OK);
    return SUCCESS;
}

#endif
