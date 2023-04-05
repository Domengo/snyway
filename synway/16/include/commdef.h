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

#ifndef _COMMDEF_H
#define _COMMDEF_H

#include "commtype.h"
#include "q_const.h"
#include "gstdio.h"
#include "porting.h"
#include "dpcs.h"   /* Data Pump Contorl & Status  */

#define WITH_DC                    (1)

#define USE_ASM                    (0)

#define USE_ANS                    (1)

#define DUMPSIMDATA                (SUPPORT_DUMP_DATA)

#define DUMP_OUT_DATA_ENABLE       (SUPPORT_SIMULATION && SUPPORT_DUMP_DATA)/* Dump PCMout and DTEout data. 0: disable; 1: enable */

#define DRAWEPG                    (SUPPORT_DRAW_EPG)/* draw EPG */

#define SIMAUTOHANGUP              (0)/* For single modem simulation, automatic hangup */

#define DUMPOTHERDTEOUTFILE        (0)/* for dump other DTE out file. When it is 1, DUMPSIMDATA = 1 */

#if SAMPLE_RATE_INIT_7200

#define HW_DEF_BUFSIZE             (72)
#define HW_DEF_SAM_RATE            (HWAPI_7200SR)
#define HW_DEF_SAM_PER_SEC         (7200)

#elif SAMPLE_RATE_INIT_9600

#define HW_DEF_BUFSIZE             (96)
#define HW_DEF_SAM_RATE            (HWAPI_9600SR)
#define HW_DEF_SAM_PER_SEC         (9600)

#else /* 8000 or SAMCONV */

#define HW_DEF_BUFSIZE             (80)
#define HW_DEF_SAM_RATE            (HWAPI_8000SR)
#define HW_DEF_SAM_PER_SEC         (8000)

#endif

#define BUFSIZE                    (512)
#define PCM_BUFSIZE                (96)/* 96=0.01s*(9600sample/s) */

/* Design for calculating MIPS performance.                             *
 *Return values from StopTimer represents the % of processor power use. *
 *Timer functions supports up to 10 timer IDs.                          */
#define TIMER_ON                   (0)
#define COMPARE_BYTE               (0)

#if TIMER_ON
#define MC_RECORDER                (1)/* fix */
#else
#define MC_RECORDER                (0)/* 0/1 */
#endif

#define MAX_DATA_SIZE              (5000000)

#define BER_CALC                   (0)

#define DUMPDATA_FLAG              (0)

#if DUMPDATA_FLAG
extern SDWORD DumpTone1[];
extern SDWORD DumpTone2[];
extern SDWORD DumpTone3[];
extern SDWORD DumpTone4[];
extern SDWORD DumpTone5[];
extern SDWORD DumpTone6[];
extern SDWORD DumpTone7[];
extern SDWORD DumpTone8[];

extern UDWORD DumpTone1_Idx;
extern UDWORD DumpTone2_Idx;
extern UDWORD DumpTone3_Idx;
extern UDWORD DumpTone4_Idx;
extern UDWORD DumpTone5_Idx;
extern UDWORD DumpTone6_Idx;
extern UDWORD DumpTone7_Idx;
extern UDWORD DumpTone8_Idx;
#endif

extern UDWORD Count;

#endif
