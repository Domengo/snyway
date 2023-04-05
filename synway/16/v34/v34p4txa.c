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

/*************************************************************************/
/* Phase4 Tx vectors, answer modem                                       */
//*************************************************************************/

#include "v34ext.h"

#if 0 /// LLL temp
void V34_Phase4_Send_S(V34Struct *pV34)
{
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    EchoStruc  *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOut_sym;

    ++p4->send_symbol_counter;

    if (p4->send_symbol_counter >= 128)
    {
        (p4->tx_vec_idx) ++;      /* Send S_BAR */
        p4->send_symbol_counter = 0;

        /* init time counter for detect J' */
        if (pV34Rx->modem_mode == ANS_MODEM)
        {
            p4->Time_cnt_Jpi_start = pV34->Time_cnt;
        }

        /************ Turn Circuit 107 ON **********/
        /*        IO_DceDsrEnb(pGlobal); */
        /************ Turn Circuit 107 ON **********/
    }

    cqOut_sym = V34_Create_S_or_SBar(&(p4->S_point));
    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOut_sym, (CQWORD *)&pEc->pcBulk_delay);
}


void V34_Phase4_Send_S_Bar(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc             *pEc = &(pV34->Echo);
    Phase4_Info            *p4 = &(pV34->p4);
    CQWORD cqOut_sym;
    UBYTE i;

    ++p4->send_symbol_counter;

    if (p4->send_symbol_counter >= 16)
    {
        (p4->tx_vec_idx) ++;              /* Send TRN sequence */

        if (pV34Rx->modem_mode == ANS_MODEM)
        {
            p4->Time_cnt_E_start = pV34->Time_cnt;  /* 12:50 PM 8/29/96, store the starting time counter for receiving E */
        }

        if (pRx->Renego.clear_down)
        {
            p4->send_symbol_counter = 0;
        }
        else
        {
            p4->send_symbol_counter = 512;    /* 512 TRN symbol, standard */
        }

        pV34Rx->sc = 0; /* Initial scrambler */
        pShellMap->scram_idx = 16;           /* Prime # scrambled bits to 16 */

        for (i = 0; i < 10; i++)
        {
            p4->pByte_stream[i] = 0xFF;
        }
    }

    cqOut_sym = V34_Create_S_or_SBar(&(p4->S_point_BAR));

    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOut_sym, (CQWORD *)&pEc->pcBulk_delay);
}
#endif