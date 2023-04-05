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
#include "dceext.h"

#if SUPPORT_V21CH2FAX

void V21ch2_DPCS_Init(UBYTE **pTable)
{
    V21Struct  *pV21  = (V21Struct *)(pTable[V21_STRUC_IDX]) ;
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    UBYTE      *pHwApi = (UBYTE *) pTable[HWAPIDATA_IDX];
    SWORD    *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    UBYTE Mode, i;

    memset(pV21, 0, sizeof(V21Struct));

    pV21->ubSampleRate = V21_SAMPLE_RATE_7200;

    pV21->pTable = pTable;
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

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        *pPCMout++ = 0;
    }

    PutReg(pDpcs->MMSR0, V21CH2);

    /* === Check V21fax mode: TX_RX =0 ->transmit, 1&up ->receive=== */
    Mode = RdReg(pDpcs->MFCF0, TX_RX);

    if (Mode != 0)
    {
        pV21->modem_mode = ANS_MODEM;
    }
    else
    {
        pV21->modem_mode = CALL_MODEM;
    }

    pMhsp->pfRealTime = V21_Ch2_DPCS_isr;
    pMhsp->RtCallRate = V21_CALL_RATE;
    pMhsp->RtBufSize  = pV21->ubBufferSize;

    V21_Ch2_Init_main(pV21);

    pV21->pTable = pTable;

    pV21->Ch2TimeOutCount = 0;
    pV21->ubCh2DiscCount  = 0;

    pDpcs->BITSPERCYL = pMhsp->RtCallRate * 1;

    TRACE0("V21: DPCS INIT Called");
}


void V21_Ch2_Init_main(V21Struct *pV21)
{
    if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
    {
#if V21_7200
        /* use only BpfHigh in any case */
        DspFirInit(&(pV21->BpfHigh), (QWORD *)V21_tHFilter_Coef_72, pV21->pqDelaylineH, V21_FILTER_LENGTH_72);

        /* don't need anstone now */
        DspFirInit(&(pV21->BpfLow), (QWORD *)V21_tLFilter_Coef_72, pV21->pqDelaylineL, V21_FILTER_LENGTH_72);

        /* low-pass filter in demodulator */
        DspFirInit(&(pV21->LpFilter), (QWORD *)V21_tLpcoef_72, pV21->pqDelayline, V21_LOWPASS_FILTER_LENGTH_72);
#endif
    }
    else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
    {
#if V21_9600
        /* use only BpfHigh in any case */
        DspFirInit(&(pV21->BpfHigh), (QWORD *)V21_tHFilter_Coef_96, pV21->pqDelaylineH, V21_FILTER_LENGTH_96);

        /* don't need anstone now */
        DspFirInit(&(pV21->BpfLow), (QWORD *)V21_tLFilter_Coef_96, pV21->pqDelaylineL, V21_FILTER_LENGTH_96);

        /* low-pass filter in demodulator */
        DspFirInit(&(pV21->LpFilter), (QWORD *)V21_tLpcoef_96, pV21->pqDelayline, V21_LOWPASS_FILTER_LENGTH_96);
#endif
    }

    if (pV21->modem_mode == ANS_MODEM)  /* ==== answewr, Fax-receive mode ==== */
    {
        if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
        {
#if V21_7200
            DspIIR_Cas51_Init((&pV21->BpfToneIIR_72), (QWORD *)V21_tCallTone_FilterCoef_72, pV21->pqDelaylineT_72, V21_BIQUAD_NUM);
#endif
        }
        else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
        {
            /* call tone filter - 1650 Hz */
#if V21_9600
            DspFirInit(&(pV21->BpfToneIIR_96), (QWORD *)V21_tCallTone_FilterCoef_96, pV21->pqDelaylineT_96, V21_FILTER_LENGTH_96);
#endif
        }

        pV21->pfTxVec = V21_Ch2Td;
        pV21->pfRxVec = V21_Ch2Fax_Detect_tone;
    }
    else /* ==== 1, which is CALL, Fax-transmit mode ==== */
    {
        pV21->BpfTxPtr = &(pV21->BpfHigh);
        pV21->pfTxVec  = V21_Ch2MarkTone;
        pV21->pfRxVec  = V21_Ch2Rd;
    }

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
#if SUPPORT_ENERGY_REPORT
    pV21->Detect_FaxTone  = 0;
    pV21->uFlagPattern    = 0xFFFF;
#endif
    pV21->Count_Tx        = 5;
    pV21->Disconnect_flag = 0;

    pV21->TxMode          = 0;
    pV21->RxMode          = 0;
}

#endif
