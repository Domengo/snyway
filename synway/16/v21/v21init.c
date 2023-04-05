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
#include "aceext.h"

void V21_Struc_Init(V21Struct *pV21)
{
    if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
    {
#if V21_7200
        ACEStruct *pAce = (ACEStruct *)pV21->pTable[ACE_STRUC_IDX];

        pV21->ubBufferSize           = V21_BUFSIZE_72;

        pV21->ubMaxSilenceRef        = pAce->pT30ToDce->carrierLossTimeout * V21_CALL_RATE;//V21_MAX_SILENCE_72;

        pV21->ubCh1_PDEL_Delay_Len   = V21CH1_PDEL_DELAY_LENGTH_72;
        pV21->ubCh2_PDEL_Delay_Len   = V21CH2_PDEL_DELAY_LENGTH_72;

        pV21->qCh1_Modem_Beta        = V21_qCH1_MODEM_BETA_72;
        pV21->qCh2_Modem_Beta        = V21_qCH2_MODEM_BETA_72;

        pV21->qCh1_Mark_Delta_Phase  = V21_CH1_MARK_DELTA_PHASE_72;//???
        pV21->qCh1_Space_Delta_Phase = V21_CH1_SPACE_DELTA_PHASE_72;//???

        pV21->qCh2_Mark_Delta_Phase  = V21_CH2_MARK_DELTA_PHASE_72;//???
        pV21->qCh2_Space_Delta_Phase = V21_CH2_SPACE_DELTA_PHASE_72;//???

        pV21->qEnergy_Ref            = V21_ENERGY_REFERENCE_72;

        pV21->qCarrierLoss_Thres     = V21_CARRIER_LOSS_THRES_72;

        pV21->qdModem_AGC_Ref        = V21MODEM_AGC_REFERENCE_72;//???
        pV21->qdCh2Fax_AGC_Ref       = V21CH2F_AGC_REFERENCE_72;
#endif
    }
    else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
    {
#if V21_9600
        pV21->ubBufferSize           = V21_BUFSIZE_96;

        pV21->ubMaxSilenceRef        = V21_MAX_SILENCE_96;

        pV21->ubCh1_PDEL_Delay_Len   = V21CH1_PDEL_DELAY_LENGTH_96;
        pV21->ubCh2_PDEL_Delay_Len   = V21CH2_PDEL_DELAY_LENGTH_96;

        pV21->qCh1_Modem_Beta        = V21_qCH1_MODEM_BETA_96;
        pV21->qCh2_Modem_Beta        = V21_qCH2_MODEM_BETA_96;

        pV21->qCh1_Mark_Delta_Phase  = V21_CH1_MARK_DELTA_PHASE_96;//???
        pV21->qCh1_Space_Delta_Phase = V21_CH1_SPACE_DELTA_PHASE_96;//???

        pV21->qCh2_Mark_Delta_Phase  = V21_CH2_MARK_DELTA_PHASE_96;//???
        pV21->qCh2_Space_Delta_Phase = V21_CH2_SPACE_DELTA_PHASE_96;//???

        pV21->qEnergy_Ref            = V21_ENERGY_REFERENCE_96;

        pV21->qCarrierLoss_Thres     = V21_CARRIER_LOSS_THRES_96;

        pV21->qdModem_AGC_Ref        = V21MODEM_AGC_REFERENCE_96;//???
        pV21->qdCh2Fax_AGC_Ref       = V21CH2F_AGC_REFERENCE_96;
#endif
    }
}
