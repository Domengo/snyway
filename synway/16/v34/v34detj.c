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

/* ---------------------------------------------------------------------- */
/*  Module to detect J sequence                                         */
/*                                                                      */
/*  Input: one bit at a time                                            */
/*  Output: 4 or 16                                                     */
/*          return a value of 4 means constellation size is 4-point     */
/*          return a value of 16 means constellation size is 16-point   */
/*                                                                      */
/*        Feb 22, 96. Mask lower 16-bit of sequence, in case integer    */
/*                    is 32 bits.  - Benjamin Chan.                     */
/* ---------------------------------------------------------------------- */

#include "v34ext.h"

#if 0 /// LLL temp
UBYTE V34_Detect_J_Sequence(UBYTE inbyte, V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);

    pV34Rx->sequence  = (pV34Rx->sequence << 2) & 0xFFFFU;
    pV34Rx->sequence |= (UWORD)(inbyte & 0x1) << 1;
    pV34Rx->sequence |= (inbyte >> 1) & 0x1;

    if (pV34Rx->sequence == 0x0991)
    {
        return (V34_4_POINT_TRN);
    }
    else if (pV34Rx->sequence == 0x0d91)
    {
        return (V34_16_POINT_TRN);
    }
    else if (pV34Rx->sequence == 0xF991)
    {
        return (V34_PI_POINT_TRN);
    }

    return(0);
}
#endif