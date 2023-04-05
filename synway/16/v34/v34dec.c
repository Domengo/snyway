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

void  V34_Decoder_Init(DeCodeStruc *pDeCode)
{
    VA_INFO *dec = &(pDeCode->Vdec);
    VA_NUMBER *pNumber = &(dec->number);
    VA_DATA_ADDRESS *pAddr = &(dec->Addr);

    dec->win_size    = V34_VA_WINTHRESHOLD;
    dec->rx_COV_type = 0; /* 16 state */

    pNumber->branch = (UBYTE)4 << dec->rx_COV_type;
    pNumber->d4set  = pNumber->branch << 1;
    pNumber->state  = pNumber->d4set << 1;

    switch (dec->rx_COV_type)
    {
        case 0:
            pAddr->pTrellis_ptr = (UBYTE *)(&V34_tTrellis16[0]);
            pAddr->pD4set_ptr   = (UBYTE **)(V34_tS16D4set_ptr);
            break;

#if ALL_VA
        case 1:
            pAddr->pTrellis_ptr = (UBYTE *)(&V34_tTrellis32[0]);/* No table */
            pAddr->pD4set_ptr   = (UBYTE **)(V34_tS32D4set_ptr);/* No table */
            break;

        case 2:
            pAddr->pTrellis_ptr = (UBYTE *)(&V34_tTrellis64[0]);/* No table */
            pAddr->pD4set_ptr   = (UBYTE **)(V34_tS64D4set_ptr);/* No table */
            break;
#endif
    }

    dec->hist_idx = 0;
    dec->count    = dec->win_size;

    dec->VA_sym_inidx  = 0;
    dec->VA_sym_outidx = 0;
    dec->VA_sym_count  = 0;
    /* V34_Init_Sync_Detector(dec); */

    pDeCode->qdScaleAdjust     = 0;
    pDeCode->nScaleAdjustCount = 120;
    pDeCode->qdScaleAdjustSum  = 0;
    pDeCode->qdScaleAdjustSum1 = 0;
}

void V34_Precode_Sub(ReceiveStruc *pRx, CQWORD *pcYout)
{
    DeCodeStruc *DeCode = &(pRx->DeCode);
#if RX_PRECODE_ENABLE
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    PrecodeStruct *precode_ptr = &(pRx->rx_precode);
#endif
    CQWORD cX, cU;
    UBYTE  i;

    for (i = 0; i < 2; i++)
    {
        cU.r = pcYout[i].r >> 7;
        cU.i = pcYout[i].i >> 7;

#if RX_PRECODE_ENABLE
        /* If V34_Precoder is used, call V34_Precoder again to get exact result */
        /* Precoder(&cX, &p4); ..... */
        cU.r -= precode_ptr->cC.r;
        cU.i -= precode_ptr->cC.i;

        cX.r = pcYout[i].r - precode_ptr->cP.r;
        cX.i = pcYout[i].i - precode_ptr->cP.i;

        V34_Precoder(precode_ptr, &cX, RxMod->rx_b);
#endif

        pRx->pcRx_sym[pRx->rx_sym_inidx ++] = cU;

        pRx->rx_sym_inidx &= V34_SYM_BUF_MASK;
    }

    pRx->rx_sym_count += 2;
}

#if 0 /// LLL temp
void V34_Decoder(ReceiveStruc *pRx, CQWORD *pcSym)
{
    V34StateStruc *pV34State = &pRx->V34State;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    DeCodeStruc      *DeCode = &(pRx->DeCode);
    VA_INFO *dec = &(DeCode->Vdec);
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    CQWORD cX, cY;
    CQWORD pcYout[2];
    CQWORD cSlice;
    CQWORD cqErrsym, cqD_sym, cqSymbol, cqUs;
#if RX_PRECODE_ENABLE
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    PrecodeStruct *precode_ptr = &(pRx->rx_precode);
#endif
    QDWORD qdTemp;
#if DRAWEPG
    CQWORD cqEYE;
#endif

    cqSymbol.r = pcSym->r;
    cqSymbol.i = pcSym->i;
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        cqSymbol = V34_Rotator(&cqSymbol, pJT->qS, pJT->qC);
    }

#endif

    cqSymbol.r += QQMULQ15(cqSymbol.r, QD16Q(DeCode->qdScaleAdjust));
    cqSymbol.i += QQMULQ15(cqSymbol.i, QD16Q(DeCode->qdScaleAdjust));

#if 0
    if (DumpTone1_Idx < 1000000) { DumpTone1[DumpTone1_Idx++] = cqSymbol.r; }
    if (DumpTone2_Idx < 1000000) { DumpTone2[DumpTone2_Idx++] = cqSymbol.i; }
    if (DumpTone3_Idx < 1000000) { DumpTone3[DumpTone3_Idx++] = pTCR->nTimingIdx; }      /* Timer */
    if (DumpTone4_Idx < 1000000) { DumpTone4[DumpTone4_Idx++] = pTCR->udError_phase_out; }/* Carrier */
    if (DumpTone5_Idx < 1000000) { DumpTone5[DumpTone5_Idx++] = DeCode->qdScaleAdjust; }
#endif

    V34_4Point_Slicer(&(pRx->Renego), &cqSymbol);

    cqUs = cqSymbol;

    if (pRx->rx_nlinear)
    {
        V34_Non_Linear_Decoder(pRx->qRx_nl_scale, &cqSymbol);
    }

#if 0
    if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = cqSymbol.r; }
    if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = cqSymbol.i; }
#endif

    /* Scale down for different data rate and change to 9.7 format */
    cX.r = QDQMULQDR16((pRx->qdRx_scale >> 1), cqSymbol.r);
    cX.i = QDQMULQDR16((pRx->qdRx_scale >> 1), cqSymbol.i);

    cY = cX;

#if RX_PRECODE_ENABLE
    /* All precoder is in 9.7 arithmetic */
    cY.r += pRx->VA_precode.cP.r;
    cY.i += pRx->VA_precode.cP.i;
#endif

#if DRAWEPG

    /* Get the UN-precoded symbols for EYE pattern */
    cqEYE.r = cY.r << 2;
    cqEYE.i = cY.i << 2;

#if RX_PRECODE_ENABLE
    cqEYE.r -= (pRx->VA_precode.cC.r << 2);
    cqEYE.i -= (pRx->VA_precode.cC.i << 2);
#endif

    cqEYE.r = (QWORD)QDQMULQD(pRx->qdEq_scale, cqEYE.r);
    cqEYE.i = (QWORD)QDQMULQD(pRx->qdEq_scale, cqEYE.i);

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cqEYE.r; }

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cqEYE.i; }

#endif

    pRx->pEpg[0] = cqEYE;

#endif

    /****** Slicer ***********/
    cSlice = V34_Slicer(&cY);

#if 0 /* To show slicer err */
    cqEYE.r = QDQMULQD(pRx->qdEq_scale, (cSlice.r - cY.r) << 2);
    cqEYE.i = QDQMULQD(pRx->qdEq_scale, (cSlice.i - cY.i) << 2);

    pRx->pEpg[0] = cqEYE;
#endif

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = QQMULQD(cY.r - cSlice.r, cY.r - cSlice.r) + QQMULQD(cY.i - cSlice.i, cY.i - cSlice.i); }
#endif

    /* Find the Decision point for Loop updated, Eq update */
    cqD_sym.r = cSlice.r << 2;
    cqD_sym.i = cSlice.i << 2;

#if RX_PRECODE_ENABLE
    cqD_sym.r -= (pRx->VA_precode.cP.r << 2);
    cqD_sym.i -= (pRx->VA_precode.cP.i << 2);
#endif

    cqD_sym.r = QDQMULQDR(pRx->qdEq_scale, cqD_sym.r);
    cqD_sym.i = QDQMULQDR(pRx->qdEq_scale, cqD_sym.i);

    if (pRx->rx_nlinear)
    {
        V34_Non_Linear_Encoder(pRx->qEq_nl_scale, &cqD_sym);
    }

    CQSUB(cqD_sym, cqUs, cqErrsym);

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = QQMULQD(cqErrsym.r, cqErrsym.r) + QQMULQD(cqErrsym.i, cqErrsym.i); }
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cqD_sym.r; }
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cqUs.r; }
#endif

    if (DeCode->nScaleAdjustCount < 0)
    {
        qdTemp  = QQMULQD(cqD_sym.r, cqD_sym.r) + QQMULQD(cqD_sym.i, cqD_sym.i);
        qdTemp -= QQMULQD(cqUs.r,    cqUs.r) + QQMULQD(cqUs.i,    cqUs.i);
        DeCode->qdScaleAdjust += qdTemp >> 10;
    }
    else if (DeCode->nScaleAdjustCount == 0)
    {
        DeCode->qdScaleAdjustSum  <<= 9;
        DeCode->qdScaleAdjustSum1 >>= 6;

        DeCode->qdScaleAdjust = (DeCode->qdScaleAdjustSum / (QWORD)DeCode->qdScaleAdjustSum1) << 16;

        pRx->qdARS_erregy            = 0;
        pRx->Retrain_Request_Counter = 0;

        DeCode->nScaleAdjustCount = -1;
    }
    else
    {
        if (DeCode->nScaleAdjustCount != 120)
        {
            if (cqD_sym.r < 0)
            {
                DeCode->qdScaleAdjustSum  -= (cqD_sym.r - cqUs.r);
                DeCode->qdScaleAdjustSum1 -= cqD_sym.r;
            }
            else
            {
                DeCode->qdScaleAdjustSum  += (cqD_sym.r - cqUs.r);
                DeCode->qdScaleAdjustSum1 += cqD_sym.r;
            }

            if (cqD_sym.i < 0)
            {
                DeCode->qdScaleAdjustSum  -= (cqD_sym.i - cqUs.i);
                DeCode->qdScaleAdjustSum1 -= cqD_sym.i;
            }
            else
            {
                DeCode->qdScaleAdjustSum  += (cqD_sym.i - cqUs.i);
                DeCode->qdScaleAdjustSum1 += cqD_sym.i;
            }
        }

        DeCode->nScaleAdjustCount --;
    }

    if (DeCode->nScaleAdjustCount != 119)
    {
        V34_TimingLoop(pTCR, &cqUs, &cqD_sym);

#if PHASEJITTER

        if (pJT->enable == 1)
        {
            V34_jt_loop(pJT, &cqSymbol, &cqD_sym, 0);
        }

#endif

        V34_CarrierLoop(pTCR, &cqUs, &cqD_sym);
    }

    /**********************End Update************************/

#if RX_PRECODE_ENABLE
    V34_Precoder(&pRx->VA_precode, &cX, 0);
#endif

    dec->pcVA_sym[dec->VA_sym_inidx] = cY;

    dec->VA_sym_inidx ++;
    dec->VA_sym_inidx &= V34_VA_BUF_MASK;

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cY.r; }
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cY.i; }
#endif

    dec->VA_sym_count ++;
    if (dec->VA_sym_count >= 2)
    {
		dec->VA_sym_count = 0;

        V34_VA_Decode(DeCode, pcYout);

#if 0

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pcYout[0].r; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pcYout[0].i; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pcYout[1].r; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pcYout[1].i; }

#endif

        V34_Precode_Sub(pRx, pcYout);
    }

    /* ======================Retrain Request ======================= */
    V34_ARS_Calc(pRx, &cqErrsym);
    ++pRx->Retrain_Request_Counter;

    if (pRx->Retrain_Request_Counter >= 1024)
    {
        pRx->qSum_erregy = QDR16Q(pRx->qdARS_erregy);

#if 0

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pRx->qRTRN_Req_Thres; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pRx->qSum_erregy; }

        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pRx->qPrev_erregy; }

#endif

        if (pRx->qSum_erregy > (pRx->qRTRN_Req_Thres >> 2))
        {
            if ((pRx->Retrain_HI_BER_Counter > 2) || (pRx->qSum_erregy > pRx->qRTRN_Req_Thres))
            {
                TRACE0("Retrain by local");

                pRx->Retrain_flag = RETRAIN_INIT;

                pV34State->retrain_reason = V34_BIT_ERROR_TOO_HIGH;
            }
            else if (pRx->qSum_erregy > (pRx->qPrev_erregy << 2))
            {
                pRx->Retrain_HI_BER_Counter++;
            }
            else if (pRx->qSum_erregy > pRx->qPrev_erregy)
            {
                pRx->Retrain_LO_BER_Counter++;

                if (pRx->Retrain_LO_BER_Counter > 10)
                {
                    /*
                    Netro project about retrain for V.34 when adding noise after data mode.
                    It is a problem of baseline. It must retrain but re-rate at the function
                    V34_Decoder. About what time to retrain, what time to re-rate for V34,
                    the problem is not be fixed so far.

                    The problem is similar to the bug related to V22bis at v22dout.c.
                    At the version 01.27.132, we fixed the bug related to retrain of V.22bis
                    for Hang up at different Signal level test. The problem was
                    that when reducing PCMin level, should retrain, but ds->qErrorAvg too small
                    when shift 8 and could not retrain. The AGC can not update.
                    The old code shifted 6 before version 01.12, it was modified to shift 8
                    since version 01.13 (02/27/02). At the version 01.27.132, fixed the bug
                    by changing back shifting 6 to retrain. But the retrain condition
                    did not adapt for some case.
                    */
                    pRx->Renego.renego_generate = 1;/* pRx->Retrain_flag = RETRAIN_INIT; */
                    TRACE2("GAO RENEGO Request. retrain=%d, renego_generate=%d", pRx->Retrain_flag, pRx->Renego.renego_generate);
                    pV34State->retrain_reason = V34_BIT_ERROR_TOO_HIGH;
                }
            }
        }
        else
        {
            if (pRx->Retrain_LO_BER_Counter > 0)
            {
                pRx->Retrain_LO_BER_Counter--;
            }

            pRx->Retrain_HI_BER_Counter = 0;
        }

        pRx->qPrev_erregy = pRx->qSum_erregy;
        pRx->Retrain_Request_Counter = 0;
        pRx->qdARS_erregy = 0;
    }
}
#endif