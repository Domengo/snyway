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
#include "mhsp.h"
#include "ptable.h"
#include "v17ext.h"
#include "hwapi.h"
#include "gaoapi.h"

void V17_DPCS_Init(UBYTE **pTable)
{
    UBYTE          *pHwApi    = (UBYTE *)pTable[HWAPIDATA_IDX];
    V17Struct       *pV17      = (V17Struct *)(pTable[V17_STRUC_IDX]);
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    MhspStruct     *pMhsp     = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    DpcsStruct     *pDpcs     = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);
#if SUPPORT_V54
    UBYTE LALtype;
    UBYTE isLoopBack;
#endif
    UBYTE isReceiveImage_mode;
    UBYTE ubTraintype, ubRate;

    TRACE0("V17: DPCS INIT Called");

    ubTraintype = RdReg(pDpcs->MFCF0, TCF_OR_IMAGE);

    /* initialize structure */
    memset(pV17, 0, sizeof(V17Struct));

    pV17->pTable = pTable;

    /* Report V17 is used */
    PutReg(pDpcs->MMSR0, V17);

    /* Check if send or receive image */
    isReceiveImage_mode = RdReg(pDpcs->MCR0, RECEIVE_IMAGE);

    /* Check if loopback mode */
#if SUPPORT_V54
    isLoopBack = RdReg(pDpcs->MCR0, TEST_MODE);

    if (isLoopBack)
    {
        /* Report Status as LDL and LAL */
        LALtype = RdReg(pDpcs->MTC, LAL_MODE);

        if (LALtype)
        {
            pV17->ImageSendMode = ANALOG_LOOP_BACK;
        }
        else
        {
            pV17->ImageSendMode = DIGITAL_LOOP_BACK;
        }
    }
    else
#endif
    {
        if (isReceiveImage_mode)
        {
            pV17->ImageSendMode = IMAGE_RECEIVE_MODE;

            TRACE0("V17: Receive Image MODE");
        }
        else
        {
            pV17->ImageSendMode = IMAGE_SEND_MODE;

            TRACE0("V17: Send Image MODE");
        }
    }

    /* Report Status as Training Mode */
    pV17->ubTraintype = ubTraintype;

    /* Get Data Rates from DPCS */
    ubRate = RdReg(pDpcs->MBC1, MAX_RATE);

    switch (ubRate)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            pV17->ubTxRateIdx = V17_RATE_IDX_7200;
            pV17->ubRxRateIdx = V17_RATE_IDX_7200;
            break;
        case 7:
            pV17->ubTxRateIdx = V17_RATE_IDX_9600;
            pV17->ubRxRateIdx = V17_RATE_IDX_9600;
            break;
        case 8:
            pV17->ubTxRateIdx = V17_RATE_IDX_12000;
            pV17->ubRxRateIdx = V17_RATE_IDX_12000;
            break;
        default:
            pV17->ubTxRateIdx = V17_RATE_IDX_14400;
            pV17->ubRxRateIdx = V17_RATE_IDX_14400;
            break;
    }

    /* initialize V.17 modem */
    V17_init(pV17);

    if (pV17->ubTraintype == V17_SHORT_TRAIN)
    {
        pV32Share->qTimingErr1  = pFaxShareMem->qTimingErr1;
        pV32Share->qCarErrPhase = pFaxShareMem->qCarErrPhase;
        pV32Share->qSagcScale   = pFaxShareMem->qFaxAgc;

#if V32_FREQUENCY_OFFSET
        pV32Share->nDemodDeltaOffset = pFaxShareMem->nDemodDeltaOffset;
#endif
    }
    else
    {
        TRACE1("V17: V21 gain: %d", pFaxShareMem->qV21Agc);

        if (pFaxShareMem->qV21Agc > 0)
        {
            pV32Share->qSagcScale = pFaxShareMem->qV21Agc;
        }
        else
        {
            pV32Share->qSagcScale = 0xFF;
        }
    }

    /* select hardware buffer size */
    /* Hardware Initialization for Sampling rate */
    pHwApi    = (UBYTE *)pTable[HWAPIDATA_IDX];

    pHwApi[0] = HWAPI_BUFSIZE | HWAPI_SAMPLERATE;
    pHwApi[1] = HWAPI_7200SR;
    pHwApi[2] = 72;   /* 36 during Ranging,after Ranging it will be 72 */

    /* setup real-time vector */
    pMhsp->pfRealTime = V17_DPCS_ISR;
    pMhsp->RtCallRate = 2;
    pMhsp->RtBufSize  = V32_BUF_SIZE;

    pDpcs->BITSPERCYL = pMhsp->RtCallRate * pV32Share->ubTxBitsPerSym;

    TRACE0("V17: ********** Start ************");
}

void V17_DPCS_ISR(UBYTE **pTable)
{
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);
    V17Struct    *pV17 = (V17Struct *)(pTable[V17_STRUC_IDX]);
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
#if DRAWEPG
    SWORD       *pEpg = (SWORD *)pTable[EPGDATA_IDX];
#endif
    QWORD      *pPCMin = (QWORD *)(pTable[PCMINDATA_IDX]);
    QWORD     *pPCMout = (QWORD *)(pTable[PCMOUTDATA_IDX]);
    UBYTE    i, Disc;

#if DRAWEPG
    *pEpg = 0;  /* reset number of points for EPG */
#endif

    /* setup buffers  */
    pV32Share->PCMinPtr  = pPCMin;
    pV32Share->PCMoutPtr = pPCMout;

    for (i = 0; i < V32_BUF_SIZE; i++)
    {
        pPCMout[i] = 0;
    }

    /* process samples */
    V17_isr(pV17);

    if ((pV32Share->isRxDataMode == 1) || (pV32Share->isTxDataMode == 1)) /* setup data mode */
    {
        PutReg(pDpcs->MSR0, DATA);
    }

    if (pV17->V21DetectNumber >= 2) /* V21 signal is detected */
    {
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
        TRACE0("V17: Disconnect after V21 detected");

        /* report V21 signal detected to T30 */
        SetReg(pDpcs->MFSR0, FAX_V21);

        return;
    }

    /* check for a disconnect */
    Disc = RdReg(pDpcs->MCF, DISCONNECT);

    /* check for carrier loss in the data mode */
    if (pV17->CarrLoss_Flag == 2)
    {
        /* initialization for checking carrier loss next time */
        pV17->CarrLoss_Flag = 0;
        Disc = 1;
        TRACE0("V17: Carrier lost Detected");
    }

    if (pV17->sTimeOutCounter > 4000)
    {
        pV17->sTimeOutCounter = 0;
        Disc = 1;
    }

    if (Disc)
    {
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
        TRACE0("V17: DISCONNECT");

        pFaxShareMem->qTimingErr1  = pV32Share->qTimingErr1;
        pFaxShareMem->qCarErrPhase = pV32Share->qCarErrPhase;
        pFaxShareMem->qFaxAgc      = pV32Share->qSagcScale;

#if V32_FREQUENCY_OFFSET
        pFaxShareMem->nDemodDeltaOffset = pV32Share->nDemodDeltaOffset;
#endif
    }
}
