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
#include "mhsp.h"
#include "hwapi.h"
#include "gaoapi.h"
#include "cpext.h"

void Cp_DPCS_Init(UBYTE **pTable)
{
    UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];
    CpStruct *pCp = (CpStruct *)pTable[CP_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    DtmfGenStruct *pDtmf = &pCp->Dtmf;
    SWORD Temp100, Temp10, Temp5;
    UBYTE Mode;
    UBYTE modType;

    memset(pCp, 0, sizeof(CpStruct));
    pCp->pTable = pTable;

    /* init. call progress */
    pDtmf->OffTime    = QQMULQR15(pSreg[11], q01); /* Sets the spcing for tone dialing. */
    pDtmf->OnTime     = pDtmf->OffTime;            /* Sets the duration fo tone dialing. */
    pCp->pCmdString   = pTable[HOSTMSGDATA_IDX];
    pCp->CmdStringIdx = 0;

    Mode = RdReg(pDpcs->TELCOCR1, PULSE_CONTROL);
    Mode = (Mode >> 4) & 0x3;

    Temp100 = UBUBMULU(q100, CP_CALL_RATE);
    Temp10  = UBUBMULU(q10,  CP_CALL_RATE);
    Temp5   = UBUBMULU(q5 ,  CP_CALL_RATE);

    /* Init Pulse dialing parameters */
    switch (Mode)
    {
        case 0: /* Make/Break ratio 39/61, 10pps */
            pCp->DigitOffTime    = Temp100;
            pCp->nPulseMakeTime  = QQMULQR8(CP_CONST1, CP_CALL_RATE);/* 39/10*256 = 998 */
            pCp->nPulseBreakTime = Temp10 - pCp->nPulseMakeTime;
            break;
        case 1: /* Make/Break ratio 33/67, 10pps */
            pCp->DigitOffTime    = Temp100;
            pCp->nPulseMakeTime  = QQMULQR8(CP_CONST2, CP_CALL_RATE);/* 33/10*256 = 845 */
            pCp->nPulseBreakTime = Temp10 - pCp->nPulseMakeTime;
            break;
        case 2: /* Make/Break ratio 39/61, 20pps */
            pCp->DigitOffTime    = Temp100;
            pCp->nPulseMakeTime  = QQMULQR8(CP_CONST3, CP_CALL_RATE);/* 39/20*256 = 499 */
            pCp->nPulseBreakTime = Temp5 - pCp->nPulseMakeTime;
            break;
        case 3: /* Make/Break ratio 33/67, 20pps */
            pCp->DigitOffTime    = Temp100;
            pCp->nPulseMakeTime  = QQMULQR8(CP_CONST4, CP_CALL_RATE);/* 33/20*256 = 422 */
            pCp->nPulseBreakTime = Temp5 - pCp->nPulseMakeTime;
            break;
    }

    pMhsp->pfRealTime = CP_DpcsIsr;
    pMhsp->RtCallRate = CP_CALL_RATE;
    pMhsp->RtBufSize  = CP_BUF_SIZE;

    /* V8 mode is on or off */
    modType = GetReg(pDpcs->MMR0);

    pCp->V25PhaseChange        = PHASE_180_DEGREE;/* 180 degree phase change */
    pCp->V25PhaseChangeCounter = 0;

    if (modType == MODEM_V34 || modType == MODEM_V90 || modType == AUTOMODE || modType == V34FAX)
    {
        pCp->V8Mode = 1;
    }
    else if (modType == BELL103 || modType == BELL212A) /* when B103 or BELL212A */
    {
        pCp->V8Mode = 2;
    }
    else
    {
        pCp->V8Mode = 0;
    }

    /* call or ans mode */
    ClrReg(pDpcs->DIALEND);
    Mode = RdReg(pDpcs->MCR0, ANSWER);

    if (Mode)
    {
        pCp->modem_mode = ANS_MODEM;
    }
    else
    {
        pCp->modem_mode = CALL_MODEM;
    }

    pCp->ubCountry       = pSreg[35];
    pDtmf->sbLevelRef_dB = pSreg[36];

    /* call init function */
    CP_init(pCp);

    /* select hardware buffer size */
    pHwApi[0] |= HWAPI_BUFSIZE | HWAPI_SAMPLERATE;
    pHwApi[1]  = CP_HWAPI_SR;
    pHwApi[2]  = CP_BUF_SIZE;

    TRACE0("CP: DPCS INIT Called");
    /* TRACE0("CP has send the ask to change the sample rate to 7200"); */
}
