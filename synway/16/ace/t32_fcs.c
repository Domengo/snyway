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
UBYTE CLASS2_FCS(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30SessionSubParm *pSta = &pAce->pT30ToDte->faxParmStatus;

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case '?':
            {
                CircBuffer *DteRd = pAce->pCBOut;
                UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                Ace_ParseSubParmToClass2Dte(pTable, pSta);

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                return SUCCESS;
            }
            break;
        }
    }

    return FAIL;
}
#endif
