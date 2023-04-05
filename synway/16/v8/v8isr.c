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

#include "v8ext.h"
#include "mhsp.h"
#include "ptable.h"
#include "gaoapi.h"
#if SUPPORT_V34FAX
#include "aceext.h"
#endif

void V8_Handler(UBYTE **pTable)
{
    V8Struct   *pV8     = (V8Struct *)pTable[V8_STRUC_IDX];
    MhspStruct *pMhsp   = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    SWORD      *pPCMin  = (SWORD *)pTable[PCMINDATA_IDX];
    SWORD      *pPCMout = (SWORD *)pTable[PCMOUTDATA_IDX];
    DpcsStruct *pDpcs   = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    V21Struct   *pV21 = &(pV8->v21);
#if WITH_DC
    DCStruct     *pDC = (DCStruct *) pTable[DC_STRUC_IDX];
#endif
    UBYTE j;
#if SUPPORT_V34FAX
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isV34Enabled = *pAce->pT30ToDce->pIsV34Enabled;
#endif

#if SUPPORT_V92A
    UBYTE shortPhase1Flag = 0;
#endif


#if DRAWEPG
    UBYTE i = 0;
    SWORD *pEpg = (SWORD *)(pTable[EPGDATA_IDX]);

    pEpg++;
#endif

    for (j = 0; j < pV8->call_rate; j++)
    {
        pV21->PCMinPtr  = pPCMin;
        pV21->PCMoutPtr = pPCMout;

#if SUPPORT_V92A

        if (pV8->QCA1d_detected == 2)
        {
            shortPhase1Flag = 1;
            pV8->QCA1d_detected = 0;
        }

#endif

        pV8->DCE_timer--;
        V8_DPCS_Phase1(pTable);
        pV8->pfTx[pV8->tx_vec_idx](pV8);

        pPCMout += pV21->ubBufferSize;

#if WITH_DC

        if (pDC->DC_Count > 0)
        {
            pDC->DC_Count ++;

            if (pDC->DC_Count > 360)    /* (100 + 20) * 3 */
            {
                pDC->DC_Count = 0;
                pDC->DC_Start = 1;
            }
        }

        if (pDC->DC_Start == 1)
        {
            DcEstimator(pDC, pPCMin, pV21->ubBufferSize, -7);    /* Estimate DC level, and remove DC from input PCM stream */
        }

#if V8_HIGHPASS_FILTER_ENABLE

        for (i = 0; i < pV21->ubBufferSize; i++)
        {
            pPCMin[i] = DspIIR_Cas51(&pV8->Hpf, pPCMin[i]);
        }

#endif
#endif

#if 0

        for (i = 0; i < pV21->ubBufferSize; i++)
        {
            if (DumpTone3_Idx < 200000) { DumpTone3[DumpTone3_Idx++] =  pPCMin[i]; }
        }

#endif

#if DRAWEPG

        for (i = 0; i < pV21->ubBufferSize; i += 4)
        {
            *pEpg++ = pV8->epg_pos - 16383;

            *pEpg++ = pPCMin[i];

            pV8->epg_pos = (pV8->epg_pos + 1) & 0x7FFF;
        }

#endif

        pV8->pfRx[pV8->rx_vec_idx](pV8);
        pPCMin += pV21->ubBufferSize;

#if SUPPORT_V34FAX

        if (pV8->V34fax_Enable == 0 && isV34Enabled)
        {
            pAce->pDceToT30->isV34Selected = FALSE;

            V8_Disconnect(pTable);

#if SUPPORT_V21CH2FAX
            pAce->V8Detect_State = V8_PASS_V21;

            if (pV8->modem_mode == ANS_MODEM)
            {
                Ace_ChangeState(pAce, ACE_FTH);
            }
            else
            {
                Ace_ChangeState(pAce, ACE_FRH);
            }

            PutReg(pDpcs->MMR0, V21CH2);

            pMhsp->RtCallRate = 1;/* For V.21 */
            pMhsp->RtBufSize  = 72;
#else
            Ace_ChangeLineState(pAce, GO_V17FAX);
#endif
            break;
        }

#endif

        if (pV8->AnsAm_Detected == 1)
        {
            SetReg(pDpcs->MTONESR, MTONE6_V8DET);

            pV8->AnsAm_Detected = 0;
        }

        if (pV21->Detect_tone == 1)    /* if (pV8->CM_Detected == 1) */
        {
            SetReg(pDpcs->MTONESR, MTONE6_V8DET);
            /* pV8->CM_Detected = 0; */
        }

#if SUPPORT_V92A

        if (pV8->TONEq_finish)
        {
            pV8->phase1_end = 1;
        }

#endif

        if (pV8->phase1_end == 1)
        {
#if SUPPORT_V34FAX
            pAce->V8Detect_State = V8_PASS_V8;
#endif

            if (
#if (SUPPORT_V90)
                pV8->V90_Enable    ||
#endif
#if SUPPORT_V34FAX
                pV8->V34fax_Enable ||
#endif
                pV8->V34_Duplex)
            {
#if SUPPORT_V34FAX
                pAce->pDceToT30->isV34Selected = TRUE;
#endif
                /* V.34 Phase 2 entry point */
                pMhsp->pfRealTime = Phase2_DPCS_Init;
                pMhsp->RtCallRate = MODEM_CALL_16_96;/* For Phase 2 */
                pMhsp->RtBufSize  = MODEM_BUFSIZE_16;

#if SUPPORT_V90
                TRACE1("V8: pV8->V90_Enable=%d", pV8->V90_Enable);
#endif
            }
            else
            {
                pMhsp->RtCallRate = 1;/* For V.32, V.22 or V.21 */
                pMhsp->RtBufSize  = 72;

#if (SUPPORT_V32 + SUPPORT_V32BIS)

                if ((pDpcs->MMR0 == AUTOMODE) && pV8->V32_Enable)
                {
                    pMhsp->pfRealTime = V32_DPCS_Init;
                }
                else
#endif
#if SUPPORT_V22_OR_B212
                    if ((pDpcs->MMR0 == AUTOMODE) && pV8->V22_Enable)
                    {
                        pMhsp->pfRealTime = V22_DPCS_Init;
                    }
                    else
#endif
#if SUPPORT_V21M
                        if ((pDpcs->MMR0 == AUTOMODE) && pV8->V21_Enable)
                        {
                            pMhsp->pfRealTime = V21_DPCS_Init;
                        }
                        else
#endif
                        {
                            V8_Disconnect(pTable);
                            TRACE0("V8: No supported modem");
                            PutReg(pDpcs->DEBUGSR, (0x42));
                        }
            }

            break;
        }
    }

#if SUPPORT_V92A

    if (shortPhase1Flag == 1) /* Proceed to SHORT Phase 1 */
    {
        // Conditional removing V.42 QDP/ADP detection on QCA1d V.42 bit
        SetReg(pDpcs->MCF, V42SKIPDETECT);
        TRACE0("V8: SHORT PHASE 1");
        pV8->call_rate = SHORTPHASE1_CALL_RATE;

        pV21->ubBufferSize = SHORTPHASE1_BUFSIZE;

        V8_V90a_TimRx_init(&pV8->TIMRx);
    }

#endif
}
