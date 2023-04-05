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

#include "cpext.h"
#include "ptable.h"
#include "hwapi.h"
#include "sregdef.h"

void CP_Dummy(CpStruct *pCp)
{
}

void CP_TxState_Send_Silence(CpStruct *pCp)
{
    /* stay here forever */

#if SUPPORT_FAX_SEND_CNG
    if (pCp->Fax_Callmode == 0)
#endif
    {
        pCp->TxDelayCnt = TX_TIMEOUT;
    }
}

void CP_TxState_Send_TSilence(CpStruct *pCp)
{
    if (pCp->TxDelayCnt <= 2)
    {
        pCp->Disconnect_flag = 1;
        pCp->TxDelayCnt   = 2; /* Stay in time state until get disconnect */
    }
}

void CP_TxState_Send_Nothing(CpStruct *pCp)
{
    /* stay here forever unless dial command sent */
    if (pCp->Dial_Flag == 1)
    {
        pCp->Dial_Flag = 2;
        pCp->TxDelayCnt = 100;
    }
    else if (pCp->Dial_Flag == 2)
    {
        if (pCp->TxDelayCnt <= 1)
        {
            pCp->Dial_Flag = 0;
        }
    }
    else
    {
        pCp->TxDelayCnt = TX_TIMEOUT;
    }
}

void CP_TxState_ANSTone(CpStruct *pCp)
{
    pCp->V25PhaseChangeCounter ++;

    if (pCp->V25PhaseChangeCounter >= UBUBMULUB(45, CP_CALL_RATE)) /* Do 450ms 180 degree Phase change */
    {
        pCp->GenTonePhase += pCp->V25PhaseChange;

        pCp->V25PhaseChangeCounter = 0;
    }
}

void CP_TxState_Send_DTMFTones(CpStruct *pCp)
{
    DtmfGenStruct *pDtmf = &pCp->Dtmf;
    SWORD digit = '-';
    DpcsStruct *pDpcs = (DpcsStruct *)(pCp->pTable[DPCS_STRUC_IDX]);
    ACEStruct *pAce = (ACEStruct *)(pCp->pTable[ACE_STRUC_IDX]);
    UWORD *pClk = (UWORD *)(pCp->pTable[CLOCKDATA_IDX]);

    if (pCp->TxOffTime <= 0)
    {
        /* reset counters and start on-time */
        pCp->TxOffTime = pDtmf->OffTime;
        pCp->TxOnTime  = pDtmf->OnTime;
        pCp->TxOffOn   = TX_ON;

        /* determine what to do next */

        /* read string */
        while (digit == '-')
        {
            digit = pCp->pCmdString[pCp->CmdStringIdx++];
        }

        if (InitDtmfGen(&pCp->Dtmf, (CHAR)digit) == 0)
        {
            if (digit == 'w') /* send silence while waiting for dial-tone */
            {
                /* init. 'TxVecIdx' and 'TxDelayCnt' to start handshaking */
                pCp->TxVecIdx   = 0;
                pCp->TxDelayCnt = pCp->pTxDelayptr[pCp->TxVecIdx];
                pCp->pfTxVecptr[pCp->TxVecIdx](pCp);

                /* set state machine */
                pAce->CPState = CP_START;
                pAce->StartTime = *pClk;

                ResetReg(pDpcs->TONESR, TONE1_DET);

                ClrReg(pDpcs->MCF);
                pCp->Dial_Flag = 0;
                /* TestFlg = 1; */
            }
            else if (digit == END_TOKEN) /* done dialing */
            {
                pCp->TxDelayCnt = 0;

#if SUPPORT_FAX_SEND_CNG

                if (pCp->Fax_Callmode == 0)
#endif
                {
                    pCp->TxEnd = 1;
                }

                PutReg(pDpcs->DIALEND, 1);
            }
            else if (digit == 'p') /* Change to pulse dialing ? */
            {
                SetReg(pDpcs->MCR1, PULSE_MODE);

                CP_init(pCp);
                pCp->Dial_Flag = 1;
            }
            /* ',' is actually handled by ACE, if there is ACE module.
            If no ACE, this part should be enabled send silence for digit */
            else if (digit == ',')
            {
                UBYTE *pSreg = (UBYTE *)pCp->pTable[ACESREGDATA_IDX];

                pDtmf->bGen_Flag = 0;
                pCp->TxOnTime = UBUBMULU(pSreg[COMMA_DIAL_PAUSE_TIME] * q100, CP_CALL_RATE);
            }
        }
    }

    if (pCp->TxOnTime <= 0)
    {
        /* reset counters and start off-time */
        pCp->TxOffTime = pDtmf->OffTime;
        pCp->TxOnTime  = pDtmf->OnTime;
        pCp->TxOffOn   = TX_OFF;

        /* to send silence... */
        pDtmf->bGen_Flag = 0;
    }

    /* count on-time or off-time */
    if (pCp->TxOffOn == TX_ON)
    {
        pCp->TxOnTime--;
    }
    else if (pCp->TxOffOn == TX_OFF)
    {
        pCp->TxOffTime--;
    }
}

void CP_TxState_Send_Pulses(CpStruct *pCp)
{
    UBYTE digit = '-';
    UBYTE *pHwApi = (UBYTE *)(pCp->pTable[HWAPIDATA_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pCp->pTable[DPCS_STRUC_IDX]);
    ACEStruct *pAce = (ACEStruct *)(pCp->pTable[ACE_STRUC_IDX]);
    UWORD *pClk = (UWORD *)(pCp->pTable[CLOCKDATA_IDX]);
#if WITH_DC
    DCStruct *pDC = (DCStruct *)(pCp->pTable[DC_STRUC_IDX]);
#endif

    if (pCp->TxOffTime <= 0)
    {
        /* reset counters and start on-time */
        pCp->TxOffTime = pCp->DigitOffTime;
        pCp->TxOnTime  = pCp->nPulseMakeTime;
        pCp->TxBrkTime = pCp->nPulseBreakTime;
        pCp->TxOffOn   = TX_BREAK;

        /* read string */
        while (digit == '-')
        {
            digit = pCp->pCmdString[pCp->CmdStringIdx++];
        }

        /* Send another digit */
        if ((digit >= '0') && (digit <= '9'))
        {
            pCp->PulseCount = digit - '0';

            if (pCp->PulseCount == 0)
            {
                pCp->PulseCount = 10;
            }

            /* DO on-hook, break pulse */
            pHwApi[0] = HWAPI_CPONHOOK;
        }
        else if (digit == ',')/* Send silence, PAUSE */
        {
            UBYTE *pSreg = (UBYTE *)pCp->pTable[ACESREGDATA_IDX];

            pCp->TxOffOn = TX_OFF;
            pCp->TxOffTime = UBUBMULU(pSreg[COMMA_DIAL_PAUSE_TIME] * q100, CP_CALL_RATE);
        }
        else if (digit == 'w') /* send silence while waiting for dial-tone */
        {
            /* init. 'TxVecIdx' and 'TxDelayCnt' to start handshaking */
            pCp->TxVecIdx   = 0;
            pCp->TxDelayCnt = pCp->pTxDelayptr[pCp->TxVecIdx];
            pCp->pfTxVecptr[pCp->TxVecIdx](pCp);

            /* set state machine */
            pAce->CPState = CP_START;
            pAce->StartTime = *pClk;

            ResetReg(pDpcs->TONESR, TONE1_DET);

            ClrReg(pDpcs->MCF);
            pCp->Dial_Flag = 0;
        }
        else if (digit == END_TOKEN)/* done dialing */
        {
            pCp->TxDelayCnt   = 0;
            pCp->Disconnect_flag = 1;

            pHwApi[0] = HWAPI_CPOFFHOOK;

#if WITH_DC
            Init_DC(pDC);
#endif
            //pAce->timerT0 = 1;
            //TRACE0("T0 timer starts after completing pulse dialling.");

            pCp->TxEnd = 1;
        }
        else if (digit == 't')
        {
            ResetReg(pDpcs->MCR1, PULSE_MODE);

            CP_init(pCp);

            pCp->Dial_Flag = 1;
        }
    }

    /* Make PULSE if Break timer goes to 0 */
    if (pCp->TxBrkTime <= 0)
    {
        pCp->TxBrkTime = pCp->nPulseBreakTime;
        pCp->TxOffOn = TX_ON;

        /* DO off-hook, make pulse */
        pHwApi[0] = HWAPI_CPOFFHOOK;
    }

    /* Finish 1 pulse if make timer expires */
    if (pCp->TxOnTime <= 0)
    {
        /* Decrement pulse counter, if pulse counter goes to 0, */
        /* one digit has been sent.                             */
        pCp->PulseCount--;

        pCp->TxOnTime  = pCp->nPulseMakeTime;

        if (pCp->PulseCount <= 0)
        {
            pCp->TxOffOn = TX_OFF;
        }
        else
        {
            pCp->TxBrkTime = pCp->nPulseBreakTime;
            pCp->TxOffOn = TX_BREAK;

            /* DO on-hook, generate pulse */
            pHwApi[0] = HWAPI_CPONHOOK;
        }
    }

    if (pCp->TxOffOn == TX_ON)
    {
        pCp->TxOnTime--;
    }
    else if (pCp->TxOffOn == TX_BREAK)
    {
        pCp->TxBrkTime--;
    }
    else if (pCp->TxOffOn == TX_OFF)
    {
        pCp->TxOffTime--;
    }
}

#if SUPPORT_FAX_SEND_CNG
void CP_TxState_CNG(CpStruct *pCp)
{
    if (pCp->TxDelayCnt == 1)
    {
        if (pCp->CNGtone_on)
        {
            pCp->CNGtone_on = 0;
            pCp->TxDelayCnt = (UWORD)UQMULQD(300, CP_CALL_RATE);
            pCp->pfModVec   = CP_Mod_Dummy;
        }
        else if (!pCp->RemoteAnsToneDetected) // fixing issue 159, stop sending CNG when the beginning of CED/ANSam is detected
        {
            pCp->CNGtone_on = 1;
            pCp->TxDelayCnt = UBUBMULU(50, CP_CALL_RATE);
            pCp->pfModVec   = CP_Mod;
        }
    }
}
#endif
