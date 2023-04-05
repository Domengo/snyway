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

/*-------------------------------------------------------------------------*/
/* Call Progress Transmit & Receive Initialization                         */
/*-------------------------------------------------------------------------*/

#include "cpext.h"

/* -------------------------- */
/*   Real_Time ISR Function   */
/* -------------------------- */
void CP_Isr(CpStruct *pCp)
{
    /* transmit and receive one buffer */
    CP_tx_buf(pCp);
    CP_Detect(pCp);
}

void CP_tx_buf(CpStruct *pCp)
{
    /* check for next state */
    if (pCp->TxDelayCnt <= 0)
    {
        /* go to next state, set up delay counter */
        pCp->TxVecIdx++;
        pCp->TxDelayCnt = pCp->pTxDelayptr[pCp->TxVecIdx];
        pCp->pfTxVecptr[pCp->TxVecIdx](pCp);
    }

    /* call TX functions */
    pCp->pfTxStateVec(pCp);
    pCp->pfModVec(pCp);

    /* decrement TX delay counter */
    pCp->TxDelayCnt--;
}

void CP_init(CpStruct *pCp)
{
    CP_FSM_init(pCp);            /* init function vectors and delay counters */

    CP_tx_init(pCp);            /* initialize TX vectors */

    CP_Detect_init(pCp);        /* initialize RX functions */
}

void CP_tx_init(CpStruct *pCp)
{
    DtmfGenStruct *pDtmf = &pCp->Dtmf;
    SBYTE i;

    /* initialize variables */
    pCp->Dial_Flag    = 0;
    pCp->TxEnd        = 0;
    pCp->GenTone_Flag = 0;

    pDtmf->qHighLevelRef = DTMF_LEVEL_REF_HIGH;
    pDtmf->qLowLevelRef  = DTMF_LEVEL_REF_LOW;

    for (i = 0; i < (pDtmf->sbLevelRef_dB - 5); i++)
    {
        pDtmf->qHighLevelRef = QQMULQ15(pDtmf->qHighLevelRef, 29205);
        pDtmf->qLowLevelRef  = QQMULQ15(pDtmf->qLowLevelRef,  29205);
    }
}

SWORD CP_Cadence_OnTimeMS(SWORD OnTime)
{
    return(((((SDWORD)OnTime * CP_SAMPLERATE) + 3999) / 4000) + 20);
}

SWORD CP_Cadence_OffTimeMS(SWORD OffTime)
{
    return(((((SDWORD)OffTime * CP_SAMPLERATE) + 3999) / 4000) - 20);
}


void CP_Tone_init_CANADA(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = (TONE350 | TONE440);
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(250);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = (TONE480 | TONE620);
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(500) + 3;

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = (TONE480 | TONE620);
    pCp->CongestionToneReference[0].ubCadenceNum   = 1;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(250);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(250) + 3;

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = (TONE440 | TONE480);
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(2000);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(4000) + 3;
}

void CP_Tone_init_CHINA(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = TONE450;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1200);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE450;
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(350);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(350) + 3;

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = TONE450;
    pCp->CongestionToneReference[0].ubCadenceNum   = 1;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(700);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(700) + 3;

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = TONE450;
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(4000) + 3;
}

void CP_Tone_init_FRANCE(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = TONE440;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1700);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE440;
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(500);

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = TONE440;
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1500);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(3500);
}

void CP_Tone_init_GERMANY(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = TONE425;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1200);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE425;
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(480);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(480);

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = TONE425;
    pCp->CongestionToneReference[0].ubCadenceNum   = 1;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(240);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(240);

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = TONE425;
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(4000);
}

void CP_Tone_init_ISRAEL(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = TONE400;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1200);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE400;
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(500);

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = TONE400;
    pCp->CongestionToneReference[0].ubCadenceNum   = 1;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(250);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(250);

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = TONE400;
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(3000);
}

void CP_Tone_init_KOREA(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = (TONE350 | TONE440);
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(250);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = (TONE480 | TONE620);
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(500) + 3;

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = (TONE480 | TONE620);
    pCp->CongestionToneReference[0].ubCadenceNum   = 1;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(300);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(200) + 3;

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = (TONE440 | TONE480);
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(2000) + 3;
}

void CP_Tone_init_NETHERLANDS(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = TONE425;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1200);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE425;
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(500);

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = TONE425;
    pCp->CongestionToneReference[0].ubCadenceNum   = 1;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(250);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(250);

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = TONE425;
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(4000);
}

void CP_Tone_init_SPAIN(CpStruct *pCp)
{
    /*  dial tone */
    pCp->DialToneReference[0].nTone         = TONE425;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1700);

    /*  special dial tone 1 */
    pCp->DialToneReference[1].nTone          = TONE425;
    pCp->DialToneReference[1].ubCadenceNum   = 1;
    pCp->DialToneReference[1].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->DialToneReference[1].nCadenceOff[0] = CP_Cadence_OffTimeMS(1000);

    /*  special dial tone 2 */
    pCp->DialToneReference[2].nTone          = TONE425;
    pCp->DialToneReference[2].ubCadenceNum   = 1;
    pCp->DialToneReference[2].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->DialToneReference[2].nCadenceOff[0] = CP_Cadence_OffTimeMS(50);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE425;
    pCp->BusyToneReference[0].ubCadenceNum   = 3;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(200);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(200);
    pCp->BusyToneReference[0].nCadenceOn[1]  = CP_Cadence_OnTimeMS(200);
    pCp->BusyToneReference[0].nCadenceOff[1] = CP_Cadence_OffTimeMS(200);
    pCp->BusyToneReference[0].nCadenceOn[2]  = CP_Cadence_OnTimeMS(200);
    pCp->BusyToneReference[0].nCadenceOff[2] = CP_Cadence_OffTimeMS(200);

    /*  busy tone 2 */
    pCp->BusyToneReference[1].nTone          = TONE425;
    pCp->BusyToneReference[1].ubCadenceNum   = 1;
    pCp->BusyToneReference[1].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->BusyToneReference[1].nCadenceOff[0] = CP_Cadence_OffTimeMS(500);

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = TONE425;
    pCp->CongestionToneReference[0].ubCadenceNum   = 3;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(200);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(200);
    pCp->CongestionToneReference[0].nCadenceOn[1]  = CP_Cadence_OnTimeMS(200);
    pCp->CongestionToneReference[0].nCadenceOff[1] = CP_Cadence_OffTimeMS(200);
    pCp->CongestionToneReference[0].nCadenceOn[2]  = CP_Cadence_OnTimeMS(200);
    pCp->CongestionToneReference[0].nCadenceOff[2] = CP_Cadence_OffTimeMS(600);

    /*  congestion tone 2 */
    pCp->CongestionToneReference[1].nTone          = TONE425;
    pCp->CongestionToneReference[1].ubCadenceNum   = 1;
    pCp->CongestionToneReference[1].nCadenceOn[0]  = CP_Cadence_OnTimeMS(250);
    pCp->CongestionToneReference[1].nCadenceOff[0] = CP_Cadence_OffTimeMS(250);

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = TONE425;
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1500);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(3000);

    /*  ringback tone 2 */
    pCp->RingBackToneReference[1].nTone          = TONE425;
    pCp->RingBackToneReference[1].ubCadenceNum   = 1;
    pCp->RingBackToneReference[1].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->RingBackToneReference[1].nCadenceOff[0] = CP_Cadence_OffTimeMS(4000);
}

void CP_Tone_init_UK(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = TONE50;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(600);

    /*  dial tone 2 */
    pCp->DialToneReference[1].nTone         = (TONE350 | TONE440);
    pCp->DialToneReference[1].ubCadenceNum  = 0;
    pCp->DialToneReference[1].nCadenceOn[0] = CP_Cadence_OnTimeMS(600);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE400;
    pCp->BusyToneReference[0].ubCadenceNum   = 2;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(375);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(375);
    pCp->BusyToneReference[0].nCadenceOn[1]  = CP_Cadence_OnTimeMS(375);
    pCp->BusyToneReference[0].nCadenceOff[1] = CP_Cadence_OffTimeMS(375);

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = TONE400;
    pCp->CongestionToneReference[0].ubCadenceNum   = 2;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(400);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(350);
    pCp->CongestionToneReference[0].nCadenceOn[1]  = CP_Cadence_OnTimeMS(225);
    pCp->CongestionToneReference[0].nCadenceOff[1] = CP_Cadence_OffTimeMS(525);

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = (TONE400 | TONE450);
    pCp->RingBackToneReference[0].ubCadenceNum   = 2;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(400);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(200);
    pCp->RingBackToneReference[0].nCadenceOn[1]  = CP_Cadence_OnTimeMS(400);
    pCp->RingBackToneReference[0].nCadenceOff[1] = CP_Cadence_OffTimeMS(2000);
}

void CP_Tone_init_VANUATU(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = TONE425;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1200);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE400;
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(750);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(750) + 3;

    /*  busy tone 2 */
    pCp->BusyToneReference[1].nTone          = TONE425;
    pCp->BusyToneReference[1].ubCadenceNum   = 1;
    pCp->BusyToneReference[1].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->BusyToneReference[1].nCadenceOff[0] = CP_Cadence_OffTimeMS(500) + 3;

    /*  ringback tone 1 */
    pCp->RingBackToneReference[0].nTone          = TONE425;
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(4000) + 3;
}

void CP_Tone_init_AUSTRALIA(CpStruct *pCp)
{
    /*  dial tone 1 (ITU standard 425X25Hz) */
    pCp->DialToneReference[0].nTone         = TONE425;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1200);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE400;
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(375);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(375);

    /*  ringback tone 1 (ITU standard 400X17Hz 0.4-0.2-0.4-2.0) */
    pCp->RingBackToneReference[0].nTone          = TONE400;
    pCp->RingBackToneReference[0].ubCadenceNum   = 2;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(400);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(200);
    pCp->RingBackToneReference[0].nCadenceOn[1]  = CP_Cadence_OnTimeMS(400);
    pCp->RingBackToneReference[0].nCadenceOff[1] = CP_Cadence_OffTimeMS(2000);

    /*  ringback tone 2 (Captured from phone line 425X25Hz 0.4-0.2-0.4-2.0) */
    pCp->RingBackToneReference[1].nTone          = TONE425;
    pCp->RingBackToneReference[1].ubCadenceNum   = 2;
    pCp->RingBackToneReference[1].nCadenceOn[0]  = CP_Cadence_OnTimeMS(400);
    pCp->RingBackToneReference[1].nCadenceOff[0] = CP_Cadence_OffTimeMS(200);
    pCp->RingBackToneReference[1].nCadenceOn[1]  = CP_Cadence_OnTimeMS(400);
    pCp->RingBackToneReference[1].nCadenceOff[1] = CP_Cadence_OffTimeMS(2000);

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = TONE400;
    pCp->CongestionToneReference[0].ubCadenceNum   = 1;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(375);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(375);
}

void CP_Tone_init_JAPAN(CpStruct *pCp)
{
    /*  dial tone 1 */
    pCp->DialToneReference[0].nTone         = TONE400;
    pCp->DialToneReference[0].ubCadenceNum  = 0;
    pCp->DialToneReference[0].nCadenceOn[0] = CP_Cadence_OnTimeMS(1200);

    /*  dial tone 2 (PABX) */
    pCp->DialToneReference[1].nTone         = TONE400;
    pCp->DialToneReference[1].ubCadenceNum  = 1;
    pCp->DialToneReference[1].nCadenceOn[0] = CP_Cadence_OnTimeMS(250);
    pCp->DialToneReference[1].nCadenceOff[0] = CP_Cadence_OffTimeMS(250);

    /*  dial tone 3 (2nd dialtone 1) */
    pCp->DialToneReference[2].nTone         = TONE400;
    pCp->DialToneReference[2].ubCadenceNum  = 1;
    pCp->DialToneReference[2].nCadenceOn[0] = CP_Cadence_OnTimeMS(125);
    pCp->DialToneReference[2].nCadenceOff[0] = CP_Cadence_OffTimeMS(125);

    /*  dial tone 4 (2nd dialtone 2) */
    pCp->DialToneReference[3].nTone         = TONE440;
    pCp->DialToneReference[3].ubCadenceNum  = 1;
    pCp->DialToneReference[3].nCadenceOn[0] = CP_Cadence_OnTimeMS(125);
    pCp->DialToneReference[3].nCadenceOff[0] = CP_Cadence_OffTimeMS(125);

    /*  dial tone 5 (2nd dialtone 3) */
    pCp->DialToneReference[4].nTone         = TONE400;
    pCp->DialToneReference[4].ubCadenceNum  = 1;
    pCp->DialToneReference[4].nCadenceOn[0] = CP_Cadence_OnTimeMS(150);
    pCp->DialToneReference[4].nCadenceOff[0] = CP_Cadence_OffTimeMS(150);

    /*  busy tone 1 */
    pCp->BusyToneReference[0].nTone          = TONE400;
    pCp->BusyToneReference[0].ubCadenceNum   = 1;
    pCp->BusyToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->BusyToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(500);

    /*  ringback tone 1 (400X16Hz) */
    pCp->RingBackToneReference[0].nTone          = TONE400;
    pCp->RingBackToneReference[0].ubCadenceNum   = 1;
    pCp->RingBackToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(1000);
    pCp->RingBackToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(2000);

    /*  congestion tone 1 */
    pCp->CongestionToneReference[0].nTone          = TONE400;
    pCp->CongestionToneReference[0].ubCadenceNum   = 1;
    pCp->CongestionToneReference[0].nCadenceOn[0]  = CP_Cadence_OnTimeMS(500);
    pCp->CongestionToneReference[0].nCadenceOff[0] = CP_Cadence_OffTimeMS(500);
}
