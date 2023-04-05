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

/***************************************************
This is a general header file for Data IO Interface.
****************************************************/

#ifndef _IOAPI_H
#define _IOAPI_H

#include "commdef.h"

/* Use 'DSPIORDBUFDATA_IDX' to obtain data from terminal */
/* Use 'DSPIOWRBUFDATA_IDX' to send data to the terminal */
/* both stored in LSB to MSB format */

void IoRdProcess(UBYTE **pTable, UWORD NumBits);
void IoWrProcess(UBYTE **pTable, UWORD NumBits);

#if SUPPORT_V21CH2FAX
void V21Fax_IoRdProcess(UBYTE **pTable);
void V21Fax_IoWrProcess(UBYTE **pTable);
#endif

#endif
