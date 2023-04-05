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

/******************************************************************************
        Description :
                Definitions for S Registers
 *****************************************************************************/

#ifndef _SREGDEF_H
#define _SREGDEF_H

/****** S registers ********/
#define NUMBER_S_REGISTERS      (64)

#define AUTO_ANS_RING_NUMBER    (0)
#define AT_ESCAPE_CHARACTER     (2)
#define CR_CHARACTER            (3)
#define LF_CHARACTER            (4)
#define BS_CHARACTER            (5)
#define BLIND_DIAL_PAUSE_TIME   (6)
#define NO_ANSWER_TIMEOUT       (7)
#define COMMA_DIAL_PAUSE_TIME   (8)
#define NO_CARRIER_DISCONNECT   (10)
#define TONE_DIAL_LENGTH        (11)
#define ESCAPE_GUARD_TIME       (12)
#define DIAL_TONE_DET_TIMEOUT   (13)
#define V54_SELF_TEST           (16)
#define V54_SELF_TIMER          (18)
#define AUTO_RETRAIN_CONTROL    (21)
#define V54_RESPONSE_CONTROL    (23)
#define V90D_PCM_CODING         (30)
#define V34_SYMBOL_RATE         (33)
#define TRANSMIT_POWER          (34)
#define CP_COUNTRY_CODE         (35)
#define DTMF_TRANSMIT_POWER     (36)
#define V42BIS_COMPRESSION_DIR  (37)    /* Not Used? */
#define V42BIS_DICTIONARY_SIZE  (38)    /* Not Used? */
#define V42BIS_MAX_STRING_LEN   (39)    /* Not Used? */
#define V92_OPTIONS             (40)
#define DOWNSTREAM_POWER_INDEX  (41)
#define DRIVER_PARAM1           (50)    /* Registers 50-63 can be used for driver */
#define DRIVER_PARAM2           (51)
#define DRIVER_PARAM3           (52)
#define DRIVER_PARAM4           (53)
#define DRIVER_PARAMS_STARTIDX  (DRIVER_PARAM1)

/* V90D_PCM_CODING */
#define A_LAW                   (1)
#define MU_LAW                  (0)

/* V92_OPTIONS */
#define QUICK_CONNECT_DISABLE   (0x01)
#define V92_DISABLE             (0x02)
#define DIL_ENABLE              (0x04)
#define HPF_100HZ               (0x08)

/* V42BIS_DICTIONARY_SIZE */
#define V42B_DICT_SMALL         (9)     /* 2 << 9 */
#define V42B_DICT_LARGE         (11)    /* 2 << 11 */

/* V42BIS_MAX_STRING_LEN */
#define V42B_STRING_SMALL       (6)     /* With V42B_DICT_SMALL */
#define V42B_STRING_LARGE       (32)    /* With V42B_DICT_LARGE */

#endif /* _SREGDEF_H */
