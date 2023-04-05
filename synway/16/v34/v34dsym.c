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

/* -------------------------------------------------------------------- */
/*  The Detect_sequence routine is to receive symbol and generate bit   */
/*  sequence for sequence J, TRN, MP, MP', and E.                       */
/*                                                                      */
/*  Input: COMPLEX C - the symbol from the Equalizer                    */
/*         COMPLEX *pcOutsym - the output decision symbol               */
/*         constel_size - has the value 4 or 16, which represent either */
/*                        4 or 16 point constellation.                  */
/*                                                                      */
/*  Output: 2 bits or 4 bits, depending on the constellation size       */
/*                                                                      */
/*           I2n,I1n (I1n is in the least significant bit)              */
/*           4 bits if the constellation size is 16;                    */
/*      Q4n,Q3n,Q2n,Q1n (I1n is the least significant bit)              */
/*                                                                      */
/*  Date: Feb 14, 1996.                                                 */
/*        Feb 21, 1996. Change table due to TRN bug, Benjamin           */
/*        Mar 20, 1996. Add STARTUP_LEVEL to symbol table and           */
/*                      SYMBOL_SCALE_DOWN                               */
/*                      change qMin_dist_sq and qDist_sq to float       */
/*        Apr 30, 1996. Jack Liu Porting to VXD lib.                    */
/*  Author: Dennis, GAO Research & Consulting Ltd.                      */
/* -------------------------------------------------------------------- */

#include "v34ext.h"

UBYTE  V34_Symbol_To_Bits(CQWORD cqC, CQWORD *pcOutsym, UWORD constel_size)
{
    QWORD qMin_dist_sq;
    QWORD qDist_sq;
    CQWORD cqDiff;
    CQWORD *pcP;
    UBYTE  current_i, i;

    if (constel_size == V34_4_POINT_TRN)
    {
        pcP = (CQWORD *)V34_tTRAIN_MAP_4;
    }
    else
    {
        pcP = (CQWORD *)V34_tTRAIN_MAP_16;
    }

    qMin_dist_sq = qONE;

    current_i = 0;

    for (i = 0; i < constel_size; i++)
    {
        CQSUB(cqC, pcP[i], cqDiff);

        qDist_sq = QQMULQR15(cqDiff.r, cqDiff.r) + QQMULQR15(cqDiff.i, cqDiff.i);

        if (qDist_sq < qMin_dist_sq)
        {
            qMin_dist_sq = qDist_sq;
            current_i = i;
        }
    }

    *pcOutsym = pcP[current_i];

    return(current_i);
}
