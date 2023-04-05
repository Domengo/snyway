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

/* ----------------------------------------------------------------------- *
* Decoder (Viterbi)
*
* INPUT:  V32Struct *pV32 - pointer to modem data structure
*                - provides 'cqRotateIQ' which is the I-Q
*                  point to be decoded
*
* OUTPUT: V32Struct *pV32 - pointer to modem data structure
*                - write decoded output (a group of bits)
*
* modified the V32bis's Viterbi algorithm   by Hong Wang       Feb 1998
* The new V32bis's Viterbi algorithm just calculate 8 points instead 128
* points.
* ----------------------------------------------------------------------- */

#include "v3217ext.h"

/*  pattern1      pattern2    */
/*                     1        */
/*    1   2                    */
/*                  2  3    4    */
/* 3  4 p 5  6         p        */
/*                  5  6    7    */
/*      7   8                 */
/*                     8        */
CONST SWORD Patn1[16] =
{
    -1, 2, 1, 2, -3,  0, -1,  0,
    1, 0, 3, 0, -1, -2,  1, -2
};

CONST SWORD Patn2[16] =
{
    0,  3, -2,  1, 0,  1, 2,  1,
    -2, -1,  0, -1, 2, -1, 0, -3
};

/* (1) x=(cInPnt.r+7)>>1,(2) y=(cInPnt.i+7)>>1 This tabel is for 16 points Viterbi */
CONST UBYTE Vp16_tTable[8][8] =
{
    {0x0c, 0x05, 0x0a, 0x00, 0x0c, 0x05, 0x0a, 0x00},
    {0x06, 0x0f, 0x03, 0x09, 0x06, 0x0f, 0x03, 0x09},
    {0x0b, 0x01, 0x0c, 0x05, 0x0a, 0x00, 0x0d, 0x04},
    {0x02, 0x08, 0x06, 0x0f, 0x03, 0x09, 0x07, 0x0e},
    {0x0c, 0x05, 0x0b, 0x01, 0x0d, 0x04, 0x0a, 0x00},
    {0x06, 0x0f, 0x02, 0x08, 0x07, 0x0e, 0x03, 0x09},
    {0x0b, 0x01, 0x0d, 0x04, 0x0b, 0x01, 0x0d, 0x04},
    {0x02, 0x08, 0x07, 0x0e, 0x02, 0x08, 0x07, 0x0e},
};

/* (1) x=(cInPnt.r+9)>>1,(2) y=(cInPnt.i+9)>>1 This tabel is for 32 points Viterbi */
CONST UBYTE Vp32_tTable[10][10] =
{
    {0x1e, 0x07, 0x10, 0x0e, 0x1c, 0x07, 0x10, 0x0e, 0x1c, 0x06},
    {0x00, 0x19, 0x09, 0x17, 0x01, 0x1b, 0x09, 0x17, 0x01, 0x1b},
    {0x12, 0x0f, 0x1e, 0x07, 0x10, 0x0e, 0x1c, 0x06, 0x11, 0x0c},
    {0x08, 0x15, 0x00, 0x19, 0x09, 0x17, 0x01, 0x1b, 0x0b, 0x16},
    {0x1f, 0x05, 0x12, 0x0f, 0x1e, 0x06, 0x11, 0x0c, 0x1c, 0x04},
    {0x00, 0x18, 0x08, 0x15, 0x02, 0x1a, 0x0b, 0x16, 0x01, 0x1b},
    {0x12, 0x0f, 0x1f, 0x05, 0x13, 0x0d, 0x1d, 0x04, 0x11, 0x0c},
    {0x08, 0x15, 0x02, 0x18, 0x0a, 0x14, 0x03, 0x1a, 0x0b, 0x16},
    {0x1f, 0x05, 0x13, 0x0d, 0x1f, 0x05, 0x13, 0x0d, 0x1d, 0x04},
    {0x02, 0x18, 0x0a, 0x14, 0x03, 0x18, 0x0a, 0x14, 0x03, 0x1a}
};

/* (1) x=(cInPnt.r+11)>>1,(2) y=(cInPnt.i+11)>>1 This tabel is for 64 points Viterbi */
CONST UBYTE Vp64_tTable[12][12] =
{
    {0x34, 0x15, 0x2f, 0x07, 0x34, 0x15, 0x2c, 0x04, 0x31, 0x16, 0x2c, 0x04},
    {0x1c, 0x3d, 0x08, 0x20, 0x1c, 0x3d, 0x0d, 0x25, 0x19, 0x3e, 0x0d, 0x25},
    {0x2e, 0x06, 0x32, 0x13, 0x2e, 0x06, 0x30, 0x17, 0x2a, 0x02, 0x30, 0x17},
    {0x09, 0x21, 0x1a, 0x3b, 0x09, 0x21, 0x18, 0x3f, 0x0b, 0x23, 0x18, 0x3f},
    {0x34, 0x15, 0x2f, 0x07, 0x34, 0x15, 0x2c, 0x04, 0x31, 0x16, 0x2c, 0x04},
    {0x1c, 0x3d, 0x08, 0x20, 0x1c, 0x3d, 0x0d, 0x25, 0x19, 0x3e, 0x0d, 0x25},
    {0x2d, 0x05, 0x36, 0x11, 0x2d, 0x05, 0x35, 0x14, 0x28, 0x00, 0x35, 0x14},
    {0x0c, 0x24, 0x1e, 0x39, 0x0c, 0x24, 0x1d, 0x3c, 0x0f, 0x27, 0x1d, 0x3c},
    {0x37, 0x10, 0x2b, 0x03, 0x37, 0x10, 0x29, 0x01, 0x33, 0x12, 0x29, 0x01},
    {0x1f, 0x38, 0x0a, 0x22, 0x1f, 0x38, 0x0e, 0x26, 0x1b, 0x3a, 0x0e, 0x26},
    {0x2d, 0x05, 0x36, 0x11, 0x2d, 0x05, 0x35, 0x14, 0x28, 0x00, 0x35, 0x14},
    {0x0c, 0x24, 0x1e, 0x39, 0x0c, 0x24, 0x1d, 0x3c, 0x0f, 0x27, 0x1d, 0x3c},
};

/* (1)    x=(cInPnt.r+15>>1,(2) y=(cInPnt.i+15>>1 This tabel is for 128 points Viterbi */
CONST UBYTE Vp128_tTable[16][16] =
{
    {0x67, 0x2f, 0x50, 0x00, 0x63, 0x2d, 0x55, 0x05, 0x61, 0x28, 0x55, 0x05, 0x61, 0x28, 0x57, 0x07},
    {0x37, 0x7f, 0x19, 0x49, 0x33, 0x7d, 0x1b, 0x4b, 0x31, 0x78, 0x1b, 0x4b, 0x31, 0x78, 0x1f, 0x4f},
    {0x58, 0x08, 0x67, 0x2f, 0x50, 0x00, 0x63, 0x2d, 0x55, 0x05, 0x61, 0x28, 0x57, 0x07, 0x69, 0x20},
    {0x11, 0x41, 0x37, 0x7f, 0x19, 0x49, 0x33, 0x7d, 0x1b, 0x4b, 0x31, 0x78, 0x1f, 0x4f, 0x39, 0x70},
    {0x56, 0x2b, 0x58, 0x08, 0x67, 0x2f, 0x54, 0x04, 0x62, 0x2c, 0x57, 0x07, 0x69, 0x20, 0x53, 0x03},
    {0x35, 0x7b, 0x11, 0x41, 0x37, 0x7f, 0x1a, 0x4a, 0x32, 0x7c, 0x1f, 0x4f, 0x39, 0x70, 0x1d, 0x4d},
    {0x5d, 0x0d, 0x65, 0x2b, 0x5c, 0x0c, 0x66, 0x2e, 0x56, 0x06, 0x6a, 0x24, 0x53, 0x03, 0x69, 0x20},
    {0x13, 0x43, 0x35, 0x7b, 0x12, 0x42, 0x36, 0x7e, 0x1e, 0x4e, 0x3a, 0x74, 0x1d, 0x4d, 0x39, 0x70},
    {0x60, 0x29, 0x5d, 0x0d, 0x64, 0x2a, 0x5e, 0x0e, 0x6e, 0x26, 0x52, 0x02, 0x6b, 0x25, 0x53, 0x03},
    {0x30, 0x79, 0x13, 0x43, 0x34, 0x7a, 0x16, 0x46, 0x3e, 0x76, 0x1c, 0x4c, 0x3b, 0x75, 0x1d, 0x4d},
    {0x5d, 0x0d, 0x60, 0x29, 0x5f, 0x0f, 0x6c, 0x22, 0x5a, 0x0a, 0x6f, 0x27, 0x51, 0x01, 0x6b, 0x25},
    {0x13, 0x43, 0x30, 0x79, 0x17, 0x47, 0x3c, 0x72, 0x14, 0x44, 0x3f, 0x77, 0x18, 0x48, 0x3b, 0x75},
    {0x60, 0x29, 0x5f, 0x0f, 0x68, 0x21, 0x5b, 0x0b, 0x6d, 0x23, 0x59, 0x09, 0x6f, 0x27, 0x51, 0x01},
    {0x30, 0x79, 0x17, 0x47, 0x38, 0x71, 0x15, 0x45, 0x3d, 0x73, 0x10, 0x40, 0x3f, 0x77, 0x18, 0x48},
    {0x5f, 0x0f, 0x68, 0x21, 0x5b, 0x0b, 0x68, 0x21, 0x5b, 0x0b, 0x6d, 0x23, 0x59, 0x09, 0x6f, 0x27},
    {0x17, 0x47, 0x38, 0x71, 0x15, 0x45, 0x38, 0x73, 0x15, 0x45, 0x3d, 0x73, 0x10, 0x40, 0x3f, 0x77},
};


void V32_RX_Decode_init(V32ShareStruct *pV32Share)
{
#if V32_VITERBI
    UBYTE i, j, k;

    /* point to top of circular buffer */
    pV32Share->ubVitWinIdx = 0;

    /* initialize the dist/path tables */
    for (i = 0; i < V32_VIT_NUM_WIN; i++)
    {
        for (j = 0; j < V32_VIT_STATES; j++)
        {
            k = (i << V32_VIT_STATES_SHIFT) + j;

            pV32Share->qdVitWinDist[k]  = Q31_MAX;
            pV32Share->ubVitWinPoint[k] = 0x00;
            pV32Share->ubVitWinPath[k]  = (j & 0x01) << 2;
        }
    }

    pV32Share->qdVitWinDist[0] = 0;
#endif
}


/* ---------- Viterbi Decoder ---------- */
#if !USE_ASM

void V32_RX_Decode128p(V32ShareStruct *pV32Share)
{
#if V32_VITERBI
    UBYTE i, j, k;

    CONST UBYTE *Vp_tTable[16];
    SWORD XYoff, XYbound, EncMask, RestMask, EncShift;
    CONST SWORD *Pattern;

    /* input point, map point */
    CQWORD cInPnt, cInPnt1, cInPnt2;
    SBYTE x, y, x1, y1;
    UBYTE DataBits;
    SWORD REdist, IMdist;

    /* store min distance, path */
    UBYTE min_path = 0;
    UBYTE min_point;
    QDWORD min_dist, new_dist;

    /* variables for backtracking */
    QDWORD begin_dist;
    UBYTE  begin_state = 0;
    UBYTE  WinIdx, back_state, curr_state;

    /* variables for finding output bits */
    UBYTE  path_idx, state_idx, short_idx;
    UBYTE  code_data, rest_data;

    /* move the time window index (circular) */

    UBYTE prev_win    = pV32Share->ubVitWinIdx;
    pV32Share->ubVitWinIdx    = (prev_win + 1) & V32_VIT_NUM_WIN_1; /* % V32_VIT_NUM_WIN; */

    /* number of non-encoded bits in one group */
    DataBits  = (pV32Share->ubRxBitsPerSym + 1) - 3;
    EncShift  = DataBits;
    EncMask = (SWORD)0x07 << DataBits;
    RestMask = 0x0f >> (4 - DataBits);

    if (DataBits == 4)
    {
        XYoff   = 15;
        XYbound = 11;

        for (i = 0; i < 16; i++)
        {
            Vp_tTable[i] = Vp128_tTable[i];
        }
    }
    else if (DataBits == 3)
    {
        XYoff   = 11;
        XYbound = 7;

        for (i = 0; i < 12; i++)
        {
            Vp_tTable[i] = Vp64_tTable[i];
        }
    }
    else if (DataBits == 2)
    {
        XYoff   = 9;
        XYbound = 5;

        for (i = 0; i < 10; i++)
        {
            Vp_tTable[i] = Vp32_tTable[i];
        }
    }
    else
    {
        XYoff   = 7;
        XYbound = 3;

        for (i = 0; i < 8; i++)
        {
            Vp_tTable[i] = Vp16_tTable[i];
        }
    }

    /* input point */
    /* ------------------------------------------------
    find shortest distance for each path
    ------------------------------------------------ */

    i = DataBits & 1;

    if (i)
    {
        /* Don't Rotate for 12kbps and 7.2kbps */
        cInPnt2.r = pV32Share->cqRotateIQ.r;
        cInPnt2.i = pV32Share->cqRotateIQ.i;
    }
    else
    {
        /* Rotate 45degree and scale for 14.4kbps and 9.6kbps */
        cInPnt2.r = pV32Share->cqRotateIQ.r - pV32Share->cqRotateIQ.i;
        cInPnt2.i = pV32Share->cqRotateIQ.r + pV32Share->cqRotateIQ.i;
    }

    if (DataBits <= 2) /* scale down by 2 for 9.6 and 7.2kbps */
    {
        cInPnt2.r >>= 1;
        cInPnt2.i >>= 1;
    }

    /* Slice Incoming Points */
    cInPnt.r = cInPnt2.r >> V32_SIGMAP_SCALE;
    cInPnt.i = cInPnt2.i >> V32_SIGMAP_SCALE;

    if (cInPnt.r >  XYbound) { cInPnt.r =  XYbound; }

    if (cInPnt.r < -XYbound) { cInPnt.r = -XYbound; }

    if (cInPnt.i >  XYbound) { cInPnt.i =  XYbound; }

    if (cInPnt.i < -XYbound) { cInPnt.i = -XYbound; }

    cInPnt1.r = cInPnt2.r - (cInPnt.r << V32_SIGMAP_SCALE);
    cInPnt1.i = cInPnt2.i - (cInPnt.i << V32_SIGMAP_SCALE);

    if (cInPnt1.r < 0)
    {
        cInPnt1.r = -cInPnt1.r;
    }

    if (cInPnt1.i < 0)
    {
        cInPnt1.i = -cInPnt1.i;
    }

    i = (cInPnt.r + cInPnt.i) & 1;

    if (i == 0)/* both cInPnt.r and cInPnt.i are even or odd */
    {
        if (cInPnt1.r > cInPnt1.i)
        {
            cInPnt.r ++;
        }
        else
        {
            cInPnt.i ++;
        }

        /* One is even, other is odd of cInPnt.r and cInPnt.i */
    }
    else/* One is even, other is odd of cInPnt.r and cInPnt.i */
    {
        if ((cInPnt1.r + cInPnt1.i) > (1 << V32_SIGMAP_SCALE))
        {
            cInPnt.r ++;
            cInPnt.i ++;
        }

        /* One is even, other is odd of cInPnt.r and cInPnt.i */
    }

    i = cInPnt.r & 1;

    if (i == 0)
    {
        Pattern = Patn1;
    }
    else
    {
        Pattern = Patn2;
    }

    for (k = 0; k < 8; k++)
    {
        x1 = (SBYTE)(cInPnt.r + *Pattern++);
        y1 = (SBYTE)(cInPnt.i + *Pattern++);

        x = (SBYTE)((x1 + XYoff) >> 1);  /* get x of point k+1 */
        y = (SBYTE)((y1 + XYoff) >> 1);  /* get y of point k+1 */

        path_idx =  Vp_tTable[y][x];

        /*    caculate Eucliden distant */
        REdist = (cInPnt2.r >> 2) - (SWORD)(x1 << 7);
        IMdist = (cInPnt2.i >> 2) - (SWORD)(y1 << 7);
        min_dist = QQMULQD(REdist, REdist) + QQMULQD(IMdist, IMdist);

        i = (path_idx & EncMask) >> EncShift;
        min_point = path_idx & RestMask;
        pV32Share->ubVitPathPt[i]   = min_point;
        pV32Share->qdVitPathDist[i] = min_dist >> 3;
    }

    /* ----------------------------------------------------
    update shortest path to each state for
    current time window
    ---------------------------------------------------- */

    begin_dist = Q31_MAX;
    k = pV32Share->ubVitWinIdx << V32_VIT_STATES_SHIFT;

    for (i = 0; i < V32_VIT_STATES; i++)
    {
        min_dist = Q31_MAX;
        /* ignore path if does not exist */
        x = (i & 1) << 2;
        y = x + 4;

        for (j = x; j < y; j++)
        {
            /* find previous state for path transition  */
            state_idx = (j << V32_VIT_STATES_SHIFT) + i;
            state_idx = ubV32_VitPathToStateTab[state_idx];

            /* see if shortest distance for state */
            short_idx = (prev_win << V32_VIT_STATES_SHIFT) + state_idx;
            new_dist = pV32Share->qdVitWinDist[short_idx] + pV32Share->qdVitPathDist[j];

            if (min_dist > new_dist)
            {
                min_dist  = new_dist;
                min_point = pV32Share->ubVitPathPt[j];
                min_path  = state_idx;
            }
        }

        /* update the path, point, distance for the state */
        pV32Share->qdVitWinDist[k+i]  = min_dist;
        pV32Share->ubVitWinPoint[k+i] = min_point;
        pV32Share->ubVitWinPath[k+i]  = min_path;

        /* find current min for finding start of shortest path */
        if (begin_dist > min_dist)
        {
            begin_dist = min_dist;
            begin_state = i;
        }
    }

    /* now normalize the accumulated metrics */
    for (i = 0; i < V32_VIT_STATES; i++)
    {
        pV32Share->qdVitWinDist[k+i] -= begin_dist;
    }

    /* now find the data for the 16th previous time window */
    WinIdx = pV32Share->ubVitWinIdx;
    back_state = pV32Share->ubVitWinPath[(WinIdx << V32_VIT_STATES_SHIFT) + begin_state];

    for (i = V32_VIT_NUM_WIN; i > 0; i--)
    {
        curr_state = back_state;
        WinIdx = (pV32Share->ubVitWinIdx + i) & V32_VIT_NUM_WIN_1;/* % V32_VIT_NUM_WIN; */
        back_state = pV32Share->ubVitWinPath[(WinIdx << V32_VIT_STATES_SHIFT) + curr_state];
    }

    /* index for looking up path state */
    path_idx = (back_state << 3) + curr_state;

    /* for encoded data */
    code_data = ubV32_VitStateToPathTab[path_idx];

    /* for non-coded data */
    rest_data = pV32Share->ubVitWinPoint[(WinIdx << V32_VIT_STATES_SHIFT) + curr_state];

    /* form the output */
    pV32Share->ubTrellisDecodeOut = (code_data << DataBits) + rest_data;

#endif
}
#endif
