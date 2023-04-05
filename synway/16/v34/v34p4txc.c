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

/****************************************************************************/
/* Phase4 Tx vectors                                                        */
/*                                                                          */
/* Functions:                                                               */
/*      SWORD Phase4_Send_J_Bar(V34Struct *, SWORD *, UBYTE *)              */
/*      SWORD Phase4_Send_TRN(V34Struct *, SWORD *, UBYTE *)                */
/*      SWORD Phase4_Send_TRN_2000MS(V34Struct *, SWORD *, UBYTE *)         */
/*      CQWORD Phase4_Send_MP_Share(V34Struct *, Phase4_Info)               */
/*      SWORD Phase4_Send_MP(V34Struct *, SWORD *, UBYTE *)                 */
/*      SWORD Phase4_Finish_MP(V34Struct *, SWORD *, UBYTE *)               */
/*      SWORD Phase4_Finish_MP_Pi(V34Struct *, SWORD *, UBYTE *)            */
/*      SWORD Phase4_Send_E (V34Struct *, SWORD *, UBYTE *)                 */
/*      SWORD Phase4_Send_B1_Frame (V34Struct *, SWORD *, UBYTE *)          */
/*      SWORD Begin_Data_Transfer(V34Struct *, SWORD *, UBYTE *)            */
/*                                                                          */
/* Lasted Updated:                                                          */
/*      Apr 03, 1996.                                                       */
/*      Apr 09, 1996. Benjamin, modifies call parameters of modulate        */
/*      May 07, 1996. Benjamin, take out SYMBOL_SCALE_DOWN from             */
/*                    tx_scale_const table.                                 */
/*      Jul 11, 1996. Benjamin, change input parameter to V34Struct *       */
/* Author:                                                                  */
/*      Dennis Chan, GAO Research & Consulting Ltd.                         */
/*      Benjamin Chan, GAO Research & Consulting Ltd.                       */
/****************************************************************************/

#include <string.h>
#include "v34ext.h"

void V34_Pre_Data_Share(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    Phase4_Info *p4 = &(pV34->p4);
    SBYTE nbyte, nbits, b;

    nbyte = pV34Tx->nbyte;
    nbits = pV34Tx->nbits;
    b     = pV34Tx->b;

    if ((pV34Tx->current_SWP_bit & 0x1) == 0)
    {
        --b;

        if (--nbits < 0)
        {
            nbits = 7;
            --nbyte;
        }
    }

    (*pV34Rx->pfScram_byte)(p4->pByte_stream, &(pV34Rx->sc), pShellMap->pScram_buf, nbyte);
    (*pV34Rx->pfScram_bit)(p4->pByte_stream[nbyte], &(pV34Rx->sc), &(pShellMap->pScram_buf[b - nbits]), nbits);

    pShellMap->scram_idx = 0;
    V34_Send_Mapping_Frame(pV34);
}

#if 0 /// LLL temp
/*********************************************************/
/* Send J' part, Finish sending J and send only one J'   */
/*********************************************************/
void V34_Phase4_Send_J_Bar(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    UBYTE i;
    CQWORD cqOutsymbol;

    if (pShellMap->scram_idx >= 16)
    {
        p4->pByte_stream[0] = V34_POINT_J_PI & 0xFF;
        p4->pByte_stream[1] = (V34_POINT_J_PI >> 8) & 0xFF;
        (*pV34Rx->pfScram_byte)(p4->pByte_stream, &(pV34Rx->sc), pShellMap->pScram_buf, 2);
        pShellMap->scram_idx = 0;
        p4->send_symbol_counter = 8;

        /************* Turn Circuit 107 ON *************/
        /*        IO_DceDsrEnb(pGlobal); */
        /************* Turn Circuit 107 ON *************/
    }

    cqOutsymbol = V34_Create_J(pV34);
    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    if ((--(p4->send_symbol_counter)) <= 0)
    {
        /* Initialize scrambler */
        pV34Rx->sc = 0;
        pShellMap->scram_idx = 16;
        (p4->tx_vec_idx)++; /* goto send 512 TRN */

        /* time out init for receive E */
        if (pV34Rx->modem_mode == CALL_MODEM)
        {
            p4->Time_cnt_E_start = pV34->Time_cnt;
        }

        (p4->send_symbol_counter) = 512;

        for (i = 0; i < 10; i++)
        {
            p4->pByte_stream[i] = 0xFF;
        }
    }
}

/**************************/
/* Send at least 512 TRN  */
/**************************/
void V34_Phase4_Send_TRN(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOutsymbol;

    if (pShellMap->scram_idx >= 16)
    {
        (*pV34Rx->pfScram_byte)(p4->pByte_stream, &(pV34Rx->sc), pShellMap->pScram_buf, 2);
        pShellMap->scram_idx = 0;
    }

    cqOutsymbol = V34_Create_TRN(pShellMap->pScram_buf, &(pShellMap->scram_idx), pV34Rx->Num_Trn_Tx);

    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);

    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    if (--(p4->send_symbol_counter) <= 0)
    {
        /* goto send TRN up to 2000ms  */
        (p4->tx_vec_idx)++;

        p4->send_symbol_counter = 0x7FFF; /* Assign max. number */
    }
}

/*****************************/
/* Send up to 2000ms of TRN  */
/*****************************/
void V34_Phase4_Send_TRN_2000MS(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOutsymbol;

    if (pShellMap->scram_idx >= 16)
    {
        (*pV34Rx->pfScram_byte)(p4->pByte_stream, &(pV34Rx->sc), pShellMap->pScram_buf, 2);
        pShellMap->scram_idx = 0;
    }

    cqOutsymbol = V34_Create_TRN(pShellMap->pScram_buf, &(pShellMap->scram_idx), pV34Rx->Num_Trn_Tx);

    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);

    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    --(p4->send_symbol_counter);

    if ((p4->send_symbol_counter <= 0) && (pShellMap->scram_idx >= 16))
    {
        /* 4 - Points training, initialize diff. encoder with last TRN symbol */
        /* Reinitialize diff. encoder to send MP */
        if (pV34Rx->Num_Trn_Tx == V34_4_POINT_TRN)
        {
            pV34Tx->z = pShellMap->pScram_buf[pShellMap->scram_idx-2] + (pShellMap->pScram_buf[pShellMap->scram_idx-1] << 1);
        }
        else
        {
            pV34Tx->z = pShellMap->pScram_buf[pShellMap->scram_idx-4] + (pShellMap->pScram_buf[pShellMap->scram_idx-3] << 1);
        }

        /* Goto next state to send MP */
        (p4->tx_vec_idx)++;

        /* Prime the bit stream, 4-bit at a time */
        pShellMap->scram_idx = 8;

        /* Assume it is a 4-point signal */
        if (pV34Rx->Num_Trn_Tx == V34_4_POINT_TRN)
        {
            p4->send_symbol_counter = p4->mp_size >> 1;
        }
        else
        {
            p4->send_symbol_counter = p4->mp_size >> 2;
        }

        p4->mp_idx = 0;
        p4->mp_scram_size = 0;
    }
}

/*******************************************************************/
/* this module for sending MP is a subroutine shared by the three  */
/* module    1. Phase4_Send_MP                                       */
/*           2. Phase4_Finish_MP                                   */
/*           3. Phase4_Finish_MP_Pi                                   */
/* F!****************************************************************/
CQWORD  V34_Phase4_Send_MP_Share(V34Struct *pV34, Phase4_Info *p4)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    UBYTE i, q;

    /* Scramble 8 bits at a time */
    if (p4->mp_scram_size <= 0)
    {
        (*pV34Rx->pfScram_byte)(&(p4->pMp_buf[p4->mp_idx]), &(pV34Rx->sc), pShellMap->pScram_buf, 1);
        pShellMap->scram_idx = 0;
        p4->mp_scram_size = 8;
        ++p4->mp_idx;
    }

    /* Create the symbol from MP sequence */
    i = pShellMap->pScram_buf[pShellMap->scram_idx] + 2 * pShellMap->pScram_buf[pShellMap->scram_idx+1];
    pShellMap->scram_idx += 2;
    p4->mp_scram_size    -= 2;
    pV34Tx->z = (pV34Tx->z + i) & 0x3;
    i = pV34Tx->z;

    if (pV34Rx->Num_Trn_Tx == V34_4_POINT_TRN)
    {
        return(V34_tTRAIN_MAP_4[i]);
    }
    else
    {
        q = pShellMap->pScram_buf[pShellMap->scram_idx] + 2 * pShellMap->pScram_buf[pShellMap->scram_idx+1];
        pShellMap->scram_idx += 2;
        p4->mp_scram_size -= 2;
        return(V34_tTRAIN_MAP_16[4*q + i]);
    }
}

/******************************************************************/
/* Send out MP and wait for remote MP,                              */
/* this module will be terminated once MP is received on the      */
/* receiving side                                                  */
/******************************************************************/
void V34_Phase4_Send_MP(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOutsymbol;

    cqOutsymbol = V34_Phase4_Send_MP_Share(pV34, p4);
    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    if (p4->mp_idx >= p4->mp_size)
    {
        if (p4->mp_idx == V34_MP_1_SIZE)
        {
            pV34Rx->pfScram_bit((p4->pMp_buf[p4->mp_idx]), &(pV34Rx->sc), pShellMap->pScram_buf + 8, 4); /* The last 4 bits in MP1 */
            p4->mp_scram_size += 4;
        }

        p4->mp_idx = 0;
    }
}

void V34_Phase4_Finish_MP(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOutsymbol;

    cqOutsymbol = V34_Phase4_Send_MP_Share(pV34, p4);
    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    if (p4->mp_idx >= p4->mp_size)
    {
        if (p4->mp_idx == V34_MP_1_SIZE)
        {
            (*pV34Rx->pfScram_bit)((p4->pMp_buf[p4->mp_idx]), &(pV34Rx->sc), pShellMap->pScram_buf + 8, 4); /* The last 4 bits in MP1 */
            p4->mp_scram_size += 4;
        }

        p4->mp_idx = 0;

        /* Current MP is finished */
        /* Set the acknowledge bit of MP */
        /* Finalize the tx bit rate and rx bit rate, not implement */
        memcpy(p4->pMp_buf, p4->pMp_buf_pi, sizeof(p4->pMp_buf));

        if (pV34->clear_down == 0 && pShellMap->M > 1)
        {
            V34_Calc_g8(pShellMap->M, pShellMap->pG4, pShellMap->pG8);
        }

        (p4->tx_vec_idx)++;
        /* Just send one MP', so call Finish_MP_Pi */
        /*        (p4->tx_vec_idx)++; */
    }
}

/* V34_Phase4_MP_Pi()******************************************/
void V34_Phase4_Finish_MP_Pi(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOutsymbol;

    cqOutsymbol = V34_Phase4_Send_MP_Share(pV34, p4);
    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    if ((p4->mp_idx >= p4->mp_size) && (pShellMap->scram_idx >= 8))
    {
        if (p4->mp_idx == V34_MP_1_SIZE)
        {
            (*pV34Rx->pfScram_bit)((p4->pMp_buf[p4->mp_idx]), &(pV34Rx->sc), pShellMap->pScram_buf + 8, 4); /* The last 4 bits in MP1 */
            p4->mp_scram_size += 4;
            /* Send out all the leftover MP bits in scram buffer */
            (p4->tx_vec_idx) ++;
        }
        else if (p4->mp_idx == V34_MP_0_SIZE)
        {
            (p4->tx_vec_idx) += 2;
        } /* Don't need to call Flush MP Pi */   /* Goto Send E */

        p4->mp_idx = 0;

        if (pV34->clear_down == 0 && pShellMap->M > 1)
        {
            V34_Calc_z8(pShellMap->M, pShellMap->pG8, pShellMap->pZ8);
        }
    }
}


void V34_Phase4_Flush_MP_Pi(V34Struct *pV34)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOutsymbol;

    cqOutsymbol = V34_Phase4_Send_MP_Share(pV34, p4);
    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    if (p4->mp_scram_size <= 0)
    {
        /* All MP' bits are sent, prepare to send E */
        (p4->tx_vec_idx) ++;
    }
}

/*****************************/
/* Prepare to E for sending  */
/*****************************/
void V34_Phase4_Prepare_E(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOutsymbol;

    (*pV34Rx->pfScram_byte)(p4->pByte_stream, &(pV34Rx->sc), pShellMap->pScram_buf, 2);
    (*pV34Rx->pfScram_bit)(p4->pByte_stream[0], &(pV34Rx->sc), pShellMap->pScram_buf + 16, 4);

    pShellMap->scram_idx  = 0;
    pV34Tx->tx_sym_inidx  = 0;
    pV34Tx->tx_sym_outidx = 0;

    V34_Create_E(pV34);          /* Create the E sequence */

    cqOutsymbol.r = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].r;
    cqOutsymbol.i = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].i;

    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    pV34Tx->tx_sym_outidx ++;
    pV34Tx->tx_sym_outidx &= V34_SYM_BUF_MASK;

    (p4->tx_vec_idx) ++;
}

/***********/
/* Send E  */
/***********/
void V34_Phase4_Send_E(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    EchoStruc   *pEc    = &(pV34->Echo);
    Phase4_Info *p4     = &(pV34->p4);
    CQWORD cqOutsymbol;
    QDWORD qdTemp;

    cqOutsymbol.r = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].r;
    cqOutsymbol.i = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].i;

    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    pV34Tx->tx_sym_outidx ++;
    pV34Tx->tx_sym_outidx &= V34_SYM_BUF_MASK;

    if (pV34Tx->tx_sym_outidx == pV34Tx->tx_sym_inidx)
    {
        /* Prepare for transmitting one data frame */
        (p4->tx_vec_idx)++;

        pV34Rx->sc = 0;

        pV34Tx->z = 0;

        pV34Tx->Y0 = 0;
        pV34Tx->Aux.Aux_Flag = 0;

        pV34Tx->qdTx_scale = V34_tBASIC_SCALE[pV34Rx->tx_symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400];
        pV34Tx->qTx_nl_scale = V34_tOFFSET_NL_SCALE0[pV34Rx->tx_symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400];

        if (pV34Tx->Min_Exp)
        {
            qdTemp = QDQMULQD(pV34Tx->qdTx_scale, V34_tOFFSET_SCALE[pV34Rx->tx_symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400]);

            pV34Tx->qdTx_scale += qdTemp;

            pV34Tx->qTx_nl_scale = V34_tOFFSET_NL_SCALE1[pV34Rx->tx_symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400];
        }

        /* Since last data frame will used the last two bits of */
        /* the bit inversion pattern, which is bit 1 and bit 0  */
        /* we setup the variable to be 1.                       */
        pV34Tx->current_inv_bit = 0x1;

        /* That is the last data frame */
        pV34Tx->current_J = pV34Tx->Js - 1;

        memset(p4->pByte_stream, 0xFF, sizeof(p4->pByte_stream));

        /************ IO tx mode setup *****************/

        V34_Pre_Data_Share(pV34);
    }
}

/**************************/
/* Send B1, 1 data frame  */
/**************************/
void V34_Phase4_Send_B1_Frame(V34Struct *pV34)
{
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    EchoStruc *pEc = &(pV34->Echo);
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqOutsymbol;

    /* Pick out one symbol from buffer and send it out */

    cqOutsymbol.r = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].r;
    cqOutsymbol.i = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].i;

    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    pV34Tx->tx_sym_outidx ++;
    pV34Tx->tx_sym_outidx &= V34_SYM_BUF_MASK;

    if (pV34Tx->tx_sym_outidx == pV34Tx->tx_sym_inidx)
    {
        V34_Pre_Data_Share(pV34);

        if (pV34Tx->current_J == 0)
        {
            /*   (p4->tx_vec_idx)++;    */
            pV34->Echo.ec_adapt = 0;
            pV34->Echo.qNe_beta = 7; /* 0.00009 */
            pV34->Echo.qFe_beta = 7; /* 0.00009 */

            p4->Phase4_TxEnd = 1;
        }
    }
}

/************************************************************************/
/* The following function is for finishing up the current buffer, not   */
/* the actual transfer mode. The actual data mode code is located in    */
/* v34_tx.c.                                                            */
/************************************************************************/
void V34_Begin_Data_Transfer(V34Struct *pV34)
{
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    EchoStruc *pEc = &(pV34->Echo);
    CQWORD cqOutsymbol;

    cqOutsymbol.r = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].r;
    cqOutsymbol.i = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].i;

    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    pV34Tx->tx_sym_outidx ++;
    pV34Tx->tx_sym_outidx &= V34_SYM_BUF_MASK;

    if (pV34Tx->tx_sym_outidx == pV34Tx->tx_sym_inidx)
    {
        V34_Pre_Data_Share(pV34);
    }

    TRACE0("Called ?");
}
#endif
