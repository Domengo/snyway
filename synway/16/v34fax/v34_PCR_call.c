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

#include "v34fext.h"
#include "ioapi.h"
#include "acedef.h"

#if SUPPORT_V34FAX

#define DUMP_IMAGE_FROM_IO      (0)        /* t2 & t3,  GAO Tx */
#define TRANSMITTED_PAGE        (1)        /* page number from 1, not 0 */

void V34Fax_PCR_Send_B1(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info  *p3      = &(pV34->p3);
    V34TxStruct  *pV34Tx  = &(pV34->V34Tx);
    CQWORD cqOutsymbol;

    if (pV34Fax->tx_sym_cnt == 0)
    {
        V34Fax_PCR_B1_SetUp(pV34);
    }

    pV34Fax->tx_sym_cnt++;

    /* Pick out one symbol from Tx buffer and send it out */
    cqOutsymbol = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx];

    V34_Modulate(pV34Tx, &cqOutsymbol);

    pV34Tx->tx_sym_outidx ++;
    pV34Tx->tx_sym_outidx &= V34_SYM_BUF_MASK;

    /* Enter here only 112 symbols,
    left the last Mapping frame (8 symbols) sent in Image mode */
    if (pV34Tx->tx_sym_outidx == pV34Tx->tx_sym_inidx)
    {
        V34_Pre_Data_Share(pV34);

        if (pV34Tx->current_J == 0)
        {
            p3->tx_vec_idx ++;             /* send Image Data */
            pV34Fax->tx_sym_cnt = 0;
            TRACE1("Tx Image(Enter_CC_Order=%d)", pV34Fax->Enter_CC_Order);
        }
    }
}


void V34Fax_PCR_Transmitter(V34Struct *pV34)
{
#if DUMP_IMAGE_FROM_IO
    V34FaxStruct   *pV34Fax   = &(pV34->V34Fax);
    UBYTE i;
#endif
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    UBYTE *pIOBuf = (UBYTE *)(pV34->pTable[DSPIORDBUFDATA_IDX]);
    CQWORD cqOutsymbol;
    SBYTE nbyte, nbits, b;

    /* In fact, the first Mapping frame is B1 */
    cqOutsymbol = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx];

    V34_Modulate(pV34Tx, &cqOutsymbol);

    pV34Tx->tx_sym_outidx ++;
    pV34Tx->tx_sym_outidx &= V34_SYM_BUF_MASK;

    if (pV34Tx->tx_sym_outidx == pV34Tx->tx_sym_inidx)
    {
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

        /**************** IO Read from terminal *******************/
        IoRdProcess(pV34->pTable, b);

#if DUMP_IMAGE_FROM_IO

        if (pV34Fax->Enter_CC_Order >= TRANSMITTED_PAGE)
        {
            if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = b; }

            for (i = 0; i < 8; i++) //10
            {
                if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[i]; }
            }
        }

#endif
        /**************** IO Read from terminal ********************/

        (*pV34Rx->pfScram_byte)(pIOBuf, &(pV34Rx->sc), pShellMap->pScram_buf, nbyte);
        (*pV34Rx->pfScram_bit)(pIOBuf[nbyte], &(pV34Rx->sc), &(pShellMap->pScram_buf[b - nbits]), nbits);

        pShellMap->scram_idx = 0;

        V34_Send_Mapping_Frame(pV34);
    }
}


void V34Fax_PCR_Silence_Tx(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34TxStruct  *pV34Tx  = &(pV34->V34Tx);
    UBYTE i;

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        pV34Tx->PCMoutPtr[i] = 0;
    }

    pV34Fax->tx_sym_cnt++;

    if (pV34Fax->tx_sym_cnt >= 4)        /* 3 */
    {
        pV34Fax->PCR_TXend_Flag = 1;
    }
}


void V34Fax_PCR_B1_SetUp(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase4_Info *p4 = &(pV34->p4);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    Q30   qdTemp;
    UBYTE i;

    pV34Tx->Aux.Aux_Flag = 0;

    pV34Rx->sc = 0;/* Initial scrambler */

    pV34Tx->z  = 0;/* V34_DIFF_ENCODE_INIT */
    pV34Tx->Y0 = 0;

    pV34Tx->qdTx_scale = V34_tBASIC_SCALE[pV34Fax->symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400];

    if (pV34Tx->Min_Exp)
    {
        qdTemp = QDQMULQD(pV34Tx->qdTx_scale, V34_tOFFSET_SCALE[pV34Fax->symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400]);

        pV34Tx->qdTx_scale += qdTemp;

        pV34Tx->qTx_nl_scale = V34_tOFFSET_NL_SCALE1[pV34Fax->symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400];
    }
    else
    {
        pV34Tx->qTx_nl_scale = V34_tOFFSET_NL_SCALE0[pV34Fax->symbol_rate][pV34Rx->tx_bit_rate - V34_BIT_2400];
    }

    /* Since last data frame will used the last two bits of */
    /* the bit inversion pattern, which is bit 1 and bit 0  */
    /* we setup the variable to be 1.                       */
    pV34Tx->current_inv_bit = 0x1;

    /* That is the last data frame */
    pV34Tx->current_J = pV34Tx->Js - 1;

    for (i = 0; i < 10; i++)
    {
        p4->pByte_stream[i] = 0xFF;
    }

    V34_Pre_Data_Share(pV34);

    TRACE0("Tx B1");
}

#endif
