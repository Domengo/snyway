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
#include "ptable.h"
#include "hwapi.h"
#include "v34fext.h"
#include "mhsp.h"
#include "dcestru.h"

#if SUPPORT_V34FAX

void V34F_Test_Retrain_State(UBYTE **pTable)
{
    DpcsStruct   *pDpcs      = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    Ph2Struct    *pPH2       = (Ph2Struct *)(pTable[PH2_STRUC_IDX]);
    Info_hStruc  *pIh        = &(pPH2->Ih);
    V34Struct     *pV34      = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct   *pV34Rx    = &(pV34->V34Rx);
    ReceiveStruc  *pRx       = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    V34StateStruc *pV34State = &pRx->V34State;

    UBYTE power_reduction;
    UBYTE trn_length;
    UBYTE high_freq;
    UBYTE preemp_idx;
    UBYTE symbol_rate;
    UBYTE TRN_constellation_point;

    UBYTE S2743;
    UBYTE S2800;
    UBYTE S3429;
    UBYTE S3000_low_carrier;
    UBYTE S3000_high_carrier;
    UBYTE S3200_low_carrier;
    UBYTE S3200_high_carrier;
    UBYTE S3429_enable;
    UBYTE reduce_transmit_power;
    UBYTE max_allowed_diff;
    UBYTE transmit_from_CME;
    UBYTE V34bis;

    UBYTE  Retrain_Times;
    SWORD  AGC_gain;
    UBYTE  modem_mode;

    Info_0Struc *pInfo;

    pV34State->Retrain_Times ++;

    PutReg(pDpcs->DEBUGSR, (0x80 + pV34State->retrain_reason));
    PutReg(pDpcs->MSR0, RTRN_DET);

    pRx->Retrain_flag = 0;

    TRACE0("Set retrain bit in V34");

    Retrain_Times = pV34State->Retrain_Times;

    AGC_gain = pAGC->AGC_gain;

    modem_mode = pV34Fax->modem_mode;

    S2743                 = pV34Rx->S2743;
    S2800                 = pV34Rx->S2800;
    S3429                 = pV34Rx->S3429;
    S3000_low_carrier     = pV34Rx->S3000_low_carrier;
    S3000_high_carrier    = pV34Rx->S3000_high_carrier;
    S3200_low_carrier     = pV34Rx->S3200_low_carrier;
    S3200_high_carrier    = pV34Rx->S3200_high_carrier;
    S3429_enable          = pV34Rx->S3429_enable;
    reduce_transmit_power = pV34Rx->reduce_transmit_power;
    max_allowed_diff      = pV34Rx->max_allowed_diff;
    transmit_from_CME     = pV34Rx->transmit_from_CME;
    V34bis                = pV34Rx->V34bis;

    power_reduction   = pV34Fax->power_reduction;
    trn_length        = pV34Fax->trn_length;
    high_freq         = pV34Fax->high_carrier_freq;
    preemp_idx        = pV34Fax->preemp_idx;
    symbol_rate       = pV34Fax->symbol_rate;

    TRN_constellation_point = pV34Fax->TRN_constellation_point;

    memset(pPH2, 0, sizeof(Ph2Struct));

    pPH2->V34fax_Enable = 1;

    pPH2->Ph2State.Retrain_Times = Retrain_Times;

    pPH2->AGC_gain = AGC_gain;

    pPH2->modem_mode = modem_mode;

    if (pPH2->modem_mode == CALL_MODEM)
    {
        pInfo = &(pPH2->I0a);
    }
    else
    {
        pInfo = &(pPH2->I0c);
    }

    pInfo->S2743                 = 0;
    pInfo->S2800                 = 0;
    pInfo->S3429                 = 0;
    pInfo->S3000_low_carrier     = 0;
    pInfo->S3000_high_carrier    = 0;
    pInfo->S3200_low_carrier     = 0;
    pInfo->S3200_high_carrier    = 0;
    pInfo->S3429_enable          = 0;
    pInfo->reduce_transmit_power = reduce_transmit_power;
    pInfo->max_allowed_diff      = max_allowed_diff;
    pInfo->transmit_from_CME     = transmit_from_CME;
    pInfo->V34bis                = V34bis;

    pIh->power_reduction         = power_reduction;
    pIh->trn_length              = trn_length;
    pIh->high_carrier_freq       = high_freq;
    pIh->pre_em_index            = preemp_idx;
    pIh->sym_rate_call_to_answer = symbol_rate;
    pIh->TRN_constellation       = TRN_constellation_point;

    Ph2_Init(pTable);
}

#endif
