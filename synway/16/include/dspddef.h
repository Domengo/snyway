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

#ifndef _DSPDDEF_H
#define _DSPDDEF_H

#include "commdef.h"
#include "commmac.h"

#define HILB_TAP_LEN         (47)
#define HILB_HALF_TAP_LEN    ((HILB_TAP_LEN-1)>>1) /* Half filter size - 1 */

#define TIMING_TAPS8         (8) /* Timing Recovery for V17, V27, V29 & V32 */
#define TIMING_TAPS10        (10)/* Timing Recovery for nothing */
#define TIMING_TAPS14        (14)/* Timing Recovery for V34 */

#define TIMING_RES           (12)
#define TIMELINE_RES         (6)
#define TIMEFILT_INTERP      ((1 << (TIMING_RES - TIMELINE_RES)) - 1)

#define POLYCOEF_ARRAY_LEN8  (TIMING_TAPS8  << (TIMING_RES - TIMELINE_RES))
#define POLYCOEF_ARRAY_LEN10 (TIMING_TAPS10 << (TIMING_RES - TIMELINE_RES))
#define POLYCOEF_ARRAY_LEN14 (TIMING_TAPS14 << (TIMING_RES - TIMELINE_RES))

#define DSP_PSF_COEF_LEN     (14)

#define DC_SHIFT_NUM         (15)

/* for Pulse Shape Filtering, used in V32 and V29 */
#define V32V29_PSF_DELAY_LEN (13)

#define EC_INTER_NUM         (3)

#define PHASE_180_DEGREE     (32768)

#if WITH_DC && (!USE_ASM)
#define LP_FILTER_SHIFT      (11)
#endif

#define LOG_CONST            (1406)

#endif
