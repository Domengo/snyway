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

#include "v17ext.h"

void V17_Send_A(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    /* setup tx functions */
    pV17->pfDataInVec  = V17_Dummy;

    pV32Share->pfScramVec   = V32Share_Dummy;
    pV32Share->pfDiffEncVec = V32Share_Dummy;
    pV32Share->pfEncodeVec  = V32Share_Dummy;
    pV32Share->pfSigMapVec  = V17_TX_SigMap_Send_A;
}

void V17_Send_SILENCE(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    /* setup tx functions */
    pV17->pfDataInVec  = V17_Dummy;

    pV32Share->pfScramVec   = V32Share_Dummy;
    pV32Share->pfDiffEncVec = V32Share_Dummy;
    pV32Share->pfEncodeVec  = V32Share_Dummy;
    pV32Share->pfSigMapVec  = V32_SigMapResetData;
}

void V17_Send_seg1(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    /* setup tx functions */
    pV17->pfDataInVec  = V17_Dummy;

    pV32Share->pfScramVec   = V32Share_Dummy;
    pV32Share->pfDiffEncVec = V32Share_Dummy;
    pV32Share->pfEncodeVec  = V32Share_Dummy;
    pV32Share->pfSigMapVec  = V17_TX_SigMap_Send_AB;
}

void V17_Send_seg2(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    V32_ScramMaskInit(pV32Share);
    /* Initial Scrambler state */
    pV32Share->udScramSReg = 0xabb37400;

    /* setup tx functions */
    pV17->pfDataInVec = V17_TX_DataIn_Send_eq_training;

    pV32Share->pfScramVec   = V32_ScramUsingGPC;
    pV32Share->pfDiffEncVec = V32_ByPassDiffEnc;
    pV32Share->pfEncodeVec  = V32_BypassTrellisEncoder;
    pV32Share->pfSigMapVec  = V32_SigMap;
}

void V17_Send_seg3(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    /* V32_ScramMaskInit(pV32Share); */ /* Harryc */
    V32_DiffEncInit(pV32Share);
    pV32Share->ubDiffEncState = 0x03;
    pV17->ubSeg3Cnt = 0;

    /* setup tx functions */
    pV17->pfDataInVec  = V17_TX_DataIn_bridge;

    pV32Share->pfScramVec   = V32_ScramUsingGPC;
    pV32Share->pfDiffEncVec = V32_DiffEnc;
    pV32Share->pfEncodeVec  = V32_BypassTrellisEncoder;
    pV32Share->pfSigMapVec  = V32_SigMap;
}

void V17_Send_seg4(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);
    UBYTE ubTemp;

    ubTemp = pV17->ubTxRateIdx + 3;

    pV32Share->ubTxBitsPerSym = ubTemp;
    pV32Share->uScramInbits   = (1 << ubTemp) - 1;

    if (pV17->ubTxRateIdx == V17_RATE_IDX_14400)
    {
        pV32Share->pcSigMapTab = cV32_IQTab_14400T;
    }
    else if (pV17->ubTxRateIdx == V17_RATE_IDX_12000)
    {
        pV32Share->pcSigMapTab = cV32_IQTab_12000T;
    }
    else if (pV17->ubTxRateIdx == V17_RATE_IDX_9600)
    {
        pV32Share->pcSigMapTab = cV32_IQTab_9600T;
    }
    else /* if (pV17->ubTxRateIdx == V17_RATE_IDX_7200) */
    {
        pV32Share->pcSigMapTab = cV32_IQTab_7200T;
    }

    V32_ScramMaskInit(pV32Share);     /* Harryc */
    V32_EncodeInit(pV32Share);
    pV32Share->pDiffEncTab    = V32_DiffEncTab_T;
    pV32Share->ubDiffEncState = 0x02;

    /* setup tx functions */
    pV17->pfDataInVec  = V17_Dummy;

    pV32Share->pfScramVec   = V32_ScramUsingGPC;
    pV32Share->pfDiffEncVec = V32_DiffEnc;
    pV32Share->pfEncodeVec  = V32_TrellisEncode;
    pV32Share->pfSigMapVec  = V32_SigMap;
}

/* ----- Send segment TCF ----- */
void V17_Send_TCF(V17Struct *pV17)
{
    V32ShareStruct *pV32Share = &(pV17->V32Share);

    //V32_ScramMaskInit(pV32Share);      /* Harryc */

    /* setup tx functions */
    pV17->pfDataInVec  = V17_TX_DataIn_Send_TCF;

    pV32Share->pfScramVec   = V32_ScramUsingGPC;
    pV32Share->pfDiffEncVec = V32_DiffEnc;
    pV32Share->pfEncodeVec  = V32_TrellisEncode;
    pV32Share->pfSigMapVec  = V32_SigMap;
}
