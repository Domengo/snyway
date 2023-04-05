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

/*****************************************************
DESCRIPTION: Designed mainly for DTMF control.

  COMMAND SUPPORTED:
  AT+DTMF=0 - Disable/Stop DTMF detection.
  AT+DTMF=1 - Enable/Start DTMF detection.
******************************************************/

#include "aceext.h"

#if (SUPPORT_DTMF + SUPPORT_V44)
UBYTE AT_plusD(UBYTE **pTable)
{
    UBYTE DTMFType;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    /* Look for ID command after +C */
#if SUPPORT_V44

    if (pAce->CurrCommand[pAce->ATidx] == 'S')
    {
        pAce->ATidx++;
        return(AT_plusDS(pTable));
    }

#endif

#if SUPPORT_DTMF

    if ((pAce->CurrCommand[pAce->ATidx  ] != 'T')
        || (pAce->CurrCommand[pAce->ATidx+1] != 'M')
        || (pAce->CurrCommand[pAce->ATidx+2] != 'F'))
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    /* Look for "=" or "?" */
    pAce->ATidx += 3;

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

    /* Read value */
    pAce->ATidx++;

    if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx]))
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    DTMFType = AceReadValue_2Digit(pAce);
    pAce->ATidx--;

    if ((DTMFType == 0xFF) || (DTMFType > 1))
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    switch (DTMFType)
    {
        case 0:
            ResetReg(pDpcs->DTMFR, DTMF_ENABLE);     /* disable */
            PutReg(pDpcs->MCF,   DISCONNECT);  /* If running, stop it */
            break;

        case 1:
            SetReg(pDpcs->DTMFR, DTMF_ENABLE);  /* Enable */
            PutReg(pDpcs->MCF,   CONNECT);      /* Start DTMF detection */
            break;
    }

    /* set result code: "OK") */
    AceSetResponse(pTable, ACE_OK);
    return SUCCESS;
#endif
}
#endif
