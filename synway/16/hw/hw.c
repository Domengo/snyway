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

/********************************************************
HARDWARE LIBRARY
********************************************************/

#include <string.h>
#include "hwstru.h"
#include "gaoapi.h"
#include "ptable.h"
#include "porting.h"
#if SAMCONV
#include "smp.h"
#endif

#if SUPPORT_DUMP_DATA
#include <stdio.h>

UBYTE  DumpOrSimuType;
UBYTE  Case_State;
extern SWORD  DumpPCMinData[];
extern UDWORD DumpPCMinCount;
SWORD  DumpPCMouData[MAX_DATA_SIZE];
UDWORD DumpPCMouCount = 0;
extern UBYTE CaseEndFlag;
extern FILE *PCMinRdFile;
#endif

CONST UBYTE MsClock_Size[8] =
{
    72, 90, 96, 82, 84, 103, 80, 160
};

#if SAMCONV
void SwChgSamRate(UBYTE **pTable, UWORD rate, UWORD bufSize)
{
    HwStruct *pHw = (HwStruct *)pTable[HW_STRUC_IDX];

    InDe_Struct *pMultiFilterOut = (InDe_Struct *)(pTable[SMPDATAOUT_IDX]);
    InDe_Struct *pMultiFilterIn  = (InDe_Struct *)(pTable[SMPDATAIN_IDX]);

    if (rate == HWAPI_9600SR)
    {
        Init_InterDeci(pMultiFilterOut, 5, 6, INDE5_6LEN, (QWORD *)lpf5_6_h, pHw->BufferSize, bufSize);
        Init_InterDeci(pMultiFilterIn,  6, 5, INDE6_5LEN, (QWORD *)lpf6_5_h, bufSize, pHw->BufferSize);
    }
    else if (rate == HWAPI_7200SR)
    {
        Init_InterDeci(pMultiFilterOut, 10, 9, INDE10_9LEN, (QWORD *)lpf10_9_h, pHw->BufferSize, bufSize);
        Init_InterDeci(pMultiFilterIn,  9, 10, INDE9_10LEN, (QWORD *)lpf9_10_h, bufSize, pHw->BufferSize);
    }
    else if (rate == HWAPI_10287SR)
    {
        Init_InterDeci(pMultiFilterOut, 7, 9, INDE7_9LEN, (QWORD *)lpf7_9_h, pHw->BufferSize, bufSize);
        Init_InterDeci(pMultiFilterIn,  9, 7, INDE9_7LEN, (QWORD *)lpf9_7_h, bufSize, pHw->BufferSize);
    }
    else if (rate == HWAPI_9000SR)
    {
        Init_InterDeci(pMultiFilterOut, 8, 9, INDE8_9LEN, (QWORD *)lpf8_9_h, pHw->BufferSize, bufSize);
        Init_InterDeci(pMultiFilterIn,  9, 8, INDE9_8LEN, (QWORD *)lpf9_8_h, bufSize, pHw->BufferSize);
    }
    else if (rate == HWAPI_8000SR)
    {
#if 1 /* Initialized for V90d, 5ms */
        Init_InterDeci(pMultiFilterIn,  6, 5, INDE6_5LEN, (QWORD *)lpf6_5_h, 40, 48);
#else /* Initialized for V90d, 7.5ms */
        Init_InterDeci(pMultiFilterIn,  6, 5, INDE6_5LEN, (QWORD *)lpf6_5_h, 60, 72);
#endif
    }
    else if (rate == HWAPI_8229SR)
    {
        Init_InterDeci(pMultiFilterOut, 35, 36, INDE35_36LEN, (QWORD *)lpf35_36_h, pHw->BufferSize, bufSize);
        Init_InterDeci(pMultiFilterIn,  36, 35, INDE36_35LEN, (QWORD *)lpf36_35_h, bufSize, pHw->BufferSize);
    }
    else if (rate == HWAPI_8400SR)
    {
        Init_InterDeci(pMultiFilterOut, 20, 21, INDE20_21LEN, (QWORD *)lpf20_21_h, pHw->BufferSize, bufSize);
        Init_InterDeci(pMultiFilterIn,  21, 20, INDE21_20LEN, (QWORD *)lpf21_20_h, bufSize, pHw->BufferSize);
    }
}
#endif

/* hardware initialization function */
void HwInit(UBYTE **pTable)
{
    HwStruct *pHw = (HwStruct *) pTable[HW_STRUC_IDX];
    UBYTE *pHwApi = (UBYTE *)pTable[HWAPIDATA_IDX];

    /* reset hardware control register */
    pHwApi[0] = 0;

    /* Initialize hardware structure */
    pHw->OnOffHook = HW_ONHOOK;

    pHw->BufferSize     = HW_DEF_BUFSIZE;
    pHw->Pre_BufferSize = HW_DEF_BUFSIZE;

    pHw->SampRate     = HW_DEF_SAM_RATE;
    pHw->Pre_SampRate = HW_DEF_SAM_RATE;
}

/* hardware transfer function, from Hw to modem */
void HwXferIn(UBYTE **pTable, QWORD *PCM_in)
{
    HwStruct *pHw = (HwStruct *)pTable[HW_STRUC_IDX];
    UWORD *pClk = (UWORD *)pTable[CLOCKDATA_IDX];
    QWORD *pPCMin = (QWORD *)pTable[PCMINDATA_IDX];
#if SAMCONV
    InDe_Struct *pMultiFilterIn = (InDe_Struct *)(pTable[SMPDATAIN_IDX]);
#endif
#if SUPPORT_DUMP_DATA
    QWORD qPCMin;
#endif

#if SAMCONV

    if (pHw->SampRate == HWAPI_8000SR)
    {
#if SUPPORT_DUMP_DATA

        if (DumpOrSimuType > 0 && Case_State > 0)
        {
            UBYTE i;

            for (i = 0; i < pHw->BufferSize; i++)
            {
#if (SUPPORT_SIMULATION & (!DUALMODEMSIM))

                if (CaseEndFlag == 0)
                {
                    if (fread(&qPCMin, sizeof(QWORD), 1, PCMinRdFile) == 0)
                    {
                        CaseEndFlag = 1;
                        TRACE0("PCMinRdFile: Read Data End 3");
                        return;
                    }

                    PCM_in[i] = qPCMin;
                }

#endif

                if (DumpPCMinCount < MAX_DATA_SIZE) { DumpPCMinData[DumpPCMinCount++] = PCM_in[i]; }
            }
        }

#endif

        memcpy(pPCMin, PCM_in, pHw->BufferSize * sizeof(QWORD));
    }
    else
    {
#if SUPPORT_DUMP_DATA

        if (DumpOrSimuType > 0 && Case_State > 0)
        {
            UBYTE i;

            for (i = 0; i < pMultiFilterIn->BufSizeIn; i++)
            {
#if (SUPPORT_SIMULATION & (!DUALMODEMSIM))

                if (CaseEndFlag == 0)
                {
                    if (fread(&qPCMin, sizeof(QWORD), 1, PCMinRdFile) == 0)
                    {
                        CaseEndFlag = 1;
                        TRACE0("PCMinRdFile: Read Data End 4");
                        return;
                    }

                    PCM_in[i] = qPCMin;
                }

#endif

                if (DumpPCMinCount < MAX_DATA_SIZE) { DumpPCMinData[DumpPCMinCount++] = PCM_in[i]; }
            }
        }

#endif

        SampleRate_Change(pMultiFilterIn, PCM_in, pPCMin);
    }

#else

#if SUPPORT_DUMP_DATA

    if (DumpOrSimuType > 0 && Case_State > 0)
    {
        UBYTE i;

        for (i = 0; i < pHw->BufferSize; i++)
        {
#if (SUPPORT_SIMULATION & (!DUALMODEMSIM))

            if (CaseEndFlag == 0)
            {
                if (fread(&qPCMin, sizeof(QWORD), 1, PCMinRdFile) == 0)
                {
                    CaseEndFlag = 1;
                    TRACE0("PCMinRdFile: Read Data End 5");
                    return;
                }

                PCM_in[i] = qPCMin;
            }

#endif

            if (DumpPCMinCount < MAX_DATA_SIZE) { DumpPCMinData[DumpPCMinCount++] = PCM_in[i]; }
        }
    }

#endif

    memcpy(pPCMin, PCM_in, pHw->BufferSize * sizeof(QWORD));

#endif

    /* Clock Ticking */
    pClk[1] += pHw->BufferSize;  /* sample clock */

    if (pClk[1] >= MsClock_Size[pHw->SampRate])
    {
        (pClk[0])--;
        pClk[1] -= MsClock_Size[pHw->SampRate];
    }

    if (pHw->BufferSize != pHw->Pre_BufferSize || pHw->SampRate != pHw->Pre_SampRate)
    {
        pClk[1] = 0;

        pHw->Pre_BufferSize = pHw->BufferSize;
        pHw->Pre_SampRate   = pHw->SampRate;
    }
}

/* hardware transfer function, from modem to Hw */
void HwXferOut(UBYTE **pTable, QWORD *PCM_out)
{
#if SAMCONV
    InDe_Struct *pMultiFilterOut = (InDe_Struct *)(pTable[SMPDATAOUT_IDX]);
#endif
    HwStruct *pHw = (HwStruct *)pTable[HW_STRUC_IDX];
    QWORD *pPCMout = (QWORD *)pTable[PCMOUTDATA_IDX];
    UBYTE  ubHwOutBuffSize;
#if DUMP_OUT_DATA_ENABLE
    UBYTE  i;
#endif

#if SAMCONV

    if (pHw->SampRate == HWAPI_8000SR)
    {
        memcpy(PCM_out, pPCMout, pHw->BufferSize * sizeof(QWORD));

        ubHwOutBuffSize = pHw->BufferSize;
    }
    else
    {
        SampleRate_Change(pMultiFilterOut, pPCMout, PCM_out);

        ubHwOutBuffSize = (UBYTE)(pMultiFilterOut->BufSizeOut);
    }

#else

    memcpy(PCM_out, pPCMout, pHw->BufferSize * sizeof(QWORD));

    ubHwOutBuffSize = pHw->BufferSize;
#endif

#if DUMP_OUT_DATA_ENABLE

    if (DumpOrSimuType > 0 && Case_State > 0)
    {
        for (i = 0; i < ubHwOutBuffSize; i++)
        {
            if (DumpPCMouCount < MAX_DATA_SIZE) { DumpPCMouData[DumpPCMouCount++] = PCM_out[i]; }
        }
    }

#endif
}
