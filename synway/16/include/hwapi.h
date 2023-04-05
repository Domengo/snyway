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

/**************************************************************************
    Description : This is a general header file for Hardware Interface.
**************************************************************************/

#ifndef _HWAPI_H
#define _HWAPI_H

#include "commdef.h"

/*
   bytes:
      1     - control field
      2     - sampling rate / AUDIO on/off
      3     - buffer size
*/

/* hardware control settings */
#define HWAPI_ONHOOK           (0x80)
#define HWAPI_OFFHOOK          (0x40)
#define HWAPI_SAMPLERATE       (0x20)
#define HWAPI_BUFSIZE          (0x10)
#define HWAPI_CPONHOOK         (0x08)
#define HWAPI_CPOFFHOOK        (0x04)
#define HWAPI_CIDON            (0x02)
#define HWAPI_CIDOFF           (0x01)

/* Audio control data */
#define HWAPI_AUDIO_ON         (1)
#define HWAPI_AUDIO_OFF        (0)

void HwInit(UBYTE **pTable);
void HwXferIn(UBYTE **pTable,  QWORD *PCM_in);
void HwXferOut(UBYTE **pTable, QWORD *PCM_out);

#if SAMCONV
void SwChgSamRate(UBYTE **pTable, UWORD rate, UWORD bufSize);
#endif

#endif
