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

/**********************************************************************
*      File Name : COMMMAC.H                                          *
*                                                                     *
*      Description : This is a general header file for all the        *
*                    useful macros.                                   *
*                                                                     *
*      References : (1) Fixed point arithmetic                        *
*                       Appendix C, ADSP-2100 Family User's Manual    *
*                   (2) TI application books                          *
**********************************************************************/

#ifndef _COMMMAC_H
#define _COMMMAC_H

#include "commdef.h"

#define UBUBMULUB(a,b)        ((UBYTE)((UBYTE)(a) * (UBYTE)(b)))
#define UBUBMULU(a,b)         ((UWORD)((UBYTE)(a) * (UBYTE)(b)))
#define SBQMULQ(a,b)          ((QWORD)((SBYTE)(a) * (QWORD)(b)))
#if defined(__TMS320C55X__)
#define QQMULQD(a,b)          ((QDWORD)((QDWORD)(a) * (QWORD)(b)))
#define UQMULQD(a,b)          ((QDWORD)((UDWORD)(a) * (QWORD)(b)))
#define UUMULUD(a,b)          ((UDWORD)((UDWORD)(a) * (UWORD)(b)))
#define QDQMULQD_HI(a,b)      ((QDWORD)((QDWORD)((a) >> 16    ) * (QWORD)(b)))
#else
#define QQMULQD(a,b)          ((QDWORD)((QWORD)(a) * (QWORD)(b)))
#define UQMULQD(a,b)          ((QDWORD)((UWORD)(a) * (QWORD)(b)))
#define UUMULUD(a,b)          ((UDWORD)((UWORD)(a) * (UWORD)(b)))
#define QDQMULQD_HI(a,b)      ((QDWORD)((QWORD)((a) >> 16    ) * (QWORD)(b)))
#endif
#if defined (BLACKFIN_BUILD) || defined (__TMS320C55X__)
#define QDQMULQD_LO(a,b)      ((QDWORD)((UDWORD)((a) &  0xFFFF) * (QWORD)(b)))
#else
#define QDQMULQD_LO(a,b)      ((QDWORD)((UWORD)((a) &  0xFFFF) * (QWORD)(b)))
#endif

#define QQMULQDR7(a, b)       (         (QQMULQD(a,b) + 0x0040) >> 7  )
#define UBUBMULUBR8(a, b)     ((UBYTE)( (UBUBMULU(a,b)+ 0x0080) >> 8 ))
#define QQMULQR8(a, b)        ((QWORD)( (QQMULQD(a,b) + 0x0080) >> 8 ))  /* (QWORD)(Round(QWORD * Q8)) */
#define QQMULQ8(a, b)         ((QWORD)(  QQMULQD(a,b)           >> 8 ))  /* (QWORD)((QWORD * Q8)) */
#define QQMULQ15(a,b)         ((QWORD)(  QQMULQD(a,b)           >> 15))
#define QQMULQR15(a,b)        ((QWORD)( (QQMULQD(a,b) + 0x4000) >> 15))

#define UQMULQ15(a,b)         ((QWORD)(  UQMULQD(a,b)           >> 15))
#define UQMULQR15(a,b)        ((QWORD)( (UQMULQD(a,b) + 0x4000) >> 15))

#define QDQMULQD0(a,b)        ((QDQMULQD_HI(a,b) << 16) +   QDQMULQD_LO(a,b)                 )
#define QDQMULQDR7(a,b)       ((QDQMULQD_HI(a,b) << 9 ) + ((QDQMULQD_LO(a,b) + 0x0040) >>  7))
#define QDQMULQDR8(a,b)       ((QDQMULQD_HI(a,b) << 8 ) + ((QDQMULQD_LO(a,b) + 0x0080) >>  8))
#define QDQMULQD(a,b)         ((QDQMULQD_HI(a,b) << 1 ) +  (QDQMULQD_LO(a,b)           >> 15))
#define QDQMULQDR(a,b)        ((QDQMULQD_HI(a,b) << 1 ) + ((QDQMULQD_LO(a,b) + 0x4000) >> 15))
#define QDQMULQDR16(a,b)      ( QDQMULQD_HI(a,b)        + ((QDQMULQD_LO(a,b) + 0x8000) >> 16))

#define ROUND9(a)             (       ( (a) + 0x0100) >> 9  )
#define ROUND11(a)            (       ( (a) + 0x0400) >> 11 )
#define ROUND14(a)            (       ( (a) + 0x2000) >> 14 )
#define ROUND15(a)            (       ( (a) + 0x4000) >> 15 )
#define QD15Q(a)              ((QWORD)( (a)           >> 15))
#define QDR15Q(a)             ((QWORD)(((a) + 0x4000) >> 15))
#define QD16Q(a)              ((QWORD)( (a)           >> 16))
#define QDR16Q(a)             ((QWORD)(((a) + 0x8000) >> 16))

#define QABS(a)               (((a)<0)? -(a):(a))

/* ********************************************** */
/*             COMPLEX macros                     */
/* ********************************************** */
#define CQSUB(a,b,c) {        \
        (c).r = (a).r - (b).r;    \
        (c).i = (a).i - (b).i;    \
    }

#define CQPOW(a)        ( QQMULQD((a).r, (a).r) + QQMULQD((a).i, (a).i) )
#define CQMULr(a,b)     QDR15Q( QQMULQD((a).r, (b).r) - QQMULQD((a).i, (b).i) )
#define CQMULi(a,b)     QDR15Q( QQMULQD((a).r, (b).i) + QQMULQD((a).i, (b).r) )
#define CQMUL(a,b,c)    { (c).r = CQMULr((a), (b)); (c).i = CQMULi((a), (b)); }


/* -----===== Dpcs macros =====----- */
/* ---------------------------------------------------------------
usage for one bit value:
         SetReg(MCR0, ORG)   <- sets ORG bit on
         ResetReg(MCR0, ORG) <- sets ORG bit off
         RdReg (MSR0, DISC)  <- output is DISC
   ------------------------------------------------------------- */
#define WrReg(reg, bit_pos, val)  \
    {                                 \
        (reg) &= ~(bit_pos);           \
        (reg) |= (val) & (bit_pos);    \
    }

#define PutReg(reg, val)       ((reg) = (val))     /* WrReg(reg, 0xFF, val) */
#define SetReg(reg, bit_pos)   ((reg)|= (bit_pos)) /* WrReg(reg, bit_pos, 0xFF) */
#define ResetReg(reg, bit_pos) ((reg)&=~(bit_pos)) /* WrReg(reg, bit_pos, 0x00) */
#if defined (__TMS320C55X__)
#define ClrReg(reg)            ((reg) = (UBYTE)(0))/* clear all bits */ /* WrReg(reg, 0xFF, 0x00) */
#else
#define ClrReg(reg)            ((reg) = (0))       /* clear all bits */ /* WrReg(reg, 0xFF, 0x00) */
#endif

#define RdReg(reg, bit_pos)    (UBYTE)((reg) & (bit_pos))
INLINE UBYTE RdRegShft(reg, bit_pos)
{
    reg = RdReg(reg, bit_pos);

    while ((bit_pos & BIT0) == 0)
    {
        bit_pos >>= 1;
        reg >>= 1;
    }

    return reg;
}
#define GetReg(reg)            (reg)              /* RdReg(reg, 0xFF) */

#define ReadBit(i, j)          ((i) &   (1 << (j)))
#define SetBit(i, j)           ((i) |=  (1 << (j)))
#define ResetBit(i, j)         ((i) &= ~(1 << (j)))

#define MAX16(a, b)            ((a) >= (b) ? (a) : (b))
#define MIN16(a, b)            ((a) <= (b) ? (a) : (b))

#define MAX32(a, b)            ((a) >= (b) ? (a) : (b))
#define MIN32(a, b)            ((a) <= (b) ? (a) : (b))

/* Short form posfix convention:
* -----------------
* R -- Round QDWORD to QWORD
* T -- Truncate QDWORD to QWORD
* D -- keep result as Double word
* O -- shift to avoid result Overflow
-------------------------------------
* a,b single precision variable
* d,e double precision variable
*/

/* Multiplication with overflow
* ----------------------------
* d, e are either double or single precision.  Suppose the double may be
*      16 to 18 bits only.  Its more than QWORD but far from Q30
*      so we have to give up some precision to avoid overflow (if we keep
*      the result as double
*      however, when round/truncate back to QWORD, its still likely to have
*      overflow, so codes calling this macros might to have trouble!!
*/

/* ********************************************************************
*                       Usage of common macros
* ********************************************************************
*
* Type Notations:
* --------------
* Q     single precision,  same as QWORD for fixed point arithmetic
* QD    double precision,  same as Q30 for fixed point arithmetic
* QDD   64 bit precision,  only used for PC simulation
* Q131  Q1.31 (used with ASM fractional mode)
* Q88   Q8.8  (used in AGC calculation)
* Q97   Q9.7  (used in DFT calculation)
*
* a,b,c single precision arguments
* d,e,f double precision arguments
*
* Short form Postfix Notations
* ----------------------------
* R -- Round QDWORD to QWORD
* T -- Truncate QDWORD to QWORD
* D -- keep result as Double word
* O -- shift to avoid result Overflow, e.g.
*      O22 -- shift both arguments right by 2
*
* Input Output Precisions:
* -----------------------
* Generally, the inputs and output should be of same precision.
* That is, the output is default to single if (both) inputs are single.
* Truncation is assumed if no rounding is specified.
*
* If we want to keep the result as double precision, we have to use macros
* with _TO_QD postfix instead (or alternative short form)
*
* Normally double precision arguments should gives double precision
* result.  This is simple for addition and subtraction.  But it is not so
* for multiplicatin and division.
*
* We have macros for multiple precision multiplication, but they have never
* been called.  There are some "pseudo-" multi-precison macros
* created to avoid overflow in the intermediate
* product.  Use them with care to make sure that the final result is of
* appropriate precision figures.
*
* The division macros are more or less for demonstration purpose.  We have
* better to use library code instead.  However, take care that library code
* may modify the status bits like SXM, but without restoring them after exit.
*
* TI intrinsic macros:
* -------------------
* The defaults for TI intrinsic macros are QWORD, Q31 (likely with fractional
* mode on) and Saturation active.  This library code may change the
* CPU status (e.g., SXM and OVM) while without restoring them.
*
* Our defaults are QWORD, Q30 (no factional mode) and NO saturation.
*
* For efficiency, we only adopted a very few intrinsic macros.
*
* NEW MACROS
* ----------
* New macros are added for :
* 1   saturation
* 2   short form & unify the naming & style, especially Q & QD
* 3   group all common used macros into 1 file
*
* Note that for QDWORD, the limit is 0xC000 0000 to 0x3FFF FFFF but not
*                                    0x8000 0000 to 0x7FFF FFFF !!
*
* Recommended Macros: (This is NOT a full list)
* ------------------
*/
#endif
