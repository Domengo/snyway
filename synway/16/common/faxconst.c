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

#include "common.h"
#include "faxapi.h"

CONST UBYTE T30_BytesPer10ms[] = {3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42};

#if SUPPORT_T30 || SUPPORT_DTE
CONST UWORD ImageWidth_Tiff[3][8] =
{
    {TIFF_A4_WIDTH_R8, TIFF_A4_WIDTH_R16, TIFF_A4_WIDTH_100DPI, TIFF_A4_WIDTH_200DPI, TIFF_A4_WIDTH_300DPI, TIFF_A4_WIDTH_400DPI, TIFF_A4_WIDTH_600DPI, TIFF_A4_WIDTH_1200DPI,},
    {TIFF_B4_WIDTH_R8, TIFF_B4_WIDTH_R16, TIFF_B4_WIDTH_100DPI, TIFF_B4_WIDTH_200DPI, TIFF_B4_WIDTH_300DPI, TIFF_B4_WIDTH_400DPI, TIFF_B4_WIDTH_600DPI, TIFF_B4_WIDTH_1200DPI,},
    {TIFF_A3_WIDTH_R8, TIFF_A3_WIDTH_R16, TIFF_A3_WIDTH_100DPI, TIFF_A3_WIDTH_200DPI, TIFF_A3_WIDTH_300DPI, TIFF_A3_WIDTH_400DPI, TIFF_A3_WIDTH_600DPI, TIFF_A3_WIDTH_1200DPI},
};
#endif

#if TRACEON || SUPPORT_T31_PARSER
CONST CHAR *T31_Action_Command[] =
{
    "NUL",
    "ATD",
    "ATA",
    "ATH",
    "ATO",
    "AT+FTS",
    "AT+FRS",
    "AT+FTM",
    "AT+FRM",
    "AT+FTH",
    "AT+FRH",
    "AT+A8M",
};
#endif

#if TRACEON || SUPPORT_T32_PARSER
CONST CHAR *T32_Action_Command[] =
{
    "NUL",
    "ATD",
    "ATA",
    "AT+FDT",
    "AT+FDR",
    "AT+FIP",
    "AT+FPS",
};

CONST CHAR PLUS_F[] = "+F";

CONST CHAR *T32_Info_Txt[] =
{
    "CO",
    "CS:",
    "IS:",
    "TC:",
    "TI:",
    "CI:",
    "PI:",
    "NF:",
    "NS:",
    "NC:",
    "SA:",
    "PA:",
    "PW:",
    "PS:",
    "ET:",
    "PS:",
    "HS:",
    "PO",
    "HT:",
    "HR:",
};
#endif

#if SUPPORT_DTE
#if SUPPORT_PARSER
CONST CHAR AT_PLUS_F[] = "AT+F";
#endif

#if SUPPORT_PARSER
CONST CHAR DTE_DCE_INIT_PARM[] = "ATE0S0=0";
#endif

#if SUPPORT_T32_PARSER
CONST CHAR *T32_Parm_Command[] =
{
    "CLASS",
    "CC",
    "IS",
    "CS",
    "LI",
    "PI",
    "LP",
    "SP",
    "NR",
    "IE",
    "PS",
    "CQ",
    "IT",
    "PP",
    "BO",
    "EA",
    "MI",
    "MM",
    "MR",
    "PR",
    "LO",
    "NS",
    "CR",
    "BU",
    "AP",
    "RQ",
    "AA",
    "CT",
    "HS",
    "RY",
    "MS",
    "ND",
    "FC",
    "BS",
};
#endif
#endif

