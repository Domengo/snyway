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

#include "v21ext.h"
#include "ptable.h"
#if SUPPORT_FAX_DATAPUMP
#include "faxshmem.h"
#endif
#include "aceext.h"

void V21_ModemCh2(V21Struct *pV21)
{
    pV21->TxBits = 0x01;
    V21_Ch2Tx(pV21);

    if (pV21->Detect_tone == 1)
    {
        pV21->pfTxVec = V21_Ch2Tx;
        pV21->Count_Tx = 100;
    }
}

void V21_Ch2MarkTone(V21Struct *pV21)
{
    pV21->TxBits = 0x01;
    V21_Ch2Tx(pV21);

    pV21->Count_Tx --;

    if (pV21->Count_Tx == 0)
    {
        pV21->TxMode  = 1; /* Data mode */
        pV21->pfTxVec = V21_Ch2Tx;
    }
}

void V21_Ch2Tx(V21Struct *pV21)
{
    SWORD qDeltaphase;

    if (pV21->TxBits)
    {
        qDeltaphase = pV21->qCh2_Mark_Delta_Phase;
    }
    else
    {
        qDeltaphase = pV21->qCh2_Space_Delta_Phase;
    }

    V21_Tx(pV21, qDeltaphase);
}

#if SUPPORT_V21CH2FAX

void V21_Ch2Fax_Detect_tone(V21Struct *pV21)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV21->pTable[DPCS_STRUC_IDX]);
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pV21->pTable[FAXSHAREMEM_IDX]);
    QWORD  qFsample, qTemp;
    QDWORD qdAvg_Energy0, qdAvg_Energy21;
    UBYTE  i;

    qdAvg_Energy0  = 0;
    qdAvg_Energy21 = 0;

    pV21->Ch2TimeOutCount ++;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        qFsample = pV21->PCMinPtr[i];

        qTemp = DspFir(&(pV21->BpfHigh), qFsample);

        /* Full band energy */
        qdAvg_Energy0 += QQMULQD(qFsample, qFsample) >> 8;

        /* V.21 band energy */
        qdAvg_Energy21 += QQMULQD(qTemp, qTemp) >> 8;
    }

    /* V.21 band energy at least 25% of total energy */
    if ((qdAvg_Energy21 > 2000)  && (qdAvg_Energy21 > (qdAvg_Energy0 >> 2))) // fixing issue 123, the original threshold for qdAvg_Energy21 is 2000
    {
        pV21->DetCounter++;

        if (pV21->DetCounter >= 30) /* 30 = 100ms */
        {
            pV21->Ch2TimeOutCount = 0;

            pV21->Detect_tone = 1;
            pV21->DetCounter  = 0;

            pV21->pfRxVec = V21_Ch2Rx;

            pV21->RxMode = 1; /* data mode */

            PutReg(pDpcs->MSR0, DATA);
            PutReg(pDpcs->MBSC,   DATA_RATE_300);
            PutReg(pDpcs->MBSCTX, DATA_RATE_300);

            TRACE0("V21: Fax Ch2 Tone Detected");
            pV21->qdCarrierThreshold = 0;
            pV21->Carrier_Flag       = 0;

            qTemp = QDsqrt(qdAvg_Energy21);
            pV21->qAgc_gain = (QWORD)QDQDdivQD(pV21->qdCh2Fax_AGC_Ref, qTemp);
            TRACE2("V21: (2)qEgy_sqrt = %d, pV21->qAgc_gain = %d", qTemp, pV21->qAgc_gain);

            pFaxShareMem->qV21Agc = pV21->qAgc_gain;
        }

        pV21->Silence_samples = 0;
    }
    else
    {
        pV21->DetCounter = 0;

        pV21->Silence_samples++;
    }
}

void V21_Ch2Td(V21Struct *pV21)  /* Transmit silence */
{
    UBYTE  i;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        *pV21->PCMoutPtr++ = 0;
    }
}

#endif

#if SUPPORT_V21CH2MODEM
void V21_Ch2Modem_Detect_tone(V21Struct *pV21)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV21->pTable[DPCS_STRUC_IDX]);
    QWORD  qFsample1, qFsample2, qTemp;
    QDWORD qdAvg_Energy0, qdAvg_Energy21;
    UBYTE  i;

    qdAvg_Energy0  = 0;
    qdAvg_Energy21 = 0;

    for (i = 0; i < pV21->ubBufferSize; i++)
    {
        qFsample1 = pV21->PCMinPtr[i];

        /* Dummy call, filling delayline */
        qTemp = DspFir(&(pV21->BpfHigh), qFsample1);

        /* Look for absolute power */
        qdAvg_Energy0 += QQMULQD(qFsample1, qFsample1) >> 8;

#if (V21_7200 && V21_9600)

        if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
        {
            qFsample2 = DspIIR_Cas51(&(pV21->BpfToneIIR_72), qFsample1);
        }
        else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
        {
            qFsample2 = DspFir(&(pV21->BpfToneIIR_96), qFsample1);
        }

#elif V21_7200
        qFsample2 = DspIIR_Cas51(&(pV21->BpfToneIIR_72), qFsample1);
#elif V21_9600
        qFsample2 = DspFir(&(pV21->BpfToneIIR_96), qFsample1);
#endif

        qdAvg_Energy21 += QQMULQD(qFsample2, qFsample2) >> 8;
    }

    if ((qdAvg_Energy21 >= (qdAvg_Energy0 >> 2)) && (qdAvg_Energy21 > pV21->qEnergy_Ref))
    {
        pV21->DetCounter++;

        if (pV21->DetCounter > 6)
        {
            pV21->Ch2TimeOutCount = 0;

            pV21->Detect_tone = 1;
            pV21->DetCounter  = 0;

            pV21->pfRxVec = V21_Ch2Rx;

            pV21->RxMode = 1; /* data mode */

            PutReg(pDpcs->MSR0, DATA);
            PutReg(pDpcs->MBSC,   DATA_RATE_300);
            PutReg(pDpcs->MBSCTX, DATA_RATE_300);

            TRACE0("V21: Modem Ch2 Tone Detected");
            pV21->qdCarrierThreshold = 0;
            pV21->Carrier_Flag       = 0;

            qTemp = QDsqrt(qdAvg_Energy21);
            pV21->qAgc_gain = (QWORD)QDQDdivQD(pV21->qdModem_AGC_Ref, qTemp);
            TRACE2("V21: (3)qEgy_sqrt = %d, pV21->qAgc_gain = %d", qTemp, pV21->qAgc_gain);
        }
        else
        {
            pV21->Ch2TimeOutCount ++;
        }
    }
    else
    {
        if (qdAvg_Energy21 < (pV21->qEnergy_Ref >> 2))
        {
            pV21->Ch2TimeOutCount ++;
        }

        pV21->DetCounter = 0;
    }
}
#endif

void V21_Ch2Rx(V21Struct *pV21)
{
    V21_Rx(pV21, V21_CH2);
}

void V21_Ch2Rd(V21Struct *pV21)
{
}
