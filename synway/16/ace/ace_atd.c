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

#include <string.h>
#include "aceext.h"
#include "gaoapi.h"
#include "hwapi.h"

UBYTE Ace_ATD(UBYTE **pTable)
{
#if WITH_DC
    DCStruct *pDC = (DCStruct *)pTable[DC_STRUC_IDX];
#endif
    UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UWORD *pClk = (UWORD *)pTable[CLOCKDATA_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
#if SUPPORT_FAX
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
#endif

    /* error command in on line state */
    if ((pAce->LineState != OFF_LINE)
#if SUPPORT_V54
        || (pAce->ubSelfTest != SELFTEST_OFF)
#endif
       )
    {
#if SUPPORT_PARSER
        AceSetResponse(pTable, ACE_ERROR);
#else
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
#endif
        return FAIL;
    }

    Ace_ChangeLineState(pAce, CALL_PROGRESS);

    MhostConnect(pTable);

    ResetReg(pDpcs->MCR0, ANSWER);    /* CALL mode */

    /* go off-hook */
    pHwApi[0] = HWAPI_OFFHOOK;

#if WITH_DC
    Init_DC(pDC);
#endif

#if SUPPORT_V54
    ClrReg(pDpcs->MTC);
    ResetReg(pDpcs->MCR0, TEST_MODE);
    /* PutReg(pDpcs->MCF, DISCONNECT); */
#endif

    /* set state machine */
    pAce->CPState = CP_START;
    pAce->StartTime = *pClk;

#if SUPPORT_PARSER && SUPPORT_MODEM
    pAce->CountPlus = 0;
    pAce->GuardTime = 0;
#endif

#if SUPPORT_FAX

    if (pAce->FaxClassType != FCLASS0)
    {
        UBYTE *pDialString = (UBYTE *)pTable[HOSTMSGDATA_IDX];

        pAce->currentT30DceCmd = T30_DCE_ATD;
        memcpy(pDialString, pT30ToDce->tempParm, pT30ToDce->tempParmLen);
#if SUPPORT_V34FAX

        if (*pT30ToDce->pIsV34Enabled)
        {
            WrReg(pDpcs->MBC0, MIN_RATE, (pT30ToDce->initV34PrimRateMin + 3));
            WrReg(pDpcs->MBC1, MAX_RATE, (pT30ToDce->initV34PrimRateMax + 3));
        }
    }

#endif
#endif

    return SUCCESS;
}

#if SUPPORT_PARSER
UBYTE AT_D(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE i, CurrCommandLen;
    UBYTE *pDialString;

    /* copy dial string */
    pAce->ATidx++;

    if (pAce->CurrCommand[pAce->ATidx] == 'P')
    {
        SetReg(pDpcs->MCR1, PULSE_MODE);
        pAce->ATidx++;
    }
    else
    {
        if (pAce->CurrCommand[pAce->ATidx] == 'T')
        {
            pAce->ATidx++;
        }

        ResetReg(pDpcs->MCR1, PULSE_MODE);
    }

#if SUPPORT_T32_PARSER

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        pDialString = &pAce->pDteToT30->tempData[1];
    }
    else
#endif
#if SUPPORT_T31_PARSER
        if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
        {
            pDialString = pAce->pT30ToDce->tempParm;
        }
        else
#endif
        {
            pDialString = (UBYTE *)pTable[HOSTMSGDATA_IDX];
        }

    CurrCommandLen = (UBYTE)strlen((CHAR *)pAce->CurrCommand);

    for (i = 0; pAce->ATidx < CurrCommandLen && (pAce->CurrCommand[pAce->ATidx]) != ';'; i++)
    {
        pDialString[i] = pAce->CurrCommand[pAce->ATidx++];
    }

    pDialString[i++] = (UBYTE)0xFF;

    if ((pAce->CurrCommand[pAce->ATidx]) == ';')
    {
        if (pAce->Semicolon == SEMICOLON_NONE)
        {
            pAce->Semicolon = SEMICOLON_ON_FIRST;    /* first semicolon present */
        }
        else
        {
            pAce->Semicolon = SEMICOLON_ON_AGAIN;    /* another semicolon present */
        }

        pAce->ATidx++;
    }

#if SUPPORT_T32_PARSER

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        Ace_SetT32Command(pAce, DTE_T30_ATD);
        pAce->pDteToT30->tempDataLen = i;
    }
    else
#endif
#if SUPPORT_T31_PARSER
        if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
        {
            Ace_SetT31Command(pAce, T30_DCE_ATD);
            pAce->pT30ToDce->tempParmLen = i;
        }
        else
#endif
        {
            pAce->Mandate = TRUE;
            Ace_ATD(pTable);
        }

    return SUCCESS;
}
#endif
