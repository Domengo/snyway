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

#include "v29ext.h"
#include "ptable.h"
#include "ioapi.h"
#include "mhsp.h"

void V29_Guard_or_ABtone_Detect(V29Struct *pV29)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV29->pTable[DPCS_STRUC_IDX]);
    DftStruct *pDft = &(pV29->Dft);
    QDWORD qdAvg_Energy;
    //UBYTE  ubTraintype;

    if (pV29->ubSagc_Flag == 0)
    {
        qdAvg_Energy = DSPD_DFT_Detect(pDft, pV29->qDemodIn, 3);

        if (qdAvg_Energy >= 0)
        {
            qdAvg_Energy >>= 1;

            if (pV29->SilenceCount < V29_SILENCE_LEN_REF)
            {
                if (qdAvg_Energy < V29_ENERGY_REF)
                {
                    pV29->SilenceCount ++;
                }
                else
                {
                    pV29->SilenceCount = 0;
                }
            }
            else if (qdAvg_Energy > V29_ENERGY_REF)
            {
                if (pV29->V21DetectDelayCnt < 100)
                {
                    pV29->V21DetectDelayCnt++;
                }

                if (pV29->qdMagSq[V29_TONE_500HZ] > qdAvg_Energy && (pV29->qdMagSq[V29_TONE_2900HZ] > (qdAvg_Energy >> 3)))
                {
                    pV29->ubGuardToneCnt = 0;

                    pV29->ubABtoneCnt ++;

                    if (pV29->ubABtoneCnt >= 2)
                    {
                        TRACE0("V29: AB Detected Directly");

                        //ubTraintype = RdReg(pDpcs->MFCF0, TCF_OR_IMAGE);

                        //if (ubTraintype == V29_IMAGE_DATA)
                        {
                            SetReg(pDpcs->MFCF0, CARRIER_GOT);
                        }

#if SUPPORT_ENERGY_REPORT
                        pV29->Detect_FaxTone = 1;
#endif

                        pV29->ubSagc_Flag = 1;
                        pV29->qdSagcEnergy = 0;

                        pV29->qdCorrThres = pV29->qdCorrelationQ >> 2;

                        pV29->nRxDelayCnt = 0;
                        pV29->nRxState++;
                    }
                }
                else if ((pV29->qdMagSq[V29_TONE_1700HZ] >> 2) > qdAvg_Energy && pV29->qdMagSq[V29_TONE_1700HZ] > pV29->qdMagSq[V29_V21TONE_1650HZ])
                {
                    pV29->ubABtoneCnt = 0;

                    pV29->ubGuardToneCnt ++;

                    if (pV29->ubGuardToneCnt > 2)
                    {
                        TRACE0("V29: Guard tone Detected first");

                        //ubTraintype = RdReg(pDpcs->MFCF0, TCF_OR_IMAGE);

                        //if (ubTraintype == V29_IMAGE_DATA)
                        {
                            SetReg(pDpcs->MFCF0, CARRIER_GOT);
                        }

                        pV29->nRxDelayCnt = 0;

                        DSPD_DFT_Init(pDft, 72, 2, (SWORD *)V29_DetectTone_Coef, pV29->qdMagSq, pV29->qQ1Q2, 4);
                    }
                }
                else
                {
                    if (pV29->V21DetectDelayCnt > 16/* V21 Tone detection. Delay 160ms to avoid V21 transmit echo */
                        && pV29->qdMagSq[V29_V21TONE_1650HZ] > qdAvg_Energy && pV29->qdMagSq[V29_V21TONE_1650HZ] > pV29->qdMagSq[0]
                        && pV29->qdMagSq[V29_V21TONE_1650HZ] > pV29->qdMagSq[1] && pV29->qdMagSq[V29_V21TONE_1650HZ] > pV29->qdMagSq[2])
                    {
                        pV29->V21DetectNumber++;
                    }
                    else
                    {
                        pV29->V21DetectNumber = 0;
                    }

                    pV29->ubGuardToneCnt = 0;
                    pV29->ubABtoneCnt = 0;
                }
            }
        }
    }
}

void V29_AB_Detect_after_Guardtone(V29Struct *pV29)
{
    DftStruct *pDft = &(pV29->Dft);
    QDWORD qdAvg_Energy;

    qdAvg_Energy = DSPD_DFT_Detect(pDft, pV29->qDemodIn, V29_SYM_SIZE);

    if (qdAvg_Energy > 0)
    {
        qdAvg_Energy >>= 1;

        if (qdAvg_Energy > V29_ENERGY_REF && pV29->qdMagSq[V29_TONE_500HZ] > qdAvg_Energy && pV29->qdMagSq[V29_TONE_2900HZ] > (qdAvg_Energy >> 3))
        {
            pV29->ubABtoneCnt ++;

            if (pV29->ubABtoneCnt >= 1)
            {
                DpcsStruct *pDpcs = (DpcsStruct *)(pV29->pTable[DPCS_STRUC_IDX]);

                SetReg(pDpcs->MFCF0, CARRIER_GOT);

                TRACE1("V29: RxDelayCnt: %d", pV29->nRxDelayCnt);
                TRACE0("V29: AB Detected after Guard tone");
#if SUPPORT_ENERGY_REPORT
                pV29->Detect_FaxTone = 1;
#endif
                pV29->qdCorrThres = pV29->qdCorrelationQ >> 2;

                pV29->ubSagc_Flag  = 1;
                pV29->qdSagcEnergy = 0;
                pV29->nRxDelayCnt  = 0;
            }
        }
    }
}

void V29_Receive_Segment2(V29Struct *pV29)
{
    if (pV29->ubSagc_Flag == 1 && pV29->nRxDelayCnt == (V29_GET_SEGMENT2_DELAY - V29_AGC_CALCULATION_POINT))/* use 64 symbol to calculate qdSagcEnergy */
    {
        V29_Sagc(pV29, 1626);
    }

    if (pV29->nRxDelayCnt == (V29_GET_SEGMENT2_DELAY - V29_AGC_CALCULATION_POINT - 10))
    {
        pV29->qdCorrThres = pV29->qdCorrelationQ >> 2;
    }

    if (pV29->ubSagc_Flag == 0 && pV29->qdCorrelationQ < pV29->qdCorrThres)
    {
        pV29->nRxDelayCnt = 1;
    }
}

void V29_Receive_Segment3(V29Struct *pV29)
{
    if (pV29->nRxDelayCnt == (V29_SEG3_384_SI >> 2))
    {
        pV29->qEqBeta = 14000;

        pV29->qCarAcoef = q097;
        pV29->qCarBcoef = q003;
    }

    if ((pV29->ubTraintype == V29_TCF_TRAIN  && pV29->nRxDelayCnt == (V29_SEG3_384_SI >> 1))
        || (pV29->ubTraintype == V29_IMAGE_DATA && pV29->nRxDelayCnt == V29_SEG3_384_SI >> 1))
    {
        pV29->pfTimUpdateVec = V29_RX_TimingUpdate;
        pV29->pfCarrierVec   = V29_CarrierUpdate;
        pV29->qEqBeta = 20000;
    }
}

void V29_Receive_DataMode(V29Struct *pV29)
{
    UBYTE *pIORxBuf = (UBYTE *)(pV29->pTable[DSPIOWRBUFDATA_IDX]);

    *pIORxBuf = pV29->ubDescramOutbits;

    IoWrProcess(pV29->pTable, pV29->ubBitsPerSym);

    if (pV29->nRxDelayCnt < 10)
    {
        pV29->nRxDelayCnt = V29_R_DATA_TIME_10000;
    }
}
