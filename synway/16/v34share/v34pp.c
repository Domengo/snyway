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

/*-----------------------------------------------------------------------*/
/* Generation of the PP sequence in PHASE 3.                             */
/*                                                                       */
/* INPUT: UBYTE k, I - k and I are indices to create the PP sequence     */
/*           as described in ITU V.34 p.33                               */
/*                                                                       */
/* OUTPUT: COMPLEX p - the complex symbol point in the PP sequence       */
/*                                                                       */
/* Note: It is neccessary to call S_init at the beginning of the S       */
/*   sequence.                                                           */
/*                                                                       */
/* Date: Feb 2, 1996.                                                    */
/*   Apr 30, 1996. Jack Liu Porting to VXD lib                           */
/* Author: Bejamin Chan, GAO Research & Consulting Ltd.                  */
/*-----------------------------------------------------------------------*/

#include "shareext.h"

/* k=0--71; I=0--3 */
CQWORD V34_Create_PP(UBYTE k, UBYTE I)
{
    CQWORD cqC;
    UWORD  qIdx;
    UWORD  qTemp;

    qIdx = UBUBMULU(k, I);

    if ((k % 3) == 1)
    {
        qIdx += 4;
    }

    qTemp = (UWORD)(UUMULUD(qIdx, q1_OVER_6) & 0xFFFF); /* 2/12 */

    SinCos_Lookup_Fine(qTemp, &cqC.i, &cqC.r);

    cqC.r = QQMULQR15(cqC.r, V34_PP_STARTUP);
    cqC.i = QQMULQR15(cqC.i, V34_PP_STARTUP);

    return(cqC);
}
