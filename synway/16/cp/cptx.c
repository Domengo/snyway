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
#if SUPPORT_FAX
#include "ptable.h"
#endif

void CP_Send_Dummy(CpStruct *pCp)
{
    pCp->pfTxStateVec = CP_Dummy;
    pCp->pfModVec     = CP_Mod_Dummy;
}

void CP_Send_Timeout_Silence(CpStruct *pCp)
{
    pCp->pfTxStateVec = CP_TxState_Send_TSilence;
    pCp->pfModVec     = CP_Mod_Dummy;
}

void CP_Send_Silence(CpStruct *pCp)
{
    /* setup tx functions */
    pCp->pfTxStateVec = CP_TxState_Send_Silence;
    pCp->pfModVec     = CP_Mod_Dummy;
}

void CP_Send_Nothing(CpStruct *pCp)
{
    /* setup tx functions */
    pCp->pfTxStateVec = CP_TxState_Send_Nothing;
    pCp->pfModVec     = CP_Mod_Dummy;
}

void CP_Send_DTMFTones(CpStruct *pCp)
{
    DtmfGenStruct *pDtmf = &pCp->Dtmf;

    /* setup TX variables */
    pCp->TxOffTime = 0;
    pCp->TxOnTime  = pDtmf->OnTime;

    /* setup TX functions */
    pCp->pfTxStateVec = CP_TxState_Send_DTMFTones;
    pCp->pfModVec     = CP_Mod;
}

void CP_Send_Pulses(CpStruct *pCp)
{
    /* setup TX variables */
    pCp->TxOffTime = 0;

    /* setup TX functions */
    pCp->pfTxStateVec = CP_TxState_Send_Pulses;
    pCp->pfModVec     = CP_Mod_Dummy;
}

void CP_Send_ANSTone(CpStruct *pCp)
{
    ACEStruct *pAce = (ACEStruct *)(pCp->pTable[ACE_STRUC_IDX]);
    pCp->GenTone_Flag = 1;
    pCp->GenToneFreq  = CP_ANS_TONE_FREQ;
    pCp->GenTonePhase = 0;

    /* setup TX functions */
    if (pAce->FaxClassType == FCLASS3)
    {
        pCp->pfTxStateVec = CP_Dummy;
    }
    else
    {
        pCp->pfTxStateVec = CP_TxState_ANSTone;
    }

    pCp->pfModVec = CP_Mod;
}

#if SUPPORT_FAX_SEND_CNG
void CP_Send_CNG(CpStruct *pCp)
{
    pCp->GenTone_Flag = 1;        /* let CP_MOD generate CNG */
    pCp->GenToneFreq  = CP_CNG_TONE_FREQ;
    pCp->GenTonePhase = 0;

    /* setup TX functions */
    pCp->pfTxStateVec = CP_TxState_CNG;
    pCp->pfModVec     = CP_Mod;
}
#endif
