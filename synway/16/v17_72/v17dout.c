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
#include "ioapi.h"
#include "ptable.h"

#if SUPPORT_V17/* The switch is only for compiling, cannot delete!!! */

void V17_Guard_or_ABtone_Detect(V17Struct *pV17)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV17->pTable[DPCS_STRUC_IDX]);
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    DftStruct *pDft = &pV32Share->Dft;
    QDWORD    qdAvg_Energy;

    pV32Share->Poly.nTimingIdx += 72;

    if (pV32Share->ubSagc_Flag == 0)
    {
        qdAvg_Energy = DSPD_DFT_Detect(pDft, pV32Share->qDemodIn, V32_SYM_SIZE);

        if (qdAvg_Energy >= 0)
        {
            qdAvg_Energy >>= 1;

#if 0

            if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = qdAvg_Energy; }

            if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV32Share->qdMagSq[V17_600HZ]; }

            if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV32Share->qdMagSq[V17_1800HZ]; }

            if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV32Share->qdMagSq[V17_3000HZ]; }

            if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV32Share->qdMagSq[V17_V21TONE_1650HZ]; }

#endif

            if (pV17->SilenceCount < V17_SILENCE_LEN_REF)
            {
                if (qdAvg_Energy < V17_ENERGY_REF)
                {
                    pV17->SilenceCount ++;
                }
                else
                {
                    pV17->SilenceCount = 0;
                }
            }
            else if (qdAvg_Energy > V17_ENERGY_REF)
            {
                if (pV17->V21DetectDelayCnt < 100)
                {
                    pV17->V21DetectDelayCnt++;
                }

                if (pV32Share->qdMagSq[V17_600HZ] > qdAvg_Energy && pV32Share->qdMagSq[V17_3000HZ] > qdAvg_Energy)
                {
                    /* 600Hz and 3000Hz both detected. Some FAX do not send TONE A */
                    pV17->ubGuardToneCnt = 0;
                    pV17->ubABtoneCnt ++;

                    if (pV17->ubABtoneCnt >= 2)
                    {
                        /* jump to next RX state, SEGMENT 1 train */
                        TRACE0("V17: AB Detected Directly");

                        //if (pV17->ubTraintype == V17_SHORT_TRAIN) // fixing issue 111
                        {
                            SetReg(pDpcs->MFCF0, CARRIER_GOT);
                        }

#if SUPPORT_ENERGY_REPORT
                        pV17->Detect_FaxTone = 1;
#endif

                        pV32Share->ubSagc_Flag = 1;
                        pV32Share->qdSagcEnergy = 0;

                        pV32Share->nRxDelayCnt = 0;
                        pV32Share->nRxState++;
                    }
                }
                else if ((pV32Share->qdMagSq[V17_1800HZ] >> 2) > qdAvg_Energy && pV32Share->qdMagSq[V17_1800HZ] > pV32Share->qdMagSq[V17_V21TONE_1650HZ])
                {
                    /* 1800Hz only, A TONE */
                    pV17->ubABtoneCnt = 0;
                    pV17->ubGuardToneCnt ++;

                    if (pV17->ubGuardToneCnt >= 3) // fixing issue 109
                    {
                        TRACE0("V17: A Detected first");

                        //if (pV17->ubTraintype == V17_SHORT_TRAIN)
                        {
                            SetReg(pDpcs->MFCF0, CARRIER_GOT);
                        }

                        /* Prepare to jump to next RX state, detect AB TONE */
                        pV32Share->nRxDelayCnt = 0;

                        DSPD_DFT_Init(pDft, 72, 2, (QWORD *)qV17_DetectToneCoefTab, pV32Share->qdMagSq, pV32Share->qQ1Q2, 4);
                    }
                }
                else
                {
                    if (pV17->V21DetectDelayCnt > 10/* V21 Tone detection. Delay 100ms to avoid V21 transmit echo */
                        && pV32Share->qdMagSq[V17_V21TONE_1650HZ] > qdAvg_Energy && pV32Share->qdMagSq[V17_V21TONE_1650HZ] > pV32Share->qdMagSq[0]
                        && pV32Share->qdMagSq[V17_V21TONE_1650HZ] > pV32Share->qdMagSq[1] && pV32Share->qdMagSq[V17_V21TONE_1650HZ] > pV32Share->qdMagSq[2])
                    {
                        pV17->V21DetectNumber++;
                    }
                    else
                    {
                        pV17->V21DetectNumber = 0;
                    }

                    pV17->ubGuardToneCnt = 0;
                    pV17->ubABtoneCnt    = 0;
                }
            }
        }
    }
}


void V17_AB_Detect_after_Guardtone(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    DftStruct *pDft = &pV32Share->Dft;
    QDWORD    qdAvg_Energy;

    qdAvg_Energy = DSPD_DFT_Detect(pDft, pV32Share->qDemodIn, V32_SYM_SIZE);

    if (qdAvg_Energy > 0)
    {
        qdAvg_Energy >>= 1;

#if 0

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = qdAvg_Energy; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV32Share->qdMagSq[V17_600HZ]; }

        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV32Share->qdMagSq[V17_1800HZ]; }

        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV32Share->qdMagSq[V17_3000HZ]; }

#endif

        if (qdAvg_Energy > V17_ENERGY_REF && pV32Share->qdMagSq[V17_600HZ] > qdAvg_Energy && pV32Share->qdMagSq[V17_3000HZ] > (qdAvg_Energy >> 3))
        {
            pV17->ubABtoneCnt ++;

            if (pV17->ubABtoneCnt >= 1)
            {
                /* jump to next RX state, SEGMENT 1 train */
                DpcsStruct *pDpcs = (DpcsStruct *)(pV17->pTable[DPCS_STRUC_IDX]);

                SetReg(pDpcs->MFCF0, CARRIER_GOT);

                TRACE1("V17: RxDelayCnt: %d", pV32Share->nRxDelayCnt);
                TRACE0("V17: AB Detected after A");

#if SUPPORT_ENERGY_REPORT
                pV17->Detect_FaxTone = 1;
#endif

                pV32Share->ubSagc_Flag = 1;
                pV32Share->qdSagcEnergy = 0;
                pV32Share->nRxDelayCnt = 0;
            }
        }
    }
}

/* output for segment 1 of long train */
void V17_RX_DataOut_Seg1L(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    if (pV32Share->ubTimeJamOK == 1)
    {
        pV32Share->pfTimUpdateVec = V32_TimingUpdateBySliceIQ;
        pV32Share->ubTimeJamOK    = 0;
    }

    if (pV32Share->nRxDelayCnt == V17_SEG1_DELAY - V17_AGC_CALCULATION_POINT)
    {
        V32_Sagc(pV32Share, 3344);
    }

    if (pV32Share->nRxDelayCnt == 350)
    {
        pV32Share->pfTimUpdateVec = V17_RX_TimeJam;
        pV32Share->qTimingErr1    = 0;
        pV32Share->qTimingErr2    = 0;
    }

    if (pV32Share->nRxDelayCnt == V17_SEG1_DELAY - V17_AGC_CALCULATION_POINT - 10)
    {
        pV17->qdCorrThres = pV17->qdCorrelation >> 1;
    }

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV17->qdCorrelation; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV17->qdCorrThres; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV32Share->nRxDelayCnt; }

#endif

    /* initialization of segment 2 of long train */
    if ((pV17->qdCorrelation < pV17->qdCorrThres) && (pV32Share->ubSagc_Flag == 0))
    {
        pV32Share->nRxDelayCnt = 0;

        pV32Share->udTrainScramSReg = 0xabb37400;
    }
}

/* output for segment 1 of short train */
void V17_RX_DataOut_Seg1S(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    QDWORD  qdTmp, qdTmp1, qdTmp2;
    QDWORD  qdPhaseErr, qdMagErr;
    UBYTE i;

#if 0

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV32Share->cDemodIQBuf[0].r; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV32Share->cDemodIQBuf[1].r; }

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV32Share->cDemodIQBuf[2].r; }

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV32Share->cDemodIQBuf[0].i; }

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV32Share->cDemodIQBuf[1].i; }

    if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV32Share->cDemodIQBuf[2].i; }

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV32Share->cqEqOutIQ.r; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV32Share->cqEqOutIQ.i; }

    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV32Share->Poly.nTimingIdx; }

#endif

    if (pV32Share->nRxDelayCnt == V17_SEG1_DELAY - V17_AGC_CALCULATION_POINT)
    {
        pV32Share->ubSagc_Flag = 0;
    }

    if (pV32Share->nRxDelayCnt == V17_SEG1_DELAY)
    {
        pV32Share->Poly.nTimingIdx = -8192;
        pV32Share->qdMinValue = 0;
        pV17->qdMaxPhaseErr = 0;
        pV17->qdMinPhaseErr0 = 0x7FFFFFF;
        pV17->qdMinPhaseErr1 = 0x7FFFFFF;
        pV17->timingDelta = 128;
        pV32Share->qdCarr_r = 0;
        pV32Share->qdCarr_i = 0;
    }
    else if ((pV32Share->nRxDelayCnt < (V17_SEG1_DELAY - 54)) && (pV32Share->nRxDelayCnt > (V17_SEG1_DELAY - 186)))
    {
#if 0

        if (pV32Share->nRxDelayCnt > (V17_SEG1_DELAY - 119))
        {
            if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV32Share->cqNewEqOut[0].r; }

            if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV32Share->cqNewEqOut[0].i; }

            if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = pV32Share->cqNewEqOut[1].r; }

            if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = pV32Share->cqNewEqOut[1].i; }

            if (DumpTone7_Idx < 100000) { DumpTone7[DumpTone7_Idx++] = pV32Share->cqNewEqOut[2].r; }

            if (DumpTone8_Idx < 100000) { DumpTone8[DumpTone8_Idx++] = pV32Share->cqNewEqOut[2].i; }
        }

#endif

        if ((pV32Share->nRxDelayCnt & 0x1) && (pV32Share->nRxDelayCnt > (V17_SEG1_DELAY - 119)))
        {
            /* Find the Maximum Error Point */
            for (i = 0; i < 3; i++)
            {
                qdPhaseErr  = QQMULQD(pV32Share->cqOldEqOut[i].r, pV32Share->cqNewEqOut[i].r);
                qdPhaseErr += QQMULQD(pV32Share->cqOldEqOut[i].i, pV32Share->cqNewEqOut[i].i);

                if (qdPhaseErr < 0)
                {
                    qdPhaseErr = -qdPhaseErr;
                }

                qdMagErr = QQMULQD(pV32Share->cqOldEqOut[i].r, pV32Share->cqOldEqOut[i].r);
                qdMagErr += QQMULQD(pV32Share->cqOldEqOut[i].i, pV32Share->cqOldEqOut[i].i);
                qdMagErr -= QQMULQD(pV32Share->cqNewEqOut[i].r, pV32Share->cqNewEqOut[i].r);
                qdMagErr -= QQMULQD(pV32Share->cqNewEqOut[i].i, pV32Share->cqNewEqOut[i].i);

                if (qdMagErr < 0)
                {
                    qdMagErr = -qdMagErr;
                }

                qdPhaseErr += qdMagErr >> 1;

#if 0
                DumpTone1[DumpTone1_Idx + (i*32)] = qdPhaseErr;
                DumpTone2[DumpTone2_Idx + (i*32)] = qdMagErr;

                if (i == 2)
                {
                    DumpTone1_Idx++;
                    DumpTone2_Idx++;

                    if ((DumpTone1_Idx == 32) || (DumpTone1_Idx == 128) || (DumpTone1_Idx == 224))
                    {

                        DumpTone1_Idx += 64;
                        DumpTone2_Idx += 64;
                    }
                }

#endif

                /* Find the Maximum Error Point */
                if (qdPhaseErr > pV17->qdMaxPhaseErr)
                {
                    pV17->qdMaxPhaseErr = qdPhaseErr;
                    pV17->time_index = pV32Share->Poly.nTimingIdx + (i << 12) + 6144;

                    if (pV17->time_index > 4096)
                    {
                        pV17->time_index = pV17->time_index - 12288;
                    }
                }

                /* Find the First Minimum Error Point */
                if (qdPhaseErr < pV17->qdMinPhaseErr0)
                {
                    pV17->qdMinPhaseErr0 = qdPhaseErr;
                    pV17->timeIndex0 = pV32Share->Poly.nTimingIdx + (i * 4096);
                }
            }

            pV32Share->Poly.nTimingIdx += pV17->timingDelta;
        }
        else if (pV32Share->nRxDelayCnt == (V17_SEG1_DELAY - 119))
        {
            qdPhaseErr = pV17->time_index - pV17->timeIndex0;

            if (qdPhaseErr > 6144)
            {
                qdPhaseErr = qdPhaseErr - 12288;
            }
            else if (qdPhaseErr < -6144)
            {
                qdPhaseErr = qdPhaseErr + 12288;
            }

            if (qdPhaseErr > 0)
            {
                pV17->timingDelta = 128;
            }
            else
            {
                pV17->timingDelta = -128;
            }

            pV32Share->Poly.nTimingIdx = pV17->time_index;
        }
        else if ((pV32Share->nRxDelayCnt & 0x1) && (pV32Share->nRxDelayCnt > (V17_SEG1_DELAY - 172)))
        {
            /* Find the Second Minimum Error Point */
            qdPhaseErr = QQMULQD(pV32Share->cqOldEqOut[0].r, pV32Share->cqNewEqOut[0].r);
            qdPhaseErr += QQMULQD(pV32Share->cqOldEqOut[0].i, pV32Share->cqNewEqOut[0].i);
            qdMagErr = QQMULQD(pV32Share->cqOldEqOut[0].r, pV32Share->cqOldEqOut[0].r);
            qdMagErr += QQMULQD(pV32Share->cqOldEqOut[0].i, pV32Share->cqOldEqOut[0].i);
            qdMagErr -= QQMULQD(pV32Share->cqNewEqOut[0].r, pV32Share->cqNewEqOut[0].r);
            qdMagErr -= QQMULQD(pV32Share->cqNewEqOut[0].i, pV32Share->cqNewEqOut[0].i);

            if (qdMagErr < 0)
            {
                qdMagErr = -qdMagErr;
            }

            if (qdPhaseErr < 0)
            {
                qdPhaseErr = -qdPhaseErr;
            }

            qdPhaseErr += qdMagErr >> 1;

            if (qdPhaseErr < pV17->qdMinPhaseErr1)
            {
                pV17->qdMinPhaseErr1 = qdPhaseErr;
                pV17->timeIndex1 = pV32Share->Poly.nTimingIdx;
            }

            pV32Share->Poly.nTimingIdx += pV17->timingDelta;
        }
        else if (pV32Share->nRxDelayCnt == (V17_SEG1_DELAY - 185))
        {
            if (pV17->qdMinPhaseErr0 < pV17->qdMinPhaseErr1)
            {
                pV32Share->Poly.nTimingIdx = pV17->timeIndex0;
                pV17->timingDelta = pV17->timeIndex1 - pV17->timeIndex0;
            }
            else
            {
                pV32Share->Poly.nTimingIdx = pV17->timeIndex1;
                pV17->timingDelta = pV17->timeIndex0 - pV17->timeIndex1;
            }

            if ((pV17->timingDelta < 1024) && (pV17->timingDelta > -1024))
            {
                pV17->timingDelta = 0;
            }

            pV17->qdMinPhaseErr1 = 0;
            pV17->qdMinPhaseErr0 = 0;
            pV32Share->pfEqVec   = V32_Eq_D;
            pV17->pfDetectVec    = V17_Dummy;

            V32_Sagc(pV32Share, 3344);
        }
    }
    else if ((pV32Share->nRxDelayCnt < (V17_SEG1_DELAY - 187)) && (pV32Share->nRxDelayCnt > (V17_SEG1_DELAY - 220)))
    {
        pV32Share->qdCarr_r += pV32Share->cqEqOutIQ.r;
        pV32Share->qdCarr_i += pV32Share->cqEqOutIQ.i;

        if (pV32Share->nRxDelayCnt == (V17_SEG1_DELAY - 219))
        {
            pV32Share->qdCarr_r >>= 5;
            pV32Share->qdCarr_i >>= 5;

            V32_Rotate_Temp(pV32Share);
        }
    }
    else if ((pV32Share->nRxDelayCnt <= (V17_SEG1_DELAY - 220)) && (pV32Share->nRxDelayCnt > (V17_SEG1_DELAY - 300)))
    {
        V17_Rotate(pV17);

#if 0

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV32Share->cqRotateIQ.r; }

        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pV32Share->cqRotateIQ.i; }

#endif

        qdTmp  = QQMULQD((pV32Share->cqRotateIQ.r - V32_SIGMAP6), (pV32Share->cqRotateIQ.r - V32_SIGMAP6)) +
                 QQMULQD((pV32Share->cqRotateIQ.i - V32_SIGMAP2), (pV32Share->cqRotateIQ.i - V32_SIGMAP2));

        qdTmp1 = QQMULQD((pV32Share->cqRotateIQ.r - V32_SIGMAP2), (pV32Share->cqRotateIQ.r - V32_SIGMAP2)) +
                 QQMULQD((pV32Share->cqRotateIQ.i + V32_SIGMAP6), (pV32Share->cqRotateIQ.i + V32_SIGMAP6));

        qdTmp2 = QQMULQD((pV32Share->cqRotateIQ.r + V32_SIGMAP6), (pV32Share->cqRotateIQ.r + V32_SIGMAP6)) +
                 QQMULQD((pV32Share->cqRotateIQ.i + V32_SIGMAP2), (pV32Share->cqRotateIQ.i + V32_SIGMAP2));

        if ((qdTmp < qdTmp1) && (qdTmp < qdTmp2))
        {
            pV32Share->udTrainScramSReg = 0xabb37400;

            V32_RxTrainScramUsingGPC(pV32Share);

            pV32Share->cqSliceIQ.r = V32_SIGMAP6;
            pV32Share->cqSliceIQ.i = V32_SIGMAP2;

            V32_CarUpdateUseSliceIQ(pV32Share);

            pV32Share->nRxDelayCnt = 0;
        }
    }
    else if (pV32Share->nRxDelayCnt <= (V17_SEG1_DELAY - 300))
    {
        pV32Share->nRxDelayCnt = 0;
        /* TRACE0("Resync fail"); */
    }
}

/* output for segment 2 of long train */
void V17_RX_DataOut_Seg2L(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    /* Train scrambler starts to work when equalizer delay line already have about 8 cdcd... */
    if (pV32Share->nRxDelayCnt == V17_LONG_TRN_SEG2_DELAY - V17_EQ_DELAY)
    {
        pV32Share->pfTrainScramVec  = V32_RxTrainScramUsingGPC;
        pV32Share->pfTrainSigMapVec = V32_RxTrainSigMap;

        pV32Share->qEqBeta          = V17_EQ_TRAIN_BETA;
        pV32Share->pfEqUpdateVec    = V32_EqUpdate;
    }

    if (pV32Share->nRxDelayCnt == 500)
    {
        pV32Share->qEqBeta        = V17_EQ_FINE_BETA;

        pV32Share->pfCarrierVec   = V32_CarUpdateUseTrainIQ;

        pV32Share->pfTimUpdateVec = V32_TimingUpdateByTRN2;
    }

    /* The initialization of segment 3 is for long train */
    if (pV32Share->nRxDelayCnt == 1)
    {
        pV32Share->ubDiffDecodeState = 3;
    }
}

void V17_RX_DataOut_Seg2S_Seg3_Sub(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    V32_RX_Decode_init(pV32Share);

    pV32Share->ubRxBitsPerSym  = pV17->ubRxRateIdx + 3;
    pV32Share->ubSigDemapSize  = 16 << pV17->ubRxRateIdx;

    pV32Share->ubDiffDecodeTab = ubV32_DiffDecodeTab_T;
    pV32Share->ubSigTransMap   = V17_SigTransMapTable[pV17->ubRxRateIdx];

    switch (pV17->ubRxRateIdx)
    {
        case V17_RATE_IDX_14400:
            pV32Share->pcSigDemapTab    = cV32_IQTab_14400T;

            pV32Share->pnSigRotmapTab   = nV32_IQRotateTab_14400T;
            pV32Share->ubSigIQtoBitsTab = ubV32_IQtoBitsTab_14400T;
            break;
        case V17_RATE_IDX_12000:
            pV32Share->pcSigDemapTab    = cV32_IQTab_12000T;

            pV32Share->pnSigRotmapTab   = nV32_IQRotateTab_12000T;
            pV32Share->ubSigIQtoBitsTab = ubV32_IQtoBitsTab_12000T;
            break;
        case V17_RATE_IDX_9600:
            pV32Share->pcSigDemapTab    = cV32_IQTab_9600T;

            pV32Share->pnSigRotmapTab   = nV32_IQRotateTab_9600T;
            pV32Share->ubSigIQtoBitsTab = ubV32_IQtoBitsTab_9600T;
            break;
        case V17_RATE_IDX_7200:
            pV32Share->pcSigDemapTab    = cV32_IQTab_7200T;

            pV32Share->pnSigRotmapTab   = nV32_IQRotateTab_7200T;
            pV32Share->ubSigIQtoBitsTab = ubV32_IQtoBitsTab_7200T;
            break;
        default:
            break;
    }
}

/* output for segment 2 of short train */
void V17_RX_DataOut_Seg2S(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    if (pV32Share->nRxDelayCnt == 37)
    {
        pV32Share->qTimingErr2 = 0;
    }

    if ((pV32Share->nRxDelayCnt <= 35) && (pV32Share->nRxDelayCnt >= 30))
    {
        pV17->qdMinPhaseErr1 += QQMULQD(pV32Share->cqEqErrorIQ.r, pV32Share->cqEqErrorIQ.r);
        pV17->qdMinPhaseErr1 += QQMULQD(pV32Share->cqEqErrorIQ.i, pV32Share->cqEqErrorIQ.i);

        if (pV32Share->nRxDelayCnt == 30)
        {
            pV32Share->Poly.nTimingIdx += pV17->timingDelta;
        }
    }

    if ((pV32Share->nRxDelayCnt <= 12) && (pV32Share->nRxDelayCnt >= 7))
    {
        pV17->qdMinPhaseErr0 += QQMULQD(pV32Share->cqEqErrorIQ.r, pV32Share->cqEqErrorIQ.r);
        pV17->qdMinPhaseErr0 += QQMULQD(pV32Share->cqEqErrorIQ.i, pV32Share->cqEqErrorIQ.i);

        if (pV32Share->nRxDelayCnt == 7)
        {
            if (pV17->qdMinPhaseErr1 < pV17->qdMinPhaseErr0)
            {
                pV32Share->Poly.nTimingIdx -= pV17->timingDelta;
            }
        }
    }

    /* initialization of segment 4 is for short train */
    if (pV32Share->nRxDelayCnt == 1)
    {
        pV32Share->ubDiffDecodeState = 0;
        pV32Share->udDescramSReg = pV32Share->udTrainScramSReg;

        V17_RX_DataOut_Seg2S_Seg3_Sub(pV17);
    }
}


/* output for segment 3 */
void V17_RX_DataOut_Seg3(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    /* initialization of segment 4 */
    if (pV32Share->nRxDelayCnt == 1)
    {
        pV32Share->ubDiffDecodeState = 2;

        V17_RX_DataOut_Seg2S_Seg3_Sub(pV17);
    }
}


/* output for segment 4 */
void V17_RX_DataOut_Seg4(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    /* initialize TCF */
    if (pV32Share->nRxDelayCnt == 1)
    {
        pV32Share->ubRxBitsPerSym = pV17->ubRxRateIdx + 3;

        /* setup data mode for receiver */
        pV32Share->isRxDataMode = 1;
    }
}

/* output for TCF & Image Data */
void V17_RX_DataOut_DataMode(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    UBYTE *pIORxBuf = (UBYTE *)(pV17->pTable[DSPIOWRBUFDATA_IDX]);

    if (pV32Share->nRxDelayCnt <= 10)
    {
        pV32Share->nRxDelayCnt = 20000;
    }

    *pIORxBuf  = pV32Share->ubDescramOutbits;

#if 0

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV32Share->ubDescramOutbits; }

#endif

    IoWrProcess(pV17->pTable, pV32Share->ubRxBitsPerSym);
}
#endif
