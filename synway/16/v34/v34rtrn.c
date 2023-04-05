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
#include "mhsp.h"
#include "hwapi.h"
#include "v34ext.h"

#if 0 /// LLL temp
/********************************************************************/
/* check the retrain request, this function use the Echo err buffer */
/* before you call the function, make sure the err buffer is filled */
/********************************************************************/
void V34_Check_Rtrn_Req(ReceiveStruc *pRx)
{
    V34StateStruc *pV34State = &pRx->V34State;
    RetrainStruc  *pRT = &pRx->Rtrn;
    V34AgcStruc   *pAGC = &(pRx->rx_AGC);
    QWORD  tmp;
    UBYTE  i, result;

    result = 0;

    for (i = 0; i < EC_INTER_NUM; i++)
    {
        tmp = QQMULQR8(pRx->qEchoCancel_Out[i], pAGC->qGain);

#if 0

        if (DumpTone1_Idx < 1000000) { DumpTone1[DumpTone1_Idx++] = tmp; }

#endif

        result |= V34_DFT(&(pRT->tonedet), tmp);
    }

    if (result == 1)
    {
        V34_Check_Retrain_Tone(pRT);
    }

    if (pRT->ToneDet_Flag == 1)
    {
        TRACE0("Retrain Tone Detected");

        pRT->ToneDet_Flag = 0;

        pRx->Retrain_flag         = RETRAIN_RESPOND;
        pV34State->retrain_reason = V34_RETRAIN_DETECTED;
    }
}

void  V34_Test_Retrain_State(UBYTE **pTable)
{
    DpcsStruct    *pDpcs     = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    Ph2Struct     *pPH2      = (Ph2Struct *)(pTable[PH2_STRUC_IDX]);
    V34Struct     *pV34      = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct   *pV34Rx    = &(pV34->V34Rx);
    ReceiveStruc  *pRx       = &pV34Rx->RecStruc;
    V34AgcStruc   *pAGC      = &(pRx->rx_AGC);
    V34StateStruc *pV34State = &pRx->V34State;
    UBYTE ubTemp;

    UBYTE S2743;                 /* bit 12 */
    UBYTE S2800;                 /* bit 13 */
    UBYTE S3429;                 /* bit 14 */
    UBYTE S3000_low_carrier;     /* bit 15 */
    UBYTE S3000_high_carrier;    /* bit 16 */
    UBYTE S3200_low_carrier;     /* bit 17 */
    UBYTE S3200_high_carrier;    /* bit 18 */
    UBYTE S3429_enable;          /* bit 19 */
    UBYTE reduce_transmit_power; /* bit 20 */
    UBYTE max_allowed_diff;      /* bit 21-23, value 0-5 */
    UBYTE transmit_from_CME;     /* bit 24 */
    UBYTE V34bis;                /* bit 25 */

    UBYTE  Retrain_Times;
    SWORD  AGC_gain;
    UBYTE  modem_mode;

    Info_0Struc *pInfo;

    if (pRx->Renego.clear_down == 1)
    {
        TRACE0("GOT RTRN in CLRDWN, DISC!!");
        pV34State->hang_up = 1;
        return;
    }

    ubTemp = RdReg(pDpcs->MCR0, RTRND);

    if ((pRx->Retrain_flag == RETRAIN_RESPOND) && (ubTemp == 0))
    {
        return;
    }

    pV34State->Retrain_Times ++;

    PutReg(pDpcs->DEBUGSR, (0x80 + pV34State->retrain_reason));
    PutReg(pDpcs->MSR0, RTRN_DET);

    pRx->Retrain_flag = 0;

    TRACE0("Set retrain bit in V34");

    /* Save Information required by Phase2 */
    Retrain_Times = pV34State->Retrain_Times;

    AGC_gain = pAGC->AGC_gain;

    modem_mode            = pV34Rx->modem_mode;
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

    memset(pPH2, 0, sizeof(Ph2Struct));

#if SUPPORT_V34FAX
    pPH2->V34fax_Enable = 0;
#endif
    /* Restore Information for Phase2 */
    pPH2->Ph2State.Retrain_Times = Retrain_Times;

    pPH2->AGC_gain = AGC_gain;

    pPH2->modem_mode = modem_mode;

#if (SUPPORT_V90A + SUPPORT_V90D)
    pPH2->V90_Enable  = 0;
    pPH2->Retrain_V90 = V90A_FALLBACK_V34;
#endif

    if (pPH2->modem_mode == CALL_MODEM)
    {
        pInfo = &(pPH2->I0a);
    }
    else
    {
        pInfo = &(pPH2->I0c);
    }

    pInfo->S2743                  = S2743;
    pInfo->S2800                  = S2800;
    pInfo->S3429                  = S3429;
    pInfo->S3000_low_carrier      = S3000_low_carrier;
    pInfo->S3000_high_carrier     = S3000_high_carrier;
    pInfo->S3200_low_carrier      = S3200_low_carrier;
    pInfo->S3200_high_carrier     = S3200_high_carrier;
    pInfo->S3429_enable           = S3429_enable;
    pInfo->reduce_transmit_power  = reduce_transmit_power;
    pInfo->max_allowed_diff       = max_allowed_diff;
    pInfo->transmit_from_CME      = transmit_from_CME;
    pInfo->V34bis                 = V34bis;

    Ph2_Init(pTable);
}

void V34_Check_Retrain_Tone(RetrainStruc *pRT)/* check retrain tone */
{
#if 0

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pRT->pqMag_sqr[V34_RTN_TONE]; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pRT->pqMag_sqr[V34_RTN_REF]; }

#endif

    if (((pRT->pqMag_sqr[V34_RTN_TONE] >> 4) > pRT->pqMag_sqr[V34_RTN_REF]) && (pRT->pqMag_sqr[V34_RTN_TONE] > 200))
    {
        ++pRT->tone_count;

        if (pRT->tone_count > pRT->Tone_50ms)
        {
            pRT->ToneDet_Flag = 1;
            pRT->tone_count   = 0;
        }
    }
    else
    {
        pRT->tone_count = 0;
    }
}
#endif
