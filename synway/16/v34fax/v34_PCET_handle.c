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
#include "mhsp.h"
#include "gaoapi.h"

#if SUPPORT_V34FAX

void V34Fax_PCET_Handler(UBYTE **pTable)
{
    SWORD *pPCMin  = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    MphStruc *pMphRx = &pV34Fax->MphRx;
    Phase3_Info *p3 = &(pV34->p3);
    Phase4_Info *p4 = &(pV34->p4);
    MpStruc *pMpTx = &p4->MpTx;
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
#if WITH_DC
    DCStruct *pDC = (DCStruct *)pTable[DC_STRUC_IDX];
#endif
#if DRAWEPG
    SWORD *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);
#endif
    CQWORD tDsym[3], TimingOut[4];
    UBYTE Num_Symbol, i, j, nbyte;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    V34_DPCS_Phase3(pTable);

    /* Number of symbols depends on sampling rate used */
    Num_Symbol = pV34->NumSymbol;

#if WITH_DC
    /* Low pass DC estimator filter */
    DcEstimator(pDC, pPCMin, UBUBMULUB(Num_Symbol, V34_SYM_SIZE), -11);
#endif

#if DRAWEPG
    *pEpg++ = Num_Symbol;
#endif

    pV34Tx->PCMinPtr  = pPCMin;
    pV34Tx->PCMoutPtr = pPCMout;

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        for (i = 0; i < Num_Symbol; i++)
        {
            if (pV34Fax->PCET_TXend_Flag == 1)
            {
                break;
            }

            p3->pfTx[p3->tx_vec_idx](pV34);

            pV34Tx->PCMinPtr  += V34_SYM_SIZE;
            pV34Tx->PCMoutPtr += V34_SYM_SIZE;
#if DRAWEPG
            *pEpg++ = pV34Tx->pcTx_sym[i].r;
            *pEpg++ = pV34Tx->pcTx_sym[i].i;
#endif
        }

        if (pV34Fax->PCET_TXend_Flag == 1)
        {
            for (; i < Num_Symbol; i++) /* left samples is silence */
            {
                for (j = 0; j < V34_SYM_SIZE; j++)
                {
                    pV34Tx->PCMoutPtr[j] = 0;
                }

                pV34Tx->PCMoutPtr += V34_SYM_SIZE;
            }

            TRACE0("V.34 MPE starts...");

            V34Fax_MPET_Init(pTable);
        }
    }
    else
    {
        for (i = 0; i < Num_Symbol; i++)
        {
            for (j = 0; j < V34_SYM_SIZE; j++)
            {
                pRx->qEchoCancel_Out[j] = pV34Tx->PCMinPtr[j];

                pV34Tx->PCMoutPtr[j] = 0;
            }

            V34_Demodulate(pRx, tDsym);

            nbyte = V34Fax_Timing_Rec(pV34Fax, tDsym, TimingOut);

            for (j = 0; j < nbyte; j++)
            {
                pV34Fax->Proc_Timing_Buf[pV34Fax->Proc_Timing_In++] = TimingOut[j];

                if (pV34Fax->Proc_Timing_In == 6)
                {
                    pV34Fax->Proc_Timing_In = 0;
                }
            }

            pV34Fax->Proc_Timing_Num += nbyte;

            while (pV34Fax->Proc_Timing_Num >= 3)
            {
                for (j = 0; j < 3; j++)
                {
                    pV34Rx->tTimingOut[j] = pV34Fax->Proc_Timing_Buf[pV34Fax->Proc_Timing_Out++];

                    if (pV34Fax->Proc_Timing_Out == 6)
                    {
                        pV34Fax->Proc_Timing_Out = 0;
                    }
                }

                pV34Fax->Proc_Timing_Num -= 3;
                p3->pfRx[p3->rx_vec_idx](pV34);
            }

            if (pV34Fax->Silence_Detect_Count > 0 && pRx->Retrain_flag == 0)
            {
                pV34Fax->Silence_Count1       = 0;
                pV34Fax->Silence_Count2       = 0;
                pV34Fax->Silence_Egy_Ref      = 0;
                pV34Fax->Silence_Egy          = 0;
                pV34Fax->Silence_Detect_Count = 0;

                p3->detect_symbol_counter = 0;

                pMphRx->max_data_rate = pMpTx->max_call_to_ans_rate;    /* from PCET */

                TRACE0("V.34 MPE starts...");

                V34Fax_MPET_Init(pTable);

                break;
            }

            pV34Tx->PCMinPtr  += V34_SYM_SIZE;
            pV34Tx->PCMoutPtr += V34_SYM_SIZE;

#if DRAWEPG
            *pEpg++ = pRx->cqEQ_symbol.r;
            *pEpg++ = pRx->cqEQ_symbol.i;
#endif
        }

        /* time out counter for different states  */
        pV34->Time_cnt++;
        /* Phase3 Timeout control */
        p3->TimeOut -= 4;
    }

    pV34Fax->timeout_count++;

    if (pV34Fax->timeout_count > V34FAX_PCET_TIMEOUT)
    {
        TRACE0("Time Out2");
        pV34Fax->timeout_count = 0;
        pAce->V34Fax_State = V34FAX_DISCONNECT;

        Disconnect_Init(pTable);
        pAce->pDceToT30->isDleEot = TRUE; TRACE0("V34: <DLE><EOT>");
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
    }
    else if (pV34Fax->Silence_Detect_Count > 0 && pRx->Retrain_flag)
    {
        V34F_Test_Retrain_State(pTable);
    }
}

#endif
