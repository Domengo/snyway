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

#ifndef _HDLCSTRU_H
#define _HDLCSTRU_H

#include "commdef.h"
#include "hdlcdef.h"

typedef struct
{
    UWORD FrameStart;              /* the index of the first meaningful raw data */
    UWORD FrameEnd;                /* the index of the last meaningful raw data */
    UWORD WhereToStart;            /* from where to start the action of cutting out transparency bits */
    UWORD RawDataEnd;              /* the index of the last raw data received */
    UWORD WorkingBuffEnd;          /* the index of the last bit in Working Buffer */
    UWORD WorkingBuffLoc;          /* from where to start the action of encoding in working buffer */
    UBYTE Status;                  /* the status of frame, should be one of the follow */
    UWORD CRC;                     /* CRC value. The initial vaule is 0xFFFF */
    UBYTE RawData[HDLCBUFLEN];     /* the buffer to store the raw data from Modem */
    UBYTE WorkingBuff[HDLCBUFLEN]; /* the working buffer */
    UBYTE OnesCount;               /* the counter of 1's */
} HdlcStruct;

#endif