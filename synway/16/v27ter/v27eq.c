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
/* T/2-Spaced Equalizer                                                    */
/*                                                                         */
/* Functions: void  Equalizer_init(MomdemStruct *pV27)                       */
/*                                                                         */
/* INPUT: V27Struct *pV27 - pointer to modem data structure                  */
/*                              - 'TimingIQ[]' is an array with the two    */
/*                                input IQ values.                         */
/*                                                                         */
/* OUTPUT: V27Struct *pV27 - pointer to modem data structure                 */
/*                              - 'EqOutIQ' contains the output IQ values  */
/*                                from the equalizer                       */
/***************************************************************************/

#include "v27ext.h"

void V27_EqSkipFilter(V27Struct *pV27)
{
    pV27->cqEqOutIQ.r = pV27->cDemodIQBuf[0].r;
    pV27->cqEqOutIQ.i = pV27->cDemodIQBuf[0].i;
}

void V27_Eq(V27Struct *pV27)
{
    QCFIRStruct *pcFir = &pV27->cfirEq;
    CQWORD *pcHead;
    CQWORD *pcTail;
    UWORD  Len;

    Len = pcFir->nTapLen;

    pcHead = pcFir->pcDline + pcFir->nOffset;
    pcTail = pcHead + Len;

    /* Save the input sample to the delayline */
    *pcHead++ = pV27->Poly.cqTimingIQ[0];
    *pcTail++ = pV27->Poly.cqTimingIQ[0];

    /* Reset the pointer to the beginning if necessary */
    pcFir->nOffset ++;

    if (pcFir->nOffset >= Len)
    {
        pcFir->nOffset = 0;
        pcHead = pcFir->pcDline;
    }

    pV27->cqEqOutIQ = DspcFir(pcFir, &(pV27->Poly.cqTimingIQ[1]));

    /* Scale up output */
    pV27->cqEqOutIQ.r <<= 1;
    pV27->cqEqOutIQ.i <<= 1;
}

void V27_RX_EqUpdate(V27Struct *pV27)
{
    DspcFirLMS(&(pV27->cfirEq), pV27->qEqBeta, &(pV27->cqEqErrorIQ));
}
