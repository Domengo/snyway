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

#ifndef  _V27DEF_H
#define _V27DEF_H

#include "commdef.h"
#include "dspddef.h"
#include "faxshmem.h"

#define V27_SHORT_TRAIN_SUPPORT       (0)

#define V27_CARRIER_BETA              (q005)

#define V27_4800                      (1)    /* defines different speeds */
#define V27_2400                      (0)

#define V27_FILTER_LENGTH             (48)

#define V27_BUF_SIZE                  (96)   /* Trans. Buffer size */

#define V27_SYM_SIZE_1600             (6)  /* Samples per symbol for 1600 */
#define V27_NUM_SYM_1600              (V27_BUF_SIZE/V27_SYM_SIZE_1600) /* Number of symbols for 1600 in buffer */
#define V27_PSF_DLINE_LEN_1600        (V27_FILTER_LENGTH/V27_SYM_SIZE_1600)

#define V27_SYM_SIZE_1200             (8)  /* Samples per symbol for 1200 */
#define V27_NUM_SYM_1200              (V27_BUF_SIZE/V27_SYM_SIZE_1200) /* Number of symbols for 1200 in buffer */
#define V27_PSF_DLINE_LEN_1200        (V27_FILTER_LENGTH/V27_SYM_SIZE_1200)

#define V27_CORR_NUM_SYMBOLS          (2) /* ( EPR 405, changed from 5, when use 5, we will have non trivial values for AB tone signals) */
#define V27_CORR_NUM_SAMPLES_1600     (V27_CORR_NUM_SYMBOLS * V27_SYM_SIZE_1600)
#define V27_CORR_NUM_SAMPLES_1200     (V27_CORR_NUM_SYMBOLS * V27_SYM_SIZE_1200)

#define V27_MAG0                      (6100)
#define V27_MAG45                     (4314)

#define V27_CARRIER_1800_DELTA_PHASE  (48) /* (1800/9600)*256 , 45 value is for */
/* simulation only.Actual value is 45.333 */

#define V27_TX_FSM_SIZE               (6)
#define V27_RX_FSM_SIZE               (8)

#define V27_1600_T_SEG1_308_SI        (308)
#define V27_1200_T_SEG1_231_SI        (231)
#define V27_R_SEG1_308_SI             (300) /* 308 */

#define V27_1600_T_SEG2_36_SI         (36)
#define V27_1200_T_SEG2_27_SI         (27)

#define V27_R_SEG2_36_SI              (500) /* 36 */
#define V27_T_SEG3_50_SI              (50)
#define V27_R_SEG3_50_SI              (1000)
#define V27_T_SEG4_1074_SI            (1074)
#define V27_1600_R1_SEG4_537_SI       (521)/* (522) Must 537 - delay-time of dteect (V27_CORR_NUM_SAMPLES_1600 / 2) - 1 */
#define V27_1200_R1_SEG4_537_SI       (516)/* (517) Must 537 - delay-time of dteect (V27_CORR_NUM_SAMPLES_1200 / 2) - 1 */
#define V27_R2_SEG4_537_SI            (537)
#define V27_SEG5_8_SI                 (8)

#define V27_T_SHORT_SEG1_308_SI       (308)
#define V27_T_SHORT_SEG2_36_SI        (36)
#define V27_R_SHORT_SEG2_36_SI        (36)
#define V27_T_SHORT_SEG3_14_SI        (14)
#define V27_R_SHORT_SEG3_14_SI        (30)
#define V27_T_SHORT_SEG4_58_SI        (58)
#define V27_R2_SHORT_SEG4_58_SI       (58)
#define V27_SEG5_SHORT_8_SI           (8)
#define V27_T_DATA_TIME_2000          (11017)          /* transmit data time 2000 */
#define V27_R_DATA_TIME_1995          (11017)          /* receiver data time 1995 */

/* for Equalizer */
#define V27_EQ_DELAY                  (7)  /* EQ delay */
#define V27_EQ_LENGTH                 (30) /* 24 */
#define V27_EQ_ADAPT_BETA             (q0075)
#define V27_EQ_TRN1raw_BETA           (9000)    /* 32767 * 0.25  */
#define V27_EQ_TRN2fine_BETA          (q005)
#define V27_EQ_SHORT_BETA              (9000)

/* For Timing Reconstruction */
#define V27_TIMING_DELAY_HALF         (TIMING_TAPS8 + V27_SYM_SIZE_1200 - 1)
#define V27_TIME_FAI_STEP             (64)

#define V27_SNR_LIMIT                 (16384)   /* 5000 */

#define V27_R_DATA_TIME_10000         (10000)

/* SAGC */
#define V27_SAGC_BETA                 (256)

#define V27_GUARD_PATTERN             (0x0980)
#define V27_TIME_PERIOD_MAX           (1024)

#define V27_TONE_NUMBER               (4)
#define V27_TONE_LOW                  (0) /* for baud rate 1600: 1000Hz; for baud rate 1200: 1200Hz */
#define V27_TONE_HIGH                 (1) /* for baud rate 1600: 2600Hz; for baud rate 1200: 2400Hz */
#define V27_TONE_CARRIER              (2) /* carrier: 1800Hz */
#define V27_V21TONE_1650HZ            (3)

#define V27_CORR_THRESHOLD            (30000) /* changed from 100000, when use 100000, detect the */
/* second phase reversal other than the first, */

#define V27_TCF_TRAIN                 (0)
#define V27_IMAGE_DATA                (2)

#if !V27_HW7200SAMPLERATE_SUPPORT
/* Because sample rate change from 7200 to 9600, */
/* the start time (32000--V27_7200CHANGE9600_CNT) don't count */
#define V27_7200CHANGE9600_CNT        (31950)
#endif

#define V27_AGC_CALCULATION_POINT     (40)
#define V27_ENERGY_REF                (409600)

#define V27_SILENCE_LEN_REF           (1)

#endif
