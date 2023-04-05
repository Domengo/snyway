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
  This file takes the output of equalizer as input and from
  that it calculates the closest point from the map.
************************************************************/

#include "v29ext.h"

void V29_Slicer_init(V29Struct *pV29)
{
    if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_9600)
    {
        pV29->ubSigDemapSize = 16;
        pV29->pcSigDemapptr  = (CQWORD *)V29_tIQTable_Decode_9600;
    }
    else
#if V29_MODEM_ENABLE
        if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_7200)
#endif
        {
            pV29->ubSigDemapSize = 8;
            pV29->pcSigDemapptr  = (CQWORD *)V29_tIQTable_Decode_7200;
        }

#if V29_MODEM_ENABLE
        else /* if (pV29->ubBitsPerSym == V29_BITS_PER_SYMBOL_4800) */
        {
            pV29->ubSigDemapSize = 4;
            pV29->pcSigDemapptr  = (CQWORD *)V29_tIQTable_Decode_4800;
        }

#endif
}

void V29_RX_Slicer(V29Struct *pV29)
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
    cqInPnt.r = pV29->cqRotateIQ.r;
    cqInPnt.i = pV29->cqRotateIQ.i;

    for (i = 0; i < pV29->ubSigDemapSize; i++)
    {
        /* find point in I-Q Table */
        qREdist = cqInPnt.r - pV29->pcSigDemapptr[i].r;
        qIMdist = cqInPnt.i - pV29->pcSigDemapptr[i].i;

        /* see if shortest distance for path */
        qdNew_dist = QQMULQD(qREdist, qREdist) + QQMULQD(qIMdist, qIMdist);

        if (qdMin_dist > qdNew_dist)
        {
            qdMin_dist  = qdNew_dist;
            ubMin_point = i;
        }
    }

    /* store the output */
    pV29->ubDecodeOut = ubMin_point;

    /* store I-Q value for point */
    pV29->cqSliceIQ.r = pV29->pcSigDemapptr[ubMin_point].r;
    pV29->cqSliceIQ.i = pV29->pcSigDemapptr[ubMin_point].i;
}
