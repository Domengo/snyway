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

/***************************************************************/
/* Input: symbol_rate,data_rate,auxiliary_data,                */
/*  CM -- mode_modem_type(CM = 0, expanded, otherwise minimum) */
/*  carrier_type (carrier_type = 0, low carrier frequency,     */
/*      carrier_type = 1, high carrier frequency.)             */
/*  Aux -- auxiliary channel (Aux = 1, using auxiliary channel)*/
/***************************************************************/
void  V34_Send_Mapping_Frame(V34Struct *pV34)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    SWORD AMP_bit, SWP_bit;

    /************************* Framing (8) *****************************/
#if 0

    if (pV34Tx->Aux.Aux_Flag == 1)
    {
        AMP_bit = (pV34Tx->Aux.current_AMP_bit) & 0x1;
        pV34Tx->Aux.current_AMP_bit >>= 1;
    }

#endif

    AMP_bit = 0;
    /* bit=0 low frame (b-1)      bit=1 high frame (b)     */
    SWP_bit = (pV34Tx->current_SWP_bit) & 0x1;
    pV34Tx->current_SWP_bit >>= 1;

    /*******************************************************************/

    /************************* Encoder (9) *****************************/
    /* Do one single mapping frame encoding, including Parser,         */
    /* Shell Mapper, Differential Encoder, Mapper, Precoder,           */
    /* Trellis Encoder, and Non-linear Encoder.                        */
    /*******************************************************************/
    V34_Encoder(SWP_bit, AMP_bit, pV34Tx);
}
