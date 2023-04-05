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
#include "v29ext.h"
#include "ptable.h"
#include "mhsp.h"
#include "hwapi.h"
#include "gaoapi.h"

void V29_Dummy(V29Struct *pV29)
{
}

void V29_DPCS_Init(UBYTE **pTable)
{
    UBYTE      *pHwApi = (UBYTE *) pTable[HWAPIDATA_IDX];
    V29Struct  *pV29  = (V29Struct *)(pTable[V29_STRUC_IDX]) ;
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    SWORD *pPCMout   = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);
#if SUPPORT_V54
    UBYTE LALtype;
    UBYTE isLoopBack;
#endif
    UBYTE isReceiveImage_mode;
    UBYTE ubTraintype, ubRate;
    UBYTE i;

    pHwApi[0] = HWAPI_BUFSIZE | HWAPI_SAMPLERATE;
    pHwApi[1] = HWAPI_7200SR;
    pHwApi[2] = 72;

    ubTraintype = RdReg(pDpcs->MFCF0, TCF_OR_IMAGE);

    /* initialize structure */
    memset(pV29, 0, sizeof(V29Struct));

    pV29->pTable = pTable;

    PutReg(pDpcs->MMSR0, V29);

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
            pV29->ImageSendMode = ANALOG_LOOP_BACK;    /* analog loop test */
        }
        else
        {
            pV29->ImageSendMode = DIGITAL_LOOP_BACK;    /* digital loop test */
        }
    }
    else
#endif
    {
        if (isReceiveImage_mode)
        {
            pV29->ImageSendMode = IMAGE_RECEIVE_MODE;

            for (i = 0; i < V29_BUF_SIZE; i++)
            {
                pPCMout[i] = 0;
            }

            TRACE0("V29: Receive Image MODE");
        }
        else
        {
            pV29->ImageSendMode = IMAGE_SEND_MODE;

            TRACE0("V29: Send Image MODE");
        }
    }

    /* Get Data Rates from DPCS */
    ubRate = RdReg(pDpcs->MBC1, MAX_RATE);

    /* define the bits per symbol according to Rate.
    Rate will be determined by V21 modem */
    if (ubRate == 7)
    {
        pV29->ubRateSig    = V29_9600T;
        pV29->ubBitsPerSym = V29_BITS_PER_SYMBOL_9600;
    }
    else
#if V29_MODEM_ENABLE
        if (ubRate == 6)
#endif
        {
            pV29->ubRateSig    = V29_7200T;
            pV29->ubBitsPerSym = V29_BITS_PER_SYMBOL_7200;
        }

#if V29_MODEM_ENABLE
        else /* if (ubRate == 5) */
        {
            pV29->ubRateSig    = V29_4800T;
            pV29->ubBitsPerSym = V29_BITS_PER_SYMBOL_4800;
        }

#endif

    /* Report Status as Training Mode */
    pV29->ubTraintype = ubTraintype;

    /* initializes real time vectors */
    pMhsp->pfRealTime = V29_isr;
    pMhsp->RtCallRate = 1;
    pMhsp->RtBufSize  = V29_BUF_SIZE;

    /* initialize V.29 modem */
    V29_init_main(pV29);

    if (pV29->ubTraintype == V29_IMAGE_DATA)
    {
        pV29->qTimingErr1  = pFaxShareMem->qTimingErr1;
        pV29->qdCarErrPhase = ((QDWORD) pFaxShareMem->qCarErrPhase) << 16;
        pV29->qSagcScale   = pFaxShareMem->qFaxAgc;
    }
    else
    {
        if (pFaxShareMem->qV21Agc > 0)
        {
            pV29->qSagcScale = pFaxShareMem->qV21Agc;
        }
        else
        {
            pV29->qSagcScale = 256;
        }
    }

    pDpcs->BITSPERCYL = pMhsp->RtCallRate * pV29->ubBitsPerSym;

    TRACE0("V29: ********** V29 Start ************");
}

void V29_init_main(V29Struct *pV29)
{
    /* assign pointers to actual functions */
    V29_FSM_init(pV29);

    V29_TX_init(pV29);
    V29_RX_init(pV29);

    TRACE2("V29: Init, ImageSendMode=%d, BitsPerSym=%d", pV29->ImageSendMode, pV29->ubBitsPerSym);
}

void V29_TX_init(V29Struct *pV29)
{
    /* TX functions initialize */
    pV29->udScramSReg = 0;        /* Scrambler */
    pV29->sbOld_phase = 0;        /* Encoder */
    V29_iFilter_init(pV29);       /* Filter */
    pV29->ModPhase    = 0;        /* Modulator */

    pV29->isTxDataMode = 0;
}

void V29_RX_init(V29Struct *pV29)
{
    DftStruct *pDft = &(pV29->Dft);

    pV29->sTimeOutCounter = 0;

    pV29->ubSagc_Flag    = 0;
    pV29->qdSagcEnergy   = 0;

    pV29->qdCarrLoss_egy = 0;
    pV29->CarrLoss_Flag  = 0;

    pV29->SilenceCount   = 0;

    V29_Demodulate_init(pV29);
    V29_AutoCorrDetect_init(pV29);

    RX_Timing_init(&pV29->Poly);    /* Timing loop */
    pV29->ubTimingPhase = 0;

    DspcFir_2T3EQInit(&pV29->cfirEq, pV29->cEqCoef, pV29->cEqDline, V29_EQ_LENGTH); /* Equalizer */
    pV29->TrainScramSReg = 0x00;    /* TrainScrambler */

    V29_Slicer_init(pV29);          /* Slicer */
    V29_CarrierRecovery_init(pV29); /* Carrier Recovery */
    V29_TimingUpdate_init(pV29);    /* Timing Recovery */
    pV29->DescramSReg = 0;          /* Descrambler */

#if SUPPORT_ENERGY_REPORT
    pV29->Detect_FaxTone = 0;
#endif

    DSPD_DFT_Init(pDft, 72, V29_TONE_NUMBER, (SWORD *)V29_DetectTone_Coef, pV29->qdMagSq, pV29->qQ1Q2, 4);
}
