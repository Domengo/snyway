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
        File Name : MHSP.H

        Description :
                Modem host signal processor structure
                This structure will share between the host and the mhsp
**********************************************************************/

#ifndef _MHSP_H
#define _MHSP_H

#include "commdef.h"

/** Modem Buffer size defines **/
#define MODEM_BUFSIZE_16             (16)
#define MODEM_BUFSIZE_32             (32)

/** Modem Call rate defines **/
#define MODEM_CALL_16_96             (6)
#define MODEM_CALL_32_96             (3)

typedef struct
{
    UBYTE RtCallRate;
    UBYTE RtBufSize;
    void (*pfRealTime)(UBYTE **);
    UWORD Total_TimeOut_Count;
} MhspStruct;

/* external function prototypes */
#if DRAWEPG
void draw_EPG(UBYTE **pTable);
#endif

void Cid_Dpcs_Init(UBYTE **pTable);
void Dtmf_Dpcs_Init(UBYTE **pTable);
void CidGen_Dpcs_Init(UBYTE **pTable);
void Cp_DPCS_Init(UBYTE **pTable);
void AUTO_DPCS_Init(UBYTE **pTable);
void V8_DPCS_Init(UBYTE **pTable);
void V21_DPCS_Init(UBYTE **pTable);
void V23_DPCS_Init(UBYTE **pTable);
void V22_DPCS_Init(UBYTE **pTable);
void V32_DPCS_Init(UBYTE **pTable);
void V17_DPCS_Init(UBYTE **pTable);
void V27_DPCS_Init(UBYTE **pTable);
void V29_DPCS_Init(UBYTE **pTable);
void V21ch2_DPCS_Init(UBYTE **pTable);
void BELL103_DPCS_Init(UBYTE **pTable);
void BELL212A_DPCS_Init(UBYTE **pTable);
void V34_LAL_Startup(UBYTE **pTable);
void MhspInit(UBYTE **pTable);
void FastMhspInit(UBYTE **pTable);
void MhspMain(UBYTE **pTable);
void Mhsp_LineDisconnect(UBYTE **pTable);
void Disconnect_Init(UBYTE **);
void Reset_Init(UBYTE **pTable);
void MhspIdle(UBYTE **pTable);
#endif
