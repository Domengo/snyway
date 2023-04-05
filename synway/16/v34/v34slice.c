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

#include "v34ext.h"

/*********************************************************************/
/* Slice the input symbol to the nearest ODD integer. All input and  */
/* output are in 9.7 format.                                         */
/*********************************************************************/
CQWORD V34_Slicer(CQWORD *pcS)
{
    CQWORD cSlice;

    cSlice.r = pcS->r & 0xFF80;
    cSlice.i = pcS->i & 0xFF80;

    /* Check if slice symbol is even, if it is even, round it to */
    /* the nearest odd number.                                   */
    if ((cSlice.r & 0x0080) == 0) /* Even number */
    {
        cSlice.r += 0x0080;
    }

    if ((cSlice.i & 0x0080) == 0) /* Even number */
    {
        cSlice.i += 0x0080;
    }

    return(cSlice);
}


/*********************************************************************/
/* Slice the input symbol to the nearest symbol in the 4-Points map. */
/* This is designed for used in data mode for detecting Rate         */
/* Renego pattern, which is the S pattern.                           */
/*********************************************************************/
void V34_4Point_Slicer(RenegoStruc *pRN, CQWORD *pcSym)
{
    UBYTE quadsym;

    /* Find quadrant of present symbol */
    /*    01 _|_ 00
          11  |  10      */
    quadsym = 0;

    if (pcSym->r < 0)
    {
        quadsym |= 0x01;
    }

    if (pcSym->i < 0)
    {
        quadsym |= 0x02;
    }

    /* compare with 2 previous symbol quadrants */
    if ((quadsym == pRN->quadsym2) && (quadsym != pRN->quadsym1))
    {
        pRN->S_DetCounter++;
    }
    else
    {
        pRN->S_DetCounter = 0;
    }

    pRN->quadsym2 = pRN->quadsym1;
    pRN->quadsym1 = quadsym;

    if (pRN->S_DetCounter > 90)
    {
        pRN->renego = 1;
    }
}
