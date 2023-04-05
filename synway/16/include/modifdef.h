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

#ifndef _MODIFDEF_H
#define _MODIFDEF_H

#include "commdef.h"

#define DUALMODEMSIM             (0)

#if DUALMODEMSIM/*************************************************/
#define HAWK_ADI                 (0)/* fix */
#define HAWK_SLAB                (0)/* fix */
#define HAWK_ST7550              (0)/* fix */
#define T1FRAMER                 (0)/* fix */

extern  UBYTE ModemAorB;

#else/************************************************************/

#define SUPPORT_PCI_BOARD        (0)
#define SUPPORT_HAWKBOARD        (!SUPPORT_PCI_BOARD)/* fix */

#if SUPPORT_PCI_BOARD/**************/

#define HAWK_ADI                 (0)/* fix */
#define HAWK_SLAB                (1)/* fix */
#define HAWK_ST7550              (0)/* fix */
#define T1FRAMER                 (0)/* fix */

#else/* SUPPORT_HAWKBOARD************/

/* Attention: HAWK_ADI + HAWK_SLAB + HAWK_ST7550 = 1 */
#define HAWK_ADI                 (1)
#define HAWK_SLAB                (0)
#define HAWK_ST7550              (0)
#define T1FRAMER                 (0)/* It must be 1 when run at T1 digital channel */

#endif/*****************************/

#endif/***********************************************************/

/* system delay */
#if HAWK_ADI
#if SAMCONV
#define V32_SYS_DELAY            (50)
#else
#define V32_SYS_DELAY            (45)
#endif
#elif HAWK_SLAB
#if SAMCONV
#define V32_SYS_DELAY            (47)
#else
#define V32_SYS_DELAY            (45)
#endif
#elif HAWK_ST7550
#define V32_SYS_DELAY            (39)
#elif DUALMODEMSIM
#if SAMCONV
#define V32_SYS_DELAY            (53)
#else
#define V32_SYS_DELAY            (55)
#endif
#endif

#if T1FRAMER

#define V34_PHASE2_SYS_DELAY     (21)

#else

#if HAWK_ADI
#if SAMCONV
#define V34_PHASE2_SYS_DELAY     (21)
#else
#define V34_PHASE2_SYS_DELAY     (18)
#endif
#elif HAWK_SLAB
#if SAMCONV
#define V34_PHASE2_SYS_DELAY     (19)
#else
#define V34_PHASE2_SYS_DELAY     (18)
#endif
#elif HAWK_ST7550
#if SAMCONV
#define V34_PHASE2_SYS_DELAY     (18)
#else
#define V34_PHASE2_SYS_DELAY     (16)
#endif
#elif DUALMODEMSIM
#if SAMCONV
#define V34_PHASE2_SYS_DELAY     (20)
#else
#define V34_PHASE2_SYS_DELAY     (10)
#endif
#endif

#endif

#if HAWK_ADI
#if SAMCONV
#define V34_SYM2400_SYS_DELAY    (55)
#define V34_SYM2743_SYS_DELAY    (77)
#define V34_SYM2800_SYS_DELAY    (75)
#define V34_SYM3000_SYS_DELAY    (72)
#define V34_SYM3200_SYS_DELAY    (81)
#define V34_SYM3429_SYS_DELAY    (80)
#else
#define V34_SYM2400_SYS_DELAY    (53)
#define V34_SYM2743_SYS_DELAY    (61)
#define V34_SYM2800_SYS_DELAY    (61)
#define V34_SYM3000_SYS_DELAY    (65)
#define V34_SYM3200_SYS_DELAY    (69)
#define V34_SYM3429_SYS_DELAY    (69)
#endif
#elif HAWK_SLAB
#if SAMCONV
#define V34_SYM2400_SYS_DELAY    (54)
#define V34_SYM2743_SYS_DELAY    (76)
#define V34_SYM2800_SYS_DELAY    (74)
#define V34_SYM3000_SYS_DELAY    (71)
#define V34_SYM3200_SYS_DELAY    (80)
#define V34_SYM3429_SYS_DELAY    (79)
#else
#define V34_SYM2400_SYS_DELAY    (52)
#define V34_SYM2743_SYS_DELAY    (60)
#define V34_SYM2800_SYS_DELAY    (60)
#define V34_SYM3000_SYS_DELAY    (64)
#define V34_SYM3200_SYS_DELAY    (68)
#define V34_SYM3429_SYS_DELAY    (68)
#endif /* SAMCONV */
#elif HAWK_ST7550
#define V34_SYM2400_SYS_DELAY    (47)
#define V34_SYM2743_SYS_DELAY    (55)
#define V34_SYM2800_SYS_DELAY    ( 0)/* HAWK_ST7550 unsupported 2800 */
#define V34_SYM3000_SYS_DELAY    (59)
#define V34_SYM3200_SYS_DELAY    (63)
#define V34_SYM3429_SYS_DELAY    (63)
#elif DUALMODEMSIM
#if SAMCONV
#define V34_SYM2400_SYS_DELAY    (62)
#define V34_SYM2743_SYS_DELAY    ( 0)
#define V34_SYM2800_SYS_DELAY    ( 0)
#define V34_SYM3000_SYS_DELAY    (81)
#define V34_SYM3200_SYS_DELAY    (90)
#define V34_SYM3429_SYS_DELAY    (94)
#else
#define V34_SYM2400_SYS_DELAY    (63)
#define V34_SYM2743_SYS_DELAY    (67)
#define V34_SYM2800_SYS_DELAY    (67)
#define V34_SYM3000_SYS_DELAY    (69)
#define V34_SYM3200_SYS_DELAY    (71)
#define V34_SYM3429_SYS_DELAY    (71)
#endif
#endif

#define V90A_SYS_DELAY           (32)

#if T1FRAMER
#define V90D_SYS_DELAY           (51)/* for 40 buffer size(5ms):51. for 60 buffer size(7.5ms):?. */
#elif DUALMODEMSIM
#define V90D_SYS_DELAY           (32)/* for ?  buffer size(5ms):32. for 60 buffer size(7.5ms):?. */
#else
#define V90D_SYS_DELAY           (26)/* for 40 buffer size(5ms):26. for 60 buffer size(7.5ms):46. */
#endif

#endif
