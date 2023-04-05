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

/* this function assume a & b are absolute & QWORD */
QWORD qDiva(QWORD a, QWORD b)
{
    QDWORD qdTemp1, qdTemp2;
    UBYTE i;

    qdTemp1 = (QDWORD)(a - b) << 16;
    qdTemp2 = (QDWORD)(b) << 15;

    for (i = 0; i < 16; i++)
    {
        if (qdTemp1 < 0)
        {
            qdTemp1 = ((qdTemp1 + qdTemp2) << 1);
        }
        else
        {
            qdTemp1 = (((qdTemp1 - qdTemp2) << 1) | 1);
        }
    }

    return (QWORD)(qdTemp1 & 0xFFFF);
}

#if 0
QDWORD QDQDdivQD(QDWORD qdDividend, QDWORD qdDivisor)
{
    QDWORD qdRemainder, qdTemp, qdQuotient;
    UBYTE  ubSign;
    UBYTE  ubShift, ubDividendBitNum, ubDivisorBitNum;
    UBYTE  i;
    SBYTE  bDiffBitNum;

    if (qdDividend == 0)
    {
        return(0);
    }
    else if (qdDivisor == 0)
    {
        return (Q31_MAX);
    }
    else if (qdDividend > 0)
    {
        if (qdDivisor > 0)
        {
            ubSign = 0;
        }
        else
        {
            qdDivisor = -qdDivisor;
            ubSign = 1;
        }
    }
    else
    {
        if (qdDivisor > 0)
        {
            qdDividend = -qdDividend;
            ubSign = 1;
        }
        else
        {
            qdDividend = -qdDividend;
            qdDivisor   = -qdDivisor;
            ubSign = 0;
        }
    }

    if (qdDividend < qdDivisor)
    {
        return(0);
    }

    ubShift = 0;

    if (qdDividend < 32768)
    {
        if (qdDividend < 128)
        {
            while (128 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 8 - ubShift;
        }
        else
        {
            while (32768 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 16 - ubShift;
        }
    }
    else
    {
        if (qdDividend < 0x00800000)
        {
            while (0x00800000 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 24 - ubShift;
        }
        else
        {
            while (TWO_30 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 31 - ubShift;
        }
    }

    ubShift = 0;

    if (qdDivisor < 32768)
    {
        if (qdDivisor < 128)
        {
            while (128 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 8 - ubShift;
        }
        else
        {
            while (32768 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 16 - ubShift;
        }
    }
    else
    {
        if (qdDivisor < 0x00800000)
        {
            while (0x00800000 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 24 - ubShift;
        }
        else
        {
            while (TWO_30 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 31 - ubShift;
        }
    }

    bDiffBitNum = ubDividendBitNum - ubDivisorBitNum;

    if (bDiffBitNum == 0)
    {
        qdQuotient = 1;
    }
    else
    {
        qdRemainder = qdDividend - ((QDWORD)qdDivisor << bDiffBitNum);
        qdTemp = (QDWORD)(qdDivisor) << (bDiffBitNum - 1);
        qdQuotient = 0;

        for (i = 0; i < bDiffBitNum; i++)
        {
            if (qdRemainder < 0)
            {
                qdRemainder = ((qdRemainder + qdTemp) << 1);
            }
            else
            {
                qdRemainder = ((qdRemainder - qdTemp) << 1);
                qdQuotient |= 1;
            }

            qdQuotient <<= 1;
        }

        if (qdRemainder >= 0)
        {
            qdQuotient |= 1;
        }
    }

    if (ubSign == 1)
    {
        qdQuotient = -qdQuotient;
    }

    return (qdQuotient);
}
#endif

QDWORD QDQDdivQD(QDWORD qdDividend, QDWORD qdDivisor)
{
    QDWORD qdTemp1, qdTemp2, qdTempRemain, qdQuotient;
    UBYTE  ubSign;
    UBYTE  ubShift, ubDividendBitNum, ubDivisorBitNum;
    UBYTE  i;
    SBYTE  bDiffBitNum;

    if (qdDividend == 0)
    {
        return(0);
    }
    else if (qdDivisor == 0)
    {
        return (Q31_MAX);
    }
    else if (qdDividend > 0)
    {
        if (qdDivisor > 0)
        {
            ubSign = 0;
        }
        else
        {
            qdDivisor = -qdDivisor;
            ubSign = 1;
        }
    }
    else
    {
        if (qdDivisor > 0)
        {
            qdDividend = -qdDividend;
            ubSign = 1;
        }
        else
        {
            qdDividend = -qdDividend;
            qdDivisor   = -qdDivisor;
            ubSign = 0;
        }
    }

    if (qdDividend < qdDivisor)
    {
        return(0);
    }

    ubShift = 0;

    if (qdDividend < 32768)
    {
        if (qdDividend < 128)
        {
            while (128 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 8 - ubShift;
        }
        else
        {
            while (32768 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 16 - ubShift;
        }
    }
    else
    {
        if (qdDividend < 0x00800000)
        {
            while (0x00800000 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 24 - ubShift;
        }
        else
        {
            while (TWO_30 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 31 - ubShift;
        }
    }

    ubShift = 0;

    if (qdDivisor < 32768)
    {
        if (qdDivisor < 128)
        {
            while (128 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 8 - ubShift;
        }
        else
        {
            while (32768 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 16 - ubShift;
        }
    }
    else
    {
        if (qdDivisor < 0x00800000)
        {
            while (0x00800000 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 24 - ubShift;
        }
        else
        {
            while (TWO_30 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 31 - ubShift;
        }
    }

    bDiffBitNum = ubDividendBitNum - ubDivisorBitNum;

    if (bDiffBitNum == 0)
    {
        qdTempRemain = qdDividend - qdDivisor;
        qdQuotient = 1;
    }
    else
    {
        qdTemp1 = ((QDWORD)qdDivisor) << bDiffBitNum;
        qdTempRemain = qdDividend;
        qdQuotient = 0;

        for (i = 0; i < bDiffBitNum; i++)
        {
            qdTemp2 = qdTempRemain - qdTemp1;

            if (qdTemp2 >= 0)
            {
                qdTempRemain = qdTemp2;
                qdQuotient |= 1;
            }

            qdTempRemain <<= 1;
            qdQuotient <<= 1;
        }

        qdTemp2 = qdTempRemain - qdTemp1;

        if (qdTemp2 >= 0)
        {
            qdTempRemain = qdTemp2;
            qdQuotient |= 1;
        }

        qdTempRemain >>= bDiffBitNum;
    }

    if (ubSign == 1)
    {
        qdQuotient = -qdQuotient;
    }

    return (qdQuotient);
}

#if 0
QDWORD QDQDdivQDRem(QDWORD qdDividend, QDWORD qdDivisor, QDWORD *qdRemainder)
{
    QDWORD qdTemp1, qdTemp2, qdTempRemain, qdQuotient;
    UBYTE  ubSign, ubSign_Dividend;
    UBYTE  ubShift, ubDividendBitNum, ubDivisorBitNum;
    UBYTE  i;
    SBYTE  bDiffBitNum;

    if (qdDividend == 0)
    {
        *qdRemainder = 0;
        return(0);
    }
    else if (qdDivisor == 0)
    {
        *qdRemainder = Q31_MAX;
        return (Q31_MAX);
    }
    else if (qdDividend > 0)
    {
        ubSign_Dividend = 0;

        if (qdDivisor > 0)
        {
            ubSign = 0;
        }
        else
        {
            qdDivisor = -qdDivisor;
            ubSign = 1;
        }
    }
    else
    {
        ubSign_Dividend = 1;

        if (qdDivisor > 0)
        {
            qdDividend = -qdDividend;
            ubSign = 1;
        }
        else
        {
            qdDividend = -qdDividend;
            qdDivisor   = -qdDivisor;
            ubSign = 0;
        }
    }

    if (qdDividend < qdDivisor)
    {
        if (ubSign_Dividend == 1)
        {
            *qdRemainder = -qdDividend;
        }
        else
        {
            *qdRemainder = qdDividend;
        }

        return(0);
    }

    ubShift = 0;

    if (qdDividend < 32768)
    {
        if (qdDividend < 128)
        {
            while (128 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 8 - ubShift;
        }
        else
        {
            while (32768 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 16 - ubShift;
        }
    }
    else
    {
        if (qdDividend < 0x00800000)
        {
            while (0x00800000 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 24 - ubShift;
        }
        else
        {
            while (TWO_30 > (qdDividend << ubShift))
            {
                ubShift ++;
            }

            ubDividendBitNum = 31 - ubShift;
        }
    }

    ubShift = 0;

    if (qdDivisor < 32768)
    {
        if (qdDivisor < 128)
        {
            while (128 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 8 - ubShift;
        }
        else
        {
            while (32768 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 16 - ubShift;
        }
    }
    else
    {
        if (qdDivisor < 0x00800000)
        {
            while (0x00800000 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 24 - ubShift;
        }
        else
        {
            while (TWO_30 > (qdDivisor << ubShift))
            {
                ubShift ++;
            }

            ubDivisorBitNum = 31 - ubShift;
        }
    }

    bDiffBitNum = ubDividendBitNum - ubDivisorBitNum;

    if (bDiffBitNum == 0)
    {
        qdTempRemain = qdDividend - qdDivisor;
        qdQuotient = 1;
    }
    else
    {
        qdTemp1 = ((QDWORD)qdDivisor) << bDiffBitNum;
        qdTempRemain = qdDividend;
        qdQuotient = 0;

        for (i = 0; i < bDiffBitNum; i++)
        {
            qdTemp2 = qdTempRemain - qdTemp1;

            if (qdTemp2 >= 0)
            {
                qdTempRemain = qdTemp2;
                qdQuotient |= 1;
            }

            qdTempRemain <<= 1;
            qdQuotient <<= 1;
        }

        qdTemp2 = qdTempRemain - qdTemp1;

        if (qdTemp2 >= 0)
        {
            qdTempRemain = qdTemp2;
            qdQuotient |= 1;
        }

        qdTempRemain >>= bDiffBitNum;
    }

    if (ubSign_Dividend == 1)
    {
        *qdRemainder = -qdTempRemain;
    }
    else
    {
        *qdRemainder = qdTempRemain;
    }

    if (ubSign == 1)
    {
        qdQuotient = -qdQuotient;
    }

    return (qdQuotient);
}
#endif