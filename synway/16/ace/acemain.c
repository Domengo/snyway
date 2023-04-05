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

/*******************************************************************
* Description:   AT command processing function including class 1  *
* which can get command from keyborad / serial port                *
* Author:        Yalan Xu                                          *
*******************************************************************/

#include <string.h>
#include "aceext.h"
#include "hwapi.h"
#include "mhsp.h"
#include "gaoapi.h"
#include "v8ext.h"
#include "v251.h"

void Ace_SetResultCode(UBYTE **pTable, UBYTE aceResultCode)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

#if SUPPORT_FAX

    if (pAce->FaxClassType != FCLASS0)
    {
        Ace_SetClass1ResultCode(pAce, aceResultCode + 1);
    }
    else
#endif
    {
#if SUPPORT_PARSER
        AceSetResponse(pTable, aceResultCode);
#endif
    }
}

void Ace_ChangeState(ACEStruct *pAce, UBYTE newState)
{
    if (pAce->classState != newState)
    {
        pAce->classState = newState; TRACE1("ACE: State %s", Class_State[newState]);
    }
}

void Ace_ChangeLineState(ACEStruct *pAce, UBYTE newLineState)
{
    if (pAce->LineState != newLineState)
    {
        pAce->LineState = newLineState; TRACE1("ACE: %s", Line_State[pAce->LineState]);
    }
}

static void Ace_Disconnect(UBYTE **pTable, UBYTE aceResultCode)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    /* stop the modem */
    PutReg(pDpcs->MCF, DISCONNECT);
    pAce->CPState = NO_CP;
#if SUPPORT_PARSER
    pAce->ATidx = 0;
    pAce->Semicolon = SEMICOLON_NONE;
#endif
    pAce->Comma = 0;
    /* set result code */
    Ace_SetResultCode(pTable, aceResultCode);
}

/* check for hangup */
static void AceHangupCheck(UBYTE **pTable)
{
    UBYTE modem_status;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    /* read the modem state */
    modem_status = RdReg(pDpcs->MSR0, DISC_DET | LINE_DISC_DET);

    if (pAce->classState == ACE_ATH
        || (pAce->LineState == ONLINE_DATA)
        || (pAce->LineState == ONLINE_COMMAND)
        || (pAce->LineState == ONLINE_RETRAIN))
    {
        if (modem_status & DISC_DET)
        {
#if 1 /* SUPPORT_MODEM */

            if (pAce->LineState == ONLINE_DATA || pAce->LineState == ONLINE_RETRAIN)
            {
                TRACE0("ACE:ONLINE_DATA-LostOfCarrier");
                Ace_SetResultCode(pTable, ACE_NO_CARRIER);
            }
            else if (pAce->LineState == ONLINE_COMMAND)  /* Disconnect by Command. */
            {
                if (pAce->classState == ACE_ATH)
                {
                    Ace_SetResultCode(pTable, ACE_OK);
                }
                else
                {
                    TRACE0("ACE:ONLINE_ELSE-LostOfCarrier");
                    Ace_SetResultCode(pTable, ACE_NO_CARRIER);
                }
            }

#endif

#if SUPPORT_V54

            /* clear all self test command! */
            if (pDpcs->MCR0 & TEST_MODE)
            {
                ClrReg(pDpcs->MTC);
                ResetReg(pDpcs->MCR0, TEST_MODE);

                pDpcs->MCR1 = pAce->ubMCR1_BUFFER;
                pAce->ubSelfTest = SELFTEST_OFF;
                ClrReg(pSreg[16]);
            }

            V54_Init(pTable);      /* To make sure hangup terminate all self test */

            /* pAce->ATidx = 0; */
#endif

            /* hangup and get ready to return to command state */
            /****************************/
            /* get ready to return to command state */
            Ace_ChangeLineState(pAce, GOING_OFF_LINE);

            /* go on-hook */
            pHwApi[0] = HWAPI_ONHOOK;
            /********************************/

            MhspInit(pTable);/* Reinitialize call rate first, then reinitialize Hw */

            PutReg(pDpcs->MCF, LINE_DISCONNECT);/* Reinitialize Hw */

            /* clear DISC_DET bit */
            /* BEN: Do not clear Disconnect bit */
            /*            ClrReg(pDpcs->MSR0); */
            /* clear cut line flag */
            Ace_ChangeState(pAce, ACE_NOF);
        }
    }

    if ((pAce->LineState == GOING_OFF_LINE) && (modem_status & LINE_DISC_DET))
    {
        Ace_ChangeLineState(pAce, OFF_LINE);

        Ace_SetResultCode(pTable, ACE_OK);

        ClrReg(pDpcs->MSR0);

#if SUPPORT_FAX
        {
            DceToT30Interface *pDceToT30 = pAce->pDceToT30;

            TRACE0("ACE: Reset some fax variables");

            if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
            {
                // for class 2 or 3, OK needs to be passed to DTE. FCLASS will be reset after +FHS
#if SUPPORT_T31_PARSER
                Ace_ParseRespToClass1Dte(pTable);
#endif
                pAce->FaxClassType = FCLASS0;
            }

            pDceToT30->ctrlChnlBitRate = 0;
            pDceToT30->isDleEot = FALSE;
            pDceToT30->isV34Selected = FALSE;
            pDceToT30->selectedV34Chnl = 0;
            pAce->pT30ToDce->flowCntlCmd = NULL_DATA_CMD;
#if SUPPORT_T31_PARSER
            pAce->selectedV34Chnl = 0;
#endif
            pAce->isDleEotRcved = FALSE;
            pAce->isXmtingHdlc = pAce->isRcvingHdlc = FALSE;
        }
#endif
    }
}

static void AceRingDet(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (pAce->LineState == OFF_LINE)
    {
        DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
        UWORD *pClk = (UWORD *)pTable[CLOCKDATA_IDX];
        UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
        UWORD nTimeElapedIn10mSec, uTimeOut;
        UBYTE modem_status = RdReg(pDpcs->MSR0, RING_DET);

        /* automatic answer if s0!=0 */
        if (modem_status & RING_DET)
        {
            /* Clear Ring Flag */
            ClrReg(pDpcs->MSR0);
            /* response 'RING' */

            Ace_SetResultCode(pTable, ACE_RING);

            pAce->StartTime = *pClk;  /* Init clock for RING timeout */

            pAce->RingCount++;

            if (pSreg[0])
            {
                if (pAce->RingCount == pSreg[0])
                {
                    Ace_ATA(pTable);
                    pAce->RingCount = 0;
                }
            }
        }

        nTimeElapedIn10mSec = pAce->StartTime - (*pClk);

#if SUPPORT_V34FAX
        uTimeOut = 2800;
#else
        uTimeOut = 1200;
#endif

        if ((pAce->RingCount > 0) && (nTimeElapedIn10mSec > uTimeOut))
        {
            pAce->RingCount = 0;

            TRACE1("ACE: Ring count reset:%d", nTimeElapedIn10mSec);
        }
    }

}

/* go to online data/fax mode */
static void GoOnline(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    pAce->CPState = NO_CP;
}

/* check for call progress */
static UBYTE AceCp(UBYTE **pTable, UBYTE Keyhit)
{
    UBYTE modem_status, toneType;
#if SUPPORT_MODEM
    UBYTE ECC_mode;
#endif
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UWORD *pClk = (UWORD *)pTable[CLOCKDATA_IDX];
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
#if SUPPORT_V34FAX
    UBYTE isV34Enabled = *pAce->pT30ToDce->pIsV34Enabled;
#endif
    UWORD nTimeElapedIn10mSec;
    QWORD qTimeElapedInSec;

    nTimeElapedIn10mSec = pAce->StartTime - (*pClk);
    qTimeElapedInSec    = UQMULQ15(nTimeElapedIn10mSec, q001);

    modem_status = RdReg(pDpcs->MSR0, DATA | DISC_DET);

    toneType = RdReg(pDpcs->TONESR, TONE1_DET | TONE2_DET | TONE5_DET);

    switch (pAce->CPState)
    {
        case NO_CP:
            break;

        case CP_START:
#if SUPPORT_FXS

            if (qTimeElapedInSec >= pSreg[BLIND_DIAL_PAUSE_TIME])
            {
                /* goto next state */
                pAce->CPState = CP_DIALSTOP; /// LLL don't dial at all
                pAce->timerT0 = 1;
                TRACE0("ACE: T0 timer starts after completing dialling.");
            }

#else

            if (Keyhit)
            {
                /* go back to command state and issue "no dialtone" */
                TRACE0("ACE: No dialtone 1");
                Ace_Disconnect(pTable, ACE_NO_DIALTONE);
            }
            else if (pAce->DialToneDet && ((pAce->Semicolon == SEMICOLON_NONE) || (pAce->Semicolon == SEMICOLON_ON_FIRST))) /* Not to detect dialtone after semicolon present */
            {
                if (qTimeElapedInSec >= pSreg[DIAL_TONE_DET_TIMEOUT]) /* S13 timeout */
                {
                    /* go back to command state and issue "no dialtone */
                    TRACE0("ACE: No dialtone 2");
                    Ace_Disconnect(pTable, ACE_NO_DIALTONE);
                }
                else if (toneType & TONE1_DET)
                {
                    /* clear the tone_det bit */
                    ResetReg(pDpcs->TONESR, TONE1_DET);
                    pAce->CPState = CP_DIAL;
                    TRACE0("ACE: Dial Tone detected.");
                }
            }
            else if (qTimeElapedInSec >= pSreg[BLIND_DIAL_PAUSE_TIME])
            {
                /* goto next state */
                pAce->CPState = CP_DIAL;
            }

#endif
            return 1;

        case CP_DIAL:
            /* dialing */
            PutReg(pDpcs->MCF, DIAL);
            pAce->StartTime = *pClk;
            pAce->CPState = CP_DIALFIN;

            return 1;
        case CP_DIALFIN:
        {
            if (Keyhit || (qTimeElapedInSec >= pSreg[NO_ANSWER_TIMEOUT]))
            {
                /* go back to command state and issue "no carrier" */
                TRACE0("ACE: No carrier 1");
                Ace_Disconnect(pTable, ACE_NO_CARRIER);
            }
            /* if dial finished */
            else if (GetReg(pDpcs->DIALEND))
            {
                ClrReg(pDpcs->MSR0);/* clear DISC_DET bit */

#if SUPPORT_PARSER

                if ((pAce->Semicolon == SEMICOLON_ON_FIRST) || (pAce->Semicolon == SEMICOLON_ON_AGAIN)) /* semicolon present */
                {
                    pAce->CPState = NO_CP;
                    Ace_ChangeLineState(pAce, OFF_LINE);
                    pAce->Semicolon = SEMICOLON_AFTER; /* after semicolon */

                    pAce->Mandate = TRUE;
                    AceSetResponse(pTable, ACE_OK);
                }
                else
#endif
                {
                    /* nothing more to dial */
                    /* clear the tone_det_bit */
                    ResetReg(pDpcs->TONESR, TONE5_DET); /* ANS tone */
                    ResetReg(pDpcs->TONESR, TONE2_DET); /* BUSY tone */

                    pAce->StartTime = *pClk;

                    pAce->CPState = CP_DIALSTOP;
#if SUPPORT_PARSER
                    pAce->Semicolon = SEMICOLON_NONE;
#endif
                    pAce->timerT0 = 1;
                    TRACE0("ACE: T0 timer starts after completing dialling.");
                }
            }

            return 1;
        }

        case CP_DIALSTOP:
        {
            if (Keyhit || (qTimeElapedInSec >= pSreg[NO_ANSWER_TIMEOUT]))
            {
                /* go back to command state and issue "no carrier" */
                TRACE0("ACE: No carrier 2");
                TRACE2("ACE: qTimeElapedInSec=%d, pSreg[NO_ANSWER_TIMEOUT]=%d", qTimeElapedInSec, pSreg[NO_ANSWER_TIMEOUT]);
                Ace_Disconnect(pTable, ACE_NO_CARRIER);
            }

#if SUPPORT_AUTO
            /* don't detect answer tone in Auto mode and wait 2 sec for busy tone */
            else if ((pAce->FaxClassType == FCLASS0) && pAce->AutoMode && (qTimeElapedInSec >= 2) && pAce->ubBusyToneFreq == 0)
            {
                /* stop the call-progress */
                PutReg(pDpcs->MCF, DISCONNECT);
                pAce->CPState = CP_CONNECT;
                TRACE0("ACE: ANS TONE detection bypassed in Auto mode. -->CP_CONNECT");
            }

#endif
            /* check for answer tone */
            else if (toneType & TONE5_DET)
            {
                /* clear the tone_det_bit */
                ResetReg(pDpcs->TONESR, TONE5_DET);
                /* stop the call-progress */
                PutReg(pDpcs->MCF, DISCONNECT); // fixing issue 159, ACE terminates CP
                pAce->CPState = CP_CONNECT;

                pAce->timerT0 = 0;
                TRACE0("ACE: T0 timer reset before expiring.");
                TRACE1("T0 address: %p", &pAce->timerT0);
#if SUPPORT_FAX

                if (pAce->FaxClassType != FCLASS0)
                {
#if SUPPORT_V34FAX

                    if (!isV34Enabled)
#endif
                    {
                        Ace_ChangeState(pAce, ACE_FRH);
                        pAce->isOptFrm = FALSE;
                    }
                }

#endif
                /* TRACE0("ACE:ANS TONE detected. -->CP_CONNECT"); */
            }
            else if (pAce->BusyDet)
            {
                /* check busy */
                if (toneType & TONE2_DET)
                {
                    /* clear the tone_det_bit */
                    ResetReg(pDpcs->TONESR, TONE2_DET);
                    /* go back to command state and issue "busy" */
                    TRACE0("ACE: Busy tone detected");
                    Ace_Disconnect(pTable, ACE_BUSY);
                }
            }

            return 1;
        }

        case CP_ANSSTOP:
        case CP_CONNECT:
        {
            if (Keyhit || (qTimeElapedInSec >= pSreg[NO_ANSWER_TIMEOUT]))
            {
                /* go back to command state and issue "no carrier" */
                TRACE0("ACE: No carrier 3");
                Ace_Disconnect(pTable, ACE_NO_CARRIER);
            }
            else if (modem_status & DISC_DET)
            {
                /* clear DISC_DET bit */
                ClrReg(pDpcs->MSR0);

#if SUPPORT_MODEM

                if (pAce->FaxClassType == FCLASS0)
                {
                    PutReg(pDpcs->MCF, CONNECT);  /* set modem connect flag */

                    if (pDpcs->MCR1 & V42)
                    {
                        ResetReg(pDpcs->MSR1, V42_FAIL);
                    } /* Do V.42 init */
                    else
                    {
                        SetReg(pDpcs->MSR1, V42_FAIL);
                    }

                    pAce->CPState = CP_FINISH;
                    Ace_ChangeLineState(pAce, MODEM_TRAINING);
                }

#endif
#if SUPPORT_V34FAX
                else if (isV34Enabled)
                {
                    PutReg(pDpcs->MCF, CONNECT); /* start V.8 */
                }

#endif
                GoOnline(pTable);
            }

            return 1;
        }
#if SUPPORT_MODEM
        case CP_FINISH:
        {
            if (Keyhit || (qTimeElapedInSec >= pSreg[NO_ANSWER_TIMEOUT]))
            {
                /* go back to command state and issue "no carrier" */
                TRACE1("No carrier 5, Keyhit=%d", Keyhit);
                TRACE2("qTimeElapedInSec=%d, pSreg[NO_ANSWER_TIMEOUT]=%d", qTimeElapedInSec, pSreg[NO_ANSWER_TIMEOUT]);

                Ace_Disconnect(pTable, ACE_NO_CARRIER);
            }
            else if (modem_status & DISC_DET) /* Disconnect detect from Modem */
            {
                TRACE0("No carrier 6");

                Ace_Disconnect(pTable, ACE_NO_CARRIER);
            }
            else if (modem_status & DATA)
            {
                ECC_mode = RdReg(pDpcs->MCR1, V42 | V42BIS | MNP);

                if (ECC_mode == 0) /* No V.42 or V.42bis or MNP */
                {
                    GoOnline(pTable);
                }
                else/* Use V.42 or V.42bis or MNP */
                {
                    ECC_mode = RdReg(pDpcs->MSR1, V42bis_DATA | V42_DATA | V42_FAIL | MNP_DATA | MNP_DATA1 | MNP_FAIL);

                    if ((ECC_mode != 0) && (ECC_mode != V42_FAIL)) /* if V.42 fails, we need MNP status also */
                    {
                        GoOnline(pTable);
                    }
                    else if ((ECC_mode == V42_FAIL)
#if SUPPORT_MNP
                             && (!(pDpcs->MCR1 & MNP))
#endif
                            )
                    {
                        /* by Horace */
                        GoOnline(pTable);
                    }
                }
            }

            return 1;
        }
#endif

        /* ata command */
        case CP_ANSWER:
        {
            if (Keyhit || (qTimeElapedInSec >= pSreg[NO_ANSWER_TIMEOUT]))
            {
                /* go back to command state and issue "no carrier" */
                TRACE0("ACE: No carrier 7");

                Ace_Disconnect(pTable, ACE_NO_CARRIER);
                /* TRACE0("issue no carrier when call_answer_case"); */
            }
            else if (modem_status & DISC_DET)
            {
                // CP finishes sending CED/Silence
                ResetReg(pDpcs->MSR0, DISC_DET);
                TRACE0("ACE: CP finishes sending CED/Silence");
                PutReg(pDpcs->MCF, CONNECT); /* start V.21/V.8 */
#if SUPPORT_FAX

                if (pAce->FaxClassType != FCLASS0)
                {
                    GoOnline(pTable);
#if SUPPORT_V34FAX

                    if (!isV34Enabled)
#endif
                    {
                        Ace_ChangeState(pAce, ACE_FTH);
                    }
                }
                else
#endif
                {
                    pAce->CPState = CP_ANSSTOP;
                }
            }

#if SUPPORT_AUTO
            else if ((pAce->FaxClassType == FCLASS0) && pAce->AutoMode)/* don't send answer tone in CP for Auto mode */
            {
                /* stop the call-progress */
                PutReg(pDpcs->MCF, DISCONNECT);
                pAce->CPState = CP_ANSSTOP;
            }

#endif

            return 1;
        }
    }

    return 0;
}

void ACEMain(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE Keyhit = FALSE;
#if SUPPORT_PARSER
    UBYTE temp, ch = 0;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    CircBuffer *DteWr = pAce->pCBIn;
    UBYTE NoGetData_FromDTE_Flag;

#if SUPPORT_T31_PARSER

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
    {
        Ace_ParseDataFromClass1Dte(pTable);
    }

#endif
#if SUPPORT_T32_PARSER

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        Ace_ParseDataFromClass2Dte(pTable);
    }

#endif

    /* check input char */
    temp = (pAce->LineState == ONLINE_COMMAND)
#if SUPPORT_FAX
           || (pAce->LineState == ONLINE_FAX_COMMAND) // CONNECT hasn't been sent out
#endif
           || (pAce->LineState == OFF_LINE) || (pAce->LineState == CALL_PROGRESS)
           || (pAce->LineState == MODEM_TRAINING);

    if (DteWr->CurrLen && temp)
    {
        NoGetData_FromDTE_Flag  = RdReg(pDpcs->MCF,  RTRN     | RRATE);
        NoGetData_FromDTE_Flag |= RdReg(pDpcs->MSR0, RTRN_DET | RRATE_DET);

        if (pAce->LineState == MODEM_TRAINING)
        {
            NoGetData_FromDTE_Flag |= RdReg(pDpcs->MSR0, DATA);
        }

        if (NoGetData_FromDTE_Flag == 0)
        {
            if (GetByteFromCB(DteWr, &ch))
            {
                Keyhit = 1;

#if FAX_DEBUG
                TRACE13("~%c", ch);
#endif
            }
        }
    }

#endif

    if (pAce->timerT0 > TIMER_T0_LENGTH)
    {
        TRACE1("T0 address: %p", &pAce->timerT0);
        pAce->timerT0 = 0;
        TRACE0("ACE: ERROR. T0 elapsed");
        Ace_Disconnect(pTable, ACE_ERROR);
    }
    else if (pAce->timerT0 != 0)
    {
        pAce->timerT0++;
    }

    AceHangupCheck(pTable);

    AceRingDet(pTable);

    if (AceCp(pTable, Keyhit))
    {
        return;
    }

#if SUPPORT_PARSER

    if (Keyhit)
    {
        AceAt(pTable, ch);
    }

#endif

#if SUPPORT_T31

    if (pAce->FaxClassType != FCLASS0)
    {
        Ace_GetClass1Command(pTable);
    }

#endif

    Ace_Fsm(pTable);

#if SUPPORT_T31_PARSER

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
    {
        if (pAce->LineState == ONLINE_FAX_DATA)
        {
            Ace_ParseDataToClass1Dte(pTable);
        }

        Ace_ParseRespToClass1Dte(pTable);
    }

#endif
#if SUPPORT_T32_PARSER

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        if (pAce->LineState == ONLINE_FAX_DATA)
        {
            Ace_ParseDataToClass2Dte(pTable);
        }

        Ace_ParseRespToClass2Dte(pTable);
    }

#endif
}
