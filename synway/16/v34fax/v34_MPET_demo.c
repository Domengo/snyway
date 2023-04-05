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

#include "v34fext.h"

#if SUPPORT_V34FAX

#define DUMP_PCM_IN        0        /* t3 */

void V34Fax_Gain(V34FaxStruct *pV34Fax)
{
    UBYTE i;
    QWORD qSagcScale;
    QWORD qTempR, qTempI;

    qSagcScale = pV34Fax->qGain;

    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        qTempR = pV34Fax->cDemodIQBuf[i].r;
        qTempI = pV34Fax->cDemodIQBuf[i].i;

        pV34Fax->cDemodIQBuf[i].r = QQMULQR8(qTempR, qSagcScale);
        pV34Fax->cDemodIQBuf[i].i = QQMULQR8(qTempI, qSagcScale);
    }
}


void V34Fax_Rx_BPF_Init(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        DspIIR_Cas51_Init(&pV34Fax->bqBpf, (QWORD *)V34Fax_HPF_Coeff, pV34Fax->dBpfDline, V34FAX_BIQUAD_NUM);
    }
    else
    {
        DspIIR_Cas51_Init(&pV34Fax->bqBpf, (QWORD *)V34Fax_LPF_Coeff, pV34Fax->dBpfDline, V34FAX_BIQUAD_NUM);
    }

    DspFirInit(&pV34Fax->bpfx, (QWORD *)V34Fax_Hilbert_Coeff, pV34Fax->dBpfDlineR, HILBERT_TAP_LEN);
}


void V34Fax_Rx_BPF(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34TxStruct  *pV34Tx  = &(pV34->V34Tx);
    QFIRStruct   *bpfx    = &(pV34Fax->bpfx);
    UBYTE i;

    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        pV34Tx->PCMinPtr[i] = DspIIR_Cas51(&pV34Fax->bqBpf, pV34Tx->PCMinPtr[i]);

#if DUMP_PCM_IN

        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV34Tx->PCMinPtr[i]; }

#endif
        pV34Fax->cDemodIQBuf[i].i = DspFir_Hilbert(bpfx, pV34Tx->PCMinPtr[i]);
        pV34Fax->cDemodIQBuf[i].r = bpfx->pDline[bpfx->nOffset + HILBERT_DELAY_LEN];
    }
}


void V34Fax_Demodulation_Init(V34FaxStruct *pV34Fax)
{
    QDWORD qdTemp;

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        if (pV34Fax->FreqOffset_Enable == 0)
        {
            pV34Fax->DeModCos = V34Fax_2400COS;

            pV34Fax->DeModSin = V34Fax_2400SIN;
        }
        else
        {
            pV34Fax->demodPhaseIncrement = 21845;
            /* Call modem does not receive line probing -- no frequency offset calculation */
        }
    }
    else
    {
        if (pV34Fax->FreqOffset_Enable == 0)
        {
            pV34Fax->DeModCos = V34Fax_1200COS;

            pV34Fax->DeModSin = V34Fax_1200SIN;
        }
        else
        {
            pV34Fax->demodPhaseIncrement = 10923;

#if FREQUENCY_OFFSET
            /* 18641 = (65536/7200Hz << 11): FREQUENCY_OFFSET_CONST_7200 */
            /* phase += (18641 >> 11) * (frequencyOffset >> 8) */
            qdTemp = ((QDWORD)pV34Fax->frequencyOffset * FREQUENCY_OFFSET_CONST_7200) >> 19;

            pV34Fax->demodPhaseIncrement += (UWORD)qdTemp;
#endif
        }
    }

    pV34Fax->ubOffset = 0;
}


void V34Fax_Demodulation(V34FaxStruct *pV34Fax)
{
    UBYTE i;
    QWORD c, s;
    QWORD tempR, tempI;
    QWORD qOffset;
    UWORD qIdxc, qIdxs;
    CQWORD *pcHead1, *pcHead2;

    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        if (pV34Fax->FreqOffset_Enable == 0)
        {
            /* find cos, sin values */
            c = pV34Fax->DeModCos[i];
            s = pV34Fax->DeModSin[i];
        }
        else
        {
            qIdxc = pV34Fax->demodPhase >> 8;

            qIdxs = (qIdxc - 64) & COS_SAMPLE_NUM; /* Step back Pi/2 for Sine value */

            qOffset = pV34Fax->demodPhase & 0xFF;

            c = DSP_tCOS_TABLE[qIdxc] + QQMULQ8(DSP_tCOS_OFFSET[qIdxc], qOffset);

            s = DSP_tCOS_TABLE[qIdxs] + QQMULQ8(DSP_tCOS_OFFSET[qIdxs], qOffset);

            pV34Fax->demodPhase += pV34Fax->demodPhaseIncrement;
        }

        tempR = (QWORD)((QQMULQD(pV34Fax->cDemodIQBuf[i].r, c) + QQMULQD(pV34Fax->cDemodIQBuf[i].i, s)) >> 15);
        tempI = (QWORD)((QQMULQD(pV34Fax->cDemodIQBuf[i].i, c) - QQMULQD(pV34Fax->cDemodIQBuf[i].r, s)) >> 15);

        pV34Fax->cDemodIQBuf[i].r = tempR;
        pV34Fax->cDemodIQBuf[i].i = tempI;

        if (pV34Fax->FreqOffset_Enable == 0)
        {
            if ((pV34Fax->PPS_detect_flag) && (pV34Fax->Dline_feed_flag == 0))
            {
#if 0

                if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = tempR; }

                if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = tempI; }

#endif

                if ((pV34Fax->max_R >= tempR) && (pV34Fax->max_R > 2000))
                {
                    pV34Fax->Dline_feed_flag = 1;

                    pV34Fax->PPS_detect_flag = 0;

                    pV34Fax->max_R = -30000;
                }
                else if (pV34Fax->max_R < tempR)
                {
                    pV34Fax->max_R = tempR;
                }

                if ((pV34Fax->min_R <= tempR) && (pV34Fax->min_R < -2000))
                {
                    pV34Fax->Dline_feed_flag = 1;

                    pV34Fax->PPS_detect_flag = 0;

                    pV34Fax->min_R = 30000;
                }
                else if (pV34Fax->min_R > tempR)
                {
                    pV34Fax->min_R = tempR;
                }

                if ((pV34Fax->max_I >= tempI) && (pV34Fax->max_I > 2000))
                {
                    pV34Fax->Dline_feed_flag = 1;

                    pV34Fax->PPS_detect_flag = 0;

                    pV34Fax->max_I = -30000;
                }
                else if (pV34Fax->max_I < tempI)
                {
                    pV34Fax->max_I = tempI;
                }

                if ((pV34Fax->min_I <= tempI) && (pV34Fax->min_I < -2000))
                {
                    pV34Fax->Dline_feed_flag = 1;

                    pV34Fax->PPS_detect_flag = 0;

                    pV34Fax->min_I = 30000;
                }
                else if (pV34Fax->min_I > tempI)
                {
                    pV34Fax->min_I = tempI;
                }
            }
        }

        if ((pV34Fax->FreqOffset_Enable == 1) || ((pV34Fax->FreqOffset_Enable == 0) && pV34Fax->Dline_feed_flag))
        {
            pcHead1 = pV34Fax->cTimDline + pV34Fax->ubOffset;
            pcHead2 = pV34Fax->cTimDline + pV34Fax->ubOffset + V34FAX_TIMING_DELAY_HALF;

            /* insert sample into timing delay line (double buffer) */
            (*pcHead1).r = tempR;
            (*pcHead1).i = tempI;

            (*pcHead2).r = tempR;
            (*pcHead2).i = tempI;

            pV34Fax->ubOffset ++;

            if (pV34Fax->ubOffset >= V34FAX_TIMING_DELAY_HALF)
            {
                pV34Fax->ubOffset = 0;
            }
        }
    }
}


void V34Fax_Carrier_Update_Init(V34FaxStruct *pV34Fax)
{
    pV34Fax->uRotatePhase = 0;
    pV34Fax->qCarErrPhase = 0;
}


void V34Fax_Carrier_Update(V34FaxStruct *pV34Fax)
{
    QDWORD cr_temp;
    QWORD qErrPhaseIn;

    cr_temp  = QQMULQD(pV34Fax->cqSliceIQ.r, pV34Fax->cqRotateIQ.i);
    cr_temp -= QQMULQD(pV34Fax->cqSliceIQ.i, pV34Fax->cqRotateIQ.r);

    qErrPhaseIn = QDR15Q(cr_temp);

    pV34Fax->qCarErrPhase += qErrPhaseIn;

    if (pV34Fax->FreqOffset_Enable == 0)
    {
        pV34Fax->uRotatePhase += ((qErrPhaseIn >> 2) + (pV34Fax->qCarErrPhase >> 6));
    }
    else
    {
        /* For large frequency offset, speed up carrier update for call modem */
        /* call modem does not measure frequency offset */
        pV34Fax->uRotatePhase += ((qErrPhaseIn >> 2) + (pV34Fax->qCarErrPhase >> 4));
    }

    SinCos_Lookup_Fine(pV34Fax->uRotatePhase, &pV34Fax->qRotateSin, &pV34Fax->qRotateCos);
}


void V34Fax_Rotate(V34FaxStruct *pV34Fax)
{
    QDWORD tempR, tempI;

    tempR  = QQMULQD(pV34Fax->cqEqOutIQ.r, pV34Fax->qRotateCos);
    tempR += QQMULQD(pV34Fax->cqEqOutIQ.i, pV34Fax->qRotateSin);

    tempI  = QQMULQD(pV34Fax->cqEqOutIQ.i, pV34Fax->qRotateCos);
    tempI -= QQMULQD(pV34Fax->cqEqOutIQ.r, pV34Fax->qRotateSin);

    pV34Fax->cqRotateIQ.r = QD15Q(tempR);
    pV34Fax->cqRotateIQ.i = QD15Q(tempI);
}


void V34Fax_Derotate(V34FaxStruct *pV34Fax)
{
    QDWORD tempR, tempI;

    pV34Fax->cqEqErrorIQ.r = pV34Fax->cqSliceIQ.r - pV34Fax->cqRotateIQ.r;
    pV34Fax->cqEqErrorIQ.i = pV34Fax->cqSliceIQ.i - pV34Fax->cqRotateIQ.i;

    tempR  = QQMULQD(pV34Fax->cqEqErrorIQ.r, pV34Fax->qRotateCos);
    tempR -= QQMULQD(pV34Fax->cqEqErrorIQ.i, pV34Fax->qRotateSin) ;

    tempI  = QQMULQD(pV34Fax->cqEqErrorIQ.i, pV34Fax->qRotateCos);
    tempI += QQMULQD(pV34Fax->cqEqErrorIQ.r, pV34Fax->qRotateSin) ;

    pV34Fax->cqEqErrorIQ.r = QDR15Q(tempR);
    pV34Fax->cqEqErrorIQ.i = QDR15Q(tempI);
}


void V34Fax_Decoder(V34FaxStruct *pV34Fax)
{
    UBYTE  ubDecode = 0, phase;
    UBYTE  ubCookedBits;

    if (pV34Fax->cqSliceIQ.r == 4096 && pV34Fax->cqSliceIQ.i == 4096)
    {
        //ubDecode = 0;
    }
    else if (pV34Fax->cqSliceIQ.r == 4096 && pV34Fax->cqSliceIQ.i == -4096)
    {
        ubDecode = 1;
    }
    else if (pV34Fax->cqSliceIQ.r == -4096 && pV34Fax->cqSliceIQ.i == -4096)
    {
        ubDecode = 2;
    }
    else if (pV34Fax->cqSliceIQ.r == -4096 && pV34Fax->cqSliceIQ.i == 4096)
    {
        ubDecode = 3;
    }
    else
    {
        TRACE0("ERROR Decoding!");
    }

    /* Differential decoding */
    phase = ubDecode & 0x3;

    phase = (phase - pV34Fax->ubDecoderOldPhase) & 0x3;

    ubCookedBits = (ubDecode & 0xC) | phase;

    pV34Fax->ubDescramInbits = ubCookedBits; /* !! type conversion !! */

    pV34Fax->ubDecoderOldPhase = (pV34Fax->ubDecoderOldPhase + ubCookedBits) & 0x3;
}


void V34Fax_Descramble(V34FaxStruct *pV34Fax)
{
    UBYTE  i, ubNumBits;
    UBYTE  ubInBits;
    UBYTE  in_stream[4];
    UBYTE  ubOutByte;

    ubNumBits = pV34Fax->ubRxBitsPerSym;
    ubInBits  = pV34Fax->ubDescramInbits;

    for (i = 0; i < ubNumBits; i++)
    {
        in_stream[i] = ubInBits & 1;

        ubInBits >>= 1;
    }

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        V34_Descram_Bit_GPC(in_stream, &pV34Fax->udDescramSReg, &ubOutByte, ubNumBits);
    }
    else
    {
        V34_Descram_Bit_GPA(in_stream, &pV34Fax->udDescramSReg, &ubOutByte, ubNumBits);
    }

    pV34Fax->ubDescramOutbits = ubOutByte;
}


void V34Fax_EqUpdate(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->pfRxSetUpVec == V34Fax_ALT_Rx_SetUp)
    {
        if ((pV34Fax->cqEqErrorIQ.r < 2000) && (pV34Fax->cqEqErrorIQ.r > -2000))
        {
            DspcFirLMS(&(pV34Fax->cfirEq), pV34Fax->qEqBeta, &(pV34Fax->cqEqErrorIQ));
        }
    }
    else
    {
        DspcFirLMS(&(pV34Fax->cfirEq), pV34Fax->qEqBeta, &(pV34Fax->cqEqErrorIQ));
    }
}


void V34Fax_EQ(V34FaxStruct *pV34Fax)
{
    QCFIRStruct *pcFir = &pV34Fax->cfirEq;
    CQWORD *pcHead;
    CQWORD *pcTail;
    UWORD Len;

    if (pV34Fax->FreqOffset_Enable == 0)
    {
        if (pV34Fax->Dline_feed_flag == 0)
        {
            return;
        }
    }

    Len = pcFir->nTapLen;

    pcHead = pcFir->pcDline + pcFir->nOffset;
    pcTail = pcHead + Len;

    *pcHead++ = pV34Fax->cTimBufIQ[0];
    *pcTail++ = pV34Fax->cTimBufIQ[0];

    pcFir->nOffset ++;

    if (pcFir->nOffset >= Len)
    {
        pcFir->nOffset = 0;
        pcHead = pcFir->pcDline;
    }

    pV34Fax->cqEqOutIQ = DspcFir(pcFir, &(pV34Fax->cTimBufIQ[1]));

    pV34Fax->cqEqOutIQ.r <<= 1;
    pV34Fax->cqEqOutIQ.i <<= 1;
}


void V34Fax_PPh_Slice(V34FaxStruct *pV34Fax)
{
    pV34Fax->cqSliceIQ = V34Fax_Create_PPh(pV34Fax->ubPPh_kref, pV34Fax->ubPPh_Iref);

    pV34Fax->ubPPh_Iref++;

    if (pV34Fax->ubPPh_Iref > 1)
    {
        pV34Fax->ubPPh_Iref = 0;
        pV34Fax->ubPPh_kref++;
    }
}


void V34Fax_Slice(V34FaxStruct *pV34Fax)
{
    QWORD x, y;
    QWORD temp_x, temp_y;
    QWORD qTemp, qMin;
    UBYTE i, minIdx;

    x = (pV34Fax->cqRotateIQ.r) >> 9;
    y = (pV34Fax->cqRotateIQ.i) >> 9;

    qMin = 32767;
    minIdx = 0;

    for (i = 0; i < 4; i++)
    {
        temp_x = V34FAX_MAP_TAB[i].r >> 9;
        temp_y = V34FAX_MAP_TAB[i].i >> 9;

        temp_x -= x;
        temp_y -= y;

        qTemp = (QWORD)(QQMULQD(temp_x, temp_x) + QQMULQD(temp_y, temp_y));

        if (qTemp < qMin)
        {
            qMin   = qTemp;
            minIdx = i;
        }
    }

    pV34Fax->cqSliceIQ = V34FAX_MAP_TAB[minIdx];
}


CQWORD V34Fax_Create_PPh(UBYTE k, UBYTE I)
{
    CQWORD cqC;
    UBYTE  qIdx;

    qIdx = (((UWORD)k * 2 * (k - I)) + 1) & 7;

    if (qIdx == 1)
    {
        cqC = V34FAX_MAP_TAB[0];
    }
    else if (qIdx == 3)
    {
        cqC = V34FAX_MAP_TAB[3];
    }
    else if (qIdx == 5)
    {
        cqC = V34FAX_MAP_TAB[2];
    }
    else
    {
        cqC = V34FAX_MAP_TAB[1];
    }

    return (cqC);
}

#endif
