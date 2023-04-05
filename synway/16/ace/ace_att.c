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

#if SUPPORT_PARSER
UBYTE AT_T(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    ResetReg(pDpcs->MCR1, PULSE_MODE);

    /* set result code: "OK") */
    AceSetResponse(pTable, ACE_OK);
    return SUCCESS;
}
#endif
