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

#include <stdio.h>
#include <string.h>
#include "aceext.h"
#include "gaoapi.h"
#include "common.h"

#if (SUPPORT_V44 || SUPPORT_V42INFO)
#include "v42ext.h"
#endif

#if SUPPORT_PARSER
UBYTE AT_I(UBYTE **pTable)
{
#if SUPPORT_V44 || SUPPORT_ENERGY_REPORT
    DpcsStruct  *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
#endif
#if SUPPORT_V44
    UBYTE Code;
    V42Struct    *pV42 = (V42Struct *)pTable[V42_STRUC_IDX];
    V42ECFStruct *pECF = &(pV42->ECF);
#endif
#if SUPPORT_V42INFO
    V42Info *pV42Info = (V42Info *)pTable[V42INFO_STRUC_IDX];
    UWORD count;
#endif
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    UBYTE stringBuf[ACE_RESPONSE_LEN];
#if SUPPORT_ENERGY_REPORT
    QDWORD qdSignal_Level;
    UBYTE  TempStr[6], s_UnitsPlace[3];
#endif

    /* Modified for EPR327 */
    if ((pAce->CurrCommand[pAce->ATidx+1] == '0')
        || !COMM_IsNumber(pAce->CurrCommand[pAce->ATidx+1]))
    {
        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

        if (pAce->CurrCommand[pAce->ATidx+1] == '0')
        {
            pAce->ATidx++;
        }

        sprintf((CHAR *)stringBuf, "Version: %s (%s - %s)", VERSION_LABEL, ResponseCode[ACE_MM], ResponseCode[ACE_MR]); // fixing issue 20, unify the version information
        PutStringToCB(DteRd, (UBYTE *)(stringBuf), (UWORD)(strlen((CHAR *)stringBuf)));

        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }

#if SUPPORT_V44
    else if (pAce->CurrCommand[pAce->ATidx + 1] == '1')
    {
        Code = RdReg(pDpcs->MSR1, V44_DATA);

        if (Code)
        {
            PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
            PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
            pAce->ATidx++;

            V44ParamGetValue(&(pECF->m_V44Param), stringBuf);/* V42 Get V44 parameter */

            PutStringToCB(DteRd, stringBuf, (UWORD)strlen((CHAR *)stringBuf));

            AceSetResponse(pTable, ACE_OK);
            return SUCCESS;
        }
        else
        {
            AceSetResponse(pTable, ACE_ERROR);
            return FAIL;
        }
    }

#endif
#if SUPPORT_V42INFO
    else if (pAce->CurrCommand[pAce->ATidx + 1] == '7')
    {
        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
        pAce->ATidx++;

        while ((count = V42InfoOutput(pV42Info, stringBuf, ACE_RESPONSE_LEN)) > 0)
        {
            PutStringToCB(DteRd, stringBuf, count);
            PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
            PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
        }

        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }

#endif
#if SUPPORT_ENERGY_REPORT/* ATI6 report V22 carrier level in dB */
    else if ((pAce->CurrCommand[pAce->ATidx+1] == '6') && (pAce->LineState == ONLINE_COMMAND))
    {
        pAce->ATidx++;

        qdSignal_Level = DSPD_Log10(pDpcs->ENERGY);
        qdSignal_Level = 23 - QDQMULQDR(qdSignal_Level, 10);

        if (qdSignal_Level < 0) /**/
        {
            qdSignal_Level = 0;
        }

        Number100_10_1((UWORD)qdSignal_Level, s_UnitsPlace);

        TempStr[0] = pSreg[CR_CHARACTER];
        TempStr[1] = pSreg[LF_CHARACTER];

        if (qdSignal_Level != 0)
        {
            TempStr[2] = '-';
        }
        else
        {
            TempStr[2] = ' ';
        }

        TempStr[3] = (UBYTE)(s_UnitsPlace[1] + 0x30);
        TempStr[4] = (UBYTE)(s_UnitsPlace[2] + 0x30);

        PutStringToCB(DteRd, TempStr, 5);

        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }

#endif
    else
    {
        /* set result code: "ERROR" */
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
}
#endif
