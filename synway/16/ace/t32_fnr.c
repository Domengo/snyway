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
UBYTE CLASS2_FNR(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isEnd = FALSE;
    UBYTE rpr, tpr, idr, nsr;

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case '=':

                if (pAce->CurrCommand[pAce->ATidx] == '?')
                {
                    pAce->ATidx++;
                    AceSetResponse(pTable, ACE_FNR_CAP);
                    return SUCCESS;
                }
                else if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &rpr, &isEnd, FALSE, 0))
                {
                    if (rpr <= 1 && !isEnd)
                    {
                        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &tpr, &isEnd, FALSE, 0))
                        {
                            if (tpr <= 1 && !isEnd)
                            {
                                if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &idr, &isEnd, FALSE, 0))
                                {
                                    if (idr <= 1 && !isEnd)
                                    {
                                        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &nsr, &isEnd, FALSE, 0))
                                        {
                                            if (nsr <= 1)
                                            {
                                                if (rpr) { SetReg(pAce->pDteToT30->negoReporting, FNR_RPR); }

                                                if (tpr) { SetReg(pAce->pDteToT30->negoReporting, FNR_TPR); }

                                                if (idr) { SetReg(pAce->pDteToT30->negoReporting, FNR_IDR); }

                                                if (nsr) { SetReg(pAce->pDteToT30->negoReporting, FNR_NSR); }

                                                return SUCCESS;
                                            }
                                        }
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

                PutByteToCB(DteRd, RdReg(pAce->pDteToT30->negoReporting, FNR_RPR) ? '1' : '0');
                PutByteToCB(DteRd, ',');
                PutByteToCB(DteRd, RdReg(pAce->pDteToT30->negoReporting, FNR_TPR) ? '1' : '0');
                PutByteToCB(DteRd, ',');
                PutByteToCB(DteRd, RdReg(pAce->pDteToT30->negoReporting, FNR_IDR) ? '1' : '0');
                PutByteToCB(DteRd, ',');
                PutByteToCB(DteRd, RdReg(pAce->pDteToT30->negoReporting, FNR_NSR) ? '1' : '0');

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                return SUCCESS;
            }
        }
    }

    return FAIL;
}
#endif
