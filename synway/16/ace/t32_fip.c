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

#if SUPPORT_T32_PARSER
UBYTE CLASS2_FIP(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isErr = FALSE;

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        if (pAce->CurrCommand[pAce->ATidx] == '=')
        {
            UBYTE isEnd = FALSE;
            UBYTE dummy;

            pAce->ATidx++;

            if (!COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &dummy, &isEnd, FALSE, 0))
            {
                isErr = TRUE;
            }
        }
    }
    else
    {
        isErr = TRUE;
    }

    if (isErr)
    {
        return FAIL;
    }
    else
    {
        Ace_SetT32Command(pAce, DTE_T30_FIP);
        return SUCCESS;
    }
}
#endif
