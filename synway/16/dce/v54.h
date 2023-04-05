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

/*****************************************
        V54 definitions and structure
*****************************************/

#ifndef _V54_H
#define _V54_H

#include "commdef.h"

/*      --- RDL control bits ---
 *
 * S register 23 : bit 0  -- enable to response remote RDL request
 * S register 16 : bit 4,5 -- &T6 &T7 RDL (local) test initiation
 */

#define RDLT_TERM         (BIT0)      /*             T0                  */
#define RDLT_REQ          (BIT4)      /* local RDL request &T6, 7        */
#define RDLT_RESP         (BIT7)      /* response to remote RDL request  */

/* ---- RDL status bits ---- */
#define RDLS_PREP_TX      ( 1)      /* preparation signal transmission */
#define RDLS_PREP_TX_DONE ( 2)      /*      "        "    transmission finished */
#define RDLS_PREP_RX      ( 2)      /*      "        "    received   */
#define RDLS_ACK_TX       ( 3)      /* acknowledge signal transmission */
#define RDLS_ACK_TX_DONE  ( 4)      /*      "        "    finished   */
#define RDLS_ACK_RX       ( 5)      /*      "        "    received   */
#define RDLS_TEST         ( 6)
#define RDLS_TERM_TX      ( 7)      /* termination signal transmission */
#define RDLS_TERM_BINARY  ( 8)      /* termination binary 1 transmission */
#define RDLS_TERM_TX_DONE ( 9)
#define RDLS_TERM_LB_DONE (10)      /* the termination binary 1 loop back received */
#define RDLS_TERM_RX      (11)      /*      "        "    received   */

typedef struct
{
    UBYTE  bLoopBack;
    UBYTE  uState;
    UBYTE  uCmd;
    UBYTE  uScramBuf;
    UBYTE  uDescramBuf;
    SWORD  nNumScramOne;        /* number of (scrambled) 1 received */
    SWORD  nNumScramZero;       /* number of (scrambled) 0 received */
    SWORD  nTxBitCnt;           /* number of bits to be transmitted */
    SWORD  nNumBinaryOne;       /* when modem is idle, it sends binary ONE */
    /*  this saturates the V54 descrambler */
} V54Struct;

void  V54_Init(UBYTE **pTable);
void  V54_StartRDL(UBYTE **pTable);
void  V54_TerminateRDL(UBYTE **pTable);
UBYTE V54_TerminateDone(UBYTE **pTable);
UBYTE V54_RxByte(UBYTE **pTable, UBYTE in_byte);
UBYTE V54_TxByte(UBYTE **pTable, UBYTE *pOut);
UBYTE ReportSelfTestErr(UBYTE **pTable, SWORD err_cnt);
UBYTE V54_RDL_Acknowledge(UBYTE **);

#endif
