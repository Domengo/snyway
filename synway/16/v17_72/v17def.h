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

#ifndef _V17DEF_H
#define _V17DEF_H

#include "v3217def.h"
#include "faxshmem.h"

#define V17_RATE_IDX_7200         (0)
#define V17_RATE_IDX_9600         (1)
#define V17_RATE_IDX_12000        (2)
#define V17_RATE_IDX_14400        (3)

#define V17_TOTAL_SAMPLE          (6)    /* Band edge is 1200Hz; Sampling Rate is 7200Hz */

#define V17_LONG_TRAIN            (0)
#define V17_SHORT_TRAIN           (2)

/* For Equalizer */
#define V17_EQ_TRAIN_BETA         (5000) /* 9830; 0.2500;           */
#define V17_EQ_FINE_BETA          (1024) /* 0; 2048;  1311; 0.0400; */
#define V17_EQ_ADAPT_BETA         (256)  /* 0; 1024;  1311; 0.0400; */
#define V17_EQ_DATA_BETA          (128)  /* 0; 512;  0x1000; 8192;  */

#define V17_JAM_DPH_IDX           (43)   /* 256/V32_SYM_SIZE: length of cosine table is 256 */

#define V17_TIME_CONST            (3911) /* 64*64*3/pi */

#define V17_TEMP                  (5)/* 0: V32_EQ_LENGTH=48; 5: V32_EQ_LENGTH=36 */
#define V17_LONG_TRN_SEG2_DELAY   (2996 - V17_TEMP)
#define V17_SEG1_DELAY            (400)

#define V17_EQ_DELAY              (19 - V17_TEMP)

/* for auto-correlation */
#define V17_CORR_NUM_SYMBOLS      (5)
#define V17_CORR_NUM_SAMPLES      (V17_CORR_NUM_SYMBOLS * V32_SYM_SIZE)
#define V17_SYNC_SAMPLES          (V32_SYM_SIZE << 1)
#define V17_SYNC_DEMOD_SHIFT      (4)
#define V17_SYNC_DEMOD_SYMBOLS      (1<<(V17_SYNC_DEMOD_SHIFT+1))

#define V17_TONE_NUMBER           (4)
#define V17_600HZ                 (0)
#define V17_3000HZ                (1)
#define V17_1800HZ                (2)
#define V17_V21TONE_1650HZ        (3)

#define V17_AGC_CALCULATION_POINT (100)

#define V17_ENERGY_REF            (397000) // fixing issue 191
#define V17_SILENCE_LEN_REF       (1)

#endif
