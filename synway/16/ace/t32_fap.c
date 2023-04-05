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

#if SUPPORT_T32_PARSER
UBYTE CLASS2_FAP(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isEnd = FALSE;
    UBYTE sub, sep, pwd;

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case '=':

                if (pAce->CurrCommand[pAce->ATidx] == '?')
                {
                    pAce->ATidx++;
                    AceSetResponse(pTable, ACE_FAP_CAP);
                    return SUCCESS;
                }
                else if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &sub, &isEnd, FALSE, 0))
                {
                    if (sub <= 1 && !isEnd)
                    {
                        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &sep, &isEnd, FALSE, 0))
                        {
                            if (sep <= 1 && !isEnd)
                            {
                                if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &pwd, &isEnd, FALSE, 0))
                                {
                                    if (pwd <= 1)
                                    {
                                        if (sub) { SetReg(pAce->pDteToT30->addressPollingCap, FAP_SUB); }

                                        if (sep) { SetReg(pAce->pDteToT30->addressPollingCap, FAP_SEP); }

                                        if (pwd) { SetReg(pAce->pDteToT30->addressPollingCap, FAP_PWD); }

                                        return SUCCESS;
                                    }
                                }
                            }
                        }
                    }
                }

                break;
            case '?':
            {
                CircBuffer *DteRd = pAce->pCBOut;
                UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                PutByteToCB(DteRd, RdReg(pAce->pDteToT30->addressPollingCap, FAP_SUB) ? '1' : '0');
                PutByteToCB(DteRd, ',');
                PutByteToCB(DteRd, RdReg(pAce->pDteToT30->addressPollingCap, FAP_SEP) ? '1' : '0');
                PutByteToCB(DteRd, ',');
                PutByteToCB(DteRd, RdReg(pAce->pDteToT30->addressPollingCap, FAP_PWD) ? '1' : '0');

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                return SUCCESS;
            }
        }
    }

    return FAIL;
}
#endif
