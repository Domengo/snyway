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

#ifndef ERROR_CODE_H
#define ERROR_CODE_H

// GAO Fax API Errors returned by API functions
#define SUCCESSFUL                           (0)
#define ERROR_SUBADDRESS                     (-2)
#define ERROR_LINE_ENCODING                  (-3)
#define ERROR_VER_RESOLUTION                 (-4)
#define ERROR_BIT_RATE                       (-5)
#define ERROR_MMR_REQUIRES_ECM               (-6)
#define ERROR_ERR_CORRECT_MODE               (-7)
#define ERROR_LOG_LEVEL                      (-8)
#define ERROR_NEED_CALL_FAXSYS_INIT          (-9)
#define ERROR_CHANNEL_NUM                    (-10)
#define ERROR_HEADER_FORMAT                  (-11)
#define ERROR_DATE_FORMAT                    (-12)
#define ERROR_TIME_FORMAT                    (-13)
#define ERROR_START_PAGE_NUM                 (-14)
#define ERROR_TIFF_ENCODE_FORMAT             (-17)
#define ERROR_NEED_CALL_FAX_INIT             (-18)
#define ERROR_TIFF_NULL                      (-19)
#define ERROR_HEADER_NULL                    (-20)
#define ERROR_CHANNEL_NOT_STOPPED            (-21)
#define ERROR_COMMAND_IN_PROGRESS            (-22)
#define ERROR_CHANNEL_NOT_READY              (-23)
#define ERROR_ALLOCATING_MEMORY              (-24)
#define ERROR_UNDEFINED_STATE                (-25)
#define ERROR_OPENING_HPI_DRIVER             (-26)
#define ERROR_CHANNEL_NOT_RECEIVING          (-27)
#define ERROR_CHANNEL_NOT_TRANSMITTING       (-28)
#define ERROR_CREATING_HPI_THREAD            (-29)
#define ERROR_OPENING_BOOT_FILE              (-30)
#define ERROR_RESETTING_DSP                  (-31)
#define ERROR_LOWERING_HPI_PRIORITY          (-32)
#define ERROR_ASSERTING_RESET                (-34)
#define ERROR_READING_BOOT_READY             (-35)
#define ERROR_DSP_NOT_RESETTING              (-36)
#define ERROR_DEASSERTING_RESET              (-37)
#define ERROR_DSP_NOT_RESET                  (-38)
#define ERROR_WRITING_DSP_REGISTER           (-39)
#define ERROR_READING_DSP_REGISTER           (-40)
#define ERROR_LOADING_BOOT_TABLE             (-41)
#define ERROR_OPENING_LOG_FILE               (-42)
#define ERROR_CREATING_SEMAPHORE             (-43)
#define ERROR_FAILED_TO_GET_SEMAPHORE        (-44)
#define ERROR_FAX_NULL                       (-45)
#define ERROR_LOG_NULL                       (-46)
#define ERROR_BUFFER_NULL                    (-47)
#define ERROR_NO_SESSION_YET                 (-48)
#define ERROR_LOCALID                        (-49)
#define ERROR_INVALID_LENGTH                 (-50)
#define ERROR_LOG_NUMBER_TOO_BIG             (-51)
#define ERROR_APPLICATION_LOADER_TIMEOUT     (-52)
#define ERROR_APPLICATION_SETS_REGISTERS     (-53)
#define ERROR_READING_DSP_MODULE_VERSION     (-54)

// Channel Errors returned by Fax_GetError()
#define ERROR_IMG_UNSPECIFIED                (-300)
#define ERROR_IMG_UNSUPPORTED_TIFF_PAGE      (-301)
#define ERROR_IMG_UNSUPPORTED_IMAGE_SETTINGS (-302)
#define ERROR_IMG_INITIALIZING_DECODER       (-303)
#define ERROR_IMG_INITIALIZING_ENCODER       (-304)
#define ERROR_IMG_INITIALIZING_SCALER        (-305)
#define ERROR_IMG_BAD_MMR_DATA               (-306)
#define ERROR_IMG_ENCODING_ERROR             (-307)
#define ERROR_IMG_UNEXPECTED_EOF             (-308)
#define ERROR_IMG_NO_RECEIVED_DATA           (-309)
#define ERROR_TXT_PAGE_HEADER_TOO_LONG       (-401)
#define ERROR_TXT_BAD_CHARACTER_NUMBER       (-402)
#define ERROR_TXT_GET_SYSTEM_TIME            (-403)
#define ERROR_TXT_FORMAT_TIME_STRING         (-404)
#define ERROR_TXT_FORMAT_DATE_STRING         (-405)

// 00-0F Call placement and termination
#define HANGUP_NORMAL                        (0x00)
#define HANGUP_RING_WITHOUT_HANDSHAKE        (0x01)
#define HANGUP_ABORTED_FROM_FKS_OR_CAN       (0x02)
#define HANGUP_NO_LOOP_CURRENT               (0x03)
#define HANGUP_RINGBACK_NO_ANSWER            (0x04)
#define HANGUP_RINGBACK_NO_CED               (0x05)
// 10-1F Transmit Phase A and miscellaneous errors
#define HANGUP_UNSPECIFIED_PHASE_A_ERROR     (0x10)
#define HANGUP_NO_ANSWER_T1_TIMEOUT          (0x11)
// 20-2F Transmit Phase B hangup codes
#define HANGUP_UNSPECIFIED_PHASE_B_XMIT      (0x20)
#define HANGUP_REMOTE_CANNOT_RCV_OR_SEND     (0x21)
#define HANGUP_COMREC_ERROR_PHASE_B_XMIT     (0x22)
#define HANGUP_INVALID_CMD_PHASE_B_XMIT      (0x23)
#define HANGUP_RSPREC_ERROR_PHASE_B_XMIT     (0x24)
#define HANGUP_DCS_SENT_3_TIMES_NO_RSP       (0x25)
#define HANGUP_DISDTC_RCVED_3_TIMES          (0x26)
#define HANGUP_FAILURE_TO_TRAIN_AT_FMS       (0x27)
#define HANGUP_INVALID_RSP_PHASE_B_XMIT      (0x28)
// 40-4F Transmit Phase C hangup codes
#define HANGUP_UNSPECIFIED_PHASE_C_XMIT      (0x40)
#define HANGUP_UNSPECIFIED_IMG_FORMAT        (0x41)
#define HANGUP_IMG_CONVERSION_ERROR          (0x42)
#define HANGUP_DTE_T30_DATA_UNDERFLOW        (0x43)
#define HANGUP_UNREC_TRANS_DATA_CMD          (0x44)
#define HANGUP_IMG_ERROR_LN_LEN_WRONG        (0x45)
#define HANGUP_IMG_ERROR_PG_LEN_WRONG        (0x46)
#define HANGUP_IMG_ERROR_COMP_CODE_WRONG     (0x47)
// 50-6F Transmit Phase D hangup codes
#define HANGUP_UNSPECIFIED_PHASE_D_XMIT      (0x50)
#define HANGUP_RSPREC_ERROR_PHASE_D_XMIT     (0x51)
#define HANGUP_NO_RSP_TO_MPS_3_TIMES         (0x52)
#define HANGUP_INVALID_RSP_TO_MPS            (0x53)
#define HANGUP_NO_RSP_TO_EOP_3_TIMES         (0x54)
#define HANGUP_INVALID_RSP_TO_EOP            (0x55)
#define HANGUP_NO_RSP_TO_EOM_3_TIMES         (0x56)
#define HANGUP_INVALID_RSP_TO_EOM            (0x57)
#define HANGUP_CANNOT_CONT_PIPPIN_XMIT       (0x58)
// 70-8F Receive Phase B hangup codes
#define HANGUP_UNSPECIFIED_PHASE_B_RCV       (0x70)
#define HANGUP_RSPREC_ERROR_PHASE_B_RCV      (0x71)
#define HANGUP_COMREC_ERROR_PHASE_B_RCV      (0x72)
#define HANGUP_T2_TIMEOUT_PG_NOT_RCVED       (0x73)
#define HANGUP_T1_TIMEOUT_AFTER_EOM          (0x74)
// 90-9F Receive Phase C hangup codes
#define HANGUP_UNSPECIFIED_PHASE_C_RCV       (0x90)
#define HANGUP_MISSING_EOL_AFTER_5_SEC       (0x91)
#define HANGUP_BAD_CRC_OR_FRM_ECM            (0x92)
#define HANGUP_T30_DTE_DATA_OVERFLOW         (0x93)
// A0-BF Receive Phase D hangup codes
#define HANGUP_UNSPECIFIED_PHASE_D_RCV       (0xA0)
#define HANGUP_INVALID_RSP_PHASE_D_RCV       (0xA1)
#define HANGUP_INVALID_CMD_PHASE_D_RCV       (0xA2)
#define HANGUP_CANNOT_CONT_PIPPIN_RCV        (0xA3)

#endif
