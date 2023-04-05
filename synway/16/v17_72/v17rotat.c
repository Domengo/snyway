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

/***************************************************************************/
/* Rotater                                                                 */
/*                                                                         */
/* INPUT:  'EqOutIQ' which is the pointto be rotated.                      */
/*                                                                         */
/* OUTPUT: 'cqRotateIQ' which is the rotated output.                       */
/*                                                                         */
/* 98Dec28 Wood        change coding style, naming convention & optimize   */
/* 97Jun   Hong        It is modified for the fixed point mode.            */
/* 96Apr19 Thomas Paul changed naming convention                           */
/*                                                                         */
/* Author: Thomas Paul, GAO Research & Consulting Ltd.                     */
/*                                                                         */
/***************************************************************************/

#include "v17ext.h"
#include "ptable.h"

void V17_Rotate(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    QDWORD rot;

    rot  = QQMULQD(pV32Share->cqEqOutIQ.r, pV32Share->qRotateCos);
    rot += QQMULQD(pV32Share->cqEqOutIQ.i, pV32Share->qRotateSin);

    pV32Share->cqRotateIQ.r = QDR15Q(rot);

    rot  = QQMULQD(pV32Share->cqEqOutIQ.i, pV32Share->qRotateCos);
    rot -= QQMULQD(pV32Share->cqEqOutIQ.r, pV32Share->qRotateSin);

    pV32Share->cqRotateIQ.i = QDR15Q(rot);
}

void V17_BypassRotate(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    pV32Share->cqRotateIQ = pV32Share->cqEqOutIQ;
}
