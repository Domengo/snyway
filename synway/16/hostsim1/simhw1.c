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

/*****************************************************************************
*  The file is designed for Codec simulation only.
*
*  All Functions are created based on Local and Remote modem simulation.
*
*****************************************************************************/

#include <string.h>
#include "commdef.h"
#include "gaoapi.h"
#include "modifdef.h"
#include "hwstru.h"

extern UWORD DumpRingData[];
extern QWORD DumpRingCount;
extern UWORD uRingDetTimer;
extern UBYTE DumpRingCountMax;

struct HawkDef
{
    UWORD OnOffHook;
    SWORD RingOnCount;
    SWORD RingOffCount;
    UWORD BufferSize;
} Hawk;

void HwChgState(GAO_ModStatus *pStatus)
{
    pStatus->RingDetected = 0;

    if (DumpRingCount < DumpRingCountMax)
    {
        if (uRingDetTimer == DumpRingData[DumpRingCount])
        {
            pStatus->RingDetected = 1;
            DumpRingCount++;
        }

        uRingDetTimer++;
    }

    if (pStatus->StateChange)
    {
        if (pStatus->StateChange & HOOK_STATE_CHG)
        {
            pStatus->StateChange &= (~HOOK_STATE_CHG);
        }

#if (SUPPORT_CID_DET + SUPPORT_CID_GEN)

        if (pStatus->HookState & CID_STATE_CHG)
        {
            pStatus->StateChange &= ~CID_STATE_CHG;
        }

#endif

        if (pStatus->StateChange & BUFFER_SIZE_CHG)
        {
            pStatus->StateChange &= (~BUFFER_SIZE_CHG);

            Hawk.BufferSize = pStatus->BufferSize;

            TRACE1("BufferSize=%d", Hawk.BufferSize);
        }

        if (pStatus->StateChange & SAMPLE_RATE_CHG)
        {
            pStatus->StateChange &= (~SAMPLE_RATE_CHG);
        }
    }
}

UBYTE Processor_Side_Initialize(void)
{
    /* Initialize hardware structure */
    Hawk.OnOffHook  = HW_ONHOOK;

    Hawk.BufferSize = HW_DEF_BUFSIZE;

    return TRUE;
}
