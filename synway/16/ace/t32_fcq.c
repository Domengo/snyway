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

#if SUPPORT_T32_PARSER
UBYTE CLASS2_FCQ(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case '=':

                switch (pAce->CurrCommand[pAce->ATidx++])
                {
                    case '?':
                        AceSetResponse(pTable, ACE_FCQ_CAP);
                        return SUCCESS;
                    case '0':

                        if (pAce->CurrCommand[pAce->ATidx++] == ',')
                        {
                            if (pAce->CurrCommand[pAce->ATidx++] == '0')
                            {
                                return SUCCESS;
                            }
                        }

                        break;
                }

                break;
            case '?':
            {
                CircBuffer *DteRd = pAce->pCBOut;
                UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                PutByteToCB(DteRd, ((pAce->pDteToT30->copyQtyChk >> 2) & 3) + '0');
                PutByteToCB(DteRd, ',');
                PutByteToCB(DteRd, ((pAce->pDteToT30->copyQtyChk >> 0) & 3) + '0');

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                return SUCCESS;
            }
        }
    }

    return FAIL;
}
#endif
