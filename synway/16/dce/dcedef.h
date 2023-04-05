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

#ifndef _DCEDEF_H
#define _DCEDEF_H

#include "commdef.h"

#if SUPPORT_V34FAX

#define DCE_CHAR_MODE            1
#define DCE_HDLCFAX_MODE        2
#define DCE_FAX_MODE            3
#define DCE_V42_MODE            4
#define DCE_MNP_MODE             5

#else

#define DCE_CHAR_MODE            (0)
#define DCE_HDLCFAX_MODE        (1)
#define DCE_FAX_MODE            (2)
#define DCE_V42_MODE            (3)
#define DCE_MNP_MODE             (4)

#endif

/* DCE state */
#define DCE_STATE_INIT          (0)
#define DCE_STATE_ESTABLISH     (1)

#if SUPPORT_V34FAX
#define DCE_STATE_DATA          (2)
#define DCE_STATE_DISCONNECT    (3)
#endif

#if (SUPPORT_MODEM || SUPPORT_V34FAX)
#define STOP_MASK               (0x0100)   /* Create a mask for the stop bit */
#endif

#endif
