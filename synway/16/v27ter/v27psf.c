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
#include "ptable.h"

/* This function initializes two ifilter one by one and
calls the Init_ifilter function for each one.  */

void V27_iFilter_init(V27Struct *pV27)
{
    if (pV27->ubBitsPerSym == 3)   /* baud rate 1600 */
    {
        DspiFirInit(&(pV27->pPsfI), (SWORD *)tV27_low_pass_coef_1600, pV27->qPsfDlineI, V27_PSF_DLINE_LEN_1600, V27_SYM_SIZE_1600);
        DspiFirInit(&(pV27->pPsfQ), (SWORD *)tV27_low_pass_coef_1600, pV27->qPsfDlineQ, V27_PSF_DLINE_LEN_1600, V27_SYM_SIZE_1600);
    }
    else
    {
        /* baud rate 1200 */
        DspiFirInit(&(pV27->pPsfI), (SWORD *)tV27_low_pass_coef_1200, pV27->qPsfDlineI, V27_PSF_DLINE_LEN_1200, V27_SYM_SIZE_1200);
        DspiFirInit(&(pV27->pPsfQ), (SWORD *)tV27_low_pass_coef_1200, pV27->qPsfDlineQ, V27_PSF_DLINE_LEN_1200, V27_SYM_SIZE_1200);
    }
}

void V27_TX_Psf(V27Struct *pV27)/* pulse shaping */
{
#if DRAWEPG
    SWORD pntLoc;
    SWORD *pEpg = (SWORD *)(pV27->pTable[EPGDATA_IDX]);

    /* EPG sending */
    pntLoc = *pEpg;
    (*pEpg++) ++;
    pEpg += (pntLoc << 1);
    *pEpg++ = pV27->cqSigMapIQ.r;
    *pEpg++ = pV27->cqSigMapIQ.i;
#endif

    DspiFir(&(pV27->pPsfI), pV27->cqSigMapIQ.r, pV27->pqPsfOutIptr);
    DspiFir(&(pV27->pPsfQ), pV27->cqSigMapIQ.i, pV27->pqPsfOutQptr);
}
