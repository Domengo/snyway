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

#ifndef _SHAREDEF_H
#define _SHAREDEF_H

#include "commdef.h"
#include "modifdef.h"

#define TX_PRECODE_ENABLE         (1)
#define RX_PRECODE_ENABLE         (0)

#define V34_PRECODE_COEF_LEN      (3)

#define V34AUXCH                  (0)

/* #define V34_PP_STARTUP_LEVEL               (QDWORD)676070 */
/* #define V34_SYMBOL_SCALE_DOWN                         512 */
/* QQMULQ15(V34_PP_STARTUP_LEVEL, V34_SYMBOL_SCALE_DOWN) = 10564 */
#define V34_PP_STARTUP            (10564)

#define V34_SYM_SIZE              (3)   /* Number of samples per symbol */
#define V34_SYM_BUF_SIZE          (32)  /* Have to be power of 2, good for circ-buf */

#define V34_PSF_DELAY_LEN         (17)  /* (V34_TXFIR_LENGTH(51)/V34_SYM_SIZE) */

#define V34_SCRAM_BUF_SIZE        (79)

#define V34_SYM_BUF_MASK          (0x1F)

#define V34_4_POINT_TRN           (4)
#define V34_16_POINT_TRN          (16)
#define V34_PI_POINT_TRN          (0x7F)

#define V34_RX_SYM_BIT_4POINT     (2) /* 2 bits per symbol due to 4 Points training */
#define V34_RX_SYM_BIT_16POINT    (4) /* 4 bits per symbol due to 16 Points training */

#define UMASK                     (0x1F) /*Mask for upper 5 bits first */
#define LMASK                     (0xE0) /*Mask for left over bits,if input is 8 bits,3 left*/
#define RS                        (8)    /* Right shift 8 bits, since input is 8 bits */
#define LS                        (24)   /* Left shift the remaining, for output aligning, 24bit */

#define MP_TYPE_0                 (0)
#define MP_TYPE_1                 (1)

#if !USE_ASM
#define QNLCONST_OVER_6        (16421)/*  (0.3125 / 0.10393217) / 6      */
#define QNLCONST_OVER_120      (2469) /* ((0.3125 / 0.10393217)^2 ) / 6  */

#define qNlGain                   (6159) /* (g/R^2)/2, (0.3125/0.10393217)/2 ->Q4.12 */
#define qNlDecoderC1              (10724)/* 1/6 * 2 */
#define qNlDecoderC2              (7696 << 1) /* C2' * 4 */
#define qNlDecoderC3              (3536 << 2) /* C3' * 8 */
#endif

#endif
