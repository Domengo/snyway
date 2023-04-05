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
UBYTE AT_X(UBYTE **pTable)
{
    UBYTE XCommVal;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx+1]))
    {
        /* disable dial tone and busy detection */
        pAce->DialToneDet = 0;
        pAce->BusyDet = 0;
        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }
    else if ((pAce->CurrCommand[pAce->ATidx+1] >= '0') && (pAce->CurrCommand[pAce->ATidx+1] <= '4'))
    {
        XCommVal = (UBYTE)(pAce->CurrCommand[pAce->ATidx+1] - '0');
        pAce->ATidx++;

        switch (XCommVal)
        {
            case 0:
            case 1:
                pAce->DialToneDet = 0;
                pAce->BusyDet     = 0;
                break;
            case 2:
                pAce->DialToneDet = 1;
                pAce->BusyDet     = 0;
                break;
            case 3:
                pAce->DialToneDet = 0;
                pAce->BusyDet     = 1;
                break;
            case 4:
                pAce->DialToneDet = 1;
                pAce->BusyDet     = 1;
                break;
        }

        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }
    else
    {
        /* set result code: "ERROR") */
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
}
#endif
