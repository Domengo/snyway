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

/*********************************************************************
  This file initilises all the Transmitter vectors for all
  the handshaking segments and datamode for v29 fax/modem.
**********************************************************************/

#include "v29ext.h"
#include "ptable.h"

void V29_TEP(V29Struct *pV29)
{
    TRACE0("V29: V29_TEP");

    pV29->pfDataInVec = V29_HS_TEP;           /* HandShaking TEP tone generation */
    pV29->pfScramVec  = V29_Dummy;
    pV29->pfEncodeVec = V29_Dummy;
}

void V29_Segment_1(V29Struct *pV29)
{
    TRACE0("V29: V29_Segment_1");

    pV29->pfDataInVec = V29_HS_Segment_1;           /* HandShaking segment 1 */
    pV29->pfScramVec  = V29_Dummy;
    pV29->pfEncodeVec = V29_Dummy;
}

void V29_Segment_2(V29Struct *pV29)
{
    TRACE0("V29: V29_Segment_2");

    pV29->pfDataInVec = V29_Dummy;      /* HandShaking segment 2 */
    pV29->pfScramVec  = V29_Dummy;
    pV29->pfEncodeVec = V29_Segment2_Encode;
}

void V29_Segment_3(V29Struct *pV29)
{
    TRACE0("V29: V29_Segment_3");

    pV29->udScramSReg = 0x54000000;         /* 0101010  initial condition for pseudo sequence */

    pV29->pfDataInVec = V29_HS_Segment_3;   /* HandShaking segment 3 */
    pV29->pfScramVec  = V29_Dummy;
    pV29->pfEncodeVec = V29_Segment3_Encode;
}

void V29_Segment_4(V29Struct *pV29)
{
    TRACE0("V29: V29_Segment_4");

    pV29->udScramSReg   = 0;

    pV29->pfDataInVec = V29_HS_Segment_4;  /* HandShaking segment 4 */
    pV29->pfScramVec  = V29_TX_Scrambler;
    pV29->pfEncodeVec = V29_TX_Encoder;
}

void V29_Data_Mode(V29Struct *pV29)
{
    TRACE0("V29: V29_Data_Mode");

    pV29->pfDataInVec = V29_Data_Transfer; /* Data Mode starts */
    pV29->pfScramVec  = V29_TX_Scrambler;
    pV29->pfEncodeVec = V29_TX_Encoder;
}
