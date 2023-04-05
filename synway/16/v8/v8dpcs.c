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
#include "ptable.h"
#include "mhsp.h"
#include "hwapi.h"
#include "aceext.h"

void V8_Disconnect(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    SWORD     *pPCMin = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD    *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    UBYTE i;

    ClrReg(pDpcs->MCF); /* Clear Disconnect flag */

    for (i = 0; i < pMhsp->RtBufSize; i++)
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

    Disconnect_Init(pTable);
}

/********************/
/* V.8  Entry point */
/********************/
void V8_DPCS_Init(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    UBYTE     *pHwApi = (UBYTE *)(pTable[HWAPIDATA_IDX]);
    V8Struct     *pV8 = (V8Struct *)(pTable[V8_STRUC_IDX]);
    SWORD     *pPCMin = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD    *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    V21Struct   *pV21 = &(pV8->v21);
#if SUPPORT_V34FAX
    ACEStruct   *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isV34Enabled = *pAce->pT30ToDce->pIsV34Enabled;
#endif
#if SUPPORT_V92A
    UBYTE      *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
#endif
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
#if DRAWEPG
    SWORD       *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    UBYTE flag, i;

    memset(pV8, 0, sizeof(V8Struct));

#if SUPPORT_V34FAX

    if (isV34Enabled)
    {
        pV8->V34fax_Enable = 1;

        pV21->ubSampleRate = V21_SAMPLE_RATE_7200;
    }
    else
#endif
    {
#if SUPPORT_V34FAX
        pV8->V34fax_Enable = 0;
#endif

#if SUPPORT_V92A
        pV21->ubSampleRate = V21_SAMPLE_RATE_9600;
#else
        pV21->ubSampleRate = V21_SAMPLE_RATE_7200;
#endif
    }

    pV21->pTable = pTable;
    V21_Struc_Init(pV21);

    pV8->call_rate = V8_CALL_RATE;

    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;

    if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
    {
        pHwApi[1] = HWAPI_7200SR;
        pHwApi[2] = 72;

        pV8->AnsamTime_Len   = V8_SMPL_450MS_72;
        pV8->qdAnsam_AGC_Ref = ANSAM_AGC_REF_72;

        pV8->DeltaPhase_2100 = DELTAPHASE_2100_72;
        pV8->DeltaPhase_15   = DELTAPHASE_15_72;
        pV8->DeltaPhase_2085 = DELTAPHASE_2085_72;
        pV8->DeltaPhase_2115 = DELTAPHASE_2115_72;
#if SUPPORT_CT
        pV8->DeltaPhase_1300 = DELTAPHASE_1300_72;
#endif

#if SUPPORT_V8BIS
        pV8->DeltaPhase_400  = DELTAPHASE_400_72;
        pV8->DeltaPhase_1375 = DELTAPHASE_1375_72;
        pV8->DeltaPhase_2002 = DELTAPHASE_2002_72;
#endif
    }
    else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
    {
        pHwApi[1] = HWAPI_9600SR;
        pHwApi[2] = 96;

        pV8->AnsamTime_Len   = V8_SMPL_450MS_96;
        pV8->qdAnsam_AGC_Ref = ANSAM_AGC_REF_96;

        pV8->DeltaPhase_2100 = DELTAPHASE_2100_96;
        pV8->DeltaPhase_15   = DELTAPHASE_15_96;
        pV8->DeltaPhase_2085 = DELTAPHASE_2085_96;
        pV8->DeltaPhase_2115 = DELTAPHASE_2115_96;
#if SUPPORT_CT
        pV8->DeltaPhase_1300 = DELTAPHASE_1300_96;
#endif

#if SUPPORT_V8BIS
        pV8->DeltaPhase_400  = DELTAPHASE_400_96;
        pV8->DeltaPhase_1375 = DELTAPHASE_1375_96;
        pV8->DeltaPhase_2002 = DELTAPHASE_2002_96;
#endif
    }

#if USE_ANS
    /** Check modem mode, Originate or Answer **/
    flag = RdReg(pDpcs->MCR0, ANSWER);

    if (0 == flag)
    {
        pV8->modem_mode = CALL_MODEM;
    }
    else
    {
        pV8->modem_mode = ANS_MODEM;
    }

#else
    pV8->modem_mode = CALL_MODEM;
#endif

#if DRAWEPG
    *pEpg = 8;
#endif

    flag = GetReg(pDpcs->MCF);

    if (flag & DISCONNECT)
    {
        WrReg(pDpcs->ABORTSR, REASON, 0); /* Disconnect reason */
        V8_Disconnect(pTable);
    }

    pMhsp->pfRealTime = V8_Handler;
    pMhsp->RtCallRate = 1;
    pMhsp->RtBufSize  = pHwApi[2];

    for (i = 0; i < pMhsp->RtBufSize; i++)
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

    /* Check if HOST want to do Loop back test */
#if SUPPORT_V54
    flag = RdReg(pDpcs->MCR0, TEST_MODE);

    if (flag)
    {
        /* V34_LAL_Startup(pTable); */
        return;
    }

#endif

    /* V.34 Phase 1 Entry point */
    pV8->V90_Enable    = 0;    /* Enabled below if required */

    if (pV8->modem_mode == CALL_MODEM)
    {
#if SUPPORT_V90A
#if SUPPORT_V90D

        if (ModemAorB == 0)
#endif
        {
            flag = GetReg(pDpcs->MMR0);

            if ((flag == MODEM_V90) || (flag == MODEM_V92) || (flag == 0))
            {
                pV8->V90_Enable = 1;    /* Auto V.90a default */
            }
        }

#endif
    }

#if USE_ANS
    else
    {
#if SUPPORT_V90D
#if SUPPORT_V90A

        if (ModemAorB == 1)
#endif
        {
            flag = GetReg(pDpcs->MMR0);

            if (flag == MODEM_V90 || flag == 0)
            {
                pV8->V90_Enable = 1;    /* Auto V.90d default */
            }
        }

#endif
    }

#endif /* USE_ANS */

#if SUPPORT_V92A

    if (((pSreg[V92_OPTIONS] & QUICK_CONNECT_DISABLE) == 0) && (pV8->V90_Enable))
    {
        pV8->QCEnabled = 1;
    }
    else
    {
        pV8->QCEnabled = 0;
    }

#if SUPPORT_V8BIS

    if ((pV8->V90_Enable == 1) && (pV8->QCEnabled))
    {
        pV8->V8bisEnable = 1;
    }
    else
    {
        pV8->V8bisEnable = 0;
    }

#endif

#endif

    V8_Init(pTable);

    TRACE0("V8: DPCS INIT");
}


void V8_DPCS_Phase1(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    V8Struct     *pV8 = (V8Struct *)(pTable[V8_STRUC_IDX]);
    UBYTE disc, flag;

    disc = 0;

    if (pV8->hang_up == 1)
    {
        WrReg(pDpcs->ABORTSR, REASON, 1); /* Disconnect reason */
        pV8->hang_up = 0;
        disc = 1;
    }

    flag = GetReg(pDpcs->MCF);

    if (flag & DISCONNECT)
    {
        WrReg(pDpcs->ABORTSR, REASON, 0); /* Disconnect reason */
        pV8->hangup_reason = 9; /* V34_HOST_DISCONNECT; */
        disc = 1; /* TRUE; */
    }

#if SUPPORT_V8BIS

    if (pV8->V8bisTimeout == 1)
    {
        pV8->V8bisEnable  = 0;
        pV8->V8bisTimeout = 0;

        V8_Init(pTable);
    }

#endif

    if (disc)
    {
        V8_Disconnect(pTable);
    }
}
