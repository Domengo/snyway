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
#include "gaoapi.h"
#include "common.h"

UBYTE Ace_ATO(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    pAce->currentT30DceCmd = T30_DCE_ATO;
    return SUCCESS;
}

#if SUPPORT_PARSER
UBYTE AT_O(UBYTE **pTable)
{
    UBYTE State;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

#if SUPPORT_V54

    if (pAce->ubSelfTest != SELFTEST_OFF)/*** Self Test ***/
    {
        AceSetResponse(pTable, ACE_ERROR);
        return SUCCESS;
    }

#endif

    if (pAce->CurrCommand[pAce->ATidx+1] == '0'
        || !COMM_IsNumber(pAce->CurrCommand[pAce->ATidx+1]))
    {
        /* check if online command mode */
        if (pAce->LineState == ONLINE_COMMAND)
        {
            /* go to online data mode */
            Ace_ChangeLineState(pAce, ONLINE_DATA);
            /* set result code: "Connect") */
            AceSetResponse(pTable, ACE_CONNECT_MODEM);
        }

#if SUPPORT_T31_PARSER
        else if (pAce->FaxClassType != FCLASS0)
        {
        }

#endif
        /* set result code: "NO Carrier"); */
        else
        {
            AceSetResponse(pTable, ACE_NO_CARRIER);
        }

        State = SUCCESS;
    }
    else if (pAce->CurrCommand[pAce->ATidx+1] == '1')
    {
        /* check if online command mode */
        if (pAce->LineState == ONLINE_COMMAND)
        {
            /* go to online retrain mode */
            Ace_ChangeLineState(pAce, ONLINE_RETRAIN);

            PutReg(pDpcs->MCF, RTRN);

            /* set control flag for result code after retrain finished) */
            pAce->ubControlFlag = CONTROL_FLAG_ATO1;
        }

        State = SUCCESS;
    }
    else if (pAce->CurrCommand[pAce->ATidx+1] == '2')
    {
        /* check if online command mode */
        if (pAce->LineState == ONLINE_COMMAND)
        {
            /* go to online retrain mode */
            Ace_ChangeLineState(pAce, ONLINE_RETRAIN);

            PutReg(pDpcs->MCF, RRATE);

            /* set control flag for result code after rerate finished) */
            pAce->ubControlFlag = CONTROL_FLAG_ATO1;
        }

        State = SUCCESS;
    }
    /* set result code: "ERROR"); */
    else
    {
        AceSetResponse(pTable, ACE_ERROR);
        State = FAIL;
    }

    if ((pAce->CurrCommand[pAce->ATidx+1] >= '0') && (pAce->CurrCommand[pAce->ATidx+1] <= '2'))
    {
        pAce->ATidx++;
    }

#if SUPPORT_T31_PARSER

    if (State == SUCCESS)
    {
        Ace_SetT31Command(pAce, T30_DCE_ATO);
    }

#endif

    return State;
}
#endif
