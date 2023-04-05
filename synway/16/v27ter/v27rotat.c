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
/* Rotator                                                                 */
/*                                                                         */
/* INPUT:  V27Struct *pV27 - pointer to modem data structure               */
/*                              - provides 'EqOutIQ' which is the point    */
/*                                to be rotated.                           */
/*                                                                         */
/* OUTPUT: V27Struct *pV27 - pointer to modem data structure               */
/*                              - provides 'RotateIQ' which is the         */
/*                                rotated output.                          */
/* ----------------------------------------------------------------------- */

#include "v27ext.h"
#include "ptable.h"

void V27_RX_Rotate_Dummy(V27Struct *pV27)
{
    pV27->cqRotateIQ.r = pV27->cqEqOutIQ.r;
    pV27->cqRotateIQ.i = pV27->cqEqOutIQ.i;

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV27->cqRotateIQ.r; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV27->cqRotateIQ.i; }

#endif
}

void V27_RX_Rotate(V27Struct *pV27)
{
    SDWORD qTempR, qTempI;
#if DRAWEPG
    SWORD  pntLoc;
    SWORD *pEpg = (SWORD *)(pV27->pTable[EPGDATA_IDX]);
#endif

    qTempR = QQMULQD(pV27->cqEqOutIQ.r, pV27->qRotateCos);
    qTempI = QQMULQD(pV27->cqEqOutIQ.i, pV27->qRotateSin);
    pV27->cqRotateIQ.r = QD15Q(qTempR + qTempI);

    qTempR = QQMULQD(pV27->cqEqOutIQ.i, pV27->qRotateCos);
    qTempI = QQMULQD(pV27->cqEqOutIQ.r, pV27->qRotateSin);
    pV27->cqRotateIQ.i = QD15Q(qTempR - qTempI);

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV27->cqEqOutIQ.r; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV27->cqEqOutIQ.i; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV27->qRotateCos; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV27->qRotateSin; }

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV27->cqRotateIQ.r; }

    if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = pV27->cqRotateIQ.i; }

#endif

#if DRAWEPG
    /* draw EPG */
    pntLoc = *pEpg;
    (*pEpg++) ++;
    pEpg += (pntLoc << 1);
    *pEpg++ = pV27->cqRotateIQ.r;
    *pEpg++ = pV27->cqRotateIQ.i;
#endif
}
