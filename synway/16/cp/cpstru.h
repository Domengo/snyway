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

#ifndef _CPSTRU_H
#define _CPSTRU_H

#include "dspdstru.h"
#include "acestru.h"
#include "cpdef.h"

/* maximum 4 cadence in one tone */
/* on - off is one cadence         */
typedef struct
{
    UWORD nTone;
    UBYTE ubCadenceNum;
    SWORD nCadenceOn[CADENCEONNUMBER];
    SWORD nCadenOnErr[CADENCEONNUMBER];
    SWORD nCadenceOff[CADENCEONNUMBER];
    SWORD nCadenOffErr[CADENCEONNUMBER];
} CaStruct;

typedef struct
{
    UWORD LowPhase;
    UWORD HighPhase;
    UWORD LowFreq;
    UWORD HighFreq;

    SWORD OffTime;
    SWORD OnTime;
    UBYTE bGen_Flag;

    SBYTE sbLevelRef_dB;
    QWORD qHighLevelRef;
    QWORD qLowLevelRef;

#if DTMF_GEN_INDEPENDENT
    UBYTE bOn;
#endif
} DtmfGenStruct;

typedef struct _cp_struct CpStruct;
typedef void (*CpFnptr)(CpStruct *);

struct _cp_struct
{
    UBYTE **pTable;

    SWORD *PCMinPtr;
    SWORD *PCMoutPtr;

    /* TX Vector and Finite-State-Machine */
    CpFnptr pfTxVecptr[CP_TX_FSM_SIZE];  /* FSM for 'TxVec'                 */
    SWORD pTxDelayptr[CP_TX_FSM_SIZE];   /* FSM for 'TxVec' delay counter   */
    SWORD TxVecIdx;                      /* index for FSM                   */
    SWORD TxDelayCnt;                    /* delay counter for 'TxVec'       */

    UBYTE modem_mode;

    /* transmit vectors (pointer to functions)   */
    CpFnptr pfModVec;         /* modulation     */
    CpFnptr pfTxStateVec;     /* transmit state */

    /* start to dial */
    UBYTE Dial_Flag;
    UBYTE Disconnect_flag;

    /* -------------- TX variables -------------- */

    /* AT command string */
    UBYTE *pCmdString;
    SWORD  CmdStringIdx;

    /* TONE generation */
    UBYTE GenTone_Flag;
    UWORD GenToneFreq;
    UWORD GenTonePhase;

    DtmfGenStruct Dtmf;

    SWORD DigitOffTime;
    SWORD nPulseMakeTime;
    SWORD nPulseBreakTime;
    SWORD PulseCount;

    UBYTE TxOffOn;
    SWORD TxOffTime;
    SWORD TxOnTime;
    SWORD TxBrkTime;

    UWORD LastDetTones;
    UWORD DetTones;

    DftStruct Dft;
    QDWORD qdMagSq[TONE_MAX_FREQ];
    QWORD  qQ1Q2[TONE_Q1Q2SIZE];

    SBYTE  V8Mode;
    UWORD  V25PhaseChange;
    SWORD  V25PhaseChangeCounter;

    UBYTE  TxEnd;

    UBYTE AnsCount;
    UBYTE BellAnsCount;

    /*  multi cadence detection */
    UWORD nTone_First;            /* (one to three) tones detected in previous cycles */
    UWORD nTone;                  /* (one to two) tones confirmed */
    UBYTE ubHistoryIndex;
    SWORD nHistoryCadenceOn[16];
    SWORD nHistoryCadenceOff[16];

    CaStruct DialToneReference[GROUPTONENUMBER];
    CaStruct BusyToneReference[GROUPTONENUMBER];
    CaStruct CongestionToneReference[GROUPTONENUMBER];
    CaStruct RingBackToneReference[GROUPTONENUMBER];

    SWORD   nCurrCadence;

    SWORD   nVadTotalEgy;
    SWORD   nVadTotalEgy1;

    UBYTE   ubDialToneDetected;
    UBYTE   ubBusyToneDetected;
#if SUPPORT_AUTO
    UBYTE   ubBusyToneFreq;
#endif
    UBYTE   ubCongToneDetected;
    UBYTE   ubRingToneDetected;

    UBYTE   ubDetected;
    UBYTE     ubLastVadDet;
    UBYTE     ubCountry;

#if SUPPORT_FAX

    QDWORD  qdEnergy;
    UBYTE   Fax_Callmode;
    UBYTE   V21ToneCount;

#if SUPPORT_FAX_SEND_CNG
    UBYTE   CNGtone_on;
    UBYTE   RemoteAnsToneDetected; // used to fix issue 129
#endif

#endif
};

#endif
