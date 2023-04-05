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

#ifndef _V3217DEF_H
#define _V3217DEF_H

#include "commdef.h"
#include "dspddef.h"

#define V32_VITERBI                  (1)

/* for signal mapping */
#define V32_SIGMAP_SCALE             (9)
#define V32_SIGMAP2                  (2 << V32_SIGMAP_SCALE)
#define V32_SIGMAP6                  (6 << V32_SIGMAP_SCALE)

#define V32_SYM_SIZE                 (3)                         /* samples per symbol */
#define V32_NUM_SYM                  (12)                        /* symbols in a buffer */
#define V32_BUF_SIZE                 (V32_SYM_SIZE * V32_NUM_SYM)/* TX/RX buffer size */

/* for Timing Loop */
#define V32_TIMING_DELAY_HALF        (TIMING_TAPS8 + V32_SYM_SIZE - 1) /* 20 for the big array coefficient */

/* for Equalizer */
#define V32_EQ_LENGTH                 (36)
#define V32_EQ_DELAY                 (QQMULQR15(V32_EQ_LENGTH, q03))/* V32_EQ_LENGTH * 0.3 */

/* for Viterbi Decoding */
#define V32_VIT_STATES_SHIFT         (3)
#define V32_VIT_STATES               (1 << V32_VIT_STATES_SHIFT)
#define V32_VIT_ALLPATHS             (8)
#define V32_VIT_NUM_WIN              (16)
#define V32_VIT_NUM_WIN_1            (V32_VIT_NUM_WIN - 1)

/* phase angles in degree, map to index of cos table */
#define V32_PHASE_180                (128)
#define V32_PHASE_90                 (64)

#define V32_MOD_DELTA_PHASE          (V32_PHASE_90)/** 64 =256/4 **/
#define V32_NEARECHO_DELTA_PHASE     (3 * V32_MOD_DELTA_PHASE)

#define V32_TRN_SCRAMINBITS          (0x03)

/* ----- Hilbert Transform ----- */
#define V32_HILB_TAP_LEN             (35) /* 27,49 */
#define V32_HILB_HALF_TAP_LEN        ((V32_HILB_TAP_LEN - 1)>>1) /* 13,24 */ /* Half filter size - 1 */

#define V32_TIME_FAI_STEP            (32)

#if SUPPORT_V17
#define V32_FREQUENCY_OFFSET         (1)
#else
#define V32_FREQUENCY_OFFSET         (0)/* fix */
#endif

#endif
