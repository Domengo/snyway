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

#if defined(_T30EXT_H)
//Reentry!
#else
#define _T30EXT_H

#include "t30def.h"
#include "t30stru.h"
#include "circbuff.h"

extern CONST UBYTE T30_BytesPer10ms[14];
extern CONST UWORD ImageWidth_Tiff[3][8];

// T30-DTE interface
void T30_SetResponse(T30Struct *, UBYTE Number);
void T30_UploadPgEndToDte_ECM(T30Struct *pT30);
void T30_AssumeEopUponInvCmd(T30Struct *pT30);
void T30_UploadImgToDte(T30Struct *pT30, UBYTE *pSrc, UWORD len, UBYTE isBitReversed);

// T30-DCE interface
UBYTE T30_ConvertSpdToT31Mod(UBYTE T30Speed, UBYTE isLongTraining);
void T30_SetCommand(T30ToDceInterface *pT30ToDce, UBYTE command);

// T30 HDLC TX/RX
UBYTE T30_GetHDLCfromRemote(T30Struct *pT30, UBYTE isCmd);
void T30_ProcessOptionalFrame(T30Struct *pT30, UWORD type);
void T30_StopHDLCReceiving(T30Struct *pT30);
void T30_StopHDLCSending(T30Struct *);
UBYTE T30_MakeDIS_DTC(T30Struct *, UBYTE);
UBYTE T30_MakeDCS_CTC(T30Struct *, UBYTE);
UBYTE T30_CommandTx(T30Struct *, UWORD);
UBYTE T30_AnalysisDIS_DTC(T30Struct *, UBYTE);
UBYTE T30_AnalysisDCS_CTC(T30Struct *, UBYTE);

// T30 API
void T30_Init(T30Struct *);
void T30_Main(T30Struct *);

// T30 FSM, hangup phase
void T30_PhaseHangUp(T30Struct *pT30);

// Phase A
void T30_PhaseA(T30Struct *pT30);

// Phase B
void T30_PhaseB_AfterEOM(T30Struct *pT30);
void T30_PhaseA_Call(T30Struct *pT30);
void T30_PhaseA_Ans(T30Struct *pT30);
void T30_PhaseB_SendDCS(T30Struct *pT30);
void T30_PhaseB_GetDCS(T30Struct *pT30);
void T30_PhaseB_TCF_Tx(T30Struct *pT30);
void T30_PhaseB_TCF_Rx(T30Struct *pT30);
void T30_PhaseB_SendConf(T30Struct *pT30);
void T30_PhaseB_GetConf(T30Struct *pT30);

// Phase C
void T30_PhaseC_ImageDataTx_NoECM(T30Struct *pT30);
void T30_PhaseC_ImageDataRx_NoECM(T30Struct *pT30);

// Phase D
void T30_PhaseC_SendConf_NoECM(T30Struct *pT30);
void T30_PhaseC_GetConf_NoECM(T30Struct *pT30);
void T30_PhaseD_SendPE_NoECM(T30Struct *pT30);
void T30_PhaseD_GetPE_NoECM(T30Struct *pT30);

// Phase E
void T30_PhaseE_Call(T30Struct *pT30);
void T30_PhaseE_Ans(T30Struct *pT30);

void T30_T4InitLeftoverBuf(T4ECMstruc *pT4);

void T30_PhaseC_ImageDataTx_ECM(T30Struct *pT30);
void T30_PhaseC_ImageDataRx_ECM(T30Struct *pT30);

void T30_PhaseC_SendConf_ECM(T30Struct *pT30);
void T30_PhaseC_GetConf_ECM(T30Struct *pT30);
void T30_PhaseD_SendPE_ECM(T30Struct *pT30);
void T30_PhaseD_GetPE_ECM(T30Struct *pT30);

// Universal utility function
void T30_UpdateT31Parms(T30Struct *pT30);
void T30_InitT32Parms(DteToT30Interface *pDteToT30);
void T30_InitForPhaseBReEntry(T30Struct *pT30);
void T30_LogHdlc(UBYTE *msg, UWORD len, UBYTE isTransmit, UBYTE isT38);
void T30_GotoPointB(T30Struct *pT30, UBYTE hangupCode);
void T30_GotoPointC(T30Struct *pT30, UBYTE hangupCode);

// Line counting
void FXL_T6Init(T6Lines_Struct *pT6Lines, UWORD uImageWidth);
void FXL_CountFaxLines(T30Struct *pT30, UBYTE *ch);
int FXL_CountFaxLinesT6(T6Lines_Struct *pT6Lines, UBYTE *pData, UWORD uLen);

#endif

