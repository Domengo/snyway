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

#include "dspdext.h"

#if SUPPORT_V34FAX
void DspIIR_Cas51_Init(IIR_Cas5Struct *pIIR, SWORD *pCoef, SWORD *pDline, UBYTE numbiquad)
{
    UBYTE i;

    pIIR->pCoef = pCoef;
    pIIR->numbiquad = numbiquad;
    pIIR->pDline = pDline;

    for (i = 0; i < ((numbiquad + 1) << 1); i++)
    {
        *pDline++ = 0;
    }
}

#if !USE_ASM
SWORD DspIIR_Cas51(IIR_Cas5Struct *pIIR, SWORD insample)
{
    UBYTE i, ubNumbiquad;
    SWORD temp, temp1, shift_flag;
    SWORD *pCoef;
    SWORD *pDline;
    SDWORD yn;

    pCoef = pIIR->pCoef;
    pDline = pIIR->pDline;
    ubNumbiquad = pIIR->numbiquad;

    temp = insample;

    for (i = 0; i < ubNumbiquad; i++)
    {
        shift_flag = *pCoef++;

        /* y(n)=b0*x(n) + b1*x(n-1) + b2*x(n-2) */
        yn = QQMULQD(*pCoef++, temp);
        temp1 = *pDline;
        (*pDline++) = temp;                       /* update x(n-1) */
        yn += QQMULQD(*pCoef++, temp1);
        yn += QQMULQD(*pCoef++, *pDline);
        (*pDline++) = temp1;                      /* update x(n-2) */

        /* y(n)=y(n) + (-a1)*y(n-1) + (-a2)*y(n-2) */
        yn += QQMULQD(*pCoef++, *pDline++);
        yn += QQMULQD(*pCoef++, *pDline--);

        if (shift_flag)
        {
            temp = (SWORD)((yn + 0x2000) >> 14);
        }
        else
        {
            temp = QDR15Q(yn);
        }
    }

    temp1 = *pDline;
    (*pDline++) = temp;                           /* update y(n-1) */
    *pDline = temp1;                              /* update y(n-2) */

    return(temp);
}
#endif
#endif