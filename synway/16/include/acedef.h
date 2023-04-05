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

#ifndef _ACEDEF_H
#define _ACEDEF_H

#include "commdef.h"

#define ACE_RESPONSE_LEN           (100)

#define FAX_DEBUG                  (1)

#if SUPPORT_V34FAX

#define UNDEF_MODEM                (0xFF)

#define V8_PASS_NO                 (0)/* V8 no pass                 */
#define V8_PASS_V21                (1)/* V8 pass with V21 for V17   */
#define V8_PASS_V8                 (2)/* V8 pass with V8  for V34fax */

#define T31_V34FAX_NONE               (0)
#define T31_V34FAX_CFR_RXED        (1)
#define T31_V34FAX_TX_ONES           (2)
#define T31_V34FAX_pri_RXED           (3)
#define V34FAX_TX_FLAG               (4)
#define T31_V34FAX_TX_FLAG2           (5)
#define T31_V34FAX_TX_FCD           (6)
#define T31_V34FAX_ctrl_RXED       (7)
#define V34FAX_TX_ctrl               (8)
#define V34FAX_TX_EOP               (9)
#define T31_V34FAX_TX_DCN           (10)
#define V34FAX_TX_ONES2               (11)
#define T31_V34FAX_WAIT_EOT           (12)
#define T31_V34FAX_EOT_RXED           (13)
#define V34FAX_RX_SILENCE           (15)
#define V34FAX_DISCONNECT           (16)
#define T31_V34FAX_MCF_RXED           (17)
#define T31_V34FAX_TX_NULL           (18)
#define T31_V34FAX_TX_MPS           (19)
#define T31_V34FAX_WAIT_MCF           (20)
#define T31_V34FAX_TX_EOP_MPS_NULL (21)

#define T31_V34FAX_ATA_RXED           (101)
#define V34FAX_RX_CM               (102)
#define V34FAX_TX_DIS               (103)
#define V34FAX_RX_DCS               (104)
#define T31_V34FAX_TX_CFR           (105)
#define V34FAX_RX_ONES               (106)
#define T32_V34FAX_RX_EOP_or_MPS   (109)
#define T31_V34FAX_MPS_RXED           (110)
#define T31_V34FAX_EOP_RXED           (111)
#define T31_V34FAX_TX_MCF           (112)
#define T31_V34FAX_TX_PPR           (113)
#define T31_V34FAX_DCN_RXED           (114)
#define T31_V34FAX_NULL_RXED       (115)
#define T31_V34FAX_EOM_RXED           (116)
#define T31_V34FAX_PPR_RXED           (117)
#define T31_V34FAX_PPR_RESEND       (118)

#define RCP_FRAME_LENGTH           (3)      /* RCP frame length */
#define FCD_FRAME_LENGTH           (256 + 4)/* FCD frame length, infact 68 or 260, think later */
#endif

#define HDLC_FLAG_LEN              (44) /* 40 for V34Fax??? */

#define SUCCESS                    (1)
#define FAIL                       (0)

#define AT_SIZE                    (80)

#define BIT_RATE_NUM               (43)

/* ACE Response code */
#define ACE_OK                     (0)
#define ACE_CONNECT                (1)
#define ACE_RING                   (2)
#define ACE_NO_CARRIER             (3)
#define ACE_ERROR                  (4)
#define ACE_CONNECT_1200           (5)
#define ACE_NO_DIALTONE            (6)
#define ACE_BUSY                   (7)
#define ACE_NO_ANSWER              (8)
#define ACE_CONNECT_MODEM          (9)
#define ACE_FCC                    (10)
#define ACE_FCC_V34FAX             (11)
#define ACE_CHAR_RANGE             (12)
#define ACE_A8E_CAP                (13)
#define ACE_MI                     (14)
#define ACE_MM                     (15)
#define ACE_MR                     (16)
#define ACE_BOOL_CAP               (17)
#define ACE_FC_CAP                 (18)
#define ACE_IPR_CAP                (19)
#define ACE_FPR_CAP                (20)
#define ACE_MOD_CAP                (21)
#define ACE_0_CAP                  (22)
#define ACE_FAP_CAP                (23)
#define ACE_FBO_CAP                (24)
#define ACE_FCQ_CAP                (25)
#define ACE_FFC_CAP                (26)
#define ACE_FMS_CAP                (27)
#define ACE_FMS_CAP_V34FAX         (28)
#define ACE_FPS_CAP                (29)
#define ACE_FRY_CAP                (30)
#define ACE_FNR_CAP                (31)

/* ACE Modulation code */
#define MMRCODE_NONE               (0)
#define MMRCODE_V21                (1)
#define MMRCODE_V23                (2)
#define MMRCODE_V22                (3)
#define MMRCODE_V22BIS             (4)
#define MMRCODE_V32                (5)
#define MMRCODE_V32BIS             (6)
#define MMRCODE_V34                (7)
#define MMRCODE_V90                (8)
#define MMRCODE_B103               (9)
#define MMRCODE_B212A              (10)
#define MMRCODE_V92                   (11)

/* ACE Protocol code */
#define PROTOCODE_NONE             (0)
#define PROTOCODE_V42              (1)
#define PROTOCODE_V42FAIL          (2)
#define PROTOCODE_V42BIS           (3)
#define PROTOCODE_MNP4             (4)
#define PROTOCODE_MNPFAIL          (5)
#define PROTOCODE_MNP3             (6)
#define PROTOCODE_MNP2             (7)
#define PROTOCODE_V44               (8)

#define CONTROL_FLAG_ZERO          (0)
#define CONTROL_FLAG_ATO1          (1)

/* call progress state */
#define NO_CP                      (0)
#define CP_START                   (1)
#define CP_DIAL                    (2)
#define CP_DIALFIN                 (3)
#define CP_DIALSTOP                (4)
#define CP_CONNECT                 (5)
#define CP_FINISH                  (6)
#define CP_ANSWER                  (7)
#define CP_ANSSTOP                 (8)
#define CP_PAUSE                   (9)

/* Semicolon definition */
#define SEMICOLON_NONE             (0)
#define SEMICOLON_ON_FIRST         (1) /* first semicolon present, dialtone detection needed */
#define SEMICOLON_ON_AGAIN         (2) /* another semicolon present, dialtone detection not needed */
#define SEMICOLON_AFTER            (3) /* after semicolon, OK issued and return to offline command */
/* dialtone detection not needed afterwards */

/***** Self test definition *****/
#define SELFTEST_OFF               (0x00)
#define SELFTEST_ON                (0x01)
#define ANDT0_UNSELECTED           (0x00)
#define ANDT0_SELECTED             (0x01)
#define SELF_TEST_LAL              (0x01)
#define SELF_TEST_LAL_PATT         (0x08)
#define SELF_TEST_RDL              (0x06)
#define SELF_TEST_RDL_PATT         (0x07)

#define SELF_TEST_1                (SELF_TEST_LAL)
#define SELF_TEST_8                (SELF_TEST_LAL_PATT)
#define SELF_TEST_6                (SELF_TEST_RDL)
#define SELF_TEST_7                (SELF_TEST_RDL_PATT)

#define SELF_TEST_6_END            (3)
#define SELF_TEST_7_END            (4)
#define SELF_TEST_8_END            (5)

#if SUPPORT_V54
#define SELF_TEST_DATA             (0x55)/* Test  pattern */
#endif

/************ S16(&T) ***************/
#define SREG_LAL_PATT              (BIT6)
#define SREG_RDL_PATT              (BIT5)

#define SREG_LAL_SELF              (BIT6)
#define SREG_RDL_SELF              (BIT5)
#define SREG_RDL                   (BIT4)
#define SREG_LAL                   (BIT0)

/************ S23(Parity,&T4,&T5) ***************/
#define SREG_PARITY                (BIT5|BIT4)
#define SREG_AND_T4                (BIT0)
#define RDL_ENABLE                 (SREG_AND_T4)

#define ACE_SELFTEST_TIMEOUT       (1500)/* timeout in 15 seconds */

#define AT_COMMAND_NAME_LEN        (17)
#define CLASS1COMM_NAME_LEN        (22)

#if SUPPORT_MODEM/************************/

#if SUPPORT_AUTO
#define DEFAULT_MODEM              (AUTOMODE)
#else/* Support_AUTO */

#if SUPPORT_ONLYONE_MODEM

#if SUPPORT_V21M
#define DEFAULT_MODEM              (MODEM_V21)
#endif

#if SUPPORT_V23
#define DEFAULT_MODEM              (MODEM_V23)
#endif

#if SUPPORT_V22
#define DEFAULT_MODEM              (MODEM_V22)
#endif

#if SUPPORT_V22BIS
#define DEFAULT_MODEM              (MODEM_V22bis)
#endif

#if SUPPORT_V32
#define DEFAULT_MODEM              (MODEM_V32)
#endif

#if SUPPORT_V32BIS
#define DEFAULT_MODEM              (MODEM_V32bis)
#endif

#if SUPPORT_V34
#define DEFAULT_MODEM              (MODEM_V34)
#endif

#if (SUPPORT_V90A + SUPPORT_V90D)
#define DEFAULT_MODEM              (MODEM_V90)
#endif

#if SUPPORT_B103
#define DEFAULT_MODEM              (MODEM_BELL103)
#endif

#if SUPPORT_BELL212A
#define DEFAULT_MODEM              (BELL212A)
#endif

#endif/* Support_ONLYONE_MODEM */
#endif/* Support_AUTO */
#endif/* Support_MODEM *//**********************/

#define TIMER_T0_LENGTH             (6000) // 60s
#define TIMER_V34_TX                (5000)
#define ECHO_DELAY                    (20)   // delay to Rx V.21 after Tx V.21

/* class 1 state */
#define ACE_NOF                    (0)
#define ACE_FTS                    (1)
#define ACE_FRS                    (2)
#define ACE_FTM                    (3)
#define ACE_FRM                    (4)
#define ACE_FTH                    (5)
#define ACE_FRH                    (6)
#define ACE_A8M                    (7)
#define ACE_TX1S                   (8) // Transmitting 1's
#define ACE_P2S                    (9) // Primary channel to silence transition
#define ACE_S2C                    (10) // Silence to control channel transition
#define ACE_ATH                    (11)
#define ACE_A8E                    (12)
#define ACE_FRH_HS                 (13)
#define ACE_FTH_HS                 (14)

#define MI_RESPONSE_CODE0_LEN     (22)

#endif
