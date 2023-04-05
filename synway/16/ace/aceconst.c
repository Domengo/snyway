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

#include "aceext.h"
#include "cpdef.h"
#include "v8def.h"

#if SUPPORT_PARSER
#if SUPPORT_MODEM
CONST CHAR RateCode[BIT_RATE_NUM][6] =
{
    "75",
    "300",
    "600",
    "1200",
    "2400",
    "4800",
    "7200",
    "9600",
    "12000",
    "14400",
    "16800",
    "19200",
    "21600",
    "24000",
    "26400",
    "28800",
    "31200",
    "33600",
    "24000",
    "25333",
    "26666",
    "28000",
    "29333",
    "30666",
    "32000",
    "33333",
    "34666",
    "36000",
    "37333",
    "38666",
    "40000",
    "41333",
    "42666",
    "44000",
    "45333",
    "46666",
    "48000",
    "49333",
    "50666",
    "52000",
    "53333",
    "54666",
    "56000"
};

CONST CHAR ModulationCode[12][10] =
{
    "/Vxx",
    "/V21",
    "/V23",
    "/V22",
    "/V22BIS",
    "/V32",
    "/V32BIS",
    "/V34",
    "/V90",
    "/BELL103",
    "/BELL212A",
    "/V92"
};

CONST CHAR ProtocolCode[9][13] =
{
    "/NONE",
    "/LAPM/V42",
    "/V42/FAIL",
    "/LAPM/V42BIS",
    "/MNP4",
    "/MNP/FAIL",
    "/MNP3",
    "/MNP2",
    "/LAPM/V44"
};
#endif

/* AT command list, each letter represents one command */
CONST CHAR AtCommandName_UpCase [AT_COMMAND_NAME_LEN + 1] = "ADSHZOIEQVXLMPT&+";

#if SUPPORT_T31_PARSER
/* Class 1 FAX FTM/FRM modulation code, from V.21 300bps . V17 14400bps */
CONST CHAR Class1Mod[] = "3,24,48,72,73,74,96,97,98,121,122,145,146";
#endif

#endif

/* Default S-registers values */
CONST UBYTE SRegDefVal[NUMBER_S_REGISTERS] =
{
    0,                        /* s0: ring count for auto-answer (0 - disable)        */
    0,                        /* s1: reserved                                        */
    43,                        /* s2: escape sequence (+)                            */
    13,                        /* s3: command line termination char (CR)            */
    10,                        /* s4: response formatting char (LF)                */
    8,                        /* s5: command line editing char (BS)                */
    2,                        /* s6: pause before blind dialing                    */
    122,                    /* s7: wait time for carrier--time out                */
    2,                        /* s8: length of dialing pause                        */
    0,                        /* s9: reserved                                        */
    14,                        /* s10: wait time for carrier absence                */
    70,                        /* s11: duration and spacing for tone dialing        */
    50,                        /* s12: Escape Guard Time                            */
    10,                        /* s13: dial-tone detect timeout                    */
    0,                        /* s14:                                                */
    0,                        /* s15:                                                */
    0,                        /* s16: For V.54                                    */
    0,                        /* s17:                                                */
    0,                        /* s18: V54_SELF_TIMER                                */
    0,                        /* s19:                                                */
    0,                        /* s20:                                                */
    0x02,                    /* s21: AUTO_RETRAIN_CONTROL                        */
    0,                        /* s22:                                                */
    0,                        /* s23: V54_RESPONSE_CONTROL                        */
    0,                        /* s24:                                                */
    0,                        /* s25:                                                */
    0,                        /* s26:                                                */
    0,                        /* s27:                                                */
    0,                        /* s28:                                                */
    0,                        /* s29:                                                */
    MU_LAW,                    /* s30: V90D_PCM_CODING                                */
    0,                        /* s31:                                                */
    0,                        /* s32:                                                */
    0xC5,                    /* s33: V34_SYMBOL_RATE                                */
    12,                        /* s34: TRANSMIT_POWER                                */
    CP_CANADA,                /* s35: CP_COUNTRY_CODE                                */
    9,                        /* s36: DTMF_TRANSMIT_POWER                            */
    3,                        /* s37: V42BIS_COMPRESSION_DIR                        */
    V42B_DICT_LARGE,        /* s38: V42BIS_DICTIONARY_SIZE                        */
    V42B_STRING_LARGE,        /* s39: V42BIS_MAX_STRING_LEN                        */
    0,                        /* s40: V92_OPTIONS                                    */
    0,                        /* s41: DOWNSTREAM_POWER_INDEX                        */
    0,                        /* s42:    RESERVED FOR FUTURE USE                        */
    0,                        /* s43:    RESERVED FOR FUTURE USE                        */
    0,                        /* s44:    RESERVED FOR FUTURE USE                        */
    0,                        /* s45:    RESERVED FOR FUTURE USE                        */
    0,                        /* s46:    RESERVED FOR FUTURE USE                        */
    0,                        /* s47:    RESERVED FOR FUTURE USE                        */
    0,                        /* s48:    RESERVED FOR FUTURE USE                        */
    0,                        /* s49:    RESERVED FOR FUTURE USE                        */
    0,                        /* s50:    RESERVED FOR DRIVER                            */
    0,                        /* s51:    RESERVED FOR DRIVER                            */
    0,                        /* s52:    RESERVED FOR DRIVER                            */
    0,                        /* s53:    RESERVED FOR DRIVER                            */
    0,                        /* s54:    RESERVED FOR DRIVER                            */
    0,                        /* s55:    RESERVED FOR DRIVER                            */
    0,                        /* s56:    RESERVED FOR DRIVER                            */
    0,                        /* s57:    RESERVED FOR DRIVER                            */
    0,                        /* s58:    RESERVED FOR DRIVER                            */
    0,                        /* s59:    RESERVED FOR DRIVER                            */
    0,                        /* s60:    RESERVED FOR DRIVER                            */
    0,                        /* s61:    RESERVED FOR DRIVER                            */
    0,                        /* s62:    RESERVED FOR DRIVER                            */
    0,                        /* s63:    RESERVED FOR DRIVER                            */
};

#if TRACEON
CONST CHAR *Class_State[] =
{
    "NOF",
    "FTS",
    "FRS",
    "FTM",
    "FRM",
    "FTH",
    "FRH",
    "A8M",
    "TX1S",
    "P2S",
    "S2C",
    "ATH",
    "A8E",
    "FRH_HS",
    "FTH_HS",
};

CONST CHAR *Line_State[] =
{
    "OFF_LINE",
    "ONLINE_COMMAND",
    "ONLINE_DATA",
    "ONLINE_FAX_COMMAND",
    "ONLINE_FAX_DATA",
    "ONLINE_RETRAIN",
    "ONLINE_RERATE",
    "CALL_PROGRESS",
    "MODEM_TRAINING",
    "GOING_OFF_LINE",
};

#endif