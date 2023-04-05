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

#ifndef _CPDEF_H
#define _CPDEF_H

#include "commdef.h"

#define CP_7200               (1)
#define CP_8000               (0)
#define CP_9600               (0)

#if SUPPORT_FAX

#define SUPPORT_FAX_SEND_CNG  (1)

#if SUPPORT_FAX_SEND_CNG
#define CP_TX_FSM_SIZE        (7)/* FSM size */
#define TONE_MAX_FREQ         (16)/* num of tones */
#else
#define CP_TX_FSM_SIZE        (5)
#define TONE_MAX_FREQ         (15)
#endif

#else

#define SUPPORT_FAX_SEND_CNG  (0)/* fix */

#define CP_TX_FSM_SIZE        (5)
#define TONE_MAX_FREQ         (14)

#endif

#define TONE_Q1Q2SIZE         (TONE_MAX_FREQ<<1)

#define CP_CALL_RATE          (1)

/* TX, RX Timeouts */
#define TX_TIMEOUT            (32000)
#define RX_TIMEOUT            (32000)

/* end of AT string */
#define END_TOKEN             (0xFF)

/* tone on-time or on-time */
#define TX_OFF                (0)
#define TX_ON                 (1)
#define TX_BREAK              (2)

/* Tone define */
#define DIALTONE              (TONE1_DET)
#define BUSYTONE              (TONE2_DET)
#define RECEIVEROFFHOOKTONE      (TONE3_DET)
#define RINGBACKTONE          (TONE4_DET)
#define CONGESTIONTONE          (TONE8_DET)

#define DET_TONETHRES         (3000)
#define DET_ANSTONETHRES      (511)

#define DET_CED_CNT_THRES        (26)
#define DET_ANSAM_CNT_THRES    (5)

#if CP_7200
#define CP_SAMPLERATE          (7200)
#define CP_HWAPI_SR          (HWAPI_7200SR)
#define CP_ANS_TONE_FREQ    (19115)  /* (2100Hz / 7200Hz * 65536) */
#define CP_BUF_SIZE         (72)
#elif CP_8000
#define CP_SAMPLERATE          (8000)
#define CP_HWAPI_SR          (HWAPI_8000SR)
#define CP_ANS_TONE_FREQ    (17203)  /* (2100Hz / 8000Hz * 65536) */
#define CP_BUF_SIZE         (80)
#elif CP_9600
#define CP_SAMPLERATE          (9600)
#define CP_HWAPI_SR          (HWAPI_9600SR)
#define CP_ANS_TONE_FREQ    (14336)  /* (2100Hz / 9600Hz * 65536) */
#define CP_BUF_SIZE         (96)
#endif

#if SUPPORT_FAX_SEND_CNG
#if CP_7200
#define CP_CNG_TONE_FREQ      (10012)  /* (1100Hz / 7200Hz * 65536) */
#elif CP_8000
#define CP_CNG_TONE_FREQ      (9011)   /* (1100Hz / 8000Hz * 65536) */
#elif CP_9600
#define CP_CNG_TONE_FREQ      (7509)   /* (1100Hz / 9600Hz * 65536) */
#endif
#endif

#if CP_9600
/* DTMF tones for 9600Hz sampling rate */
#define ROW0                  (4758)     /* 697Hz / 9600Hz * 65536 */
#define ROW1                  (5257)     /* 770Hz                 */
#define ROW2                  (5816)     /* 852Hz                 */
#define ROW3                  (6424)     /* 941Hz                 */
#define COL0                  (8253)     /* 1209Hz                */
#define COL1                  (9120)     /* 1336Hz                */
#define COL2                  (10083)    /* 1477Hz                */
#define COL3                  (11148)    /* 1633Hz                */
#elif CP_8000
/* DTMF tones for 8000Hz sampling rate */
#define ROW0                  (5710)     /* 697Hz / 8000Hz * 65536 */
#define ROW1                  (6308)     /* 770Hz                 */
#define ROW2                  (6980)     /* 852Hz                 */
#define ROW3                  (7709)     /* 941Hz                 */
#define COL0                  (9904)     /* 1209Hz                */
#define COL1                  (10945)    /* 1336Hz                */
#define COL2                  (12100)    /* 1477Hz                */
#define COL3                  (13378)    /* 1633Hz                */
#elif CP_7200
/* DTMF tones for 7200Hz sampling rate */
#define ROW0                  (6344)     /* 697Hz / 7200Hz * 65536 */
#define ROW1                  (7009)     /* 770Hz                 */
#define ROW2                  (7755)     /* 852Hz                 */
#define ROW3                  (8565)     /* 941Hz                 */
#define COL0                  (11005)    /* 1209Hz                */
#define COL1                  (12161)    /* 1336Hz                */
#define COL2                  (13444)    /* 1477Hz                */
#define COL3                  (14864)    /* 1633Hz                */
#endif

#define CP_CONST1             (998)/* 39/10*256 = 998 */
#define CP_CONST2             (845)/* 33/10*256 = 845 */
#define CP_CONST3             (499)/* 39/20*256 = 499 */
#define CP_CONST4             (422)/* 33/20*256 = 422 */

#define CADENCEONNUMBER          (4)  /* maximum cadence */
#define GROUPTONENUMBER          (5)  /* tone type */ /* this allow 5 kind of dialtone or busytone or... */

#define TONE350                  (1)     /* 350Hz Dialtone */
#define TONE440                  (1<<1) /* 440Hz Dialtone */
#define TONE480               (1<<2) /* 480  Hz */
#define TONE620                (1<<3) /* 620  Hz */
#define TONE1400               (1<<4) /* 1400 Hz */
#define TONE2260               (1<<5) /* 2260 Hz */
#define TONE2450               (1<<6) /* 2450 Hz */
#define TONE2600               (1<<7) /* 2600 Hz */
#define TONE50                  (1<<8) /* 50   Hz */
#define TONE400               (1<<9) /* 400  Hz */
#define TONE425               (1<<10)/* 425  Hz */
#define TONE450               (1<<11)/* 450  Hz */

#define CP_WINDOW_SIZE          (4)
#define CP_VAD_THRESHOLD      (50)

#define CP_VAD_ON              (1)
#define CP_VAD_OFF              (0)

/* COUNTRIES */
#define CP_CANADA                 (1)
#define CP_CHINA              (2)
#define CP_FRANCE                 (3)
#define CP_GERMANY                 (4)
#define CP_ISRAEL                 (5)
#define CP_KOREA              (6)
#define CP_NETHERLANDS           (7)
#define CP_SPAIN                 (8)
#define CP_UK                     (9)
#define CP_VANUATU            (10)
#define CP_AUSTRALIA          (11)
#define CP_JAPAN              (12)

#define SUPPORT_STANDARD_ETSI (1)
#define SUPPORT_STANDARD_BELL (0)

#if SUPPORT_STANDARD_ETSI
#define DTMF_LEVEL_REF_LOW    (10580)
#define DTMF_LEVEL_REF_HIGH   (13370)
#elif SUPPORT_STANDARD_BELL
#define DTMF_LEVEL_REF_LOW    (13370)
#define DTMF_LEVEL_REF_HIGH   (13370)
#endif

#define DTMF_GEN_INDEPENDENT  (0)

#endif
