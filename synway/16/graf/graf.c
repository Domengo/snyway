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

#include <string.h>
#include "ptable.h"
#include "graf.h"

long dot_idx = 0;

CONST UBYTE MOD_8[8] =
{
    0x08, 0x04, 0x02, 0x01, 0x80, 0x40, 0x20, 0x10
};

CONST UBYTE MOD_V[8] =
{
    0x7f, 0xbf, 0xcf, 0xef, 0xf7, 0xfb, 0xfc, 0xfe
};

extern UBYTE *videoMem;
extern long prev_dots[32768*4];

/* --------------- DRAW EPG ---------------- */

void draw_EPG(UBYTE **pTable)
{
    UWORD i;
    SWORD size;
    SWORD Xpos;
    SWORD Ypos;
    long  Loc;
    long  xLoc;
    long  yLoc;
    SWORD *pEye;
    int xmod;

    pEye = (SWORD *)pTable[EPGDATA_IDX];
    size = *pEye;
    pEye++;

    for (i = 0; i < size; i++)
    {
        Xpos = *pEye++;
        Ypos = *pEye++;

#if DISPLAYCOLOR
        /* Software EYE */
        xLoc = 320 * ((256 + (long)(Xpos >> 7))) >> 9;
        yLoc = 200 * ((256 - (long)(Ypos >> 7))) >> 9;
        Loc  = 320 * yLoc + xLoc;

        if (Loc < 0)
        {
            Loc = 0;
        }

        if (Loc > 63999)
        {
            Loc = 63999;
        }

        /* plot new dot */
        *(videoMem + Loc) = RGBred;

        /* put dots in delay line */
        prev_dots[dot_idx] = Loc;
        dot_idx = (dot_idx + 1) & DOT_IDX;

        /* erase old dot */
        Loc = prev_dots[dot_idx];
        *(videoMem + Loc) = RGBblack;
        /* Software EYE */

        /* Hardware EYE */
        /*outp(0x220 + 24, (char)(Xpos >> 8));*/
        /*outp(0x220 + 25, (char)(Ypos >> 8));*/

        /*outp(0x220 + 30, 0x44);*/
        /*outp(0x220 + 31, 0x44);*/
        /* Hardware EYE */
#else
        /* Software EYE */
#if V90_GRAF
        xLoc = 318 + ((Xpos * 3) >> 8);
        xmod = xLoc & 0x7;
        xLoc = (xLoc + 4) >> 3;
        yLoc = 240 + ((Ypos * 3) >> 8);
#else
        xLoc = (636 * (256 + (Xpos >> 7))) >> 9; /*576*/
        xmod = xLoc & 0x7;
        xLoc = (xLoc + 4) >> 3;
        yLoc = (530 * (256 - (Ypos >> 7))) >> 9; /*480*/
#endif
        Loc  = 80 * yLoc + xLoc;

        if (Loc < 0)
        {
            Loc = 0;
        }

        if (Loc > 38400)
        {
            Loc = 38400;
        }

        /* plot new dot */
        *(videoMem + Loc) |= MOD_8[xmod];

        /* put dots in delay line */
        prev_dots[dot_idx] = Loc;

        dot_idx = (dot_idx + 1) & DOT_IDX;

        /* erase old dot */
        Loc = prev_dots[dot_idx];
        *(videoMem + Loc) = 0;
        /* Software EYE */
#endif
    }
}

#if 0
/* --------------- DRAW COEF ---------------- */
void draw_v90coef_init(COEFDRAW *pCd, Q30 *coef, SWORD X, SWORD Y, SWORD numcoef, SWORD step)
{
    pCd->V90coef    = coef;
    pCd->V90coefptr = coef;
    pCd->centre     = 80 * Y + (X - 8);
    pCd->numpnts    = numcoef;
    pCd->step       = step;
    pCd->scnidx     = 0;
}

void draw_v90coef(COEFDRAW *pCd)
{
    int coefLoc;
    int dotLoc;
    long coefVal;
    int xmod;

    coefVal = *(pCd->V90coefptr);

    /* erase old coef value */
    coefLoc = pCd->scndots[pCd->scnidx];
    xmod    = pCd->scnidx & 0x7;
    dotLoc  = pCd->centre + (pCd->scnidx >> 3) + coefLoc;

    if (dotLoc < 0)
    {
        dotLoc = 0;
    }

    if (dotLoc > 38400)
    {
        dotLoc = 38400;
    }

    *(videoMem + dotLoc) &= MOD_V[xmod];

    /* plot new coef value */
    coefLoc = 80 * (coefVal >> 10);
    xmod = pCd->scnidx & 0x7;
    dotLoc = pCd->centre + (pCd->scnidx >> 3) + coefLoc;

    if (dotLoc >= 38400)
    {
        dotLoc = 38400;
    }

    if (dotLoc < 0)
    {
        dotLoc = 0;
    }

    *(videoMem + dotLoc) |= MOD_8[xmod];

    /* store coef location */
    pCd->scndots[pCd->scnidx] = coefLoc;

    /* next coef, point */
    pCd->scnidx  ++;
    pCd->V90coefptr += pCd->step;

    /* go back to first coef if end reached */
    if (pCd->scnidx >= pCd->numpnts)
    {
        pCd->scnidx = 0;
        pCd->V90coefptr = pCd->V90coef;
    }
}
#endif

void draw_coef_init(COEFDRAW *pCd, CQWORD *coef, SWORD X, SWORD Y, SWORD numcoef, SWORD step, SWORD REALorIMAG)
{
#if DISPLAYCOLOR
    pCd->coef = coef;
    pCd->coefptr = coef;
    pCd->centre = 320 * Y + (X - numcoef / 2);
    pCd->numpnts = numcoef;
    pCd->step = step;

    /*for(i=0; i<320; i++) pCd->scndots[i] = 0;*/
    pCd->scnidx = 0;
    pCd->REALorIMAG = REALorIMAG;
#else
    pCd->coef = coef;
    pCd->coefptr = coef;
    pCd->centre = 80 * Y + (X - 8);
    pCd->numpnts = numcoef;
    pCd->step = step;
    /*for(i=0; i<320; i++) pCd->scndots[i] = 0;*/
    pCd->scnidx = 0;
    pCd->REALorIMAG = REALorIMAG;
#endif
}

void draw_coef(COEFDRAW *pCd)
{
    int coefLoc;
    int dotLoc;
    int coefVal;
    int xmod;

#if DISPLAYCOLOR

    /* get current coef value */
    if (pCd->REALorIMAG == 0)
    {
        coefVal = -(pCd->coefptr->r) / 32768.0;
    }
    else
    {
        coefVal = -(pCd->coefptr->i) / 32768.0;
    }

    /* erase old coef value */
    coefLoc = pCd->scndots[pCd->scnidx];
    dotLoc  = pCd->centre + pCd->scnidx + coefLoc;

    if (dotLoc < 0)
    {
        dotLoc = 0;
    }

    if (dotLoc > 63999)
    {
        dotLoc = 63999;
    }

    *(videoMem + dotLoc) = RGBblack;

    /* plot new coef value */
    coefLoc = 320 * (long)(coefVal * 15.0);
    dotLoc = pCd->centre + pCd->scnidx + coefLoc;

    if (dotLoc >= 64000)
    {
        dotLoc = 63999;
    }

    if (dotLoc < 0)
    {
        dotLoc = 0;
    }

    *(videoMem + dotLoc) = RGByellow;

    /* store coef location */
    pCd->scndots[pCd->scnidx] = coefLoc;

    /* next coef, point */
    pCd->scnidx  ++;
    pCd->coefptr += pCd->step;

    /* go back to first coef if end reached */
    if (pCd->scnidx >= pCd->numpnts)
    {
        pCd->scnidx  = 0;
        pCd->coefptr = pCd->coef;
    }

#else

    /* get current coef value */
    if (pCd->REALorIMAG == 0)
    {
        coefVal = -(pCd->coefptr->r);
    }
    else
    {
        coefVal = -(pCd->coefptr->i);
    }

    /* erase old coef value */
    coefLoc = pCd->scndots[pCd->scnidx];
    xmod = pCd->scnidx & 0x7;
    dotLoc = pCd->centre + (pCd->scnidx >> 3) + coefLoc;

    if (dotLoc < 0)
    {
        dotLoc = 0;
    }

    if (dotLoc > 38400)
    {
        dotLoc = 38400;
    }

    *(videoMem + dotLoc) &= MOD_V[xmod];

    /* plot new coef value */
    coefLoc = 80 * (coefVal >> 10);
    xmod = pCd->scnidx & 0x7;
    dotLoc = pCd->centre + (pCd->scnidx >> 3) + coefLoc;

    if (dotLoc >= 38400)
    {
        dotLoc = 38400;
    }

    if (dotLoc < 0)
    {
        dotLoc = 0;
    }

    *(videoMem + dotLoc) |= MOD_8[xmod];

    /* store coef location */
    pCd->scndots[pCd->scnidx] = coefLoc;

    /* next coef, point */
    pCd->scnidx  ++;
    pCd->coefptr += pCd->step;

    /* go back to first coef if end reached */
    if (pCd->scnidx >= pCd->numpnts)
    {
        pCd->scnidx = 0;
        pCd->coefptr = pCd->coef;
    }

#endif
}


/* --------------- DRAW ERROR ---------------- */


void draw_err_init(ERRDRAW *pEd, SWORD X, SWORD Y, SWORD numerr)
{
    /*UWORD i;*/
#if DISPLAYCOLOR
    pEd->centre = 320 * (long)Y + (long)(X - numerr / 2);
    pEd->numpnts = numerr;

    /*for(i=0; i<320; i++) pEd->scndots[i] = 0;*/
    pEd->scnidx = 0;
#else

#endif
}

void draw_err(ERRDRAW *pEd, float errVal)
{
#if DISPLAYCOLOR
    int dots;
    long errLoc;
    char *pCentre;
    char *dotLoc;
    char *zeroLoc;
    long scnidx;

    /* init dotLoc, zeroLoc */
    pCentre  = videoMem + pEd->centre;

    /* erase old dots */
    scnidx = pEd->scnidx;

    for (dots = 0; dots < pEd->numpnts; dots++)
    {
        errLoc = pEd->scndots[scnidx];
        dotLoc = pCentre + dots + errLoc;
        *dotLoc = RGBblack;
        scnidx++;
    }

    /* add new coef value */
    errLoc = (long)(errVal * 15.0);

    if (errLoc >  45)
    {
        errLoc =  45;
    }

    if (errLoc < -45)
    {
        errLoc = -45;
    }

    errLoc *= -320;
    pEd->scndots[pEd->scnidx] = errLoc;
    pEd->scndots[pEd->scnidx + pEd->numpnts] = errLoc;
    pEd->scnidx++;

    if (pEd->scnidx >= pEd->numpnts)
    {
        pEd->scnidx = 0;
    }

    /* draw new dots */
    scnidx = pEd->scnidx;

    for (dots = 0; dots < pEd->numpnts; dots++)
    {
        errLoc = pEd->scndots[scnidx];
        zeroLoc = pCentre + dots;
        dotLoc  = zeroLoc +  errLoc;
        *zeroLoc = RGBblue;
        *dotLoc = RGByellow;
        scnidx++;
    }

#endif

}
