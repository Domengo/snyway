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

#include "ioapi.h"
#include "ptable.h"
#include "aceext.h"
#include "v34fext.h"

#if SUPPORT_V34FAX

#define DUMP_CONSTELLATION              (0)     /* t3 & t4 */

#define RECEIVED_DATA                   (0)     /* t1-bits */
#define RECEIVED_E                      (0)     /* t1-bits */
#define RECEIVED_ONES                   (0)     /* t1-bits */

#define RECEIVED_PAGE                   (3)     /* page number */

void V34Fax_AC_Detect(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34TxStruct  *pV34Tx  = &(pV34->V34Tx);
    QDWORD qdAvg_Energy;
    QWORD  qPcmIn[V34FAX_SYM_LEN];
    UBYTE  i;

    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        qPcmIn[i] = (QWORD)(QQMULQD(pV34Tx->PCMinPtr[i], pV34Fax->qGain) >> 8);
    }

    qdAvg_Energy = DSPD_DFT_Detect(&(pV34Fax->Dft_AC), qPcmIn, V34FAX_SYM_LEN);

    if (qdAvg_Energy > 0)
    {
        if ((pV34Fax->qdMagSq_AC[0] + pV34Fax->qdMagSq_AC[1]) > 6000000)
        {
            if ((qdAvg_Energy > MPET_PPH_SH_MIN_LEVEL_REF)
             && (pV34Fax->qdMagSq_AC[0]  > (qdAvg_Energy << 1))
             && (pV34Fax->qdMagSq_AC[1] > (qdAvg_Energy << 1))
             && (pV34Fax->qdMagSq_AC[2] < (qdAvg_Energy >> 2)))
            {
                pV34Fax->MPET_Retrn_AC_count ++;
            }
            else
            {
                pV34Fax->MPET_Retrn_AC_count = 0;
            }
        }
        else
        {
            pV34Fax->MPET_Retrn_AC_count = 0;
        }

        if (pV34Fax->MPET_Retrn_AC_count >= 5)
        {
            pV34Fax->MPET_Retrn_Type = RETRAIN_RESPOND;
            pV34Fax->MPET_Retrn_AC_count = 0;
            pV34Fax->AC_Detect_Start_Flag = 0;
            pV34Fax->MPET_Retrning = 0;
        }
    }
}


void V34Fax_Symbol_Rx(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);

    if (pV34Fax->nRxDelayCnt <= 0)
    {
        pV34Fax->nRxState++;
        pV34Fax->pfRxSetUpVec = pV34Fax->pfRxSetUpVecTab[pV34Fax->nRxState];
        pV34Fax->nRxDelayCnt  = pV34Fax->nRxDelayCntTab[pV34Fax->nRxState];

        pV34Fax->pfRxSetUpVec(pV34Fax);
    }

    V34Fax_Rx_BPF(pV34);

    pV34Fax->pfDetectVec(pV34);

    V34Fax_Gain(pV34Fax);

    V34Fax_Demodulation(pV34Fax);

    pV34Fax->nTimIdx -= (QWORD)V34FAX_SYM_LEN << V34FAX_TIMELINE_RES;

    while (pV34Fax->nTimIdx < 0)
    {
        V34Fax_Timing_Recovery(pV34Fax);

        pV34Fax->pfEqVec(pV34Fax);

        V34Fax_Rotate(pV34Fax);

        pV34Fax->pfSliceVec(pV34Fax);
        pV34Fax->pfTimUpdateVec(pV34Fax);
        pV34Fax->pfDerotateVec(pV34Fax);
        pV34Fax->pfCarrierVec(pV34Fax);
        pV34Fax->pfEqUpdateVec(pV34Fax);
        pV34Fax->pfDecodeVec(pV34Fax);
        pV34Fax->pfDescramVec(pV34Fax);
        pV34Fax->pfDataOutVec(pV34Fax);

        if (pV34Fax->AC_Detect_Start_Flag == 1)
        {
            V34Fax_AC_Detect(pV34);
        }

        pV34Fax->nTimIdx += (QWORD)V34FAX_SYM_LEN << V34FAX_TIMELINE_RES;
    }

    pV34Fax->nRxDelayCnt--;

#if DUMP_CONSTELLATION

    if (pV34Fax->Enter_CC_Order == RECEIVED_PAGE)
    {
        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV34Fax->cqRotateIQ.r; }

        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV34Fax->cqRotateIQ.i; }
    }

#endif
}


void V34Fax_PPh_Sh_Detection(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34TxStruct  *pV34Tx  = &(pV34->V34Tx);
    QDWORD qdAvg_Energy;
    QDWORD qdPPh_Energy;
    QDWORD qdSh_Energy;
    QWORD  qPcmIn[V34FAX_SYM_LEN];
    UBYTE i;

#if 0
    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV34Tx->PCMinPtr[i]; }
    }
#endif

    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        qPcmIn[i] = (QWORD)(QQMULQD(pV34Tx->PCMinPtr[i], pV34Fax->qGain) >> 8);
    }

    qdAvg_Energy = DSPD_DFT_Detect(&(pV34Fax->Dft), qPcmIn, V34FAX_SYM_LEN);

    if ((qdAvg_Energy > 0)
        && (((pV34Fax->modem_mode == CALL_MODEM) && (pV34Fax->nTxState != 0))
            || (pV34Fax->modem_mode == ANS_MODEM)))
    {
        qdPPh_Energy = pV34Fax->qdMagSq[1] + pV34Fax->qdMagSq[2];

        qdSh_Energy = pV34Fax->qdMagSq[0] + pV34Fax->qdMagSq[3];

        if (pV34Fax->qdMagSq[4] < 1000000)
        {
            if ((qdSh_Energy + qdPPh_Energy) > 6000000)
            {
                if ((qdAvg_Energy > MPET_PPH_SH_MIN_LEVEL_REF)
                    && (qdSh_Energy > qdAvg_Energy)
                    && (qdSh_Energy > (qdPPh_Energy << 1)))
                {
                    pV34Fax->SHDetect_Count ++;

                    pV34Fax->PPh_ShDetect_Count = 0;

                    pV34Fax->PPhDetect_Count = 0;
                }
                else if ((qdAvg_Energy > MPET_PPH_SH_MIN_LEVEL_REF)
                         && (qdPPh_Energy > qdAvg_Energy)
                         && (qdSh_Energy <= (qdPPh_Energy << 1))
                         && (qdSh_Energy > qdPPh_Energy))
                {
                    pV34Fax->SHDetect_Count = 0;

                    pV34Fax->PPh_ShDetect_Count ++;

                    pV34Fax->PPhDetect_Count = 0;
                }
                else if ((qdAvg_Energy > MPET_PPH_SH_MIN_LEVEL_REF)
                         && (qdPPh_Energy > qdAvg_Energy))
                {
                    pV34Fax->PPhDetect_Count ++;

                    pV34Fax->PPh_ShDetect_Count = 0;

                    pV34Fax->SHDetect_Count = 0;
                }
                else
                {
                    pV34Fax->SHDetect_Count = 0;

                    pV34Fax->PPh_ShDetect_Count = 0;

                    pV34Fax->PPhDetect_Count = 0;
                }
            }
            else
            {
                pV34Fax->SHDetect_Count = 0;

                pV34Fax->PPh_ShDetect_Count = 0;

                pV34Fax->PPhDetect_Count = 0;
            }
        }

        if ((pV34Fax->SHDetect_Count >= 1) && (pV34Fax->PPhDetect_Only == 0))
        {
            pV34Fax->SHDetect_Count = 0;

            pV34Fax->ShDetected = 1;

            TRACE0("Sh Detected");
        }

        if ((pV34Fax->PPhDetect_Count >= 1) || (pV34Fax->PPh_ShDetect_Count >= 2))
        {
            pV34Fax->PPhDetected = 1;

            pV34Fax->PPhDetect_Count = 0;

            pV34Fax->PPh_ShDetect_Count = 0;

            pV34Fax->PPhDetect_Only = 0;

            TRACE0("PPh Detected 1");
        }
    }
}

void V34Fax_PPh_Detection(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);

    pV34Fax->PPhDetect_Only = 1;

    V34Fax_PPh_Sh_Detection(pV34);

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        if (pV34Fax->nTxState != 0)
        {
            if (pV34Fax->PPhDetected)
            {
                pV34Fax->PPh_DetCount++;

                pV34Fax->PPS_detect_flag = 1;
            }
            else
            {
                pV34Fax->PPh_DetCount = 0;
            }
        }
    }
    else/* ANS_MODEM */
    {
        if (pV34Fax->PPhDetected)
        {
            pV34Fax->PPh_DetCount++;

            pV34Fax->PPS_detect_flag = 1;
        }
        else
        {
            pV34Fax->PPh_DetCount = 0;
        }
    }
}


void V34Fax_PPh_Sh_AnsRx(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->PPhDetected)/* PPh detected in second train. */
    {
        pV34Fax->reRateFlag = RERATE_RESPOND;

        V34Fax_MPET_Fsm_Init(pV34Fax);  /* Change to initial-training mode */

        pV34Fax->nRxDelayCnt = 0;       /* Re-Train EQ using PPh */

        pV34Fax->nTxDelayCnt = 0;       /* send PPh */
    }

    if (pV34Fax->ShDetected)
    {
        if (pV34Fax->reRateFlag == RERATE_INIT && pV34Fax->nTxState == 0)
        {
            /* FSM already set to rerate mode */
            pV34Fax->nTxDelayCnt = 0;   /* send PPh */
        }
        else
        {
            pV34Fax->nRxDelayCnt = 0;   /* SBAR-Resync */
        }
    }
}


void V34Fax_PPh_Sh_CallRx(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->PPhDetected)
    {
        pV34Fax->reRateFlag = RERATE_RESPOND;

        V34Fax_MPET_Fsm_Init(pV34Fax);

        pV34Fax->nRxDelayCnt = 0;       /* Re-Train EQ using PPh */

        if (pV34Fax->nTxState == 3)
        {
            pV34Fax->nTxDelayCnt = pV34Fax->nTxDelayCntTab[pV34Fax->nTxState];
        }
    }

    if (pV34Fax->ShDetected)
    {
        pV34Fax->nRxDelayCnt = 0;    /* SBAR-Resync */

        if (pV34Fax->pfDataInVec == V34Fax_ALT_Tx)
        {
            pV34Fax->nTxDelayCnt = 130;
        }
    }
}


void V34Fax_ShB_Rx_ReSync(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->nRxDelayCnt == 22)
    {
        V34Fax_SyncTiming(pV34Fax);

        pV34Fax->pfEqVec = V34Fax_EQ;
    }
    else if (pV34Fax->nRxDelayCnt == 18)
    {
        pV34Fax->cqRotatePrevIQ.r = pV34Fax->cqRotateIQ.r;
        pV34Fax->cqRotatePrevIQ.i = pV34Fax->cqRotateIQ.i;
    }
    else if (pV34Fax->nRxDelayCnt == 17)
    {
        V34Fax_SyncCarrier(pV34Fax);
    }
    else if ((pV34Fax->nRxDelayCnt < 17) && (pV34Fax->nRxDelayCnt > 8))
    {
        /* Look for Sh bar */
        if (pV34Fax->cqRotateIQ.i < 0)
        {
            pV34Fax->nRxDelayCnt = 7;

            if ((pV34Fax->modem_mode == ANS_MODEM) && (pV34Fax->reRateFlag != RERATE_INIT))
            {
                pV34Fax->nTxDelayCnt = 0;    /* send Sh */
            }
        }
        else
        {
            pV34Fax->nRxDelayCnt = 16;
        }
    }
}


void V34Fax_PPh_Rx(V34FaxStruct *pV34Fax)//George NOV21
{
    if (pV34Fax->PPh_DetCount >= 4)
    {
        pV34Fax->PPh_DetCount = 0;
        pV34Fax->nRxDelayCnt = 0;       /* EQ train by PPh */

        if (pV34Fax->modem_mode == ANS_MODEM && pV34Fax->MPET_Retrning == 0)
        {
            pV34Fax->nTxDelayCnt = 0;    /* send PPh */
        }

        pV34Fax->PPhDetected = 1;

        pV34Fax->PPhDetect_Only = 0;

        TRACE0("PPh Detected");
    }
}


void V34Fax_PPh_Train(V34FaxStruct *pV34Fax)
{
#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV34Fax->cqRotateIQ.r; }
    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV34Fax->cqRotateIQ.i; }
#endif
}


void V34Fax_ALT_Rx(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->ubDescramOutbits == 0x2)
    {
        pV34Fax->ALT_DetCount++;
    }
    else
    {
        pV34Fax->ALT_DetCount = 0;
    }

    /* Here may not detect 0x02, if equalizer is not good enough in short time */
    if (((pV34Fax->modem_mode == CALL_MODEM) && (pV34Fax->MPET_Retrning == 0) && (pV34Fax->ALT_DetCount >= 2))
        || (pV34Fax->ALT_DetCount >= 5))/* The original 2 is too small */
    {
        /* Remaining ALT TX sent length defined here */
        pV34Fax->ALT_DetCount = 0;

        if (pV34Fax->modem_mode == CALL_MODEM)
        {
            if (pV34Fax->reRateFlag == RERATE_RESPOND)
            {
                pV34Fax->nTxDelayCnt = 0;  /* Stop sending ALT, send PPh */
            }
            else if (pV34Fax->Enter_CC_Order == 0)
            {
                /* initial call modem, figure 25 */
                pV34Fax->nTxDelayCnt += 83 - pV34Fax->nRxDelayCntTab[pV34Fax->nRxState];
            }
            else if (pV34Fax->PPhDetected)
            {
                /* Figure 26, 115 symbol */
                pV34Fax->nTxDelayCnt += 115 - pV34Fax->nTxDelayCntTab[pV34Fax->nTxState];

                if (pV34Fax->nTxDelayCnt < 0)
                {
                    pV34Fax->nTxDelayCnt = 0;    /* probably won't use it */
                }
            }

            /* In Sh detected case, set in V34Fax_PPh_Sh_CallRx */
        }
        else if (pV34Fax->modem_mode == ANS_MODEM)
        {
            if (pV34Fax->pfDataInVec == V34Fax_ALT_Tx)
            {
                pV34Fax->nTxDelayCnt += 115 - pV34Fax->nTxDelayCntTab[pV34Fax->nTxState];

                if (pV34Fax->nTxDelayCnt < 0)
                {
                    pV34Fax->nTxDelayCnt = 0;    /* probably won't use it */
                }
            }
            else if (pV34Fax->Enter_CC_Order >= 1)
            {
                //pV34Fax->nTxDelayCnt = 18; /* still send Sh for 18T */
            }
        }

        pV34Fax->nRxDelayCnt = 0;    /* detect MPh */

        if (pV34Fax->MPET_Retrn_Type == RETRAIN_INIT && pV34Fax->MPET_Retrning == 1)
        {
            pV34Fax->nTxDelayCnt = 0;
        }

        pV34Fax->AC_Detect_Start_Flag = 1;

        TRACE0("ALT detected");
    }
}


SBYTE V34Fax_MPh_Detect(V34FaxStruct *pV34Fax)
{
    SBYTE iResult;
    UBYTE bit0, bit1;

    bit0 = pV34Fax->ubDescramOutbits & 0x1;
    bit1 = (pV34Fax->ubDescramOutbits >> 1) & 0x1;

    iResult  = V34Fax_Detect_Mph(bit0, &pV34Fax->MphRx);
    iResult |= V34Fax_Detect_Mph(bit1, &pV34Fax->MphRx);

    return iResult;
}


void V34Fax_MPh_Rx(V34FaxStruct *pV34Fax)
{
    MphStruc *pMphRx = &pV34Fax->MphRx;
    MphStruc *pMphTx = &pV34Fax->MphTx;

    if (V34Fax_MPh_Detect(pV34Fax) > 0)
    {
        ACEStruct *pAce = (ACEStruct *)pV34Fax->pTable[ACE_STRUC_IDX];
        DceToT30Interface *pDceToT30 = pAce->pDceToT30;
        UBYTE i;
        UBYTE isValidBRFound = FALSE;

        pV34Fax->ubMphDetect++;   /* set flag so that TX state will finish current MPh */
        pV34Fax->nRxDelayCnt = 0; /* E Rx */

        pV34Fax->PPhDetected = 0;

        TRACE1("MPh %d detected", pMphRx->type);

        if (pMphRx->max_data_rate > pMphTx->max_data_rate)
        {
            pMphRx->max_data_rate = pMphTx->max_data_rate;
        }

        for (i = V34_BIT_33600; i >= V34_BIT_2400; i--)
        {
            if (V34_tINIT_PARA_DATA[pV34Fax->symbol_rate][i-1].b != 0 && ((pMphRx->data_rate_cap_mask & ((UWORD)1 << (i - 1))) != 0))
            {
                if (pMphRx->max_data_rate > i)
                {
                    if (pMphRx->pre_max_data_rate <= i)
                    {
                        if (pMphRx->pre_max_data_rate > V34_BIT_2400)
                        {
                            i = pMphRx->pre_max_data_rate - 1;

                            if (V34_tINIT_PARA_DATA[pV34Fax->symbol_rate][i-1].b != 0 && ((pMphRx->data_rate_cap_mask & ((UWORD)1 << (i - 1))) != 0))
                            {
                                // the downgraded bit rate is valid
                                isValidBRFound = TRUE;
                                pMphRx->max_data_rate = i;

                                pMphRx->pre_max_data_rate = pMphRx->max_data_rate;

                                break;
                            }
                        }
                        else
                        {
                            // it's invalid to further downgrade
                            break;
                        }
                    }
                    else
                    {
                        // the bit rate being exammed is valid
                        isValidBRFound = TRUE;
                        pMphRx->max_data_rate = i;

                        pMphRx->pre_max_data_rate = pMphRx->max_data_rate;

                        break;
                    }
                }
                else
                {
                    // the bit rate received is valid
                    isValidBRFound = TRUE;

                    break;
                }
            }
        }

        if (!isValidBRFound)
        {
            // cannot find a valid bit rate
            pV34Fax->Disconnect_flag = TRUE;
            pDceToT30->isDleEot = TRUE; TRACE0("ACE: <DLE><EOT>");
            Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
            return;
        }

        pMphTx->max_data_rate = pMphRx->max_data_rate;

        pDceToT30->priChnlBitRate = pMphTx->max_data_rate;

        TRACE1("Rx Max Data Rate   %u", (UWORD)pMphRx->max_data_rate * 2400);
        TRACE1("Rx CC rate         %d",  pMphRx->control_channel_rate);
        TRACE1("Rx Trellis Encoder %d",  pMphRx->TrellisEncodSelect);
        TRACE1("Rx Nliner Encoder  %d",  pMphRx->NonlinearEncodParam);
        TRACE1("Rx Shaping         %d",  pMphRx->const_shap_select_bit);
        TRACE1("Rx Rate Mask     %.4X",  pMphRx->data_rate_cap_mask);
        TRACE1("Rx Asymmetric Rate %d",  pMphRx->asymmetric_control_rate);

        if (pMphRx->control_channel_rate < pMphTx->control_channel_rate)
        {
            pMphTx->control_channel_rate = pMphRx->control_channel_rate;
        }
        else
        {
            pMphRx->control_channel_rate = pMphTx->control_channel_rate;
        }

        if (pDceToT30->ctrlChnlBitRate == 0)
        {
            DpcsStruct *pDpcs = (DpcsStruct *)pV34Fax->pTable[DPCS_STRUC_IDX];

            // first time
            pDceToT30->infoTxtRsp = INFO_TXT_F34; TRACE2("V34: +F34=%d,%d", pMphTx->max_data_rate, pMphRx->control_channel_rate + 1);
            Ace_SetClass1ResultCode(pAce, RESULT_CODE_CONNECT);

            if (RdReg(pDpcs->MCR0, ANSWER))
            {
                Ace_ChangeState(pAce, ACE_FTH);
            }
            else
            {
                Ace_ChangeState(pAce, ACE_FRH);
                pAce->isOptFrm = FALSE;
            }
        }

        pDceToT30->ctrlChnlBitRate = pMphRx->control_channel_rate + 1;

        switch (pMphRx->max_data_rate)
        {
            case V34_BIT_33600:
                pV34Fax->Timeout_thres = 4000;
                break;
            case V34_BIT_31200:
                pV34Fax->Timeout_thres = 5000;
                break;
            case V34_BIT_28800:
                pV34Fax->Timeout_thres = 6000;
                break;
            case V34_BIT_26400:
                pV34Fax->Timeout_thres = 6000;
                break;
            case V34_BIT_24000:
                pV34Fax->Timeout_thres = 7000;
                break;
            case V34_BIT_21600:
                pV34Fax->Timeout_thres = 7000;
                break;
            case V34_BIT_19200:
                pV34Fax->Timeout_thres = 8000;
                break;
            case V34_BIT_16800:
                pV34Fax->Timeout_thres = 10000;
                break;
            case V34_BIT_14400:
                pV34Fax->Timeout_thres = 12000;
                break;
            case V34_BIT_12000:
                pV34Fax->Timeout_thres = 14000;
                break;
            case V34_BIT_9600:
                pV34Fax->Timeout_thres = 16000;
                break;
            case V34_BIT_7200:
                pV34Fax->Timeout_thres = 18000;
                break;
            case V34_BIT_4800:
                pV34Fax->Timeout_thres = 24000;
                break;
            case V34_BIT_2400:
                pV34Fax->Timeout_thres = 32000;
                break;
        }

        if (pMphRx->type == 1)
        {
            TRACE2("Precoder Coef0  %d %d", pMphRx->PrecodeCoeff_h[0].r, pMphRx->PrecodeCoeff_h[0].i);
            TRACE2("Precoder Coef1  %d %d", pMphRx->PrecodeCoeff_h[1].r, pMphRx->PrecodeCoeff_h[1].i);
            TRACE2("Precoder Coef2  %d %d", pMphRx->PrecodeCoeff_h[2].r, pMphRx->PrecodeCoeff_h[2].i);
        }

        if (pV34Fax->pfDataInVec == V34Fax_ALT_Tx) /* prepare for TX MPH */
        {
            pV34Fax->nTxDelayCnt = 115 - pV34Fax->nTxDelayCntTab[pV34Fax->nTxState] + pV34Fax->nTxDelayCnt;

            if (pV34Fax->nTxDelayCnt < 0)
            {
                pV34Fax->nTxDelayCnt = 5;    /* still send ALT for 5T */
            }
        }
    }
}


void V34Fax_E_Rx(V34FaxStruct *pV34Fax)
{
    ACEStruct *pAce = (ACEStruct *)pV34Fax->pTable[ACE_STRUC_IDX];
    MphStruc *pMphRx = &pV34Fax->MphRx;
    UBYTE flag;

#if RECEIVED_E
    if (pV34Fax->Enter_CC_Order == RECEIVED_PAGE)
    {
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = (pV34Fax->ubDescramOutbits >> 1) & 1; }
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = pV34Fax->ubDescramOutbits & 1; }
    }
#endif

    pV34Fax->udEpattern <<= 2;
    pV34Fax->udEpattern |= (pV34Fax->ubDescramOutbits & 0x3);

    flag = 0;

    if (((pV34Fax->modem_mode == CALL_MODEM) && ((pV34Fax->udEpattern & 0xFFFFFU) == 0xFFFFFU))/* For some case, may only get 18bits. Old version is 0x3FFFFU, Zhiyong hope change to 0xFFFFFU??? */
        || ((pV34Fax->modem_mode == ANS_MODEM) && ((pV34Fax->udEpattern & 0xFFFFFU) == 0xFFFFFU)))
    {
        flag = 1;       /* E detected */

        TRACE0("E detected");

        if (pV34Fax->modem_mode == CALL_MODEM)
        {
            pAce->V34Fax_State = V34FAX_TX_FLAG;
        }
    }

    if ((pV34Fax->udEpattern & 0xFFFFFU) == 0xBDBDBU)
    {
        flag = 1;       /* Flag detected */

        TRACE0("Flag detected in E");
    }

    if ((pV34Fax->udEpattern & 0xFFFFFU) == 0xFCFCFU)
    {
        flag = 1;       /* Flag detected */

        TRACE0("Flag(1) detected in E");
    }

    if (flag)
    {
        pV34Fax->nRxDelayCnt = 0;
        pV34Fax->rx_byte     = 0;
        pV34Fax->E_detected  = 1;

        //if ((pV34Fax->modem_mode == CALL_MODEM) && (pV34Fax->Enter_CC_Order >= 1)) //fixing issue 112
        //pV34Fax->nTxDelayCnt = 0; /* send E */ //fixing issue 112
    }

    if (pV34Fax->Enter_CC_Order >= 1)
    {
        if (V34Fax_MPh_Detect(pV34Fax) > 0)
        {
            // fixing issue 196, need to validate the MPH received
            MphStruc *pMphTx = &pV34Fax->MphTx;

            if (pMphRx->max_data_rate > pMphTx->max_data_rate)
            {
                pMphRx->max_data_rate = pMphTx->max_data_rate;
            }
            else
            {
                pMphTx->max_data_rate = pMphRx->max_data_rate;
            }

            pV34Fax->ubMphDetect++;

            pV34Fax->PPhDetected = 0;

            TRACE1("MPh %d detected in E", pMphRx->type);
        }
    }
}


void V34Fax_T30_Rx(V34FaxStruct *pV34Fax)
{
    UBYTE *pIOWrBuf = (UBYTE *)pV34Fax->pTable[DSPIOWRBUFDATA_IDX];
    //ACEStruct *pAce = (ACEStruct *)pV34Fax->pTable[ACE_STRUC_IDX];
    UBYTE rx_bits;

    /* forever */
    pV34Fax->nRxDelayCnt = 10;

    /* flip descrambler output bits */
    if (pV34Fax->ubRxBitsPerSym == V34FAX_SYM_BITS_2400)
    {
        rx_bits = V34fax_4BitReversal[pV34Fax->ubDescramOutbits & 0x0f];
    }
    else
    {
        rx_bits = V34fax_2BitReversal[pV34Fax->ubDescramOutbits & 0x03];
    }

#if RECEIVED_DATA
    if (pV34Fax->Enter_CC_Order == RECEIVED_PAGE)
    {
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = rx_bits & 1; }
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = (rx_bits >> 1) & 1; }
    }
#endif

    if (rx_bits == 0x03)
    {
        pV34Fax->ONEs_DetCount++;
    }
    else
    {
        pV34Fax->ONEs_DetCount = 0;
    }

#if 0

    if ((rx_bits == 0x02) || (rx_bits == 0x01))
    {
        pV34Fax->MPET_Retrn_AC_count ++;
    }
    else
    {
        pV34Fax->MPET_Retrn_AC_count = 0;
    }

    if (pV34Fax->MPET_Retrn_AC_count > 60)
    {
        pV34Fax->MPET_Retrn_AC_count = 0;

        pV34Fax->MPET_Retrn_Type = RETRAIN_RESPOND;
        pV34Fax->MPET_Retrning = 0;
    }

#endif

    {
        pIOWrBuf[0] = rx_bits;
        IoWrProcess(pV34Fax->pTable, pV34Fax->ubRxBitsPerSym);
    }

    V34Fax_Silence_Rx(pV34Fax);
}


void V34Fax_ONEs_Rx(V34FaxStruct *pV34Fax)
{
    UBYTE rx_bits;

    /* forever */
    pV34Fax->nRxDelayCnt = 100;

    /* flip descrambler output bits */
    if (pV34Fax->ubRxBitsPerSym == V34FAX_SYM_BITS_2400)
    {
        rx_bits = V34fax_4BitReversal[pV34Fax->ubDescramOutbits & 0x0f];
    }
    else
    {
        rx_bits = V34fax_2BitReversal[pV34Fax->ubDescramOutbits & 0x03];
    }

#if RECEIVED_ONES
    if (pV34Fax->Enter_CC_Order == RECEIVED_PAGE)
    {
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = rx_bits & 1; }
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = (rx_bits >> 1) & 1; }
    }
#endif

    if (rx_bits == 0x03)
    {
        pV34Fax->Ones_Rx_count0++;
    }
    else
    {
        pV34Fax->Ones_Rx_count0 = 0;
    }

#if 0

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        if (rx_bits == 0x00) /* wrong quit, no ONEs transmitted, April 22,2005 */
        {
            pV34Fax->nDetCounter++;
        }
    }

#endif

    //V34Fax_Silence_Rx(pV34Fax);
}


void V34Fax_Silence_Rx(V34FaxStruct *pV34Fax)
{
    ACEStruct *pAce = (ACEStruct *)pV34Fax->pTable[ACE_STRUC_IDX];

    /* forever */
    //pV34Fax->nRxDelayCnt = 100;

    if ((pV34Fax->cqRotateIQ.r < 2000) && (pV34Fax->cqRotateIQ.r > -2000))
    {
        if ((pV34Fax->cqRotateIQ.i < 2000) && (pV34Fax->cqRotateIQ.i > -2000))
        {
            pV34Fax->silence_cnt++;
        }
        else
        {
            pV34Fax->silence_cnt = 0;
        }
    }
    else
    {
        pV34Fax->silence_cnt = 0;
    }

    if ((pV34Fax->modem_mode == ANS_MODEM) && (pV34Fax->silence_cnt > 10))
    {
        if (pAce->isToTurnOffCarrier)
        {
            pV34Fax->silence_cnt = 0;
            pAce->isToTurnOffCarrier = FALSE;
            pV34Fax->Disconnect_flag = TRUE;
        }
        else if (!pV34Fax->V34Fax_PCR_Start_Flag)
        {
            CircBuffer *pAscCBRd = (CircBuffer *)pV34Fax->pTable[ASCCBRDDATA_IDX];

            pAscCBRd->CurrLen = 0;
            pAscCBRd->End = pAscCBRd->Start;
            pV34Fax->V34Fax_PCR_Start_Flag = TRUE;
            pAce->pDceToT30->selectedV34Chnl = V34_PRIMARY_CHNL; TRACE0("V34: <DLE><PRI>");
        }
    }
}

#endif
