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

/* send tones */
void CP_Mod_Dummy(CpStruct *pCp)
{
    int i;

    /* generate silence */
    for (i = 0; i < CP_BUF_SIZE; i++)
    {
        pCp->PCMoutPtr[i] = 0;
    }
}

SBYTE DialChangeToDigital(CHAR DialNum)
{
    if (DialNum >= '0' && DialNum <= '9')
    {
        return ((UBYTE)(DialNum - 0x30));
    }
    else if (DialNum == 'a' || DialNum == 'A')
    {
        return (10);
    }
    else if (DialNum == 'b' || DialNum == 'B')
    {
        return (11);
    }
    else if (DialNum == 'c' || DialNum == 'C')
    {
        return (12);
    }
    else if (DialNum == 'd' || DialNum == 'D')
    {
        return (13);
    }
    else if (DialNum == '*')
    {
        return (14);
    }
    else if (DialNum == '#')
    {
        return (15);
    }
    else
    {
        return (-1);
    }
}

UBYTE InitDtmfGen(DtmfGenStruct *pDtmf, CHAR DialNum)
{
    SBYTE sbDigital;

    sbDigital = DialChangeToDigital(DialNum);

    if (sbDigital < 0)
    {
        /* TRACE0("Not dial number!"); */
        return (0);
    }

    pDtmf->bGen_Flag = 1;

#if DTMF_GEN_INDEPENDENT
    pDtmf->OnTime  = 0;
    pDtmf->OffTime = 0;
    pDtmf->bOn     = 1;
#endif

    pDtmf->LowFreq   = DtmfDigitRow[sbDigital];
    pDtmf->HighFreq  = DtmfDigitCol[sbDigital];

    pDtmf->LowPhase  = 0;
    pDtmf->HighPhase = 0;

    return (1);
}

void DtmfGeneration(DtmfGenStruct *pDtmf, QWORD *PcmOut)
{
    QWORD qTone1, qTone2;
    UBYTE i;

    for (i = 0; i < CP_BUF_SIZE; i++)
    {
        /* value for tone 1 */
        qTone1 = DSP_tCOS_TABLE[pDtmf->LowPhase >> 8];
        pDtmf->LowPhase += pDtmf->LowFreq;

        /* value for tone 2 */
        qTone2 = DSP_tCOS_TABLE[pDtmf->HighPhase >> 8];
        pDtmf->HighPhase += pDtmf->HighFreq;

        /* store DTMF output */
        PcmOut[i] = QQMULQ15(qTone1, pDtmf->qLowLevelRef) + QQMULQ15(qTone2, pDtmf->qHighLevelRef);
    }
}

void CP_Mod(CpStruct *pCp)
{
    DtmfGenStruct *pDtmf = &pCp->Dtmf;
    QWORD qTone;
    UBYTE i;

    if (pDtmf->bGen_Flag)
    {
        DtmfGeneration(pDtmf, pCp->PCMoutPtr);
    }
    else if (pCp->GenTone_Flag)
    {
        /* generate a tone */
        for (i = 0; i < CP_BUF_SIZE; i++)
        {
            qTone = DSP_tCOS_TABLE[pCp->GenTonePhase >> 8];
            pCp->GenTonePhase += pCp->GenToneFreq;

            /* store tone output */
            pCp->PCMoutPtr[i] = QQMULQ15(qTone, qSINGLE_TONE_12DBM0);/* -12dBm0 */
        }
    }
    else
    {
        /* generate SILENCE */
        for (i = 0; i < CP_BUF_SIZE; i++)
        {
            pCp->PCMoutPtr[i] = 0;
        }
    }
}

#if DTMF_GEN_INDEPENDENT
void GenDtmf(DtmfGenStruct *pDtmf, SWORD *pPCMout)/* generate CP_BUF_SIZE data per call (10ms) */
{
    UBYTE i;

    if (pDtmf->bGen_Flag == 1)
    {
        if (pDtmf->bOn == 1)
        {
            pDtmf->OnTime++;

            if (pDtmf->OnTime == 7)    /* 70ms */
            {
                pDtmf->bOn = 0;
            }

            DtmfGeneration(pDtmf, pPCMout);
        }
        else
        {
            for (i = 0; i < CP_BUF_SIZE; i++)
            {
                pPCMout[i] = 0;
            }

            pDtmf->OffTime++;

            if (pDtmf->OffTime == 7)
            {
                pDtmf->bGen_Flag = 0;
                pDtmf->OffTime = 0;
                pDtmf->OnTime  = 0;
            }
        }
    }
}
#endif
