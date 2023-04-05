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

#ifndef _DSPDEXT_H
#define _DSPDEXT_H

#include "dspdstru.h"

/* The "D" after the "Dsp" stands for fix point function. */
QDWORD DSPD_Atan2(QWORD axialX, QWORD axialY);

/* Normal cfir filtering function */
void DspcFirInit(QCFIRStruct *, CQWORD *, CQWORD *, UWORD);
CQWORD DspcFir(QCFIRStruct *, CQWORD *);
/* 2T/3 Equalizer cFir Filtering */
void DspcFir_2T3EQInit(QCFIRStruct *pcFir, CQWORD *pcCoef, CQWORD *pcDline, UWORD len);
CQWORD DspcFir_2T3EQ(QCFIRStruct *, CQWORD *);

/* cifir filtering function; interpolating factor set to 3 (for echo canceller) */
/* In fixed point mode: 32 bit coef with 64 bit accumulator */
void DspI3cFir_EC_16_32(CQDWORD *, UWORD pcCoef_Len, CQWORD *, QWORD *);
void DspI3cFir_EC_16_32_Update(CQDWORD *pcCoef, UWORD pcCoef_Len, CQWORD *pcAdapt, QWORD *qEchoCancel_Out);
void DspI3cFir_Adapt_EC_16_32(CQDWORD *pcCoef, CQWORD *pcCoef16, CQDWORD *pcCoefEnd, CQWORD *pcDline, CQWORD *pcAdapt, QWORD *qEchoCancel_Out, QWORD *out);

/* Normal fir filtering function */
void  DspFirInit(QFIRStruct *, QWORD *, QWORD *, UBYTE);
QWORD DspFir(QFIRStruct *, QWORD);
QWORD DspDeciFir(QFIRStruct *pFir, QWORD *pinsample);
/* Hilbert Fir */
QWORD DspFir_Hilbert(QFIRStruct *, QWORD);

/* Normal ifir filtering function with rearranged coefficient */
void DspiFirInit(QIFIRStruct *, QWORD *, QWORD *, UBYTE, UBYTE);
void DspiFir(QIFIRStruct *, QWORD, QWORD *);
void Dspi3Fir_PsfInit(QIFIRStruct *, QWORD *, QWORD *, UBYTE, UBYTE);
void Dspi3Fir_Psf(QIFIRStruct *pIFir, QWORD, QWORD *);

/* Complex LMS adaptation */
void DspcFirLMS(QCFIRStruct *, QWORD, CQWORD *);
/* Complex LMS adaptation used for 2T3 Equalizer */
void DspcFirLMS_2T3(QCFIRStruct *, QWORD, CQWORD *);

/* Goertzel Algorithm, Detect Dft Magnitude only */
void   DSPD_DFT_Init(DftStruct *, SWORD, UBYTE, SWORD *, SDWORD *, SWORD *, UBYTE);
QDWORD DSPD_DFT_Detect(DftStruct *, SWORD *, SWORD);

void   DSPD_DetectToneInit(DspdDetectToneStruct *, SWORD, UBYTE, UWORD *, QWORD *, QDWORD *, QDWORD *, QDWORD *, UBYTE);
QDWORD DSPD_DetectTone(DspdDetectToneStruct *, QWORD *, UWORD);

/* Pure DFT on complex input: Detect Magnitude and Phase */
void DSPD_DFTPure_Init(QDFTPUREStruct *, SWORD, SWORD, UBYTE);
UBYTE DSPD_DFTPure_Detect(QDFTPUREStruct *, UWORD, CQWORD *);

/* Single precision Sin function */
QWORD DspDSinf(QWORD qPhase);

QWORD qDiva(QWORD a, QWORD b);/* div for QWORD */

/* div for QDWORD */
QDWORD QDQDdivQD(QDWORD qdDividend, QDWORD qDivisor);
QDWORD QDQDdivQDRem(QDWORD qdDividend, QDWORD qdDivisor, QDWORD *qdRemainder);

QWORD QDsqrt(UDWORD qX);/* sqrt for QDWORD */

QDWORD  DSPD_Log10(QDWORD qX);/* log10 for QDWORD */

void Number100_10_1(UWORD Num, UBYTE UnitsPlace[]);

void DspIIR_Cas51_Init(IIR_Cas5Struct *, SWORD *, SWORD *, UBYTE);
SWORD DspIIR_Cas51(IIR_Cas5Struct *, SWORD);

/* _CRC_ */
void UpdateCRC_B(UWORD *crc, UBYTE data);
void UpdateCRC_O(UWORD *crc, UBYTE data);
void CRC16_Gen(UWORD *crc, UBYTE);
void CRC16_nBits(UWORD *crc, UWORD, UBYTE);

#if WITH_DC
void Init_DC(DCStruct *pDC);
void DcEstimator(DCStruct *pDC, SWORD *pPCM, UBYTE BufSize, SBYTE shift);
#endif

CQWORD Timing_Interpolation(CQWORD *pcFdptr, QWORD *pqFcoef_ptr, UBYTE Timing_Taps);
void PolyFilter(PolyFilterStruct *pPoly);
void RX_Timing_init(PolyFilterStruct *pPoly);
void RX_TimingRecovery(PolyFilterStruct *pPoly, UBYTE ubSym_size, UBYTE ubTimingCnt_Len);

void SinCos_Lookup_Fine(UWORD Phase, QWORD *qSin, QWORD *qCos);
void Phase_Update(UWORD *Phase, UWORD DeltaPhase);
void SinCos_Lookup(UWORD *phase, QWORD delta_phase, QWORD *SinOut, QWORD *CosOut);
void Sin_Lookup_Fine(UWORD *Phase, UWORD DeltaPhase, QWORD *qSin);

SDWORD Tim_drift_calc(QDWORD *all_angles, UBYTE Angle_num);

extern CONST QWORD DSP_tCOS_TAB4[4];
extern CONST QWORD DSP_tCOS_TAB12[12];
extern CONST QWORD DSP_tCOS_TABLE16[16];

/* Cos table 256 points */
extern CONST SWORD DSP_tCOS_TABLE[256];
extern CONST QWORD DSP_tCOS_OFFSET[256];

extern CONST SDWORD DSP_tATAN_TABLE10[64];
extern CONST QWORD DSP_tATAN_OFFSET10[64];

extern CONST QWORD tHilbert_Coef[];
extern CONST QWORD Dspi3Fir_Psf_coef[];
extern CONST QWORD PolyPhaseFiltCoef8[];
extern CONST QWORD PolyPhaseFiltCoef10[];
extern CONST QWORD PolyPhaseFiltCoef14[];
#endif
