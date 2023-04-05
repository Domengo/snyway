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

/************************************************************************/
/*  MODULE:         SWORD label(CQWORD a)                               */
/*  FILE NAME:                                                          */
/*          label.c                                                     */
/*  INPUT:                                                              */
/*          CQWORD a;     -Input a.r and a.i of a 2D point              */
/*  OUTPUT:                                                             */
/*          result;        -Output the label that comprises y4y3y2y1    */
/*  NOTES:                                                              */
/*          This program will be invoked by the module D4min_find() in  */
/*          file: viterbi.c; it can also be used in the convolutional   */
/*              encoder                                                 */
/************************************************************************/

#include "shareext.h"

#if !USE_ASM
SBYTE V34_Label(CQWORD cIn)
{
    SBYTE result;
    UBYTE Tempr, Tempi, Bit;

    Tempr = (UBYTE)((cIn.r - 1) >> 1);
    Tempi = (UBYTE)((cIn.i - 1) >> 1);

    Tempr &= 0x3;
    Tempi &= 0x3;

    Bit = (Tempr << 2) + Tempi;

    result = V34_Inverse_Map_Label[Bit];

    return(result);
}
#endif
