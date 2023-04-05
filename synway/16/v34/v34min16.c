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

#include "v34ext.h"

#if !USE_ASM
void V34_State16_D2Min(CQWORD *pcSym, CQWORD pcResult[])
{
    CQWORD pcTemp_res[4];
    SWORD  r_p1, i_p1, r_n1, i_n1;
    SWORD  ar, ai, dr;
    UBYTE  idxr, idxi;
    UBYTE  i, lab;

    /* round the real and imaginary parts of "pcSym" */
    /* into ax_int and ay_int respectively.        */
    ar = pcSym->r >> V34_SHIFT_BITS; /* round to 9.7 format*/
    ai = pcSym->i >> V34_SHIFT_BITS; /* round to 9.7 format*/

    idxr = ar & 0x1;
    idxi = ai & 0x1;

    /* if ax_int and ay_int are both even */
    /*  Subset displacement
    3   0
    x ------- received point, aint
    2   1
    */

    if ((idxr == 1) && (idxi == 1))
    {
        /*  Subset displacement
        3   0
        c ------- move the received point here
        received point----(2)x   1
            */
        ar ++;
        ai ++;
    }

    else if ((idxr == 1) && (idxi == 0))
    {
        /*  Subset displacement
        3   0
        received point----   x c ------- move the received point
        2   1
        */
        ar ++;
    }
    else if ((idxr == 0) && (idxi == 1))
        /*  Subset displacement
        3   0
        c ------- move the received point
        2 x 1
        */
    {
        ai ++;
    }

    r_p1 = ar + 1;
    i_p1 = ai + 1;

    r_n1 = ar - 1;
    i_n1 = ai - 1;

    /* immediately, we can find 4 point, each belonging to a subset */
    pcTemp_res[0].r = r_p1;
    pcTemp_res[0].i = i_p1;

    pcTemp_res[1].r = r_p1;
    pcTemp_res[1].i = i_n1;

    pcTemp_res[2].r = r_n1;
    pcTemp_res[2].i = i_n1;

    pcTemp_res[3].r = r_n1;
    pcTemp_res[3].i = i_p1;

    /* Find the labels associated with the above 8 signal points */
    for (i = 0; i < 4; i++)
    {
        /* for 16 state encoder, the label can alternatively and
        easily obtainded by:
            */ // (1,1)->0, (1,3)->1, (3,3)->2, (3,1)->3, MSb of s(2m) and s(2m+1) doesn't matter
        idxr = pcTemp_res[i].r & 0x2;
        dr = idxr >> 1; // check if the 2nd LSb is 1
        idxi = (pcTemp_res[i].i >> 1) & 0x1; // check if the 2nd LSb is 1
        lab = (idxr + (dr ^ idxi)) & 0xFF;
        pcResult[lab].r = pcTemp_res[i].r << V34_SHIFT_BITS;
        pcResult[lab].i = pcTemp_res[i].i << V34_SHIFT_BITS;
    }
}

#if VITERBI_ON
void  V34_State16_D4Min(iD4SYMBOL *pIn, iD4SYMBOL *pOut, UBYTE *pU)
{
    VA_INFO *pVA;
    CQWORD *piD2_near1, *piD2_near2;
    SDWORD *D4_metric;
    SDWORD min_metric0[4], min_metric1[4];
    SDWORD metric, minmetric;
    SWORD l1, l2;
    UBYTE i;
    UBYTE *set;
    UBYTE *d4set_ptr;

    pVA = (VA_INFO *)pU;
    D4_metric = pVA->pD4_metric;
    piD2_near1 = pVA->pD2_near1;
    piD2_near2 = pVA->pD2_near2;
    d4set_ptr = (UBYTE *)pVA->Addr.pD4set_ptr;

    for (i = 0; i < 4; i++)
    {
        l1 = pIn->cS1.r - piD2_near1[i].r;
        l2 = pIn->cS1.i - piD2_near1[i].i;
        min_metric0[i] = QQMULQD(l1, l1) + QQMULQD(l2, l2);

        l1 = pIn->cS2.r - piD2_near2[i].r;
        l2 = pIn->cS2.i - piD2_near2[i].i;
        min_metric1[i] = QQMULQD(l1, l1) + QQMULQD(l2, l2);
    }

    for (i = 0; i < 8; i++)
    {
        set = d4set_ptr + i * 4;
        minmetric  = min_metric0[set[0]];
        minmetric += min_metric1[set[1]];

        metric  = min_metric0[set[2]];
        metric += min_metric1[set[3]];

        if (metric < minmetric)
        {
            minmetric = metric;
            set += 2;
        }

        D4_metric[0] = minmetric;
        D4_metric[8] = minmetric; /* Double buffer for V0 adjustment */
        ++D4_metric;

        pOut->cS1.r = piD2_near1[set[0]].r;
        pOut->cS1.i = piD2_near1[set[0]].i;

        pOut->cS2.r = piD2_near2[set[1]].r;
        pOut->cS2.i = piD2_near2[set[1]].i;

        ++pOut;
    }

}
#else
void  V34_State16_D4Min(iD4SYMBOL *pIn, iD4SYMBOL *pOut, UBYTE *pU)
{
    VA_INFO *pVA;
    CQWORD *piD2_near1, *piD2_near2;
    SDWORD min_metric1[4], min_metric2[4];
    SDWORD MinMetric1, MinMetric2;
    SWORD l1, l2;
    UBYTE i;

    pVA = (VA_INFO *)pU;
    piD2_near1 = pVA->pD2_near1;
    piD2_near2 = pVA->pD2_near2;

    for (i = 0; i < 4; i++)
    {
        l1 = pIn->cS1.r - piD2_near1[i].r;
        l2 = pIn->cS1.i - piD2_near1[i].i;
        min_metric1[i] = QQMULQD(l1, l1) + QQMULQD(l2, l2);

        l1 = pIn->cS2.r - piD2_near2[i].r;
        l2 = pIn->cS2.i - piD2_near2[i].i;
        min_metric2[i] = QQMULQD(l1, l1) + QQMULQD(l2, l2);
    }

    MinMetric1 = 0x7FFFFFF;
    MinMetric2 = 0x7FFFFFF;

    for (i = 0; i < 4; i++)
    {
        if (MinMetric1 > min_metric1[i])
        {
            MinMetric1 = min_metric1[i];

            pOut->cS1.r = piD2_near1[i].r;
            pOut->cS1.i = piD2_near1[i].i;
        }

        if (MinMetric2 > min_metric2[i])
        {
            MinMetric2 = min_metric2[i];

            pOut->cS2.r = piD2_near2[i].r;
            pOut->cS2.i = piD2_near2[i].i;
        }
    }
}
#endif

#endif
