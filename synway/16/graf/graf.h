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

#ifndef _GRAF_H
#define _GRAF_H

#include "commdef.h"

#define V90_GRAF        (1)

#if V90_GRAF
#define  DOT_IDX        (0x7FFF)
#else
#define  DOT_IDX        (0x3FF)
#endif

#define REAL            (0)
#define IMAG            (1)

#define GRAF_NUM_PNTS   (511)/*127*/

#define DISPLAYCOLOR    (0)

#define RGBblack        (0)
#define RGBred          (1)
#define RGBgreen        (2)
#define RGBblue         (3)
#define RGByellow       (4)

#define SCREEN_WIDTH    (320)

typedef struct
{
    CQWORD *coef;
    CQWORD *coefptr;
    long centre;
    UDWORD numpnts;
    long step;
    long scndots[SCREEN_WIDTH*2];
    long scnidx;
    SWORD REALorIMAG;
} COEFDRAW;


typedef struct
{
    long centre;
    UDWORD numpnts;
    long scndots[SCREEN_WIDTH*2];
    long scnidx;
} ERRDRAW;

typedef struct
{
    UBYTE r;
    UBYTE g;
    UBYTE b;
} RGBstruct;

/* function prototypes */
void draw_coef_init(COEFDRAW *, CQWORD *, SWORD, SWORD, SWORD, SWORD, SWORD);
void draw_coef(COEFDRAW *);
void draw_err_init(ERRDRAW *, SWORD, SWORD, SWORD);
void draw_err(ERRDRAW *, float);

#endif
