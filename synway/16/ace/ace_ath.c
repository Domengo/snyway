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
#include "mhsp.h"
#include "common.h"
#include "hwapi.h"

UBYTE Ace_ATH(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE *pParms;

#if SUPPORT_FAX

    if (pAce->FaxClassType != FCLASS0)
    {
        pParms = pAce->pT30ToDce->tempParm;
        pAce->currentT30DceCmd = T30_DCE_ATH; // fixing bug 2, moved into this FCLASS check
    }
    else
#endif
    {
        pParms = (UBYTE *)pTable[HOSTMSGDATA_IDX];
    }

    TRACE1("ACE: Command ATH received on state %s", Class_State[pAce->classState]);

#if SUPPORT_V54
    pAce->ubSelfTest = SELFTEST_OFF;
#endif

    pMhsp->Total_TimeOut_Count = 0;

    if (pParms[0] == 0)
    {
        DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

        if (pAce->LineState == ONLINE_DATA || pAce->LineState == ONLINE_COMMAND)/* go to command state */
        {
            if ((pDpcs->MSR1 & V42_DATA) || (pDpcs->MSR1 & (MNP_DATA | MNP_DATA1)))
            {
                PutReg(pDpcs->MCF, EC_DISC);
            }
            else
            {
                PutReg(pDpcs->MCF, DISCONNECT);
            }

            PutReg(pDpcs->DEBUGSR, (0x01));
        }
        else
        {
            PutReg(pDpcs->MCF, DISCONNECT);
        }

        Ace_ChangeState(pAce, ACE_ATH);

        return SUCCESS;
    }
    else
    {
        UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];
#if WITH_DC
        DCStruct *pDC = (DCStruct *)pTable[DC_STRUC_IDX];
#endif

        /* go off hook */
        pHwApi[0] = HWAPI_OFFHOOK;

#if WITH_DC
        Init_DC(pDC);
#endif

        /* set result code: "OK") */
        Ace_SetResultCode(pTable, ACE_OK);

        return SUCCESS;
    }
}

#if SUPPORT_PARSER
UBYTE AT_H(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE *pParms;

#if SUPPORT_FAX

    if (pAce->FaxClassType != FCLASS0)
    {
        pParms = pAce->pT30ToDce->tempParm;
    }
    else
#endif
    {
        pParms = (UBYTE *)pTable[HOSTMSGDATA_IDX];
    }

    if (pAce->CurrCommand[pAce->ATidx + 1] == '0')
    {
        pAce->ATidx++;
        pParms[0] = 0;
    }
    else if (pAce->CurrCommand[pAce->ATidx + 1] == '1')
    {
        pAce->ATidx++;
        pParms[0] = 1;
    }
    else if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx + 1]))
    {
        pParms[0] = 0;
    }
    else
    {
        /* set result code: "ERROR") */
        AceSetResponse(pTable, ACE_ERROR);
        TRACE2("ACE: ERROR. <%s>: IDX <%d>", pAce->CurrCommand, pAce->ATidx);

        return FAIL;
    }

#if SUPPORT_FAX
#if SUPPORT_T32_PARSER

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        pAce->pDteToT30->isDiscCmd = TRUE;
    }
    else
#endif
#if SUPPORT_T31_PARSER
        if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
        {
            Ace_SetT31Command(pAce, T30_DCE_ATH);
        }
        else
#endif
#endif
        {
            pAce->Mandate = TRUE;
            Ace_ATH(pTable);
        }

    return SUCCESS;
}
#endif
