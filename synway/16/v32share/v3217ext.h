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

#ifndef _V3217EXT_H
#define _V3217EXT_H

#include "dspdext.h"
#include "v3217str.h"

void  V32Share_Dummy(V32ShareStruct *pV32Share);
void  V32_RxTrainScramUsingGPC(V32ShareStruct *pV32Share);
void  V32_CarrierUpdate(V32ShareStruct *pV32Share, CQWORD cPhaseIQ);
void  V32_Rotate_Temp(V32ShareStruct *pV32Share);
void  V32_CarUpdateUseSliceIQ(V32ShareStruct *pV32Share);
void  V32_CarrierRecovery_init(V32ShareStruct *pV32Share);
void  V32_Derotate_Get_TRN(V32ShareStruct *pV32Share);
void  V32_Derotate(V32ShareStruct *pV32Share);
void  V32_TimingUpdate_init(V32ShareStruct *pV32Share);
void  V32_TimingUpdateByTRN2(V32ShareStruct *pV32Share);
void  V32_TimingUpdateBySliceIQ(V32ShareStruct *pV32Share);
void  V32_TimingUpdate(V32ShareStruct *pV32Share, CQWORD cTimeIQ);
void  V32_TimingLoopFilter(V32ShareStruct *pV32Share, QWORD qTimingErr);
void  V32_RxTrainSigMap(V32ShareStruct *pV32Share);
void  V32_EqUpdate(V32ShareStruct *pV32Share);
void  V32_CarUpdateUseTrainIQ(V32ShareStruct *pV32Share);
void  V32_CalSagcScaleEnergy(V32ShareStruct *pV32Share);
void  V32_Sagc(V32ShareStruct *pV32Share, QWORD qAGCRef);
void  V32_DiffDecInit(V32ShareStruct *pV32Share);
void  V32_BypassEq(V32ShareStruct *pV32Share);
void  V32_BypassTrainEncode(V32ShareStruct *pV32Share);
void  V32_Demodulate(V32ShareStruct *pV32Share);
void  V32_Eq_D(V32ShareStruct *pV32Share);
void  V32_BypassDiffDec(V32ShareStruct *pV32Share);
void  V32_DescramUsingGPC(V32ShareStruct *pV32Share);
void  V32_DescramMask_Init(V32ShareStruct *pV32Share);
void  V32_Slice(V32ShareStruct *pV32Share);
void  V32_DiffDec(V32ShareStruct *pV32Share);
void  V32_ScramUsingGPC(V32ShareStruct *pV32Share);
void  V32_DiffEnc(V32ShareStruct *pV32Share);
void  V32_TrellisEncode(V32ShareStruct *pV32Share);
void  V32_ScramMaskInit(V32ShareStruct *pV32Share);
void  V32_BypassScram(V32ShareStruct *pV32Share);
void  V32_BypassTrellisEncoder(V32ShareStruct *pV32Share);
void  V32_SigMapResetData(V32ShareStruct *pV32Share);
void  V32_SigMap(V32ShareStruct *pV32Share);
void  V32_ByPassDiffEnc(V32ShareStruct *pV32Share);
void  V32_RX_Decode_init(V32ShareStruct *pV32Share);
void  V32_RX_Decode128p(V32ShareStruct *pV32Share);
void  V32_DiffEncInit(V32ShareStruct *pV32Share);
void  V32_EncodeInit(V32ShareStruct *pV32Share);
void  V32_PsfInit(UBYTE **pTable, V32ShareStruct *pV32Share);
void  V32_Psf(V32ShareStruct *pV32Share);
void  V32_Mod(V32ShareStruct *pV32Share);
void  V32_DemodInit(V32ShareStruct *pV32Share);
void  V32_SliceInit(V32ShareStruct *pV32Share);

extern CONST UBYTE ubV32_DiffDecodeTab_T[];
extern CONST UBYTE ubV32_DiffDecodeTab_NR[16];
extern CONST CQWORD cV32_IQTab_14400T[];
extern CONST SWORD nV32_IQRotateTab_14400T[];
extern CONST UBYTE ubV32_IQtoBitsTab_14400T[];
extern CONST SWORD nV32_IQRotateTab_4800NR[];
extern CONST UBYTE ubV32_IQtoBitsTab_4800NR[];
extern CONST UBYTE ubV32_VitPathToStateTab[];
extern CONST CQWORD cV32_IQTab_12000T[];
extern CONST SWORD nV32_IQRotateTab_12000T[];
extern CONST UBYTE ubV32_IQtoBitsTab_12000T[];
extern CONST CQWORD cV32_IQTab_9600T[];
extern CONST SWORD nV32_IQRotateTab_9600T[];
extern CONST UBYTE ubV32_IQtoBitsTab_9600T[];
extern CONST CQWORD cV32_IQTab_7200T[];
extern CONST SWORD nV32_IQRotateTab_7200T[];
extern CONST UBYTE ubV32_IQtoBitsTab_7200T[];
extern CONST UBYTE V32_DiffEncTab_T[];
extern CONST UBYTE V32_DiffEncTab_NR[16];
extern CONST UBYTE ubBitReversalTab_6Bits[64];
extern CONST CQWORD cV32_IQTab_4800NR[];
extern CONST SWORD nV32_HilbCoefTab[];
extern CONST UBYTE ubV32ScramMaskTab[10];
extern CONST UBYTE ubV32_VitStateToPathTab[];
extern CONST UBYTE V32_TrellisEncTab_T[];

#endif
