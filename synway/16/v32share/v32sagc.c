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

#include "v3217ext.h"

void V32_CalSagcScaleEnergy(V32ShareStruct *pV32Share)
{
    QWORD  qInVal;
    UBYTE  i;

    /* update AGC energy */
    for (i = 0; i < V32_SYM_SIZE; i++)
    {
        qInVal = pV32Share->qDemodIn[i];

        pV32Share->qdSagcEnergy += QQMULQD(qInVal, qInVal) >> 9;
    }
}

void V32_Sagc(V32ShareStruct *pV32Share, QWORD qAGCRef)
{
    QDWORD qdAGCscale;
    QWORD qEgy;

    qEgy = QDsqrt(pV32Share->qdSagcEnergy);

    if (qEgy == 0)
    {
        TRACE0("V32: Energy is zero!!!");
        pV32Share->qSagcScale = 256;

        return;
    }

    qdAGCscale = QQMULQD(qAGCRef, pV32Share->qSagcScale) / qEgy;

    if (qdAGCscale > 32767)
    {
        pV32Share->qSagcScale = 32767;
    }
    else
    {
        pV32Share->qSagcScale = (QWORD)qdAGCscale;
    }

    pV32Share->ubSagc_Flag = 0;

    TRACE1("V32: qSagcScale=%d", pV32Share->qSagcScale);
}
