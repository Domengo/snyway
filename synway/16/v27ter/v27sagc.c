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

void V27_CalSagcScaleEnergy(V27Struct *pV27)
{
    UBYTE i;
    QWORD qDemodIn;

    for (i = 0; i < pV27->ubSymBufSize; i++)
    {
        if (pV27->PCMinPtr[i] < 0)
        {
            qDemodIn = -pV27->qDemodIn[i];
        }
        else
        {
            qDemodIn =  pV27->qDemodIn[i];
        }

        pV27->qdSagcEnergy += QQMULQR15(qDemodIn - pV27->qdSagcEnergy, V27_SAGC_BETA);
    }
}

void V27_Sagc(V27Struct *pV27, QWORD Reference_Energy)
{
    pV27->qSagcScale = QQMULQD(pV27->qSagcScale, Reference_Energy) / pV27->qdSagcEnergy; /* 8.8 format */

    pV27->ubSagc_Flag = 0;

    TRACE1("V27: V27->qSagcScale= %d", pV27->qSagcScale);
}
