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

void V29_RX_Derotate_Get_TRN(V29Struct *pV29)
{
    QDWORD qDe_temp1, qDe_temp2;
    QWORD  qEr, qEi;

    /* find error */
    qEr = pV29->cqTrainIQ.r - pV29->cqRotateIQ.r;
    qEi = pV29->cqTrainIQ.i - pV29->cqRotateIQ.i;

    /* derotate error */
    qDe_temp1 = QQMULQD(qEr, pV29->qRotateCos);
    qDe_temp2 = QQMULQD(qEi, pV29->qRotateSin);
    pV29->cqEqErrorIQ.r = QDR15Q(qDe_temp1 - qDe_temp2);

    qDe_temp1 = QQMULQD(qEi, pV29->qRotateCos);
    qDe_temp2 = QQMULQD(qEr, pV29->qRotateSin);
    pV29->cqEqErrorIQ.i = QDR15Q(qDe_temp1 + qDe_temp2);
}

void  V29_RX_Derotate(V29Struct *pV29)
{
    QDWORD qDe_temp1, qDe_temp2;
    QWORD  qEr, qEi;

    /* find error */
    qEr = pV29->cqSliceIQ.r - pV29->cqRotateIQ.r;
    qEi = pV29->cqSliceIQ.i - pV29->cqRotateIQ.i;

    /* derotate error */
    qDe_temp1 = QQMULQD(qEr, pV29->qRotateCos);
    qDe_temp2 = QQMULQD(qEi, pV29->qRotateSin);
    pV29->cqEqErrorIQ.r = QDR15Q(qDe_temp1 - qDe_temp2);

    qDe_temp1 = QQMULQD(qEi, pV29->qRotateCos);
    qDe_temp2 = QQMULQD(qEr, pV29->qRotateSin);
    pV29->cqEqErrorIQ.i = QDR15Q(qDe_temp1 + qDe_temp2);
}
