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

#include <stdio.h>
#include "porting.h"
#include "ptable.h"
#include "acestru.h"
#include "hwstru.h"
#include "hwapi.h"
#include "gaoapi.h"
#include "sregdef.h"
#include "gaostru.h"

#if SAMCONV
#include "dspdext.h"
#endif

SWORD GAO_ModStatus_Init(GAO_ModStatus *pStatus)
{
    pStatus->StateChange  = SAMPLE_RATE_CHG | BUFFER_SIZE_CHG;

    pStatus->HookState    = ON_HOOK;
#if (SUPPORT_CID_DET + SUPPORT_CID_GEN)
    pStatus->CidState     = CID_OFF;
#endif
    pStatus->RingDetected = 0;
    pStatus->ModemState   = OFF_LINE;
    pStatus->ModemCmd     = NO_CMD;
    pStatus->TX_RATE      = 255;/* Not Connected Flag */
    pStatus->RX_RATE      = 255;/* Not Connected Flag */

    pStatus->ConfirmBufferSize = HW_DEF_BUFSIZE;
    pStatus->BufferSize = HW_DEF_BUFSIZE;

    pStatus->SampRate   = HW_DEF_SAM_RATE;

    pStatus->ubFlowType = FLOWCONTROL_HW;
    pStatus->ubSerPortRate = 0;  /* Not set by AT command yet */

    return 0;
}

SWORD GAO_ModStatus_Upd(void *pLibDataMem, GAO_ModStatus *pStatus)
{
    GAO_LibStruct *pLib = (GAO_LibStruct *)pLibDataMem;
    GaoModemStruct *pModem = &(pLib->m_Modem);
    UBYTE     **pTable = (UBYTE **)pModem;
    HwStruct   *pHw    = (HwStruct *)pTable[HW_STRUC_IDX];
    UBYTE      *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];
    DpcsStruct *pDpcs  = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    ACEStruct  *pAce   = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE      *pSreg  = (UBYTE *)pTable[ACESREGDATA_IDX];
    UBYTE HwApiStatus;
    UBYTE i;
#if SAMCONV
    UBYTE ubTemp;
    UBYTE buffersize = 0;
    SDWORD sdTemp;
#endif

    HwApiStatus = pHwApi[0];/* Check hardware control field */
    pStatus->CountryCode = (UBYTE)pSreg[35];

    /* Pass modem request to hardware */
    if ((HwApiStatus & HWAPI_ONHOOK) || (HwApiStatus & HWAPI_CPONHOOK))
    {
        pStatus->HookState = ON_HOOK;
        pStatus->StateChange |= HOOK_STATE_CHG;
        pHw->OnOffHook = HW_ONHOOK;
    }
    else if ((HwApiStatus & HWAPI_OFFHOOK) || (HwApiStatus & HWAPI_CPOFFHOOK))
    {
        pStatus->HookState = OFF_HOOK;
        pStatus->StateChange |= HOOK_STATE_CHG;
        pHw->OnOffHook = HW_OFFHOOK;
    }

#if (SUPPORT_CID_DET || SUPPORT_CID_GEN)

    if (HwApiStatus & HWAPI_CIDON)
    {
        pStatus->CidState = CID_ON;
        pStatus->StateChange |= CID_STATE_CHG;
    }
    else if (HwApiStatus & HWAPI_CIDOFF)
    {
        pStatus->CidState = CID_OFF;
        pStatus->StateChange |= CID_STATE_CHG;
    }

#endif

    /* Sampling rate and buffersize */
#if SAMCONV
    ubTemp = 0;
#endif

    if (HwApiStatus & HWAPI_SAMPLERATE)
    {
#if SAMCONV

        if (pHw->SampRate != pHwApi[1])
        {
            ubTemp = 1;
            pHw->SampRate = pHwApi[1];
        }

#else
        pHw->SampRate = pHwApi[1];
        pStatus->SampRate = pHw->SampRate;
        pStatus->StateChange |= SAMPLE_RATE_CHG;
#endif
    }

    if (HwApiStatus & HWAPI_BUFSIZE)
    {
#if SAMCONV

        if (pHw->BufferSize != pHwApi[2])
        {
            ubTemp = 1;

            pHw->BufferSize = pHwApi[2];
        }

#else
        pHw->BufferSize = pHwApi[2];

        pStatus->BufferSize = pHw->BufferSize;

        pStatus->StateChange |= BUFFER_SIZE_CHG;
#endif
    }

#if SAMCONV

    if (ubTemp == 1)
    {
        if (pHw->SampRate == HWAPI_7200SR)
        {
            sdTemp = QQMULQD(pHw->BufferSize, 8000);
            buffersize = (UBYTE)QDQDdivQD(sdTemp, 7200);
        }
        else if (pHw->SampRate == HWAPI_9000SR)
        {
            sdTemp = QQMULQD(pHw->BufferSize, 8000);
            buffersize = (UBYTE)QDQDdivQD(sdTemp, 9000);
        }
        else if (pHw->SampRate == HWAPI_9600SR)
        {
            sdTemp = QQMULQD(pHw->BufferSize, 8000);
            buffersize = (UBYTE)QDQDdivQD(sdTemp, 9600);
        }
        else if (pHw->SampRate == HWAPI_10287SR)
        {
            if (pHw->BufferSize == 90)
            {
                buffersize = 70;
            }
            else
            {
                TRACE0("Sth wrong");
            }
        }
        else if (pHw->SampRate == HWAPI_8000SR)
        {
            buffersize = pHw->BufferSize;
        }
        else if (pHw->SampRate == HWAPI_8400SR)
        {
            sdTemp = QQMULQD(pHw->BufferSize, 8000);
            buffersize = (UBYTE)QDQDdivQD(sdTemp, 8400);
        }
        else if (pHw->SampRate == HWAPI_8229SR)
        {
            if (pHw->BufferSize == 72)
            {
                buffersize = 70;
            }
            else
            {
                TRACE0("Sth wrong");
            }
        }

        if (pStatus->BufferSize != buffersize)
        {
            pStatus->BufferSize = buffersize;

            pStatus->StateChange |= BUFFER_SIZE_CHG;
        }

        SwChgSamRate(pTable, pHw->SampRate, buffersize);
    }

#endif
    /* End of Sampling rate and buffersize */

    /* reset hardware control register */
    pHwApi[0] = 0;

    /* Pass current modem state and connect bit rate to customer */

#if !SUPPORT_DTE
    pStatus->ubFlowType = pAce->ubFlowControlType;

    pStatus->CarrierDetect_Flag = pAce->CarrierDetect_Flag;

    pStatus->ubSerPortRate = pAce->ubSerPortRate;

    if (pAce->CarrierDetect_Flag == 1)/* Modem connected */
    {
        pStatus->RX_RATE = GetReg(pDpcs->MBSC);
        pStatus->TX_RATE = GetReg(pDpcs->MBSCTX);
    }
    else//EPR#378/* After our modem connect, we will report current connect rate. After modem disconnect, we should report 255 instead of previous connect rate. */
    {
        pStatus->TX_RATE = 255;
        pStatus->RX_RATE = 255;
    }

#endif

    if (pStatus->ModemState != pAce->LineState)
    {
        pStatus->ModemState = pAce->LineState;
    }

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pAce->LineState; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pStatus->ModemState; }

#endif

    /* Pass retrain and hangup reason to cousomer */
    pStatus->ubDebugSR = GetReg(pDpcs->DEBUGSR);

    ClrReg(pDpcs->DEBUGSR);

    /* Pass hardware ring detect status to modem */
    if ((pStatus->RingDetected == 1) && (pHw->OnOffHook == HW_ONHOOK))
    {
        PutReg(pDpcs->MSR0, RING_DET);/* Set RING_DET status */
    }
    else
    {
        ResetReg(pDpcs->MSR0, RING_DET);/* Reset RING detected status, Can't use ClrReg!!! */
    }

    pStatus->RingDetected = 0;/* clear it */

    /* Pass customer's disconnect request to modem */
    if (DISCONNECT_CMD == pStatus->ModemCmd)
    {
        PutReg(pDpcs->MCF, DISCONNECT);
        pStatus->ModemCmd = NO_CMD;
    }
    else if (RETRAIN_CMD == pStatus->ModemCmd)
    {
        PutReg(pDpcs->MCF, RTRN);
        pStatus->ModemCmd = NO_CMD;
    }

    pStatus->RetrainRerate_flag  = RdReg(pDpcs->MCF,  RTRN     | RRATE);
    pStatus->RetrainRerate_flag |= RdReg(pDpcs->MSR0, RTRN_DET | RRATE_DET);

    for (i = 0; i < NUM_DRIVER_PARAMS; i++)
    {
        pStatus->ParameterToDriver[i] = pSreg[DRIVER_PARAMS_STARTIDX + i];
    }

    return 0;
}

extern void MDM_Proc(UBYTE *pModemDataMem, QWORD *PCM_in, QWORD *PCM_out);

SWORD GAO_PCM_Proc(void *pLibDataMem, SWORD *pPCMIn, SWORD *pPCMOut)
{
    GAO_LibStruct *pLib = (GAO_LibStruct *)(pLibDataMem);

#if SUPPORT_T30

    if (RdReg(pLib->m_Modem.m_Ace.FaxClassType, FCLASS_MAJ) > FCLASS1)
    {
        T30_Main(&pLib->m_T30);
    }

#endif

    MDM_Proc((UBYTE *)(&(pLib->m_Modem)), pPCMIn, pPCMOut);

    return 0;
}
