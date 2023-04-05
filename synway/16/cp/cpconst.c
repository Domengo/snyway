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

#include "cpext.h"

/* TX Tables */

CONST UWORD DtmfDigitRow[16] =
{
    ROW3,    ROW0,    ROW0,    ROW0,
    ROW1,    ROW1,    ROW1,    ROW2,
    ROW2,    ROW2,    ROW0,    ROW1,
    ROW2,    ROW3,    ROW3,    ROW3
};

CONST UWORD DtmfDigitCol[16] =
{
    COL1,    COL0,    COL1,    COL2,
    COL0,    COL1,    COL2,    COL0,
    COL1,    COL2,    COL3,    COL3,
    COL3,    COL3,    COL0,    COL2
};

/* The Gcoefs is the Goertzel coefficient for different tones */
/* The way to calculate the coeffs :   32768*cos(2*PI*f/fs)   */
/*     f  - is the tone to be detected                        */
/*     fs - is the sampling rate                              */
#if CP_7200  /* for 7200 Hz sampling rate */
CONST QWORD CP_tToneGcoefs[TONE_MAX_FREQ] =
{
    /* Dialtone */
    31251,        /* 350Hz Dialtone */
    30382,    /* 440Hz Dialtone */

    /* Line Busy Tone */
    29935,    /* 480 Hz */
    28088,    /* 620 Hz */

    /* Audible Ring Tone, 440 Hz, 480 Hz */

    /* Receiver off-hook tone */
    11207,    /* 1400 Hz */
    -12803,   /* 2260 Hz */
    -17606,   /* 2450 Hz */
    -21063,   /* 2600 Hz */

    /* others */
    32737,     /* 50  Hz */
    30792,     /* 400 Hz */
    30540,     /* 425 Hz */
    30274,     /* 450 Hz */

    /* ANS tone */
    -8481,     /* 2100Hz ANS tone */

    /* Bell tone */
    -11876,    /* 2225 Hz */

#if SUPPORT_FAX

#if SUPPORT_FAX_SEND_CNG
    /* CNG tone */
    18795,     /* 1100 Hz */
#endif

    /* V21 tone */
    4277       /* 1650 Hz */
#endif
};
#elif CP_8000  /* for 8000 Hz sampling rate */
CONST QWORD CP_tToneGcoefs[TONE_MAX_FREQ] =
{
    /* Dialtone */
    31538,        /* 350Hz Dialtone */
    30831,    /* 440Hz Dialtone */

    /* Line Busy Tone */
    30467,    /* 480 Hz */
    28959,    /* 620 Hz */

    /* Audible Ring Tone, 440 Hz, 480 Hz */

    /* Receiver off-hook tone */
    14876,    /* 1400 Hz */
    -6645,    /* 2260 Hz */
    -11342,   /* 2450 Hz */
    -14876,   /* 2600 Hz */

    32743,    /* 50   Hz */
    31164,    /* 400  Hz */
    30959,    /* 425  Hz */
    30743,    /* 450  Hz */

    /* ANS tone */
    -2571,    /* 2100Hz ANS tone */

    /* Bell tone */
    -5760,    /* 2225 Hz */

#if SUPPORT_FAX

#if SUPPORT_FAX_SEND_CNG
    /* CNG tone */
    21281,    /* 1100 Hz */
#endif

    /* V21 tone */
    8895      /* 1650 Hz */
#endif
};

#elif CP_9600  /* for 9600 Hz sampling rate */
CONST QWORD CP_tToneGcoefs[TONE_MAX_FREQ] =
{
    31912,        /* 350Hz */  /* Dialtone1 */
    31419,    /* 440Hz */  /* Dialtone2 */

    /* Line Busy Tone */
    31164,    /* 480Hz */
    30107,    /* 620Hz */

    /* Audible Ring Tone, 440Hz, 480Hz */

    /* Receiver off-hook tone */
    19948,    /* 1400 Hz */
    2998,     /* 2260 Hz */
    -1072,    /* 2450 Hz */
    -4277,    /* 2600 Hz */

    32750,    /* 50   Hz */
    31651,    /* 400  Hz */
    31508,    /* 425  Hz */
    31357,    /* 450  Hz */

    /* ANS tone */
    6393,     /* 2100 Hz */

    /* USB1 tone */
    3212,     /* 2250 Hz */
#if SUPPORT_FAX

#if SUPPORT_FAX_SEND_CNG
    /* CNG tone */
    24636,    /* 1100 Hz */
#endif

    /* V21 tone */
    15447     /* 1650 Hz */
#endif
};
#endif
