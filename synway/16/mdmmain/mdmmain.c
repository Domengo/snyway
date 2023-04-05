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

#include <string.h>
#include "commdef.h"
#include "mdmmain.h"
#include "dceext.h"

/* Modem system initialization functions */
void MDM_Init(UBYTE *pModemDataMem)
{
    GaoModemStruct *pModem = (GaoModemStruct *)pModemDataMem;
    UBYTE **pTable = pModem->m_pTable;

    TRACE1("Total structure size=%d", sizeof(GaoModemStruct));

    /* Initialize Modem data memory */
    memset(pModem, 0, sizeof(GaoModemStruct));

#if (SAMCONV + V27_HW7200SAMPLERATE_SUPPORT + CID_HW7200SAMPLERATE_SUPPORT)
    pTable[SMPDATAIN_IDX]         = (UBYTE *) & (pModem->m_SmpDataIn);
    pTable[SMPDATAOUT_IDX]        = (UBYTE *) & (pModem->m_SmpDataOut);
#endif
    pTable[MHSP_STRUC_IDX]        = (UBYTE *) & (pModem->m_Mhsp);
    pTable[HW_STRUC_IDX]          = (UBYTE *) & (pModem->m_Hw);
    pTable[PCMINDATA_IDX]         = (UBYTE *)pModem->m_PcmIn;
    pTable[PCMOUTDATA_IDX]        = (UBYTE *)pModem->m_PcmOut;
    pTable[HOSTMSGDATA_IDX]       = (UBYTE *)pModem->m_HostMessage;
    pTable[DPCS_STRUC_IDX]        = (UBYTE *) & (pModem->m_Dpcs);
    pTable[HWAPIDATA_IDX]         = (UBYTE *) & (pModem->m_HwApiData);
#if WITH_DC
    pTable[DC_STRUC_IDX]          = (UBYTE *) & (pModem->m_Dc);
#endif
#if DRAWEPG
    pTable[EPGDATA_IDX]           = (UBYTE *) & (pModem->m_Epg);
#endif
    pTable[DSPIORDBUFDATA_IDX]    = (UBYTE *)pModem->m_DSPIoReadBuf;
    pTable[DSPIOWRBUFDATA_IDX]    = (UBYTE *)pModem->m_DSPIoWriteBuf;
    pTable[CLOCKDATA_IDX]         = (UBYTE *) & (pModem->m_ClockData);
    pTable[ACE_STRUC_IDX]         = (UBYTE *) & (pModem->m_Ace);
    pTable[ACESREGDATA_IDX]       = (UBYTE *) & (pModem->m_AceSRegister);
    pTable[ASCCBWRDATA_IDX]       = (UBYTE *) & (pModem->m_ASCWriteBuffer);
    pTable[ASCCBRDDATA_IDX]       = (UBYTE *) & (pModem->m_ASCReadBuffer);
    pTable[ASCTXDATA_IDX]         = (UBYTE *) & (pModem->m_ASCWriteMemory);
    pTable[ASCRXDATA_IDX]         = (UBYTE *) & (pModem->m_ASCReadMemory);
#if SUPPORT_V54
    pTable[V54_STRUC_IDX]         = (UBYTE *) & (pModem->m_V54);
#endif
#if SUPPORT_FAX_DATAPUMP
    pTable[FAXSHAREMEM_IDX]       = (UBYTE *) & (pModem->m_FaxShareMem);
#endif
#if SUPPORT_V42INFO
    pTable[V42INFO_STRUC_IDX]     = (UBYTE *) & (pModem->m_V42Info);
#endif
    pTable[DCE_STRUC_IDX]         = (UBYTE *) & (pModem->m_Dce);
#if 0 /// LLL temp
#if (SUPPORT_MODEM || SUPPORT_V34FAX)
    pTable[DCE_SHAREDATA_IDX]     = (UBYTE *) & (pModem->m_Protocol);
#endif
#endif
    pTable[MODEM_SHAREDATA_IDX]   = (UBYTE *) & (pModem->m_DataPump);
#if (SUPPORT_V8 + SUPPORT_V22_OR_B212 + SUPPORT_V32 + SUPPORT_V32BIS)
    pTable[MODEM_OFFSET_AUTO_IDX] = (UBYTE *) & (pModem->m_DataPump.AutoShare.V8V22V32);
#endif
#if USE_ASM
    pTable[CIRCDATA_IDX]          = (UBYTE *)pModem->m_CircData;
#endif

    /* Modem system initialization */
    HwInit(pTable);
    FastMhspInit(pTable);
    DCEInit(pTable);
}

void MDM_Proc(UBYTE *pModemDataMem, QWORD *PCM_in, QWORD *PCM_out)
{
    UBYTE **pTable = (UBYTE **)pModemDataMem;

#if !DUMPOTHERDTEOUTFILE /* Must delete the code when dump data from USR to DTE */
    DCEMain(pTable);
    ACEMain(pTable);
#endif

    HwXferIn(pTable, PCM_in);/* Sample rate process */

#if !DUMPOTHERDTEOUTFILE /* Must delete the code when dump data from USR to DTE */
    MhspMain(pTable);

    HwXferOut(pTable, PCM_out);/* Sample rate process */
#endif
}
