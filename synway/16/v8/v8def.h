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

#ifndef _V8DEF_H
#define _V8DEF_H

#include "commdef.h"
#include "v21ext.h"

#define V8_CALL_RATE                (3)

#if SUPPORT_V92D
#define QC1A_SYNC_OCTET         (0xAA)
#define CM_SYNC_OCTET           (0x07)
#define V92D_BUF_SIZE           (48)
#define V92D_CALL_RATE          (1)
#endif

#define V8_SMPL_450MS_72            (3240)
#define V8_SMPL_450MS_96            (4320)

#define DELTAPHASE_2100_72          (19115) /* (2100Hz / 7200Hz * 65536) */
#define DELTAPHASE_2100_96          (14336) /* (2100Hz / 9600Hz * 65536) */

#define DELTAPHASE_15_72            (137)   /* (  15Hz / 7200Hz * 65536) */
#define DELTAPHASE_15_96            (102)   /* (  15Hz / 9600Hz * 65536) */

#define DELTAPHASE_2085_72          (18978) /* (2085Hz / 7200Hz * 65536) */
#define DELTAPHASE_2085_96          (14234) /* (2085Hz / 9600Hz * 65536) */

#define DELTAPHASE_2115_72          (19251) /* (2115Hz / 7200Hz * 65536) */
#define DELTAPHASE_2115_96          (14438) /* (2115Hz / 9600Hz * 65536) */

#if SUPPORT_CT
#define DELTAPHASE_1300_72          (11833) /* (1300Hz / 7200Hz * 65536) */
#define DELTAPHASE_1300_96          (8875)  /* (1300Hz / 9600Hz * 65536) */
#endif

#define ANSAM_AGC_REF_72            ((QDWORD)2874 << 8)
#define ANSAM_AGC_REF_96            ((QDWORD)3319 << 8)

/* changed from 8 to 16 to work with connection chipset modems. */
#define V8_V21_RXBUFSIZE            (16)    /*power of 2*/

#define V8_ANSAM_TIMEOUT_TX         (600 * V8_CALL_RATE)/*6 second time out*/
#define V8_ANSAM_TIMEOUT_RX         (600 * V8_CALL_RATE)/*6 second time out*/ // fixing bug 39, increased the timer to 6 seconds
#define V8_JM_TIMEOUT_RX            (300 * V8_CALL_RATE)// 3 seconds

#define V8_CM_TYPE                  (1)
#define V8_CJ_TYPE                  (2)
#define V8_JM_TYPE                  (3)
#define V8_CI_TYPE                  (4)

#define V8_TE                       (150)   /* Minimum time to disable the network, 0.5 s */
#define V8BIS_TE                    (1)

#define V8_CM_LENGTH1               (80)    /* for V34 */
#define V8_CM_LENGTH2               (90)    /* support V90 and V92 */

#if SUPPORT_V92A
#define V8_QC1A_LENGTH          (60)
#define V8_QC2A_LENGTH          (96)
#define V8_PPM_FACTOR           (5372)  /* 1000000 * (1/32768) * (301/(79 * 2pi)) * (1/(301 * 12)) << V8_PPM_SHIFT */
#define V8_PPM_SHIFT            (20)
#define V8_DRIFT_FACTOR         (228)   /* THIS IS SAME FORMAT AS PHASE 2: (5372/3018 * 128) */
#define V8_DRIFT_DELTA          (0)
#define SUPPORT_V8BIS           (1)
#define SHORTPHASE1_CALL_RATE   (1)
#define SHORTPHASE1_BUFSIZE     (96)
#else
#define SUPPORT_V8BIS           (0)     /* fixed */
#endif

#if SUPPORT_V92D
#define V8_QCA1d_LENGTH         (70)
#endif

#if SUPPORT_V8BIS
#define DELTAPHASE_400_72       (2731)  /* 65536* 400/9600 */
#define DELTAPHASE_400_96       (3641)  /* 65536* 400/7200 */

#define DELTAPHASE_1375_72      (9387)  /* 65536*1375/9600 */
#define DELTAPHASE_1375_96      (12515) /* 65536*1375/7200 */

#define DELTAPHASE_2002_72      (13667) /* 65536*2002/9600 */
#define DELTAPHASE_2002_96      (18222) /* 65536*2002/7200 */

#define CRE_SHIFT               (10)
#define V8BIS_DETECT_NUM        (5)
#endif

#define V8_CJ_LENGTH                (30)
#define V8_TIME_OUT_VALUE           (32767) /* This is the maximum value */
#define V8_LEN_OF_STR               (10)

#define V8_ANSAM_SHIFT              (5)
#define V8_ANS_SHIFT                (5)

#define V8_qANSAM_AMPLITUDE         (5659)
#define V8_ANSAM_FILTER_LENGTH      (51)

/* Sequence Decoder Values */
#define V8_MODN0_OCTET              (0xA0)
#define V8_MODN0_OCTET_MASK         (0xF0)
#define V8_MODN0_V90_DISABLE        (0xA0)
#define V8_MODN0_V90_DISABLE_MASK   (0xF4)
#define V8_MODN0_V34_DUPLEX         (0xA2)
#define V8_MODN0_V34_DUPLEX_MASK    (0xF2)
#define V8_MODN0_V34_HDX            (0xA1)
#define V8_MODN0_V34_HDX_MASK       (0xF1)

#define V8_MODN1_OCTET              (0x08)
#define V8_MODN1_OCTET_MASK         (0x1C)
#define V8_MODN1_V32_ENABLE         (0x80)
#define V8_MODN1_V32_ENABLE_MASK    (0x80)
#define V8_MODN1_V22_ENABLE         (0x40)
#define V8_MODN1_V22_ENABLE_MASK    (0x40)

#define V8_MODN2_OCTET              (0x08)
#define V8_MODN2_OCTET_MASK         (0x1C)
#define V8_MODN2_V21_ENABLE         (0x01)
#define V8_MODN2_V21_ENABLE_MASK    (0x01)

#define V8_V90D_AVAIL_OCTET         (0xE0)
#define V8_V90D_AVAIL_OCTET_MASK    (0xFB)

#define V8_V90A_AVAIL_OCTET         (0xE0)
#define V8_V90A_AVAIL_OCTET_MASK    (0xFD)

#define V8_END_OF_RECEIVE_OCTET     (0x7E)

#define V8_HIGHPASS_FILTER_ENABLE   (0)/*0: disable high-pass filter; 1: enable*/

#endif
