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

#ifndef _PORTING_H
#define _PORTING_H

// Build
#define VERSION_NUMBER               (16) // an SWORD
#define VERSION_LABEL                ("28.004.015.0016")  // this is the inside version number
#define CUSTOMER_BUILD               (0) // if it's customer build, all trace, epg, and dump must be off

// PCM source
#if defined(_MSC_VER)
#define SUPPORT_REALTIME             (0 || CUSTOMER_BUILD)
#else
#define SUPPORT_REALTIME             (1 || CUSTOMER_BUILD)
#endif

#define SUPPORT_SIMULATION           (!SUPPORT_REALTIME)
// Hardware platform specific
#define SUPPORT_FXS                  (0) // 1: for simu and real time of platforms that have FXS ports, 0: for platforms that have FXO ports.
#define SUPPORT_DRAW_EPG             (1 && !CUSTOMER_BUILD) // 1: for dos platform; 0: for simulation or other platforms

// DTE/DCE components included
#define SUPPORT_DTE                  (0) // 1: can be stand-alone modem/fax dte, or integrated fax (dte + dce); 0: stand-alone dce
#define SUPPORT_DCE                  (1) // 1: can be stand-alone modem/fax dce, or integrated fax (dte + dce); 0: stand-alone dte
#define SUPPORT_PARSER               (1) // can be at parser of either dte or dce
#define SUPPORT_T31                  (1) // can be t.31 of either dte or dce, with/without parser
#define SUPPORT_T32                  (1) // can be t.32 of either dte or dce, with/without parser
#define SUPPORT_T38EP                (0) // t.38 endpoint
#define SUPPORT_T38GW                (0) // t.38 gateway
#define SUPPORT_T31_PARSER           (1 && SUPPORT_PARSER && SUPPORT_T31 && !(SUPPORT_DTE && SUPPORT_DCE)) // can be t.31 parser of either dte or dce
#define SUPPORT_T32_PARSER           (1 && SUPPORT_PARSER && SUPPORT_T32 && !(SUPPORT_DTE && SUPPORT_DCE)) // can be t.32 parser of either dte or dce
// Do NOT change manually
#define SUPPORT_T38                  (SUPPORT_T38EP || SUPPORT_T38GW) // can be either t.38 dte and/or dce
#define SUPPORT_T38_DCE              (SUPPORT_DCE && SUPPORT_T38) // 1: t.38 dce; 0: analogue dce or no dce
#define SUPPORT_VXX_DCE              (SUPPORT_DCE && (!SUPPORT_T38 || SUPPORT_T38GW)) // 1: analogue dce; 0: t.38 dce or no dce
#define SUPPORT_T30                  ((SUPPORT_DTE && SUPPORT_T31) || (SUPPORT_DCE && SUPPORT_T32)) // whether or not t.30 fsm is included

// PCM, DTE dumping, overflow protection
#define SUPPORT_DUMP_DATA            (1 && !CUSTOMER_BUILD) // 1: dump pcm and dte in the library; 0: let outside application do it
#define SUPPORT_DISK_OVER_FLOW_CHECK (0 && SUPPORT_DTE && !CUSTOMER_BUILD) // 1: check disk volume for writing PCM data files and tiff files, 0: for final release
#if SUPPORT_DISK_OVER_FLOW_CHECK
#define MAX_DUMP_DATA_MEMORY         (600000000)// in byte
#endif

// Modem components

#define SUPPORT_V251                 (1)

#if (SUPPORT_T31 || SUPPORT_T32)
#define SUPPORT_V17                  (1)
#define SUPPORT_V27TER               (1)
#define SUPPORT_V29                  (1)
#define SUPPORT_V34FAX               (1)
#else
#define SUPPORT_V17                  (0)/* fix */
#define SUPPORT_V27TER               (0)/* fix */
#define SUPPORT_V29                  (0)/* fix */
#define SUPPORT_V34FAX               (0)/* fix */
#endif

#if (SUPPORT_V17 || SUPPORT_V27TER || SUPPORT_V29 || SUPPORT_V34FAX)
#define SUPPORT_FAX_DATAPUMP         (1)/* fix */
#else
#define SUPPORT_FAX_DATAPUMP         (0)/* fix */
#endif

#if (SUPPORT_FAX_DATAPUMP && (SUPPORT_T31 || SUPPORT_T32))
#define SUPPORT_FAX                  (1)/* fix */
#else
#define SUPPORT_FAX                  (0)/* fix */
#endif

/*********************************/
#define SUPPORT_ONLYONE_MODEM        (0)

/* If SUPPORT_ONLYONE_MODEM=1, Follow switch only one=1 */
#define SUPPORT_V21M                 (0)
#define SUPPORT_V23                  (0)
#define SUPPORT_V22                  (0)
#define SUPPORT_V22BIS               (0)
#define SUPPORT_V32                  (0)
#define SUPPORT_V32BIS               (0)
#define SUPPORT_V34                  (1)

/* If DUALMODEMSIM, both SUPPORT_V90A and SUPPORT_V90D may = 1; else only one = 1 */
#define SUPPORT_V90A                 (0)
#define SUPPORT_V90D                 (0)
#define SUPPORT_V92A                 (0)
#define SUPPORT_V92D                 (0)/* fix */

#define SUPPORT_B103                 (0)
#define SUPPORT_B212A                (0)
/*********************************/

#if (SUPPORT_B212A || SUPPORT_V22 || SUPPORT_V22BIS)
#define SUPPORT_V22_OR_B212          (1)/* fix */
#else
#define SUPPORT_V22_OR_B212          (0)/* fix */
#endif

#if SUPPORT_V90A && (!(SUPPORT_V34 + SUPPORT_V92A))
#error "V.90A requires V.34 or V.92A for upstream"
#endif

#if SUPPORT_V92A && (!SUPPORT_V90A)
#error "V.92A requires V.90A for downstream"
#endif

#if SUPPORT_V90D && (!(SUPPORT_V34 + SUPPORT_V92D))
#error "V.90D requires V.34 or V.92D for upstream"
#endif

#if SUPPORT_V92D && (!SUPPORT_V90D)
#error "V.92D requires V.90D for downstream"
#endif

#if (SUPPORT_V90A + SUPPORT_V90D)
#define SUPPORT_V90                  (1)/* fix */
#else
#define SUPPORT_V90                  (0)/* fix */
#endif

#if (SUPPORT_V92A + SUPPORT_V92D)
#define SUPPORT_V92                  (1)/* fix */
#else
#define SUPPORT_V92                  (0)/* fix */
#endif

#if (SUPPORT_V34 + SUPPORT_V90 + SUPPORT_V34FAX)
#define SUPPORT_V8                   (1)/* fix */
#else
#define SUPPORT_V8                   (0)/* fix */
#endif

#if (SUPPORT_V8 + SUPPORT_V21M)
#define SUPPORT_V21CH1               (1)/* fix */
#else
#define SUPPORT_V21CH1               (0)/* fix */
#endif

#define SUPPORT_V21CH2FAX            (SUPPORT_V17 || SUPPORT_V27TER || SUPPORT_V29)/* fix */
#define SUPPORT_V21CH2MODEM          (SUPPORT_V21M)/* fix */

#if (SUPPORT_V21CH1 + SUPPORT_V21CH2FAX + SUPPORT_V21CH2MODEM)
#define SUPPORT_V21                  (1)/* fix */
#else
#define SUPPORT_V21                  (0)/* fix */
#endif

#if (SUPPORT_B103 || SUPPORT_V21M || SUPPORT_V22_OR_B212 || SUPPORT_V23 || SUPPORT_V32 || SUPPORT_V32BIS || SUPPORT_V34 || SUPPORT_V90A || SUPPORT_V90D)
#define SUPPORT_MODEM                (1)/* fix */
#else
#define SUPPORT_MODEM                (0)/* fix */
#endif

#if (SUPPORT_V22_OR_B212 || SUPPORT_V32 || SUPPORT_V32BIS || SUPPORT_V34 || SUPPORT_V90A || SUPPORT_V90D)
#define SUPPORT_MNP                  (0)
#define SUPPORT_V42                  (0)
#else
#define SUPPORT_MNP                  (0)/* fix */
#define SUPPORT_V42                  (0)/* fix */
#endif

#if (SUPPORT_MODEM && (!SUPPORT_ONLYONE_MODEM))
#define SUPPORT_AUTO                 (0)
#else
#define SUPPORT_AUTO                 (0)/* fix */
#endif

#if SUPPORT_V42
#define SUPPORT_V42INFO              (1)
#define SUPPORT_V42BIS               (1)
#define SUPPORT_V44                  (1)
#define SUPPORT_V42_SREJ             (0) /* sSREJ function */
#define SUPPORT_V42_MSREJ            (0) /* mSREJ function */  /* CAN NOT define as 1 so far */

#define DEFAULT_V42                  (1)/* fix */
#define DEFAULT_V42BIS               (2)/* fix */

#if SUPPORT_V42BIS
#define PROTOCOL_DEFAULT             (DEFAULT_V42)/* DEFAULT_V42 or DEFAULT_V42BIS */
#else
#define PROTOCOL_DEFAULT             (DEFAULT_V42)/* fix */
#endif

#else

#define SUPPORT_V42INFO              (0)/* fix */
#define SUPPORT_V42BIS               (0)/* fix */
#define SUPPORT_V44                  (0)/* fix */
#define SUPPORT_V42_SREJ             (0)/* fix */
#define SUPPORT_V42_MSREJ            (0)/* fix */
#endif

#define SUPPORT_V54                  (0)/* support remote digital loopback test */

#if SUPPORT_V90D
#define SAMCONV                      (1)/* fix */ /* for V90D or V90 dual simulation */
#define A_MU_LAW_PCM                 (0)/* V90d PCM coding; 0 for MuLaw, 1 for ALaw */
#else
#define SAMCONV                      (0)/* Notice: MUST be 0 for V90a */
#endif

#if SAMCONV
#define SAMPLE_RATE_INIT_7200        (0)/* fix */
#define SAMPLE_RATE_INIT_9600        (0)/* fix */
#else
#define SAMPLE_RATE_INIT_7200        (1) /* support 8k sample rate if both are zero! */
#define SAMPLE_RATE_INIT_9600        (0)
#endif

#if SUPPORT_V27TER

#if SAMCONV
#define V27_HW7200SAMPLERATE_SUPPORT (0)/* fix */ /* 8000 <--> 9600 */
#else
#define V27_HW7200SAMPLERATE_SUPPORT (1)/* 1: 7200 <--> 9600; 0: 9600 <--> 9600 */
#endif

#else

#define V27_HW7200SAMPLERATE_SUPPORT (0)/* fix */

#endif

#define SUPPORT_CID_DET              (0)
#define SUPPORT_CID_GEN              (0)

#if SUPPORT_CID_DET

#define CID_SNOOP_MODE               (0) /* all onhook, no DTMF Ack for type 2 */

#if (SAMPLE_RATE_INIT_7200 || SAMPLE_RATE_INIT_9600)
#define CID_HW7200SAMPLERATE_SUPPORT (0)
#else
#define CID_HW7200SAMPLERATE_SUPPORT (0)/* fix */
#endif

#else

#define CID_HW7200SAMPLERATE_SUPPORT (0)/* fix */

#endif

#define SUPPORT_DTMF                 (0)

#define SUPPORT_ENERGY_REPORT        (0) /* For Fax and V22 modem */

#define SUPPORT_CT                   (0)
#define SUPPORT_CI                   (1)

#define SUPPORT_JPEG_CODEC           (0) // 1: support jpeg encoder and decoder, the conversion between Jpeg and T4/T6 or bmp and jpeg, 0: no
#define SUPPORT_TX_PURE_TEXT_FILE    (0) // 1: support send pure text files, 0: no
#define SUPPORT_TX_COVER_PAGE        (0) // 1: support send cover page, 0: no

#endif
