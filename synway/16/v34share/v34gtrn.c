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

/* ------------------------------------------------------------------------ */
/* Generation of the TRN sequence in PHASE 3.                               */
/*                                                                          */
/* INPUT: INFO *V34I - the V.34 info structure                              */
/*                                                                          */
/* OUTPUT: COMPLEX p - the complex symbol point in the TRN sequence         */
/*                                                                          */
/* Date: Feb 2, 1996.                                                       */
/*       Apr 30, 1996. Jack Liu Porting to VXD lib                          */
/* Author: Bejamin Chan, GAO Research & Consulting Ltd.                     */
/* ------------------------------------------------------------------------ */

#include "shareext.h"

CQWORD V34_Create_TRN(UBYTE *scram_buf, UBYTE *scram_idx, SWORD num_TRN)
{
    UBYTE i, q;

    i = scram_buf[*scram_idx] + (scram_buf[(*scram_idx) + 1] << 1);

    *scram_idx += 2;

    if (num_TRN == V34_4_POINT_TRN)
    {
        return(V34_tTRAIN_MAP_4[i]);
    }
    else     /* 16-points TRN signal */
    {
        q = scram_buf[*scram_idx] + (scram_buf[(*scram_idx) + 1] << 1);

        *scram_idx += 2;

        return(V34_tTRAIN_MAP_16[(q << 2) + i]);
    }
}
