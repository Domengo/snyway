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
#include "v21ext.h"
#include "ptable.h"
#include "mhsp.h"
#include "hwapi.h"
#include "gaoapi.h"

#if SUPPORT_V21M

void V21_DPCS_Init(UBYTE **pTable)
{
#if SUPPORT_V54
    UBYTE Mode2;
#endif
    UBYTE Mode1, i;
    V21Struct  *pV21  = (V21Struct *)(pTable[V21_STRUC_IDX]) ;
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    UBYTE      *pHwApi = (UBYTE *) pTable[HWAPIDATA_IDX];
    QWORD *pPCMin  = (QWORD *)(pTable[PCMINDATA_IDX]);
    QWORD *pPCMout = (QWORD *)(pTable[PCMOUTDATA_IDX]);

    memset(pV21, 0, sizeof(V21Struct));

    pV21->ubSampleRate = V21_SAMPLE_RATE_9600;

    V21_Struc_Init(pV21);

    /* Hardware Initialization */
    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;

    if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
    {
        pHwApi[1] = HWAPI_7200SR;
        pHwApi[2] = 72;
    }
    else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
    {
        pHwApi[1] = HWAPI_9600SR;
        pHwApi[2] = 96;
    }

    for (i = 0; i < pHwApi[2]; i++)
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

    PutReg(pDpcs->MMSR0, MODEM_V21);

    Mode1 = RdReg(pDpcs->MCR0, ANSWER);
#if SUPPORT_V54
    Mode2 = RdReg(pDpcs->MCR0, TEST_MODE);

    if (Mode2)
    {
        Mode2 = RdReg(pDpcs->MTC, (LDL_MODE | LAL_MODE));

        if (Mode2) /* digital/analog loopback test */
        {
            pV21->modem_mode = DIGITAL_LOOP_BACK;
        }
    }
    else
#endif
    {
        if (Mode1 != 0)
        {
            pV21->modem_mode = ANS_MODEM;
        }
        else
        {
            pV21->modem_mode = CALL_MODEM;
        }
    }

    /* ====setting realtime pointer to V21_DPCS_isr */
    pMhsp->pfRealTime = V21_DPCS_isr;
    pMhsp->RtCallRate = V21_CALL_RATE;
    pMhsp->RtBufSize  = pV21->ubBufferSize;

    V21_Init_main(pV21);

    pV21->pTable = pTable;
}


void V21_Init_main(V21Struct *pV21)
{
    if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
    {
#if V21_7200
        DspFirInit(&(pV21->BpfHigh), (QWORD *)V21_tHFilter_Coef_72, pV21->pqDelaylineH, V21_FILTER_LENGTH_72);

        DspFirInit(&(pV21->LpFilter), (QWORD *)V21_tLpcoef_72, pV21->pqDelayline, V21_LOWPASS_FILTER_LENGTH_72);
#endif
    }
    else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
    {
#if V21_9600
        DspFirInit(&(pV21->BpfHigh), (QWORD *)V21_tHFilter_Coef_96, pV21->pqDelaylineH, V21_FILTER_LENGTH_96);

        DspFirInit(&(pV21->LpFilter), (QWORD *)V21_tLpcoef_96, pV21->pqDelayline, V21_LOWPASS_FILTER_LENGTH_96);
#endif
    }

    if (pV21->modem_mode == CALL_MODEM)
    {
        if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
        {
#if V21_7200
            DspFirInit(&(pV21->BpfLow), (QWORD *)V21_tLFilter_Coef_72,  pV21->pqDelaylineL, V21_FILTER_LENGTH_72);

            DspIIR_Cas51_Init(&(pV21->BpfToneIIR_72), (QWORD *)V21_tCallTone_FilterCoef_72, pV21->pqDelaylineT_72, V21_BIQUAD_NUM);
#endif
        }
        else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
        {
#if V21_9600
            DspFirInit(&(pV21->BpfLow), (QWORD *)V21_tLFilter_Coef_96,  pV21->pqDelaylineL, V21_FILTER_LENGTH_96);

            DspFirInit(&(pV21->BpfToneIIR_96), (QWORD *)V21_tCallTone_FilterCoef_96, pV21->pqDelaylineT_96, V21_FILTER_LENGTH_96);
#endif
        }

        pV21->BpfTxPtr = &(pV21->BpfLow);
        pV21->pfTxVec = V21_ModemCh1;
        pV21->pfRxVec = V21_Ch2Modem_Detect_tone;
    }
    else if (pV21->modem_mode == ANS_MODEM)
    {
        if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
        {
#if V21_7200
            DspFirInit(&(pV21->BpfLow), (QWORD *)V21_tBSFilter_Coef_72,  pV21->pqDelaylineL, V21_FILTER_LENGTH_72);

            DspIIR_Cas51_Init(&(pV21->BpfToneIIR_72), (QWORD *)V21_tAnsTone_FilterCoef_72, pV21->pqDelaylineT_72, V21_BIQUAD_NUM);
#endif
        }
        else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
        {
#if V21_9600
            DspFirInit(&(pV21->BpfLow), (QWORD *)V21_tBSFilter_Coef_96,  pV21->pqDelaylineL, V21_FILTER_LENGTH_96);

            DspFirInit(&(pV21->BpfToneIIR_96), (QWORD *)V21_tAnsTone_FilterCoef_96, pV21->pqDelaylineT_96, V21_FILTER_LENGTH_96);
#endif
        }

        pV21->BpfTxPtr = &(pV21->BpfHigh);
        pV21->pfTxVec = V21_ModemCh2;
        pV21->pfRxVec = V21_Ch1_Detect_tone;
    }

#if SUPPORT_V54
    else if (pV21->modem_mode == DIGITAL_LOOP_BACK)
    {
        if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
        {
#if V21_7200
            DspIIR_Cas51_Init(&(pV21->BpfToneIIR_72), (QWORD *)V21_tCallTone_FilterCoef_72, pV21->pqDelaylineT_72, V21_BIQUAD_NUM);
#endif
        }
        else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
        {
#if V21_9600
            DspFirInit(&(pV21->BpfToneIIR_96), (QWORD *)V21_tCallTone_FilterCoef_96, pV21->pqDelaylineT_96, V21_FILTER_LENGTH_96);
#endif
        }

        pV21->pfTxVec = V21_Ch2MarkTone;
        pV21->pfRxVec = V21_Ch2Modem_Detect_tone;
    }

#endif

    pV21->DetCounter      = 0;
    pV21->sbPdelIdx       = 0;
    pV21->Phase           = 0;
    pV21->ubCounter       = 0;
    pV21->Silence_samples = 0;
    pV21->State           = 1;
    pV21->TxBits          = 1;
    pV21->RxNumBits       = 0;
    pV21->qAgc_gain       = 256;
    pV21->Detect_tone     = 0;
    pV21->Count_Tx        = 100;
    pV21->Disconnect_flag = 0;
    pV21->TxMode          = 0;
    pV21->RxMode          = 0;
}

#endif
