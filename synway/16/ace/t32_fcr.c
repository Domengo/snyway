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
UBYTE CLASS2_FCR(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        if (pAce->CurrCommand[pAce->ATidx++] == '=')
        {
            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case '0': pAce->pDteToT30->isCapToRcv = FALSE; return SUCCESS;
                case '1': pAce->pDteToT30->isCapToRcv = TRUE; return SUCCESS;
            }
        }
    }

    return FAIL;
}
#endif
