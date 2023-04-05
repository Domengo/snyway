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

#ifndef _HAWKAPI_H
#define _HAWKAPI_H

#include "commdef.h"
#include "modifdef.h"

typedef GAO_ModStatus Ctrl_Hardware_Signal;

#define HAWKHW_DB_SLAB       (23593)/* To change xmit level; 3.2 db for Slab */
#define HAWKHW_DB_ADI        (26196)/* To change xmit level; -1.9 db for ADI */
#define HAWKHW_DB_ST7550     (18741)/* To change xmit level; 1.2 db for ST7550 */

#if (HAWK_ADI + T1FRAMER)
#define HW_RINGON_THRESHOLD  (6) /* 6*50=300 ms */
#else
#define HW_RINGON_THRESHOLD  (15) /* 15/3*50=250 ms */
#endif

#define HW_RINGOFF_THRESHOLD (10)
#define HW_MAX_TIMER         (10)

#define TELOUT_ADDRESS       (0x3FC3)
#define SPKROUT_ADDRESS      (0x3FC1)
#define TELIN_ADDRESS        (0x3FC0)
#define MICIN_ADDRESS        (0x3FC2)
#define RING_ADDRESS         (0x3FC7)

#define BASE_ADDR            (0x340)
#define HAWKADDR             (BASE_ADDR + 2)
#define HAWKDATA             (BASE_ADDR + 0)

typedef struct
{
    UBYTE OnOffHook;
    SWORD RingOnCount;
    SWORD RingOffCount;
    UBYTE BufferSize;
} HawkStruct;

void dnDM(UWORD *p, UWORD addr, UWORD noWord);
void upDM(UWORD *p, UWORD addr, UWORD noWord);
UBYTE dnldProg(char *filename);

#endif
