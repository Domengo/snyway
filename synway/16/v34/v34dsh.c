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

/*************************************************************/
/*  Reverse Shell Mapper to find R0                          */
/*  INPUT:                                                   */
/*  UBYTE m[4][2] - the ring index calculated from De-Mapper */
/*  SWORD rx_M    - the shaping value                        */
/*************************************************************/

#include "v34ext.h"

#if !USE_ASM

/* To find R0, we need A; to find A, all we have to find is B and D */
UDWORD  V34_De_ShellMapper(UBYTE pM[4][2], UWORD rx_M, RX_ShellMapStruct *RxShellMap)
{
    UWORD A, B, C, D, E, F, G, H, sum, M_1;
    UDWORD R0, R1, Rx;
    UWORD R2, R3, R4, R5, Ry;
    UBYTE *pP2, *pEnd2, *pStop2;
    UWORD *pP4, *pEnd4, *pStop4;

    M_1 = rx_M - 1;

    C = pM[0][0] + pM[0][1];
    E = C < rx_M ? pM[0][0] : M_1 - pM[0][1];

    sum = pM[1][1] + pM[1][0];
    B = sum + C;
    F = sum < rx_M ? pM[1][0] : M_1 - pM[1][1];

    D = pM[2][1] + pM[2][0];
    G = D < rx_M ? pM[2][0] : M_1 - pM[2][1];

    sum = pM[3][1] + pM[3][0];
    A = sum + B + D;
    H = sum < rx_M ? pM[3][0] : M_1 - pM[3][1];

    R5 = H * RxShellMap->pRx_g2[D];
    R5 += G;
    R4 = F * RxShellMap->pRx_g2[C];
    R4 += E;

    /* Find R3 */
    pP2    = RxShellMap->pRx_g2;
    pEnd2  = &RxShellMap->pRx_g2[A - B];
    pStop2 = &RxShellMap->pRx_g2[D];
    Ry = 0;

    while (pP2 < pStop2)
    {
        Ry += (UWORD)(*pP2) * (*pEnd2);

        pP2++;

        pEnd2--;
    }

    R3 = R5 + Ry;

    /* Find R2 */
    pP2 = RxShellMap->pRx_g2;
    pEnd2 = &RxShellMap->pRx_g2[B];
    pStop2 = &RxShellMap->pRx_g2[C];
    Ry = 0;

    while (pP2 < pStop2)
    {
        Ry += (UWORD)(*pP2) * (*pEnd2);

        pP2++;

        pEnd2--;
    }

    R2 = R4 + Ry;

    R1 = UUMULUD(R3, RxShellMap->pRx_g4[B]) + (UDWORD)R2;

    /* Find R0 */
    pP4 = RxShellMap->pRx_g4;
    pEnd4 = &RxShellMap->pRx_g4[A];
    pStop4 = &RxShellMap->pRx_g4[B];
    Rx = 0;

    while (pP4 < pStop4)
    {
        Rx += (UDWORD)(*pP4) * (*pEnd4);

        pP4++;

        pEnd4--;
    }

    R0 = R1 + RxShellMap->pRx_z8[A] + Rx;

    return(R0);
}
#endif
