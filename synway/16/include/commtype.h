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

/*****************************************************************
*      Description : This is a general header file for all the
*                    useful data types.
*
*      References : (1) Fixed point arithmetic
*                       Appendix C, ADSP-2100 Family User's Manual
*                   (2) TI application books
******************************************************************/

#ifndef _COMMTYPE_H
#define _COMMTYPE_H

#define CONST                 const

/* -- Global type definition for bit sensitive data ---------- */
#if defined(_MSC_VER)                 /* Visual C++            */
#define INLINE                _inline
typedef unsigned __int8       UBYTE;  /* Unsigned 8 bit data   */
typedef signed __int8         SBYTE;  /* Signed 8 bit data     */
typedef char                  CHAR;   /* 8 bit char            */

typedef unsigned __int16      UWORD;  /* Unsigned 16 bit data  */
typedef signed __int16        SWORD;  /* Signed 16 bit data    */

typedef unsigned __int32      UDWORD; /* Unsigned 32 bit data  */
typedef signed __int32        SDWORD; /* Signed 32 bit data    */

#elif defined(__GNUC__)               /* GCC                   */
#include <inttypes.h>
#define INLINE                static __inline__
typedef uint8_t               UBYTE;  /* Unsigned 8 bit data   */
typedef int8_t                SBYTE;  /* Signed 8 bit data     */
typedef char                  CHAR;   /* 8 bit char            */

typedef uint16_t              UWORD;  /* Unsigned 16 bit data  */
typedef int16_t               SWORD;  /* Signed 16 bit data    */

typedef uint32_t              UDWORD; /* Unsigned 32 bit data  */
typedef int32_t               SDWORD; /* Signed 32 bit data    */

#elif defined(_TMS320C6X)             /* TI C6X                */
#include <inttypes.h>
#define INLINE                static inline
typedef uint8_t               UBYTE;  /* Unsigned 8 bit data   */
typedef int8_t                SBYTE;  /* Signed 8 bit data     */
typedef char                  CHAR;   /* 8 bit char            */

typedef uint16_t              UWORD;  /* Unsigned 16 bit data  */
typedef int16_t               SWORD;  /* Signed 16 bit data    */

typedef uint32_t              UDWORD; /* Unsigned 32 bit data  */
typedef int32_t               SDWORD; /* Signed 32 bit data    */

#elif defined(__TMS320C55X__)
#include <inttypes.h>
#define INLINE                static inline
typedef unsigned char         UBYTE;  /* Unsigned 8 bit data   */
typedef signed char           SBYTE;  /* Signed 8 bit data     */
typedef char                  CHAR;   /* 8 bit char            */

typedef uint16_t              UWORD;  /* Unsigned 16 bit data  */
typedef int16_t               SWORD;  /* Signed 16 bit data    */

typedef uint32_t              UDWORD; /* Unsigned 32 bit data  */
typedef int32_t               SDWORD; /* Signed 32 bit data    */

typedef enum {false, true}     BOOL;
#else
#define INLINE                _inline
typedef unsigned char         UBYTE;  /* Unsigned 8 bit data   */
typedef signed char           SBYTE;  /* Signed 8 bit data     */
typedef char                  CHAR;   /* 8 bit char            */

typedef unsigned short        UWORD;  /* Unsigned 16 bit data  */
typedef signed short          SWORD;  /* Signed 16 bit data    */

typedef unsigned int          UDWORD; /* Unsigned 32 bit data  */
typedef signed int            SDWORD; /* Signed 32 bit data    */

typedef enum {false, true}     BOOL;
#endif

/* ---------- DSP data type definition in fixed point ---------- */
typedef SWORD                 QWORD;
typedef SDWORD                Q30;
typedef SDWORD                QDWORD;
typedef QWORD                 Q15;
/* ------------------- End of DSP data type definition --------- */

/* ----------- Complex number data type definition ------------- */
typedef struct
{
    QWORD r;
    QWORD i;
} CQWORD;

typedef struct
{
    QDWORD r;
    QDWORD i;
} CQDWORD;
/* ----------- End of Complex number data type definition -------- */

#define NUM_DRIVER_PARAMS     (10)
typedef struct
{
    /* From modem */
    void   *pReserve;
    UDWORD  DriverFunc;
    UWORD   FunctionReq;
    UWORD   ReserveSize;
    UWORD   TXGain;
    UWORD   RXGain;
    UBYTE   LineInUse;
    UBYTE   ParallelPickUp;
    UBYTE   CountryCode;
    UBYTE   TelephonyDeviceState;
    UBYTE   ParameterToDriver[NUM_DRIVER_PARAMS];

    UBYTE   StateChange;
    UBYTE   HookState;
    UBYTE   BufferSize;
    UBYTE   SampRate;
    UBYTE   CidState;
    UBYTE   ConfirmBufferSize;

    UBYTE   ModemState;

    UBYTE   TX_RATE;
    UBYTE   RX_RATE;

    UBYTE   ubFlowType;
    UBYTE   CarrierDetect_Flag;
    UBYTE   ubSerPortRate;

    /* To modem */
    UBYTE   ModemCmd;
    UBYTE   RingDetected;/* 0: No ring detected by user's program; 1: Ring detected by user's program */

    UBYTE   ubDebugSR;
    UBYTE   RetrainRerate_flag;
} GAO_ModStatus;


/* sampling rates. only for module process */
#define HWAPI_7200SR         (0) /* 7200  sampling rate */
#define HWAPI_9000SR         (1) /* 9000  sampling rate */
#define HWAPI_9600SR         (2) /* 9600  sampling rate */
#define HWAPI_8229SR         (3) /* 8229  sampling rate */
#define HWAPI_8400SR         (4) /* 8400  sampling rate */
#define HWAPI_10287SR        (5) /* 10287 sampling rate */
#define HWAPI_8000SR         (6) /* 8000  sampling rate */
#define HWAPI_16000SR        (7) /* 16000 sampling rate */

/* On hook and off hook define */
#define ON_HOOK              (0) /* On  hook */
#define OFF_HOOK             (1) /* Off hook */

/* Cid state definition */
#define CID_ON               (0) /* Cid on  */
#define CID_OFF              (1) /* Cid off */

/* StateChange definition, bitwise */
#define HOOK_STATE_CHG       (0x01)
#define BUFFER_SIZE_CHG      (0x02)
#define SAMPLE_RATE_CHG      (0x04)
#define CID_STATE_CHG        (0x08)

/* Driver Functions definition */
#define HOOK_SUPPORT                        (1<<0)          /* support hook */
#define SOFT_RINGDETECTION_SUPPORT          (1<<1)  /* support software ringdetection */
#define HARD_RINGDETECTION_SUPPORT          (1<<2)  /* support hardware ringdetection */
#define ONHOOK_LINE_SIGNAL_PCM_SUPPORT      (1<<3)  /* support onhook line monitor (get PCM data even onhook) */
#define ONHOOK_LINE_SIGNAL_REG_SUPPORT      (1<<4)  /* support onhook line signal register read */
#define HARD_LINEINUSE_DETECTION_SUPPORT    (1<<5)  /* support line in use base on line voltage */
#define SOFT_LINEINUSE_DETECTION_SUPPORT    (1<<6)  /* support line in use base on software */
#define HARD_PPU_DETECTION_SUPPORT          (1<<7)  /* support parallel pick up base on line current */
#define SOFT_PPU_DETECTION_SUPPORT          (1<<8)  /* support parallel pick up base on software */
#define MULTI_COUNTEY_IMPEDANCE_SUPPORT     (1<<9)  /* support multicountry impedance */
#define HW_SAMPLERATE_7200_SUPPORT          (1<<16) /* support 7200 samplerate */
#define HW_SAMPLERATE_8000_SUPPORT          (1<<17) /* support 8000 samplerate */
#define HW_SAMPLERATE_8229_SUPPORT          (1<<18) /* support 8229 samplerate */
#define HW_SAMPLERATE_8400_SUPPORT          (1<<19) /* support 8400 samplerate */
#define HW_SAMPLERATE_9000_SUPPORT          (1<<20) /* support 9000 samplerate */
#define HW_SAMPLERATE_9600_SUPPORT          (1<<21) /* support 9600 samplerate */
#define HW_SAMPLERATE_10286_SUPPORT         (1<<22) /* support 10286 samplerate */
#define HW_SAMPLERATE_16000_SUPPORT         (1<<23) /* support 16000 samplerate */
#endif
