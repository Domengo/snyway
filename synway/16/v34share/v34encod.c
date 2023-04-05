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
/* V.34 Main encoder - Including Parser, Shell Mapper, Differential        */
/*                     Encoder, Mapper, Precoder, Trellis Encoder,         */
/*                     and Non-linear encoder.                             */
/*                                                                         */
/* Notes: The following code's order should not be change, specially the   */
/*        Mapper, Precoder and Trellis encoder (Section 9.6 in ITU V.34)   */
/*        have a very specific sequence of steps for implementation.       */
/*                                                                         */
/* Last Updated:                                                           */
/*      Apr 15, 1996.                                                      */
/*      May 07, 1996. Benjamin Chan, pQQ0[4][4][5] => pQQ0[4][4][4]        */
/*                    Put calculation of cU(2m),cU(2m+1) into switch       */
/*                    statement, eliminate cc1 and ss1.                    */
/*                    Modify output from precoder pcX[2], 9.7 format to    */
/*                    7.9 format to use SC_S16toDSP macro                  */
/*      Apr 30, 1996. Jack Liu Porting to VXD lib                          */
/*      May 15, 1996. Modify porting.                                      */
/* Author:                                                                 */
/*      Sherry, GAO Research & Consulting Ltd. (first version)             */
/*      Benjamin Chan, GAO Research & Consulting Ltd.                      */
/***************************************************************************/

#include "shareext.h"

SBYTE V34_Trellis16(UWORD *Q, UBYTE y)
{
    SBYTE y0;

    y0 = *Q & 0x1;

    *Q >>= 1;

    *Q ^= (y0 == 1 ? 0xc : 0x0);

    *Q ^= ((y & 0x3) ^((y << 1) & 0x4));

    y0 = *Q & 0x1;

    return (y0);
}

SBYTE V34_Trellis32(UWORD *Q, UBYTE y)
{
    SBYTE y0;

    y0 = *Q & 0x1;

    *Q >>= 1;

    *Q ^= (y0 << 4);

    y = (y & 0x3) ^((y & 0x08) >> 1);

    *Q ^= ((y << 2) & 0x0F) ^(y >> 1);

    y0 = *Q & 0x1;

    return (y0);
}

SBYTE V34_Trellis64(UWORD *Q, UBYTE y)
{
    UWORD Q2, Q3, Q4;
    SBYTE y0;

    y0 = *Q & 0x1;

    *Q >>= 1;

    Q4 = (*Q & 0x8) << 2;

    Q3 = *Q & 0x4;

    Q2 = *Q & 0x2;

    *Q ^= ((*Q << 1) & 0x38) ^((y << 2) & 0x30);

    *Q ^= (Q3 >> 2) ^ Q2 ^(Q2 << 3) ^((y << 3) & 0x08) ^(y0 << 1);

    *Q ^= (((y >> 1) & 0x1) == 1 ? (1 + (Q3 << 2)) : 0x0);

    *Q ^= Q4 ^((Q3 << 3) & (Q4 ^((y & 0x1) << 5)));

    y0 = *Q & 0x1;

    return (y0);
}

/*-----------------------------------------------*/
/* Symbol scaling for average tx energy matained */
/* Put Tx symbols into Tx symbol buffer          */
/*-----------------------------------------------*/
void  V34_SymScale(V34TxStruct *pV34Tx, CQWORD pcX)
{
    CQWORD cqTx_sym;

    cqTx_sym.r = QDQMULQDR(pV34Tx->qdTx_scale, pcX.r);
    cqTx_sym.i = QDQMULQDR(pV34Tx->qdTx_scale, pcX.i);

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cqTx_sym.r; }
    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = cqTx_sym.i; }
#endif

    if (pV34Tx->nlinear)
    {
        V34_Non_Linear_Encoder(pV34Tx->qTx_nl_scale, &cqTx_sym);
    }

    pV34Tx->pcTx_sym[pV34Tx->tx_sym_inidx++] = cqTx_sym;

    pV34Tx->tx_sym_inidx &= V34_SYM_BUF_MASK;
}

/****************************************************************************
*  V.34 encoder                                                             *
*                                                                           *
*  INPUT: High_Map_Frame, Switching pattern bit; 1-high frame, 0-low frames *
*         AMP_bit, Auxiliary pattern bit; 1-use aux data, 0-use org. data   *
*         pV34Tx, points to V34Tx structure                                 *
****************************************************************************/

void  V34_Encoder(SWORD High_Map_Frame, SWORD AMP_bit, V34TxStruct *pV34Tx)
{
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
#if TX_PRECODE_ENABLE
    PrecodeStruct *pTx_precode = &(pV34Tx->tx_precode);
    CQWORD         cC2m;
#endif
    UBYTE          *pBufptr;
    SBYTE          pII1[4], pII2[4], pII3[4];
    UBYTE          pQQ[4][2];
    UBYTE          mi[4][2];
    UBYTE          Ybits;
    UWORD          Q;
    CQWORD         cU = {0, 0}, cY, cX2m;
    CQWORD         pcX[2];
    SBYTE          count, q, Z, m;
    SBYTE          q1, v0, s1, s2, C0 = 0, U0;
    UBYTE          i, j;
    SBYTE          x, y;
    SBYTE          b;

    /*-----------------------------------------------------------*/
    /*                           Parser                          */
    /*-----------------------------------------------------------*/
    b = pV34Tx->b;

    /* determine first K scrambled data bit in mapping frame   */
    if (b > 12)
    {
        pBufptr = High_Map_Frame ? (&pShellMap->pScram_buf[pShellMap->K + pShellMap->scram_idx]) :
                  (&pShellMap->pScram_buf[pShellMap->K - 1 + pShellMap->scram_idx]);

        q = pV34Tx->q;

        for (j = 0; j < 4; j++)
        {
            /* This could be AUX data if AMP_bit is 1 */
            pII1[j] = *pBufptr++; // I1i,j
            pII2[j] = *pBufptr++; // I2i,j
            pII3[j] = *pBufptr++; // I3i,j

            pQQ[j][0] = 0;
            pQQ[j][1] = 0;

            if (q)
            {
                for (i = 0; i < q; i++)
                {
                    pQQ[j][0] += (*pBufptr++ << i);    // Qi,j,0,q
                }

                for (i = 0; i < q; i++)
                {
                    pQQ[j][1] += (*pBufptr++ << i);    // Qi,j,1,q
                }
            }
        }
    } /* End if b > 12 */
    else
    {
        count = High_Map_Frame ? b : b - 1;
        pBufptr = &pShellMap->pScram_buf[pShellMap->scram_idx];

        switch (count)
        {
            case 8:

                for (j = 0; j < 4; j++)
                {
                    pII1[j] = *pBufptr++;
                    pII2[j] = *pBufptr++;
                    pII3[j] = 0;
                }

                break;

            case 9:
                pII1[0] = *pBufptr++;
                pII2[0] = *pBufptr++;
                pII3[0] = *pBufptr++;

                for (j = 1; j < 4; j++)
                {
                    pII1[j] = *pBufptr++;
                    pII2[j] = *pBufptr++;
                    pII3[j] = 0;
                }

                break;

            case 11:

                for (j = 0; j < 3; j++)
                {
                    pII1[j] = *pBufptr++;
                    pII2[j] = *pBufptr++;
                    pII3[j] = *pBufptr++;
                }

                pII1[j] = *pBufptr++;
                pII2[j] = *pBufptr++;
                pII3[3] = 0;
                break;

            case 12:

                for (j = 0; j < 4; j++)
                {
                    pII1[j] = *pBufptr++;
                    pII2[j] = *pBufptr++;
                    pII3[j] = *pBufptr++;
                }

                break;
        }
    }

    /* when the auxiliary channel is present, the bit pII1[ii][0] in the
    * first group is either a primary channel bit or an auxiliary
    * channel bit depending upon AMP */

    /*-----------------------------------------------------------*/
    /*                       Shell Mapper                        */
    /*   In every mapping frame, using shell mapper maps K input */
    /*   bit to produce 8 output ring indices m[ii][j][k], j= 0  */
    /*   to 3, k = 0,1.                                          */
    /*-----------------------------------------------------------*/
    if (b > 12)
    {
        V34_Shell_Mapper(pShellMap, High_Map_Frame, mi);
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            mi[i][0] = 0;
            mi[i][1] = 0;

            pQQ[i][0] = 0;
            pQQ[i][1] = 0;
        }
    }

    pShellMap->scram_idx = (UBYTE)(High_Map_Frame ? pShellMap->scram_idx + b : pShellMap->scram_idx + b - 1);

    /*-----------------------------------------------------------*/
    /*   Sequence of Operation for Mapper, Precoder and Trellis  */
    /*   Encoder. The sequence table is listed in ITU V.34.      */
    /*   The sequence specified in ITU V.34 is for one 4-D symbol*/
    /*   and the following code will repeat for 4 4-D symbols,   */
    /*   which is one mapping frame.                             */
    /*-----------------------------------------------------------*/
    for (j = 0; j < 4; j++)
    {
        /* ---------------------------------------------- */
        /*                  Mapper                      */
        /*  For finding cU(2m)                          */
        /* ---------------------------------------------- */
        q = pV34Tx->q;

        Q = pQQ[j][0] + ((UWORD)mi[j][0] << q); // Q[n] = Qi,j,k,1...q + mi,j,k (k = 0)

        /* ---------------------------------------------- */
        /*           Differential Encoder               */
        /* ---------------------------------------------- */
        pV34Tx->z = (pV34Tx->z + (pII2[j] + (pII3[j] << 1))) & 0x3;
        Z = pV34Tx->z; // 2 bits

        /* Cos(Z*PI/2), Sin(Z*PI/2) ***Clockwise****  */
        /* ------------------ cU(2m) ---------------- */
        x = (SBYTE)(V34_tRI[Q] >> 8); // the real part of the quater superconstellation of v(2m)
        y = (SBYTE)((V34_tRI[Q] << 8) >> 8);/* take out the SIGN and value in lower 8 bits */ // the imaginary part of v(2m)

        switch (Z)
        {
            case 0: cU.r =  x; cU.i =  y; break;/*Cos(0),    Sin(0)    */
            case 1: cU.r =  y; cU.i = -x; break;/*Cos(PI/2), Sin(PI/2) */
            case 2: cU.r = -x; cU.i = -y; break;/*Cos(PI),   Sin(PI)   */
            case 3: cU.r = -y; cU.i =  x; break;/*Cos(3PI/2),Sin(3PI/2)*/
        } // u(2m)
        /*----------------------------------------------*/

        /*------------- Step 2, 9.6.2 item 4 -----------*/
        /* All in 16.0 format */
        cY = cU;

#if TX_PRECODE_ENABLE
        cY.r += pTx_precode->cC.r;
        cY.i += pTx_precode->cC.i;
#endif
        /* All in 9.7 format */
        cX2m.r = cY.r << 7;
        cX2m.i = cY.i << 7;

#if TX_PRECODE_ENABLE
        cX2m.r -= pTx_precode->cP.r;
        cX2m.i -= pTx_precode->cP.i;
#endif

        /* use cY(2m) to find label 1, for conv. encoder */
        s1 = V34_Label(cY); // 9.6.3.1

        pcX[0].r = (cX2m.r << 2);/* 9.7 format to 7.9 format */
        pcX[0].i = (cX2m.i << 2);/* = pcX * (1/128) * (1/64)   */
        /*----------------------------------------------*/

#if TX_PRECODE_ENABLE
        /*------------- Step 3, Precoder ---------------*/
        cC2m = pTx_precode->cC;          /* cC2m = c(2m) */

        V34_Precoder(pTx_precode, &cX2m, b);  /* find c(2m+1), p(2m+1) */
        /*----------------------------------------------*/

        /*------------- Step 4, Modulo Encoder ---------*/
        C0 = (((cC2m.r + cC2m.i) >> 1) & 0x1) ^(((pTx_precode->cC.r + pTx_precode->cC.i) >> 1) & 0x1);

        /* c(2m), c(2m+1) */
        /*----------------------------------------------*/
#endif

        /*------ Step 5, Find V0, then create U0 -------*/
        /* Bit inversion for SuperFrame Synchronization */
        v0 = 0;
        m = (pV34Tx->current_P << 2) + j;

        if ((m % (pV34Tx->P << 1)) == 0)
        {
            v0 = pV34Tx->current_inv_bit & 0x1;
            pV34Tx->current_inv_bit >>= 1;
        }

        U0 = (C0 ^ pV34Tx->Y0) ^ v0;
        /*----------------------------------------------*/

        /*------------ Step 6, find cU(2m+1) ------------*/
        Q = pQQ[j][1] + ((UWORD)mi[j][1] << q);
        x = (SBYTE)(V34_tRI[Q] >> 8);
        y = (SBYTE)((V34_tRI[Q] << 8) >> 8);/*take out the SIGN and value in lower 8 bits*/
        q1 = Z + U0 + (pII1[j] << 1);
        q1 &= 0x3;

        switch (q1)
        {
            case 0: cU.r =  x; cU.i =  y; break; /*Cos(0),    Sin(0) */
            case 1: cU.r =  y; cU.i = -x; break; /*Cos(PI/2), Sin(PI/2)*/
            case 2: cU.r = -x; cU.i = -y; break; /*Cos(PI),   Sin(PI); */
            case 3: cU.r = -y; cU.i =  x; break; /*Cos(3PI/2),Sin(3PI/2)*/
        }
        /*----------------------------------------------*/

        /*------------- Step 7, 9.6.2 item 4 -----------*/
        /* All in 16.0 format */
        {
            cY = cU;

#if TX_PRECODE_ENABLE
            cY.r += pTx_precode->cC.r;   /* cU(2m+1) + c(2m+1) */
            cY.i += pTx_precode->cC.i;
#endif
        }

        /* All in 9.7 format */
        {
            cX2m.r = cY.r << 7;
            cX2m.i = cY.i << 7;

#if TX_PRECODE_ENABLE
            cX2m.r -= pTx_precode->cP.r;  /* cY(2m+1) - p(2m+1) */
            cX2m.i -= pTx_precode->cP.i;
#endif
        }

        /* use cY(2m+1) to find label 2, for conv. encoder */
        s2 = V34_Label(cY);

        pcX[1].r = (cX2m.r << 2);   /* 9.7 format to 7.9 format */
        pcX[1].i = (cX2m.i << 2);   /* = pcX * (1/128) * (1/64)   */
        /*----------------------------------------------*/

#if TX_PRECODE_ENABLE
        /*------------- Step 8, Precoder ---------------*/
        /* find c(2m+2), p(2m+2) for next time */
        V34_Precoder(pTx_precode, &cX2m, b);
        /*----------------------------------------------*/
#endif

        /*------------- Step 9, Trellis encoder --------*/
        Ybits = V34_tSYM2BIT_TBL[s1][s2];

        switch (pV34Tx->conv_state)
        {
            case 0: pV34Tx->Y0 = V34_Trellis16(&pV34Tx->tx_COV_state, Ybits); break;
            case 1: pV34Tx->Y0 = V34_Trellis32(&pV34Tx->tx_COV_state, Ybits); break;
            case 2: pV34Tx->Y0 = V34_Trellis64(&pV34Tx->tx_COV_state, Ybits);
        }

        V34_SymScale(pV34Tx, pcX[0]);
        V34_SymScale(pV34Tx, pcX[1]);
    }

    /* End of 4-D symbol interval, loop for 4 times */

    /* Check to see if finish one data frame */
    if (++(pV34Tx->current_P) >= pV34Tx->P)
    {
        pV34Tx->current_P = 0;
        pV34Tx->current_SWP_bit = pV34Tx->SWP;
        pV34Tx->Aux.current_AMP_bit = pV34Tx->Aux.AMP;

        /* Check to see if finish one superframe */
        /* Maybe have to synchronize the superframe????? */
        if (++(pV34Tx->current_J) >= pV34Tx->Js)
        {
            pV34Tx->current_J = 0;
            pV34Tx->current_inv_bit = pV34Tx->bit_inv_pat;
        }
    }
}
