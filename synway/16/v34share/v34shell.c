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

#include <string.h>
#include "shareext.h"

void  V34_Shell_Mapper_Init(ShellMapStruct *pShellMap)
{
    memset(pShellMap->pG2, 0, sizeof(pShellMap->pG2));
    memset(pShellMap->pG4, 0, sizeof(pShellMap->pG4));
    memset(pShellMap->pG8, 0, sizeof(pShellMap->pG8));
    memset(pShellMap->pZ8, 0, sizeof(pShellMap->pZ8));
}

#if !USE_ASM
void  V34_Calc_g2(UBYTE m, UBYTE *pG2)
{
    UBYTE *pStart, *pEnd;
    UBYTE m_1;
    UBYTE i, p;

    m_1 = (m - 1) & 0xFF;

    pStart = &(pG2[0]);
    pEnd = &(pG2[m_1 << 1]);

    p = 1;

    for (i = 0; i <= m_1; i++)
    {
        *pStart++ = p;
        *pEnd--   = p;
        p++;
    }
}

void  V34_Calc_g4(UBYTE m, UBYTE *pG2, UWORD *pG4)
{
    UWORD *pStart, *pEnd;
    UBYTE *p0, *pN, *pMid;
    UBYTE i, j, m_1, p1;
    UWORD temp;

    m_1 = (m - 1) & 0xFF;

    pStart = &(pG4[0]);
    pEnd   = &(pG4[m_1<<2]);

    for (i = 0; i <= (m_1 << 1); i++)
    {
        temp = 0;
        p0   = &(pG2[0]);
        pN   = &(pG2[i]);
        p1   = (i + 1) >> 1;
        pMid = &(pG2[p1]);

        for (j = 0; j < p1; j++)
        {
            temp += UBUBMULU(*p0++, *pN--);
        }

        temp <<= 1;

        if ((i & 1) == 0)
        {
            temp += UBUBMULU(*pMid, *pMid);
        }

        *pStart++ = temp;
        *pEnd--   = temp;
    }
}

void  V34_Calc_g8(UBYTE m, UWORD *pG4, UDWORD *pG8)
{
    UDWORD *pStart, *pEnd;
    UWORD *p0, *pN, *pMid;
    UBYTE i, j, m_1, p1;
    UDWORD temp;

    m_1 = (m - 1) & 0xFF;

    pStart = &(pG8[0]);
    pEnd   = &(pG8[m_1 << 3]);

    for (i = 0; i <= (m_1 << 2); i++)
    {
        temp = 0;
        p0 = &(pG4[0]);
        pN = &(pG4[i]);

        p1 = (i + 1) >> 1;

        pMid = &(pG4[p1]);

        for (j = 0; j < p1; j++)
        {
            temp += UUMULUD(*p0++, *pN--);
        }

        temp <<= 1;

        if ((i & 1) == 0)
        {
            temp += UUMULUD(*pMid, *pMid);
        }

        *pStart++ = temp;
        *pEnd--   = temp;
    }
}

void  V34_Calc_z8(UBYTE m, UDWORD *pG8, UDWORD *pZ8)
{
    UDWORD *pStart, udTemp;
    UDWORD *p0;
    UBYTE  i, m1;

    pStart = &(pZ8[1]);
    p0     = &(pG8[0]);

    udTemp = 0;

    if (m == 18)/* Max p is 64 */
    {
        for (i = 0; i < 64; i++)
        {
            udTemp += (*p0++);
            *pStart++ = udTemp;
        }

        for (i = 0; i < 72; i++)
        {
            *pStart++ = udTemp;
        }
    }
    else if (m == 17)/* Max p is 68 */
    {
        for (i = 0; i < 68; i++)
        {
            udTemp += (*p0++);
            *pStart++ = udTemp;
        }

        for (i = 0; i < 60; i++)
        {
            *pStart++ = udTemp;
        }
    }
    else
    {
        m1 = (m - 1) << 3;

        for (i = 0; i < m1; i++)
        {
            udTemp += (*p0++);
            *pStart++ = udTemp;
        }
    }
}


void V34_Shell_Mapper(ShellMapStruct *pShellMap, SWORD SWP_bit, UBYTE mi[4][2])
{
    UDWORD R0, R1, Rm, Rx = 0;
    UWORD  R2, R3, R4, R5, Rn, Ry = 0;
    UBYTE  count;
    SBYTE  A, B, C, D, E, F, G, H, M;
    UBYTE  i, *Si;
    UDWORD *p;
    UBYTE  *p2, *pend2;
    UWORD  *p4, *pend4;

    M = pShellMap->M;

    R0 = 0;
    count = SWP_bit ? pShellMap->K : pShellMap->K - 1;
    Si = &pShellMap->pScram_buf[pShellMap->scram_idx + count -1];

    for (i = 0; i < count; i++)
    {
        /* R0 |= (UDWORD)(*Si) << count; */
        R0 = (R0 << 1) | (*Si--);
    }

    /* Find largest A for which z8(A) <= R0 */
    count = (M - 1) << 3;
    p = &pShellMap->pZ8[0]; /* 0<=p<=8*(m-1) */

    while (R0 < p[count])
    {
        --count;
    }

    A = count;

    /*    Determine the largest integer B such that R1 >= 0    */
    R1 = R0 - pShellMap->pZ8[A]; /* if B = 0 */
    p4 = pShellMap->pG4;
    pend4 = &(pShellMap->pG4[A]);

    Rm = 0;
    B = -1;

    while (R1 >= Rm)
    {
        Rx = UUMULUD(*p4++, *pend4--);

        Rm += Rx;

        B++;
    }

    Rm -= Rx;
    R1 -= Rm;

    /*    Determine the integers    */
    R3 = (UWORD)(R1 / pShellMap->pG4[B]);
    R2 = (UWORD)(R1 - UUMULUD(R3, pShellMap->pG4[B]));         /* while 0 <= R2 <= g4(B)-1) */

    /*    Determine the largest integer C such that R4 >= 0   */
    R4 = R2;
    p2 = pShellMap->pG2;
    pend2 = &(pShellMap->pG2[B]);

    Rn = 0;
    C  = -1;

    while (R4 >= Rn)
    {
        Ry = UBUBMULU(*p2++, *pend2--);

        Rn += Ry;

        C++;
    }

    Rn -= Ry;
    R4 = R2 - Rn;

    /*    Determine the largest integer D such that R5 >= 0   */
    R5 = R3;
    p2 = pShellMap->pG2;
    pend2 = &(pShellMap->pG2[A-B]);

    Rn = 0;
    D  = -1;

    while (R5 >= Rn)
    {
        Ry = UBUBMULU(*p2++, *pend2--);

        Rn += Ry;

        D++;
    }

    Rn -= Ry;
    R5 = R3 - Rn;

    /* Determine the integers */
    F = (SBYTE)(R4 / pShellMap->pG2[C]);
    E = (SBYTE)(R4 - UBUBMULU(F, pShellMap->pG2[C]));/*  where  0 <= E <= g2(C)-1) */
    H = (SBYTE)(R5 / pShellMap->pG2[D]);
    G = (SBYTE)(R5 - UBUBMULU(H, pShellMap->pG2[D]));/*  where  0 <= G <= g2(D)-1) */

    /* the ring indices are determined from the integers A, B, C, D, E, F, G, H  */
    if (C < M)
    {
        mi[0][0] = E;
        mi[0][1] = C - mi[0][0];
    }
    else
    {
        mi[0][1] = M - 1 - E;
        mi[0][0] = C - mi[0][1];
    }

    if ((B - C) < M)
    {
        mi[1][0] = F;
        mi[1][1] = B - C - mi[1][0];
    }
    else
    {
        mi[1][1] = M - 1 - F;
        mi[1][0] = B - C - mi[1][1];
    }

    if (D < M)
    {
        mi[2][0] = G;
        mi[2][1] = D - mi[2][0];
    }
    else
    {
        mi[2][1] = M - 1 - G;
        mi[2][0] = D - mi[2][1];
    }

    if ((A - B - D) < M)
    {
        mi[3][0] = H;
        mi[3][1] = A - B - D - mi[3][0];
    }
    else
    {
        mi[3][1] = M - 1 - H;
        mi[3][0] = A - B - D - mi[3][1];
    }
}
#endif
