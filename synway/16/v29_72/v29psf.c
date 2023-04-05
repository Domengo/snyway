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

#if USE_ASM
extern QWORD   PsfDlineI[];
extern QWORD   PsfDlineQ[];
#endif

void V29_iFilter_init(V29Struct *pV29)
{
#if USE_ASM
    Dspi3Fir_PsfInit(&(pV29->pPsfI), (SWORD *)Dspi3Fir_Psf_coef, PsfDlineI, V32V29_PSF_DELAY_LEN, V29_SYM_SIZE);  /* Init for I filter */
    Dspi3Fir_PsfInit(&(pV29->pPsfQ), (SWORD *)Dspi3Fir_Psf_coef, PsfDlineQ, V32V29_PSF_DELAY_LEN, V29_SYM_SIZE);
#else
    Dspi3Fir_PsfInit(&(pV29->pPsfI), (SWORD *)Dspi3Fir_Psf_coef, pV29->qPsfDlineI, V32V29_PSF_DELAY_LEN, V29_SYM_SIZE);  /* Init for I filter */
    Dspi3Fir_PsfInit(&(pV29->pPsfQ), (SWORD *)Dspi3Fir_Psf_coef, pV29->qPsfDlineQ, V32V29_PSF_DELAY_LEN, V29_SYM_SIZE);  /* Init for Q filter */
#endif
}

void V29_TX_Psf(V29Struct *pV29)/* pulse shaping */
{
    Dspi3Fir_Psf(&(pV29->pPsfI), pV29->cqSigMapIQ.r, pV29->pqPsfOutIptr);
    Dspi3Fir_Psf(&(pV29->pPsfQ), pV29->cqSigMapIQ.i, pV29->pqPsfOutQptr);
}
