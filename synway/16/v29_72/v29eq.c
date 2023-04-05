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

/**************************************************************************
* 2T/3-Spaced Equalizer                                                   *
*                                                                         *
*            void  V29_RX_Eq(MomdemStruct *pV29)                            *
*                                                                         *
* INPUT: V29Struct *pV29 - pointer to modem data structure                  *
*                              - 'TimingIQ[]' is an array with the two    *
*                                input IQ values.                         *
*                                                                         *
* OUTPUT: V29Struct *pV29 - pointer to modem data structure                 *
*                              - 'EqOutIQ' contains the output IQ values  *
*                                from the equalizer                       *
*                                                                         *
* Date: Feb 13, 1996                                                      *
*       Mar 08, 1996. Modified for double buffering                       *
*       Jun 23, 1996. Thomas Paul. Use for V.32. Changed from T to T/2    *
*       Nov 29, 1996. Thomas Paul. Use for V29Struct                      *
*                                                                         *
*  Author: Benjamin Chan, GAO Research & Consulting Ltd.                  *
*         Xian, GAO Research & Consulting Ltd.                            *
* Hong Wang change it to 2T/3 Equalizer for 7200 sampling Rate            *
**************************************************************************/

#include "v29ext.h"

void V29_RX_Eq_Dummy(V29Struct *pV29)
{
    pV29->cqEqOutIQ.r = pV29->Poly.cqTimingIQ[0].r;
    pV29->cqEqOutIQ.i = pV29->Poly.cqTimingIQ[0].i;
}

void V29_RX_Eq(V29Struct *pV29)
{
    pV29->cqEqOutIQ = DspcFir_2T3EQ(&(pV29->cfirEq), pV29->Poly.cqTimingIQ);
}

void  V29_RX_EqUpdate(V29Struct *pV29)
{
    DspcFirLMS_2T3(&(pV29->cfirEq), pV29->qEqBeta, &pV29->cqEqErrorIQ);
}
