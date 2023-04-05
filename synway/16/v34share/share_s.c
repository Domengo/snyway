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

#include "shareext.h"

CQWORD V34_Create_S_or_SBar(UBYTE *S_point)
{
    CQWORD cqP;

    cqP = V34_tS_POINTS[*S_point];

    *S_point ^= 0x3;        /* Alternate Point 0 and Point 3 or Point 2 and Point 1 */

    return(cqP);
}
