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

CONST V34FnPtr V34Fax_PCR_Rx_Fsm[] =
{
    V34Fax_PCR_Detect_S,            /* Rx: State 0 */
    V34Fax_PCR_Detect_S_bar,        /* Rx: State 1 */
    V34Fax_PCR_S_Bar_Train,         /* Rx: State 2 */
    V34Fax_PCR_PP_EQ_Train,         /* Rx: State 3 */
    V34Fax_PCR_VA_Delay,            /* Rx: State 4 */
    V34Fax_PCR_B1_Rx,               /* Rx: State 5 */
    V34Fax_PCR_Image_Rx,            /* Rx: State 6 */
};


CONST V34FnPtr V34Fax_PCR_Tx_Fsm[] =
{
    V34Fax_PCET_PCR_Send_Silence_70MS,/* Tx: State 0 */
    V34Fax_PCET_PCR_Send_S,           /* Tx: State 1 */
    V34Fax_PCET_PCR_Send_S_Bar,       /* Tx: State 2 */
    V34Fax_PCET_PCR_Send_PP,          /* Tx: State 3 */
    V34Fax_PCR_Send_B1,               /* Tx: State 4 */
    V34Fax_PCR_Transmitter,           /* Tx: State 5 */
    V34Fax_PCR_Silence_Tx,            /* Tx: State 6 */
};

#endif
