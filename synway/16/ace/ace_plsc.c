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
DESCRIPTION: Designed mainly for CID control.

  COMMAND SUPPORTED:
  AT+CID=0 - Disable caller ID
  AT+CID=1 - Enable caller ID, with Detail output format
  AT+CID=2 - Enable caller ID, unformatted output
**********************************************************/

#include "aceext.h"

#if (SUPPORT_CID_DET + SUPPORT_CID_GEN)

UBYTE AT_plusC(UBYTE **pTable)
{
    UBYTE CIDType;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    /* Look for ID command after +C */
    if ((pAce->CurrCommand[pAce->ATidx  ] != 'I')
        || (pAce->CurrCommand[pAce->ATidx+1] != 'D'))
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

    /* Read value */
    pAce->ATidx++;

    if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx]))
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    CIDType = AceReadValue_2Digit(pAce);
    pAce->ATidx--;

    if (CIDType == 0)
    {
        ResetReg(pDpcs->TELCOCR1, CID_CONTROL);             /* disable CID detection */
    }

#if SUPPORT_CID_DET
    else if (CIDType == 1)
    {
        WrReg(pDpcs->TELCOCR1, CID_CONTROL, BIT6);          /* CID detection Type 1 */
    }
    else if (CIDType == 2)
    {
        WrReg(pDpcs->TELCOCR1, CID_CONTROL, BIT7);          /* CID detection Type 2 */
    }
    else if (CIDType == 3)
    {
        WrReg(pDpcs->TELCOCR1, CID_CONTROL, (BIT6 | BIT7)); /* CID detection Type 3 */
    }

#endif
#if SUPPORT_CID_GEN
    else if (CIDType == 4)
    {
        WrReg(pDpcs->TELCOCR1, CIDGEN_CONTROL, BIT0);       /* CID generation Type 1 */
    }
    else if (CIDType == 5)
    {
        WrReg(pDpcs->TELCOCR1, CIDGEN_CONTROL, BIT1);       /* CID generation Type 2 */
    }

#if 0
    else if (CIDType == 6)
    {
        WrReg(pDpcs->TELCOCR1, CIDGEN_CONTROL, (BIT0 | BIT1)); /* CID generation Type 3 */
    }

#endif
#endif
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
