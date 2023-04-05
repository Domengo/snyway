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
#include <string.h>
#include "v8ext.h"
#include "ptable.h"
#include "gaoapi.h"
#include "aceext.h"
#include "v251.h"
#if SUPPORT_V92A
#include "sregdef.h"
#endif

#if !SUPPORT_T38EP
void V8_V21_Init(V21Struct *pV21, UBYTE Modemmode)
{
    if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
    {
#if V21_7200
        DspFirInit(&(pV21->BpfHigh), (SWORD *)V21_tHFilter_Coef_72, pV21->pqDelaylineH, V21_FILTER_LENGTH_72);

        DspFirInit(&(pV21->BpfLow), (SWORD *)V21_tLFilter_Coef_72, pV21->pqDelaylineL, V21_FILTER_LENGTH_72);

        DspFirInit(&(pV21->LpFilter), (SWORD *)V21_tLpcoef_72, pV21->pqDelayline, V21_LOWPASS_FILTER_LENGTH_72);
#endif
    }
    else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
    {
#if V21_9600
        DspFirInit(&(pV21->BpfHigh), (SWORD *)V21_tHFilter_Coef_96, pV21->pqDelaylineH, V21_FILTER_LENGTH_96);

        DspFirInit(&(pV21->BpfLow), (SWORD *)V21_tLFilter_Coef_96, pV21->pqDelaylineL, V21_FILTER_LENGTH_96);

        DspFirInit(&(pV21->LpFilter), (SWORD *)V21_tLpcoef_96, pV21->pqDelayline, V21_LOWPASS_FILTER_LENGTH_96);
#endif
    }

    if (Modemmode == CALL_MODEM) /* 1 */
    {
        pV21->BpfTxPtr = &(pV21->BpfLow);
    }

#if USE_ANS
    else
    {
        pV21->BpfTxPtr = &(pV21->BpfHigh);

        if (pV21->ubSampleRate == V21_SAMPLE_RATE_7200)
        {
#if V21_7200
            DspIIR_Cas51_Init(&(pV21->BpfToneIIR_72), (QWORD *)V21_tAnsTone_FilterCoef_72, pV21->pqDelaylineT_72, V21_BIQUAD_NUM);
#endif
        }
        else if (pV21->ubSampleRate == V21_SAMPLE_RATE_9600)
        {
#if V21_9600
            DspFirInit(&(pV21->BpfToneIIR_96), (QWORD *)V21_tAnsTone_FilterCoef_96, pV21->pqDelaylineT_96, V21_FILTER_LENGTH_96);
#endif
        }
    }

#endif

    pV21->qAgc_gain = 256;
    pV21->State     = 1;
    pV21->TxBits    = 1;
#if 0
    pV21->RxNumBits    = 0;
#endif
}
#endif

void V8_Init(UBYTE **pTable)
{
    V8Struct   *pV8   = (V8Struct *)(pTable[V8_STRUC_IDX]);
    V21Struct  *pV21  = &(pV8->v21);
#if SUPPORT_CT
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
#endif
#if SUPPORT_V92D
    UBYTE i;
#endif

    pV21->pTable = pTable;

#if SUPPORT_V92D
#if SUPPORT_V92A

    if (ModemAorB == 1)
#endif
    {
        pV8->V92_Enable    = 0;
    }

    pV8->QCA1d_Finish  = 0;
    pV8->ANSpcm_Finish = 0;
    pV8->curPhase_980  = 0;
    pV8->TONEq_Counter = 0;
#endif

#if SUPPORT_V92A

    if (pV8->V8bisEnable)
    {
        V8_V21_Init(pV21, ANS_MODEM);
    }
    else
#endif
    {
        V8_V21_Init(pV21, pV8->modem_mode);
    }

    pV8->tx_vec_idx    = 0;
    pV8->rx_vec_idx    = 0;

    pV8->RxBits_BufferIn_idx  = 0;
    pV8->RxBits_BufferOut_idx = (V8_V21_RXBUFSIZE >> 1) - 1;

    if (pV8->modem_mode == CALL_MODEM)
    {
#if SUPPORT_V92A

        if ((pV8->V8bisEnable) && (pV8->QCEnabled))
        {
            pV21->pfTxVec = V21_Ch2Tx;
            pV21->pfRxVec = V21_Ch1Rx;
        }
        else
#endif
        {
            pV21->pfTxVec = V21_Ch1Tx;
            pV21->pfRxVec = V21_Ch2Rx;
        }
    }

#if USE_ANS
    else
    {
        pV21->pfTxVec = V21_Ch2Tx;

        pV21->pfRxVec = V21_Ch1Rx;

        pV8->ANSam_timer = pV8->AnsamTime_Len;
    }

#endif

#if SUPPORT_CT
    flag = RdReg(pDpcs->MCR0, V25AT);     /* Read control for Call tone gen */
    V8_DCE_Reset(pV8, flag);
#else
    V8_DCE_Reset(pV8);
#endif

    /* MNP | V42bis | V42 */
    pV8->CM_Octets[66] = 0;
    pV8->ANS15_Counter = 0;
    pV8->ANSam_Egy       = 0;
    pV8->JM_detected   = 0;

    DspFirInit(&(pV8->V8AnsamToneH), (QWORD *)V8_AnsamTone_FIR_Coef,  pV8->pqDelaylineH, V8_ANSAM_FILTER_LENGTH);

#if V8_HIGHPASS_FILTER_ENABLE
    DspIIR_Cas51_Init(&pV8->Hpf, (QWORD *)V8_HpfCoef, pV8->HpfDline, 2);//JULY17
#endif
}


#if SUPPORT_CT
void  V8_DCE_Reset(V8Struct *pV8, UBYTE CTflag)
#else
void  V8_DCE_Reset(V8Struct *pV8)
#endif
{
    DpcsStruct *pDpcs = (DpcsStruct *)pV8->v21.pTable[DPCS_STRUC_IDX];
    UBYTE i, j;

    for (i = 0; i < V8_CM_LENGTH2; i++)
    {
        pV8->CM_Octets[i] = V8_CM_SETUP[i];
    }

#if SUPPORT_V92D

    for (i = 0; i < V8_QCA1d_LENGTH; i++)
    {
        pV8->V8_QCA1d_Octets[i]    = 0;
    }

#endif

#if SUPPORT_V34FAX

    if (pV8->V34fax_Enable == 1)
    {
        pV8->CM_Octets[27] = 0; /* No Data modem. Refer as Table 3/V.8 */
        pV8->CM_Octets[38] = 1; /* V.34 half-duplex availity. Refer as Table 4/V.8 */
    }

#if SUPPORT_V34
    else
#endif
#endif
#if SUPPORT_V34
        pV8->CM_Octets[37] = 1;

#endif

#if (SUPPORT_V32BIS + SUPPORT_V32)
    pV8->CM_Octets[41] = 1;
#endif

#if SUPPORT_V22_OR_B212
    pV8->CM_Octets[42] = 1;
#endif

#if SUPPORT_V21M
    pV8->CM_Octets[58] = 1;
#endif

#if SUPPORT_V42
    pV8->CM_Octets[66] = 1;
#endif

#if SUPPORT_V90

    if (pV8->V90_Enable == 1)
    {
        pV8->ubCMLen = V8_CM_LENGTH2;

        pV8->CM_Octets[36] = 1; /* PCM Modem availability category present */

#if SUPPORT_V90A
        pV8->CM_Octets[86] = 1; /* V.90a or V.92a available */
#endif

#if SUPPORT_V90D
        pV8->CM_Octets[78] = 1; /* digital network connection present */
        pV8->CM_Octets[87] = 1; /* V.90d or V.92d available */
#endif
    }
    else
#endif
        pV8->ubCMLen = V8_CM_LENGTH1;

    if (pV8->modem_mode == CALL_MODEM)
    {
        pV8->DCE_timer = 1500;
        i = j = 0;

        /* initialize the vector for transmit */
#if SUPPORT_V92A

        if (pV8->QCEnabled)
        {
#if SUPPORT_CT

            if (CTflag)
            {
                pV8->pfTx[i++] = V8_Send_CallTone;
            }
            else
#endif
                pV8->pfTx[i++] = V8_Send_Silence; /* Tx: State 0  */

            /* Quick Connect */
            pV8->pfTx[i++] = V8_Send_Te;                /* Tx: State 1    */

            if (pV8->V8bisEnable)
            {
                pV8->pfTx[i++] = V8_Send_QC2a;        /* Tx: State 2    */
                pV8->pfTx[i++] = V8_Send_None;        /* Tx: State 4    */
                pV8->pfTx[i++] = V8_Send_TONEq;        /* Tx: State 5    */
                pV8->pfTx[i++] = V8_Send_TONEq_End;    /* Tx: State 6    */

                pV8->pfRx[j++] = V8_Detect_CRe;        /* Rx: State 0  */
                pV8->pfRx[j++] = V8_Dummy;            /* Rx: State 1    */
                pV8->pfRx[j++] = V8_Detect_QCA2d;        /* Rx: State 2    */
            }
            else
            {
                pV8->pfTx[i++] = V8_Send_QC1a;        /* Tx: State 2    */
                pV8->pfTx[i++] = V8_Send_CM;            /* Tx: State 3    */
                pV8->pfTx[i++] = V8_Send_None;        /* Tx: State 4    */
                pV8->pfTx[i++] = V8_Send_TONEq;        /* Tx: State 5    */
                pV8->pfTx[i++] = V8_Send_TONEq_End;    /* Tx: State 6    */

                pV8->pfRx[j++] = V8_Detect_ANSam;        /* Rx: State 0    */
                pV8->pfRx[j++] = V8_Dummy;            /* Rx: State 1    */
                pV8->pfRx[j++] = V8_Detect_QCA1d;        /* Rx: State 2    */
            }

            pV8->pfRx[j++] = V8_Detect_None;            /* Rx: State 3    */
            pV8->pfRx[j++] = V8_Detect_QTS;            /* Rx: State 4    */
            pV8->pfRx[j++] = V8_Detect_QTS_bar;        /* Rx: State 5    */
            pV8->pfRx[j++] = V8_Detect_ANSpcm;        /* Rx: State 6    */
            pV8->pfRx[j++] = V8_Detect_End_ANSPcm;    /* Rx: State 6    */
            pV8->pfRx[j++] = V8_Dummy;                /* Rx: State 7    */
        }
        else
#endif
        {
            /* No Quick Connect */
            switch (pDpcs->V8OPCNTL & A8E_V8O)
            {
                case A8E_V8O_1:
                case A8E_V8O_6:
                    pV8->isToSendMenu = TRUE;
                    pV8->pfTx[i++] = V8_Send_Silence;
                    pV8->pfRx[j++] = V8_Detect_ANSam;
                    break;
                case A8E_V8O_3:
                    pV8->pfTx[i++] = V8_Send_Silence;
                    pV8->pfRx[j++] = V8_Detect_ANSam;
                    break;
#if SUPPORT_CI
                case A8E_V8O_2:
                    pV8->pfTx[i++] = V8_Send_CI;
                    pV8->pfRx[j++] = V8_Detect_ANSam;
                    pV8->CI_Octets[0] = 0xff;
                    pV8->CI_Octets[1] = 0xc0;
                    pV8->CI_Octets[2] = 0x10 | ((pDpcs->V8CF >> 5) & 0x7);
                    pV8->CI_Octets[3] = ((pDpcs->V8CF << 3) & 0xFF) | 0x04;
                    break;
#endif
#if SUPPORT_CT
                case A8E_V8O_4:
                    pV8->pfTx[i++] = V8_Send_CallTone;
                    break;
#endif
                case A8E_V8O_5:
                    // send no tones
                    pV8->DCE_timer = V8_TE;
                    break;
                default:
                    TRACE1("V8: ERROR. Invalid operation control %d", pDpcs->V8OPCNTL);
                    break;
            }

            pV8->pfTx[i++] = V8_Send_Te;
            pV8->pfTx[i++] = V8_Send_CM;
            pV8->pfTx[i++] = V8_Send_CJ;

            pV8->pfRx[j++] = V8_Dummy;
            pV8->pfRx[j++] = V8_Detect_JM;
            pV8->pfRx[j++] = V8_Dummy;
        }
    }

#if USE_ANS
    else if (pV8->modem_mode == ANS_MODEM)
    {
        i = j = 0;

        switch (pDpcs->V8OPCNTL & A8E_V8A)
        {
            case A8E_V8A_1:
            case A8E_V8A_5:
                pV8->isToSendMenu = TRUE;
#if !SUPPORT_T38GW
                pV8->DCE_timer = 600;
                pV8->pfTx[i++] = V8_ANS_Send_Silence;
                pV8->pfRx[j++] = V8_Dummy;
#endif
                pV8->pfTx[i++] = V8_Send_ANSam;
                break;
            case A8E_V8A_2:
#if !SUPPORT_T38GW
                pV8->DCE_timer = 600;
                pV8->pfTx[i++] = V8_ANS_Send_Silence;
                pV8->pfRx[j++] = V8_Dummy;
#endif
                pV8->pfTx[i++] = V8_Send_ANSam;
                break;
            case A8E_V8A_3:
                pV8->pfTx[i++] = V8_Dummy;
                break;
            default:
                TRACE1("V8: ERROR. Invalid operation control %d", pDpcs->V8OPCNTL);
                break;
        }

        pV8->pfTx[i++] = V8_Send_JM;

        pV8->pfRx[j++] = V8_Detect_CM;
        pV8->pfRx[j++] = V8_Detect_CJ;
    }

#endif

    pV8->pRecv_string1[0] = 1;
    pV8->pRecv_string1[1] = 0x7E;
    pV8->pRecv_string2[1] = 0x7E;
    //    pV8->pRecv_string3[1] = 0x7E;

#if SUPPORT_V92A

    if (pV8->V8bisEnable)
    {
        for (i = 0; i < V8_QC2A_LENGTH; i++)
        {
            pV8->V8_QC1a_Octets[i] = V8_QC2a_SETUP92[i];
        }

        pV8->ubQC1aLen = V8_QC2A_LENGTH;
    }
    else
    {
        for (i = 0; i < V8_QC1A_LENGTH; i++)
        {
            pV8->V8_QC1a_Octets[i] = V8_QC1a_SETUP92[i];
        }

        pV8->ubQC1aLen = V8_QC1A_LENGTH;
    }

    pV8->QCA1d_detected        = 0;
    pV8->cur_send_bit_index    = 0;
    pV8->QCA1d_ONEs_Sync    = 0;
    pV8->TONEq_finish        = 0;
#endif
}

void V8_SetResponse(UBYTE **pTable, UBYTE ubResponseID, UBYTE *pResponseData, UBYTE ubLen)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    DceToT30Interface *pDceToT30 = pAce->pDceToT30;
    V8Struct *pV8 = (V8Struct *)(pTable[V8_STRUC_IDX]);

    // fixing issue 15
    //if (pDpcs->V8OPCNTL == (A8E_V8O_1 | A8E_V8A_1))
    //{
    //    switch (ubResponseID)
    //    {
    //    case V8_A8M:
    //    case V8_OK:
    //        Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
    //        break;
    //    }
    //}
    //else

    // fixing bug 38, treat V8O and V8A separately
    if ((pV8->modem_mode == CALL_MODEM && (pDpcs->V8OPCNTL & A8E_V8O) != A8E_V8O_1) ||
        (pV8->modem_mode != CALL_MODEM && (pDpcs->V8OPCNTL & A8E_V8A) != A8E_V8A_1))
    {
        pDceToT30->v8IndicationType = ubResponseID;

        switch (ubResponseID)
        {
            case V8_A8A:
                TRACE1("V8: +A8A:%d", pResponseData[0]);
                pDceToT30->v8IndicationData[0] = pResponseData[0];
                pDceToT30->v8IndicationLen = 1;
                break;
            case V8_A8J:
                TRACE1("V8: +A8J:%d", pResponseData[0]);
                pDceToT30->v8IndicationData[0] = pResponseData[0];
                pDceToT30->v8IndicationLen = 1;
                break;
            case V8_A8C:
                TRACE1("V8: +A8C:%d", pResponseData[0]);
                pDceToT30->v8IndicationData[0] = pResponseData[0];
                pDceToT30->v8IndicationLen = 1;
                break;
            case V8_A8I:
                TRACE1("V8: +A8I:%02x", pResponseData[0]);
                pDceToT30->v8IndicationData[0] = pResponseData[0];
                pDceToT30->v8IndicationLen = 1;
                break;
            case V8_A8M:
                TRACE("%s", "V8: +A8M:");
                {
                    int i;

                    for (i = 0; i < ubLen; i ++)
                    {
                        TRACE("%02x", pResponseData[i]);
                    }
                }
                TRACE0("");
                memcpy(pDceToT30->v8IndicationData, pResponseData, ubLen);
                pDceToT30->v8IndicationLen = ubLen;

                if ((pDpcs->V8OPCNTL & A8E_V8O) != A8E_V8O_6 && (pDpcs->V8OPCNTL & A8E_V8A) != A8E_V8A_5)
                {
                    Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
                }

                break;
            case V8_OK:
                pDceToT30->v8IndicationType = V8_NULL;

                if ((pDpcs->V8OPCNTL & A8E_V8O) != A8E_V8O_6 && (pDpcs->V8OPCNTL & A8E_V8A) != A8E_V8A_5) // fixing issue 15
                {
                    Ace_SetClass1ResultCode(pAce, RESULT_CODE_OK);
                }

                break;
        }
    }
}

