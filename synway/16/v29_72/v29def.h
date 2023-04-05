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

#ifndef _V29DEF_H
#define _V29DEF_H

#include "commdef.h"
#include "dspddef.h"
#include "faxshmem.h"

#define V29_MODEM_ENABLE               (0) /* 0: disable; 1: enable */

#define V29_BUF_SIZE                   (72)   /* Trans. Buffer size */
#define V29_SYM_SIZE                   (3)    /* Samples per symbol */
#define V29_NUM_SYM                    (V29_BUF_SIZE/V29_SYM_SIZE)   /* Number of symbols in buffer */

#if V29_MODEM_ENABLE
#define V29_4800T                      (2)
#define V29_BITS_PER_SYMBOL_4800       (2)
#define V29_MAPPER_4800                (1968)
#endif

#define V29_9600T                      (0)    /* defines different speeds */
#define V29_7200T                      (1)

#define V29_BITS_PER_SYMBOL_9600       (4)
#define V29_BITS_PER_SYMBOL_7200       (3)

#define V29_MAPPER_9600                (1607)
#define V29_MAPPER_7200                (2517)

#define V29_CARRIER_1700_DELTA_PHASE   (15474)   /* 11605: for 9600 sampling rate */

#define V29_TX_FSM_SIZE                (6)
#define V29_RX_FSM_SIZE                (7)

/* Hand Shaking */
#define V29_MAG0                       (6100)    /* TEP magnitude */
#define V29_T_TEP_456_SI               (456)     /* 190ms */
#define V29_T_SEG1_48_SI               (48)
#define V29_T_SEG2_128_SI              (128)               /* Transmit segment2 128 */
#define V29_SEG3_384_SI                (384)
#define V29_SEG4_48_SI                 (48)
#define V29_GET_SEGMENT1_DELAY         (9999)
#define V29_GET_SEGMENT2_DELAY         (9999)

#define V29_T_DATA_TIME_10000          (10000)
#define V29_R_DATA_TIME_10000          (10000)

/* Eqalizer */
#define V29_EQ_DELAY                   (18)
#define V29_EQ_LENGTH                  (36)
#define V29_EQ_TRAIN_BETA              (3000)
#define V29_EQ_ADAPT_BETA              (1857)

/* Timing Reconstruction */
#define V29_TIMING_DELAY_HALF          (TIMING_TAPS8 + V29_SYM_SIZE - 1)

#define V29_TIME_FAI_STEP              (32)

#define V29_CORR_NUM_SYMBOLS           (5)
#define V29_CORR_NUM_SAMPLES           (V29_CORR_NUM_SYMBOLS * V29_SYM_SIZE)

#define V29_AGC_REF                    (8192)/* AGC Ref level */

#define V29_TONE_NUMBER                (4)
#define V29_TONE_500HZ                 (0)
#define V29_TONE_2900HZ                (1)
#define V29_TONE_1700HZ                (2)
#define V29_V21TONE_1650HZ             (3)

#define V29_TCF_TRAIN                  (0)
#define V29_IMAGE_DATA                 (2)

#define V29_AGC_CALCULATION_POINT      (64)

#define V29_ENERGY_REF                 (204800)
#define V29_SILENCE_LEN_REF            (1)

#endif
