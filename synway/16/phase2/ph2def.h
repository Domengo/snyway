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

#ifndef _PH2DEF_H
#define _PH2DEF_H

#include "commdef.h"
#include "modifdef.h"

#define PH2_SYM_SIZE                     (16)

#if SUPPORT_V90D
#define V34_OPTIONAL                 (0)
#else
#define V34_OPTIONAL                 (1)
#endif

#if SUPPORT_V92A
#define SUPPORT_V92A_MOH             (1)
#else
#define SUPPORT_V92A_MOH             (0) /* fixed */
#endif

#if SUPPORT_V92D
#define SUPPORT_V92D_MOH             (1)
#else
#define SUPPORT_V92D_MOH             (0) /* fixed */
#endif

#define V34_PHASE2_10MS                  (6)  /* Number of symbols in  10ms at 9.6kHz */
#define V34_PHASE2_40MS                  (24) /* Number of symbols in  40ms at 9.6kHz */
#define V34_PHASE2_50MS                  (30) /* Number of symbols in  50ms at 9.6kHz */
#define V34_PHASE2_72MS                  (43) /* 72ms: According to ITU-T of V90 or V34, the silence of between V8 and phase2 must be 75+/-5ms, but the silence length of phase2 is 72ms, the silence length of v8 is 0~6.666ms. */
#define V34_PHASE2_160MS                 (96) /* Number of symbols in 160ms at 9.6kHz */
#define V34_PHASE2_300MS                 (180)/* Number of symbols in 300ms at 9.6kHz */
#define V34_PHASE2_400MS                 (240)/* Number of symbols in 400ms at 9.6kHz */
#define V34_PHASE2_500MS                 (300)/* Number of symbols in 500ms at 9.6kHz */
#define V34_PHASE2_550MS                 (330)/* Number of symbols in 550ms at 9.6kHz */
#define V34_PHASE2_650MS                 (390)/* Number of symbols in 650ms at 9.6kHz */

#define V34_PROBING_TONES                (21)
#define V34_REPETITION_RATE_INDEX        (64)
#define V34_MAX_TONE_SIZE                (21) /* max_tone_size is the total number of different */
/* frequency of the signal output */

#define V34_FILTER_LENGTH_1              (64)
#define V34_DELAYLINE_LENGTH             (4)  /* V34_FILTER_LENGTH_1 / 16 */
#define V34_DPSK_CALL_LEN                (63) /* (45) */
#define V34_DPSK_ANS_LEN                 (63) /* (45) */

#define V34_INFO_0_SIZE                  (49+4)  /* 49 without V90 */
#if (SUPPORT_V90A + SUPPORT_V90D)
#define V34_INFO_0d_SIZE                 (62+4)  /* 62 with V90 */
#endif
#define V34_DATA_SEQUENCE_0              (50)    /* 37 without V90 */
#define V34_INFO_1A_SIZE                 (70+4)
#define V34_DATA_SEQUENCE_1A             (58)
#define V34_INFO_1C_SIZE                 (109+4)
#define V34_DATA_SEQUENCE_1C             (97)

#if SUPPORT_V34FAX
#define V34_INFO_H_SIZE                  (51+4)
#define V34_DATA_SEQUENCE_H              (39)
#define PH2_B1TONE_ENERGY_REF            (10000)
#endif

#define V34_SYM_2400                     (0)
#define V34_SYM_2743                     (1)
#define V34_SYM_2800                     (2)
#define V34_SYM_3000                     (3)
#define V34_SYM_3200                     (4)
#define V34_SYM_3429                     (5)
#define V90_SYM_8000                     (6)

#define V34_MAX_RETRAIN                  (4)

/* retrain & hangup reason */
#define PH2_NO_SYMBOL_RATE_SUPPORT       (1)
#define PH2_DETECT_INFO1A_TIMEOUT        (2)
#define PH2_DETECT_INFO1C_TIMEOUT        (3)
#define PH2_DETECT_TONEB1_TIMEOUT        (4)
#define PH2_DETECT_INFO1A_CRC_ERROR      (5)
#define PH2_DETECT_INFO1C_CRC_ERROR      (6)
#define PH2_DETECT_RETRAIN_IN_DET_INFO1A (7)
#define PH2_DETECT_RETRAIN_IN_DET_INFO1C (8)

#define PH2_HANGUP_V92MH_CLRD             (2)
#define PH2_RECONNECT_V92MH                 (3)


/* Time out value for V.34 different phase */
#define V34PHASE2_TIMEOUT                 (10000)

#define V34_qCALL_MODEM_CARRIER         (32) /* 1200Hz/9600Fs * 256 cos tbl size */
#define V34_qANS_MODEM_CARRIER          (64) /* 2400hz/9600Fs * 256 */

#define V34_qL1_AMPLITUDE               (2492)
#define V34_qL2_AMPLITUDE               (1246)

/* CONST for sin macro */
#define V34_qTONEA                      (0)  /* Goertzel Coef  = 2 * cos(2*PI*2400Hz/9600Hz) */
#define V34_qTONEAL                     (12)  /* 25.12 2325Hz      * 32768 >> 7  (9.7)format */
#define V34_qTONEAR                     (-12)  /* -25.12 2475Hz */
#define V34_qTONEB1                     (181)  /* 362.04 1200Hz */    /*   180   */
#define V34_qTONEB1L                    (189)  /* 379.37 1125Hz */    /*   188   */
#define V34_qTONEB1R                    (171)  /* 343.84 1275Hz */

#define V34_qDPSK_MAG                   (16384)

/* time out and retrain CONST */
#define V34_INFO_1A_TIME                (420)   /* Number of symbols in 700ms  at 9.6kHz */
#define V34_INFO_1C_TIME                (1200)  /* Number of symbols in 2000ms at 9.6kHz */
#define V34_TONE_A1_PR_TIME             (390)   /* Number of symbols in 650ms  at 9.6kHz */
#define V34_TONEA2_TIME                 (390)   /* Number of symbols in 650ms  at 9.6kHz */
#define V34_TONEB1_TIME                 (390)   /* Number of symbols in 650ms  at 9.6kHz */
#define V34_TIME_B_PR_40MS              (1200)  /* Number of symbols in 2000ms at 9.6kHz */

#define V34_TONE_ENERGY                 ((QDWORD)100<<11)
#define V34_TONE_ENERGY1                ((QDWORD)70<<11)

#define PH2_RTD_CONST                   (427)  /* 256*10/6: pDpcs->RTD in msec, pPH2->RTD in 1/600 sec */
#define PH2_CONST1                      (150)
#define PH2_CONST2                      (868)
#define PH2_AGC_REFERENCE_A             (1032)
#define PH2_AGC_REFERENCE_B             (1558)

#define lo1                              (0)
#define lo2                              (1)
#define hi1                              (2)
#define hi2                              (3)

#define PH2_ANGLE_NUM                    (57)   /* number of detected and saved angles during L2 */

#define V90A_FALLBACK_V34                (1)
#define V90A_RETRAIN                     (2)

#define ONE_NUM                          (45)
#define PH2_THRESHOLD1                   ((SWORD)50)
#define PH2_THRESHOLD2                   ((SWORD)10)

#if SUPPORT_V34FAX
#define PH2_THRESHOLD3                   ((SWORD)100)
#endif

#define PH2_THRESHOLD4                   ((SWORD)3)

#define LOWEST_THD                       ((QDWORD)(-65534))

#if (SUPPORT_V92A_MOH + SUPPORT_V92D)
#define MH_SEQUENCE_DETECTED         (2)
#define MH_REQ                         (0x0C)
#define MH_ACK                         (0x0A)
#define MH_NACK                         (0x0E)
#define MH_CLRD                         (0x09)
#define MH_CDA                         (0x0D)
#define MH_FRR                         (0x0B)
#define MH_SEQUENCE_SIZE             (40)
#endif

#define PH2_NEW_LOWPASS_FILT_COEF        (0)

#if PH2_NEW_LOWPASS_FILT_COEF
#define P2FIR_GAIN                   (7)/* gain of filter */
#endif

#endif
