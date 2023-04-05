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

#ifndef _V34DEF_H
#define _V34DEF_H

#include "sharedef.h"

#define V34_SNR                       (0)

#define ALL_VA                        (0)

#define P3_RXFUNC_EQ1                 (1)
#define P3_RXFUNC_PP_EQ               (2)
#define P3_RXFUNC_TRN_EQ              (3)

#if DRAWEPG
#define V34DRAWTIME                   (0)
#else
#define V34DRAWTIME                   (0)/* fix */
#endif

#define FREQUENCY_OFFSET              (1)

#define NON_LINEAR                    (1)
#define VITERBI_ON                    (1)
#define PHASEJITTER                   (1) /* Only work with 4 point training */

#define V34_4_POINT_J                 (0x8990U)   /* J pattern indicates 4  points training */
#define V34_16_POINT_J                (0x89B0U)   /* J pattern indicates 16 points training */
#define V34_POINT_J_PI                (0x899FU)

#define V34_EXPECT_POWER_LEVEL        (3406)

#define V34_CALL_MODEM_TX_VEC_SIZE    (20)
#define V34_CALL_MODEM_RX_VEC_SIZE    (20)
#define V34_CALL_MODEM_EC_VEC_SIZE    (5)

/* New echo canceller defines */
#define V34_RTD_DLY                   (4100) /* maximum about 1.2 Sec RTD supported, 300 for 87.5 ms, all based on 3429 symbol rate */

/* In current implementation, Near and Far Echo Length must equal */
#define V34_EC_SUBLEN                 (32)
#define V34_ECHO_COEF_LEN             (V34_SYM_SIZE * V34_EC_SUBLEN)

#define V34_SYS_DLY                   (78) /* estimated V34 system delay related to near echo */
#define V34_MAX_BULK_LEN              (V34_RTD_DLY + V34_SYS_DLY + V34_EC_SUBLEN + 1 + V34_EC_SUBLEN)

#define V34_EQ_LENGTH                 (84)
#define V34_EQ_S_LEN                  (9)  /* Buffer length for S */

#define V34_EQ_DELAY                  (33)

#define V34_MP_0_SIZE                 (11)
#define V34_MP_1_SIZE                 (23)

#define V34_SHIFT_BITS                (7)    /* 2^7 = 128 */

#define V34_VA_BUF_MASK               (0x1F) /* VA_BUF_SIZE -1 */
#define V34_VA_BUF_SIZE               (48)   /* (VA_WinThreshold-1) * 2 */
#define V34_VA_WINTHRESHOLD           (25)   /* Must >= 5 ? */
#define V34_VA_STATENUM               (16)
#define V34_VA_D4SETNUM               (8)   /* (16): possible maximum 4d sets */
#define VITERBI_BRANCH_NUM            (4)

#define V34_AGC_AVE_LEN               (16)

#define V34_BAUD_ADJUST_LEN           (30)  /* Timing Recovery */
#define V34_INTER_POINT               (127) /* 64 */
#define V34_TIMJ_FACTOR               (121) /* 128*3/Pi = 122??? */

/* Time out value for V.34 different phase */
#define V34PHASE3_TIMEOUT             (10000)
#define V34PHASE4_TIMEOUT             (10000)

/* Tx power level */
#define V34_POWER_FACTOR              (31750)

#define V34_qBETA                     (q008)
#define V34_qBETA_S                   (500)

#define V34_qBETA_PP                  (q01)
#define V34_qBETA_TRN                 (q009) /* q008 */

#define V34_qBETA_TRN_MIDDLE          (1550) /* 750 */
#define V34_qBETA_J                   (1400)
#define V34_qBETA_TRN_FINE            (1250) /* 250 */

#define V34_qBETA_TRN_PH4             (300)

#define V34_qBETA_TCR                 (600) /* IN FLOATING MODE 0.03 */

/* ---------- Timing/Carrier  Recovery --------------------- */
#define V34_qTIMING_THRES             (255) /*492; 983*/
#define V34_qCARRIERLOOP_B3           (196) /* 0.0 */

/****************************************************/
#define V34_qAGC_H1                   (6287)  /* 0.191882955 */
#define V34_qAGC_H2                   (25031) /* 0.763899804 */
#define V34_qAGC_L1                   (1532)  /* 0.046769378 */
#define V34_qAGC_L2                   (384)   /* 0.011747936 */
#define V34_qAGC_K1                   (4)     /* IN FLOATING MODE 0.0001 */
#define V34_qAGC_K2                   (4)     /* IN FLOATING MODE 0.0001 */

/* AGC windows for S signal in Phase 3  */
/* Reference energy for S signal is 0.073 */
#define V34_qS_H1                     (3821)  /* 0.116613812 */
#define V34_qS_H2                     (13558) /* 0.413761421 */
#define V34_qS_L1                     (2990)  /* 0.09125 */
#define V34_qS_L2                     (842)   /* 0.025717744 */
/* AGC K-factor for S signal in Phase 3 */
#define V34_qS_K1                     (q005)
#define V34_qS_K2                     (q005)

#define V34_qPP_H1                    (3821)
#define V34_qPP_H2                    (13558)
#define V34_qPP_L1                    (2990)
#define V34_qPP_L2                    (842)
#define V34_qPP_K1                    (q005)
#define V34_qPP_K2                    (q005)

/* AGC windows for TRN signal in Phase 3 */
#define V34_qTRN_H1                   (3821)
#define V34_qTRN_H2                   (13558)
#define V34_qTRN_L1                   (2990)
#define V34_qTRN_L2                   (842)
/* AGC K-factor for TRN signal in Phase 3 */
#define V34_qTRN_K1                   (32)
#define V34_qTRN_K2                   (32)
/****************************************/

#define V34_SDETECT_NUMSAMPLES        (12)
#define V34_STONE_DELTAPHASE          (q1_OVER_3) /* Bandedge:   1/6  */
#define V34_SREF_DELTAPHASE           (q1_OVER_6) /* Bandedge/2: 1/12 */
#define V34_STONE_FACTOR              (3)     /* For S Detection:  (STone+SCar) > 1/Factor * SEgy    */
#define V34_SCAR_FACTOR               (8)     /* For S Detection:  SCar > 1/Factor * SEgy    */
#define V34_SREF_FACTOR               (8)     /* For S Detection:  SRef < 1/Factor * STone   */

#if SUPPORT_V34FAX
#define V34_SEGY_THRESHOLD_FAX        (125)   /* For S Detection:  SEgy > Thresh  */
#endif
#define V34_SEGY_THRESHOLD            (100)   /* For S Detection:  SEgy > Thresh  */

#define V34_SDET_SHIFT                (2)
#define V34_SCAR_SHIFT                (1)

#if PHASEJITTER
#define V34_JTTAPS                    (20)

#define JTDECSHIFT                    (0)
#define MIN_PD_CORR                   (0)
#define JTWAIT                        (20)
#endif

/* V34_TX_MAX_THROUGHPUT based on 33600 at 3429 symbol rate */
#define V34_TX_MAX_THROUGHPUT         (40) /* 79/8(bit/symbol)*32(symbol/call)/8(bit/char)=40(char/call) */

#define V34_CONST1                    (49)

/* retrain_reason */
#define V34_RETRAIN_DETECTED          (1)
#define V34_PHASE4_TIMEOUT            (2)
#define V34_ANS_DETECT_S_TIMEOUT      (3)
#define V34_ANS_DETECT_SSBAR_TIMEOUT  (4)
#define V34_DETECT_J_TIMEOUT          (5)
#define V34_PH4_DETECT_JBAR_TIMEOUT   (6)
#define V34_PH4_DETECT_SSBAR_TIMEOUT  (7)
#define V34_PH4_DETECT_E_TIMEOUT      (8)
#define V34_BIT_ERROR_TOO_HIGH        (9)
#define V34_EQ_TRAINING_BAD           (10)

/* hangup_reason */
#define V34_ECHO_BUF_SMALL            (1)
#define V34_PHASE3_TIMEOUT            (2)
#define V34_RETRAIN_TIMEOUT           (3)
#define V34_HOST_DISCONNECT           (4)
#define V34_BITRATE_NOT_IN_RANGE      (5)
#define V34_NO_COMMON_BIT_RATE        (6)

#define V34_RTN_TONE                  (0)
#define V34_RTN_REF                   (1)

#define V34_DIFF_DECODE(pDeCode, twobits, outbit) \
    {                                                 \
        (outbit) = ((twobits) - (pDeCode)->dZ) & 0x3;  \
        (pDeCode)->dZ = (SBYTE)(twobits);              \
    }

#endif
