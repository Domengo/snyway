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

#ifndef _DSM_DEF_H_DTE_
#define _DSM_DEF_H_DTE_

#include "gstdio.h"

#define MSG_BUF_EMPTY                             (0)
#define MSG_BUF_NOT_EMPTY                         (1)

#define ID_STRING_LEN_MAX                         (20)

// fixing bug 23, need to double the timer
#define DSM_RESP_TIMER_LEN                        (10000 << 1) // 100 seconds, the maximum amount of time DSM will wait for the response from the DCE

// Operation Stage
#define DTE_OS_CONFIG                             (0)
#define DTE_OS_COMMAND                            (1)
#define DTE_OS_DATA                               (2)

// Messages' Type
#define MSG_TYPE_CONTROL                          (1) // DL
#define MSG_TYPE_CONFIG                           (2) // DL
#define MSG_TYPE_REPORT                           (3) // UL
#define MSG_TYPE_IMAGE_DATA                       (4) // DL or UL
#define MSG_TYPE_MAINTENANCE                      (5) // DL or UL
#define MSG_TYPE_TEST                             (6) // DL or UL
#define MSG_TYPE_READ                             (7) // DL or UL

// fax transmission or receiving
#define DSM_FAX_TRANSMISSION                      (1)
#define DSM_FAX_RECEIVING                         (2)
#define DSM_FAX_TRANSMISSION_WITH_COVERP          (3)

#define DSM_TX_TIFF_FILE                          (1)
#define DSM_TX_TIFF_FILE_BY_POLL                  (2)
#define DSM_RX_TIFF_FILE                          (3)
#define DSM_RX_TIFF_FILE_BY_POLL                  (4)
#define DSM_RE_TX_TIFF_FILE                       (5)
#define DSM_RE_TX_TIFF_FILE_BY_POLL               (6)

//type of a fax call
#define FAX_CALL_IDL                              (0)
#define FAX_CALL_ORG                              (1)
#define FAX_CALL_ANS                              (2)

//response code
#define DSM_RC_NULL                               (0)
#define DSM_RC_OK                                 (1)
#define DSM_RC_CONNECT                            (2)
#define DSM_RC_RING                               (3)
#define DSM_RC_NO_CARRIER                         (4)
#define DSM_RC_ERROR                              (5)
#define DSM_RC_NODIAL_TONE                        (7)
#define DSM_RC_BUSY                               (8)
#define DSM_RC_NO_ANSWER                          (9)
#define DSM_RC_HANGUPING                          (10)

//image quality check
#define DISABLE_DCE_RX_IMAGE_QUALITY_CHECK        (0)
#define ENABLE_DCE_RX_IMAGE_QUALITY_CHECK         (1)
#define ENABLE_DCE_RX_IMAGE_QUALITY_CORRECTING    (2)
#define DISABLE_DCE_TX_IMAGE_QUALITY_CHECK        (0)
#define ENABLE_DCE_TX_IMAGE_QUALITY_CHECK         (1)
#define ENABLE_DCE_TX_IMAGE_QUALITY_CORRECTING    (2)

//action after PRI
#define STILL_DURING_PRI                          (0)
#define FAX_TX_AFTER_PRI                          (1)
#define FAX_RX_AFTER_PRI                          (2)
#define HANGUP_AFTER_PRI                          (3)

//post page commands
#define DSM_PMC_IDL                               (0)
#define DSM_PMC_MPS                               (1)
#define DSM_PMC_EOM                               (2)
#define DSM_PMC_EOP                               (3)

// states of DSM
#define DSM_STATE_INIT                            (0)
#define DSM_STATE_ATD_ATA                         (1)
#define DSM_STATE_FDT                             (2)
#define DSM_STATE_FDR                             (3)
#define DSM_STATE_CONNECT                         (4)
#define DSM_STATE_IMAGE                           (5)
#define DSM_STATE_PRI                             (6)
#define DSM_STATE_FKS                             (7)
#define DSM_STATE_GOING_HANGUP                    (8)

// For message buffer
#define MSG_DIR_IDLE                              (0)
#define MSG_DOWN_LINK                             (1)
#define MSG_UP_LINK                               (2)

//Fax classes
#define DTE_DCE_SERVICE_CLASS0                    (1)
#define DTE_DCE_SERVICE_CLASS1                    (2)
#define DTE_DCE_SERVICE_CLASS1_0                  (3)
#define DTE_DCE_SERVICE_CLASS2                    (4)
#define DTE_DCE_SERVICE_CLASS2_0                  (5)
#define DTE_DCE_SERVICE_CLASS2_1                  (6)
#define DTE_DCE_SERVICE_CLASS8                    (7)
#define DTE_DCE_SERVICE_CLASS3                    (8)

//Vertical resolution
#define VR_NORMAL_385                             (0x01)
#define VR_FINE_77                                (0x02)
#define VR_PERFECT_154                            (0x03)
#define VR_100L_INCH                              (0x04)
#define VR_200L_INCH                              (0x05)
#define VR_300L_INCH                              (0x06)
#define VR_400L_INCH                              (0x07)
#define VR_600L_INCH                              (0x08)
#define VR_800L_INCH                              (0x09)
#define VR_1200L_INCH                             (0x0A)

// X resolution
#define R4_X_RESOLUTION                           (0x01)
#define R8_X_RESOLUTION                           (0x02)
#define R16_X_RESOLUTION                          (0x03)
#define X_RESOLUTION_100DPI                       (0x04)
#define X_RESOLUTION_200DPI                       (0x05)
#define X_RESOLUTION_300DPI                       (0x06)
#define X_RESOLUTION_400DPI                       (0x07)
#define X_RESOLUTION_600DPI                       (0x08)
#define X_RESOLUTION_1200DPI                      (0x09)

// Bit rate
#define BR_DSM_2400                               (0x00)
#define BR_DSM_4800                               (0x01)
#define BR_DSM_7200                               (0x02)
#define BR_DSM_9600                               (0x03)
#define BR_DSM_12000                              (0x04)
#define BR_DSM_14400                              (0x05)
#define BR_DSM_16800                              (0x06)
#define BR_DSM_19200                              (0x07)
#define BR_DSM_21600                              (0x08)
#define BR_DSM_24000                              (0x09)
#define BR_DSM_26400                              (0x0A)
#define BR_DSM_28800                              (0x0B)
#define BR_DSM_31200                              (0x0C)
#define BR_DSM_33600                              (0x0D)

//Paper Width
#define DSM_A4_LETTER_LEGAL                       (1)
#define DSM_B4                                    (2)
#define DSM_A3                                    (4)

//Image Data format
#define DSM_DF_T4_1D                              (1)
#define DSM_DF_T4_2D                              (2)
#define DSM_DF_T6_2D                              (4)
#define DSM_DF_JPEG_MODE                          (8)

#if SUPPORT_TX_PURE_TEXT_FILE
#define DSM_DF_TEXT_FILE                          (16)
#endif

// ECM
#define DSM_ECM_OFF                               (0)
#define DSM_ECM_ON                                (1)

// Messages' Code                Interface A/E/B
// Configuration
#define CFG_SERVICE_CLASSES                       (0x1001)
#define CFG_DCE_CAPA_VR                           (CFG_SERVICE_CLASSES + 1)
#define CFG_DCE_CAPA_BR                           (CFG_DCE_CAPA_VR + 1)
#define CFG_DCE_CAPA_WD                           (CFG_DCE_CAPA_BR + 1)
#define CFG_DCE_CAPA_LN                           (CFG_DCE_CAPA_WD + 1)
#define CFG_DCE_CAPA_DF                           (CFG_DCE_CAPA_LN + 1)
#define CFG_DCE_CAPA_EC                           (CFG_DCE_CAPA_DF + 1)
#define CFG_DCE_CAPA_BF                           (CFG_DCE_CAPA_EC + 1)
#define CFG_DCE_CAPA_ST                           (CFG_DCE_CAPA_BF + 1)
#define CFG_DCE_CAPA_JP                           (CFG_DCE_CAPA_ST + 1)
#define CFG_LOCAL_ID                              (CFG_DCE_CAPA_JP + 1)
#define CFG_POLL_ID                               (CFG_LOCAL_ID + 1)
#define CFG_LOCAL_POLL_IND                        (CFG_POLL_ID + 1)  // local has doc to send when local is a called side
#define CFG_LOCAL_POLL_REQ                        (CFG_LOCAL_POLL_IND + 1) // local is able to receive a doc when local is a calling side
#define CFG_LOCAL_RECV_CAPA                       (CFG_LOCAL_POLL_REQ + 1) // local is able to receive
#define CFG_HDLC_FRAME_DEBUG                      (CFG_LOCAL_RECV_CAPA + 1)
#define CFG_NEGOTIATION_PARA_REPORT               (CFG_HDLC_FRAME_DEBUG + 1)
#define CFG_LOCAL_SUB_SEP_PWD_CAPA                (CFG_NEGOTIATION_PARA_REPORT + 1)
#define CFG_SUB_STRING                            (CFG_LOCAL_SUB_SEP_PWD_CAPA + 1)
#define CFG_SEP_STRING                            (CFG_SUB_STRING + 1)
#define CFG_PWD_STRING                            (CFG_SEP_STRING + 1)
#define CFG_LOCAL_PRI_ENABLE                      (CFG_PWD_STRING + 1)
#define CFG_COPY_QUALITY_CHECK                    (CFG_LOCAL_PRI_ENABLE + 1)
#define CFG_RECV_QUALITY_THRESHOLD                (CFG_COPY_QUALITY_CHECK + 1)
#define CFG_ADAPTIVE_ANS                          (CFG_RECV_QUALITY_THRESHOLD + 1)
#define CFG_DCE_PHASE_C_RESP_TIMEOUT              (CFG_ADAPTIVE_ANS + 1)
#define CFG_ECM_ATTEMP_BLOCK_RETRY_NUM            (CFG_DCE_PHASE_C_RESP_TIMEOUT + 1)
#define CFG_CONTINUE_CORRECT_4TH_PPR              (CFG_ECM_ATTEMP_BLOCK_RETRY_NUM + 1)
#define CFG_CONTINUE_NEXT_AFTER_GIVEUP_RESEND     (CFG_CONTINUE_CORRECT_4TH_PPR + 1)
#define CFG_MIN_PHASE_C_RATE                      (CFG_CONTINUE_NEXT_AFTER_GIVEUP_RESEND + 1)
#define CFG_TIME_OUT_FOR_DCE                      (CFG_MIN_PHASE_C_RATE + 1)
#define CFG_PACKET_PROTOCOL_DTE_DCE               (CFG_TIME_OUT_FOR_DCE + 1)
#define CFG_BIT_ORDER                             (CFG_PACKET_PROTOCOL_DTE_DCE + 1)
#define CFG_RECV_EOL_ALIGN                        (CFG_BIT_ORDER + 1)
#define CFG_SEND_DF_CONVERSION                    (CFG_RECV_EOL_ALIGN + 1)
#define CFG_TIFF_FILE_PTR_RX                      (CFG_SEND_DF_CONVERSION + 1)
#define CFG_TIFF_FILE_PTR_TX                      (CFG_TIFF_FILE_PTR_RX + 1)
#define CFG_CONFIRM_TX_TIFF_FILES                 (CFG_TIFF_FILE_PTR_TX + 1)
#define CFG_TIFF_FILE_PTR_TX_BY_POLL              (CFG_CONFIRM_TX_TIFF_FILES + 1)
#define CFG_RE_TRANSMIT_CAPA_NON_ECM              (CFG_TIFF_FILE_PTR_TX_BY_POLL + 1)
#define CFG_RING_NUMBER_FOR_AUTO_ANS              (CFG_RE_TRANSMIT_CAPA_NON_ECM + 1)
#define CFG_COVER_PAGE_ATTRIB                     (CFG_RING_NUMBER_FOR_AUTO_ANS + 1)
#define CFG_INIT_DCE_CUST                         (CFG_COVER_PAGE_ATTRIB + 1)
#define CFG_INIT_DCE_DEF                          (CFG_INIT_DCE_CUST + 1)
#define CFG_INIT_T38                              (CFG_INIT_DCE_DEF + 1)
#define CFG_WRITE_FCC                             (CFG_INIT_T38 + 1)
#define CFG_V8_OPR_CTRL                           (CFG_WRITE_FCC + 1)
#define CFG_FCLASS1_LAST                          (CFG_V8_OPR_CTRL + 1)

// Messages' Code              Interface A/E/B
// Control
#define CTL_FAX_CALL_ORG                          (0x1101)
#define CTL_FAX_CALL_ANS                          (CTL_FAX_CALL_ORG + 1)
#define CTL_REQUEST_PRI                           (CTL_FAX_CALL_ANS + 1)
#define CTL_ACCEPT_PRI                            (CTL_REQUEST_PRI + 1)
#define CTL_ACTION_AFTER_PRI                      (CTL_ACCEPT_PRI + 1)

// Messages' Code           Interface B/E/A
// Report
#define RPT_RESULT_CODE                           (0x1201)
#define RPT_REMOTE_ID                             (RPT_RESULT_CODE + 1)
#define RPT_HDLC_FLAG                             (RPT_REMOTE_ID + 1)
#define RPT_NEGOTIATED_PARAS                      (RPT_HDLC_FLAG + 1)
#define RPT_RECV_POST_PAGE_CMD                    (RPT_NEGOTIATED_PARAS + 1)
#define RPT_HANGUP_STATUS                         (RPT_RECV_POST_PAGE_CMD + 1)
#define RPT_SUB_SEP_PWD_FRAME                     (RPT_HANGUP_STATUS + 1)
#define RPT_REMOTE_POLL_IND                       (RPT_SUB_SEP_PWD_FRAME + 1)
#define RPT_RECV_POST_PAGE_RESP                   (RPT_REMOTE_POLL_IND + 1)
#define RPT_TRANSITION_TO_VOICE_PRI               (RPT_RECV_POST_PAGE_RESP + 1)
#define RPT_RING_INCOMING                         (RPT_TRANSITION_TO_VOICE_PRI + 1)
#define RPT_DCE_DISC                              (RPT_RING_INCOMING + 1)
#define RPT_VALIDITY_OF_TX_TIFF_FILE              (RPT_DCE_DISC + 1)
#define RPT_GAOFAXLIB_NEED_COVERPAGE_DATA         (RPT_VALIDITY_OF_TX_TIFF_FILE + 1)
#define RPT_FIS_OF_REMOTE_SIDE                    (RPT_GAOFAXLIB_NEED_COVERPAGE_DATA + 1)
#define RPT_FTC_OF_REMOTE_SIDE                    (RPT_FIS_OF_REMOTE_SIDE + 1)

// Test
// Action commands
#define TEST_FDT                                  (0x1301)
#define TEST_FDR                                  (TEST_FDT + 1)
#define TEST_FKS                                  (TEST_FDR + 1)
#define TEST_FIP                                  (TEST_FKS + 1)
// Parameters
#define TEST_FCLASS                               (TEST_FIP + 1)
#define TEST_FCC                                  (TEST_FCLASS + 1)
#define TEST_FIS                                  (TEST_FCC + 1)
#define TEST_FCS                                  (TEST_FIS + 1)
#define TEST_FLI                                  (TEST_FCS + 1)
#define TEST_FPI                                  (TEST_FLI + 1)
#define TEST_FLP                                  (TEST_FPI + 1)
#define TEST_FSP                                  (TEST_FLP + 1)
#define TEST_FNR                                  (TEST_FSP + 1)
#define TEST_FIE                                  (TEST_FNR + 1)
#define TEST_FPS                                  (TEST_FIE + 1)
#define TEST_FCQ                                  (TEST_FPS + 1)
#define TEST_FIT                                  (TEST_FCQ + 1)
#define TEST_FPP                                  (TEST_FIT + 1)
#define TEST_FBO                                  (TEST_FPP + 1)
#define TEST_FEA                                  (TEST_FBO + 1)
#define TEST_FMI                                  (TEST_FEA + 1)
#define TEST_FMM                                  (TEST_FMI + 1)
#define TEST_FMR                                  (TEST_FMM + 1)
#define TEST_FPR                                  (TEST_FMR + 1)
#define TEST_FLO                                  (TEST_FPR + 1)
#define TEST_FNS                                  (TEST_FLO + 1)
#define TEST_FBU                                  (TEST_FNS + 1)
#define TEST_FAP                                  (TEST_FBU + 1)
#define TEST_FRQ                                  (TEST_FAP + 1)
#define TEST_FAA                                  (TEST_FRQ + 1)
#define TEST_FCT                                  (TEST_FAA + 1)
#define TEST_FHS                                  (TEST_FCT + 1)
#define TEST_FRY                                  (TEST_FHS + 1)
#define TEST_FMS                                  (TEST_FRY + 1)
#define TEST_FND                                  (TEST_FMS + 1)
#define TEST_FFC                                  (TEST_FND + 1)
#define TEST_FBS                                  (TEST_FFC + 1)

// Read parameters
#define READ_FCLASS                               (0x1401)
#define READ_FCC                                  (READ_FCLASS + 1)
#define READ_FIS                                  (READ_FCC + 1)
#define READ_FCS                                  (READ_FIS + 1)
#define READ_FLI                                  (READ_FCS + 1)
#define READ_FPI                                  (READ_FLI + 1)
#define READ_FLP                                  (READ_FPI + 1)
#define READ_FSP                                  (READ_FLP + 1)
#define READ_FNR                                  (READ_FSP + 1)
#define READ_FIE                                  (READ_FNR + 1)
#define READ_FPS                                  (READ_FIE + 1)
#define READ_FCQ                                  (READ_FPS + 1)
#define READ_FIT                                  (READ_FCQ + 1)
#define READ_FPP                                  (READ_FIT + 1)
#define READ_FBO                                  (READ_FPP + 1)
#define READ_FEA                                  (READ_FBO + 1)
#define READ_FMI                                  (READ_FEA + 1)
#define READ_FMM                                  (READ_FMI + 1)
#define READ_FMR                                  (READ_FMM + 1)
#define READ_FPR                                  (READ_FMR + 1)
#define READ_FLO                                  (READ_FPR + 1)
#define READ_FNS                                  (READ_FLO + 1)
#define READ_FBU                                  (READ_FNS + 1)
#define READ_FAP                                  (READ_FBU + 1)
#define READ_FRQ                                  (READ_FAP + 1)
#define READ_FAA                                  (READ_FRQ + 1)
#define READ_FCT                                  (READ_FAA + 1)
#define READ_FHS                                  (READ_FCT + 1)
#define READ_FRY                                  (READ_FHS + 1)
#define READ_FMS                                  (READ_FRY + 1)
#define READ_FND                                  (READ_FMS + 1)
#define READ_FFC                                  (READ_FND + 1)
#define READ_FBS                                  (READ_FFC + 1)

// DSM and T.32/T.30   Interface B
// message code
// control
#define CTL_TRANSMIT_PAGE                         (0x2101)
#define CTL_RECEIVE_PAGE                          (CTL_TRANSMIT_PAGE + 1)
#define CTL_DTE_READY_RECV                        (CTL_RECEIVE_PAGE + 1)
#define CTL_AT_FKS                                (CTL_DTE_READY_RECV + 1)

// DSM and IMG         Interface C
// message code
// control
#define CTL_IMAGE_CONVERSION                      (0x3001)

// Messages' Code
// cfg                  Interface E
#define CFG_TX_IMG_FORMAT                         (0x5001)
// report
#define RPT_TIFF_FILE_RUN_MODE                    (0x5201)

// Image Data or Speech   Interface E/C/D
#define IMG_DATA_AND_STATUS                       (0x5301)

// Maintenance type msg code
//    Downlink
#define MTN_REQUEST_FAXLIB_VERSION                (0x1301)

// Maintenance Uplink msg
#define MTN_RESP_FAXLIB_VERSION                   (0x1311)

// for customer
#define RPT_FAX_STATUS                            (RPT_RESULT_CODE)
#define RPT_NEXT_PAGE                             (RPT_RECV_POST_PAGE_CMD)
#define RPT_HANGUP_CODE                           (RPT_HANGUP_STATUS)
#define RPT_TIFF_FILE_OPERATION_MODE              (RPT_TIFF_FILE_RUN_MODE)

#endif
