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
UBYTE CLASS2_FMS(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case '=':

                if (pAce->CurrCommand[pAce->ATidx] == '?')
                {
                    pAce->ATidx++;
#if SUPPORT_V34FAX

                    if (pAce->FaxClassType == FCLASS21)
                    {
                        AceSetResponse(pTable, ACE_FMS_CAP_V34FAX);
                    }
                    else
#endif
                    {
                        AceSetResponse(pTable, ACE_FMS_CAP);
                    }

                    return SUCCESS;
                }
                else
                {
                    UBYTE fms;
                    UBYTE isEnd = FALSE;

                    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &fms, &isEnd, TRUE, BR_2400))
                    {
                        if (fms <= BR_14400
#if SUPPORT_V34FAX
                            || (pAce->FaxClassType == FCLASS21 && fms <= BR_33600)
#endif
                           )
                        {
                            pAce->ATidx++;
                            pAce->pDteToT30->minPhaseCSpd = fms;
#if SUPPORT_V34FAX
                            pAce->pT30ToDce->initV34PrimRateMin = fms + 1;
#endif
                            return SUCCESS;
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

                COMM_HexToAscii(DteRd, pAce->pDteToT30->minPhaseCSpd);

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                return SUCCESS;
            }
        }
    }

    return FAIL;
}
#endif
