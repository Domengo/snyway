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
/* Pulse Shape Filter                                                      */
/*                                                                         */
/* INPUT:  'TxSigMapIQ' which is the I-Q point to be pulse-shape filtered  */
/*                                                                         */
/* OUTPUT: pulse-shaped filtered output in 'PsfOutIptr', and 'PsfOutQptr'  */
/***************************************************************************/

#include <string.h>
#include "v3217ext.h"

#if USE_ASM
#include "ptable.h"
#endif

void V32_PsfInit(UBYTE **pTable, V32ShareStruct *pV32Share)
{
    /* init. interpolating filters */
#if USE_ASM
    QWORD *PsfDlineQ = (QWORD *)(pTable[CIRCDATA_IDX]);
    QWORD *PsfDlineI = (QWORD *)(pTable[CIRCDATA_IDX] + 32);

    Dspi3Fir_PsfInit(&pV32Share->PsfI, (QWORD *)Dspi3Fir_Psf_coef, PsfDlineI, V32V29_PSF_DELAY_LEN, V32_SYM_SIZE);
    Dspi3Fir_PsfInit(&pV32Share->PsfQ, (QWORD *)Dspi3Fir_Psf_coef, PsfDlineQ, V32V29_PSF_DELAY_LEN, V32_SYM_SIZE);
#else
    Dspi3Fir_PsfInit(&pV32Share->PsfI, (QWORD *)Dspi3Fir_Psf_coef, pV32Share->qPsfDlineI, V32V29_PSF_DELAY_LEN, V32_SYM_SIZE);
    Dspi3Fir_PsfInit(&pV32Share->PsfQ, (QWORD *)Dspi3Fir_Psf_coef, pV32Share->qPsfDlineQ, V32V29_PSF_DELAY_LEN, V32_SYM_SIZE);
#endif

    pV32Share->qXmitLevelAdjust = 461;
}

/* pulse shaping */
void V32_Psf(V32ShareStruct *pV32Share)
{
    /* Xmit power level control */
    pV32Share->cqSigMapIQ.r = QQMULQR8(pV32Share->cqSigMapIQ.r, pV32Share->qXmitLevelAdjust);
    pV32Share->cqSigMapIQ.i = QQMULQR8(pV32Share->cqSigMapIQ.i, pV32Share->qXmitLevelAdjust);

    Dspi3Fir_Psf(&pV32Share->PsfI, pV32Share->cqSigMapIQ.r, pV32Share->qPsfOutBufI);
    Dspi3Fir_Psf(&pV32Share->PsfQ, pV32Share->cqSigMapIQ.i, pV32Share->qPsfOutBufQ);
}
