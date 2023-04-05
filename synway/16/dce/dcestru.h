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

#ifndef _DCESTRU_H
#define _DCESTRU_H

#include "dcedef.h"

#if (SUPPORT_MODEM || SUPPORT_V34FAX)

typedef struct
{
    /* Shared variables both in char mode and HDLC mode */
    UDWORD bitBufTx;
    UDWORD bitBufRx;
    UBYTE  numBitTx;
    UBYTE  numBitRx;
} AscStruct;

#endif

typedef struct
{
    void (*pfDCETxVec)(UBYTE **pTable, UBYTE *);/* Tx Vector */
    void (*pfDCERxVec)(UBYTE **pTable, UBYTE);  /* Rx Vector */

    UWORD numBitRd;
    UWORD bitBufRd;

    UWORD numBitWr;
    UWORD bitBufWr;

    UBYTE state;
} DceStruct;          /* Size=92 bytes(48-HDLC + 11-Galina + 33-the rest) */

#endif
