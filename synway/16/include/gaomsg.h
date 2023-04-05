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

#ifndef _GAO_MSG_H_
#define _GAO_MSG_H_

#include "commtype.h"
#include "dsmdef.h"

#define MAX_LEN_API_MSG                  (128)

typedef struct
{
    UBYTE Direction;
    UBYTE MsgType;
    UWORD MsgCode;
    UBYTE Buf[MAX_LEN_API_MSG + 5];
} GAO_Msg;

SWORD GAO_Msg_Proc(void *pLibDataMem, GAO_Msg *pMsg);

#endif



