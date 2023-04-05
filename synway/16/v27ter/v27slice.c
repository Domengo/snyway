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

/***********************************************************
file : V27slice.c

  This file takes the output of equalizer as input and from
  that it calculates  the closest point from the map.
**************************************************************/

#include "v27ext.h"

void V27_Slicer_init(V27Struct *pV27)
{
    if (pV27->ubBitsPerSym == 3)
    {
        pV27->ubSigDemapSize = 8;
        pV27->pcSigDemapptr = (CQWORD *)tV27_IQTable_Decode_4800;
    }
    else
    {
        pV27->ubSigDemapSize = 4;
        pV27->pcSigDemapptr = (CQWORD *)tV27_IQTable_Decode_2400;
    }
}

void V27_RX_Slicer(V27Struct *pV27)
{
    UBYTE i;
    CQWORD cqInPnt;
    UBYTE  ubMin_point;
    QDWORD qdMin_dist, qdNew_dist;
    QWORD  qREdist, qIMdist;

    /* find closest IQ point */
    ubMin_point = 0;
    qdMin_dist  = Q31_MAX;

    /* input IQ point */
    cqInPnt.r = pV27->cqRotateIQ.r;   /* cqEqOutIQ.r */
    cqInPnt.i = pV27->cqRotateIQ.i;   /* cqEqOutIQ.i */

    for (i = 0; i < pV27->ubSigDemapSize; i++)
    {
        /* find point in I-Q Table */
        qREdist  = cqInPnt.r - pV27->pcSigDemapptr[i].r;
        qIMdist  = cqInPnt.i - pV27->pcSigDemapptr[i].i;

        /* see if shortest distance for path */
        qdNew_dist = QQMULQD(qREdist, qREdist) + QQMULQD(qIMdist, qIMdist);

        if (qdMin_dist > qdNew_dist)
        {
            qdMin_dist  = qdNew_dist;
            ubMin_point = i;
        }
    }

    /* store I-Q value for point */
    pV27->cqSliceIQ.r = pV27->pcSigDemapptr[ubMin_point].r;
    pV27->cqSliceIQ.i = pV27->pcSigDemapptr[ubMin_point].i;

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV27->cqSliceIQ.r; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV27->cqSliceIQ.i; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = ubMin_point; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV27->cqRotateIQ.r; }

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV27->cqRotateIQ.i; }

#endif
}
