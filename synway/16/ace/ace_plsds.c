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

/* This file contains the function that processes the AT command: AT+DS44 */

#include <string.h>
#include "aceext.h"

#if SUPPORT_V44
#include "v44ext.h"

UBYTE AT_plusDS(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    SWORD result;
    UBYTE stringBuf[V44_PARAM_STRING_LEN];

    /* Look for S44 command after +D */
    if (pAce->CurrCommand[pAce->ATidx] != '4' || pAce->CurrCommand[pAce->ATidx + 1] != '4')
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    pAce->ATidx += 2;

    if (pAce->CurrCommand[pAce->ATidx] == '=')
    {
        pAce->ATidx++;

        if (pAce->CurrCommand[pAce->ATidx] == '?')
        {
            stringBuf[0] = pSreg[CR_CHARACTER];
            stringBuf[1] = pSreg[LF_CHARACTER];
            stringBuf[2] = 0;
            PutStringToCB(DteRd, stringBuf, (UWORD)strlen(stringBuf));

            V44ParamGetRange(&(pAce->m_V44Parameter), stringBuf);
            PutStringToCB(DteRd, stringBuf, (UWORD)strlen(stringBuf));

            AceSetResponse(pTable, ACE_OK);
            return SUCCESS;
        }
        else
        {
            result = V44ParamSet(&(pAce->m_V44Parameter), pAce->CurrCommand, &(pAce->ATidx));

            if (V44_PARAM_OK != result)
            {
                AceSetResponse(pTable, ACE_ERROR);
                return FAIL;
            }

            pAce->ATidx--;
        }
    }
    else if (pAce->CurrCommand[pAce->ATidx] == '?')
    {
        stringBuf[0] = pSreg[CR_CHARACTER];
        stringBuf[1] = pSreg[LF_CHARACTER];
        stringBuf[2] = 0;
        PutStringToCB(DteRd, stringBuf, (UWORD)strlen(stringBuf));

        V44ParamGetValue(&(pAce->m_V44Parameter), stringBuf);
        PutStringToCB(DteRd, stringBuf, (UWORD)strlen(stringBuf));

        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }
    else
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    /* set result code: "OK") */
    AceSetResponse(pTable, ACE_OK);
    return SUCCESS;
}
#endif
