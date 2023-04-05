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

#include "v27ext.h"

void V27_TX_Modulate(V27Struct *pV27)
{
    UBYTE  i;
    QWORD  cf, sf;
    QWORD  cos, sin;
    QDWORD qdTemp;
    UWORD  Phase;

    Phase = pV27->ModPhase;

    for (i = 0; i < pV27->ubSymBufSize; i++)
    {
        /* find cos, sin values */
        SinCos_Lookup(&Phase, V27_CARRIER_1800_DELTA_PHASE, &sin, &cos);

        /* modulate and store I and Q value for point */
        cf = pV27->pqPsfOutIptr[i];
        sf = pV27->pqPsfOutQptr[i];
        qdTemp = QQMULQD(cf, cos) - QQMULQD(sf, sin);
        pV27->PCMoutPtr[i] = QD15Q(qdTemp);
    }

    pV27->ModPhase = Phase;
}
