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

#ifndef _DCEEXT_H
#define _DCEEXT_H

#include "dcestru.h"

void  DCEInit(UBYTE **pTable);
void  DCEMain(UBYTE **pTable);

#if (SUPPORT_MODEM || SUPPORT_V34FAX)
void  ASCInit(UBYTE **pTable);
void  ASCMain(UBYTE **pTable);
void  ASCIOTx(UBYTE **pTable, UBYTE *ch);
void  ASCIORx(UBYTE **pTable, UBYTE ch);
#endif

#if SUPPORT_FAX
void  FAXInit(UBYTE **pTable);
void  FAXMain(UBYTE **pTable);
void  FAXIOTx(UBYTE **pTable, UBYTE *ch);
void  FAXIORx(UBYTE **pTable, UBYTE ch);
#endif

#if SUPPORT_V42
void  V42Init(UBYTE **pTable);
void  V42Main(UBYTE **pTable);
void  V42IOTx(UBYTE **pTable, UBYTE *ch);
void  V42IORx(UBYTE **pTable, UBYTE ch);
#endif

#if SUPPORT_MNP
void  Mnp_Init(UBYTE **pTable);
void  Mnp_Main(UBYTE **pTable);
void  Mnp_IO_Tx(UBYTE **pTable, UBYTE *ch);
void  Mnp_IO_Rx(UBYTE **pTable, UBYTE ch);
#endif

#endif
