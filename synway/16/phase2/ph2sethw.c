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
#include "ph2ext.h"
#include "mhsp.h"   /* Modem Host Signal Processor */
#include "ptable.h" /* Data memory table defines   */
#include "hwapi.h"  /* Hardware Defines            */
#include "gaoapi.h"


/**********************************************************************
* Setup Codec interrupt sampling frequence and buffer size for phase 2.
**********************************************************************/

void Ph2_HW_Setup(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    UBYTE     *pHwApi = (UBYTE *)(pTable[HWAPIDATA_IDX]);
    SWORD     *pPCMin = (SWORD *)(pTable[PCMINDATA_IDX]);
    SWORD    *pPCMout = (SWORD *)(pTable[PCMOUTDATA_IDX]);
    UBYTE i;

    pMhsp->RtCallRate = MODEM_CALL_16_96;    /* Can't removed when retrain back from V34, V90a or V90d. */
    pMhsp->RtBufSize  = MODEM_BUFSIZE_16;

    for (i = 0; i < 96; i++) /* 96 = MODEM_CALL_16_96 * MODEM_BUFSIZE_16 */
    {
        *pPCMin++  = 0;
        *pPCMout++ = 0;
    }

    pHwApi[0] = HWAPI_SAMPLERATE | HWAPI_BUFSIZE;
    pHwApi[1] = HWAPI_9600SR;
    pHwApi[2] = pMhsp->RtCallRate * pMhsp->RtBufSize;
}
