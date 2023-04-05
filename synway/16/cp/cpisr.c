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

#include "ptable.h"
#include "cpext.h"
#include "mhsp.h"
#include "circbuff.h"

/* for returning tone detection code */
void CP_DetToneStatus(UBYTE **pTable, UWORD uDetTones)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    switch (uDetTones)
    {
        case DIALTONE :
            SetReg(pDpcs->TONESR, DIALTONE);
            /* TRACE1("Detect DialTone in CP, uDetTones=%d", uDetTones); */
            break;
        case BUSYTONE :
            SetReg(pDpcs->TONESR, BUSYTONE);
            TRACE1("CP: Detect BusyTone in CP, uDetTones=%d", uDetTones);
            break;
        case RECEIVEROFFHOOKTONE :
            SetReg(pDpcs->TONESR, RECEIVEROFFHOOKTONE);
            /* TRACE1("Detect ReceiverOffhookTone in CP, uDetTones=%d", uDetTones); */
            break;
        case RINGBACKTONE :
            SetReg(pDpcs->TONESR, RINGBACKTONE);
            /* TRACE1("Detect RingBackTone in CP, uDetTones=%d", uDetTones); */
            break;
        case CONGESTIONTONE :
            SetReg(pDpcs->TONESR, CONGESTIONTONE);
            /* TRACE1("Detect CongestionTone in CP, uDetTones=%d", uDetTones); */
            break;
        default : /* UNKNOWN TONE */
            break;
    }
}

void CP_DpcsIsr(UBYTE **pTable)
{
#if WITH_DC
    DCStruct   *pDC   = (DCStruct *) pTable[DC_STRUC_IDX];
#endif
    ACEStruct  *pAce  = (ACEStruct *)pTable[ACE_STRUC_IDX];
#if SUPPORT_V34FAX
    UBYTE isV34Enabled = *pAce->pT30ToDce->pIsV34Enabled;
#endif
    CpStruct   *pCp   = (CpStruct *)pTable[CP_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    QDWORD qdPreEnergy, qdEnergy;
    UBYTE Disc;
    UBYTE Dial;
    UBYTE i;

    /* setup buffers */
    pCp->PCMinPtr  = (SWORD *)pTable[PCMINDATA_IDX];
    pCp->PCMoutPtr = (SWORD *)pTable[PCMOUTDATA_IDX];

    /* check for dial command */
    Dial = RdReg(pDpcs->MCF, DIAL);

    if (Dial)
    {
        pCp->Dial_Flag = 1;

        ClrReg(pDpcs->MCF);
    }

#if WITH_DC

    if (pDC->DC_Count > 0)
    {
        pDC->DC_Count ++;

        if (pDC->DC_Count > 120) /* 100 + 20 */
        {
            pDC->DC_Count = 0;
            pDC->DC_Start = 1;
        }
    }

    if (pDC->DC_Start == 1)
    {
        DcEstimator(pDC, pCp->PCMinPtr, CP_BUF_SIZE, -5);    /* Low pass DC estimator filter */
    }

#endif

    /* process samples */
    CP_Isr(pCp);

#if SUPPORT_AUTO

    if (pCp->ubBusyToneFreq == 1)
    {
        pAce->ubBusyToneFreq = 1;
    }

#endif

    /* update the detected tone status */

#if 0/* PEOPNET */
    /* detect ANSTONE or USB1 for PeopleNet */
    if ((pCp->AnsCount > 26) || (pCp->BellAnsCount > 5))
    {
        SetReg(pDpcs->TONESR, TONE5_DET); /* ANSTONE or USB1 */
        TRACE0("ANSTONE or USB1 detected!");

        pCp->AnsCount = 0;
        pCp->BellAnsCount = 0;
    }

#else

    /* Moved out from function CP_DetToneStatus() */
    if ((pCp->modem_mode == CALL_MODEM) && (pCp->RemoteAnsToneDetected == 1))
    {
#if SUPPORT_FAX

        if (pAce->FaxClassType != FCLASS0)
        {
            qdPreEnergy = pCp->qdEnergy;

            qdEnergy = 0;

            for (i = 0; i < CP_BUF_SIZE; i++)
            {
                qdEnergy += QQMULQD(pCp->PCMinPtr[i], pCp->PCMinPtr[i]);
            }

            pCp->qdEnergy = qdEnergy;


            if ((qdPreEnergy > (qdEnergy << 1))
#if SUPPORT_V34FAX
                || (isV34Enabled)
#endif
               )
            {
                pCp->RemoteAnsToneDetected = 0;
                SetReg(pDpcs->TONESR, TONE5_DET); /* ANSTONE */
                TRACE0("CP: ANS Tone detected!");

                //pCp->AnsCount = 0;
#if SUPPORT_V34FAX

                if (isV34Enabled)
                {
                    PutReg(pDpcs->MMR0, V34FAX);
                }
                else
                {
                    pAce->V8Detect_State = V8_PASS_V21;
                }

#endif
            }
        }
        else
#endif
        {
            SetReg(pDpcs->TONESR, TONE5_DET); /* ANSTONE */
            TRACE0("CP: ANS Tone detected!");

            pCp->AnsCount = 0;
        }
    }
    else if ((pCp->V8Mode == 2) && (pCp->BellAnsCount > 33))
    {
        SetReg(pDpcs->TONESR, TONE5_DET); /* BELL TONE */
        TRACE0("CP: BELL Tone detected!");

        pCp->BellAnsCount = 0;
    }

#if SUPPORT_FAX
    else if ((pCp->Fax_Callmode) && (pCp->V21ToneCount == DET_CED_CNT_THRES))
    {
        SetReg(pDpcs->TONESR, TONE5_DET); /* Process V21 tone as ANSTONE for Fax */
        TRACE0("CP: V21 Tone detected. No Answer tone!");

        pCp->V21ToneCount = 0;
    }

#endif

#endif

    if (pCp->DetTones != pCp->LastDetTones)
    {
        CP_DetToneStatus(pTable, pCp->DetTones);
        pCp->LastDetTones = pCp->DetTones;
    }

    /* check if disconnect */
    Disc = RdReg(pDpcs->MCF, DISCONNECT);

    if (Disc || pCp->TxEnd || pCp->Disconnect_flag)
    {
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
    }
}
