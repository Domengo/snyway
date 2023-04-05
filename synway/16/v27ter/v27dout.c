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

/************************************************************************/
/* This file defines all the vectors functions for receiver handshaking */
/* and data mode.                                                       */
/************************************************************************/

#include "v27ext.h"
#include "ioapi.h"
#include "ptable.h"
#include "mhsp.h"

void V27_Guard_or_ABtone_Detect(V27Struct *pV27)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pV27->pTable[DPCS_STRUC_IDX]);
    DftStruct *pDft = &(pV27->Dft);
    QDWORD qdAvg_Energy;
    //UBYTE  ubTraintype;

    if (pV27->ubSagc_Flag == 0)
    {
        qdAvg_Energy = DSPD_DFT_Detect(pDft, pV27->qDemodIn, pV27->ubSymBufSize);

        if (qdAvg_Energy >= 0)
        {
#if 0

            if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV27->qdMagSq[V27_TONE_CARRIER]; }

            if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = qdAvg_Energy; }

#endif

            if (pV27->SilenceCount < V27_SILENCE_LEN_REF)
            {
                if (qdAvg_Energy < V27_ENERGY_REF)
                {
                    pV27->SilenceCount ++;
                }
                else
                {
                    pV27->SilenceCount = 0;
                }
            }
            else if (qdAvg_Energy > V27_ENERGY_REF)
            {
#if !V27_HW7200SAMPLERATE_SUPPORT

                /* Because sample rate change from 7200 to 9600, the start time (32000--V27_7200CHANGE9600_CNT) don't count */
                if (pV27->nRxDelayCnt >= V27_7200CHANGE9600_CNT)
                {
                    return;
                }

#endif

                if (pV27->V21DetectDelayCnt < 100)
                {
                    pV27->V21DetectDelayCnt++;
                }

                if ((pV27->qdMagSq[V27_TONE_LOW] > qdAvg_Energy) && (pV27->qdMagSq[V27_TONE_HIGH] > qdAvg_Energy))
                {
                    pV27->ubGuardToneCnt = 0;
                    pV27->ubABtoneCnt ++;

                    if (pV27->ubABtoneCnt >= 2)
                    {
                        TRACE0("V27: AB Detected Directly");

                        //ubTraintype = RdReg(pDpcs->MFCF0, TCF_OR_IMAGE);

                        //if (ubTraintype == V27_IMAGE_DATA)
                        {
                            SetReg(pDpcs->MFCF0, CARRIER_GOT);
                        }

#if SUPPORT_ENERGY_REPORT
                        pV27->Detect_FaxTone = 1;
#endif
                        pV27->Timer = 0;

                        pV27->ubSagc_Flag = 1;
                        pV27->qdSagcEnergy = 0;

                        pV27->nRxDelayCnt = 0;
                        pV27->nRxState++;
                    }
                }
                else if (((pV27->qdMagSq[V27_TONE_CARRIER] >> 1) > qdAvg_Energy) && (pV27->qdMagSq[V27_TONE_CARRIER] > pV27->qdMagSq[V27_V21TONE_1650HZ]))
                {
                    pV27->ubABtoneCnt = 0;
                    pV27->ubGuardToneCnt ++;

                    if (pV27->ubGuardToneCnt > 2)/* fixed EPR#394 */
                    {
                        TRACE0("V27: Guard tone Detected first");

                        //ubTraintype = RdReg(pDpcs->MFCF0, TCF_OR_IMAGE);

                        //if (ubTraintype == V27_IMAGE_DATA)
                        {
                            SetReg(pDpcs->MFCF0, CARRIER_GOT);
                        }

                        pV27->Timer = 0;

                        pV27->nRxDelayCnt = 0;

                        if (pV27->ubSymBufSize == V27_SYM_SIZE_1600)
                        {
                            DSPD_DFT_Init(pDft, 48, 2, (QWORD *)V27_DetectTone_1600_Coef, pV27->qdMagSq, pV27->qQ1Q2, 4);
                        }
                        else
                        {
                            DSPD_DFT_Init(pDft, 48, 2, (QWORD *)V27_DetectTone_1200_Coef, pV27->qdMagSq, pV27->qQ1Q2, 4);
                        }
                    }
                }
                else
                {
                    if ((pV27->V21DetectDelayCnt > 10) && (pV27->qdMagSq[V27_V21TONE_1650HZ] > V27_ENERGY_REF) /* V21 Tone detection. Delay 100ms to avoid V21 transmit echo */
                        && pV27->qdMagSq[V27_V21TONE_1650HZ] > (qdAvg_Energy << 1) && pV27->qdMagSq[V27_V21TONE_1650HZ] > pV27->qdMagSq[0]
                        && pV27->qdMagSq[V27_V21TONE_1650HZ] > pV27->qdMagSq[1] && pV27->qdMagSq[V27_V21TONE_1650HZ] > pV27->qdMagSq[2])
                    {
                        pV27->V21DetectNumber++;
                    }
                    else
                    {
                        pV27->V21DetectNumber = 0;
                    }

                    pV27->ubGuardToneCnt = 0;
                    pV27->ubABtoneCnt    = 0;
                }
            }
        }
    }
}

void V27_AB_Detect_after_Guardtone(V27Struct *pV27)
{
    DftStruct *pDft = &(pV27->Dft);
    QDWORD    qdAvg_Energy;

    qdAvg_Energy = DSPD_DFT_Detect(pDft, pV27->qDemodIn, pV27->ubSymBufSize);

    if (qdAvg_Energy > V27_ENERGY_REF)
    {
        if ((pV27->qdMagSq[V27_TONE_LOW] > (qdAvg_Energy >> 1)) && (pV27->qdMagSq[V27_TONE_HIGH] > (qdAvg_Energy >> 3)))
        {
            pV27->ubABtoneCnt ++;

            if (pV27->ubABtoneCnt >= 1)
            {
                DpcsStruct *pDpcs = (DpcsStruct *)(pV27->pTable[DPCS_STRUC_IDX]);

                SetReg(pDpcs->MFCF0, CARRIER_GOT);

                TRACE1("V27: RxDelayCnt: %d", pV27->nRxDelayCnt);
                TRACE0("V27: AB Detected after Guard tone");

#if SUPPORT_ENERGY_REPORT
                pV27->Detect_FaxTone = 1;
#endif

                pV27->ubSagc_Flag  = 1;
                pV27->qdSagcEnergy = 0;
                pV27->nRxDelayCnt  = 0;
            }
        }
    }
}

/* Receive 180 phase reversal */
void V27_Receive_180_Phase(V27Struct *pV27)
{
    if (pV27->nRxDelayCnt == V27_R_SEG3_50_SI - V27_AGC_CALCULATION_POINT)
    {
        V27_Sagc(pV27, 2792);
    }

    if (pV27->qdCorrelation > V27_CORR_THRESHOLD && pV27->ubSagc_Flag == 0)
    {
        pV27->nRxDelayCnt = 0;
        TRACE2("V27: Corr: %d Thres : %d", pV27->qdCorrelation, V27_CORR_THRESHOLD);
        TRACE0("V27: V.27ter eq train.....");
    }
    else if (pV27->nRxDelayCnt < 100)
    {
        pV27->nRxDelayCnt = 32767;    /* Through ACE timing-out exit */
    }
}

/* Receive data */
void V27_Receive_Data(V27Struct *pV27)
{
    UBYTE *pIORxBuf = (UBYTE *)(pV27->pTable[DSPIOWRBUFDATA_IDX]);

    *pIORxBuf  = pV27->ubDescramOutbits;
    IoWrProcess(pV27->pTable, pV27->ubBitsPerSym);

#if 0

    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV27->ubDescramOutbits; }

#endif

    if (pV27->nRxDelayCnt < 10)
    {
        pV27->nRxDelayCnt = V27_R_DATA_TIME_10000;
    }
}
