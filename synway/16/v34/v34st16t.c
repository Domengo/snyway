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

/**************************************************************/
/*        The file declares several neccessary tables related */
/*        to 2/3 16-state convolutional code                  */
/**************************************************************/

#include "v34ext.h"

/***************************************************************************/
/* This data is used to specify the 4D subsets for 16-state 2/3 TC         */
/* D2setA=0, D2setB=1, D2setC=2, D2setD=3                                  */
/* D2setE=4, D2setF=5, D2setG=6, D2setH=7                                  */
/* A 4D type is the concatenation of two 2d subsets, for example:          */
/* Type1=(0,0), is equivalent to the concaternation of subset A and A.     */
/* A 4D subset is the combination of several types; we use a 2             */
/*     dimensional array to represent a subset;                            */
/* A 4d family is the union of all subsets leading from the same state     */
/*    of the code or to the same state. Note that there are only two       */
/*    families for code.                                                   */
/* Now, for the considered code, we have totally 8 subsets, with 4 subsets */
/* make up a family.                                                       */
/***************************************************************************/



/**********************************************************************
With U0(m)=0
***********************************************************************/
/**********************************************************************
There totally are 8 4D subsets:
************************************************************************/

/* -----------------------------------------------------------------------
  In each 4D subset, there are 8 groups of labels, s(2m) and s(2m+1)
  giving total of 16 labels.
                                                                Benjamin
                                                                May 14, 1996
------------------------------------------------------------------------- */

/*************************************************************************/
/* Following are the data relevant to the trellis diagram for 16 state   */
/* 2/3 trellis code:                                                     */
/* The design is as follows:                                             */
/* Using a array named state16[][], with each4 row cosisting of the      */
/* present state, all last state that terminated on the present state,   */
/* and the corresponding subsets represented by the number 1,2,3,4,5,6,7 */
/* and 8 ( ex. 3 standing for the set S16D4set3).                        */
/* The number for the state is not the real number of the state. The     */
/* relationship is given by:                                             */
/*  0=0, 1=1,  2=6,  3=7,  4=12, 5=13, 6=10, 7=11                        */
/*  8=2, 9=3,  10=4, 11=5, 12=14,13=15,14=8, 15=9                        */
/*                                                                       */
/* 0  1   2   3   4   5   6   7   8   9   10   11   12  13   14   15     */
/* 0  1   6   7   12  13  10  11  2   3    4    5   14  15   8    9      */
/*************************************************************************/

CONST UBYTE V34_tS16D4set_ptr[8][4] =
{
    { 0, 0, 2, 2},
    { 0, 2, 2, 0},
    { 1, 1, 3, 3},
    { 1, 3, 3, 1},
    { 0, 1, 2, 3},
    { 0, 3, 2, 1},
    { 1, 2, 3, 0},
    { 1, 0, 3, 2}
};


/*************************************************************************/
/* Trellis Diagram:                                                      */
/* There are 16 Ending-States, and each ending state can have 4 branches */
/* go into it (4 Starting states). Each branch associate with a label.   */
/* i.e. From each starting state, given a label ( U(m), Y1 , Y2 ), can   */
/* lead to a particular ending state.                                    */
/* In the structure below "trellis16", each row is representing an       */
/* Ending state, and each row has 4 groups of data. The 1st data in each */
/* group is the starting state, and the 2nd data in each group is the    */
/* label.                                                                */
/* The 1st row is Ending state 0,and the last row is the Ending state 15 */
/*************************************************************************/
CONST UBYTE V34_tTrellis16[128] =
{
    0, 0,  4, 2,  8, 1,  12, 3,
    0, 1,  4, 3,  8, 0,  12, 2,
    0, 2,  4, 0,  8, 3,  12, 1,
    0, 3,  4, 1,  8, 2,  12, 0,

    1, 4,  5, 6,  9, 5,  13, 7,
    1, 5,  5, 7,  9, 4,  13, 6,
    1, 6,  5, 4,  9, 7,  13, 5,
    1, 7,  5, 5,  9, 6,  13, 4,

    2, 1,  6, 3,  10, 0, 14, 2,
    2, 0,  6, 2,  10, 1, 14, 3,
    2, 3,  6, 1,  10, 2, 14, 0,
    2, 2,  6, 0,  10, 3, 14, 1,

    3, 5,  7, 7,  11, 4, 15, 6,
    3, 4,  7, 6,  11, 5, 15, 7,
    3, 7,  7, 5,  11, 6, 15, 4,
    3, 6,  7, 4,  11, 7, 15, 5
};
