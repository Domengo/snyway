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

/* ----------------------------------------------------------------------- */
/* Rotater                                                                 */
/*                                                                         */
/* INPUT:  V29Struct *pV29 - pointer to modem data structure                 */
/*                              - provides 'EqOutIQ' which is the point    */
/*                                to be rotated.                           */
/*                                                                         */
/* OUTPUT: V29Struct *pV29 - pointer to modem data structure                 */
/*                              - provides 'RotateIQ' which is the         */
/*                                rotated output.                          */
/* ----------------------------------------------------------------------- */

#include "v29ext.h"

void  V29_RX_Rotate_Dummy(V29Struct *pV29)
{
    pV29->cqRotateIQ.r = pV29->cqEqOutIQ.r;
    pV29->cqRotateIQ.i = pV29->cqEqOutIQ.i;
}

void  V29_RX_Rotate(V29Struct *pV29)
{
    QDWORD TempR, TempI;

    TempR = QQMULQD(pV29->cqEqOutIQ.r, pV29->qRotateCos);
    TempI = QQMULQD(pV29->cqEqOutIQ.i, pV29->qRotateSin);
    pV29->cqRotateIQ.r = QDR15Q(TempR + TempI);

    TempR = QQMULQD(pV29->cqEqOutIQ.i, pV29->qRotateCos);
    TempI = QQMULQD(pV29->cqEqOutIQ.r, pV29->qRotateSin);
    pV29->cqRotateIQ.i = QDR15Q(TempR - TempI);
}
