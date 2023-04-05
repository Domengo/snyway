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

#ifndef _SHAREEXT_H
#define _SHAREEXT_H

#include "dspdext.h"
#include "sharestr.h"

void   V34_Descram_Byte_GPA(UBYTE *, UDWORD *, UBYTE *, UBYTE);
void   V34_Descram_Bit_GPA(UBYTE *, UDWORD *, UBYTE *, UBYTE);
void   V34_Non_Linear_Encoder(QWORD, CQWORD *);
void   Ph2_Init(UBYTE **);
void   V34_Shell_Mapper_Init(ShellMapStruct *pShell);
void   V34_Shell_Mapper(ShellMapStruct *pShell, SWORD High_Map_Frame, UBYTE mi[4][2]);
#if (TX_PRECODE_ENABLE + RX_PRECODE_ENABLE)
void   V34_Precoding_DataInit(PrecodeStruct *);
void   V34_Precoding_CoefInit(PrecodeStruct *, SWORD *);
void   V34_Precoder(PrecodeStruct *, CQWORD *, SBYTE);
#endif
void   V34_Scram_Bit_GPC(UBYTE, UDWORD *, UBYTE *, UBYTE);
void   V34_Scram_Byte_GPC(UBYTE *, UDWORD *, UBYTE *, UBYTE);
void   V34_Scram_Bit_GPA(UBYTE, UDWORD *, UBYTE *, UBYTE);
void   V34_Scram_Byte_GPA(UBYTE *, UDWORD *, UBYTE *, UBYTE);
void   V34_Calc_g2(UBYTE, UBYTE *);
void   V34_Calc_g4(UBYTE, UBYTE *, UWORD *);
void   V34_Calc_g8(UBYTE, UWORD *, UDWORD *);
void   V34_Calc_z8(UBYTE, UDWORD *, UDWORD *);
void   V34_CircFIR_Filter_Init(CircFilter *fir, QWORD *pqHist, QDWORD *pqCoef, UBYTE coef_len);
void   V34_Modulate_Init(UBYTE **pTable, QAMStruct *pQAM);
#if 0
void   V34_Modulate(V34TxStruct *pV34Tx, CQWORD *, EC_InsertStru *Ec_Insert);
#else
void   V34_Modulate(V34TxStruct *pV34Tx, CQWORD *);
#endif
void   V34_EcRef_Insert(EC_InsertStru *Ec_Insert, CQWORD sym, CQWORD *pcBulk_delay);
void   V34_Encoder(SWORD, SWORD, V34TxStruct *pV34Tx);
SBYTE  V34_Trellis16(UWORD *Q, UBYTE y);
SBYTE  V34_Trellis32(UWORD *Q, UBYTE y);
SBYTE  V34_Trellis64(UWORD *Q, UBYTE y);

SBYTE  V34_Label(CQWORD in);
CQWORD V34_Create_S_or_SBar(UBYTE *);
CQWORD V34_Create_TRN(UBYTE *, UBYTE *, SWORD num_TRN);
CQWORD V34_Create_PP(UBYTE k, UBYTE I);

extern CONST QWORD V34_tTxScaleDnFrom12dBm0[];
extern CONST QWORD V34_tPreempGain[6][11][2];
extern CONST QWORD V34_PsfCoef[];
extern CONST SWORD V34_tRI[416];

#if V34AUXCH
extern CONST UBYTE V34_tMAP_M[6][28][2];
#else
extern CONST UBYTE V34_tMAP_M[6][14][2];
#endif

/* Table of initial modem parameters for all symbol rate and bit rate */
#if V34AUXCH
extern CONST ModemData V34_tINIT_PARA_DATA[6][28];
#else
extern CONST ModemData V34_tINIT_PARA_DATA[6][14];
#endif

extern CONST CQWORD V34_tS_POINTS[4];
extern CONST UBYTE  V34_tSYM2BIT_TBL[8][8];
extern CONST CQWORD V34_tTRAIN_MAP_16[];
extern CONST CQWORD V34_tTRAIN_MAP_4[];
extern CONST QDWORD V34_tBASIC_SCALE[6][14];
extern CONST QWORD  V34_tOFFSET_SCALE[6][14];
extern CONST QWORD  V34_tOFFSET_NL_SCALE0[6][14];
extern CONST QWORD  V34_tOFFSET_NL_SCALE1[6][14];
extern CONST QDWORD V34_tPREEMP_COEF[11][7];
extern CONST SBYTE  V34_Inverse_Map_Label[16];

#endif
