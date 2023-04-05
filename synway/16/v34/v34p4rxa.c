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

/***********************************/
/* Phase4 Rx vectors, Answer modem */
/***********************************/

#include "v34ext.h"

#if 0 /// LLL temp
#if USE_ANS
void V34_Phase4_Det_J_Bar(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc   *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    V34StateStruc *pV34State = &pRx->V34State;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    Phase4_Info *p4 = &(pV34->p4);
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    UBYTE dbits, diffbits;
    CQWORD cqSymbol, cqErrsym, cqTrain_sym;
    CQWORD cqDsym[3], cqTimingOut[3];
    UBYTE Det_J_Bar_result_code;
    UBYTE bit_stream[2];

    V34_Demodulate(pRx, cqDsym);

    V34_Timing_Rec(pTCR, cqDsym, cqTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, cqTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        cqSymbol = V34_Rotator(&cqSymbol, pJT->qS, pJT->qC);
    }

#endif

#if DRAWEPG
    /************* For Vxd EPG debug ********/
    pRx->pEpg[0].r = (cqSymbol.r);
    pRx->pEpg[0].i = (cqSymbol.i);
    /**************** End of EPG ************/
#endif

    dbits = V34_Symbol_To_Bits(cqSymbol, &cqTrain_sym, V34_4_POINT_TRN); /* always 4 points in Phase3 */

    CQSUB(cqTrain_sym, cqSymbol, cqErrsym);

    if (pAGC->freeze_EQ == 0)
    {
        V34_2T3_Equalizer_Update(cqErrsym, V34_qBETA_TRN_FINE, pTCR);
    }

    V34_TimingLoop(pTCR, &cqSymbol, &cqTrain_sym);

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        pJT->counter++;
        V34_jt_loop(pJT, &cqSymbol, &cqTrain_sym, 0);
    }

#endif

    V34_CarrierLoop(pTCR, &cqSymbol, &cqTrain_sym);

    V34_DIFF_DECODE(DeCode, dbits, diffbits);

    bit_stream[0] = diffbits & 0x1;
    bit_stream[1] = (diffbits >> 1) & 0x1;

    (*pRx->pfDescram_bit)(bit_stream, &(pRx->dsc), pRx->pDscram_buf, 2);

    Det_J_Bar_result_code = V34_Detect_J_Sequence(pRx->pDscram_buf[0], pV34);

    if (Det_J_Bar_result_code == V34_PI_POINT_TRN)
    {
        p4->rx_vec_idx++; /* goto detect TRN */
        p4->detect_symbol_counter = 0;
        pV34Rx->train_sc = 0;
        pV34Rx->train_scram_idx = 16;

#if PHASEJITTER
        V34_jt_decision(pJT);
#endif
    }

    /* TRACE1("The value must be const for different RTD finally = %d", pV34->Time_cnt - p4->Time_cnt_Jpi_start - p4->pqTime_out_value[0]); */

    /* test time out, V.34 Page48 11.4.2.2.1, simply start retraining */
    if ((pV34Rx->modem_mode == ANS_MODEM) && (p4->Time_cnt_Jpi_start) && ((pV34->Time_cnt - p4->Time_cnt_Jpi_start) > p4->pqTime_out_value[0]))
    {
        pRx->Retrain_flag         = RETRAIN_INIT;
        pV34State->retrain_reason = V34_PH4_DETECT_JBAR_TIMEOUT;
    }
}

#endif
#endif