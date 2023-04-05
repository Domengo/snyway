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
#include "v27ext.h"
#include "ptable.h"
#include "mhsp.h"
#include "hwapi.h"
#include "gaoapi.h"
#if V27_HW7200SAMPLERATE_SUPPORT
#include "smp.h"
#endif

void V27_Dummy(V27Struct *pV27)
{
}

void V27_DPCS_Init(UBYTE **pTable)
{
    UBYTE      *pHwApi = (UBYTE *) pTable[HWAPIDATA_IDX];
    V27Struct  *pV27  = (V27Struct *)(pTable[V27_STRUC_IDX]) ;
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    SWORD *pPCMout   = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);
#if V27_HW7200SAMPLERATE_SUPPORT
    InDe_Struct *pMultiFilterOut = (InDe_Struct *)(pTable[SMPDATAOUT_IDX]);
    InDe_Struct *pMultiFilterIn  = (InDe_Struct *)(pTable[SMPDATAIN_IDX]);
#endif
#if SUPPORT_V54
    UBYTE LALtype;
    UBYTE isLoopBack;
#endif
    UBYTE isReceiveImage_mode;
    UBYTE ubTraintype, ubRate;
    UBYTE i;

    pHwApi[0] = HWAPI_BUFSIZE | HWAPI_SAMPLERATE;

#if V27_HW7200SAMPLERATE_SUPPORT
    pHwApi[1] = HWAPI_7200SR;
    pHwApi[2] = 72;

    Init_InterDeci(pMultiFilterOut, 3, 4, INDE3_4LEN, (QWORD *)lpf3_4_h, 96, 72);
    Init_InterDeci(pMultiFilterIn,  4, 3, INDE4_3LEN, (QWORD *)lpf4_3_h, 72, 96);
#else
    pHwApi[1] = HWAPI_9600SR;
    pHwApi[2] = 96;
#endif

    /* initialize structure */
    memset(pV27, 0, sizeof(V27Struct));

    pV27->pTable = pTable;

    PutReg(pDpcs->MMSR0, V27TER);

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
            pV27->ImageSendMode = ANALOG_LOOP_BACK;    /* analog loop test */
        }
        else
        {
            pV27->ImageSendMode = DIGITAL_LOOP_BACK;    /* digital loop test */
        }
    }
    else
#endif
    {
        if (isReceiveImage_mode)
        {
            pV27->ImageSendMode = IMAGE_RECEIVE_MODE;

            for (i = 0; i < V27_BUF_SIZE; i++)
            {
                pPCMout[i] = 0;
            }

            TRACE0("V27: Receive Image MODE");
        }
        else
        {
            pV27->ImageSendMode = IMAGE_SEND_MODE;

            TRACE0("V27: Send Image MODE");
        }
    }

    /* Get Data Rates from DPCS */
    ubRate = RdReg(pDpcs->MBC1, MAX_RATE);

    /* define the bits per symbol according to Rate.
    Rate will be determined by V21 modem */
    if (ubRate == 4)
    {
        pV27->ubRateSig = V27_2400;

        pV27->ubBitsPerSym = 2;
        /* initialize Tx struct parameters */
        pV27->ubSymBufSize = V27_SYM_SIZE_1200;
        pV27->ubNumSym     = V27_NUM_SYM_1200;
    }
    else
    {
        pV27->ubRateSig = V27_4800;

        pV27->ubBitsPerSym = 3;
        /* initialize Tx struct parameters */
        pV27->ubSymBufSize = V27_SYM_SIZE_1600;
        pV27->ubNumSym     = V27_NUM_SYM_1600;
    }

    /* initializes real time vectors */
    pMhsp->pfRealTime = V27_Dpcs_Isr;
    pMhsp->RtCallRate = 1;
#if V27_HW7200SAMPLERATE_SUPPORT
    pMhsp->RtBufSize  = 72;
#else
    pMhsp->RtBufSize  = V27_BUF_SIZE;
#endif

    pV27->Timer = 0;

    /* initialize V.27ter modem */
    V27_init_main(pV27);

    ubTraintype = RdReg(pDpcs->MFCF0, TCF_OR_IMAGE);/* Report Status as Training Mode */

    if (ubTraintype == V27_IMAGE_DATA)
    {
        pV27->qTimingErr1  = pFaxShareMem->qTimingErr1;
        pV27->qCarErrPhase = pFaxShareMem->qCarErrPhase;
        pV27->qSagcScale   = pFaxShareMem->qFaxAgc;
    }
    else
    {
        if (pFaxShareMem->qV21Agc > 0)
        {
            pV27->qSagcScale = pFaxShareMem->qV21Agc;
        }
        else
        {
            pV27->qSagcScale = 256;
        }
    }

    pDpcs->BITSPERCYL = pMhsp->RtCallRate * pV27->ubBitsPerSym;

    TRACE0("V27: ********** V27 Start ************");
}

void V27_init_main(V27Struct *pV27)
{
#if V27_SHORT_TRAIN_SUPPORT
    pV27->Short_Trn_Flag = 0; /* Determine Short or long Train */
#endif

    /* assign pointers to actual functions */
    V27_FSM_init(pV27);

    V27_TX_init(pV27);
    V27_RX_init(pV27);

    TRACE2("V27: Init, ImageSendMode=%d, BitsPerSym=%d", pV27->ImageSendMode, pV27->ubBitsPerSym);
}

void V27_TX_init(V27Struct *pV27)
{
    /* TX functions initialize */
    V27_Scrambler_init(pV27);     /* Scrambler */
    pV27->sbOld_phase = 0;        /* Encoder */
    V27_iFilter_init(pV27);       /* Filter */
    pV27->ModPhase = 128;;        /* Modulator */

    pV27->isTxDataMode = 0;
}

void V27_RX_init(V27Struct *pV27)
{
    DftStruct *pDft = &(pV27->Dft);
    UBYTE i;

    if (pV27->ubRateSig == V27_4800)
    {
        DSPD_DFT_Init(pDft, 48, V27_TONE_NUMBER, (QWORD *)V27_DetectTone_1600_Coef, pV27->qdMagSq, pV27->qQ1Q2, 4);
    }
    else
    {
        DSPD_DFT_Init(pDft, 48, V27_TONE_NUMBER, (QWORD *)V27_DetectTone_1200_Coef, pV27->qdMagSq, pV27->qQ1Q2, 4);
    }

    for (i = 0; i < pV27->ubSymBufSize; i++)
    {
        pV27->cDemodIQ_Prev[i].r  = 0;
        pV27->cDemodIQ_Prev[i].i  = 0;
    }

    pV27->ubSagc_Flag    = 0;
    pV27->qdSagcEnergy   = 0;

    pV27->qdCarrLoss_egy = 0;
    pV27->CarrLoss_Flag  = 0;

    pV27->SilenceCount   = 0;

    pV27->sTimeOutCounter = 0;

    V27_Demodulate_init(pV27);
    V27_AutoCorrDetect_init(pV27);

    RX_Timing_init(&pV27->Poly);
    pV27->ubTimingPhase = 0;

    DspcFirInit(&pV27->cfirEq, pV27->cEqCoef, pV27->pcEqDelay, V27_EQ_LENGTH); /* Equalizer */

    pV27->TrainScramSReg = 0x00;      /* TrainScrambler */
    pV27->sbTrainold_phase = 0;       /* TrainEncoder */
    V27_Slicer_init(pV27);            /* Slicer */
    V27_RX_TimingUpdate_init(pV27);   /* Timing Update */
    V27_CarrierRecovery_init(pV27);   /* Carrier recovery */
    V27_Descrambler_init(pV27);       /* Descrambler */

#if SUPPORT_ENERGY_REPORT
    pV27->Detect_FaxTone = 0;
#endif
}
