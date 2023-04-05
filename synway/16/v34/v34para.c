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

void  V34_Para_Tx_Init(V34Struct *pV34, ModemData *pM, UBYTE M)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);

    pV34Tx->b       = pM->b;
    pV34Tx->Aux.Ws  = pM->Ws;
    pV34Tx->Js      = pM->Js;
    pV34Tx->P       = pM->P;
    pShellMap->K    = pM->K;
    pShellMap->M    = M;
    pV34Tx->SWP     = pM->SWP;
    pV34Tx->Aux.AMP = pM->AMP;
    pV34Tx->nbyte   = pM->nbyte;
    pV34Tx->nbits   = pM->nbits;
}

void  V34_Para_Rx_Init(RX_ModemDataStruc *RxMod, ModemData *pM, UBYTE M)
{
    RxMod->rx_b     = pM->b;
    RxMod->rx_Ws    = pM->Ws;
    RxMod->rx_Js    = pM->Js;
    RxMod->rx_P     = pM->P;
    RxMod->rx_K     = pM->K;
    RxMod->rx_M     = M;
    RxMod->rx_SWP   = pM->SWP;
    RxMod->rx_AMP   = pM->AMP;
    RxMod->rx_nbyte = pM->nbyte;
    RxMod->rx_nbits = pM->nbits;
}

void  V34_Carrier_Init(V34Struct *pV34)
{
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    QAMStruct *pQAM = &(pV34Tx->QAM);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
#if FREQUENCY_OFFSET
    QDWORD qdTemp;
    QDWORD qdOffset;
#endif

    /************************* Modulation carrier ***********************/
    pQAM->tx_carrier_freq = V34_tCarrier_tbl[pV34Rx->tx_symbol_rate][pQAM->tx_high_freq];
    pQAM->qTx_carrier_offset = V34_tCarrier_offset[pV34Rx->tx_symbol_rate][pQAM->tx_high_freq];

    /************************ Demodulation carrier **********************/
    pRx->rx_carrier_freq = V34_tCarrier_tbl[pV34Rx->rx_symbol_rate][pV34Rx->rx_high_freq];
    pRx->qRx_carrier_offset = V34_tCarrier_offset[pV34Rx->rx_symbol_rate][pV34Rx->rx_high_freq];

#if FREQUENCY_OFFSET
    {
        /* frequencyOffset is Q8 signed value in Hz */
        qdTemp = QQMULQD(pV34Rx->frequencyOffset, V34_tCarrier_offset_factor[pV34Rx->rx_symbol_rate]);
        qdTemp = (qdTemp + 0x0800) >> 12;

        while (qdTemp > 32767)
        {
            qdTemp -= 32768;
            pRx->rx_carrier_freq++;
        }

        while (qdTemp < 0)
        {
            qdTemp += 32768;
            pRx->rx_carrier_freq--;
        }

        qdOffset = qdTemp + pRx->qRx_carrier_offset;

        if (qdOffset > 32767)
        {
            pRx->rx_carrier_freq++;
            qdOffset -= 32768;
        }

        pRx->qRx_carrier_offset = (QWORD)qdOffset;
    }
#endif
}

#if 0 /// LLL temp
void  V34_Parameter_Function_Init_Tx(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);

    V34_Para_Tx_Init(pV34, (ModemData *)&V34_tINIT_PARA_DATA[pV34Rx->tx_symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400],
                     V34_tMAP_M[pV34Rx->tx_symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400][pV34Tx->Min_Exp]);

    /* Determine the Superframe sychronization bit pattern */
    if (pV34Tx->Js == 8)
    {
        pV34Tx->bit_inv_pat = 0x5FEE;
    }
    else
    {
        pV34Tx->bit_inv_pat = 0x1FEE;
    }

    pV34Tx->current_inv_bit = pV34Tx->bit_inv_pat;
    pV34Tx->current_SWP_bit = pV34Tx->SWP;
    pV34Tx->Aux.current_AMP_bit = pV34Tx->Aux.AMP;
    pV34Tx->current_J = 0;
    pV34Tx->current_P = 0;

    if (pV34Tx->b > 12)
    {
        pV34Tx->q = (pV34Tx->b - 12 - pShellMap->K) / 8;
    }
    else
    {
        pV34Tx->q = 0;
    } /* For safety */
}
#endif


void  V34_Parameter_Function_Init_Rx(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    DeCodeStruc *DeCode = &(pRx->DeCode);
    VA_INFO        *dec = &(DeCode->Vdec);    /* VA INFO structure */
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);

    V34_Para_Rx_Init(RxMod,
                     (ModemData *)&(V34_tINIT_PARA_DATA[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate - V34_BIT_2400]),
                     V34_tMAP_M[pV34Rx->rx_symbol_rate][pV34Rx->rx_bit_rate - V34_BIT_2400][pV34Rx->rx_me]);

    /* V34_Agc2_Win_Init(pV34); */

    /* Determine the Superframe sychronization bit pattern */
    if (RxMod->rx_Js == 8)
    {
        dec->rx_bit_inv_pat = 0x5FEE;
    }
    else
    {
        dec->rx_bit_inv_pat = 0x1FEE;
    }

    dec->current_rx_inv_bit = dec->rx_bit_inv_pat;
    dec->current_rx_J = 0;
    dec->current_rx_P = 0;
    dec->D4_sym_idx = 0;

    pRx->current_rx_SWP_bit = RxMod->rx_SWP;
    pV34Rx->current_rx_AMP_bit = RxMod->rx_AMP;

    pRx->current_rx_J = 0;
    pRx->current_rx_P = 0;

    if (RxMod->rx_b > 12)
    {
        RxMod->rx_q = (RxMod->rx_b - 12 - RxMod->rx_K) >> 3;
    }
    else
    {
        RxMod->rx_q = 0;
    }

    /* ----------------------------------------------------------------------
    calculate function   g2[], g4[], g8[], z8[], whose definitions
    from Recommendation V.34 page 18
    ---------------------------------------------------------------------- */
}
