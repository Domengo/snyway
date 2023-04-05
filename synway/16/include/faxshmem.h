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

/******************************************************************************
        Description :
                Fax(V17, V29, V27ter) share memory structure with V21
 *****************************************************************************/

#ifndef _FAXSHAREMEM_H
#define _FAXSHAREMEM_H

#include "commdef.h"
#include "ptable.h"

typedef struct _FaxShareMemStruc
{
    QWORD  qV21Agc;
    QWORD  qFaxAgc;
    CQWORD cEqCoef_ago[48];
    QWORD  qTimingErr1;
    QWORD  qCarErrPhase;
    SWORD  nDemodDeltaOffset;
} FaxShareMemStruct;

#endif
