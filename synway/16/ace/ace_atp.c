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
UBYTE AT_P(UBYTE **pTable)
{
    UBYTE PCommVal;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx+1]))
    {
        /* default is ATP0 */
        WrReg(pDpcs->TELCOCR1, PULSE_CONTROL, 0x03);
        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }
    else if ((pAce->CurrCommand[pAce->ATidx+1] >= '0') && (pAce->CurrCommand[pAce->ATidx+1] <= '3'))
    {
        PCommVal = (UBYTE)(pAce->CurrCommand[pAce->ATidx+1] - '0');
        pAce->ATidx++;

        switch (PCommVal)
        {
            case 0:    ResetReg(pDpcs->TELCOCR1, PULSE_CONTROL);
                break;
            case 1:    WrReg(pDpcs->TELCOCR1, PULSE_CONTROL, 0x10);
                break;
            case 2:    WrReg(pDpcs->TELCOCR1, PULSE_CONTROL, 0x20);
                break;
            case 3:    WrReg(pDpcs->TELCOCR1, PULSE_CONTROL, 0x30);
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
