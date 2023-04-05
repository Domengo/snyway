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

/**************************************************************************/
/* Retrieve data bits of one mapping frame, given 8 u(n) symbols from     */
/* Precoder-decoder.                                                      */
/*                                                                        */
/* Notes: Input symbols all located in pV34->rx_sym, indexed by           */
/*        rx_sym_outidx.                                                  */
/*        Auxiliary channel is not implemented yet.                       */
/*                                                                        */
/* Function:                                                              */
/*  void  V34_Retrieve_Infobits(V34Struct *pV34, SWORD High_Map_Frame)    */
/*                                                                        */
/* INPUT:                                                                 */
/*      V34Struct *pV34 - V.34 INFO structure                             */
/*      SWORD High_Map_Frame - Switch pattern bit indicate high/low       */
/*      mapping frame                                                     */
/* OUTPUT:                                                                */
/*      All decoded data bit will placed in pV34->dscram_buf              */
/**************************************************************************/

void  V34_Retrieve_Infobits(ReceiveStruc *pRx, SWORD High_Map_Frame)
{
    DeCodeStruc *DeCode = &(pRx->DeCode);
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    UBYTE QQ0[4][4][5], I1[4], I2[4], I3[4], m[4][2];
    SWORD Z, Z2, Q_m, Q, Q_mask, count;
    UWORD i, j;
    UBYTE *bufptr;
    UDWORD R0;
    CQWORD cU;

    if (RxMod->rx_b > 12)
    {
        count = RxMod->rx_q;
        Q_mask = ((SWORD)1 << count) - 1;  /* Mask to find Q(n) */

        /// LLL debug
        //printf("cU: ");
        for (j = 0; j < 4; j++)
        {
            /* Find Q(2m), and Z(m) */
            cU.r = pRx->pcRx_sym[pRx->rx_sym_outidx  ].r;
            cU.i = pRx->pcRx_sym[pRx->rx_sym_outidx++].i;
            /// LLL debug
            //printf("%d+j%d ", cU.r, cU.i);

            pRx->rx_sym_outidx &= V34_SYM_BUF_MASK;
            V34_Demapper(cU, &Q_m, &Z);
            Q = Q_m & Q_mask;
            m[j][0] = ((Q_m - Q) >> count) & 0xFF;

            if (m[j][0] >= RxMod->rx_M)
            {
                m[j][0] = RxMod->rx_M - 1;
            }

            for (i = 0; i < count; i++)
            {
                QQ0[j][0][i] = (Q >> i) & 0x1;
            }

            /* Find Q(2m+1), and Z(m)+2*I1+U0 */
            cU.r = pRx->pcRx_sym[pRx->rx_sym_outidx  ].r;
            cU.i = pRx->pcRx_sym[pRx->rx_sym_outidx++].i;
            /// LLL debug
            //printf("%d+j%d ", cU.r, cU.i);

            pRx->rx_sym_outidx &= V34_SYM_BUF_MASK;

            V34_Demapper(cU, &Q_m, &Z2);
            Q = Q_m & Q_mask;
            m[j][1] = ((Q_m - Q) >> count) & 0xFF;

            if (m[j][1] >= RxMod->rx_M)
            {
                m[j][1] = RxMod->rx_M - 1;
            }

            for (i = 0; i < count; i++)
            {
                QQ0[j][1][i] = (Q >> i) & 0x1;
            }

            /* Find I1 */
            i = (Z2 - Z) & 0x3;
            I1[j] = (i >> 1) & 0xFF; /* Second bit is I1, first bit is U0 */

            /* Differential decoder, find I2 and I3 */
            V34_DIFF_DECODE(DeCode, Z, i);
            I2[j] = i & 0x1;    /* First bit */
            I3[j] = (i >> 1) & 0xFF;     /* Second bit */
        }

        /* Done 1 4-D symbol, loop for 4 times for 1 mapping frame */
        /// LLL debug
        //printf("\n");

        R0 = V34_De_ShellMapper(m, (UWORD)RxMod->rx_M, &pRx->RxShellMap);
        bufptr = pRx->pDscram_buf;
        count = High_Map_Frame ? RxMod->rx_K : RxMod->rx_K - 1;

        /* Retrieve the S bits */
        for (i = 0; i < count; i++)
        {
            *bufptr++ = (UBYTE)(R0 & 0x1);
            R0 >>= 1;
        }

        count = RxMod->rx_q;

        for (j = 0; j < 4; j++)
        {
            *bufptr++ = I1[j];
            *bufptr++ = I2[j];
            *bufptr++ = I3[j];

            for (i = 0; i < count; i++)
            {
                *bufptr++ = QQ0[j][0][i];
            }

            for (i = 0; i < count; i++)
            {
                *bufptr++ = QQ0[j][1][i];
            }
        }
    }
    else
    {
        for (j = 0; j < 4; j++)
        {
            /* Find Q(2m), and Z(m) */
            cU.r = pRx->pcRx_sym[pRx->rx_sym_outidx  ].r;
            cU.i = pRx->pcRx_sym[pRx->rx_sym_outidx++].i;

            pRx->rx_sym_outidx &= V34_SYM_BUF_MASK;
            V34_Demapper(cU, &Q_m, &Z);

            /* Find Q(2m+1), and Z(m)+2*I1+U0 */
            cU.r = pRx->pcRx_sym[pRx->rx_sym_outidx  ].r;
            cU.i = pRx->pcRx_sym[pRx->rx_sym_outidx++].i;

            pRx->rx_sym_outidx &= V34_SYM_BUF_MASK;
            V34_Demapper(cU, &Q_m, &Z2);

            /* Find I1 */
            i = (Z2 - Z) & 0x3;
            I1[j] = (i >> 1) & 0xFF; /* Second bit is I1, first bit is U0 */

            /* Differential decoder, find I2 and I3 */
            V34_DIFF_DECODE(DeCode, Z, i);
            I2[j] = i & 0x1;    /* First bit */
            I3[j] = (i >> 1) & 0xFF;     /* Second bit */
        }

        /* Done 1 4-D symbol, loop for 4 times for 1 mapping frame */

        count = High_Map_Frame ? RxMod->rx_b : RxMod->rx_b - 1;
        bufptr = pRx->pDscram_buf;

        if (count == 8)
        {
            for (j = 0; j < 4; j++)
            {
                *bufptr++ = I1[j];
                *bufptr++ = I2[j];
            }
        }
        else if (count == 9)
        {
            *bufptr++ = I1[0];
            *bufptr++ = I2[0];
            *bufptr++ = I3[0];

            for (j = 1; j < 4; j++)
            {
                *bufptr++ = I1[j];
                *bufptr++ = I2[j];
            }
        }
        else if (count == 11)
        {
            for (j = 0; j < 3; j++)
            {
                *bufptr++ = I1[j];
                *bufptr++ = I2[j];
                *bufptr++ = I3[j];
            }

            *bufptr++ = I1[3];
            *bufptr++ = I2[3];
        }
        else if (count == 12)
        {
            for (j = 0; j < 4; j++)
            {
                *bufptr++ = I1[j];
                *bufptr++ = I2[j];
                *bufptr++ = I3[j];
            }
        }
    }

    pRx->rx_sym_count -= 8;

    /* Check to see if finish one data frame */
    if (++(pRx->current_rx_P) >= RxMod->rx_P)
    {
        pRx->current_rx_P = 0;
        pRx->current_rx_SWP_bit = RxMod->rx_SWP;
        /* Check to see if finish one superframe */
        /* Maybe have to synchronize the superframe????? */

        if (++(pRx->current_rx_J) >= RxMod->rx_Js)
        {
            pRx->current_rx_J = 0;
        }
    }
}
