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

#ifndef _MDMMAIN_H
#define _MDMMAIN_H

#include "ptable.h"
#if SUPPORT_AUTO
#include "autostru.h"
#endif
#if SUPPORT_B103
#include "b103stru.h"
#endif
#if SUPPORT_DTMF
#include "dtmfstru.h"
#endif
#if SUPPORT_CID_DET
#include "cidstru.h"
#endif
#if SUPPORT_CID_GEN
#include "cidgen.h"
#endif
#include "circbuff.h"
#include "cpstru.h"
#include "dcestru.h"
#include "hwapi.h"
#include "hwstru.h"
#include "mhsp.h"
#include "sregdef.h"
#if SUPPORT_MNP
#include "mnpstru.h"
#endif
#if (SUPPORT_V34 || SUPPORT_V34FAX || SUPPORT_V90A || SUPPORTV90D)
#include "ph2stru.h"
#endif
#if SUPPORT_FAX_DATAPUMP
#include "faxshmem.h"
#endif
#if SUPPORT_V17
#include "v17stru.h"
#endif
#if SUPPORT_V21
#include "v21stru.h"
#endif
#if SUPPORT_V22_OR_B212
#include "v22stru.h"
#endif
#if SUPPORT_V23
#include "v23stru.h"
#endif
#if SUPPORT_V27TER
#include "v27stru.h"
#endif
#if SUPPORT_V29
#include "v29stru.h"
#endif
#if (SUPPORT_V32 + SUPPORT_V32BIS)
#include "v32stru.h"
#endif
#if SUPPORT_V34
#include "v34stru.h"
#endif
#if SUPPORT_V34FAX
#include "v34fstru.h"
#endif
#if SUPPORT_V42
#include "v42stru.h"
#endif
#if SUPPORT_V54
#include "v54.h"
#endif
#if SUPPORT_V8
#include "v8stru.h"
#endif
#if SUPPORT_V90A
#include "v90astru.h"
#endif
#if SUPPORT_V90D
#include "v90dstru.h"
#endif
#if (SAMCONV + V27_HW7200SAMPLERATE_SUPPORT + CID_HW7200SAMPLERATE_SUPPORT)
#include "smp.h"
#endif

#define CLOCKDATA_SIZE      (2)/* Current clock is 16bit only */
#define ACESREGDATA_SIZE    (NUMBER_S_REGISTERS)    /* Number of S registers from sregdef.h */
#define HWAPIDATA_SIZE      (3)
#define PCMINDATA_SIZE      (PCM_BUFSIZE)
#define PCMOUTDATA_SIZE     (PCM_BUFSIZE)
#define DSPIORDBUFDATA_SIZE (72)     /* 560/8=70 -> 72, Assume max #bit in 10ms is 560 bits */
#define DSPIOWRBUFDATA_SIZE (72)     /* 560/8=70 -> 72, Assume max #bit in 10ms is 560 bits */
#define HOSTMSGDATA_SIZE    (80)     /* Dial telephone Number length, 80 is AT command buffer size */
#define ASCTXDATA_SIZE      (BUFSIZE)/* BUFSIZE relates with the ACE, V42, MNP & MdmMmain */
#define ASCRXDATA_SIZE      (BUFSIZE)
#define DTETXDATA_SIZE      (BUFSIZE)
#define DTERXDATA_SIZE      (BUFSIZE)
#if DRAWEPG
#define EPGDATA_SIZE        (1024)   /* sizeof(QWORD)*display elements(512) ? */
#endif

#if (SUPPORT_AUTO + SUPPORT_V8 + SUPPORT_V22_OR_B212 + SUPPORT_V32 + SUPPORT_V32BIS)
typedef struct _AutoShareStruc
{
#if SUPPORT_AUTO
    AutoStruct m_Auto;
#endif

#if (SUPPORT_V8 + SUPPORT_V22_OR_B212 + SUPPORT_V32 + SUPPORT_V32BIS)
    union
    {
#if SUPPORT_V8
        V8Struct     m_V8;
#endif
#if SUPPORT_V22_OR_B212
        V22Struct    m_V22;
#endif
#if (SUPPORT_V32 + SUPPORT_V32BIS)
        V32Struct    m_V32;
#endif
    } V8V22V32;
#endif

} AutoShareStruct;
#endif

typedef struct _GaoModemStruc
{
    UBYTE *m_pTable[PTABLE_SIZE];

#if USE_ASM
    QWORD Alignment[32+32+32];

#if (SAMCONV + V27_HW7200SAMPLERATE_SUPPORT + CID_HW7200SAMPLERATE_SUPPORT)
    QWORD m_SmpDataIn[64+46+(64-46)+46];
    QWORD m_SmpDataOut[64+46+(64-46)+46];
#endif

    QWORD m_CircData[64];

#else

#if (SAMCONV + V27_HW7200SAMPLERATE_SUPPORT + CID_HW7200SAMPLERATE_SUPPORT)
    InDe_Struct m_SmpDataIn;
    InDe_Struct m_SmpDataOut;
#endif

#endif

    MhspStruct m_Mhsp;

    HwStruct m_Hw;

    QWORD m_PcmIn[PCMINDATA_SIZE];
    QWORD m_PcmOut[PCMOUTDATA_SIZE];

    UBYTE m_HostMessage[HOSTMSGDATA_SIZE];

    DpcsStruct m_Dpcs;

    UBYTE m_HwApiData[HWAPIDATA_SIZE];

#if WITH_DC
    DCStruct m_Dc;
#endif

#if DRAWEPG
    SWORD m_Epg[EPGDATA_SIZE];
#endif

    UBYTE m_DSPIoReadBuf[DSPIORDBUFDATA_SIZE];
    UBYTE m_DSPIoWriteBuf[DSPIOWRBUFDATA_SIZE];

    UWORD m_ClockData[CLOCKDATA_SIZE];

    ACEStruct m_Ace;

    UBYTE m_AceSRegister[ACESREGDATA_SIZE];

    CircBuffer m_ASCWriteBuffer;
    CircBuffer m_ASCReadBuffer;
    UBYTE m_ASCWriteMemory[ASCTXDATA_SIZE];
    UBYTE m_ASCReadMemory[ASCRXDATA_SIZE];

#if SUPPORT_V54
    V54Struct m_V54;
#endif

#if SUPPORT_V42INFO
    V42Info m_V42Info;
#endif

    DceStruct m_Dce;

#if SUPPORT_FAX_DATAPUMP
    FaxShareMemStruct m_FaxShareMem;
#endif

    union unDataPump
    {
#if (SUPPORT_AUTO + SUPPORT_V8 + SUPPORT_V22_OR_B212 + SUPPORT_V32 + SUPPORT_V32BIS)
        AutoShareStruct AutoShare;
#endif

        CpStruct        m_Cp;
#if (SUPPORT_V34 || SUPPORT_V34FAX || SUPPORT_V90A || SUPPORTV90D)
        Ph2Struct       m_Ph2;
#endif
#if SUPPORT_V17
        V17Struct       m_V17;
#endif
#if SUPPORT_V21
        V21Struct       m_V21;
#endif
#if SUPPORT_V23
        V23Struct       m_V23;
#endif
#if SUPPORT_V27TER
        V27Struct       m_V27;
#endif
#if SUPPORT_V29
        V29Struct       m_V29;
#endif
#if SUPPORT_V34
        V34Struct       m_V34;
#endif
#if (SUPPORT_V90A + SUPPORT_V92A)
        V90aStruct      m_V90a;
#endif
#if (SUPPORT_V90D + SUPPORT_V92D)
        V90dStruct      m_V90d;
#endif
#if SUPPORT_CID_DET
        CidStruct       m_Cid;
#endif
#if SUPPORT_CID_GEN
        CidGenStruct    m_CidGen;
#endif
#if SUPPORT_DTMF
        DtmfStruct      m_Dtmf;
#endif
    } m_DataPump;

#if 0
#if SUPPORT_MODEM
    union unProtocol
    {
#if SUPPORT_V42
        V42Struct       m_V42;
#endif
#if SUPPORT_MNP
        MNPStruct       m_Mnp;
#endif
    } m_Protocol;
#endif
#endif
} GaoModemStruct;

#endif
