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

#ifndef _V34FDEF_H
#define _V34FDEF_H

#include "aceext.h"
#include "shareext.h"
#include "v34def.h"
#include "faxshmem.h"

#define V34FAX_EQ_DELAY_9                (9)
#define V34FAX_EQ_LENGTH_27              (3 * V34FAX_EQ_DELAY_9)  /* less MIPS */

#define V34FAX_EQ_DELAY_36               (36)
#define V34FAX_EQ_LENGTH_78              (78)

#define V34FAX_EQ_MAX_LENGTH             (V34FAX_EQ_LENGTH_78)

#define V34FAX_PCET_TIMEOUT              (1000)
#define V34FAX_MPET_TIMEOUT              (6000) // fixing issue 197, increased from 1500
#define V34FAX_PCR_TIMEOUT               (15000)

#define RETX_TIMER                       (300)

#define V34FAX_EQ_LEN                    (16)

#define V34FAX_SYM_LEN                   (12)
#define V34FAX_HALFSYM_LEN               (6)
#define V34FAX_SYM_NUM                   (6)
#define V34FAX_BUF_LEN                   (72)
#define V34FAX_PSF_LEN                   (48)
#define V34FAX_PSF_DELAY_LEN             (4)
#define V34FAX_SYM_BITS_1200             (2)
#define V34FAX_SYM_BITS_2400             (4)
#define V34FAX_BIQUAD_NUM                (8)
#define HILBERT_TAP_LEN                  (27)
#define HILBERT_DELAY_LEN                (13)

#define V34FAX_TIMING_DELAY_HALF         (48)
#define V34FAX_TIMELINE_RES              (8)
#define V34FAX_TIM_STEP                  (1)
#define V34FAX_TIMELINE_INTERP           (255)
#define V34FAX_TIMEIDX_MAX               (-3072)
#define V34FAX_TIMEIDX_THRES1            (-1024)
#define V34FAX_TIMEIDX_THRES2            (-2048)

/* Primary Channel */
#define V34FAX_SYNC_LEN                  (96)

#define V34FAX_TIMING_THRES              (480)
#define V34FAX_MAX_INDEX                 (128)
#define V34FAX_DLINE_LEN                 (30)
#define V34FAX_DLINE_DLEN                (60)
#define V34FAX_TIMING_STEP               (2)

#define MP_TYPE_0_LEN                    (88)
#define MP_TYPE_1_LEN                    (188)

#define TX_RATE_LIMIT                    (V34_BIT_33600)
#define RX_RATE_LIMIT                    (V34_BIT_33600)

#define QSAT(d)                          (QWORD)(((d) > Q15_MAX) ? Q15_MAX:((d) < Q15_MIN) ? Q15_MIN:(d))

/* RESYNC - PRIMARY CHANNEL */
#define V34_SYNC_SAMPLES                 (2 * V34_SYM_SIZE)
#define V34_SYNC_DEMOD_SHIFT             (3)
#define V34_SYNC_DEMOD_SYMBOLS           (1 << (V34_SYNC_DEMOD_SHIFT + 1))

/* RESYNC - CONTROL CHANNEL */
#define EQ_SAMPLES_PER_SYMBOL            (2)
#define SH_SAMPLES                       (EQ_SAMPLES_PER_SYMBOL * 2)

/* CONTROL CHANNEL TONE DETECT: SH/PPH */
#define V34FAX_PPH_SH_DETECT_LEN         (96)//(48) /* Must be multiple of 48 */
#define V34FAX_PPH_SH_DETECT_NUMTONES    (5)
#define V34FAX_PPH_SH_DETECT_BITSHIFT    (4)

#define V34FAX_AC_DETECT_LEN             (96)
#define V34FAX_AC_DETECT_BITSHIFT        (4)

#define MPET_PPH_SH_MIN_LEVEL_REF        (2500)/* 2700 for 47dB; 3400 for 45dB */

#define V34FAX_DEBUG_ERROR               (1)/* Can set 0 to disable it */

#define FREQUENCY_OFFSET_CONST_7200      (18641)

#endif
