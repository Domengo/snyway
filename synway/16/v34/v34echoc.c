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

#if 0 /// LLL temp
void  V34_Phase3_Ec_Dummy(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    UBYTE i;

    for (i = 0; i < EC_INTER_NUM; i++)
    {
        pRx->qEchoCancel_Out[i] = pV34Tx->PCMinPtr[i];  /* No GAIN apply!!! */
    }
}

void  V34_Echo_Init(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
#if 0
    V34StateStruc *pV34State = &pRx->V34State;
#endif
    EchoStruc *ec = &(pV34->Echo);
    QWORD   qTemp, qRTD_Sym;
    SWORD   Sys_Delay_Symbol;
    UWORD   i;

    for (i = 0; i < V34_ECHO_COEF_LEN; i++)
    {
        ec->pcNe_coef[i].r = 0;
        ec->pcNe_coef[i].i = 0;

        ec->pcFe_coef[i].r = 0;
        ec->pcFe_coef[i].i = 0;
    }

    for (i = 0; i < V34_MAX_BULK_LEN; i++)
    {
        ec->pcBulk_delay[i].r = 0;
        ec->pcBulk_delay[i].i = 0;
    }

    for (i = 0; i < EC_INTER_NUM; i++)
    {
        pRx->qEchoCancel_Out[i] = 0;
    }

    Sys_Delay_Symbol = V34_tSys_Delay[pV34Rx->tx_symbol_rate];

    ec->Ec_Insert.bulk_inidx = V34_MAX_BULK_LEN - 1;

    ec->ne_bulk_outidx = ec->Ec_Insert.bulk_inidx - Sys_Delay_Symbol;

    ec->ec_adapt = 0;

    if (pV34Rx->RTD_PH2_symbol > 0)
    {
        qTemp = pV34Rx->RTD_PH2_symbol - 1;

        qRTD_Sym = V34_tRTD_Sym[pV34Rx->tx_symbol_rate];

        qTemp = QQMULQR8(qTemp, qRTD_Sym);
    }
    else
    {
        qTemp = 0;
    }

    /* move main tap of far echo canceller by 2 symbols left */
    pV34->fecount = qTemp + 1;

    if ((pV34->fecount + V34_EC_SUBLEN + 1 + V34_EC_SUBLEN) >= ec->ne_bulk_outidx)
    {
#if 0
        pV34State->hang_up = 1;
        pV34State->hangup_reason = V34_ECHO_BUF_SMALL;
#endif
        TRACE1("V34_ECHO_BUF_SMALL: pV34->fecount=%d", pV34->fecount);
        ec->fe_bulk_outidx = V34_EC_SUBLEN + 1 + V34_EC_SUBLEN;
    }
    else
    {
        if (pV34->fecount > V34_EC_SUBLEN)
        {
            ec->fe_bulk_outidx = ec->ne_bulk_outidx - pV34->fecount;
        }
        else
        {
            ec->fe_bulk_outidx = ec->ne_bulk_outidx - V34_EC_SUBLEN;
        }

#if SUPPORT_V90D
        ec->fe_bulk_outidx += 12;
#endif
    }
}

void  V34_Echo_Canceller(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    EchoStruc *ec = &(pV34->Echo);
    CQDWORD *necoef;
    CQDWORD *fecoef;
    CQWORD *nedline_ptr;
    CQWORD *fedline_ptr;

    QWORD ne_out[EC_INTER_NUM];
    QWORD fe_out[EC_INTER_NUM];
    QWORD errbeta_ne[EC_INTER_NUM];
    QWORD errbeta_fe[EC_INTER_NUM];
    UBYTE i;

    necoef = &(ec->pcNe_coef[0]);/* Near-End coef pointer Setup */

    ec->ne_bulk_outidx ++;/* Near-End bulk delayline pointer */

    if (ec->ne_bulk_outidx >= V34_MAX_BULK_LEN)
    {
        ec->ne_bulk_outidx = V34_EC_SUBLEN;
    }

    nedline_ptr = &(ec->pcBulk_delay[ec->ne_bulk_outidx]);

    /* FIR */
    DspI3cFir_EC_16_32(necoef, V34_EC_SUBLEN, nedline_ptr, ne_out);

    fecoef = &(ec->pcFe_coef[0]);/* Far-End coef pointer Setup */

    ec->fe_bulk_outidx ++;/* Far-End bulk delayline pointer */

    if (ec->fe_bulk_outidx >= V34_MAX_BULK_LEN)
    {
        ec->fe_bulk_outidx = V34_EC_SUBLEN;
    }

    fedline_ptr = &(ec->pcBulk_delay[ec->fe_bulk_outidx]);

    DspI3cFir_EC_16_32(fecoef, V34_EC_SUBLEN, fedline_ptr, fe_out);

    /*********** Residual error ***********/
    for (i = 0; i < EC_INTER_NUM; i++)
    {
        pRx->qEchoCancel_Out[i] = pV34Tx->PCMinPtr[i] - (ne_out[i] + fe_out[i]);
    }

    /* Adapt */
    if (ec->ec_adapt)
    {
        nedline_ptr = &(ec->pcBulk_delay[ec->ne_bulk_outidx]);
        necoef = &(ec->pcNe_coef[0]);

        for (i = 0; i < EC_INTER_NUM; i++)
        {
            errbeta_ne[i] = QQMULQR15(ec->qNe_beta, pRx->qEchoCancel_Out[i]);

#if 0 /* for to dump Echo Cancel Error. */

            if (DumpIdx1 < 400000) { DumpData1[DumpIdx1++] = pRx->qEchoCancel_Out[i]; }

#endif
        }

        /*********** Near-End adaptation *********/
        DspI3cFir_EC_16_32_Update(necoef, V34_EC_SUBLEN, nedline_ptr, errbeta_ne);

        fedline_ptr = &(ec->pcBulk_delay[ec->fe_bulk_outidx]);
        fecoef = &(ec->pcFe_coef[0]);

        for (i = 0; i < EC_INTER_NUM; i++)
        {
            errbeta_fe[i] = QQMULQR15(ec->qFe_beta, pRx->qEchoCancel_Out[i]);
        }

        /*********** Far-End  adaptation *********/
        DspI3cFir_EC_16_32_Update(fecoef, V34_EC_SUBLEN, fedline_ptr, errbeta_fe);
    }
}
#endif
