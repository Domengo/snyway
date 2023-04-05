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

void  V34_S_Detector_Init(S_Tone_DetStruc *SToneDet)
{
    DSPD_DFTPure_Init(&(SToneDet->Stone), V34_SDETECT_NUMSAMPLES, V34_STONE_DELTAPHASE, V34_SDET_SHIFT);
    DSPD_DFTPure_Init(&(SToneDet->SRef), V34_SDETECT_NUMSAMPLES, V34_SREF_DELTAPHASE, V34_SDET_SHIFT);

    SToneDet->DetSCount = 0;

    SToneDet->SEgy.r = 0;
    SToneDet->SEgy.i = 0;

    SToneDet->SCarEgy.r = 0;
    SToneDet->SCarEgy.i = 0;
}

UBYTE  V34_S_Detector(ReceiveStruc *pRx, CQWORD *TimingOut)
{
    S_Tone_DetStruc *SToneDet = &(pRx->S_ToneDet);
    QDFTPUREStruct  *pStone   = &(SToneDet->Stone);
    QDFTPUREStruct  *pSRef    = &(SToneDet->SRef);
    QDWORD SEgy, SCarEgy, qdTemp, qdSEgyThres;
    QWORD  RCar = 0, ICar = 0;
    UBYTE det;
#if !SUPPORT_V90D
    TimeCarrRecovStruc  *pTCR = &pRx->TCR;
    QWORD  qR, qI;
#endif

    SEgy  = QQMULQD(TimingOut[0].r, TimingOut[0].r);
    SEgy += QQMULQD(TimingOut[1].r, TimingOut[1].r);
    SEgy += QQMULQD(TimingOut[2].r, TimingOut[2].r);
    SToneDet->SEgy.r += QDR15Q(SEgy);

    SEgy  = QQMULQD(TimingOut[0].i, TimingOut[0].i);
    SEgy += QQMULQD(TimingOut[1].i, TimingOut[1].i);
    SEgy += QQMULQD(TimingOut[2].i, TimingOut[2].i);
    SToneDet->SEgy.i += QDR15Q(SEgy);

    SCarEgy = (SDWORD)TimingOut[0].r + (SDWORD)TimingOut[1].r + (SDWORD)TimingOut[2].r;

    SToneDet->SCarEgy.r += (SCarEgy >> V34_SDET_SHIFT);

    SCarEgy = (SDWORD)TimingOut[0].i + (SDWORD)TimingOut[1].i + (SDWORD)TimingOut[2].i;

    SToneDet->SCarEgy.i += (SCarEgy >> V34_SDET_SHIFT);

    DSPD_DFTPure_Detect(pSRef, 3, TimingOut);

    if (DSPD_DFTPure_Detect(pStone, 3, TimingOut))
    {
        RCar = (QWORD)(SToneDet->SCarEgy.r >> V34_SCAR_SHIFT);
        ICar = (QWORD)(SToneDet->SCarEgy.i >> V34_SCAR_SHIFT);

        SCarEgy  = QQMULQR15(RCar, RCar);
        SCarEgy += QQMULQR15(ICar, ICar);

        SCarEgy <<= (V34_SCAR_SHIFT << 1);

        SEgy = SToneDet->SEgy.r + SToneDet->SEgy.i;

        SEgy >>= (V34_SDET_SHIFT << 1);

        SEgy = QDQMULQD0(SEgy, V34_SDETECT_NUMSAMPLES);

        SToneDet->SEgy.r = 0;
        SToneDet->SEgy.i = 0;

        SToneDet->SCarEgy.r = 0;
        SToneDet->SCarEgy.i = 0;

        qdTemp = QDQMULQD0((pStone->MagResult + SCarEgy), V34_STONE_FACTOR);

        det = 0;

        if (qdTemp > SEgy)
        {
            det |= 1;
        }

        qdTemp = QDQMULQD0(SCarEgy, V34_SCAR_FACTOR);

        if (qdTemp > SEgy)
        {
            det |= 2;
        }

        qdTemp = QQMULQD(pSRef->MagResult, 2);//V34_SREF_FACTOR);

        if (qdTemp < pStone->MagResult)
        {
            det |= 4;
        }

#if SUPPORT_V34FAX

        if (pRx->V34fax_Enable == 1)
        {
            qdSEgyThres = V34_SEGY_THRESHOLD_FAX;
        }
        else
#endif
            qdSEgyThres = V34_SEGY_THRESHOLD;

        if ((det == 7) && (SEgy > qdSEgyThres))
        {
            SToneDet->DetSCount++;
        }
        else
        {
            SToneDet->DetSCount = 0;
        }
    }

    if (SToneDet->DetSCount >= 6)//3
    {
#if !SUPPORT_V90D
        /*************  Rotate Carrier to 90 degrees *********/
        qdTemp = DSPD_Atan2(RCar, ICar);

        /* Result from arctan lookup is 0 =< angle < 2*Pi */
        qdTemp -= qD_90;

        if (qdTemp < 0)
        {
            qdTemp += qD_360;
        }

        qdTemp = QDQMULQDR(qdTemp, q1_OVER_2PI);

        /* Set carrier idx of demodulator */
        pRx->DeMod.rx_carrier_idx = (UBYTE)(((qdTemp >> 7) + pRx->DeMod.rx_carrier_idx) & 0xFF);
        /******************* End Carrier Jam ******************/

        /********** TimeJam ***********************************/
        if (((ICar > 0) && (RCar > 0)) || ((ICar < 0) && (RCar < 0)))
        {
            qR = pStone->IResult.r - pStone->QResult.r;
            qI = pStone->IResult.i - pStone->QResult.i;
        }
        else
        {
            qR = pStone->IResult.r + pStone->QResult.r;
            qI = pStone->IResult.i + pStone->QResult.i;
        }

        qdTemp = DSPD_Atan2(qR, qI);

        if (qdTemp > qD_90)
        {
            qdTemp -= qD_180;
        }

        pTCR->nTimingIdx += QDQMULQDR(qdTemp, V34_TIMJ_FACTOR); /* 128*3/Pi */

        while (pTCR->nTimingIdx >= V34_INTER_POINT)
        {
            pTCR->nTimingIdx -= V34_INTER_POINT;
        }

        while (pTCR->nTimingIdx < 0)
        {
            pTCR->nTimingIdx += V34_INTER_POINT;
        }

#endif

        /********** End TimeJam *******************************/

        V34_S_Detector_Init(SToneDet);

        return(1);
    }

    return(0);
}

UBYTE V34_Detect_S_SBAR(ReceiveStruc *pRx, CQWORD *pcTimingOut, SWORD *pDetect_symbol_counter)
{
    QCFIRStruct *pEqfir = &(pRx->TCR.eqfir);
    CQWORD *pcHead;
    CQWORD *pcTail;
    CQWORD pcS_EQ_out[3];
    CQWORD cqTrain_sym;
    CQWORD cqErrsym;
    UWORD  i, Len2;
    QWORD  qBeta;

    (*pDetect_symbol_counter) ++;

    Len2 = pEqfir->nTapLen << 1;

    pcHead = pEqfir->pcDline + pEqfir->nOffset;
    pcTail = pcHead + Len2;

    /***** Insert the same symbol into Actual EQ delayline *****/
    /* Save the 3 input samples to the delayline */
    *pcHead++ = *pcTimingOut;
    *pcTail++ = *pcTimingOut;

    *pcHead++ = *(pcTimingOut + 1);
    *pcTail++ = *(pcTimingOut + 1);

    *pcHead++ = *(pcTimingOut + 2);
    *pcTail++ = *(pcTimingOut + 2);

    pEqfir->nOffset += 3;

    if (pEqfir->nOffset >= Len2)
    {
        pEqfir->nOffset = 0;
        pcHead = pEqfir->pcDline;
    }

    /* Fill up half the delay line in EQ */
    for (i = 0; i < 3; i++)
    {
        pcS_EQ_out[i] = DspcFir(&pRx->S_ToneDet.EQ_S, &pcTimingOut[i]);
    }

    if (*pDetect_symbol_counter < 4)
    {
        pRx->S_trn_point = 0;
    }
    else  if (*pDetect_symbol_counter < 20)/* Do S signal training for 20 symbols */
    {
        cqTrain_sym = V34_Create_S_or_SBar(&(pRx->S_trn_point));
        CQSUB(cqTrain_sym, pcS_EQ_out[2], cqErrsym);

#if SUPPORT_V34FAX

        if (pRx->V34fax_Enable == 1)
        {
            qBeta = V34_qBETA;
        }
        else
#endif
            qBeta = 2949;

        DspcFirLMS(&(pRx->S_ToneDet.EQ_S), qBeta, &cqErrsym);
    }
    else if (
#if SUPPORT_V34FAX
        (pRx->V34fax_Enable == 0) &&
#endif
        (*pDetect_symbol_counter < 36))
    {
        V34_Symbol_To_Bits(pcS_EQ_out[2], &cqTrain_sym, V34_4_POINT_TRN);

        CQSUB(cqTrain_sym, pcS_EQ_out[2], cqErrsym);

        DspcFirLMS(&(pRx->S_ToneDet.EQ_S), 800, &cqErrsym);
    }
    else if (*pDetect_symbol_counter < 60)
    {
        /* Find out the exact symbol, only have 4 */
        V34_Symbol_To_Bits(pcS_EQ_out[2], &cqTrain_sym, V34_4_POINT_TRN);

        CQSUB(cqTrain_sym, pcS_EQ_out[2], cqErrsym);

#if SUPPORT_V34FAX

        if (pRx->V34fax_Enable == 1)
        {
            qBeta = V34_qBETA_S;
        }
        else
#endif
            qBeta = 350;

        DspcFirLMS(&(pRx->S_ToneDet.EQ_S), qBeta, &cqErrsym);
    }
    /* Freeze S_EQ updating and training timing/carrier loop  */
    /* Detect S to S' transition */
    else
    {
        V34_Symbol_To_Bits(pcS_EQ_out[2], &cqTrain_sym, V34_4_POINT_TRN);

        if (cqTrain_sym.i < 0)     /* Detect S to S' transition */
        {
            return(1);
        }
    }

#if DRAWEPG
    pRx->cqEQ_symbol = pcS_EQ_out[2];
#endif

    return(0);
}
