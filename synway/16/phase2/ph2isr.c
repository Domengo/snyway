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

#include "ph2ext.h"
#include "mhsp.h"
#include "faxshmem.h"

void Ph2_Handler(UBYTE **pTable)
{
    Ph2Struct   *pPH2 = (Ph2Struct *)(pTable[PH2_STRUC_IDX]);
    DPSKInfo   *pDpsk = &(pPH2->Dpsk);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    SWORD     *pPCMin = (SWORD *)pTable[PCMINDATA_IDX];
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
#if WITH_DC
    DCStruct   *pDC = (DCStruct *)(pTable[DC_STRUC_IDX]);
#endif
    SWORD qSample;
    UBYTE i;
    QWORD  qNoisePcmIn[PH2_SYM_SIZE];
#if SUPPORT_V34FAX
    FaxShareMemStruct *pFaxShareMem = (FaxShareMemStruct *)(pTable[FAXSHAREMEM_IDX]);

#if DRAWEPG
    SWORD *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
    pEpg++;
#endif
#endif

    pPH2->PCMinPtr  = (SWORD *)pTable[PCMINDATA_IDX];
    pPH2->PCMoutPtr = (SWORD *)pTable[PCMOUTDATA_IDX];

    Ph2_DPCS(pTable);    /* check & process DPCS */

    /* Transmitter process */
    pPH2->pfTx[pPH2->tx_vec_idx](pPH2);

    if (pPH2->LineProbOn == 0)
    {
        /* Bandpass all Echo tx signal except receiving Line Probing */
        for (i = 0; i < PH2_SYM_SIZE; i++)
        {
#if WITH_DC
            qSample = pPCMin[i] - (QWORD)(pDC->qdDcY >> DC_SHIFT_NUM);
#else
            qSample = pPCMin[i];
#endif
            qNoisePcmIn[i] = qSample;

#if 0

            if (DumpTone5_Idx < 100000) { DumpTone5[DumpTone5_Idx++] = qSample; }

#endif

            pPCMin[i] = DspFir(&(pDpsk->BPfir), qSample);

#if 0

            if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = pPCMin[i]; }

#endif
        }

        Ph2_Get_NoiseEgy(pPH2, qNoisePcmIn);
    }
    else
    {
        /* Dummy Bandpass */
        for (i = 0; i < PH2_SYM_SIZE; i++)
        {
#if WITH_DC
            qSample = pPCMin[i] - (QWORD)(pDC->qdDcY >> DC_SHIFT_NUM);
#else
            qSample = pPCMin[i];
#endif

            pPCMin[i] = qSample;
            qSample = DspFir(&(pDpsk->BPfir), qSample);
        }
    }

    /* AGC Gain is Q8.8 format */
    if (pPH2->L2_done != 2) /* L1/L2 echo not presence */ /* 2:L1/L2 sending, avoid AGC gain apply to echo */
    {
        for (i = 0; i < PH2_SYM_SIZE; i++)
        {
            pPCMin[i] = QQMULQR8(pPH2->AGC_gain, pPCMin[i]);
        }
    }

#if SUPPORT_V34FAX && DRAWEPG

    if (pPH2->V34fax_Enable == 1)
    {
        for (i = 0; i < PH2_SYM_SIZE; i += 2)
        {
            *pEpg++ = pPH2->epg_pos - 16383;

            *pEpg++ = pPCMin[i];

            pPH2->epg_pos = (pPH2->epg_pos + 1) & 0x7FFF;
        }
    }

#endif

#if 0

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
        if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = pPCMin[i]; }
    }

#endif

    pPH2->pfRx[pPH2->rx_vec_idx](pPH2);/* Receiver process */

    if (pPH2->Phase2_End)
    {
        if (pPH2->TimingDrift_Enable == 1)
        {
            /* If phase 1 result was not calculated, use phase 2 */
            if (pPH2->drift == 0)
            {
                pPH2->drift = Tim_drift_calc(pPH2->all_angles, PH2_ANGLE_NUM);

#if SUPPORT_V34FAX

                if (pPH2->V34fax_Enable == 1)
                {
                    pPH2->drift1 = Tim_drift_calc(pPH2->all_angles1, PH2_ANGLE_NUM);

                    if ((pPH2->drift1 < -17000000) || (pPH2->drift1 > 17000000))
                    {
                        pPH2->FreqOffset_Enable = 1;
                    }
                    else
                    {
                        pPH2->FreqOffset_Enable = 0;    /*from -0.4Hz to 0.4Hz*/
                    }

                    TRACE2("pPH2->drift=%" FORMAT_DWORD_PREFIX "d, pPH2->drift1=%" FORMAT_DWORD_PREFIX "d", pPH2->drift, pPH2->drift1);
                    TRACE1("pPH2->FreqOffset_Enable=%d", pPH2->FreqOffset_Enable);
                }

#endif
            }
        }

#if SUPPORT_V90

        if (pPH2->V90_Enable)
        {
#if SUPPORT_V90A

            if (pPH2->modem_mode == ANS_MODEM)
            {
                TRACE0("V90A Phase 3 starts");
#if SUPPORT_V92A
                pMhsp->pfRealTime = V92a_Init_Analog; /* Supports V.90 and V.92 initialization */
#else
                pMhsp->pfRealTime = V90a_Init_Analog;
#endif
            }

#endif /* SUPPORT_V90A */

#if SUPPORT_V90D

            if (pPH2->modem_mode == CALL_MODEM)
            {
                TRACE0("V90D Phase 3 starts");
                pMhsp->pfRealTime = V90d_Init_Digital;
            }

#endif
        }
        else
#endif
        {
#if SUPPORT_V34FAX

            if (pPH2->V34fax_Enable == 1)
            {
                pMhsp->pfRealTime = V34Fax_PCET_Init;
                TRACE0("V34Fax PCET starts...");

                pFaxShareMem->qFaxAgc = pPH2->AGC_gain;
            }
            else
#endif
            {
#if 0 /// LLL temp
                pMhsp->pfRealTime = V34_Phase3_Init;

                TRACE0("V34 Phase3 starts...");
#endif
            }
        }

        pMhsp->RtCallRate = 1;/* For Phase3 of V.34 or V.90a */
        pMhsp->RtBufSize  = 96;

        if (pPH2->RTD_Symbol <= 0)
        {
            pDpcs->RTD = 0;
        }
        else
        {
            pDpcs->RTD = QQMULQR8(pPH2->RTD_Symbol, PH2_RTD_CONST); /* pDpcs->RTD in msec, pPH2->RTD_Symbol in symbol(10/6ms) */
        }
    }

    if (pPH2->Retrain_flag)
    {
        pPH2->Ph2State.Retrain_Times ++;

        PutReg(pDpcs->DEBUGSR, (0x50 + pPH2->Ph2State.retrain_reason));

        PutReg(pDpcs->MSR0, RTRN_DET);

        pPH2->Retrain_flag = 0;

        TRACE0("Set retrain bit in Ph2");

        if (pPH2->modem_mode == CALL_MODEM)
        {
#if SUPPORT_V34FAX

            if (pPH2->V34fax_Enable == 1)
            {
                pPH2->tx_vec_idx = 9;
            }
            else
#endif
                pPH2->tx_vec_idx = 16;
        }
        else
        {
#if SUPPORT_V34FAX

            if (pPH2->V34fax_Enable == 1)
            {
                pPH2->tx_vec_idx = 10;
            }
            else
#endif
                pPH2->tx_vec_idx = 18;
        }

        pPH2->rx_vec_idx = 1;
    }

    /* individual state time out control */
    pPH2->Time_cnt++;

    /* Phase2 Timeout control */
    pPH2->Ph2Timer--;

    /* TRACE2("pPH2->tx_vec_idx = %d, pPH2->rx_vec_idx = %d", pPH2->tx_vec_idx, pPH2->rx_vec_idx); */
}


void Ph2_Get_NoiseEgy(Ph2Struct *pPH2, QWORD *qPcmIn)
{
    QDWORD qdSNR_Egy;
    UBYTE  i;

    if (pPH2->SNR_Beg_Count < 12)
    {
        pPH2->SNR_Beg_Count ++;

        pPH2->qdNoise_Egy = 300000000;
    }
    else if (pPH2->SNR_test_End == 0)
    {
        qdSNR_Egy = 0;

        for (i = 0; i < PH2_SYM_SIZE; i++)
        {
            qdSNR_Egy += QQMULQD(qPcmIn[i], qPcmIn[i]);
        }

        if (qdSNR_Egy > 300000000 || qdSNR_Egy < 0)
        {
            qdSNR_Egy = 300000000;
        }

        pPH2->qdSNR_Egy += (qdSNR_Egy >> 1);

        if (pPH2->Sym_Count < 6)
        {
            pPH2->Sym_Count ++;
        }
        else
        {
            pPH2->Sym_Count = 0;

            if (pPH2->qdNoise_Egy > pPH2->qdSNR_Egy)
            {
                pPH2->qdNoise_Egy = pPH2->qdSNR_Egy;
            }

            pPH2->qdSNR_Egy = 0;
        }
    }
}
