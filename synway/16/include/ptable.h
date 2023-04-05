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

/******************************************************************************
        Description :
                Indices to global memory for data structure
 *****************************************************************************/

#ifndef _PTABLE_H
#define _PTABLE_H

#include "commdef.h"

/* pTable index defines */
#define MHSP_STRUC_IDX          (0)
#define DPCS_STRUC_IDX          (1)
#define CLOCKDATA_IDX           (2)

#define ACE_STRUC_IDX           (3)
#define ACESREGDATA_IDX         (4)
#define HOSTMSGDATA_IDX         (5)

#define HW_STRUC_IDX            (6)
#define HWAPIDATA_IDX           (7)
#define PCMINDATA_IDX           (8)
#define PCMOUTDATA_IDX          (9)
#define DC_STRUC_IDX            (10)

#define DCE_STRUC_IDX           (11)

#define EPGDATA_IDX             (12)

#define DSPIORDBUFDATA_IDX      (13)
#define DSPIOWRBUFDATA_IDX      (14)

#define ASCCBWRDATA_IDX         (15)
#define ASCCBRDDATA_IDX         (16)
#define ASCTXDATA_IDX           (17)
#define ASCRXDATA_IDX           (18)
#define DTETXDATA_IDX           (19)
#define DTERXDATA_IDX           (20)

#define DTERDBUF_IDX            (21)
#define DTEWRBUF_IDX            (22)

#define DCE_SHAREDATA_IDX       (23)
#define MNP_STRUC_IDX           (DCE_SHAREDATA_IDX)
#define V42_STRUC_IDX           (DCE_SHAREDATA_IDX)
#define ASC_STRUC_IDX           (DCE_SHAREDATA_IDX)

#define MODEM_SHAREDATA_IDX     (24)
#define CID_STRUC_IDX           (MODEM_SHAREDATA_IDX)
#define CIDGEN_STRUC_IDX        (MODEM_SHAREDATA_IDX)
#define DTMF_STRUC_IDX          (MODEM_SHAREDATA_IDX)
#define V21_STRUC_IDX           (MODEM_SHAREDATA_IDX)
#define V17_STRUC_IDX           (MODEM_SHAREDATA_IDX)
#define V27_STRUC_IDX           (MODEM_SHAREDATA_IDX)
#define V29_STRUC_IDX           (MODEM_SHAREDATA_IDX)
#define CP_STRUC_IDX            (MODEM_SHAREDATA_IDX)
#define AUTO_STRUC_IDX          (MODEM_SHAREDATA_IDX)
#define PH2_STRUC_IDX           (MODEM_SHAREDATA_IDX)
#define B103_STRUC_IDX          (MODEM_SHAREDATA_IDX)
#define V23_STRUC_IDX           (MODEM_SHAREDATA_IDX)
#define V34_STRUC_IDX           (MODEM_SHAREDATA_IDX)
#define V90A_STRUC_IDX          (MODEM_SHAREDATA_IDX)
#define V90D_STRUC_IDX          (MODEM_SHAREDATA_IDX)

#define MODEM_OFFSET_AUTO_IDX   (25)
#define V8_STRUC_IDX            (MODEM_OFFSET_AUTO_IDX)
#define V22_STRUC_IDX           (MODEM_OFFSET_AUTO_IDX)
#define V32_STRUC_IDX           (MODEM_OFFSET_AUTO_IDX)

#define FAXSHAREMEM_IDX         (26)
#define V54_STRUC_IDX           (27)

#define SMPDATAIN_IDX           (28)
#define SMPDATAOUT_IDX          (29)

#define T30_STRUC_IDX           (30)
#define T31_STRUC_IDX           (31)
#define T32_STRUC_IDX           (32)
#define V42INFO_STRUC_IDX       (33)

#if USE_ASM
#define CIRCDATA_IDX            (34)
#define PTABLE_SIZE             (36)/* Must be Even number */
#else
#define PTABLE_SIZE             (34)/* Must be Even number */
#endif

#endif
