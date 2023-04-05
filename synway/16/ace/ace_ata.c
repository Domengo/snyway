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
#include "hwapi.h"

UBYTE Ace_ATA(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];
    UWORD *pClk = (UWORD *)pTable[CLOCKDATA_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
#if WITH_DC
    DCStruct *pDC = (DCStruct *)pTable[DC_STRUC_IDX];
#endif
#if SUPPORT_FAX
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
#endif

    /* error command in on line state */
    if (pAce->LineState != OFF_LINE
#if SUPPORT_V54
        || pAce->ubSelfTest != SELFTEST_OFF
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

    MhostConnect(pTable);

    SetReg(pDpcs->MCR0, ANSWER);   /* ANS mode */

    /* go off-hook */
    pHwApi[0] = HWAPI_OFFHOOK;

#if WITH_DC
    Init_DC(pDC);
#endif

    /* set state machine */
    Ace_ChangeLineState(pAce, CALL_PROGRESS);
    pAce->CPState = CP_ANSWER;
    pAce->StartTime = *pClk;
    pAce->RingCount = 0; // fixing bug 4

#if SUPPORT_PARSER && SUPPORT_MODEM
    pAce->CountPlus = 0;
    pAce->GuardTime = 0;
#endif

#if SUPPORT_FAX

    if (pAce->FaxClassType != FCLASS0)
    {
        pAce->currentT30DceCmd = T30_DCE_ATA;
#if SUPPORT_V34FAX

        if (*pT30ToDce->pIsV34Enabled)
        {
            WrReg(pDpcs->MBC0, MIN_RATE, (pT30ToDce->initV34PrimRateMin + 3));
            WrReg(pDpcs->MBC1, MAX_RATE, (pT30ToDce->initV34PrimRateMax + 3));
        }

#endif
    }

#endif

    return SUCCESS;
}

#if SUPPORT_PARSER
UBYTE AT_A(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if ((pAce->CurrCommand[pAce->ATidx + 1] != 0) && (pAce->CurrCommand[pAce->ATidx + 1] != ';'))
    {
        TRACE0("ATA ERROR");
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

#if SUPPORT_T32_PARSER

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        // T.30 will pass this command down
        Ace_SetT32Command(pAce, DTE_T30_ATA);
    }
    else
#endif
#if SUPPORT_T31_PARSER
        if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
        {
            // This command will come down from T.31 interface
            Ace_SetT31Command(pAce, T30_DCE_ATA);
        }
        else
#endif
        {
            // No interface
            pAce->Mandate = TRUE;
            Ace_ATA(pTable);
        }

    return SUCCESS;
}
#endif
