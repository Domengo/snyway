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

#ifndef _ACEEXT_H
#define _ACEEXT_H

#include "acestru.h"
#include "dspdext.h"
#include "ptable.h"
#include "circbuff.h"
#include "sregdef.h"

void ACEInit(UBYTE **pTable);
void ACEMain(UBYTE **pTable);

UBYTE Ace_ATA(UBYTE **pTable);
UBYTE Ace_ATD(UBYTE **pTable);
UBYTE Ace_ATO(UBYTE **pTable);
UBYTE Ace_ATH(UBYTE **pTable);

UBYTE Ace_FTS(UBYTE **pTable);
UBYTE Ace_FRS(UBYTE **pTable);
UBYTE Ace_FTM(UBYTE **pTable);
UBYTE Ace_FRM(UBYTE **pTable);
UBYTE Ace_FTH(UBYTE **pTable);
UBYTE Ace_FRH(UBYTE **pTable);
UBYTE Ace_A8M(UBYTE **pTable);
UBYTE DLE_PRI(UBYTE **pTable);
UBYTE DLE_CTRL(UBYTE **pTable);
UBYTE DLE_RTNC(UBYTE **pTable);
UBYTE DLE_EOT(UBYTE **pTable);

void Ace_FsmFRS(UBYTE **pTable);
void Ace_FsmFTS(UBYTE **pTable);
void Ace_FsmFRM(UBYTE **pTable);
void Ace_FsmFTM(UBYTE **pTable);
void Ace_FsmFRH(UBYTE **pTable);
void Ace_FsmFTH(UBYTE **pTable);
void Ace_FsmA8E(UBYTE **pTable);

UBYTE Ace_SetupFax(UBYTE **pTable);
void Ace_V34StartMPET(DpcsStruct *pDpcs);
void Ace_V21StartFaxModem(DpcsStruct *pDpcs);
SWORD Ace_TxDataToModem(CircBuffer *pAscCBRd, UBYTE *InBuf, UWORD len);
SWORD Ace_TxDataToModem_0(CircBuffer *pAscCBRd);

void Ace_Fsm(UBYTE **pTable);
void Ace_ChangeState(ACEStruct *pAce, UBYTE newState);
void Ace_SetResultCode(UBYTE **pTable, UBYTE aceResultCode);
void Ace_ChangeLineState(ACEStruct *pAce, UBYTE newLineState);
void Ace_GetClass1Command(UBYTE **pTable);
void Ace_SetClass1ResultCode(ACEStruct *, UBYTE);

void ACE_FactoryConfigurationInit(UBYTE **pTable);
void MhostConnect(UBYTE **pTable);

extern CONST UBYTE SRegDefVal[NUMBER_S_REGISTERS];
extern CONST CHAR *Class_State[];
extern CONST CHAR *Line_State[];

UBYTE Ace_ParseUnsupportedParm(UBYTE **pTable);
void Ace_ReportFlowControl(UBYTE **pTable);
UBYTE Ace_SetReportFlowControl(UBYTE **pTable);
void Ace_ReportSerialPortRate(UBYTE **pTable, UBYTE format);
UBYTE Ace_SetReportSerialPortRate(UBYTE **pTable, UBYTE format);
void AceSetResponse(UBYTE **pTable, UBYTE Number);
void Ace_SetT31Command(ACEStruct *pAce, UBYTE command);
void Ace_SetT32Command(ACEStruct *pAce, UBYTE command);

UBYTE AT_A(UBYTE **pTable);
UBYTE AT_D(UBYTE **pTable);
UBYTE AT_T(UBYTE **pTable);
UBYTE AT_P(UBYTE **pTable);
UBYTE AT_S(UBYTE **pTable);
UBYTE AT_and(UBYTE **pTable);
UBYTE AT_H(UBYTE **pTable);
UBYTE AT_Z(UBYTE **pTable);
UBYTE AT_O(UBYTE **pTable);
UBYTE AT_I(UBYTE **pTable);
UBYTE AT_E(UBYTE **pTable);
UBYTE AT_Q(UBYTE **pTable);
UBYTE AT_V(UBYTE **pTable);
UBYTE AT_X(UBYTE **pTable);
UBYTE AT_L(UBYTE **pTable);
UBYTE AT_M(UBYTE **pTable);

UBYTE AT_plus(UBYTE **);/* Replace AT_CLASS1 */

UBYTE AT_plusA(UBYTE **pTable);
UBYTE AT_plusF(UBYTE **pTable);     /* Separate from old AT_CLASS1() */
UBYTE AT_plusI(UBYTE **pTable);
UBYTE AT_plusM(UBYTE **);
UBYTE AT_plusT(UBYTE **);
UBYTE AT_plusC(UBYTE **);
UBYTE AT_plusD(UBYTE **);
UBYTE AT_plusDS(UBYTE **);
UBYTE AT_plusE(UBYTE **);
UBYTE AT_plusFCLASS(UBYTE **pTable);/* Rename from ACE_FaxClass() */
UBYTE AT_plusFLO(UBYTE **pTable);   /* Rename from CLASS1_FLO() */
UBYTE AT_plusFMI(UBYTE **pTable);
UBYTE AT_plusFMM(UBYTE **pTable);
UBYTE AT_plusFMR(UBYTE **pTable);
UBYTE AT_plusFPR(UBYTE **pTable);

UBYTE CLASS1_F34(UBYTE **pTable);
UBYTE CLASS1_FAR(UBYTE **pTable);
UBYTE CLASS1_FCL(UBYTE **pTable);
UBYTE CLASS1_FIT(UBYTE **pTable);
UBYTE CLASS1_FTS(UBYTE **pTable);
UBYTE CLASS1_FRS(UBYTE **pTable);
UBYTE CLASS1_FTM(UBYTE **pTable);
UBYTE CLASS1_FRM(UBYTE **pTable);
UBYTE CLASS1_FTH(UBYTE **pTable);
UBYTE CLASS1_FRH(UBYTE **pTable);
UBYTE CLASS1_FDD(UBYTE **pTable);

void Ace_ParseDataFromClass1Dte(UBYTE **pTable);
void Ace_ParseDataToClass1Dte(UBYTE **pTable);
void Ace_ParseRespToClass1Dte(UBYTE **pTable);
UBYTE Ace_ParseMod(UBYTE **pTable, UBYTE command);
UBYTE Ace_ParseSilence(UBYTE **pTable, UBYTE command);

// Action
UBYTE CLASS2_FDT(UBYTE **pTable);
UBYTE CLASS2_FDR(UBYTE **pTable);
UBYTE CLASS2_FKS(UBYTE **pTable);
UBYTE CLASS2_FIP(UBYTE **pTable);
// Setting
UBYTE CLASS2_FAA(UBYTE **pTable);
UBYTE CLASS2_FAP(UBYTE **pTable);
UBYTE CLASS2_FBO(UBYTE **pTable);
UBYTE CLASS2_FBS(UBYTE **pTable);
UBYTE CLASS2_FBU(UBYTE **pTable);
UBYTE CLASS2_FCC(UBYTE **pTable);
UBYTE CLASS2_FCQ(UBYTE **pTable);
UBYTE CLASS2_FCR(UBYTE **pTable);
UBYTE CLASS2_FCS(UBYTE **pTable);
UBYTE CLASS2_FCT(UBYTE **pTable);
UBYTE CLASS2_FEA(UBYTE **pTable);
UBYTE CLASS2_FFC(UBYTE **pTable);
UBYTE CLASS2_FFD(UBYTE **pTable);
UBYTE CLASS2_FHS(UBYTE **pTable);
UBYTE CLASS2_FIE(UBYTE **pTable);
UBYTE CLASS2_FIS(UBYTE **pTable);
UBYTE CLASS2_FIT(UBYTE **pTable);
UBYTE CLASS2_FLI(UBYTE **pTable);
UBYTE CLASS2_FLP(UBYTE **pTable);
UBYTE CLASS2_FMS(UBYTE **pTable);
UBYTE CLASS2_FND(UBYTE **pTable);
UBYTE CLASS2_FNR(UBYTE **pTable);
UBYTE CLASS2_FNS(UBYTE **pTable);
UBYTE CLASS2_FPA(UBYTE **pTable);
UBYTE CLASS2_FPI(UBYTE **pTable);
UBYTE CLASS2_FPP(UBYTE **pTable);
UBYTE CLASS2_FPS(UBYTE **pTable);
UBYTE CLASS2_FPW(UBYTE **pTable);
UBYTE CLASS2_FRQ(UBYTE **pTable);
UBYTE CLASS2_FRY(UBYTE **pTable);
UBYTE CLASS2_FSA(UBYTE **pTable);
UBYTE CLASS2_FSP(UBYTE **pTable);

UBYTE Ace_ParseClass2Capability(UBYTE **pTable, UBYTE isFcc);
void Ace_ParseStringToClass2Dte(UBYTE **pTable, UBYTE *pString);
UBYTE Ace_ParseStringFromClass2Dte(UBYTE **pTable, UBYTE *pString, UBYTE len);
void Ace_ParseSubParmToClass2Dte(UBYTE **pTable, T30SessionSubParm *pSubParm);
UBYTE Ace_ParseSubParmFromClass2Dte(ACEStruct *pAce, T30SessionSubParm *pSubParm);
void Ace_ParseDataFromClass2Dte(UBYTE **pTable);
void Ace_ParseDataToClass2Dte(UBYTE **pTable);
void Ace_ParseRespToClass2Dte(UBYTE **pTable);

UBYTE AT_A8E(UBYTE **);
UBYTE AT_A8M(UBYTE **);

UBYTE AT_andF(UBYTE **);
UBYTE AT_andD(UBYTE **);
UBYTE AT_andC(UBYTE **);
UBYTE AT_andH(UBYTE **);
UBYTE AT_andN(UBYTE **);
UBYTE AT_andK(UBYTE **);
UBYTE AT_andT(UBYTE **);
UBYTE AT_andU(UBYTE **);

void AceAt(UBYTE **pTable, UBYTE ch);

extern CONST CHAR RateCode[BIT_RATE_NUM][6];
extern CONST CHAR ModulationCode[12][10];
extern CONST CHAR ProtocolCode[9][13];
extern CONST CHAR AtCommandName_UpCase[];
extern CONST CHAR Class1CommName_UpCase[];
extern CONST CHAR Class1CommClass_UpCase[6];
extern CONST CHAR Class1Mod[];
extern CONST CHAR MI_ResponseCode0[];

#endif
