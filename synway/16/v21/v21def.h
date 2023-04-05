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

#ifndef _V21DEF_H
#define _V21DEF_H

#include "commdef.h"

#define V21_SAMPLE_RATE_7200             (1)
#define V21_SAMPLE_RATE_9600             (2)

#if SUPPORT_FAX
#define V21_7200                     (1)/* fix */
#else
#define V21_7200                     (1)
#endif

#if SUPPORT_V92A
#define V21_9600                     (1)/* fix */  /* Must be 9600 for V92a */
#else
#define V21_9600                     (1)
#endif

#define V21_CALL_RATE                    (3)

#if V21_7200
#define V21_BUFSIZE_72               (24)

#define V21_CH1_MARK_DELTA_PHASE_72  (8920) /* 65536*980/7200  */
#define V21_CH1_SPACE_DELTA_PHASE_72 (10741)/* 65536*1180/7200 */
#define V21_CH2_MARK_DELTA_PHASE_72  (15019)/* 65536*1650/7200 */
#define V21_CH2_SPACE_DELTA_PHASE_72 (16839)/* 65536*1850/7200 */
#define V21_FILTER_LENGTH_72         (31)
#define V21_LOWPASS_FILTER_LENGTH_72 (21)
#define V21_MAX_SILENCE_72           (6) //(60)/* fix issue206,60 is 200ms changed  to deal with the 200ms dropout */
#define V21CH1_PDEL_DELAY_LENGTH_72  (19)
#define V21CH2_PDEL_DELAY_LENGTH_72  (18)
#define V21_qCH1_MODEM_BETA_72       (17227)
#define V21_qCH2_MODEM_BETA_72       (31307)
#define V21_ENERGY_REFERENCE_72      (800) /* 45dBm pass; 46dBm disconnect */
#define V21_CARRIER_LOSS_THRES_72    (2000)/* 45dBm pass; 46dBm disconnect */
#define V21MODEM_AGC_REFERENCE_72    ((QDWORD)386<<8)
#define V21CH2F_AGC_REFERENCE_72     (223866)
#endif

#if V21_9600
#define V21_BUFSIZE_96               (32)

#define V21_CH1_MARK_DELTA_PHASE_96  (6690) /* 65536*980/9600  */
#define V21_CH1_SPACE_DELTA_PHASE_96 (8055) /* 65536*1180/9600 */
#define V21_CH2_MARK_DELTA_PHASE_96  (11264)/* 65536*1650/9600 */
#define V21_CH2_SPACE_DELTA_PHASE_96 (12629)/* 65536*1650/9600 */
#define V21_FILTER_LENGTH_96         (65)
#define V21_LOWPASS_FILTER_LENGTH_96 (37)
#define V21_MAX_SILENCE_96           (100)
#define V21CH1_PDEL_DELAY_LENGTH_96  (25)
#define V21CH2_PDEL_DELAY_LENGTH_96  (24)
#define V21_qCH1_MODEM_BETA_96       (26460)
#define V21_qCH2_MODEM_BETA_96       (16324)
#define V21_ENERGY_REFERENCE_96      (950) /* 45dBm pass; 46dBm disconnect */
#define V21_CARRIER_LOSS_THRES_96    (2700)/* 45dBm pass; 46dBm disconnect */
#define V21MODEM_AGC_REFERENCE_96    ((QDWORD)402<<8)

#define V21CH2F_AGC_REFERENCE_96     (233145)
#endif

#define V21_CH1                         (1)
#define V21_CH2                         (2)

#define V21_BIQUAD_NUM                  (2)

#define V21_TIMEOUT                     (3000)

#if SUPPORT_V21CH2FAX
#define V21_1650HZ                  (0)
#define V21_1700HZ                  (1)
#define V21_1800HZ                  (2)
#define V21_1850HZ                  (3)
#endif

#endif
