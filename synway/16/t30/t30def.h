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

#if defined (_T30DEF_H)
/*Reentry!*/
#else
#define _T30DEF_H
#include "porting.h"
#include "commmac.h"
#include "commdef.h"
#include "errorcode.h"

// timers
#define TIMER_T1_LENGTH                 (3500) // 35s
#define TIMER_T2_LENGTH                 (600)  // 6s
#define TIMER_T3_LENGTH                 (1000) // 10s for manual operation
#define TIMER_T4_LENGTH                 (300)  // 3s
#define TIMER_T5_LENGTH                 (6000) // 60s
#define TIMER_T3S_LENGTH                (400)  // 3 + 1 s in order to be more flexible
#define TIMER_T200MS_LENGTH             (20)   // 200ms
#define T30_T4_TIMEOUT_4_5S             (518) /*4.5s+15%*/
#define TIMER_TCF_LENGTH                (300)
#define TIMER_SILENCE_CMD               (500)
#define TIMER_SILENCE_RSP               (270)
#define TIMER_MAX_PREAMBLE              (115)

#define T30_MAX_REPEAT_TIMES            (2) // retry 2 times, fixing issue 185

#define T30_RESPOND_CRP                 (1)
#define T30_RESPOND_BAD_CRP             (T30_SEND_CRP && 0)

#define SILENCE_MOD_CHG                 (4) // the silence inserted between V.21 and V.17/V.29/V.27ter modulation changes

#define SUPPORT_ECM_MODE                (1)

#define PC_ECM                          (0) /*0/1*/

#define FRAMESIZE_256_64                (0) /*0: 256byte; 1: 64byte*/

#if FRAMESIZE_256_64
#define FRAMESIZE_BIT_NUM               (6) /* for frame size 64 bytes */
#else
#define FRAMESIZE_BIT_NUM               (8) /* for frame size 256 bytes */
#endif

#define FRAMESIZE                       (1 << FRAMESIZE_BIT_NUM) /* frame size 64/256 bytes */
#define BLOCKNUM                        (256) /* block number must be 256 */

#define BLOCKNUM_BYTESIZE               (BLOCKNUM >> 3)

#define BLOCKSIZE                       (256 << FRAMESIZE_BIT_NUM)

#if SUPPORT_T38GW
#define NECM_BLOCKSIZE                  (BLOCKSIZE)
#else
#define NECM_BLOCKSIZE                  (1 << 10) // fixing bug 9
#endif

#define OPTFRAMELEN                     (20)

/* WorkingBuff: 2+2+256+2=262            HDLC frame bytes,
** RawData    : 262*8=2096                HDLC frame bits,
**              2096/5=419.2->420        MAX number of ZERO bits to insert,
**              2096+420=2516,            total bits,
**              2516/8=314.5->315,        total required bytes,
**              315+T30_THRESHOLD ::= 384    total practical bytes.
*/

#if FRAMESIZE_256_64
#define LEFTOVER_BUFSIZE                (128) // fixing issue 186
#else
#define LEFTOVER_BUFSIZE                (FRAMESIZE + 16)
#endif

#define T30DATATRANSFERLEN              (64)

/* the bit rate mask in DCS frame */
#define DCS_V17_14400                   (0x04)
#define DCS_V17_12000                   (0x14)
#define DCS_V17_9600                    (0x24)
#define DCS_V17_7200                    (0x34)
#define DCS_V29_9600                    (0x20)
#define DCS_V29_7200                    (0x30)
#define DCS_V27_4800                    (0x10)
#define DCS_V27_2400                    (0x00)
#define DCS_V34FAX                      (0x00)
#define BR_V21_300                      (0x3c)

#define T30MESNULL                      (0x7FF0)
#define T30FLAG                         (0x7FF1)
#define T30DATA                         (0x7FF2)
#define T30BUFFULL                      (0x7FF3)
#define T30TIMEOUT                      (0x7FF4)
#define T30_FRAME_ERR                   (0x7FF5)
#define T30TCFDATA                      (0x7FF6)
#define T30TCFOK                        (0x7FF7)
#define T4DATA                          (0x7FF8)
#define T4RTC_FLAG                      (0x7FF9)
#define T30TCFFAIL                      (0x7FFA)
#define T30DATAMODE                     (0x7FFB)
#define T30CRCERR                       (0x7FFC)

#define T4PAGE_END_FLAG                 (T4RTC_FLAG)

#define FRAME_HEAD                      (1)
#define FRAME_BODY                      (2)
#define FRAME_END                       (4)

/* remote capability */
#define DIS_V34F                        (0x00)
#define DIS_V27F                        (0x00)
#define DIS_V27                         (0x10)
#define DIS_V29                         (0x20)
#define DIS_V29F                        (0x30)
#define DIS_V17F                        (0x34)
#define DIS_V33F                        (0x38)

/* return values of T30_GetHDLCfromRemote() and hdlcRxStatus */
#define HDLC_INIT                       (0)
#define HDLC_NO_DATA                    (1)/* the stage before any data is received */
#define HDLC_FLAG_RXED                  (2)/* HDLC preamble is received */
#define HDLC_RX_ERROR                   (3)/* command/response receiving error */
#define HDLC_IN_PROCESS                 (4)/* HDLC receipt in Process */
#define HDLC_OPTFRM_RXED                (5)/* optional HDLC frame is received */
#define HDLC_LASTFRM_RXED               (6)/* last HDLC frame is received */
#define HDLC_CRC_OK                     (7) /* CRC good */
#define HDLC_CRC_BAD                    (8)/* CRC error */
#define HDLC_3S_TIMEOUT                 (9)/* Need to disconnect the line because of 3s timeout */

#define PROC_CNTL_STAGE_NULL            (0)

#define LS_FRAME_OPT1                   (0) /* First optional frame */
#define LS_FRAME_OPT2                   (1) /* Second optional frame */
#define LS_FRAME_OPT3                   (2) /* Third optional frame */
#define LS_FRAME_OPT4                   (3) /* Fourth optional frame */
#define LS_FRAME_LAST                   (4) /* Last frame */

#define PHASE_A_STAGE_CP                (10)
#define PHASE_A_STAGE_MENU              (11)
#define PHASE_A_STAGE_CONNECT           (12)

#define HS_RX_STAGE_FRM                 (20)
#define HS_RX_STAGE_CONNECT             (21)
#define HS_RX_STAGE_DATA                (22)
#define HS_RX_STAGE_V21                 (23)

#define LS_TX_STAGE_SILENCE             (30)
#define LS_TX_STAGE_PREAMBLE            (31)
#define LS_TX_STAGE_DATA                (32)
#define LS_TX_STAGE_CRC_OK              (33)
#define LS_TX_STAGE_CRC_BAD             (34)

#define HS_TX_STAGE_SILENCE             (40)
#define HS_TX_STAGE_SYNC                (41)
#define HS_TX_STAGE_DATA                (42)
#define HS_TX_STAGE_CRC_OK              (43)
#define HS_TX_STAGE_CRC_BAD             (44)
#define HS_TX_STAGE_END                 (45)

#define IMAGE_STATE_DATA                (0) // init state, in the middle of a line
#define IMAGE_STATE_EOL                 (1) // 11 0's, without the extra 0/1 in the case of MR
#define IMAGE_STATE_RTC_EOB             (2) // RTC for MH/MR, EOFB for MMR

// X resolution from dsmdef.h
#define X_R8                            (0x00)
#define X_R16                           (0x01)
#define X_100DPI                        (0x02)
#define X_200DPI                        (0x03)
#define X_300DPI                        (0x04)
#define X_400DPI                        (0x05)
#define X_600DPI                        (0x06)
#define X_1200DPI                       (0x07)

#define T38_GW_HDLC_FRM_NUM             (4) // holds up to 4 LS frames
#define T38_GW_ECM_FRM_NUM_256          (8) // holds up to 8
#define T38_GW_ECM_FRM_NUM_64           (34) // holds up to 34 64-byte frames
#define T38_GW_IMAGE_BUF_LEN            ((T38_GW_ECM_FRM_NUM_64 + 1) * (64 + 4) + 2) // holds up to 34 decoded 64-byte frames plus the trailing CRC

#define T30_PHASE_NULL                  (0)
#define T30_PHASE_A                     (1)
#define T30_PHASE_B                     (2)
#define T30_PHASE_C                     (3)
#define T30_PHASE_D                     (4)
#define T30_PHASE_E                     (5)
#define T30_PHASE_HANGUP                (6)

#define T38_SIDE_TX_LS                  (0)
#define T38_SIDE_RX_LS                  (1)

#endif
