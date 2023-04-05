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

/**********************************************************************
File Name : HwStru.H

  Description : This is a general header file for Hardware Contorl.

    Authors :  Thomas PAUL
***********************************************************************/

#ifndef _HWSTRU_H
#define _HWSTRU_H

#include "commdef.h"

#define HW_ONHOOK     (0)
#define HW_OFFHOOK    (1)

typedef struct
{
    UBYTE OnOffHook;
    UBYTE BufferSize;
    UBYTE SampRate;
    UBYTE Pre_BufferSize;
    UBYTE Pre_SampRate;
} HwStruct;

#endif
