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

#if SUPPORT_V34FAX

void V34Fax_PCET_PCR_Send_Silence_70MS(V34Struct *pV34)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    Phase3_Info *p3     = &(pV34->p3);
    UBYTE i;

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        pV34Tx->PCMoutPtr[i] = 0;
    }

    p3->symbol_counter ++;

    if (p3->symbol_counter >= 224) /* 70ms in 9.6kHz has 224 symbols */
    {
        (p3->tx_vec_idx) ++;
        p3->symbol_counter = 0;
        TRACE0("Tx 70ms silience");
    }
}


void V34Fax_PCET_PCR_Send_S(V34Struct *pV34)
{
    Phase3_Info *p3     = &(pV34->p3);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    CQWORD cqOut_sym;

    p3->symbol_counter ++;

    if (p3->symbol_counter >= 128)
    {
        p3->tx_vec_idx ++;      /* Send S_BAR */
        p3->symbol_counter = 0;
        TRACE0("Tx 128T S");
    }

    cqOut_sym = V34_Create_S_or_SBar(&(p3->S_point));

    V34_Modulate(pV34Tx, &cqOut_sym);
}


void V34Fax_PCET_PCR_Send_S_Bar(V34Struct *pV34)
{
    Phase3_Info *p3     = &(pV34->p3);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    //EchoStruc   *pEc    = &(pV34->Echo);
    CQWORD cqOut_sym;

    cqOut_sym = V34_Create_S_or_SBar(&(p3->S_point_BAR));

    V34_Modulate(pV34Tx, &cqOut_sym);

    p3->symbol_counter ++;

    if (p3->symbol_counter >= 16)
    {
        p3->tx_vec_idx ++;      /* goto send PP */

        TRACE0("Tx 16T S bar");

        p3->symbol_counter = 0;
        p3->I = 0;               /* Init for PP sequence */
        p3->K = 0;               /* Init for PP sequence */
    }
}


void V34Fax_PCET_PCR_Send_PP(V34Struct *pV34)
{
    V34FaxStruct   *pV34Fax   = &(pV34->V34Fax);
    Phase3_Info    *p3        = &(pV34->p3);
    V34RxStruct    *pV34Rx    = &(pV34->V34Rx);
    V34TxStruct    *pV34Tx    = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    CQWORD cqOut_sym;

    if (p3->I > 3)
    {
        p3->I = 0;
        p3->K ++;
    }

    cqOut_sym = V34_Create_PP(p3->K, p3->I);

    p3->I ++;

    V34_Modulate(pV34Tx, &cqOut_sym);

    p3->symbol_counter ++;

    if (p3->symbol_counter >= 288)
    {
        TRACE0("Tx 288T PP");
        p3->tx_vec_idx ++;              /* Send TRN sequence */
        p3->symbol_counter = 0;

        pV34Rx->sc = 0;                  /* Initial scrambler */

        pShellMap->scram_idx = 16;       /* Prime # scrambled bits to 16 */
        p3->pByte_stream[0]  = 0xFF;
        p3->pByte_stream[1]  = 0xFF;

        if (pV34Fax->trn_length > 0)
        {
            if (pV34Fax->Enter_CC_Order == 0)
            {
                pV34Fax->trn_length += (pV34Fax->trn_length >> 1);    /* GaK said HP needs more time, but it is too long, >>1, Zhiyong changed back from 3 to 1 */
            }

            /* 112 symbols is 35ms for 3200Hz symbol rate */
            /* 84 symbols is 35ms for 2400Hz symbol rate */
            switch (pV34Fax->symbol_rate)
            {
                case V34_SYM_2400:
                    p3->detect_symbol_counter = (pV34Fax->trn_length * 84);
                    break;
                case V34_SYM_2743:
                    p3->detect_symbol_counter = (pV34Fax->trn_length * 96);
                    break;
                case V34_SYM_2800:
                    p3->detect_symbol_counter = (pV34Fax->trn_length * 98);
                    break;
                case V34_SYM_3000:
                    p3->detect_symbol_counter = (pV34Fax->trn_length * 105);
                    break;
                case V34_SYM_3200:
                    p3->detect_symbol_counter = (pV34Fax->trn_length * 112);
                    break;
                case V34_SYM_3429:
                    p3->detect_symbol_counter = (pV34Fax->trn_length * 120);
                    break;
            }
        }
        else
        {
            pV34Fax->PCET_TXend_Flag = 1;
        }
    }
}


void V34Fax_PCET_Send_TRN(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info *p3 = &(pV34->p3);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    CQWORD cqOut_sym;

    if (pShellMap->scram_idx >= 16)
    {
        (*pV34Rx->pfScram_byte)(p3->pByte_stream, &(pV34Rx->sc), pShellMap->pScram_buf, 2);

        pShellMap->scram_idx = 0;
    }

    if (pV34Fax->TRN_constellation_point == 1)
    {
        cqOut_sym = V34_Create_TRN(pShellMap->pScram_buf, &(pShellMap->scram_idx), V34_16_POINT_TRN);
    }
    else
    {
        cqOut_sym = V34_Create_TRN(pShellMap->pScram_buf, &(pShellMap->scram_idx), V34_4_POINT_TRN);
    }

    V34_Modulate(pV34Tx, &cqOut_sym);

#if DRAWEPG
    pV34Tx->pcTx_sym[0] = cqOut_sym;
#endif

    p3->symbol_counter ++;

    if ((p3->symbol_counter >= p3->detect_symbol_counter) && (pShellMap->scram_idx >= 16))
    {
        pV34Fax->PCET_TXend_Flag = 1;
        TRACE0("Tx TRN");
    }
}

#endif
