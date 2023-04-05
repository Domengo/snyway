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

/*****************************************************************************
*
*      Description : This is a general header file for DPCS (Data Pump
*                    Control & Status) containing bit constants and
*                    function macros.
*
*      Authors :  Thomas PAUL
*
*      Comments : It is highly recommened to use macros to access the
*                 DPCS registers. Please refer to proper document for
*                 DPCS registers specificaitons.
*****************************************************************************/

#ifndef _DPCS_H
#define _DPCS_H

#include "commdef.h"

typedef struct
{
    /* Modem Control Registers */
    UBYTE MCR0;
    UBYTE MCR1;
    UBYTE MMR0;

    UBYTE MBR;
    UBYTE MBC0;
    UBYTE MBC1;
    UBYTE MCF;

    UBYTE MFCF0;
    UBYTE MTC;

    /* Modem Status Registers */
    UBYTE MSR0;
    UBYTE MSR1;

    UBYTE MFSR0;
    UBYTE MMSR0;

    UBYTE MMTR; /* Modem maximum throughput (chars per call) */
    /* Used by V42/V42bis for MIPS control */
    UBYTE MBSR;

    UBYTE MBSC;
    UBYTE MBSCTX;

    UBYTE TONESR;
    UBYTE MTONESR;
    UBYTE DTMFR;

    UBYTE ABORTSR;

    UBYTE DEBUGSR; /* Modem debug info */

    UBYTE LECR;
    UBYTE AECR;

    UBYTE TELCOCR1;

    SWORD RTD;     /* RTD in msec reported by data pump */

#if SUPPORT_ENERGY_REPORT
    QDWORD ENERGY;
#endif
    UBYTE DIALEND;  // 1: indicates that dialing number is finished, temporarily
    UWORD BITSPERCYL;
#if SUPPORT_V251
    UBYTE V8OPCNTL;
    UBYTE V8CF;
#endif
} DpcsStruct;

/* -----===== Dpcs masks =====----- */

/************ MCR0 **************/
#define V25AT                 BIT7
#define RDLENA                BIT6
#define GTONE                 BIT5
#define RTRND                 BIT4

#if SUPPORT_V54
#define TEST_MODE             BIT3
#endif

#define ANSWER                BIT2/* Changed the name from ORG to ANSWER to correct the concept for BIT2 of pDpcs->MCR0. It is answer-mode when it is set, it is call-mode when it is reset. */
#define RECEIVE_IMAGE         BIT1
#define FAX                   BIT0
/********************************/

/************ MCR1 **************/
#define MNP                   (BIT7|BIT6)
#define V42BIS                BIT5
#define V42                   BIT4
#define PULSE_MODE            BIT3
#define STRN                  BIT2
#define CHG_MOD               BIT1
#define AUTOBELL              BIT0
/********************************/

/********* MMR0 & MMSR0 *********/
#define CP                    BIT7 /* MMR0 only */
#define MODULATION            (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define AUTOMODE              ( 0)/* for MMR0 */
#define MODEM_NULL            ( 0)/* for MMSR0 */

#define BELL202               ( 1)
#define MODEM_V21             ( 2)
#define MODEM_V23             ( 3)
#define MODEM_V22             ( 4)
#define MODEM_V22bis          ( 5)
#define MODEM_V32             ( 6)
#define MODEM_V32bis          ( 7)
#define MODEM_V34             ( 8)
#define MODEM_V90             ( 9)
#define BELL103               (10)
#define BELL212A              (11)

#define V21CH2                (12)
#define V29                   (13)
#define V27TER                (14)
#define V17                   (15)
#define V34FAX                (16)
#define MODEM_V92             (17)
/********************************/

/******************** MBR *******************/
#define MIN_CAR_FREQ          BIT7
#define MAX_CAR_FREQ          BIT6
#define MIN_BAUD_RATE         (BIT5|BIT4|BIT3)
#define MAX_BAUD_RATE         (BIT2|BIT1|BIT0)
/********************************************/

/********************** MBC0 **************************/
#define MIN_RATE              (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)
/******************************************************/

/********************** MBC1 **************************/
#define MAX_RATE              (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)
/******************************************************/

/************* MCF **************/
#define LINE_DISCONNECT       BIT7
#define DIAL                  BIT6
#define DISCONNECT            BIT5
#define CONNECT               BIT4
#define RRATE                 BIT3
#define RTRN                  BIT2
#define EC_DISC               BIT1/* V42/MNP Error Correct disconnect */
#define V42SKIPDETECT         BIT0
/********************************/

/************* MFCF0 *************/
#define CARRIER_GOT           BIT3
#define TX_RX                 BIT2 /* MFCF0 only */
#define TCF_OR_IMAGE          BIT1/* Only for SUPPORT_FAX */
/* 0: long  train for V17, TCF train  mode for V29 & V27; */
/* 1: short train for V17, image data mode for V29 & V27; */
#define CHAR_OR_HDLC          BIT0
/*********************************/

/********      MFSR0 *************/
#if SUPPORT_ENERGY_REPORT
#define FAX_DETECT            BIT3 /* Report V21/V17/V27/V29 detected */
#endif
#define FAX_V21               BIT2 /* Detected V21 when detect V17/V27/V29 */
#define V42_HDLC              BIT1 /* V.42 in HDLC mode */
#define FAX_HDLC              BIT0
/*********************************/

#if SUPPORT_V54
/************* MTC **************/
#define LAL_MODE              BIT3
#define LDL_MODE              BIT2
#define RAL_MODE              BIT1
#define RDL_MODE              BIT0
/********************************/
#endif

/************ MSR0 **************/
#define RING_DET              BIT7
#define DATA                  BIT6
#define TRAIN                 BIT5
#define REMOTE_EC_DISC        BIT4
#define LINE_DISC_DET         BIT3
#define RTRN_DET              BIT2
#define RRATE_DET             BIT1
#define DISC_DET              BIT0
/********************************/

/************ MSR1 **************/
#define MNP_DATA1             BIT7
#define MNP_DATA              BIT6
#define MNP_TRAIN             BIT5
#define MNP_FAIL              BIT4
#define V42bis_DATA           BIT3
#define V42_DATA              BIT2
#define V44_DATA              BIT1
#define V42_FAIL              BIT0
/********************************/

/****************** MBSR ********************/
#define RDL_ACK               BIT4
#define CAR_FREQ              BIT3
#define BAUD_RATE             (BIT2|BIT1|BIT0)
/********************************************/

/********************** MBSC & MBSCTX***********************/
#define BIT_RATE              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)
/***********************************************************/

/*********** TONESR **************/
#define TONE8_DET             BIT7
#define NOANS_DET             BIT6
#define TONE6_DET             BIT5
#define TONE5_DET             BIT4
#define TONE4_DET             BIT3
#define TONE3_DET             BIT2
#define TONE2_DET             BIT1
#define TONE1_DET             BIT0
/*********************************/

/*********** MTONESR ************/
#define MTONE6_V8DET          BIT5
#define MTONE5_BellModem      BIT4
#define MTONE4_V22S1SB1NODET  BIT3
#define MTONE3_V22S1SB1DET    BIT2
#define MTONE2_DET            BIT1
#define MTONE1_DET            BIT0
/*********************************/

/******************* DTMFR ***********************/
#define DTMF_ENABLE                          (BIT7)    /* control bit */
/*************************************************/

/******************* LECR ************************/
#define LEC_ACTIVE                           (BIT0)    /* status bit  */
#define LEC_UPDATE                           (BIT1)    /* status bit  */
#define LEC_FREEZE                           (BIT5)    /* control bit */
#define LEC_RESTART                          (BIT6)    /* control bit */
#define LEC_ENABLE                           (BIT7)    /* control bit */
/*************************************************/

/******************* AECR ************************/
#define AEC_FREEZE                           (BIT5)    /* control bit */
#define AEC_RESTART                          (BIT6)    /* control bit */
#define AEC_ENABLE                           (BIT7)    /* control bit */
/*************************************************/

/****************** ABORTSR **********************/
#define REASON                (BIT3|BIT2|BIT1|BIT0)
#define REASON_REMOTE_EC_HANGUP                 (4)
#define REASON_LOCAL_EC_HANGUP                  (5)
/*************************************************/

/****************** TELCOCR1 *********************/
#define CID_CONTROL                     (BIT7|BIT6)
#define PULSE_CONTROL                   (BIT5|BIT4)
#define ONOFF_HOOK                           (BIT3)
#define LINECOMP                             (BIT2)
#define CIDGEN_CONTROL                  (BIT1|BIT0)
/*************************************************/

#endif
