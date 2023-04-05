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

/*  Q_CONST.H
 *  ---------
 *
 *  Numerical constants in Q format
 *
 */

#ifndef _Q_CONST_H
#define _Q_CONST_H

#include "commtype.h"

#define COS_SAMPLE_NUM          (255)

/* Only for SUPPORT_T31, SUPPORT_T32 and SUPPORT_MNP */
#define NUL                     (0x00)
#define DLE                     (0x10)
#define ETX                     (0x03)
#define SUB                     (0x1A)

#define Q15_MAX                 (32767)
#define Q15_MIN                 (-32768)
#define UWORD_MAX               (65535)
#define Q30_MAX                 ((Q30)0x3FFFFFFF)/* = (32768 * 32768 - 1). stick to 30 bit resolution, */
#define Q31_MAX                 ((Q30)0x7FFFFFFF)/* for ASM fractional mode */
#define TWO_30                  (0x40000000UL)

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * followings are defined for fixed point
 * floating point version to be defined
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#define qONE                    (32767)
#define qTWO                    ((QDWORD)(65535))

#define qD_60                   ((QDWORD)(34315)) /* Q17.15 format of PI/3    */
#define qD_90                   ((QDWORD)(51472)) /* Q17.15 format of PI/2    */
#define qD_180                  ((QDWORD)(102944))/* Q17.15 format of PI      */
#define qD_270                  ((QDWORD)(154416))/* Q17.15 format of 3PI/2   */
#define qD_360                  ((QDWORD)(205887))/* Q17.15 format of 2PI     */
#define q1_OVER_PI              (10430)           /* Q1.15  format of 1/PI    */
#define q1_OVER_2PI             (5215)            /* Q1.15  format of 1/(2PI) */

#define q1_OVER_3               (10923)           /* Q1.15  format of 1/3     */
#define q1_OVER_6               (5461)            /* Q1.15  format of 1/6     */
#define q1_OVER_12              (2731)            /* Q1.15  format of 1/12    */
#define q1_OVER_60              (546)             /* Q1.15  format of 1/60    */
#define q1_OVER_96              (341)             /* Q1.15  format of 1/96    */
#define q1_OVER_150             (218)             /* Q1.15  format of 1/150   */
#define q2_OVER_3               (21845)           /* Q1.15  format of 2/3     */
#define q5_OVER_6               (27307)           /* Q1.15  format of 5/6     */

#define q001                    (328)             /* Q1.15  format of 0.01    */
#define q003                    (983)             /* Q1.15  format of 0.03    */
#define q005                    (1638)            /* Q1.15  format of 0.05    */
#define q0075                   (2458)            /* Q1.15  format of 0.075   */
#define q008                    (2621)            /* Q1.15  format of 0.08    */
#define q009                    (2949)            /* Q1.15  format of 0.09    */
#define q01                     (3277)            /* Q1.15  format of 0.1     */
#define q015                    (4915)            /* Q1.15  format of 0.15    */
#define q02                     (6554)            /* Q1.15  format of 0.2     */
#define q025                    (8192)            /* Q1.15  format of 0.25    */
#define q03                     (9830)            /* Q1.15  format of 0.3     */
#define q04                     (13107)           /* Q1.15  format of 0.4     */
#define q06                     (19661)           /* Q1.15  format of 0.6     */
#define q07                     (22938)           /* Q1.15  format of 0.7     */
#define q075                    (24576)           /* Q1.15  format of 0.75    */
#define q08                     (26214)           /* Q1.15  format of 0.8     */
#define q09                     (29491)           /* Q1.15  format of 0.9     */
#define q095                    (31130)           /* Q1.15  format of 0.95    */
#define q097                    (31785)           /* Q1.15  format of 0.97    */

#define q100                    (100)
#define q10                     (10)
#define q5                      (5)

#define qLOG2                   (9864)

#define qSINGLE_TONE_12DBM0     (5713)
#define qDOUBLE_TONE_12DBM0     (8078)

/* end of fixed point q constant definitions */

#ifndef NULL
#define NULL                    (0)
#endif

#ifndef FALSE
#define FALSE                   (0)
#endif
#ifndef TRUE
#define TRUE                    (1)
#endif

#define IMAGE_RECEIVE_MODE      (0)
#define IMAGE_SEND_MODE         (1)

#define ANS_MODEM               (0)
#define CALL_MODEM              (1)

#if 1//SUPPORT_V54
#define DIGITAL_LOOP_BACK       (2)
#define ANALOG_LOOP_BACK        (3)
#endif

/* Retrain Defines */
#define RETRAIN_INIT            (1)
#define RETRAIN_RESPOND         (2)

#define RERATE_INIT             (3)
#define RERATE_RESPOND          (4)

#define CLEARDOWN_BY_LOCAL      (5)
#define MOH_BY_REMOTE           (6)

#define V34_BIT_33600           (14) /* 14*2400Hz */
#define V34_BIT_31200           (13) /* 13*2400Hz */
#define V34_BIT_28800           (12) /* 12*2400Hz */
#define V34_BIT_26400           (11) /* 11*2400Hz */
#define V34_BIT_24000           (10) /* 10*2400Hz */
#define V34_BIT_21600           (9)  /*  9*2400Hz */
#define V34_BIT_19200           (8)  /*  8*2400Hz */
#define V34_BIT_16800           (7)  /*  7*2400Hz */
#define V34_BIT_14400           (6)  /*  6*2400Hz */
#define V34_BIT_12000           (5)  /*  5*2400Hz */
#define V34_BIT_9600            (4)  /*  4*2400Hz */
#define V34_BIT_7200            (3)  /*  3*2400Hz */
#define V34_BIT_4800            (2)  /*  2*2400Hz */
#define V34_BIT_2400            (1)  /*  1*2400Hz */
#define V34_BIT_DEFAULT         (0) /* DCE default value */

#define DATA_RATE_75            (0)
#define DATA_RATE_300           (1)
#define DATA_RATE_600           (2)
#define DATA_RATE_1200          (3)
#define DATA_RATE_2400          (4)
#define DATA_RATE_4800          (5)
#define DATA_RATE_7200          (6)
#define DATA_RATE_9600          (7)
#define DATA_RATE_12000         (8)
#define DATA_RATE_14400         (9)
#define DATA_RATE_16800         (10)
#define DATA_RATE_19200         (11)
#define DATA_RATE_21600         (12)
#define DATA_RATE_24000         (13)
#define DATA_RATE_26400         (14)
#define DATA_RATE_28800         (15)
#define DATA_RATE_31200         (16)
#define DATA_RATE_33600         (17)
#define DATA_RATE_28000         (18)
#define DATA_RATE_29333         (19)
#define DATA_RATE_30666         (20)
#define DATA_RATE_32000         (21)
#define DATA_RATE_33333         (22)
#define DATA_RATE_34666         (23)
#define DATA_RATE_36000         (24)
#define DATA_RATE_37333         (25)
#define DATA_RATE_38666         (26)
#define DATA_RATE_40000         (27)
#define DATA_RATE_41333         (28)
#define DATA_RATE_42666         (29)
#define DATA_RATE_44000         (30)
#define DATA_RATE_45333         (31)
#define DATA_RATE_46666         (32)
#define DATA_RATE_48000         (33)
#define DATA_RATE_49333         (34)
#define DATA_RATE_50666         (35)
#define DATA_RATE_52000         (36)
#define DATA_RATE_53333         (37)
#define DATA_RATE_54666         (38)
#define DATA_RATE_56000         (39)

/* -----===== bit values =====----- */
#define BIT7                    (0x80)
#define BIT6                    (0x40)
#define BIT5                    (0x20)
#define BIT4                    (0x10)
#define BIT3                    (0x08)
#define BIT2                    (0x04)
#define BIT1                    (0x02)
#define BIT0                    (0x01)

#endif
