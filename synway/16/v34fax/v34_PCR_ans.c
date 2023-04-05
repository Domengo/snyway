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
#include "v34fext.h"

#if SUPPORT_V34FAX

#define DUMP_IMAGE_TO_IO    (0)    /* t2 & t3, GAO Rx */
#define RX_CONSTELLATION    (0)    /* t4 */

#define RECEIVED_PAGE       (2)

#define DISPLAY_S_SB_PP     (0)    /* t4, t5 */
#define DISPLAY_DATA        (0)    /* t3 */
#define VA_NON_LINEAR       (0)

#define BEFORE_NON_LINEAR   (0)
#define AFTER_NON_LINEAR    (0)
#define EQ_ERROR            (0)

//#define PP_EQ_BETA1       (8000)
//#define PP_EQ_BETA2       (4000)
//#define SB_EQ_BETA        (5000)

#define V34_qBETA_DATA      (0)

#define DISPLAY_ERROR       (0) /* 1: for debug display, delete in real version */

#if DISPLAY_ERROR
int err_sum;
#endif

UBYTE V34Fax_Carrier_Detect(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34TxStruct  *pV34Tx  = &(pV34->V34Tx);
    QDWORD qdCarrier_Egy;
    UBYTE  i;
    UBYTE  Carrier_Detected = 0;

    qdCarrier_Egy = 0;

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        qdCarrier_Egy += QQMULQD(pV34Tx->PCMinPtr[i], pV34Tx->PCMinPtr[i]);
    }

    if (qdCarrier_Egy > 300000000 || qdCarrier_Egy < 0)
    {
        qdCarrier_Egy = 300000000;
    }

    pV34Fax->qdCarrier_Egy += (qdCarrier_Egy >> 1);

    if (pV34Fax->Sym_Count < (pV34->NumSymbol >> 1))
    {
        pV34Fax->Sym_Count ++;
    }

    if (pV34Fax->Sym_Count == (pV34->NumSymbol >> 1))
    {
        pV34Fax->Sym_Count = 0;

        if ((pV34Fax->qdCarrier_Egy > pV34Fax->qdNoise_Egy_Ref) && (pV34Fax->qdCarrier_Egy > 50000))/* 50000 for -48dB level */
        {
            Carrier_Detected = 1;
        }

        pV34Fax->qdCarrier_Egy = 0;
    }

    return (Carrier_Detected);
}

#if 1

void V34Fax_PCR_Detect_S(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase3_Info *p3 = &(pV34->p3);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD cqSymbol;
    QDWORD qdAgy = 0, qdS_Agy_Diff;
    UBYTE  S_Detected;

    if (pV34Fax->PCR_SilenceSkipCount < 32)
    {
        pV34Fax->PCR_SilenceSkipCount ++;
    }
    else
    {
        cqSymbol.r = 0;
        cqSymbol.i = 0;

        if (pV34Fax->S_Detected == 0)
        {
            S_Detected = V34Fax_Carrier_Detect(pV34);

            if (S_Detected == 1)
            {
                pV34Fax->S_Detected = 1;
            }
        }

        if (pV34Fax->S_Detected == 1)
        {
            cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
            cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

            pV34Fax->tx_sym_cnt = 0;

            qdAgy = (QQMULQD(cqSymbol.r, cqSymbol.r) >> 1) + (QQMULQD(cqSymbol.i, cqSymbol.i) >> 1);

            qdS_Agy_Diff = qdAgy - pV34Fax->qdS_Agy_Pre;

            /// LLL debug, the code from kylink
            if ((qdS_Agy_Diff - pV34Fax->qdS_Agy_Diff) > 50000)
            {
                pV34Fax->tx_sym_cnt = 1;
            }

            pV34Fax->qdS_Agy_Diff = qdS_Agy_Diff;

            if (pV34Fax->qdS_Agy_Max < pV34Fax->qdS_Agy_Pre)
            {
                pV34Fax->qdS_Agy_Max = pV34Fax->qdS_Agy_Pre;
            }

            pV34Fax->qdS_Agy_Pre = qdAgy;

            if (pV34Fax->tx_sym_cnt)
            {
                pV34Fax->qdS_Agy_Max  = 0;
                pV34Fax->qdS_Agy_Diff = 0;
                pV34Fax->qdS_Agy_Pre  = 0;
                pV34Fax->S_Detected   = 0;

                p3->rx_vec_idx++;

                pV34Fax->tx_sym_cnt = 0;

                pV34Fax->PPS_detect_flag = 1;

                pTCR->nTimingIdx = 0;

                V34Fax_PCR_SyncInit(pV34);

                pV34Fax->HS_Data_flag = 1;

                pV34Fax->PCR_SilenceSkipCount = 0;

                TRACE0("PCR S detected");
            }
        }
    }
}

#else

void V34Fax_PCR_Detect_S(V34Struct *pV34)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV34->pTable[DPCS_STRUC_IDX]);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase3_Info *p3 = &(pV34->p3);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD cqSymbol;
    QDWORD qdAgy;

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if 0

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = cqSymbol.r; }

#endif

    pV34Fax->tx_sym_cnt = 0;

    qdAgy = (QQMULQD(cqSymbol.r, cqSymbol.r) >> 1) + (QQMULQD(cqSymbol.i, cqSymbol.i) >> 1);

    if (qdAgy > 26000000)//8000000)/* Should this one be modified too?? */
    {
        pV34Fax->tx_sym_cnt = 1;
    }

#if 0
    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = (QWORD)(qdAgy >> 16); }
#endif

#if 0
    if ((cqSymbol.r > 2000) || (cqSymbol.r < -2000) || (cqSymbol.i > 2000) || (cqSymbol.i < -2000))
    {
        pV34Fax->tx_sym_cnt = 1;
    }
#endif

    if (pV34Fax->tx_sym_cnt)
    {
        p3->rx_vec_idx++;

        pV34Fax->tx_sym_cnt = 0;

        if (pV34Fax->FreqOffset_Enable == 0)
        {
            pV34Fax->PPS_detect_flag = 1;
        }

        pTCR->nTimingIdx = 0;

        V34Fax_PCR_SyncInit(pV34);

        pV34Fax->HS_Data_flag = 1;
    }
}

#endif


void V34Fax_PCR_Detect_S_bar(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    Phase3_Info *p3 = &(pV34->p3);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    UBYTE i, j, nbyte;
    CQWORD tDsym[V34_SYM_SIZE];
    CQWORD cqTimingIn[V34_SYM_SIZE];
    CQWORD cqTimingOut[V34_SYM_SIZE+1];
    CQWORD cqSymbol;
    UBYTE Varable_Temp1;
    UBYTE Varable_Temp2;
    UBYTE Varable_Temp3;

    cqSymbol.r = 0;
    cqSymbol.i = 0;

    V34_Demodulate(pRx, tDsym);

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        cqTimingIn[i] = tDsym[i];

        cqTimingOut[i].r = 0;
        cqTimingOut[i].i = 0;
    }

    cqTimingOut[V34_SYM_SIZE].r = 0;
    cqTimingOut[V34_SYM_SIZE].i = 0;

    if (pV34Fax->EQ_Mode == 0)
    {
        Varable_Temp1 = 96;
        Varable_Temp2 = 4;
        Varable_Temp3 = 98;
    }
    else
    {
        Varable_Temp1 = 64;
        Varable_Temp2 = 8;
        Varable_Temp3 = 66;
    }

    if ((pV34Fax->sbSyncCounter <= 0) && (pV34Fax->tx_sym_cnt <= Varable_Temp1))//64))//96))
    {
        if (pV34Fax->sbSyncCounter == 0)
        {
            j = 0;
            pV34Fax->sbSyncCounter = -1;
        }
        else
        {
            j = V34_SYM_SIZE;
            pV34Fax->sbSyncCounter = 0;
        }

        for (i = 0; i < V34_SYM_SIZE; i++)
        {
            cqTimingIn[i].r = (QWORD)pV34Fax->cqdDemodSTone[i + j].r;
            cqTimingIn[i].i = (QWORD)pV34Fax->cqdDemodSTone[i + j].i;
        }

        V34Fax_Timing_Rec(pV34Fax, cqTimingIn, cqTimingOut);

        V34Fax_PCR_SyncEq(pV34, cqTimingOut);

        if ((pV34Fax->tx_sym_cnt & 0x1) && (pTCR->nTimingIdx <= 126) && (pV34Fax->tx_sym_cnt > 32))
        {
            V34Fax_PCR_SyncTiming(pV34Fax, pTCR->nTimingIdx);

            if (pTCR->nTimingIdx < 122)
            {
                pTCR->nTimingIdx += Varable_Temp2;    //4
            }
            else
            {
                pTCR->nTimingIdx = 0;
            }
        }
    }
    else if (pV34Fax->sbSyncCounter > 0)
    {
        V34Fax_PCR_SyncDemod(pV34, tDsym);

        V34Fax_Timing_Rec(pV34Fax, cqTimingIn, cqTimingOut);

        V34Fax_PCR_SyncEq(pV34, cqTimingOut);
    }
    else
    {
        nbyte = V34Fax_Timing_Rec(pV34Fax, cqTimingIn, cqTimingOut);

        for (i = 0; i < nbyte; i++)
        {
            pV34Fax->Proc_Timing_Buf[pV34Fax->Proc_Timing_In++] = cqTimingOut[i];

            if (pV34Fax->Proc_Timing_In == 6)
            {
                pV34Fax->Proc_Timing_In = 0;
            }
        }

        pV34Fax->Proc_Timing_Num += nbyte;

        while (pV34Fax->Proc_Timing_Num >= 3)
        {
            for (i = 0; i < V34_SYM_SIZE; i++)
            {
                cqTimingOut[i] = pV34Fax->Proc_Timing_Buf[pV34Fax->Proc_Timing_Out++];

                if (pV34Fax->Proc_Timing_Out == 6)
                {
                    pV34Fax->Proc_Timing_Out = 0;
                }
            }

            pV34Fax->Proc_Timing_Num -= 3;

            cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, cqTimingOut);

            cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);
        }

        if (pV34Fax->tx_sym_cnt == Varable_Temp3)//66)//98)
        {
            pTCR->nTimingIdx = pV34Fax->bestPolyIdx;

            if (pTCR->nTimingIdx > 256)
            {
                pTCR->nTimingIdx -= 384;
            }

            TRACE1("Best Idx: %d", pTCR->nTimingIdx);
        }
        else if (pV34Fax->tx_sym_cnt == 114)
        {
            pV34Fax->cqOldEqOut[0] = cqSymbol;
        }
        else if (pV34Fax->tx_sym_cnt == 115)
        {
            V34Fax_PCR_SyncCarrier(pV34, pV34Fax->cqOldEqOut[0], cqSymbol);
        }
    }

    pV34Fax->tx_sym_cnt++;

    if (pV34Fax->tx_sym_cnt >= 120)
    {
        if (cqSymbol.i < 0)
        {
            p3->rx_vec_idx++;
            pV34Fax->tx_sym_cnt = 0;

            TRACE0("PCR S bar detected");

#if DISPLAY_ERROR
            err_sum = 0;
#endif

            pV34Fax->HS_Data_flag = 0;
        }
    }

#if DISPLAY_S_SB_PP
    if ((pV34Fax->tx_sym_cnt <= 95) && (pV34Fax->tx_sym_cnt >= 32))
    {
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = pV34Fax->cqNewEqOut[0].r; }
        if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = pV34Fax->cqNewEqOut[0].i; }
        if (DumpTone3_Idx < 50000) { DumpTone3[DumpTone3_Idx++] = pV34Fax->cqNewEqOut[1].r; }
        if (DumpTone4_Idx < 50000) { DumpTone4[DumpTone4_Idx++] = pV34Fax->cqNewEqOut[1].i; }
        if (DumpTone5_Idx < 50000) { DumpTone5[DumpTone5_Idx++] = pV34Fax->cqNewEqOut[2].r; }
        if (DumpTone6_Idx < 50000) { DumpTone6[DumpTone6_Idx++] = pV34Fax->cqNewEqOut[2].i; }
    }
#else

    if (pV34Fax->tx_sym_cnt > 98)
    {
        //if (DumpTone3_Idx < 50000) DumpTone3[DumpTone3_Idx++] = cqSymbol.r;
        //if (DumpTone4_Idx < 50000) DumpTone4[DumpTone4_Idx++] = cqSymbol.i;
    }

#endif
}


void V34Fax_PCR_S_Bar_Train(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info *p3 = &(pV34->p3);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD cqSymbol;
    CQWORD Train_sym;
    CQWORD Errsym;

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);

    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

    pRx->cqEQ_symbol = cqSymbol;

    V34_Symbol_To_Bits(cqSymbol, &Train_sym, pV34Rx->Num_Trn_Rx);/* always 4 points in Phase3 */

    CQSUB(Train_sym, cqSymbol, Errsym);

#if DISPLAY_S_SB_PP
    //if (pV34Fax->Enter_CC_Order == RECEIVED_PAGE)
    {
        if (DumpTone4_Idx < 50000) { DumpTone4[DumpTone4_Idx++] = cqSymbol.r; }

        if (DumpTone5_Idx < 50000) { DumpTone5[DumpTone5_Idx++] = cqSymbol.i; }
    }
#endif

    pV34Fax->tx_sym_cnt++;

    if (pV34Fax->tx_sym_cnt >= 15)
    {
        p3->rx_vec_idx++;
        pV34Fax->tx_sym_cnt = 0;
        p3->I = 0;
        p3->K = 0;

        if (pV34Fax->FreqOffset_Enable == 0)
        {
            V34Fax_TimingLoop_Init_ForPP_and_Data(pV34Fax);
        }

        pTCR->qCarAcoef = 0; /* 0.97 */
        pTCR->qCarBcoef = 1000;

#if DISPLAY_ERROR
        TRACE1("Resync error:                  %d", err_sum >> 10);
#endif
    }

#if DISPLAY_ERROR

    if (pV34Fax->tx_sym_cnt >= 3)
    {
        //V34_2T3_Equalizer_Update(Errsym, SB_EQ_BETA, pTCR);
        //V34Fax_TimingLoop(pTCR, &cqSymbol, &Train_sym);
        //V34_CarrierLoop(pTCR, &cqSymbol, &Train_sym);

        err_sum += ((Errsym.r) * (Errsym.r));
        err_sum += ((Errsym.i) * (Errsym.i));
    }

#endif
}


void V34Fax_PCR_PP_EQ_Train(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info *p3 = &(pV34->p3);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD      cqSymbol, Train_sym, Errsym;
    /*QWORD qBeta;*/

    cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pV34Rx->tTimingOut);
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);
    pRx->cqEQ_symbol = cqSymbol;

    /* Create PP symbol pattern for training EQ */
    if (p3->I > 3)
    {
        p3->I = 0;
        p3->K ++;
    }

    Train_sym = V34_Create_PP(p3->K, p3->I);

    p3->I ++;

    CQSUB(Train_sym, cqSymbol, Errsym);

#if DISPLAY_S_SB_PP
    if (DumpTone6_Idx < 50000) { DumpTone6[DumpTone6_Idx++] = cqSymbol.r; }
    if (DumpTone5_Idx < 50000) { DumpTone5[DumpTone5_Idx++] = cqSymbol.i; }
#endif

    pV34Fax->tx_sym_cnt++;

    if (pV34Fax->tx_sym_cnt <= 288)  /* for debug */
    {
#if 0

        if (pV34Fax->tx_sym_cnt <= 100)
        {
            qBeta = PP_EQ_BETA1;
        }
        else
        {
            qBeta = PP_EQ_BETA2;
        }

        V34_2T3_Equalizer_Update(Errsym, qBeta, pTCR);
#else
        V34Fax_TimingLoop(pV34Fax, &cqSymbol, &Train_sym);
        V34_CarrierLoop(pTCR, &cqSymbol, &Train_sym);
#endif
    }

    if (pV34Fax->tx_sym_cnt >= 288)
    {
        p3->rx_vec_idx++;
        pV34Fax->tx_sym_cnt = 0;
        V34Fax_PCR_Rx_Setup(pV34);

        if (pV34Fax->FreqOffset_Enable == 0)
        {
            V34Fax_TimingLoop_Init_ForPP_and_Data(pV34Fax);
        }

        pV34Fax->HS_Data_flag = 1;

        pTCR->qdError_phase = 0;

        pTCR->qCarAcoef = q095;
        pTCR->qCarBcoef = q005;

        TRACE0("PCR PP detected");
    }
}

void V34Fax_PCR_VA_Delay(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info *p3 = &(pV34->p3);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    DeCodeStruc *DeCode = &(pRx->DeCode);
    VA_INFO *dec = &(DeCode->Vdec);
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    TimeCarrRecovStruc *pTCR = pV34Fax->pTCR;
#if RX_PRECODE_ENABLE
    PrecodeStruct *precode_ptr = &(pRx->rx_precode);
#endif
#if PHASEJITTER
    JTstruct *pJT = &pRx->jtvar;
#endif
    CQWORD cqSymbol, cqErrsym, cqUs;
    CQWORD cqD_sym;
    CQWORD pcDsym[3], pcTimingOut[3];
    CQWORD pcYout[2];
    CQWORD cX, cY;
    CQWORD cSlice;
    UBYTE  i, nbyte;

    /* Viterbi window is 25, two symbols each window */
    CQWORD TimingOut[4];

    V34_Demodulate(pRx, pcDsym);

    nbyte = V34Fax_Timing_Rec(pV34Fax, pcDsym, TimingOut);

    for (i = 0; i < nbyte; i++)
    {
        pV34Fax->Proc_Timing_Buf[pV34Fax->Proc_Timing_In++] = TimingOut[i];

        if (pV34Fax->Proc_Timing_In == 6)
        {
            pV34Fax->Proc_Timing_In = 0;
        }
    }

    pV34Fax->Proc_Timing_Num += nbyte;

    while (pV34Fax->Proc_Timing_Num >= 3)
    {
        for (i = 0; i < 3; i++)
        {
            pcTimingOut[i] = pV34Fax->Proc_Timing_Buf[pV34Fax->Proc_Timing_Out++];

            if (pV34Fax->Proc_Timing_Out == 6)
            {
                pV34Fax->Proc_Timing_Out = 0;
            }
        }

        pV34Fax->Proc_Timing_Num -= 3;

        cqSymbol = DspcFir_2T3EQ(&pTCR->eqfir, pcTimingOut);
        cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

        pRx->cqEQ_symbol = cqSymbol;

#if PHASEJITTER

        if (pJT->enable == 1)
        {
            cqSymbol = V34_Rotator(&cqSymbol, pJT->qS, pJT->qC);
        }

#endif

#if DRAWEPG
        pRx->pEpg[0] = cqSymbol;
#endif

        V34_4Point_Slicer(&pRx->Renego, &cqSymbol);

        cqUs = cqSymbol;

        if (pRx->rx_nlinear)
        {
            V34_Non_Linear_Decoder(pRx->qRx_nl_scale, &cqSymbol);
        }

#if VA_NON_LINEAR
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = cqSymbol.r; }
        if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = cqSymbol.i; }
#endif

        /* scale down for different data rate */
        cqSymbol.r = (QWORD)QDQMULQD(pRx->qdRx_scale, cqSymbol.r);
        cqSymbol.i = (QWORD)QDQMULQD(pRx->qdRx_scale, cqSymbol.i);

        /* Change to 9.7 format */
        cX.r = cqSymbol.r >> 2;
        cX.i = cqSymbol.i >> 2;

        cY = cX;

#if RX_PRECODE_ENABLE
        /* Precoder to get p(n), cY(n) = x(n) + p(n) */
        cY.r += pRx->VA_precode.cC.r;
        cY.i += pRx->VA_precode.cC.i;
#endif
        /* Slicer for instance Symbol error calculation */
        cSlice = V34_Slicer(&cY);

        cqD_sym.r = cSlice.r << 2;
        cqD_sym.i = cSlice.i << 2;

#if RX_PRECODE_ENABLE
        /* Find the Decision point for Loop updated, Eq update */
        cqD_sym.r -= pRx->VA_precode.cC.r << 2;
        cqD_sym.i -= pRx->VA_precode.cC.i << 2;
#endif

        cqD_sym.r = QDQMULQDR(pRx->qdEq_scale, cqD_sym.r);
        cqD_sym.i = QDQMULQDR(pRx->qdEq_scale, cqD_sym.i);

        if (pRx->rx_nlinear)
        {
            V34_Non_Linear_Encoder(pRx->qEq_nl_scale, &cqD_sym);
        }

        if (pRx->Renego.clear_down || pRx->Renego.renego_generate)
        {
            if (pRx->Renego.S_DetCounter > 40)
            {
                p3->rx_vec_idx ++;
            }

            return;
        }

        CQSUB(cqD_sym, cqUs, cqErrsym);

        //if (pAGC->freeze_EQ == 0)
        //   V34_2T3_Equalizer_Update(cqErrsym, V34_qBETA_TCR, pTCR);
        //V34_2T3_Equalizer_Update(cqErrsym, 1000, pTCR);   //4000

        V34Fax_TimingLoop(pV34Fax, &cqUs, &cqD_sym);

#if VA_NON_LINEAR
        if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = cqErrsym.r; }
#endif

#if PHASEJITTER

        if (pJT->enable == 1)
        {
            pJT->counter++;

            V34_jt_loop(pJT, &cqSymbol, &cqD_sym, 0);
        }

#endif

        V34_CarrierLoop(pTCR, &cqUs, &cqD_sym);

#if RX_PRECODE_ENABLE
        V34_Precoder(&pRx->VA_precode, &cX, DeCode->RxModemData.rx_b);
#endif

        dec->pcVA_sym[dec->VA_sym_inidx] = cY;

        dec->VA_sym_inidx ++;
        dec->VA_sym_inidx &= V34_VA_BUF_MASK;

        dec->VA_sym_count ++;
        if (dec->VA_sym_count >= 2)
        {
            dec->VA_sym_count = 0;

            V34_VA_Decode(DeCode, pcYout);

            if (pcYout[0].r != 0)
            {
                V34_Precode_Sub(pRx, pcYout);

                ++p3->rx_vec_idx;

                pTCR->qCarAcoef = 31785; /* 0.97 */
                pTCR->qCarBcoef = 983;   /* 0.03 */

                pRx->qdARS_erregy            = 0;
                pRx->Retrain_Request_Counter = 0;
                pRx->Retrain_LO_BER_Counter  = 0;
                pRx->Retrain_HI_BER_Counter  = 0;
                pRx->qPrev_erregy            = 32767;

                pAGC->qHLK[4] = AGC_HLK_TAB[4];  /* K1 */
                pAGC->qHLK[5] = AGC_HLK_TAB[5];  /* K2 */
            }
        }
    }
}


void V34Fax_PCR_B1_Rx(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info  *p3      = &(pV34->p3);
    V34RxStruct  *pV34Rx  = &(pV34->V34Rx);
    ReceiveStruc *pRx     = &pV34Rx->RecStruc;

    V34Fax_PCR_Receiver(pV34, 0);

    /* Only entered here for 118 symbols, because 2 symbols already finished by Viterbi delay */
    pV34Fax->tx_sym_cnt++;

    if (pRx->current_rx_J == 0)
    {
        pV34Fax->tx_sym_cnt = 0;

        p3->rx_vec_idx ++;

        TRACE1("B1 Rx(Enter_CC_Order=%d)", pV34Fax->Enter_CC_Order);
    }
}


void V34Fax_PCR_Image_Rx(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    Phase3_Info *p3 = &(pV34->p3);

    V34Fax_PCR_Receiver(pV34, 1);

    V34Fax_CarrierLoss_Detect(pV34);

    if (pV34Fax->Silence_Detect_Count > 3)
    {
        pV34Fax->Silence_Count1       = 0;
        pV34Fax->Silence_Count2       = 0;
        pV34Fax->Silence_Egy_Ref      = 0;
        pV34Fax->Silence_Egy          = 0;
        pV34Fax->Silence_Detect_Count = 0;

        p3->rx_vec_idx++;

        pV34Fax->tx_sym_cnt = 0;

        pV34Fax->PCR_RXend_Flag = 1;

        TRACE0("Image received");
    }
}


void V34Fax_PCR_Rx_Setup(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct  *pV34Rx  = &(pV34->V34Rx);
    MphStruc     *pMphTx  = &pV34Fax->MphTx;
    ReceiveStruc *pRx     = &pV34Rx->RecStruc;
    DeCodeStruc  *DeCode  = &(pRx->DeCode);

    V34_Parameter_Function_Init_Rx(pV34);

    DeCode->Vdec.rx_COV_type = pMphTx->TrellisEncodSelect;

    pRx->rx_nlinear = pMphTx->NonlinearEncodParam;

    if (DeCode->RxModemData.rx_M > 1)
    {
        V34_Calc_g2(DeCode->RxModemData.rx_M, pRx->RxShellMap.pRx_g2);
        V34_Calc_g4(DeCode->RxModemData.rx_M, pRx->RxShellMap.pRx_g2, pRx->RxShellMap.pRx_g4);
        V34_Calc_g8(DeCode->RxModemData.rx_M, pRx->RxShellMap.pRx_g4, pRx->RxShellMap.pRx_g8);
    }

    V34_Phase4_ReceiverSetup(pV34);
}


void V34Fax_PCR_Receiver(V34Struct *pV34, SWORD qToIO)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct  *pV34Rx  = &(pV34->V34Rx);
    ReceiveStruc *pRx     = &pV34Rx->RecStruc;
    DeCodeStruc  *DeCode  = &(pRx->DeCode);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    CQWORD cqSymbol;
    CQWORD pcDsym[3], pcTimingOut[3];
    UBYTE i, b, nbyte, hi_frame;
    SBYTE nbits;
    UBYTE *pIOBuf;
    CQWORD TimingOut[4];

    V34_Demodulate(pRx, pcDsym);

    nbyte = V34Fax_Timing_Rec(pV34Fax, pcDsym, TimingOut);

    for (i = 0; i < nbyte; i++)
    {
        pV34Fax->Proc_Timing_Buf[pV34Fax->Proc_Timing_In++] = TimingOut[i];

        if (pV34Fax->Proc_Timing_In == 6)
        {
            pV34Fax->Proc_Timing_In = 0;
        }
    }

    pV34Fax->Proc_Timing_Num += nbyte;

    while (pV34Fax->Proc_Timing_Num >= 3)
    {
        for (i = 0; i < 3; i++)
        {
            pcTimingOut[i] = pV34Fax->Proc_Timing_Buf[pV34Fax->Proc_Timing_Out++];

            if (pV34Fax->Proc_Timing_Out == 6)
            {
                pV34Fax->Proc_Timing_Out = 0;
            }
        }

        pV34Fax->Proc_Timing_Num -= 3;
        cqSymbol = DspcFir_2T3EQ(&(pTCR->eqfir), pcTimingOut);

        V34Fax_PCR_Decoder(pV34, &cqSymbol);

        /* When first time, already 2 symbols finished by Viterbi delay process */
        if (pRx->rx_sym_count >= 8)
        {
            nbyte = DeCode->RxModemData.rx_nbyte;
            nbits = DeCode->RxModemData.rx_nbits;

            b = DeCode->RxModemData.rx_b;

            hi_frame = pRx->current_rx_SWP_bit & 0x1;
            pRx->current_rx_SWP_bit >>= 1;

            if (hi_frame == 0)
            {
                --b;

                if (--nbits < 0)
                {
                    nbits = 7;
                    --nbyte;
                }
            }

            V34_Retrieve_Infobits(pRx, hi_frame);

            /**************** Fill IO Buffer ********************/
            pIOBuf = (UBYTE *)(pV34->pTable[DSPIOWRBUFDATA_IDX]);
            (*pRx->pfDescram_byte)(pRx->pDscram_buf, &(pRx->dsc), pIOBuf, nbyte);
            (*pRx->pfDescram_bit)(&(pRx->pDscram_buf[b-nbits]), &(pRx->dsc), &(pIOBuf[nbyte]), nbits);
            /**************** Fill IO Buffer ********************/

            /**************** IO Write to terminal ********************/
            if (qToIO)
            {
#if DUMP_IMAGE_TO_IO
                /* use V34Fax_image to analyse -> frame.txt */
                //if (pV34Fax->Enter_CC_Order == 1)
                {
                    if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = b; }
                    if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[0]; }
                    if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[1]; }
                    if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[2]; }
                    if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[3]; }
                    if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[4]; }
                    if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[5]; }
                    if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[6]; }
                    if (DumpTone3_Idx < 300000) { DumpTone3[DumpTone3_Idx++] = pIOBuf[7]; }
                }
#endif
                IoWrProcess(pV34->pTable, b);
            }
        }
    }
}


void V34Fax_PCR_Decoder(V34Struct *pV34, CQWORD *pcSym)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    DeCodeStruc *DeCode = &(pRx->DeCode);
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
    PrecodeStruct *precode_ptr;
#endif
    QDWORD qdTemp;
#if DRAWEPG
    CQWORD cqEYE;
#endif

    cqSymbol.r = pcSym->r;
    cqSymbol.i = pcSym->i;
    cqSymbol = V34_Rotator(&cqSymbol, pTCR->qS, pTCR->qC);

#if RX_CONSTELLATION
    if (pV34Fax->Enter_CC_Order == RECEIVED_PAGE)
    {
        if (DumpTone4_Idx < 5000000) { DumpTone4[DumpTone4_Idx++] = cqSymbol.r; }
        if (DumpTone5_Idx < 5000000) { DumpTone5[DumpTone5_Idx++] = cqSymbol.i; }
    }
#endif

#if BEFORE_NON_LINEAR
    if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = cqSymbol.r; }
    if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = cqSymbol.i; }
#endif

#if PHASEJITTER

    if (pJT->enable == 1)
    {
        cqSymbol = V34_Rotator(&cqSymbol, pJT->qS, pJT->qC);
    }

#endif

    /* Gain micro-adjust, only in data mode */
    cqSymbol.r += QQMULQ15(cqSymbol.r, QD16Q(DeCode->qdScaleAdjust));
    cqSymbol.i += QQMULQ15(cqSymbol.i, QD16Q(DeCode->qdScaleAdjust));

    /* Retrain detection */
    V34_4Point_Slicer(&(pRx->Renego), &cqSymbol);

    cqUs = cqSymbol;

    if (pRx->rx_nlinear)
    {
        V34_Non_Linear_Decoder(pRx->qRx_nl_scale, &cqSymbol);
    }

#if AFTER_NON_LINEAR
    if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = cqSymbol.r; }
    if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = cqSymbol.i; }
#endif

    pRx->cqEQ_symbol = cqSymbol;

    /* Scale down for different data rate and change to 9.7 format */
    cX.r = QDQMULQDR16((pRx->qdRx_scale >> 1), cqSymbol.r);
    cX.i = QDQMULQDR16((pRx->qdRx_scale >> 1), cqSymbol.i);

    cY = cX;

#if RX_PRECODE_ENABLE
    /* All precoder is in 9.7 arithmetic */
    precode_ptr = &(pRx->VA_precode);

    cY.r += precode_ptr->cP.r;
    cY.i += precode_ptr->cP.i;
#endif

#if DRAWEPG

    /* Get the UN-precoded symbols for EYE pattern */
    cqEYE.r = cY.r << 2;
    cqEYE.i = cY.i << 2;

#if RX_PRECODE_ENABLE
    cqEYE.r -= precode_ptr->cC.r << 2;
    cqEYE.i -= precode_ptr->cC.i << 2;
#endif

    cqEYE.r = (QWORD)QDQMULQD(pRx->qdEq_scale, cqEYE.r);
    cqEYE.i = (QWORD)QDQMULQD(pRx->qdEq_scale, cqEYE.i);

    /************* For EPG debug *********/
    pRx->pEpg[0] = cqEYE;
    /************** End of Epg ***********/

#endif

    /****** Slicer ***********/
    cSlice.r = cY.r & 0xFF80;
    cSlice.i = cY.i & 0xFF80;
    /* Check if slice symbol is even, if it is even, round it to */
    /* the nearest odd number.                                */

    if ((cSlice.r & 0x0080) == 0)
    {
        cSlice.r += 0x0080;
    }

    if ((cSlice.i & 0x0080) == 0)
    {
        cSlice.i += 0x0080;
    }

    /****** End Slicer *******/

    /***** Find the Decision point for Loop update, Eq update **********/
    cqD_sym.r = cSlice.r << 2;
    cqD_sym.i = cSlice.i << 2;

#if RX_PRECODE_ENABLE
    cqD_sym.r -= precode_ptr->cP.r << 2;
    cqD_sym.i -= precode_ptr->cP.i << 2;
#endif

    cqD_sym.r = QDQMULQDR(pRx->qdEq_scale, cqD_sym.r);
    cqD_sym.i = QDQMULQDR(pRx->qdEq_scale, cqD_sym.i);

    if (pRx->rx_nlinear)
    {
        V34_Non_Linear_Encoder(pRx->qEq_nl_scale, &cqD_sym);
    }

    CQSUB(cqD_sym, cqUs, cqErrsym);

    /* Gain micro-adjust update */
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

        DeCode->qdScaleAdjust = ((DeCode->qdScaleAdjustSum) / (QWORD)DeCode->qdScaleAdjustSum1) << 16;

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

#if DISPLAY_DATA
    if (DumpTone1_Idx < 500000) { DumpTone1[DumpTone1_Idx++] = cqD_sym.r; }
    if (DumpTone2_Idx < 500000) { DumpTone2[DumpTone2_Idx++] = cqD_sym.i; }
    if (DumpTone3_Idx < 500000) { DumpTone3[DumpTone3_Idx++] = cqUs.r; }
    if (DumpTone4_Idx < 500000) { DumpTone4[DumpTone4_Idx++] = cqUs.i; }
    if (DumpTone5_Idx < 500000) { DumpTone5[DumpTone5_Idx++] = cqErrsym.r; }
    if (DumpTone6_Idx < 500000) { DumpTone6[DumpTone6_Idx++] = cqErrsym.i; }
#endif

    V34_2T3_Equalizer_Update(cqErrsym, V34_qBETA_DATA, pTCR);

#if V34FAX_DEBUG_ERROR
    pV34Fax->EQ_change_count++;

    pV34Fax->current_error += (UDWORD)(QQMULQD(cqErrsym.r, cqErrsym.r) >> 7);
    pV34Fax->current_error += (UDWORD)(QQMULQD(cqErrsym.i, cqErrsym.i) >> 7);

    if (pV34Fax->EQ_change_count == 8000)
    {
        TRACE2("pV34Fax->current_error=%" FORMAT_DWORD_PREFIX "d, EQ error: %" FORMAT_DWORD_PREFIX "d", pV34Fax->current_error, (pV34Fax->current_error >> 15));

        pV34Fax->current_error = 0;

        pV34Fax->EQ_change_count = 0;
    }

#endif

#if EQ_ERROR

    if (DumpTone1_Idx < 50000) { DumpTone1[DumpTone1_Idx++] = cqErrsym.r; }

    if (DumpTone2_Idx < 50000) { DumpTone2[DumpTone2_Idx++] = cqErrsym.i; }

#endif

    if (DeCode->nScaleAdjustCount != 119)
    {
        V34Fax_TimingLoop(pV34Fax, &cqUs, &cqD_sym);

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
    /* VA Precoder */
    V34_Precoder(precode_ptr, &cX, RxMod->rx_b);
#endif

    dec->pcVA_sym[dec->VA_sym_inidx++] = cY;

    dec->VA_sym_inidx &= V34_VA_BUF_MASK;

    dec->VA_sym_count ++;
    if (dec->VA_sym_count >= 2)
    {
        dec->VA_sym_count = 0;

        V34_VA_Decode(DeCode, pcYout);

        V34_Precode_Sub(pRx, pcYout);
    }
}

#endif
