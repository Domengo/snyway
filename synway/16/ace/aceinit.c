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
#include "aceext.h"
#include "gaoapi.h"

void ACE_FactoryConfigurationInit(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    CircBuffer *pAscCBWr = (CircBuffer *)pTable[ASCCBWRDATA_IDX];
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];
    UBYTE i;
#if SUPPORT_PARSER
    CircBuffer *DteWr = pAce->pCBIn;
    CircBuffer *DteRd = pAce->pCBOut;

#if SUPPORT_T31_PARSER && !SUPPORT_T32_PARSER
    pAce->pT30ToDce->pIsV34Enabled = &pAce->isV34Enabled;
#endif

    /* AT parameters */
    pAce->Echo = 1;
    pAce->DialToneDet = 1;
    pAce->BusyDet = 1;
    pAce->Quiet = 0;
    pAce->Verbose = 1;
    pAce->Comma = 0;
    pAce->Semicolon = SEMICOLON_NONE;
    pAce->RingCount = 0;
    pAce->ubStringFlag = 0;
    pAce->ubFlowControlType = FLOWCONTROL_HW;

    pAce->FaxClassType = FCLASS0;

    CB_ResetCircBuffer(DteWr);
    CB_ResetCircBuffer(DteRd);

    /* init the pointers of function */
    pAce->AtActions[0]  = AT_A;
    pAce->AtActions[1]  = AT_D;
    pAce->AtActions[2]  = AT_S;
    pAce->AtActions[3]  = AT_H;
    pAce->AtActions[4]  = AT_Z;
    pAce->AtActions[5]  = AT_O;
    pAce->AtActions[6]  = AT_I;
    pAce->AtActions[7]  = AT_E;
    pAce->AtActions[8]  = AT_Q;
    pAce->AtActions[9]  = AT_V;
    pAce->AtActions[10] = AT_X;
    pAce->AtActions[11] = AT_L;
    pAce->AtActions[12] = AT_M;
    pAce->AtActions[13] = AT_P;
    pAce->AtActions[14] = AT_T;
    pAce->AtActions[15] = AT_and;
    pAce->AtActions[16] = AT_plus;

#if SUPPORT_T31_PARSER
    pAce->ClassFDD = 0;
#endif

#else
    pAce->FaxClassType = FCLASS3; // LLL for SUPPORT_DTE
#endif

    Ace_ChangeLineState(pAce, OFF_LINE);
    Ace_ChangeState(pAce, ACE_NOF);

    pAce->CPState = NO_CP;


    CB_InitCircBuffer(pAscCBWr, pTable[ASCTXDATA_IDX], BUFSIZE);
    CB_InitCircBuffer(pAscCBRd, pTable[ASCRXDATA_IDX], BUFSIZE);

#if SUPPORT_V34FAX
    pAce->V8Detect_State  = V8_PASS_NO;
#if SUPPORT_T31_PARSER
    pAce->selectedV34Chnl = 0;
#endif
#endif

    /* reset control registers */
    PutReg(pDpcs->MCR0, RTRND);    /* defaut enable auto retrain, controlled by Sreg21 */

    ClrReg(pDpcs->MCR1);

    /* PutReg(pDpcs->MBR, 0xC0); */ /* 2400 Symbol Rate Only */
    /* PutReg(pDpcs->MBR, 0xC9); */ /* 2743 Symbol Rate Only */
    /* PutReg(pDpcs->MBR, 0xD2); */ /* 2800 Symbol Rate Only */
    /* PutReg(pDpcs->MBR, 0xDB); */ /* 3000 Symbol Rate Only */
    /* PutReg(pDpcs->MBR, 0xE4); */ /* 3200 Symbol Rate Only */
    /* PutReg(pDpcs->MBR, 0xED); */ /* 3429 Symbol Rate Only */
    PutReg(pDpcs->MBR, 0xC5);       /* 2400 to 3429 */

    ClrReg(pDpcs->MBC0);
    ClrReg(pDpcs->MBC1);

    /* set s registers to default values */
    for (i = 0; i < NUMBER_S_REGISTERS; i++)
    {
        pSreg[i] = SRegDefVal[i];
    }

#if SUPPORT_FXS
    pSreg[BLIND_DIAL_PAUSE_TIME] = 0;    /// LLL don't need to dial at all
#endif
}

void ACEInit(UBYTE **pTable)
{
    ACE_FactoryConfigurationInit(pTable);
}

/* Initiate modem connection */
void MhostConnect(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
#if DRAWEPG
    SWORD *pEpg = (SWORD *)pTable[EPGDATA_IDX];
#endif
    CircBuffer *pAscCBWr = (CircBuffer *)pTable[ASCCBWRDATA_IDX];
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];

#if DRAWEPG
    *pEpg = 0; /* reset EPG display */
#endif

    /* clear modem status register */
    ClrReg(pDpcs->MSR0);
    ClrReg(pDpcs->MSR1);

    /* clear modem RTD value */
    ClrReg(pDpcs->RTD);

    /* set connect flag */
    PutReg(pDpcs->MCF, CONNECT);

#if 1//(!SUPPORT_V34FAX)
    ClrReg(pDpcs->MFCF0);
#endif

#if SUPPORT_FAX
#if SUPPORT_V34FAX

    if (*pAce->pT30ToDce->pIsV34Enabled)
    {
        PutReg(pDpcs->MMR0, V34FAX);
    }
    else
#endif
    {
        PutReg(pDpcs->MMR0, V21CH2);
    }

    SetReg(pDpcs->MCR0, FAX);
#endif

    SetReg(pDpcs->MMR0, CP);     /* Call progress */

    /* initialize IO */
    CB_InitCircBuffer(pAscCBWr, pTable[ASCTXDATA_IDX], BUFSIZE);
    CB_InitCircBuffer(pAscCBRd, pTable[ASCRXDATA_IDX], BUFSIZE);
}
