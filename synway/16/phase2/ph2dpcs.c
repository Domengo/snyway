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
#include "v8ext.h"
#include "ph2ext.h"
#include "mhsp.h"   /* Modem Host Signal Processor */
#include "ptable.h" /* Data memory table defines   */
#include "hwapi.h"  /* Hardware Defines            */
#include "dpcs.h"

void Phase2_DPCS_Init(UBYTE **pTable)
{
    Ph2Struct   *pPH2 = (Ph2Struct *)(pTable[PH2_STRUC_IDX]);
    V8Struct     *pV8 = (V8Struct *)(pTable[V8_STRUC_IDX]);
    V21Struct   *pV21 = &(pV8->v21);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
#if DRAWEPG
    SWORD       *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    SWORD AGC_gain;
    SBYTE flag;
    UBYTE modem_mode_V8;
    UBYTE V90_Enable_V8;
    UBYTE V92_Enable_V8;
#if SUPPORT_V92A
    QDWORD drift_V8;
#endif
#if SUPPORT_V34FAX
    UBYTE V34fax_Enable;
#endif

    AGC_gain      = pV21->qAgc_gain;
    modem_mode_V8 = pV8->modem_mode;
    V90_Enable_V8 = pV8->V90_Enable;
    V92_Enable_V8 = pV8->V92_Enable;
#if SUPPORT_V34FAX
    V34fax_Enable = pV8->V34fax_Enable;
#endif
#if SUPPORT_V92A
    drift_V8 = pV8->drift;
#endif

    /* V.8 shares same memory block with Phase2 */
    /* Following memory clear will erase all V.8 data */
    memset(pPH2, 0, sizeof(Ph2Struct));

#if DRAWEPG
    *pEpg = 8;
#endif

    if (AGC_gain < 256)
    {
        AGC_gain = 256;
    }

    pPH2->AGC_gain   = AGC_gain;
    pPH2->modem_mode = modem_mode_V8;
    pPH2->V90_Enable = V90_Enable_V8;
    pPH2->V92_Enable = V92_Enable_V8;
#if SUPPORT_V92A
    pPH2->drift = drift_V8;
#endif

#if SUPPORT_V34FAX
    pPH2->V34fax_Enable = V34fax_Enable;
#endif

    /** Set modem modulation select register to V.34 **/
    PutReg(pDpcs->MMSR0, MODEM_V34);

    flag = GetReg(pDpcs->MCF);

    if (flag & DISCONNECT)
    {
        WrReg(pDpcs->ABORTSR, REASON, 0); /* Disconnect reason */
        PutReg(pDpcs->DEBUGSR, (0x61));
    }

#if 0
    /* Check if HOST want to do Loop back test */
    flag = RdReg(pDpcs->MCR0, TEST_MODE);

    if (flag)
    {
        /* V34_LAL_Startup(pTable); */
        return;
    }

#endif

    Ph2_Init(pTable);
}

void Ph2_DPCS(UBYTE **pTable)
{
    Ph2Struct   *pPH2 = (Ph2Struct *)(pTable[PH2_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    SBYTE flag, disc_req = 0;

#if SUPPORT_V90A

    if (pPH2->Ph2State.hang_up == PH2_NO_SYMBOL_RATE_SUPPORT)
    {
        WrReg(pDpcs->ABORTSR, REASON, 1);    /* Disconnect reason */
        pPH2->Ph2State.hang_up = 0;
        disc_req = 1;
        PutReg(pDpcs->DEBUGSR, (0x62));        /* PH2_NO_SYMBOL_RATE_SUPPORT */
    }

#endif

#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)

    if (pPH2->Ph2State.hang_up == PH2_HANGUP_V92MH_CLRD)
    {
        WrReg(pDpcs->ABORTSR, REASON, 1); /* Disconnect reason */
        pPH2->Ph2State.hang_up = 0;
        disc_req = 1;
        PutReg(pDpcs->DEBUGSR, (0x62));/* PH2_NO_SYMBOL_RATE_SUPPORT */
    }

    if (pPH2->Ph2State.hang_up == PH2_RECONNECT_V92MH)
    {
        V8_DPCS_Init(pTable);
        PutReg(pDpcs->MSR0, TRAIN);
    }

#endif

#if 0

    if (pPH2->Ph2Timer <= 0)
    {
        WrReg(pDpcs->ABORTSR, REASON, 2); /* Disconnect reason */
        disc_req = 1;
        PutReg(pDpcs->DEBUGSR, (0x63));
    }

#endif

    if (pPH2->Ph2State.Retrain_Times > V34_MAX_RETRAIN) /* allow three retrain */
    {
        WrReg(pDpcs->ABORTSR, REASON, 3); /* Disconnect reason */

        pPH2->Ph2State.Retrain_Times = 0;

        disc_req = 1;
        PutReg(pDpcs->DEBUGSR, (0x64));
        TRACE0("Too many retrains: Hang up");
    }

    flag = GetReg(pDpcs->MCF);

    if (flag & DISCONNECT)
    {
        ClrReg(pDpcs->MCF);
        WrReg(pDpcs->ABORTSR, REASON, 0); /* Disconnect reason */
        disc_req = 1;
        PutReg(pDpcs->DEBUGSR, (0x65));
    }

    if (pPH2->uToneAorB_Timeout_Count > 6000 || pPH2->uINFO0_detect_Timeout_Count > 6000 || pPH2->uTimeout_Count > 6000)
    {
        pPH2->uToneAorB_Timeout_Count = 0;

        pPH2->uINFO0_detect_Timeout_Count = 0;

        pPH2->uTimeout_Count = 0;

        disc_req = 1;
    }

    if (disc_req)
    {
        Disconnect_Init(pTable);
    }
}
