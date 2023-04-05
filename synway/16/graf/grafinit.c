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

#include <dos.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include "graf.h"

long prev_dots[32768*4];
RGBstruct vgaPal[256];

UBYTE *videoMem;

void videomode(long Mode)
{
    union REGS r;

    r.x.eax = Mode;
    int386(16, &r, &r);
    /*r.x.ax = Mode;*/
    /*int86(16, &r, &r);*/
}


void set_pal(void)
{
    UWORD i;

    /* clear VGA palette */
    for (i = 0; i < 256; i++)
    {
        vgaPal[i].r = 255;
        vgaPal[i].g = 255;
        vgaPal[i].b = 255;
    }

    /* create VGA colours */
    vgaPal[RGBblack].r = 0;
    vgaPal[RGBblack].g = 0;
    vgaPal[RGBblack].b = 0;

    vgaPal[RGBred].r = 255;
    vgaPal[RGBred].g = 0;
    vgaPal[RGBred].b = 0;

    vgaPal[RGBgreen].r = 0;
    vgaPal[RGBgreen].g = 255;
    vgaPal[RGBgreen].b = 0;

    vgaPal[RGBblue].r = 0;
    vgaPal[RGBblue].g = 0;
    vgaPal[RGBblue].b = 155;

    vgaPal[RGByellow].r = 255;
    vgaPal[RGByellow].g = 255;
    vgaPal[RGByellow].b = 0;
}


void pal_graf(void)
{
    UWORD i;

#if DISPLAYCOLOR
    /* write colour palette for VGA */
    outp(0x3c6, 0xff);

    for (i = 0; i < 256; i++)
    {
        outp(0x3c8, i);
        outp(0x3c9, vgaPal[i].r >> 2);
        outp(0x3c9, vgaPal[i].g >> 2);
        outp(0x3c9, vgaPal[i].b >> 2);
    }

    /* colour screen black */
    /*for(i=0; i<64000; i++) *(videoMem + i) = RGBblack;*/
#else

    for (i = 0; i < 38400; i++) { *(videoMem + i) = 0; }

#endif
}


void init_graf(void)
{
    videoMem = (UBYTE *)0xa0000;

    memset(prev_dots, 0, (32768 * 4) * sizeof(long)); /*512*/

    /* set video mode to graphics */
#if DISPLAYCOLOR
    videomode(19);
    /* set VGA palette */
    set_pal();
#else
    videomode(17);
#endif

    pal_graf();
}

/* Reset graphics if supported */
void done_graf(void)
{
    /* set video mode to text */
    videomode(2);
}
