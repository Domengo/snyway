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

#include "v29ext.h"

/* FSM for 'RxVec' delay counter */
/* assign proper timing to different states */
CONST SWORD V29_RxDelayTable[V29_RX_FSM_SIZE] =
{
    32767,/* assign any big number */
    V29_GET_SEGMENT1_DELAY,
    V29_GET_SEGMENT2_DELAY,
    V29_EQ_DELAY,
    V29_SEG3_384_SI,
    V29_SEG4_48_SI,
    V29_R_DATA_TIME_10000
};

/* FSM for 'TxVec' delay counter */
/* assign proper symbol intervals to delay pointer for each segment */
CONST SWORD V29_TxDelayTable[V29_TX_FSM_SIZE] =
{
    V29_T_TEP_456_SI,
    V29_T_SEG1_48_SI,
    V29_T_SEG2_128_SI,
    V29_SEG3_384_SI,
    V29_SEG4_48_SI,
    V29_T_DATA_TIME_10000
};

void V29_FSM_init(V29Struct *pV29)
{
    UBYTE i;

    /* assign vector pointers to different handshaking functions */
    i = 0;
    pV29->pfTxVecptr[i++] = V29_TEP;
    pV29->pfTxVecptr[i++] = V29_Segment_1;
    pV29->pfTxVecptr[i++] = V29_Segment_2;
    pV29->pfTxVecptr[i++] = V29_Segment_3;
    pV29->pfTxVecptr[i++] = V29_Segment_4;
    pV29->pfTxVecptr[i++] = V29_Data_Mode;

    /* initilises nTxState and nTxDelayCnt */
    pV29->nTxState    = -1;
    pV29->nTxDelayCnt = 0;

    /* assign vectors to different pointers */
    i = 0;
    pV29->pfRxVecptr[i++] = V29_Rx_Segm1_ToneDetect;
    pV29->pfRxVecptr[i++] = V29_Rx_Segm1_Silent;
    pV29->pfRxVecptr[i++] = V29_Rx_Segm2;
    pV29->pfRxVecptr[i++] = V29_EQ_Delay;
    pV29->pfRxVecptr[i++] = V29_Rx_Segm3;
    pV29->pfRxVecptr[i++] = V29_Rx_Segm4;
    pV29->pfRxVecptr[i++] = V29_Rx_DataMode;

    pV29->nRxState = -1;
    pV29->nRxDelayCnt = 0;
}
