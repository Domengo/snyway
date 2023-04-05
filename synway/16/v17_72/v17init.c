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

#include "v17ext.h"

void V17_Dummy(V17Struct *pV17)
{
}

void V17_init(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    V17_FSM_init(pV17);

    V17_TX_Init(pV17);

    V17_RX_Init(pV17);

#if SUPPORT_V17/* The switch is only for compiling, cannot delete!!! */
    pV32Share->ubCnt = 0;
#endif

    /* init. 'nTxState' and 'nTxDelayCnt' to start handshaking */
    pV32Share->nTxState     = 0;
    pV17->pfTxSetUpVec      = pV17->pfTxSetUpVecTab[pV32Share->nTxState];
    pV32Share->nTxDelayCnt  = pV32Share->nTxDelayCntTab[pV32Share->nTxState];
    pV17->pfTxSetUpVec(pV17);

    /* init. 'nRxState' and 'nRxDelayCnt' to start handshaking */
    pV32Share->nRxState     = 0;
    pV17->pfRxSetUpVec      = pV17->pfRxSetUpVecTab[pV32Share->nRxState];
    pV32Share->nRxDelayCnt  = pV32Share->nRxDelayCntTab[pV32Share->nRxState];
    pV17->pfRxSetUpVec(pV17);
}

void V17_TX_Init(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    pV32Share->ubTxBitsPerSym  = 2;
    pV32Share->udScramSReg = 0; /* V17_ScramInit */

    V32_DiffEncInit(pV32Share);
    V32_EncodeInit(pV32Share);

    pV32Share->pcSigMapTab = V17_IQTable_4800NR;

    V32_PsfInit(pV17->pTable, pV32Share);

    pV32Share->uModPhase = V32_PHASE_180;       /** 128=256/2 **/

    pV32Share->isRxDataMode = 0;
    pV32Share->isTxDataMode = 0;
}

void V17_RX_Init(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    DftStruct           *pDft = &(pV32Share->Dft);

    pV17->sTimeOutCounter     = 0;
    pV32Share->ubRxBitsPerSym = 2;
    pV32Share->ubSigDemapSize = 4;

    pV32Share->ubSagc_Flag    = 0;
    pV32Share->qdSagcEnergy   = 0;

    pV17->qdCarrLoss_egy      = 0;
    pV17->CarrLoss_Flag       = 0;

    pV17->SilenceCount        = 0;

    pV32Share->isRxDataMode   = 0;
    pV32Share->isTxDataMode   = 0;

    /* initialize RX functions */
    V32_DemodInit(pV32Share);
    V17_AutoCorrDetect_init(pV17);
    RX_Timing_init(&pV32Share->Poly);

    V32_TimingUpdate_init(pV32Share);

    DspcFir_2T3EQInit(&pV32Share->cfirEq_D, pV32Share->cEqCoef_D, pV32Share->cEqDline_D, V32_EQ_LENGTH);

    V32_CarrierRecovery_init(pV32Share);

    pV32Share->udTrainScramSReg = 0;
    V32_SliceInit(pV32Share);

    if (pV17->ubTraintype == V17_LONG_TRAIN)
    {
        V17_RX_TimeJam_Init(pV32Share);
    }

    V32_RX_Decode_init(pV32Share);

    V32_DiffDecInit(pV32Share);
    pV32Share->udDescramSReg = 0;

#if SUPPORT_ENERGY_REPORT
    pV17->Detect_FaxTone = 0;
#endif

    DSPD_DFT_Init(pDft, 36, V17_TONE_NUMBER, (QWORD *)qV17_DetectToneCoefTab, pV32Share->qdMagSq, pV32Share->qQ1Q2, 4);
}
