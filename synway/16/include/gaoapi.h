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

#ifndef _GAOAPI_H
#define _GAOAPI_H

#include "commtype.h"

/*************** Defines ******************/

/* Current modem and line state define */
#define OFF_LINE             (0) /* offline command */
#define ONLINE_COMMAND       (1) /* modem online command */
#define ONLINE_DATA          (2) /* modem online data */

/* Only for SUPPORT_FAX */
#define ONLINE_FAX_COMMAND   (3) /* fax online command */
#define ONLINE_FAX_DATA      (4) /* fax online data */

#define ONLINE_RETRAIN       (5) /* modem online retrain */
#define ONLINE_RERATE        (6) /* modem online rerate */
#define CALL_PROGRESS        (7) /* call progress */
#define MODEM_TRAINING       (8) /* modem online training */
#define GOING_OFF_LINE       (9) /* transition from online to offline */
#define GO_V17FAX            (0x10) /* fall back to V17 fax */

/* Uart Flow Control type */
#define FLOWCONTROL_NO       (0)
#define FLOWCONTROL_SW       (1)
#define FLOWCONTROL_HW       (2)

/* Modem control commands define */
#define NO_CMD               (0) /* No command */
#define DISCONNECT_CMD       (1) /* Request modem disconnect */
#define RETRAIN_CMD          (2) /* Request modem retrain */

SWORD GAO_Lib_Init(void *pLibDataMem, UDWORD udLenBuf);

SWORD GAO_PCM_Proc(void *pLibDataMem, SWORD *pPCMIn, SWORD *pPCMOut);

/* for Customer to check how many bytes data modem can receive from UART */
/* i.e., empty space of DteWr buffer */
SWORD GAO_InBuf_Chk(void *pLibDataMem);

/* for Customer to check how many bytes data modem can receive from UART */
/* i.e., empty space of DteWr buffer */
SWORD GAO_OutBuf_Chk(void *pLibDataMem);

/* transmit data from remote equipment */
SWORD GAO_Buf_In(void *pLibDataMem, UBYTE *pBuf, UWORD uLenIn);

/* transmit data to remote equipment
return the data number actually transmitted */
SWORD GAO_Buf_Out(void *pLibDataMem, UBYTE *pBuf, UWORD uLenBuf);

SWORD GAO_ModStatus_Init(GAO_ModStatus *pStatus);
SWORD GAO_ModStatus_Upd(void *pLibDataMem, GAO_ModStatus *pStatus);

#endif
