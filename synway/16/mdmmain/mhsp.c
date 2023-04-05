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

/* ---------------------------
Modem Host Signal Processing.
---------------------------- */

#include <string.h>
#include "ptable.h"
#include "mhsp.h"
#include "ioapi.h"
#include "hwapi.h"
#include "gaoapi.h"
#include "circbuff.h"
#include "mdmmain.h"
#include "aceext.h"
#include "dceext.h"

#if BER_CALC
UDWORD udZeroCount = 0;
UDWORD udTotalCount = 0;
#endif

/* Scale is in Q3.13 format */
/* This table is based on modem signal level all fixed to -12dBm0. */
CONST QWORD qMdm_AttenuationTable[24] =
{
    32613,   /* 0dBm0 */
    29066,
    25905,
    23088,
    20577,
    18340,
    16345,
    14568,
    12983,
    11572,
    10313,
    9192,
    8192,    /* -12dBm0, No modification to transmit signal level */
    7301,
    6507,
    5799,
    5169,
    4607,
    4106,
    3659,
    3261,
    2907,
    2591,
    2309     /* -23dBm0 */
};

void MhspIdle(UBYTE **pTable)
{
    UBYTE      *pHwApi  = (UBYTE *)pTable[HWAPIDATA_IDX];
    ACEStruct  *pAce    = (ACEStruct *)pTable[ACE_STRUC_IDX];
    MhspStruct *pMhsp   = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    SWORD      *pPCMin  = (SWORD *)pTable[PCMINDATA_IDX];
    SWORD      *pPCMout = (SWORD *)pTable[PCMOUTDATA_IDX];
    DpcsStruct *pDpcs   = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE       Mcf, CPf, modType;
    UBYTE       i;

    if (pMhsp->Total_TimeOut_Count > 0)
    {
        pMhsp->Total_TimeOut_Count++;
    }

    if (pMhsp->Total_TimeOut_Count == 2000)
    {
        TRACE0("Timeout, ACE init");

        pHwApi[0] = HWAPI_ONHOOK;
        Ace_ChangeLineState(pAce, OFF_LINE);

        ACE_FactoryConfigurationInit(pTable);
        Ace_SetClass1ResultCode(pAce, RESULT_CODE_ERROR);
    }

    // fixing issue 195
    if (pAce->classState == ACE_FRS)
    {
        QDWORD qdEgy = 0;

        for (i = 0; i < PCM_BUFSIZE; i++)
        {
            qdEgy += QQMULQD(pPCMin[i], pPCMin[i]);

            if (qdEgy > 9000000)
            {
                break;
            }
        }

        if (qdEgy <= 9000000)
        {
            pAce->timerSilence++;
        }
        else
        {
            pAce->timerSilence = 0;
        }
    }

    /* clear buffers */
    for (i = 0; i < PCM_BUFSIZE; i++) //pMhsp->RtBufSize
    {
        pPCMin[i]  = 0;
        pPCMout[i] = 0;
    }

    Mcf = GetReg(pDpcs->MCF);

    if (Mcf & DISCONNECT)
    {
        ClrReg(pDpcs->MCF);
        PutReg(pDpcs->MSR0, DISC_DET);
    }

    /* check connect state */
    CPf = RdReg(pDpcs->MMR0, CP);

    if (Mcf & LINE_DISCONNECT)
    {
        pMhsp->pfRealTime = Mhsp_LineDisconnect;
        ClrReg(pDpcs->MCF);

        pMhsp->Total_TimeOut_Count = 0;
    }
    else if (CPf && (Mcf & CONNECT))
    {
        /* Initialize default sampling rate */
        /* pHwApi[0] = HWAPI_SAMPLERATE;
           pHwApi[1] = HWAPI_9600SR;
        */
        pMhsp->pfRealTime = Cp_DPCS_Init; /* initialize vectors */
        ResetReg(pDpcs->MMR0, CP);         /* reset CP flag */
        ClrReg(pDpcs->MCF);                /* reset connect flag */

        ClrReg(pDpcs->MSR0);               /* Clear MSR0 status */

        pMhsp->Total_TimeOut_Count = 1;
    }
    else if (Mcf & CONNECT)
    {
        /* select the modem */
        modType = GetReg(pDpcs->MMR0);

        switch (modType)
        {
#if (SUPPORT_V92A + SUPPORT_V92D)
            case MODEM_V92:
                pMhsp->pfRealTime = V8_DPCS_Init;
                break;
#endif
#if (SUPPORT_V90A + SUPPORT_V90D)
            case MODEM_V90:
                pMhsp->pfRealTime = V8_DPCS_Init;
                break;
#endif
#if SUPPORT_V21M
            case MODEM_V21:
                pMhsp->pfRealTime = V21_DPCS_Init;
                break;
#endif
#if SUPPORT_V23
            case MODEM_V23:
                pMhsp->pfRealTime = V23_DPCS_Init;
                break;
#endif
#if (SUPPORT_V22 + SUPPORT_V22BIS)
            case MODEM_V22:
            case MODEM_V22bis:
                pMhsp->pfRealTime = V22_DPCS_Init;
                break;
#endif
#if (SUPPORT_V32 + SUPPORT_V32BIS)
            case MODEM_V32:
            case MODEM_V32bis:
                pMhsp->pfRealTime = V32_DPCS_Init;
                break;
#endif
#if SUPPORT_V34
            case MODEM_V34:
#if SUPPORT_V54
                modType = GetReg(pDpcs->MTC);

                if (modType == LAL_MODE || modType == LDL_MODE)
                {
                    pMhsp->pfRealTime = V34_LAL_Startup;
                }
                else
#endif
                    pMhsp->pfRealTime = V8_DPCS_Init;

                break;
#endif
#if SUPPORT_V34FAX
            case V34FAX:
                pMhsp->pfRealTime = V8_DPCS_Init;
                break;
#endif
#if SUPPORT_V17
            case V17:      pMhsp->pfRealTime =      V17_DPCS_Init; break;
#endif
#if SUPPORT_V27TER
            case V27TER:   pMhsp->pfRealTime =      V27_DPCS_Init; break;
#endif
#if SUPPORT_V29
            case V29:      pMhsp->pfRealTime =      V29_DPCS_Init; break;
#endif
#if SUPPORT_V21CH2FAX
            case V21CH2:   pMhsp->pfRealTime =   V21ch2_DPCS_Init; break;
#endif
#if SUPPORT_B103
            case BELL103:  pMhsp->pfRealTime =  BELL103_DPCS_Init; break;
#endif
#if SUPPORT_B212A
            case BELL212A: pMhsp->pfRealTime = BELL212A_DPCS_Init; break;
#endif
#if SUPPORT_AUTO
            default:       pMhsp->pfRealTime =     AUTO_DPCS_Init; break;
#endif
        }

        ClrReg(pDpcs->MCF);          /* reset connect flag */

        pMhsp->Total_TimeOut_Count = 1;
    }

#if (SUPPORT_CID_DET + SUPPORT_CID_GEN)
    modType = RdReg(pDpcs->TELCOCR1, CID_CONTROL | CIDGEN_CONTROL);

#if SUPPORT_CID_DET

    if (modType & 0xC0)
    {
        pMhsp->pfRealTime = Cid_Dpcs_Init;

        pMhsp->Total_TimeOut_Count = 1;
    }

#endif

#if SUPPORT_CID_GEN

    if (modType & 0x03)
    {
        pMhsp->pfRealTime = CidGen_Dpcs_Init;

        pMhsp->Total_TimeOut_Count = 1;
    }

#endif

#endif

#if SUPPORT_DTMF
    modType = RdReg(pDpcs->DTMFR, DTMF_ENABLE);

    if (modType == DTMF_ENABLE)
    {
        pMhsp->pfRealTime = Dtmf_Dpcs_Init;

        pMhsp->Total_TimeOut_Count = 1;
    }

#endif
}

void Mhsp_LineDisconnect(UBYTE **pTable)
{
    UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];

    /* Initialize default sampling rate */
    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;
    pHwApi[1] = HW_DEF_SAM_RATE;
    pHwApi[2] = HW_DEF_BUFSIZE;

    /* initialize vectors */
    pMhsp->pfRealTime = MhspIdle;

    PutReg(pDpcs->MSR0, LINE_DISC_DET);
    ClrReg(pDpcs->MCF);

#if BER_CALC

    if (udTotalCount != 0)
    {
        TRACE1("TotalBit = %e", ((float)udTotalCount));
        TRACE1("BER = %e", ((float)udZeroCount / udTotalCount));
    }
    else
    {
        TRACE0("udTotalCount=0");
    }

    udZeroCount  = 0;
    udTotalCount = 0;
#endif
}

void Disconnect_Init(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];
#if DRAWEPG
    SWORD      *pEpg  = (SWORD *)pTable[EPGDATA_IDX];
#endif

    pMhsp->RtCallRate = 1;

    pMhsp->RtBufSize = HW_DEF_BUFSIZE;

    /* init the circbuffers */
    CB_InitCircBuffer(pAscCBRd, pTable[ASCRXDATA_IDX], BUFSIZE);

    /* initialize vectors */
    pMhsp->pfRealTime = MhspIdle;

    /* Pay attention to: Must not change the sample rate in here! */

    /* initialize EPG counter */
#if DRAWEPG
    *pEpg = 1;
#endif

    /* write disc. status and clear DATA bit */
    PutReg(pDpcs->MSR0, DISC_DET);

    /* clear some DPCS status registers */
    ClrReg(pDpcs->MBSR);
    ClrReg(pDpcs->MBSC);
    ClrReg(pDpcs->MBSCTX);
    ClrReg(pDpcs->MMSR0);
    ClrReg(pDpcs->TONESR);
    ClrReg(pDpcs->MTONESR);
    ClrReg(pDpcs->DTMFR);
    ClrReg(pDpcs->LECR);
    ClrReg(pDpcs->AECR);

    /* clear some DPCS control registers */
    /* ClrReg(pDpcs->MCF); */

    ClrReg(pDpcs->MFSR0);
}

void Reset_Init(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    union unDataPump *pDataPump = (union unDataPump *)pTable[MODEM_SHAREDATA_IDX];

    memset(pDpcs, 0, sizeof(DpcsStruct));

    memset(pDataPump, 0, sizeof(union unDataPump));

    ACE_FactoryConfigurationInit(pTable);

    HwInit(pTable);
    FastMhspInit(pTable);
    DCEInit(pTable);
}

/* Fast initialization. Reduce cycle from 5 to 1. */
void FastMhspInit(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];
    UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];

#if DRAWEPG
    SWORD *pEpg = (SWORD *)pTable[EPGDATA_IDX];
#endif

    pMhsp->RtCallRate = 1;

    pMhsp->RtBufSize = HW_DEF_BUFSIZE;

    /* init the circbuffers */
    CB_InitCircBuffer(pAscCBRd, pTable[ASCRXDATA_IDX], BUFSIZE);

    /* initialize vectors */
    pMhsp->pfRealTime = MhspIdle;

    /* initialize EPG counter */
#if DRAWEPG
    *pEpg = 1;
#endif

    /* Initialize default sampling rate */
    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;

    pHwApi[1] = HW_DEF_SAM_RATE;
    pHwApi[2] = HW_DEF_BUFSIZE;

#if BER_CALC

    if (udTotalCount != 0)
    {
        TRACE1("TotalBit = %e", ((float)udTotalCount));
        TRACE1("BER = %e", ((float)udZeroCount / udTotalCount));
    }
    else
    {
        TRACE0("udTotalCount=0");
    }

    udZeroCount  = 0;
    udTotalCount = 0;
#endif
}

void MhspInit(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];

    pMhsp->RtCallRate = 1;

    pMhsp->RtBufSize = HW_DEF_BUFSIZE;

    pMhsp->pfRealTime = Disconnect_Init;
}

void MhspMain(UBYTE **pTable)
{
#if SUPPORT_V54 || SUPPORT_ENERGY_REPORT
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
#endif
    SWORD     *pPCMin = (SWORD *)pTable[PCMINDATA_IDX];
    SWORD    *pPCMout = (SWORD *)pTable[PCMOUTDATA_IDX];
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    HwStruct     *pHw = (HwStruct *)pTable[HW_STRUC_IDX];
    UBYTE      *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    SWORD *pPCMinTmp, *pPCMoutTmp;
    UBYTE i, j, CallRate, BufSize, ubSamplesLeft;
    QWORD qGain;
    UBYTE ubSregTransmitPower;

    pPCMinTmp  = pPCMin;
    pPCMoutTmp = pPCMout;

    /* call vectors */
    CallRate = pMhsp->RtCallRate;
    BufSize  = pMhsp->RtBufSize;

    /* Digital loopback test mode */
#if SUPPORT_V54
    i = RdReg(pDpcs->MTC, LDL_MODE);

    if (i == LDL_MODE)
    {
        memcpy(pPCMin, pPCMout, UBUBMULUB(UBUBMULUB(CallRate, BufSize), sizeof(SWORD)));
    }

#endif

#if SUPPORT_ENERGY_REPORT
    ClrReg(pDpcs->ENERGY);
#endif

    for (i = 0; i < CallRate; i++)
    {
        pMhsp->pfRealTime(pTable);

#if DRAWEPG
        draw_EPG(pTable);
#endif
        pPCMinTmp  += BufSize;
        pPCMoutTmp += BufSize;
        pTable[PCMINDATA_IDX]  = (UBYTE *)pPCMinTmp;
        pTable[PCMOUTDATA_IDX] = (UBYTE *)pPCMoutTmp;

        if (CallRate != pMhsp->RtCallRate)
        {
            ubSamplesLeft = UBUBMULUB(CallRate - i - 1, BufSize);

            for (j = 0; j < ubSamplesLeft; j++)
            {
                pPCMoutTmp[j] = 0;
            }

            break;
        }
    }

    ubSregTransmitPower = pSreg[TRANSMIT_POWER];

    if ((ubSregTransmitPower != 12) && (ubSregTransmitPower <= 23))
    {
        qGain = qMdm_AttenuationTable[ubSregTransmitPower];

        for (i = 0; i < pHw->BufferSize; i++)
        {
            pPCMout[i] = (QWORD)((QQMULQD(pPCMout[i], qGain) + 0x1000) >> 13);
        }
    }

    pTable[PCMINDATA_IDX]  = (UBYTE *)pPCMin;
    pTable[PCMOUTDATA_IDX] = (UBYTE *)pPCMout;
}

#if SUPPORT_B212A
void BELL212A_DPCS_Init(UBYTE **pTable)
{
    V22_DPCS_Init(pTable);
    TRACE0("V22 called for BELL212A");
}
#endif
