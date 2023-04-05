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
/*                                                                         */
/* SLICE32.C                                                               */
/* --------                                                                */
/*                                                                         */
/* INPUT:  'cqRotateIQ' which is the I-Q point to be demapped              */
/*                                                                         */
/* OUTPUT: - 'ubTrellisDecodeOut': demapped output (a group of bits)       */
/*         - 'cqSliceIQ': I-Q point determined in the decision process     */
/***************************************************************************/

#include "v3217ext.h"

void V32_SliceInit(V32ShareStruct *pV32Share)
{
    pV32Share->ubSigDemapSize   = 4;
    pV32Share->pcSigDemapTab    = cV32_IQTab_4800NR;
    pV32Share->ubSigTransMap     = 1;
    pV32Share->pnSigRotmapTab   = nV32_IQRotateTab_4800NR;
    pV32Share->ubSigIQtoBitsTab = ubV32_IQtoBitsTab_4800NR;
}

/* ---------- Slice ---------- */

void V32_Slice(V32ShareStruct *pV32Share)
{
    QDWORD rot;
    SWORD  sig;
    SWORD  SigDemapDim;
    CQWORD cqRotPt;
    CQWORD cqMapPt;
    CQWORD *pcSigDemapTab;

    /* rotate and scale input IQ point */
    rot       = QQMULQD(pV32Share->cqRotateIQ.r, pV32Share->pnSigRotmapTab[0]);
    rot      += QQMULQD(pV32Share->cqRotateIQ.i, pV32Share->pnSigRotmapTab[1]);
    cqRotPt.r = QDR15Q(rot);

    rot       = QQMULQD(pV32Share->cqRotateIQ.i, pV32Share->pnSigRotmapTab[0]);
    rot      -= QQMULQD(pV32Share->cqRotateIQ.r, pV32Share->pnSigRotmapTab[1]);
    cqRotPt.i = QDR15Q(rot);

    sig = pV32Share->ubSigTransMap;

    /* obtain and store the output bits */
    SigDemapDim = sig + 1;

    cqMapPt.r = (cqRotPt.r + (SigDemapDim << (V32_SIGMAP_SCALE - 1))) >> V32_SIGMAP_SCALE;
    cqMapPt.i = (cqRotPt.i + (SigDemapDim << (V32_SIGMAP_SCALE - 1))) >> V32_SIGMAP_SCALE;

    /* clip map point */
    if (cqMapPt.r < 0)
    {
        cqMapPt.r = 0;
    }

    if (cqMapPt.i < 0)
    {
        cqMapPt.i = 0;
    }

    if (cqMapPt.r > sig)
    {
        cqMapPt.r = sig;
    }

    if (cqMapPt.i > sig)
    {
        cqMapPt.i = sig;
    }

    pV32Share->ubTrellisDecodeOut = pV32Share->ubSigIQtoBitsTab[cqMapPt.r + SigDemapDim * cqMapPt.i];

    /* store I-Q value for point */
    pcSigDemapTab     = (CQWORD *)pV32Share->pcSigDemapTab;
    pV32Share->cqSliceIQ.r = pcSigDemapTab[pV32Share->ubTrellisDecodeOut].r << V32_SIGMAP_SCALE;
    pV32Share->cqSliceIQ.i = pcSigDemapTab[pV32Share->ubTrellisDecodeOut].i << V32_SIGMAP_SCALE;
}
