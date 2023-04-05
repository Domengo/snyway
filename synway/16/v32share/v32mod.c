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
/* Quadrature Amplitude Modulation                                         */
/*                                                                         */
/* INPUT:  'qPsfOutBufI', and 'qPsfOutBufI', which provides the            */
/*         pulse-shaped filtered values to be modulated                    */
/*                                                                         */
/* OUTPUT: modulated values in the buffer pointed to by 'pPCMout'          */
/***************************************************************************/

#include "v3217ext.h"

void V32_Mod(V32ShareStruct *pV32Share)
{
    QWORD  qCos, qSin;
    UWORD  phase;
    QDWORD temp;
    UBYTE  i;

    phase = pV32Share->uModPhase;

    for (i = 0; i < V32_SYM_SIZE; i++)
    {
        /* find cos, sin values */
        SinCos_Lookup(&phase, V32_MOD_DELTA_PHASE, &qSin, &qCos);
        /* modulate and store I and Q value for point */
        temp  = QQMULQD(pV32Share->qPsfOutBufI[i], qCos);
        temp -= QQMULQD(pV32Share->qPsfOutBufQ[i], qSin);
        pV32Share->PCMoutPtr[i] = QDR15Q(temp);
    }

    pV32Share->uModPhase = phase;
}
