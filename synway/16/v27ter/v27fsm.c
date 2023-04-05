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

#include "v27ext.h"

#if V27_SHORT_TRAIN_SUPPORT
/* FSM for 'RxVec' delay counter */
/* assign proper timing to different states */
CONST SWORD V27_RxDelayTable_ShortTrain[V27_RX_FSM_SIZE-2] =
{
    V27_R_SEG1_308_SI,/*  32000; */
    V27_R_SHORT_SEG2_36_SI,
    V27_R_SHORT_SEG3_14_SI,
    V27_R2_SHORT_SEG4_58_SI,
    V27_SEG5_SHORT_8_SI,
    V27_R_DATA_TIME_1995
};

/* FSM for 'TxVec' delay counter */
/* assign proper symbol intervals to delay pointer for each segment */
CONST SWORD V27_TxDelayTable_ShortTrain[V27_TX_FSM_SIZE] =
{
    V27_T_SHORT_SEG1_308_SI,
    V27_T_SHORT_SEG2_36_SI,
    V27_T_SHORT_SEG3_14_SI,
    V27_T_SHORT_SEG4_58_SI,
    V27_SEG5_SHORT_8_SI,
    V27_T_DATA_TIME_2000
};

#endif

CONST SWORD V27_RxDelayTable_4800[V27_RX_FSM_SIZE] =
{
    32000,
    V27_R_SEG2_36_SI,
    V27_R_SEG3_50_SI,
    V27_EQ_DELAY,
    V27_1600_R1_SEG4_537_SI,
    V27_R2_SEG4_537_SI + 16,
    V27_SEG5_8_SI,
    V27_R_DATA_TIME_1995
};

CONST SWORD V27_TxDelayTable_4800[V27_TX_FSM_SIZE] =
{
    V27_1600_T_SEG1_308_SI,
    V27_1600_T_SEG2_36_SI,
    V27_T_SEG3_50_SI,
    V27_T_SEG4_1074_SI,
    V27_SEG5_8_SI,
    V27_T_DATA_TIME_2000
};

CONST SWORD V27_RxDelayTable_2400[V27_RX_FSM_SIZE] =
{
    32000,
    V27_R_SEG2_36_SI,
    V27_R_SEG3_50_SI,
    V27_EQ_DELAY,
    V27_1200_R1_SEG4_537_SI,
    V27_R2_SEG4_537_SI + 21,
    V27_SEG5_8_SI,
    V27_R_DATA_TIME_1995
};

CONST SWORD V27_TxDelayTable_2400[V27_TX_FSM_SIZE] =
{
    V27_1200_T_SEG1_231_SI,
    V27_1200_T_SEG2_27_SI,
    V27_T_SEG3_50_SI,
    V27_T_SEG4_1074_SI,
    V27_SEG5_8_SI,
    V27_T_DATA_TIME_2000
};

void V27_FSM_init(V27Struct *pV27)
{
    UBYTE i;

#if V27_SHORT_TRAIN_SUPPORT

    if (pV27->Short_Trn_Flag == 1)
    {
        /* assign vector pointers to different handshaking functions */
        i = 0;
        pV27->pfTxVecptr[i++] = V27_Segment_UnMod_Carrier;
        pV27->pfTxVecptr[i++] = V27_Segment_Silent;
        pV27->pfTxVecptr[i++] = V27_Segment_180_Phase;
        pV27->pfTxVecptr[i++] = V27_Segment_0_180_Phase;
        pV27->pfTxVecptr[i++] = V27_Segment_Scrambled_One;
        pV27->pfTxVecptr[i++] = V27_Data_Mode;

        pV27->nTxState   = -1;
        pV27->nTxDelayCnt = 0;

        /* assign vectors to different pointers */
        i = 0;
        pV27->pfRxVecptr[i++] = V27_Get_Segment_UnMod_Carrier;
        pV27->pfRxVecptr[i++] = V27_Rx_Segm2_Silent;
        pV27->pfRxVecptr[i++] = V27_Rx_Segm3_180_Phase;
        pV27->pfRxVecptr[i++] = V27_Segment_0_180_Phase_Fine;
        pV27->pfRxVecptr[i++] = V27_Get_Segment_Scrambled_One;
        pV27->pfRxVecptr[i++] = V27_Get_Data;

        pV27->nRxState   = -1;
        pV27->nRxDelayCnt = 0;
    }
    else
#endif
    {
        /* assign vector pointers to different handshaking functions */
        i = 0;
        pV27->pfTxVecptr[i++] = V27_Segment_UnMod_Carrier;
        pV27->pfTxVecptr[i++] = V27_Segment_Silent;
        pV27->pfTxVecptr[i++] = V27_Segment_180_Phase;
        pV27->pfTxVecptr[i++] = V27_Segment_0_180_Phase;
        pV27->pfTxVecptr[i++] = V27_Segment_Scrambled_One;
        pV27->pfTxVecptr[i++] = V27_Data_Mode;

        /* initilizes nTxState and nTxDelayCnt  */
        pV27->nTxState   = -1;
        pV27->nTxDelayCnt = 0;

        /* assign vectors to different pointers */
        i = 0;
        pV27->pfRxVecptr[i++] = V27_Rx_Segm1_Carrier;
        pV27->pfRxVecptr[i++] = V27_Rx_Segm2_Silent;
        pV27->pfRxVecptr[i++] = V27_Rx_Segm3_180_Phase;
        pV27->pfRxVecptr[i++] = V27_Segment_Eq_delay; /* do nothing! */
        pV27->pfRxVecptr[i++] = V27_Segment_0_180_Phase_Raw;
        pV27->pfRxVecptr[i++] = V27_Segment_0_180_Phase_Fine;
        pV27->pfRxVecptr[i++] = V27_Get_Segment_Scrambled_One;
        pV27->pfRxVecptr[i++] = V27_Get_Data;

        pV27->nRxState   = -1;
        pV27->nRxDelayCnt = 0;
    }
}
