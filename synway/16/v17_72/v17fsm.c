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

#include "v17ext.h"

CONST V17Fnptr TxLongSetUpVecTab[] =
{
    V17_Send_A,       /* send the unmodulated carrier of TEP    */
    V17_Send_SILENCE, /* send the silence of TEP                */
    V17_Send_seg1,    /* send the segment 1 of long train       */
    V17_Send_seg2,    /* send the segment 2 of long train       */
    V17_Send_seg3,    /* send the segment 3 of long train       */
    V17_Send_seg4,    /* send the segment 4 of long train       */
    V17_Send_TCF      /* send 1.5s TCF                          */
};

CONST SWORD  TxLongDelayCntTab[] =
{
    456,    /* send the unmodulated carrier of TEP */
    48,     /* send the silence of TEP             */
    256,    /* send the segment 1 of long train    */
    2976,   /* send the segment 2 of long train    */
    64,     /* send the segment 3 of long train    */
    48,     /* send the segment 4 of long train    */
    3600    /* send 1.5s TCF                       */
};


CONST  V17Fnptr TxShortSetUpVecTab[] =
{
    V17_Send_A,       /* send the unmodulated carrier of TEP    */
    V17_Send_SILENCE, /* send the silence of TEP           */
    V17_Send_seg1,    /* send the segment 1 of long train */
    V17_Send_seg2,    /* send the segment 2 of long train */
    V17_Send_seg4,    /* send the segment 4 of long train */
    V17_Send_TCF      /* send page data                   */
};


CONST SWORD  TxShortDelayCntTab[] =
{
    456,  /* send the unmodulated carrier of TEP */
    48,   /* send the silence of TEP             */
    256,  /* send the segment 1 of short train  */
    38,   /* send the segment 2 of short train  */
    48,   /* send the segment 4 of short train  */
    32000 /* send page data                     */
};

CONST  V17Fnptr RxLongSetUpVecTab[] =
{
    V17_Rx_Atone_or_ABtone,
    V17_Rx_Segm1AB_after_A,
    V17_Rx_Segm1_Long,
    V17_Rx_Segm2_Long,
    V17_Rx_Segm3_Long,
    V17_Rx_Segm4,
    V17_Rx_SegmData         /* get 1.5s TCF          */
};

CONST SWORD  RxLongDelayCntTab[] =
{
    30000,                  /* V17_Rx_Atone_or_ABtone */
    3000,                   /* V17_Rx_Segm1AB_after_A */
    V17_SEG1_DELAY,         /* V17_Rx_Segm1_Long     */
    V17_LONG_TRN_SEG2_DELAY,/* V17_Rx_Segm2_Long     */
    64,                     /* V17_Rx_Segm3_Long     */
    48,                     /* V17_Rx_Segm4          */
    3600                    /* get 1.5s TCF          */
};

CONST V17Fnptr  RxShortSetUpVecTab[] =
{
    V17_Rx_Atone_or_ABtone,
    V17_Rx_Segm1AB_after_A,
    V17_Rx_Segm1_Short,
    V17_Rx_Segm2_Short,
    V17_Rx_Segm4,
    V17_Rx_SegmData         /* get page data         */
};

CONST SWORD  RxShortDelayCntTab[] =
{
    30000,                  /* V17_Rx_Atone_or_ABtone */
    3000,                   /* V17_Rx_Segm1AB_after_A */
    V17_SEG1_DELAY,         /* V17_Rx_Segm1_Short    */
    37,                     /* V17_Rx_Segm2_Short    */
    48,                     /* V17_Rx_Segm4          */
    32000                   /* get page data         */
};

void V17_FSM_init(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    if (pV17->ubTraintype == V17_LONG_TRAIN)
    {
        /* TX Initialize */
        pV17->pfTxSetUpVecTab     = (V17Fnptr *)TxLongSetUpVecTab;
        pV32Share->nTxDelayCntTab = (SWORD *)TxLongDelayCntTab;

        /* RX Initialize */
        pV17->pfRxSetUpVecTab     = (V17Fnptr *)RxLongSetUpVecTab;
        pV32Share->nRxDelayCntTab = (SWORD *)RxLongDelayCntTab;
    }
    else
    {
        /* TX Initialize */
        pV17->pfTxSetUpVecTab     = (V17Fnptr *)TxShortSetUpVecTab;
        pV32Share->nTxDelayCntTab = (SWORD *)TxShortDelayCntTab;

        /* RX Initialize */
        pV17->pfRxSetUpVecTab     = (V17Fnptr *)RxShortSetUpVecTab;
        pV32Share->nRxDelayCntTab = (SWORD *)RxShortDelayCntTab;
    }
}
