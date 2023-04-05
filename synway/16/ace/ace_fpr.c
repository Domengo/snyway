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

#if SUPPORT_PARSER
/* serial port rate is fixed @ 19,200 */
UBYTE AT_plusFPR(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    switch (pAce->CurrCommand[pAce->ATidx++])
    {
        case '=': return Ace_SetReportSerialPortRate(pTable, 2);
        case '?': Ace_ReportSerialPortRate(pTable, 2); return SUCCESS;
    }

    return FAIL;
}
#endif
