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

#include "v34ext.h"

/* Used for calculation on Symbol errors */
void V34_ARS_Calc(ReceiveStruc *pRx, CQWORD *cqErrsym)
{
    QDWORD qdErregy;

    qdErregy  = QQMULQD(cqErrsym->r, cqErrsym->r) >> 1;
    qdErregy += QQMULQD(cqErrsym->i, cqErrsym->i) >> 1;

    pRx->qdARS_erregy += (qdErregy >> 6);
}


/******************************************************************/
/* modified by Jack Liu, Large the ARS array from 3 to 6 in order */
/* to support 2743, 2800, 3429 symbol rate                        */
/******************************************************************/
SBYTE V34_ARS_est_Sub(V34RxStruct *pV34Rx)
{
    ReceiveStruc  *pRx       = &pV34Rx->RecStruc;
    V34StateStruc *pV34State = &pRx->V34State;
    QWORD *pqV34_EQ_ARS_Thres;
    QWORD qNLDFactor;
    UBYTE SymIdx;
    SBYTE ARS_idx;
#if V34_SNR
    QDWORD SNR;
#endif

    if (pV34Rx->V34bis && pV34Rx->V34bisL)
    {
        /* Setup default bit rate for both Tx and Rx on V34bis */
        pV34Rx->tx_bit_rate = V34bis_tBit_Rate[pV34Rx->tx_symbol_rate];
    }
    else
    {
        /* Setup default bit rate for both Tx and Rx on V34 */
        pV34Rx->tx_bit_rate = V34_tBit_Rate[pV34Rx->tx_symbol_rate];
    }

    pV34Rx->rx_bit_rate = pV34Rx->tx_bit_rate;

    /* Average out ARS symbol energy error */
    pRx->qdARS_erregy >>= 9;/* divide by 1024 symbols. Before is 10, shift 5 at V34_ARS_Calc; now is 9, shift 6 at V34_ARS_Calc */

    TRACE1("ARS1: %" FORMAT_DWORD_PREFIX "d", ((pRx->qdARS_erregy + 0x10) >> 5));

#if 1 /* For 1, NLD compensation enabled */
    qNLDFactor = pV34Rx->qNLDFactor;
#else
    qNLDFactor = 0;
#endif

    if ((pV34Rx->rx_symbol_rate == V34_SYM_3429) && (qNLDFactor > 768))
    {
        qNLDFactor  = QQMULQ15(qNLDFactor, q08);
        qNLDFactor -= 102; /* Q4.11 */
        pRx->qdARS_erregy += QDQMULQD0(pRx->qdARS_erregy, qNLDFactor) >> 11;
    }
    else if (qNLDFactor > 1024)
    {
        qNLDFactor  = QQMULQ15(qNLDFactor, q04);
        qNLDFactor -= 205; /* Q4.11 */
        pRx->qdARS_erregy += QDQMULQD0(pRx->qdARS_erregy, qNLDFactor) >> 11;
    }

    TRACE1("qNLDFactor: %d", qNLDFactor);

#if V34_SNR
    /* TRACE1("ARS %d", pRx->qdARS_erregy); */

    SNR = DSPD_Log10(pRx->qdARS_erregy);/* Log value of the EQ noise */

    SNR = 20 - QDQMULQDR(SNR, 10);

    if (pV34Rx->tx_symbol_rate == V34_SYM_3429)
    {
        SNR ++;
    }

    TRACE1("SNR = %d", SNR);
#endif

    pRx->qdARS_erregy = (pRx->qdARS_erregy + 0x10) >> 5;  /* Change to 3.13 format */

    TRACE1("ARS %" FORMAT_DWORD_PREFIX "d", pRx->qdARS_erregy);

    /* Setup the upper limit of the current bit rate according to the Host */
    if (pV34Rx->Host_maxbitrate < pV34Rx->tx_bit_rate)
    {
        pV34Rx->tx_bit_rate = pV34Rx->Host_maxbitrate;
        pV34Rx->rx_bit_rate = pV34Rx->Host_maxbitrate;
    }

    SymIdx = pV34Rx->tx_symbol_rate;

    /* Do ARS Estimation */
    pqV34_EQ_ARS_Thres = (QWORD *)V34_tEQ_ARS_THRESHOLD[SymIdx];

    if (pRx->qdARS_erregy > pqV34_EQ_ARS_Thres[0])
    {
        pRx->Retrain_flag = RETRAIN_INIT;
        pV34State->retrain_reason = V34_EQ_TRAINING_BAD;
        ARS_idx = 0;
    }
    else
    {
        for (ARS_idx = pV34Rx->tx_bit_rate - V34_BIT_2400; ARS_idx >= 0; ARS_idx --)
        {
            if (pRx->qdARS_erregy <= pqV34_EQ_ARS_Thres[ARS_idx])
            {
                break;
            }
        }

        ARS_idx ++;
    }

    return (ARS_idx);
}


void  V34_ARS_est(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34StateStruc *pV34State = &pRx->V34State;
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
#if SUPPORT_V34FAX
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    MphStruc *pMphTx = &pV34Fax->MphTx;
#endif
    SBYTE ARS_idx;
    UBYTE  i;
    UWORD uMinmask;

    ARS_idx = V34_ARS_est_Sub(pV34Rx);

#if SUPPORT_V34FAX

    if (pRx->V34fax_Enable == 1)
    {
        if (pV34Rx->rx_symbol_rate == V34_SYM_2400 && ARS_idx < V34_BIT_2400)
        {
            TRACE0("For V34fax, Force Minimum Rate");

            pRx->Retrain_flag = RETRAIN_INIT;

            ARS_idx = V34_BIT_2400;
        }
        else if (pV34Rx->rx_symbol_rate != V34_SYM_2400 && ARS_idx < V34_BIT_4800)
        {
            TRACE0("For V34fax, Force Minimum Rate");

            pRx->Retrain_flag = RETRAIN_INIT;

            ARS_idx = V34_BIT_4800;
        }
    }

#endif

    /* ----- set the data signalling rate correspondingly ------ */
    if ((ARS_idx >= V34_BIT_2400) && (pV34Rx->rx_bit_rate > ARS_idx))
    {
        pV34Rx->rx_bit_rate = ARS_idx;
    }

    if (pMpTx->asymmetric_data_sig_rate == 0)
    {
        pV34Rx->tx_bit_rate = pV34Rx->rx_bit_rate;
    }

    /* Generate Bit rate mask up to supported MAX bitrate */
	pMpTx->data_rate_cap_mask = (1 << pV34Rx->tx_bit_rate) - 2;

    if (pV34Rx->tx_symbol_rate == V34_SYM_2400) /* only 2400 symbol rate can support 2400bps */
    {
        pMpTx->data_rate_cap_mask++;
    }

    /* Finalized bitrate mask up to MIN bitrate supported */
    if (pV34Rx->rx_bit_rate == V34_BIT_2400 || pV34Rx->tx_bit_rate == V34_BIT_2400)
    {
        uMinmask = (1 << pV34Rx->Host_minbitrate) - 1;
    }
    else
    {
        uMinmask = (1 << pV34Rx->Host_minbitrate) - 2;
    }

    uMinmask = ~(uMinmask >> 1);

    pMpTx->data_rate_cap_mask &= uMinmask;/* minimum speed limit */

    /* Setup the lower limit of current selected bit rate according to Host */
    if (pV34Rx->Host_minbitrate > pV34Rx->rx_bit_rate)
    {
        pV34State->hang_up = 1;
        pV34State->hangup_reason = V34_BITRATE_NOT_IN_RANGE;
    }

    if (pV34Rx->modem_mode == CALL_MODEM)
    {
        pMpTx->max_call_to_ans_rate = pV34Rx->tx_bit_rate;
        pMpTx->max_ans_to_call_rate = pV34Rx->rx_bit_rate;
    }
    else
    {
        pMpTx->max_call_to_ans_rate = pV34Rx->rx_bit_rate;
        pMpTx->max_ans_to_call_rate = pV34Rx->tx_bit_rate;
    }

    for (i = 0; i < V34_PRECODE_COEF_LEN; i++)
    {
#if RX_PRECODE_ENABLE
        pMpTx->PrecodeCoeff_h[i] = pV34->NoiseWhiten.cNWCoef[2 - i];

#if SUPPORT_V34FAX
        pMphTx->PrecodeCoeff_h[i] = pV34->NoiseWhiten.cNWCoef[2 - i];
#endif

        TRACE3("TX-PRECODE (%d) %6d  %6d", i, pMpTx->PrecodeCoeff_h[i].r, pMpTx->PrecodeCoeff_h[i].i);
#else
        pMpTx->PrecodeCoeff_h[i].r = 0;
        pMpTx->PrecodeCoeff_h[i].i = 0;

#if SUPPORT_V34FAX
        pMphTx->PrecodeCoeff_h[i].r = 0;
        pMphTx->PrecodeCoeff_h[i].i = 0;
#endif

#endif
    }

#if RX_PRECODE_ENABLE
    pV34->NoiseWhiten.ubBetaShift = 0;
#endif

    pMpTx->acknowledge_bit = 0;

    pMpTx->NonlinearEncodParam = NON_LINEAR;

    V34_Create_Mp(p4->pMp_buf, pMpTx);

    pMpTx->acknowledge_bit = 1;

    V34_Create_Mp(p4->pMp_buf_pi, pMpTx);
}
