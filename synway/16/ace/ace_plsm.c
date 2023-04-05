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

#if SUPPORT_PARSER && SUPPORT_MODEM/* The switch is only for compiling, cannot delete!!! */

UBYTE AT_plusM(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE ModType;
    UBYTE isEnd = FALSE, isErr = FALSE;

    if ((pAce->CurrCommand[pAce->ATidx] != '='))
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    pAce->ATidx++;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &ModType, &isEnd, FALSE, 0))
    {
        if (ModType > BELL212A)
        {
            isErr = TRUE;
        }
        else
        {
#if !SUPPORT_V21M

            if (ModType == MODEM_V21)
            {
                isErr = TRUE;
            }

#endif

#if !SUPPORT_V23

            if (ModType == MODEM_V23)
            {
                isErr = TRUE;
            }

#endif

#if !SUPPORT_V22

            if (ModType == MODEM_V22)
            {
                isErr = TRUE;
            }

#endif

#if !SUPPORT_V22BIS

            if (ModType == MODEM_V22bis)
            {
                isErr = TRUE;
            }

#endif

#if !SUPPORT_V32

            if (ModType == MODEM_V32)
            {
                isErr = TRUE;
            }

#endif

#if !SUPPORT_V32BIS

            if (ModType == MODEM_V32bis)
            {
                isErr = TRUE;
            }

#endif

#if !SUPPORT_V34

            if (ModType == MODEM_V34)
            {
                isErr = TRUE;
            }

#endif

#if !(SUPPORT_V90A + SUPPORT_V90D)

            if (ModType == MODEM_V90)
            {
                isErr = TRUE;
            }

#endif

#if !(SUPPORT_V92A + SUPPORT_V92D)

            if (ModType == MODEM_V92)
            {
                isErr = TRUE;
            }

#endif

#if !SUPPORT_B103

            if (ModType == BELL103)
            {
                isErr = TRUE;
            }

#endif

#if !SUPPORT_B212A

            if (ModType == BELL212A)
            {
                isErr = TRUE;
            }

#endif
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


#if SUPPORT_MODEM
    pAce->ubModemType = ModType;
#endif

#if SUPPORT_AUTO

    /* set Auto mode flag */
    if (ModType == AUTOMODE)
    {
        pAce->AutoMode = 1;
    }
    else
    {
        pAce->AutoMode = 0;
    }

#endif

    /* set result code: "OK") */
    AceSetResponse(pTable, ACE_OK);
    return SUCCESS;
}

#endif
