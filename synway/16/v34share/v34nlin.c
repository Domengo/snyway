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

#include "shareext.h"

/*****************************************************************************
   Non-linear Encoder

   INPUT: q15 qAve_eng     - NOT Using anymore, since we fix Ave_eng
        CQWORD *pcX_signal - Tx Symbol, with Tx_gain applied
             SWORD n       - # of Tx symbol

   OUTPUT: pcX_signal      - Moidfied Tx symbol with non-linear gain

   NOTES:   This version of Non-linear encoder assumes that all Input symbols
        have average Tx gain equals to 0.10393217
        Non-linear constant is equals to 0.3125 (V.34/9.7)
        We can pre-calculate all constants, in order to avoid divisions.

   Author:  Benjamin Chan, GAO Research & Consulting Ltd.

   Last Updated:
        Jan 08, 1998.
 ****************************************************************************/

#if !USE_ASM

/*pcX_signal of V34_Decoder() have NOT been correct! (3)*/
void V34_Non_Linear_Encoder(QWORD qNL_scale, CQWORD *pcX_signal)
{
    QWORD  qX;
    QWORD  qX2;
    QWORD  qGn;
    QDWORD qdS6, qdS120;
    QDWORD qdTemp;

    qdTemp  = QQMULQD(pcX_signal->r, pcX_signal->r);
    qdTemp += QQMULQD(pcX_signal->i, pcX_signal->i);

    qX = QDR15Q(qdTemp);/*NOT OVERFLOW*/   /*Rounding to 1.15*/

    /*see Equ.(9-34)/V.34*/
    qdS6 = QQMULQD(qX, QNLCONST_OVER_6);   /*2.30 format*/

    qX2 = QQMULQR15(qX, qX);

    qdS120 = QQMULQD(qX2, QNLCONST_OVER_120);/*2.30 format*/

    /*qdS6 & qdS120 have NOT been correct! (2)*/

    /*see Equ.(9-34)/V.34*/
    qGn = QDR15Q(qdS6 + qdS120);/*NOT OVERFLOW*/

#if 0//DANIEL_DUMP

    if (DumpTone1_Idx < 5000000) { DumpTone1[DumpTone1_Idx++] = pcX_signal->r + QQMULQR15(qGn, pcX_signal->r); }

    if (DumpTone2_Idx < 5000000) { DumpTone2[DumpTone2_Idx++] = pcX_signal->i + QQMULQR15(qGn, pcX_signal->i); }

#endif

    if ((ROUND15(QQMULQD(qGn, pcX_signal->r)) > 32767) || (ROUND15(QQMULQD(qGn, pcX_signal->r)) < -32768))
    {
        TRACE0("V34: BUG! OVERFLOW");
    }

    if ((ROUND15(QQMULQD(qGn, pcX_signal->i)) > 32767) || (ROUND15(QQMULQD(qGn, pcX_signal->i)) < -32768))
    {
        TRACE0("V34: BUG! OVERFLOW");
    }

    /*qGn, pcX_signal->r & pcX_signal->i have NOT been correct! (1)*/
    pcX_signal->r += QQMULQR15(qGn, pcX_signal->r);/*OVERFLOW!!!*/
    pcX_signal->i += QQMULQR15(qGn, pcX_signal->i);/*OVERFLOW!!!*/

    pcX_signal->r = QQMULQR15(pcX_signal->r, qNL_scale);
    pcX_signal->i = QQMULQR15(pcX_signal->i, qNL_scale);
}

/*****************************************************************************
   Non-linear Decoder (Fixed Point version )

   INPUT: q15 qAve_eng       - NOT Using anymore, since we fix Ave_eng
      CQWORD *pcX_signal - Tx Symbol, with Tx_gain applied

   OUTPUT: Back to normal gain

   Author:  Jack Liu, GAO Research & Consulting Ltd.
        Jan 10, 1998.
 ****************************************************************************/
void  V34_Non_Linear_Decoder(QDWORD nl_scale, CQWORD *pcX_signal)
{
    QDWORD qdMac;
    QWORD  qGain;
    QWORD  qGn;
    QWORD  qSn;
    QWORD  qSn2;
    QWORD  qTemp;

    qGain = (QWORD)(nl_scale >> 2);   /* Change to 3.13 format */
    qdMac = QQMULQD(qGain, pcX_signal->r);
    pcX_signal->r    = (QWORD)((qdMac + 0x1000) >> 13);

    qdMac = QQMULQD(qGain, pcX_signal->i);
    pcX_signal->i    = (QWORD)((qdMac + 0x1000) >> 13);

    qdMac  = QQMULQD(pcX_signal->r, pcX_signal->r);
    qdMac += QQMULQD(pcX_signal->i, pcX_signal->i);
    qTemp  = QDR15Q(qdMac);

    qdMac = QQMULQD(qNlGain, qTemp);
    qdMac = (qdMac + 0x1000) >> 13;

    if (qdMac > 32767)
    {
        qSn = 32767;
    }
    else
    {
        qSn = (QWORD)qdMac;
    }

    qSn2  = QQMULQR15(qSn, qSn);

    qdMac  = QQMULQD(qSn,  qNlDecoderC2);
    qdMac -= QQMULQD(qSn2, qNlDecoderC3);

    qTemp = QDR15Q(qdMac);

    qTemp -= qNlDecoderC1;

    qGn = QQMULQR15(qSn, (qTemp << 1));

    qTemp = QQMULQR15(qGn, pcX_signal->r);
    pcX_signal->r += qTemp;

    qTemp = QQMULQR15(qGn, pcX_signal->i);
    pcX_signal->i += qTemp;
}
#endif
