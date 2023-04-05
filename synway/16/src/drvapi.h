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

#ifndef _DRVAPI_H
#define _DRVAPI_H

#include "modifdef.h"

typedef QWORD PCM_TYPE;

BOOL Processor_Side_Initialize(void);
void Hook(UBYTE state);
void HwCheckState(GAO_ModStatus *pStatus);
void HwChgState(GAO_ModStatus *pStatus);
void DriverXferIn(void);
void Data_Send_Prepare(void);
void Device_Close();

#if SUPPORT_PCI_BOARD/********************************/
extern UBYTE Interrupt_Status;

extern PCM_TYPE *pPCMIn;
extern PCM_TYPE *pPCMOut;
void Enable_Telephony_function(GAO_ModStatus *pStatus);
void Check_Telephony_function(GAO_ModStatus *pStatus);

#else/*SUPPORT_HAWKBOARD*****************************/

extern PCM_TYPE gPCMInBuf[];
extern PCM_TYPE gPCMOutBuf[];

#endif/**********************************************/

#endif
