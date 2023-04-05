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

#if defined (_T30STRU_H)
/* Reentry! */
#else
#define _T30STRU_H

#include "hdlcstru.h"
#include "t30def.h"
#if !SUPPORT_ECM_MODE
#include "circbuff.h"
#endif
#include "faxapi.h"

typedef struct
{
    UWORD                       udLinesCount;    /* number of found lines */
    UBYTE                       ubConsecEOLs;    /* consecutive EOL counter */
    UBYTE                       ubRightZeros;    /* number of consecutive zero bits left from the previous data */
} FXL_Struct;

typedef struct
{
    UDWORD                      udBitBuf;        /* current i/o byte/word */
    int                         uBitNum;            /* current i/o bit in byte */
    UWORD                       rowpixels;        /* pixels in a scanline */

    UWORD                       *pRefRuns;        /* runs for the reference line */
    UWORD                       *pCurRuns;        /* runs for the current line */

    int                         a0;
    int                         b1;
    int                         RunLength;
    UWORD                       *pa;
    UWORD                       *pb;
    int                         State;

    UWORD                       udLinesCount;            /* number of found lines */
    UWORD                       RunsBuf[2 *(14592 + 2)];     /* runs for current and previous rows */
} T6Lines_Struct;

#if SUPPORT_ECM_MODE
typedef struct
{
#if !PC_ECM
    UBYTE                       ubECM_Block[BLOCKSIZE];/* 256*256 bytes. too much??? */
#endif
    UBYTE                       *pblock;
    UDWORD                      udImageDataByteNum;
    UBYTE                       status[BLOCKNUM_BYTESIZE];

    UWORD                       leftover_len;
    UBYTE                       leftover_buf[LEFTOVER_BUFSIZE];
    UBYTE                       frmSizeBitNum; // fixing issue 161

    UBYTE                       FCF1;
    UBYTE                       FCF2;
    UBYTE                       pgNum;
    UBYTE                       blkNum;
    UBYTE                       frmNumInBlock;
    SWORD                       frmNumUploaded;
    UWORD                       frmNumInPP;
    UBYTE                       areAllFrmRcved;
    UBYTE                       isBlkSentToDte;
    UBYTE                       isPgSentToDte;

    UBYTE                       isRcpExchanged;

    UBYTE                       PPR_count;
    UBYTE                       retryCount;

    UBYTE                       RNR_flag;
    UBYTE                       *pLast_EOL;
    UBYTE                       *pRead;
    UBYTE                       *pWrite;
} T4ECMstruc;
#endif

typedef struct _T30_struct      T30Struct;
typedef void (*T30Fnptr)(T30Struct *);

struct _T30_struct
{
#if SUPPORT_ECM_MODE
    T4ECMstruc                  T4ECM; // ECM or T.4 manipulating structure
#else
CircBuffer                  T30_TxCB; // T.4 buffer
UBYTE                       T30_TxBuf[BUFSIZE];     /* buffer interface with modem. */
#endif
    UBYTE                       isCall; // Calling or answering terminal
    UBYTE                       isTransmit; // Transmitting or receiving terminal
    UBYTE                       isMenuSent;
    DteToT30Interface           *pDteToT30; // Point to interface
    T30ToDteInterface           *pT30ToDte; // Interface
    T30ToDceInterface           *pT30ToDce; // Interface
    DceToT30Interface           *pDceToT30; // Pointer to interface
    UBYTE                       isRemoteComptRec; // Remote fax compatible?
    UBYTE                       remoteFAPCap; // Remote FAP capabilities
    UBYTE                       txStage; // HDLC or T.4 TX stage
    UBYTE                       frmStage; // opt1, opt2, opt3, ..., last
    UBYTE                       rxStage; // HDLC or T.4 RX stage
    UBYTE                       wasDisRcved; // Need to change the FCF first bit?
    UBYTE                       remoteV8Menu[4]; // Remote CM/JM
    T30Fnptr                    pfT30Vec; // State machine functions
    HdlcStruct                  T30HDLC; // HS HDLC
    UBYTE                       hdlcRxStatus; // HDLC RX state machine status
    UBYTE                       isDcsAsCmd; // Nature of DCS
    UWORD                       hdlcTypeTx; // HDLC TXed
    UWORD                       hdlcTypeRx; // HDLC RXed
    UWORD                       msltBitNum; // MSLT requirement
    UWORD                       msltAccumBitNum; // MSLT accumulated so far
    UBYTE                       pgVRforWD; // VR used to calculate WD
    UBYTE                       wasEOM; // EOM was received in the session?
    UBYTE                       wasLastTry; // Perform orderly disconnect?
    UBYTE                       isFallBackTraining; // Need to drop bit rate?
    UWORD                       TCFByteNum; // 10ms TX capability
    UWORD                       TCFResult; // TCF result
    UWORD                       Timer_T1_Counter; // T1 timer
    UWORD                       Timer_T2_Counter; // T2 timer
    UWORD                       Timer_T3_Counter; // T3 timer, not used
    UWORD                       Timer_T4_Counter; // T4 timer
    UWORD                       Timer_T5_Counter; // T5 timer
    UWORD                       Timer_3s_Counter; // 3s timer
    UWORD                       Timer_TCF; // TCF RX timer
    UBYTE                       HDLCLen; // HDLC RXed length
    UBYTE                       HDLC_TX_Buf[T30FRAMELEN]; // HDLC to TX
    UBYTE                       HDLC_RX_Buf[T30FRAMELEN]; // HDLC RXed
    UBYTE                       T30Speed; // Modulation and speed
    UBYTE                       isHdlcMade; // HDLC made?
    UBYTE                       WasTimeout; // RSP: T4 timeout; CMD: NO CARRIER before any HDLC data
    UBYTE                       rxErrorTimes; // HDLC RXed failure counts
    UBYTE                       rxDisDtcTimes; // DIS/DTC RXed counts
    UBYTE                       isFpsModified; // DTE changed FPS?
    UWORD                       LineCount; // T.4 line count
    UWORD                       ZeroCount_EOL; // 0 count
    UBYTE                       EOLCount; // EOL count
    UBYTE                       ImageState; // Image line-counting stage
    FXL_Struct                  FaxLineStru; // Structure for T.4 line count
    T6Lines_Struct              T6LineStru; // Structure for T.6 line count
#if !SUPPORT_DCE
    UBYTE                       InitStep;
#endif
};

#endif
