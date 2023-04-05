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

#include "v34ext.h"
#include "mhsp.h"   /* Modem Host Signal Processor */
#include "ptable.h" /* Data memory table defines   */
#include "hwapi.h"  /* Hardware Defines            */
#include "gaoapi.h"

#if 0 /// LLL temp
/***********************************************************
* Setup Codec interrupt sampling frequency and buffer size
* for phase 3, 4 and data modem
***********************************************************/
void V34_HW_Setup(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    UBYTE *pHwApi = (UBYTE *)(pTable[HWAPIDATA_IDX]);
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    SWORD *pPCMin  = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    UBYTE i;

    for (i = 0; i < pMhsp->RtBufSize; i++)
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

    /* Adjusting CODEC driver buffer size, to match 10ms buffer timing */
    /* The selected buffer size will affect the system delay, and need */
    /* to pay attention specially to Echo canceller system delay init. */
    /* The selected buffer size for a specific sampling rate will also */
    /* affect the number of symbols to be processed, and an adjustment */
    /* to the RTD_factor is required. Please refer to "v34glbtbl.c".   */
#if 0 /* SAMCONV */
    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;
    pHwApi[2] = 96;

    switch (pV34->tx_symbol_rate)
    {
        case V34_SYM_2400:
            pHwApi[1] = HWAPI_7200SR;/* 96 <-> 72 */
            pMhsp->RtBufSize  = 72;
            pV34->NumSymbol = 24;
            break;

        case V34_SYM_2743:
            pHwApi[1] = HWAPI_8229SR;/* 84 <-> 72 */
            pHwApi[2] = 84;           /* This is special case */
            pMhsp->RtBufSize  = 72;
            pV34->NumSymbol = 24;
            break;

        case V34_SYM_2800:
            pHwApi[1] = HWAPI_8400SR;/* 96 <-> 84 */
            pMhsp->RtBufSize  = 84;
            pV34->NumSymbol = 28;
            break;

        case V34_SYM_3000:
            pHwApi[1] = HWAPI_9000SR;/* 96 <-> 90 */
            pMhsp->RtBufSize  = 90;
            pV34->NumSymbol = 30;
            break;

        case V34_SYM_3200:
            pHwApi[1] = HWAPI_9600SR;/* 96 <-> 96 */
            pMhsp->RtBufSize  = 96;
            pV34->NumSymbol = 32;
            break;

        case V34_SYM_3429:
            pHwApi[1] = HWAPI_10287SR;/* 84 <-> 90 */
            pHwApi[2] = 84;           /* This is special case */
            pMhsp->RtBufSize  = 90;
            pV34->NumSymbol = 30;
            break;
    }

#else
    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;

    switch (pV34Rx->tx_symbol_rate)
    {
        case V34_SYM_2400:
            pHwApi[1] = HWAPI_7200SR;/* 10ms <-> 72 */
            pHwApi[2] = 72;
            pMhsp->RtBufSize  = 72;
            pV34->NumSymbol = 24;
            break;

        case V34_SYM_2743:
            pHwApi[1] = HWAPI_8229SR;/* 10.2ms <-> 84 */
#if SAMCONV
            pHwApi[2] = 72;
            pMhsp->RtBufSize = 72;
            pV34->NumSymbol  = 24;
#else
    pHwApi[2] = 84;
    pMhsp->RtBufSize = 84;
    pV34->NumSymbol  = 28;
#endif
            break;

        case V34_SYM_2800:
            pHwApi[1] = HWAPI_8400SR;/* 10ms <-> 84 */
            pHwApi[2] = 84;
            pMhsp->RtBufSize  = 84;
            pV34->NumSymbol = 28;
            break;

        case V34_SYM_3000:
            pHwApi[1] = HWAPI_9000SR;/* 10ms <-> 90 */
            pHwApi[2] = 90;
            pMhsp->RtBufSize  = 90;
            pV34->NumSymbol = 30;
            break;

        case V34_SYM_3200:
            pHwApi[1] = HWAPI_9600SR;/* 10ms <-> 96 */
            pHwApi[2] = 96;
            pMhsp->RtBufSize  = 96;
            pV34->NumSymbol = 32;
            break;

        case V34_SYM_3429:
            pHwApi[1] = HWAPI_10287SR;/* 9.3ms <-> 96 */
#if SAMCONV
            pHwApi[2] = 90;
            pMhsp->RtBufSize  = 90;
            pV34->NumSymbol = 30;
#else
    pHwApi[2] = 96;
    pMhsp->RtBufSize  = 96;
    pV34->NumSymbol = 32;
#endif
            break;
    }

#endif

    /* In case of sampling rate converter is used, */
    /* buffer size depends on current used sampling */
    /* rate.                                       */
    /* pMhsp->RtCallRate = 1; */
}
#endif