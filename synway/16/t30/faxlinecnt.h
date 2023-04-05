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

#ifndef _FAX_LINE_CNT_H_
#define _FAX_LINE_CNT_H_

#include "commtype.h"

#define T6_RC_SUCCESS             (0)     /* so far so good, continue calling */
#define T6_RC_EOL                 (-1) /* EOL was found -- normal end of the stream */
#define T6_RC_EXT                 (-2) /* 2D extension code word was found -- not supported */
#define T6_RC_1D_BCW             (-3) /* bad 1D black code word */
#define T6_RC_1D_WCW             (-4) /* bad 1D white code word */
#define T6_RC_LINELEN             (-5) /* wrong line length */
#define T6_RC_LINEEND             (-6) /* inappropriate state at line end */

/* Main state machine */
/* 2D code table states */
#define T6_STAT_PASS             (0)  /* 0001: P */
#define T6_STAT_HORIZ             (1)  /* 001: H */
#define T6_STAT_VR1                 (2)  /* 011: VR(1) */
#define T6_STAT_VR2                 (3)  /* 000011: VR(2) */
#define T6_STAT_VR3                 (4)  /* 0000011: VR(3) */
#define T6_STAT_VL1                 (5)  /* 010: VL(1) */
#define T6_STAT_VL2                 (6)  /* 000010: VL(2) */
#define T6_STAT_VL3                 (7)  /* 0000010: VL(3) */
#define T6_STAT_EXT                 (8)  /* 0000001: 2D extension */
#define T6_STAT_EOL                 (9)  /* 0000000: EOL or invalid codeword */
/* Other states */
#define T6_STAT_2DTABLE             (10) /* expecting 2D code word */
#define T6_STAT_HORIZ_WW         (11) /* horizontal white starting with white */
#define T6_STAT_HORIZ_WB         (12) /* horizontal black starting with white */
#define T6_STAT_HORIZ_BB         (13) /* horizontal black starting with black */
#define T6_STAT_HORIZ_BW         (14) /* horizontal white starting with black */

#define T6_ST_TERMW                 (0x8000)
#define T6_ST_TERMB                 (0x8000)
#define T6_ST_MAKEUPW             (0x4000)
#define T6_ST_MAKEUPB             (0x4000)
#define T6_ST_MAKEUP             (0x4000)
#define T6_ST_EOL                 (0x0000)
#define T6_ST_INVALID             (0x0000)


#define NeedBits8(n, eoflab) \
    if(uBitNum < (n)) { \
        if(pData >= pEndData) { /* EndOfData? */ \
            goto eoflab; \
        } else { \
            udBitBuf |= ((UDWORD)(*pData++ & 0xFF) << uBitNum); \
            uBitNum += 8; \
        } \
    }

#define NeedBits16(n, eoflab) \
    if(uBitNum < (n)) { \
        if(pData >= pEndData) { /* EndOfData? */ \
            goto eoflab; \
        } else { \
            udBitBuf |= ((UDWORD)(*pData++ & 0xFF) << uBitNum); \
            if((uBitNum += 8) < (n)) { \
                if(pData >= pEndData) { /* EndOfData? */ \
                    goto eoflab; \
                } else { \
                    udBitBuf |= ((UDWORD)(*pData++ & 0xFF) << uBitNum); \
                    uBitNum += 8; \
                } \
            } \
        } \
    }

#define GetBits(n)    (udBitBuf & ((1 << (n)) - 1))
#define ClrBits(n)    { uBitNum -= (n); udBitBuf >>= (n); }

#define LOOKUP16(wid, tab, eoflab) { \
        NeedBits16(wid, eoflab); \
        uTabEnt = tab[GetBits(wid)]; \
        ClrBits((uTabEnt >> 6) & 0xF); \
    }

/*
 * Append a run to the run length array for the current row and reset decoding state
 */
#define T4_T6_SETVAL(x) { *pa++ = (UWORD)(RunLength + (x)); a0 += (x); RunLength = 0; }

/*
 * Update the value of b1 using the array of runs for the reference line
 */
#define CHECK_b1() \
    if(pa != pCurRunBeg) { \
        while(b1 <= a0 && b1 < lastx) { \
            b1 += pb[0] + pb[1]; \
            pb += 2; \
        } \
    }
#endif


