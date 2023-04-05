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

#ifndef _CIRCBUFF_H
#define _CIRCBUFF_H

#include "commdef.h"

typedef struct _CircBuffer
{
    UWORD Len;
    UWORD CurrLen;

    UBYTE *Top;
    UBYTE *Start;
    UBYTE *End;
} CircBuffer;

#define CB_EMPTY_SPACE(pCB)         ((pCB)->Len - (pCB)->CurrLen - 1)

void CB_InitCircBuffer(CircBuffer *, UBYTE *, UWORD);
void CB_ResetCircBuffer(CircBuffer *CB);

UWORD CB_to_CB(CircBuffer *, CircBuffer *);

UBYTE GetByteFromCB(CircBuffer *CB, UBYTE *ch);
UBYTE PutByteToCB(CircBuffer *CB, UBYTE ch);

UWORD GetStringFromCB(CircBuffer *, UBYTE *, UWORD);
UBYTE PutStringToCB(CircBuffer *, CONST UBYTE *, UWORD);

UBYTE PeekByteFromCB(CircBuffer *CB, UBYTE *ch, UWORD offset);

UWORD GetAvailLenInCB(CircBuffer *CB);

#endif
