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
/* 2T/3-Spaced Equalizer                                                   */
/*                                                                         */
/* Functions: void V32_BypassEq(V32Struct *pV32)                           */
/*            void V32_Eq(V32Struct *pV32)                                 */
/*                                                                         */
/* INPUT: 'cqTimingIQ[]' is an array with the two input IQ values.         */
/*                                                                         */
/* OUTPUT: 'cqEqOutIQ' contains the output IQ values from the equalizer    */
/*                                                                         */
/* 97Jun   Hong        It is modified for the fixed point mode.            */
/* 96Jun23 Thomas Paul Use for V.32. Changed from T to T/2                 */
/***************************************************************************/

#include "v3217ext.h"

/* ---------- Equalizer ---------- */
void V32_BypassEq(V32ShareStruct *pV32Share)
{
    pV32Share->cqEqOutIQ = pV32Share->Poly.cqTimingIQ[0];
}

/* Data mode equalizer */
void V32_Eq_D(V32ShareStruct *pV32Share)
{
    pV32Share->cqEqOutIQ = DspcFir_2T3EQ(&(pV32Share->cfirEq_D), pV32Share->Poly.cqTimingIQ);
}

void V32_EqUpdate(V32ShareStruct *pV32Share)
{
    DspcFirLMS_2T3(&(pV32Share->cfirEq_D), pV32Share->qEqBeta, &pV32Share->cqEqErrorIQ);
}
