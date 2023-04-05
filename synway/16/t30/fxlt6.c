/*****************************************************************************
*             Copyright (c) 2005 - 2005 by GAO Research Inc.                 *
*                          All rights reserved.                              *
*                                                                            *
*  This program is a confidential and an unpublished work of GAO Research    *
*  Inc. that is protected under international and Canadian copyright laws.   *
*  This program is a trade secret and constitutes valuable property of GAO   *
*  Research Inc. All use, reproduction and/or disclosure in whole or in      *
*  part of this program is strictly prohibited unless expressly authorized   *
*  in writing by GAO Research Inc.                                           *
*****************************************************************************/

#define DEBUG_PRN  0

#include "t30ext.h"
#include "faxlinecnt.h"

#if SUPPORT_T30
#if DEBUG_PRN
#include <stdio.h>
#endif

extern const UBYTE Fax2D_TAB[64];
extern const UWORD Fax1DWhite_TAB[4096];
extern const UWORD Fax1DBlack_TAB[8192];

/*
 *    FXL_T6Init - initializes T.6 fax lines module context.
 *        Should be called for every fax page before FXL_CountFaxLinesT6
 *        Params:
 *            pT6Lines    - T.6 fax lines module context
 *            uImageWidth - number of picture elements along a scan line
 */
void FXL_T6Init(T6Lines_Struct *pT6Lines, UWORD uImageWidth)
{
    pT6Lines->rowpixels = uImageWidth;
    pT6Lines->pCurRuns = pT6Lines->RunsBuf;
    pT6Lines->pRefRuns = pT6Lines->pCurRuns + uImageWidth + 2;

    pT6Lines->uBitNum = 0;
    pT6Lines->udBitBuf = 0;
    pT6Lines->udLinesCount = 0;

    /* init reference line to white */
    pT6Lines->pRefRuns[0] = uImageWidth;
    pT6Lines->pRefRuns[1] = 0;

    pT6Lines->State = T6_STAT_2DTABLE;
    pT6Lines->a0 = 0;
    pT6Lines->RunLength = 0;
    pT6Lines->pa = pT6Lines->pCurRuns;
    pT6Lines->pb = pT6Lines->pRefRuns + 1;
    pT6Lines->b1 = uImageWidth;

    //pShare_Info->uImageWidth = uImageWidth;
}

/*
 *    FXL_CountFaxLinesT6 - counts number of T.6 encoded fax lines
 *        Params:
 *            pT6Lines   - T.6 fax lines module context
 *            pData      - pointer to data buffer (LSB is the first bit from the line)
 *            uLen       - number of bytes to process
 *            pudLineCnt - number of lines found so far
 *        Return:
 *              0 - continue to process
 *            < 0 - end of stream or error (no more calls allowed for this page)
 */
int FXL_CountFaxLinesT6(T6Lines_Struct *pT6Lines, UBYTE *pData, UWORD uLen)
{
    int lastx = pT6Lines->rowpixels;    /* last element in row */
    int a0;                    /* reference element */
    int b1;                 /* next change on prev line */
    UDWORD udBitBuf;            /* bit accumulator */
    int uBitNum;            /* # valid bits in udBitBuf */
    int RunLength;            /* length of current run */
    UWORD *pa;             /* place to stuff next run */
    UWORD *pb;                /* next run in reference line */

    UWORD *pCurRunBeg;        /* an array of run lengths for the current row */
    UBYTE *pEndData;    /* a pointer to the end of input data */
    UWORD uTabEnt;
    UWORD *pSwapTmp;
    int rc = T6_RC_SUCCESS;

    int State;

    /* Cache variables for faster processing */
    udBitBuf = pT6Lines->udBitBuf;
    uBitNum = pT6Lines->uBitNum;
    State = pT6Lines->State;
    a0 = pT6Lines->a0;
    RunLength = pT6Lines->RunLength;
    pa = pT6Lines->pa;
    pb = pT6Lines->pb;
    b1 = pT6Lines->b1;

    pCurRunBeg = pT6Lines->pCurRuns;
    pEndData = pData + uLen;

    /* When the last element of the line is horizontal one that's possible that the second
     * component, which is M(a1a2), is a zero run length codeword. In such case collected
     * line has already enough pixels, but we need to process the rest bits to
     * start next line from the rigth point */
    if ((a0 == lastx) && ((State == T6_STAT_HORIZ_WB) || (State == T6_STAT_HORIZ_BW) ||
                          (State == T6_STAT_HORIZ_WW) || (State == T6_STAT_HORIZ_BB)))
    {
        goto LblStateDispatch;
    }

    for (;;)
    {
#if (DEBUG_PRN > 1)
        printf("BitBuf=%08X, BitNum=%d a0=%d lastx=%d\n", udBitBuf, uBitNum, a0, lastx);
#endif

        /* Decode 2D-encoded data row */
        while (a0 < lastx)
        {
LblStateDispatch:

            switch (State)
            {
                case T6_STAT_2DTABLE:
                    NeedBits8(7, LblSaveStateAndExit);

                    if (GetBits(1))    /* 1: V(0) */
                    {
#if (DEBUG_PRN > 1)
                        printf("V0 a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif
                        ClrBits(1);
                        CHECK_b1();

                        T4_T6_SETVAL(b1 - a0);
                        b1 += *pb++;
                        break;
                    }
                    else
                    {
                        State = Fax2D_TAB[(GetBits(7) >> 1)];
                        ClrBits(State & 0x7);
                        State >>= 4;
                        goto LblStateDispatch;    /* no need to check while () condition */
                    }

                case T6_STAT_PASS:
#if (DEBUG_PRN > 1)
                    printf("P a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif
                    CHECK_b1();
                    b1 += *pb++;
                    RunLength += b1 - a0;
                    a0 = b1;
                    b1 += *pb++;
                    State = T6_STAT_2DTABLE;
                    break;

                case T6_STAT_VR1:
                case T6_STAT_VR2:
                case T6_STAT_VR3:
#if (DEBUG_PRN > 1)
                    printf("VR a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif
                    CHECK_b1();

                    T4_T6_SETVAL(b1 - a0 + State - T6_STAT_VR1 + 1);
                    b1 += *pb++;
                    State = T6_STAT_2DTABLE;
                    break;

                case T6_STAT_VL1:
                case T6_STAT_VL2:
                case T6_STAT_VL3:
#if (DEBUG_PRN > 1)
                    printf("VL a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif
                    CHECK_b1();
                    T4_T6_SETVAL(b1 - a0 - (State - T6_STAT_VL1 + 1));
                    b1 -= *--pb;
                    State = T6_STAT_2DTABLE;
                    break;

                case T6_STAT_EXT:
#if (DEBUG_PRN > 1)
                    printf("EXT a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif
#if DEBUG_PRN
                    printf("Uncompressed data (not supported) at scanline %u (%lu)\n",
                           pT6Lines->udLinesCount, (unsigned long)a0);
#endif
                    rc = T6_RC_EXT;
                    goto LblSaveStateAndExit;

                default:
                case T6_STAT_EOL:
#if DEBUG_PRN
                    printf("EOL\n");
#endif
                    rc = T6_RC_EOL;
                    goto LblSaveStateAndExit;

                case T6_STAT_HORIZ:

                    if ((pa - pCurRunBeg) & 1)
                    {
                        State = T6_STAT_HORIZ_BB;
                        goto LblHorizBB;    /* no need to check while () condition */
                    }

                    State = T6_STAT_HORIZ_WW;
                case T6_STAT_HORIZ_WW:
#if (DEBUG_PRN > 1)
                    printf("HWW a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif

                    for (;;)    /* white first */
                    {
                        LOOKUP16(12, Fax1DWhite_TAB, LblSaveStateAndExit);

                        if (uTabEnt & T6_ST_TERMW)
                        {
                            uTabEnt &= 0x3F;
                            T4_T6_SETVAL(uTabEnt);
                            State = T6_STAT_HORIZ_WB;
                            break;
                        }
                        else if (uTabEnt & T6_ST_MAKEUP)
                        {
                            uTabEnt = (UWORD)((uTabEnt & 0x3F) << 6);
                            a0 += uTabEnt;
                            RunLength += uTabEnt;
                            continue;
                        }
                        else
                        {
#if DEBUG_PRN
                            printf("WhiteTable: Bad code word at scanline %u (%lu)\n",
                                   pT6Lines->udLinesCount, (unsigned long)a0);
#endif
                            rc = T6_RC_1D_WCW;
                            goto LblSaveStateAndExit;
                        }
                    }

                case T6_STAT_HORIZ_WB:
#if (DEBUG_PRN > 1)
                    printf("HWB a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif

                    for (;;)    /* then black */
                    {
                        LOOKUP16(13, Fax1DBlack_TAB, LblSaveStateAndExit);

                        if (uTabEnt & T6_ST_TERMB)
                        {
                            uTabEnt &= 0x3F;
                            T4_T6_SETVAL(uTabEnt);
                            CHECK_b1();
                            State = T6_STAT_2DTABLE;
                            break;
                        }
                        else if (uTabEnt & T6_ST_MAKEUP)
                        {
                            uTabEnt = (UWORD)((uTabEnt & 0x3F) << 6);
                            a0 += uTabEnt;
                            RunLength += uTabEnt;
                            continue;
                        }
                        else
                        {
#if DEBUG_PRN
                            printf("BlackTable: Bad code word at scanline %u (%lu)\n",
                                   pT6Lines->udLinesCount, (unsigned long)a0);
#endif
                            rc = T6_RC_1D_BCW;
                            goto LblSaveStateAndExit;
                        }
                    }

                    break;

                case T6_STAT_HORIZ_BB:
LblHorizBB:
#if (DEBUG_PRN > 1)
                    printf("HBB a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif

                    for (;;)    /* black first */
                    {
                        LOOKUP16(13, Fax1DBlack_TAB, LblSaveStateAndExit);

                        if (uTabEnt & T6_ST_TERMB)
                        {
                            uTabEnt &= 0x3F;
                            T4_T6_SETVAL(uTabEnt);
                            State = T6_STAT_HORIZ_BW;
                            break;
                        }
                        else if (uTabEnt & T6_ST_MAKEUP)
                        {
                            uTabEnt = (UWORD)((uTabEnt & 0x3F) << 6);
                            a0 += uTabEnt;
                            RunLength += uTabEnt;
                            continue;
                        }
                        else
                        {
#if DEBUG_PRN
                            printf("BlackTable: Bad code word at scanline %u (%lu)\n",
                                   pT6Lines->udLinesCount, (unsigned long)a0);
#endif
                            rc = T6_RC_1D_BCW;
                            goto LblSaveStateAndExit;
                        }
                    }

                case T6_STAT_HORIZ_BW:
#if (DEBUG_PRN > 1)
                    printf("HBW a0=%d b1=%d bits=%08X(%u)\n", a0, b1, udBitBuf, uBitNum);
#endif

                    for (;;)    /* then white */
                    {
                        LOOKUP16(12, Fax1DWhite_TAB, LblSaveStateAndExit);

                        if (uTabEnt & T6_ST_TERMW)
                        {
                            uTabEnt &= 0x3F;
                            T4_T6_SETVAL(uTabEnt);
                            CHECK_b1();
                            State = T6_STAT_2DTABLE;
                            break;
                        }
                        else if (uTabEnt & T6_ST_MAKEUP)
                        {
                            uTabEnt = (UWORD)((uTabEnt & 0x3F) << 6);
                            a0 += uTabEnt;
                            RunLength += uTabEnt;
                            continue;
                        }
                        else
                        {
#if DEBUG_PRN
                            printf("WhiteTable: Bad code word at scanline %u (%lu)\n",
                                   pT6Lines->udLinesCount, (unsigned long)a0);
#endif
                            rc = T6_RC_1D_WCW;
                            goto LblSaveStateAndExit;
                        }
                    }

                    break;
            }/* switch */
        }/* while */

        /*
         * Cleanup the array of runs after decoding a row.
         * We adjust final runs to insure the user buffer is not
         * overwritten and/or undecoded area is white filled.
         */
        if (RunLength)
        {
            T4_T6_SETVAL(0);
        }

        if (a0 != lastx)
        {
#if DEBUG_PRN
            printf("Line length mismatch at scanline %u (got %lu, expected %lu)\n",
                   pT6Lines->udLinesCount, (unsigned long)a0, (unsigned long)lastx);
#endif
            rc = T6_RC_LINELEN;
            break;    /* goto LblSaveStateAndExit */
        }

        if (State != T6_STAT_2DTABLE)
        {
#if DEBUG_PRN
            printf("Line end: wrong state %u (line %u)\n", State, pT6Lines->udLinesCount);
#endif
            rc = T6_RC_LINEEND;
            break;    /* goto LblSaveStateAndExit */
        }

        pT6Lines->udLinesCount++;

        T4_T6_SETVAL(0);        /* imaginary change for reference */

        pSwapTmp = pT6Lines->pCurRuns;
        pT6Lines->pCurRuns = pT6Lines->pRefRuns;
        pT6Lines->pRefRuns = pSwapTmp;

        pa = pCurRunBeg = pT6Lines->pCurRuns;
        pb = pT6Lines->pRefRuns;
        a0 = 0;
        RunLength = 0;
        b1 = *pb++;
    }/* for */

LblSaveStateAndExit:
    /* Save cached variables to the context */
    pT6Lines->uBitNum = uBitNum;
    pT6Lines->udBitBuf = udBitBuf;
    pT6Lines->State = State;
    pT6Lines->a0 = a0;
    pT6Lines->RunLength = RunLength;
    pT6Lines->pa = pa;
    pT6Lines->pb = pb;
    pT6Lines->b1 = b1;

    //* pudLineCnt = pT6Lines->udLinesCount;
    return rc;
}
#endif

