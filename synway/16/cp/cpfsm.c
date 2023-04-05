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

/*************************************************************
*       This file contains the FSMs used to initialize           *
*       'TxVec', 'RxVec', 'TxDelayptr', and 'RxDelayptr' for     *
*       the current state indicated by 'TxVecIdx' and 'RxVecidx' *
*************************************************************/

#include "cpext.h"
#include "ptable.h"
#include "v251.h"

/***********************************************************
- a table containing the FSM used to initialize 'pfTxVecptr'
- a table containing the FSM used to initialize 'TxDelayCnt'
***********************************************************/
void CP_FSM_init(CpStruct *pCp)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pCp->pTable[DPCS_STRUC_IDX]);
    ACEStruct *pAce = (ACEStruct *)(pCp->pTable[ACE_STRUC_IDX]);
    UBYTE i, j;
    UBYTE mode;

    /* ------------------------------------------------ */
    /*                  TX Initialize                   */
    /* ------------------------------------------------ */

    if (pCp->modem_mode == CALL_MODEM)
    {
        /* init. 'pfTxVecptr' FSM */
        mode = RdReg(pDpcs->MCR1, PULSE_MODE);

#if SUPPORT_FAX
        pCp->Fax_Callmode = RdReg(pDpcs->MCR0, FAX);
#endif

        i = 0;
#if !SUPPORT_FXS /// LLL don't dial at all
        pCp->pfTxVecptr[i++] = CP_Send_Nothing;

        if (mode)
        {
            pCp->pfTxVecptr[i++] = CP_Send_Pulses;
        }
        else
        {
            pCp->pfTxVecptr[i++] = CP_Send_DTMFTones;
        }

#endif
#if SUPPORT_FAX_SEND_CNG

        if (pCp->Fax_Callmode
#if SUPPORT_V251
            && (pDpcs->V8OPCNTL & A8E_V8O) != A8E_V8O_2
            && (pDpcs->V8OPCNTL & A8E_V8O) != A8E_V8O_5
#endif
           )
        {
            pCp->pfTxVecptr[i++] = CP_Send_Silence;
            pCp->pfTxVecptr[i++] = CP_Send_CNG;
            pCp->CNGtone_on = 1;

            pCp->RemoteAnsToneDetected = 0;
        }

#endif

        pCp->pfTxVecptr[i++] = CP_Send_Silence;

        /* init. 'pTxDelayCnt' FSM */
        i = 0;
#if !SUPPORT_FXS /// LLL don't dial at all
        pCp->pTxDelayptr[i++] = TX_TIMEOUT;         /* Send_NOTHING   */
        pCp->pTxDelayptr[i++] = TX_TIMEOUT;         /* Send_DTMFTONES */
#endif
#if SUPPORT_FAX_SEND_CNG

        if (pCp->Fax_Callmode
#if SUPPORT_V251
            && (pDpcs->V8OPCNTL & A8E_V8O) != A8E_V8O_2
            && (pDpcs->V8OPCNTL & A8E_V8O) != A8E_V8O_5
#endif
           )
        {
            pCp->pTxDelayptr[i++] = (UWORD)UQMULQD(200, CP_CALL_RATE);
            pCp->pTxDelayptr[i++] = UBUBMULU(50,  CP_CALL_RATE);
        }

#endif

        pCp->pTxDelayptr[i++] = TX_TIMEOUT;         /* Send_SILENCE   */
    }
    else if (pCp->modem_mode == ANS_MODEM)
    {
#if SUPPORT_FAX
        pCp->Fax_Callmode = 0;
#endif

        i = 0;/* init. 'pfTxVecptr'  FSM */
        j = 0;/* init. 'pTxDelayCnt' FSM */

#if SUPPORT_T38GW
        pCp->pfTxVecptr[i++]  = CP_Send_Timeout_Silence;
        pCp->pTxDelayptr[j++] = UBUBMULU(1, CP_CALL_RATE);      /* 10ms */
#else

        if (pCp->V8Mode)
        {
            pCp->pfTxVecptr[i++]  = CP_Send_Timeout_Silence;

            pCp->pTxDelayptr[j++] = UBUBMULU(q10, CP_CALL_RATE);      /* 100ms */
        }
        else
        {
            pCp->pfTxVecptr[i++]  = CP_Send_Dummy;
            pCp->pTxDelayptr[j++] = UBUBMULU(200, CP_CALL_RATE);      /* Send Silence for 2000ms */

            pCp->pfTxVecptr[i++]  = CP_Send_ANSTone;

            if (pAce->FaxClassType == FCLASS3)
            {
                pCp->pTxDelayptr[j++] = (UWORD)UQMULQD(300, CP_CALL_RATE);    /* CED tone of 3000ms */
            }
            else
            {
                pCp->pTxDelayptr[j++] = (UWORD)UQMULQD(330, CP_CALL_RATE);    /* Max ANSTONE time 3300ms */
            }
        }

        pCp->pfTxVecptr[i++]  = CP_Send_Timeout_Silence;
        pCp->pTxDelayptr[j++] = UBUBMULU(7,  CP_CALL_RATE);           /* Send Silence for 70ms   */
#endif
    }

    /* ------------------------------------------------ */
    /*             Handshaking Initialize               */
    /* ------------------------------------------------ */

    /* init. 'TxVecIdx' and 'TxDelayCnt' to start handshaking */
    pCp->TxVecIdx   = 0;
    pCp->TxDelayCnt = pCp->pTxDelayptr[pCp->TxVecIdx];
    pCp->pfTxVecptr[pCp->TxVecIdx](pCp);
}
