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

#include <string.h>
#include "circbuff.h"
#include "commmac.h"

/* ============================================================================
NOTE:-A CircBuffer may only access the buffer in READ_ONLY or WRITE_ONLY mode.
-Defining _CIRCULAR_BUFFER_TEST_ uncomments CircBuff Testing Routine.
-CircBuff Testing Routine is at bottom of this source.
============================================================================ */

/****************************************************************
function CB_InitCircBuffer: Initializes a CircBuffer stucture.

  Input: CB -- CircBuffer to be initialized.
  Buffer -- Where data is to be stored to / read from.
  BufferLen -- Length of the Buffer in bytes.
****************************************************************/
void CB_InitCircBuffer(CircBuffer *CB, UBYTE *Buffer, UWORD BufferLen)
{
    CB->Len      = BufferLen;
    CB->Top      = Buffer;
    CB->Start    = CB->Top;
    CB->End      = CB->Top;
    CB->CurrLen  = 0;
}

/****************************************************************
function CB_ResetCircBuffer: Resets a CircBuffer stucture.

  Input: CB -- CircBuffer to be initialized.
****************************************************************/
void CB_ResetCircBuffer(CircBuffer *CB)
{
    CB->Start    = CB->Top;
    CB->End      = CB->Top;
    CB->CurrLen  = 0;
}

/***********************************************************
Place string data into the circular buffer.

  Input: CB -- CircBuffer where data is to be inserted.
  Str -- Character array containing data to be inserted.
  StrLen -- Number of characters to be inserted.

(Writing data is now independent of reading data)
***********************************************************/
UBYTE PutStringToCB(CircBuffer *CB, CONST UBYTE *Str, UWORD StrLen)
{
    UWORD Dist;

    if ((StrLen == 0) || (StrLen > (CB->Len - CB->CurrLen)))
    {
        return 0;
    }

    Dist = CB->Len - (UWORD)(CB->End - CB->Top);

    if (Dist > StrLen)
    {
        memcpy(CB->End, Str, StrLen * sizeof(UBYTE));
        CB->End += StrLen;
    }
    else
    {
        if (Dist > 0)
        {
            memcpy(CB->End, Str, Dist * sizeof(UBYTE));
        }

        if (StrLen > Dist)
        {
            memcpy(CB->Top, (Str + Dist), (StrLen - Dist) * sizeof(UBYTE));
        }

        CB->End = CB->Top + (StrLen - Dist);
    }

    CB->CurrLen += StrLen;

    return 1;
}

/****************************************************************
Get string data from CB buffer

  Input: I -- Circular buffer from which data is to be extracted.
  O -- Buffer where extracted data will be placed.
  InLen -- Length of the circular buffer I.
  OutLen -- Number of bytes to be extracted.
  Output: How many charaters have been read -- by Yalan Xu
*****************************************************************/
UWORD GetStringFromCB(CircBuffer *CB, UBYTE *O, UWORD OutLen)
{
    UWORD Dist, RetVal;

    if (CB->CurrLen >= OutLen)
    {
        RetVal = OutLen;
    }
    else
    {
        RetVal = CB->CurrLen;
    }

    if (RetVal == 0)
    {
        return 0;
    }

    CB->CurrLen -= RetVal; /* move this from the end of the function to here */

    Dist = CB->Len - (UWORD)(CB->Start - CB->Top);

    if (Dist > RetVal)
    {
        memcpy(O, CB->Start, RetVal * sizeof(UBYTE));

        CB->Start += RetVal;
    }
    else
    {
        if (Dist > 0)
        {
            memcpy(O, CB->Start, Dist * sizeof(UBYTE));
        }

        if (RetVal > Dist)
        {
            memcpy(O + Dist, CB->Top, (RetVal - Dist) * sizeof(UBYTE));
        }

        CB->Start = CB->Top + (RetVal - Dist);
    }

    return RetVal;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function CB_to_CB:
Copies data from CBI circular buffer into CBO circular buffer.

  Input: CBI -- Circular buffer from which data is to be extracted.
  CBO -- Circular buffer where extracted data will be placed.
  LenI -- Number of bytes of data within the circular buffer I.
  LenO -- Number of bytes of data within the circular buffer O.
  Output: Number of copied bytes.

    Author: Galina Serova(01/09/1998)
    Ben , small optimization (07/15/1998)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
UWORD CB_to_CB(CircBuffer *CBI, CircBuffer *CBO)
{
    UWORD i, BCopy; /* N bytes to copy=min(*LenI, spece in out buffer) */
    UWORD RetVal;
    UBYTE *pCBO_EndPtr;
    UBYTE *pCBI_EndPtr;

    BCopy = MIN16(CBI->CurrLen, CBO->Len - CBO->CurrLen);

    if (!BCopy)
    {
        return 0;    /* Output buffer is full or input is empty. */
    }

    RetVal = BCopy;

    pCBO_EndPtr = CBO->Top + CBO->Len;
    pCBI_EndPtr = CBI->Top + CBI->Len;

    for (i = 0; i < BCopy; i++)
    {
        *(CBO->End++) = *(CBI->Start++);

        if (CBO->End >= pCBO_EndPtr)    //??? ==
        {
            CBO->End = CBO->Top;
        }

        if (CBI->Start >= pCBI_EndPtr)  //??? ==
        {
            CBI->Start = CBI->Top;
        }
    }

    CBO->CurrLen += RetVal;
    CBI->CurrLen -= RetVal;

    return RetVal;
}

/* Put a byte data into CB buffer */
UBYTE PutByteToCB(CircBuffer *CB, UBYTE ch)
{
    if (CB->CurrLen < CB->Len - 1)
    {
        *(CB->End) = ch;

#if 0

        if (DumpTone1_Idx < 1000000) { DumpTone1[DumpTone1_Idx++] = ch; }

#endif

        if ((++(CB->End)) >= CB->Top + CB->Len)
        {
            CB->End = CB->Top;
        }

        CB->CurrLen ++;

        return 1;
    }
    else
    {
        return 0;
    }
}

/* Get a byte data from CB buffer */
UBYTE GetByteFromCB(CircBuffer *CB, UBYTE *ch)
{
    if (CB->CurrLen)
    {
        *ch = *(CB->Start);

#if 0

        if (DumpTone2_Idx < 1000000) { DumpTone2[DumpTone2_Idx++] = *ch; }

#endif

        if ((++CB->Start) >= CB->Top + CB->Len)
        {
            CB->Start = CB->Top;
        }

        CB->CurrLen --;

        return 1;
    }
    else
    {
        return 0;
    }
}

/* Peek a byte data from CB buffer */
UBYTE PeekByteFromCB(CircBuffer *CB, UBYTE *ch, UWORD offset)
{
    if (CB->CurrLen > offset)
    {
        if (CB->Start + offset >= CB->Top + CB->Len)
        {
            *ch = *(CB->Start + offset - CB->Len);
        }
        else
        {
            *ch = *(CB->Start + offset);
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UWORD GetAvailLenInCB(CircBuffer *CB)
{
    return CB->Len - CB->CurrLen - 1;
}

#ifdef _CIRCULAR_BUFFER_TEST_

UBYTE Array1[10], Output1[10];

int main()
{
    CircBuffer RdBuf, WrBuf;
    UWORD Rec;
    int Ctr = 0;

    CB_InitCircBuffer(&RdBuf, Array1, 10);
    CB_InitCircBuffer(&WrBuf, Array1, 10);

    while (1)
    {
        PutStringToCB(&WrBuf, "012345", 6);
        Rec = GetStringFromCB(&WrBuf, Output1, 4);
        Output1[Rec] = 0;
        TRACE("%s", Output1);

        if (++Ctr == 9)
        {
            Ctr = 0;
        }
    }

    return 0;
}
#endif
