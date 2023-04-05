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

#if SUPPORT_T31_PARSER
UBYTE CLASS1_FDD(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
    {
        if (pAce->CurrCommand[pAce->ATidx++] == '=')
        {
            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case '0': pAce->ClassFDD = FALSE; TRACE0("ACE: +FDD off"); return SUCCESS;
                case '1': pAce->ClassFDD = TRUE; TRACE0("ACE: +FDD on"); return SUCCESS;
            }
        }
    }

    return FAIL;
}
#endif
