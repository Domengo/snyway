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

CONST V34FnPtr V34Fax_PCET_Tx_Fsm[] =
{
    V34Fax_PCET_PCR_Send_Silence_70MS,/* Tx: State 0 */
    V34Fax_PCET_PCR_Send_S,           /* Tx: State 1 */
    V34Fax_PCET_PCR_Send_S_Bar,       /* Tx: State 2 */
    V34Fax_PCET_PCR_Send_PP,          /* Tx: State 3 */
    V34Fax_PCET_Send_TRN,             /* Tx: State 4 */
};


CONST V34FnPtr V34Fax_PCET_Rx_Fsm[] =
{
    /* Consider fast sync algorithm */
    V34Fax_PCET_Det_S,        /* Rx: State 0 */
    V34Fax_PCET_Det_S_bar,    /* Rx: State 1 */
    V34Fax_PCET_Wait_S_bar,   /* Rx: State 2 */
    V34Fax_PCET_EQ_Delay,     /* Rx: State 3 */
    V34Fax_PCET_PP_EQ,        /* Rx: State 4 */
    V34Fax_PCET_TRN_EQ,       /* Rx: State 5 */
    V34Fax_PCET_TRN_EQ_TC,    /* Rx: State 6 */
  //V34Fax_PCET_Rec_Silence,  /* Rx: State 7 */
};

#endif
