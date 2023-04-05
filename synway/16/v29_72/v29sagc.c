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

#include "v29ext.h"
#include "ptable.h"
#include "ioapi.h"
#include "mhsp.h"

void V29_CalSagcScaleEnergy(V29Struct *pV29)
{
    QWORD  qInVal;
    UBYTE  i;

    for (i = 0; i < V29_SYM_SIZE; i++)
    {
        qInVal = pV29->qDemodIn[i];

        pV29->qdSagcEnergy += QQMULQD(qInVal, qInVal) >> 9;
    }
}

void V29_Sagc(V29Struct *pV29, QWORD qAGCRef)
{
    QDWORD qdAGCscale;
    QWORD qEgy;

    qEgy = QDsqrt(pV29->qdSagcEnergy);

    if (qEgy == 0)
    {
        TRACE0("V29: Energy is zero!!!");
        pV29->qSagcScale = 256;
        return;
    }

    qdAGCscale = QQMULQD(qAGCRef, pV29->qSagcScale) / qEgy;

    if (qdAGCscale > 32767)
    {
        pV29->qSagcScale = 32767;
    }
    else
    {
        pV29->qSagcScale = (QWORD)qdAGCscale;
    }

    pV29->ubSagc_Flag = 0;

    TRACE1("V29: qSagcScale=%d", pV29->qSagcScale);
}
