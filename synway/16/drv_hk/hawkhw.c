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

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "commmac.h"
#include "hawkapi.h"
#include "gaoapi.h"
#include "hwstru.h"
#if TIMER_ON
#include "gaotimer.h"
#endif

QWORD gPCMInBuf[PCM_BUFSIZE];
QWORD gPCMOutBuf[PCM_BUFSIZE];

/* For MIPS calculation */
UWORD StartSampleCount[HW_MAX_TIMER];
UWORD StopSampleCount[HW_MAX_TIMER];
UWORD TelInAddr[HW_MAX_TIMER];

HawkStruct Hawk;

#if TIMER_ON
void StartTimer(UBYTE TimerID)
{
    UWORD currSample;

    if (TimerID >= HW_MAX_TIMER)
    {
        return;
    }

    /* Retrieve current sample clock from hardware */
    upDM(&currSample, 0x3936, 1);

    /* Retrieve buffer pointer, for timer calculation more than 1 buffer time */
    upDM(&TelInAddr[TimerID], TELIN_ADDRESS, 1);

    StartSampleCount[TimerID] = currSample;
}

SWORD StopTimer(UBYTE TimerID)
{
    UWORD currSample;
    UWORD dmAdd;

    if (TimerID >= HW_MAX_TIMER)
    {
        return -1;
    }

    /* Retrieve current sample clock from hardware */
    upDM(&currSample, 0x3936, 1);

    if (currSample < StartSampleCount[TimerID])
    {
        upDM(&dmAdd, TELIN_ADDRESS, 1);
        currSample = StartSampleCount[TimerID] - currSample;

        if (dmAdd != TelInAddr[TimerID])
        {
            currSample += Hawk.BufferSize;
        }

        StopSampleCount[TimerID] = UUMULUD(currSample, q100) / Hawk.BufferSize;
    }
    else if (currSample > StartSampleCount[TimerID])
    {
        currSample = Hawk.BufferSize - currSample;
        currSample += StartSampleCount[TimerID];

        StopSampleCount[TimerID] = UUMULUD(currSample, q100) / Hawk.BufferSize;
    }
    else
    {
        StopSampleCount[TimerID] = 0;
    }

    return(StopSampleCount[TimerID]);
}
#endif

void HWChgSamRate(UWORD rate)
{
    UWORD ii;

    dnDM(&rate, 0x3fd1, 1);

    upDM(&ii, 0x3fd0, 1);
    ii |= 0x0020;
    dnDM(&ii, 0x3fd0, 1);
}

void HWChgBufSize(UWORD size)
{
    UWORD ii;

    dnDM(&size, 0x3fd2, 1);

    upDM(&ii, 0x3fd0, 1);
    ii |= 0x0010;
    dnDM(&ii, 0x3fd0, 1);
}

BOOL Processor_Side_Initialize(void)
{
    UWORD i;
    UWORD dataready;

    for (i = 0; i < HW_MAX_TIMER; i++)
    {
        StartSampleCount[i] = 0;
        StopSampleCount[i] = 0;
        TelInAddr[i] = 0;
    }

    /* Initialize hardware */
    outpw(BASE_ADDR + 6, 0);

    /*  verify the existence of board. */
    outpw(HAWKADDR, 0x6000);    /*  write 0x5555 to dm (0x2000) */
    outpw(HAWKDATA, 0x5555);
    outpw(HAWKADDR, 0x6000);    /*  read back and verify. */
    i = inpw(HAWKDATA);

    if (i != 0x5555)
    {
        fprintf(stderr, "Board existence verification failed\n");
        fprintf(stderr, "Wrote 0x5555 to dm (0x2000), read back 0x%04x\n", i);
        fprintf(stderr, "Press space to continue\n");
        getch();
    }

    outpw(HAWKADDR, 0x6000);    /*  write 0xaaaa to dm (0x2000) */
    outpw(HAWKDATA, 0xaaaa);
    outpw(HAWKADDR, 0x6000);    /*  read back and verify. */
    i = inpw(HAWKDATA);

    if (i != 0xaaaa)
    {
        fprintf(stderr, "Board existence verification failed\n");
        fprintf(stderr, "Wrote 0xaaaa to dm (0x2000), read back 0x%04x\n", i);
        fprintf(stderr, "Press space to continue\n");
        getch();
    }

    if (dnldProg("daaloop.exe") != 0)
    {
        fprintf(stderr, "Download failed\n");
        exit(1);
    }

    /* Initialize hardware structure */
    Hawk.OnOffHook = HW_ONHOOK;
    Hawk.RingOnCount = 0;
    Hawk.RingOffCount = 0;

    Hawk.BufferSize = HW_DEF_BUFSIZE;

    dataready = 0;
    dnDM(&dataready, 0x3fdf, 1);

    while (!dataready)
    {
        upDM(&dataready, 0x3fdf, 1);
    }

    dataready = 0;
    dnDM(&dataready, 0x3fdf, 1);

    HWChgSamRate(HW_DEF_SAM_PER_SEC);
    HWChgBufSize(HW_DEF_BUFSIZE);

#if TIMER_ON
    StartTimer(1);
#endif

    return (true);
}

void Hook(UBYTE state)
{
    UWORD i;

    upDM(&i, 0x3fd0, 1);

    if (state == ON_HOOK)
    {
        i |= 0x0080;
    }
    else if (state == OFF_HOOK)
    {
        i |= 0x0040;
    }

    dnDM(&i, 0x3fd0, 1);
}

void Hw_CID_State(UBYTE state)
{
    UWORD i;

    upDM(&i, 0x3fd0, 1);

    if (state == 0x00)
    {
        i |= 0x0002;
    }
    else if (state == 0x01)
    {
        i |= 0x0004;
    }

    dnDM(&i, 0x3fd0, 1);
}

void HawkSetGain(UWORD gainid)/*No used!!!*/
{
    UWORD ii;

    if (gainid <= 3)
    {
        dnDM(&gainid, 0x3fd1, 1);

        upDM(&ii, 0x3fd0, 1);
        ii |= 0x0008;
        dnDM(&ii, 0x3fd0, 1);
    }
}


void DriverXferIn(void)
{
    UWORD dataready;
    UWORD dmAdd;

    dataready = 0;

    while (!dataready)
    {
        upDM(&dataready, 0x3fdf, 1);
    }

    dataready = 0;
    dnDM(&dataready, 0x3fdf, 1);

    /* Rx samples */
    upDM(&dmAdd, TELIN_ADDRESS, 1);
    upDM((UWORD *)gPCMInBuf, dmAdd, Hawk.BufferSize);
}

void Data_Send_Prepare(void)
{
    UBYTE i;
    UWORD dmAdd;

#if !T1FRAMER/*Can not change xmit level for T1FRAMER*/

    for (i = 0; i < Hawk.BufferSize; i++)
    {
#if   HAWK_SLAB   /* To change xmit level; 3.2 db for Slab */
        gPCMOutBuf[i] = (QWORD)(QQMULQD(gPCMOutBuf[i], HAWKHW_DB_SLAB)   >> 14);
#elif HAWK_ADI    /* To change xmit level; -1.9 db for ADI */
        gPCMOutBuf[i] = (QWORD)(QQMULQD(gPCMOutBuf[i], HAWKHW_DB_ADI)    >> 15);
#elif HAWK_ST7550 /* To change xmit level; 1.2 db for ST7550 */
        gPCMOutBuf[i] = (QWORD)(QQMULQD(gPCMOutBuf[i], HAWKHW_DB_ST7550) >> 14);
#endif
    }

#endif

    /* Tx samples */
    upDM(&dmAdd, TELOUT_ADDRESS, 1);
    dnDM((UWORD *)gPCMOutBuf, dmAdd, Hawk.BufferSize);
}

UBYTE HawkCheckRing(void)
{
    UWORD ring_flag;
    UBYTE RingDetect_Flag = 0;

    /* Poll Ring flag from hardware */
    upDM(&ring_flag, RING_ADDRESS, 1);

#if T1FRAMER

    if (ring_flag == 0)
#else
    if (ring_flag)
#endif
    {
        Hawk.RingOffCount = HW_RINGOFF_THRESHOLD;

        Hawk.RingOnCount ++;
    }
    else
    {
        Hawk.RingOffCount --;

        if (Hawk.RingOffCount < 0)
        {
            /* Init Ring Counter delay */
            Hawk.RingOffCount = HW_RINGOFF_THRESHOLD;

            if (Hawk.RingOnCount > HW_RINGON_THRESHOLD)
            {
                RingDetect_Flag = 1;
            }

            Hawk.RingOnCount = 0;
        }
    }

    /* Reset Ring flag */
    ring_flag = 0;
    dnDM(&ring_flag, RING_ADDRESS, 1);

    return (RingDetect_Flag);
}

void HwCheckState(Ctrl_Hardware_Signal *pStatus)
{
    pStatus->RingDetected = HawkCheckRing();
}

void HwChgState(Ctrl_Hardware_Signal *pStatus)
{
    if (pStatus->StateChange)
    {
        if (pStatus->StateChange & HOOK_STATE_CHG)
        {
            pStatus->StateChange &= (~HOOK_STATE_CHG);

            if (pStatus->HookState == ON_HOOK)
            {
                Hook(ON_HOOK);
            }
            else if (pStatus->HookState == OFF_HOOK)
            {
                Hook(OFF_HOOK);
            }
        }

#if (SUPPORT_CID_DET + SUPPORT_CID_GEN)

        if (pStatus->StateChange & CID_STATE_CHG)
        {
            if (pStatus->HookState == ON_HOOK)
            {
                Hw_CID_State(0x01);
            }
            else if (pStatus->HookState == OFF_HOOK)
            {
                Hw_CID_State(0x00);
            }

            pStatus->StateChange &= ~CID_STATE_CHG;
        }

#endif

        if (pStatus->StateChange & BUFFER_SIZE_CHG)
        {
            pStatus->StateChange &= (~BUFFER_SIZE_CHG);

            HWChgBufSize(pStatus->BufferSize);

            pStatus->ConfirmBufferSize = pStatus->BufferSize;/*For Hawkboard direct use modem's buffer*/

            Hawk.BufferSize = pStatus->BufferSize;

            TRACE1("BufferSize=%d", Hawk.BufferSize);
        }

        if (pStatus->StateChange & SAMPLE_RATE_CHG)
        {
            pStatus->StateChange &= (~SAMPLE_RATE_CHG);

            if (pStatus->SampRate == HWAPI_7200SR)
            {
                HWChgSamRate(7200);
            }
            else if (pStatus->SampRate == HWAPI_8000SR)
            {
                HWChgSamRate(8000);
            }
            else if (pStatus->SampRate == HWAPI_8229SR)
            {
                HWChgSamRate(8229);
            }
            else if (pStatus->SampRate == HWAPI_8400SR)
            {
                HWChgSamRate(8400);
            }
            else if (pStatus->SampRate == HWAPI_9000SR)
            {
                HWChgSamRate(9000);
            }
            else if (pStatus->SampRate == HWAPI_9600SR)
            {
                HWChgSamRate(9600);
            }
            else if (pStatus->SampRate == HWAPI_10287SR)
            {
                HWChgSamRate(10286);
            }
            else if (pStatus->SampRate == HWAPI_16000SR)
            {
                HWChgSamRate(16000);
            }
        }
    }
}

void dnDM(UWORD *p, UWORD addr, UWORD noWord)
{
    /*********************************************
    *  For a fully commented version see cmemws.c
    **********************************************/
    outpw(HAWKADDR, addr | 0x4000);

    for (; noWord != 0; noWord--)
    {
        outpw(HAWKDATA, *p++);
    }
}

void upDM(UWORD *p, UWORD addr, UWORD noWord)
{
    /*********************************************
    *  For a fully commented version see cmemws.c
    **********************************************/
    outpw(HAWKADDR, addr | 0x4000);

    for (; noWord != 0; noWord--)
    {
        *p++ = inpw(HAWKDATA);
    }
}

UBYTE dnldProg(char *filename)
{
    /*********************************************
    *  For a fully commented version see cmemws.c
    **********************************************/
    FILE *st;
    SWORD  ch, addr, pm0ReadYN, spacePD, noPKnl, noDKnl, noPWd, noDWd;
    SDWORD pm0Data, temp;

    /*  dummy write to reset board. */
    outpw(BASE_ADDR + 6, 0);

    pm0ReadYN = 'n';
    noPKnl = 0;
    noDKnl = 0;
    noPWd  = 0;
    noDWd  = 0;

    /*---------------------------------------------------------------------------
    -  open input file
    ---------------------------------------------------------------------------*/
    st = fopen(filename, "rt");

    if (st == NULL)
    {
        TRACE1("Failed to open file '%s'", filename);
        return (1);
    }

    /*---------------------------------------------------------------------------
    -  read header : <ESC><ESC>i
    ---------------------------------------------------------------------------*/
    ch = fgetc(st);

    if (ch != '\x1b')
    {
        puts("Invalid image file: First character not <ESC>");
        return (1);
    }

    ch = fgetc(st);

    if (ch != '\x1b')
    {
        puts("Invalid image file: Second character not <ESC>");
        return (1);
    }

    ch = fgetc(st);

    if (ch != 'i')
    {
        puts("Invalid image file: Third character not 'i'");
        return (1);
    }

    while (1)
    {
        /*---------------------------------------------------------------------------
        -  Read all sections
        ---------------------------------------------------------------------------*/
        /*  Reading section header. */
        ch = fgetc(st);

        if (ch == '\n')
        {
            continue;
        }

        if (ch == '\x1b')
        {
            /*  <ESC> signifies end of file. */
            /*puts ("All kernels have been downloaded");*/
            break;
        }

        if (ch != '@')
        {
            TRACE1("Expected kernel marker '@ 'not found : %c", ch);
            return (1);
        }

        /*---------------------------------------------------------------------------
        -  Check memory space
        ---------------------------------------------------------------------------*/
        ch = fgetc(st);

        switch (ch)
        {
            case 'P' :
            case 'B' :
                spacePD = 'p';
                break;

            case 'D' :
                spacePD = 'd';
                break;

            default :
                TRACE1("Expected memory space symbol '@P', '@B', or '@D' not found : %c", ch);
                return (1);
        }

        /*---------------------------------------------------------------------------
        -  Check memory type
        ---------------------------------------------------------------------------*/
        ch = fgetc(st);

        switch (ch)
        {
            case 'O' :
            case 'A' :
                break;

            default :
                TRACE1("Expected memory type symbol 'A', or 'O' not found : %c", ch);
                return (1);
        }

        /*---------------------------------------------------------------------------
        -  Reading section address
        ---------------------------------------------------------------------------*/
        if (fscanf(st, "%x", &addr) != 1)
        {
            puts("Failed to read kernel address");
            return (1);
        }

        /*TRACE2("Downloading kernel to %s memory location %04x", (spacePD == 'd') ? "data" : "program", addr);*/

        /*  Conuting kernel. */
        if (spacePD == 'p')
        {
            noPKnl++;
        }
        else
        {
            noDKnl++;
        }

        /*---------------------------------------------------------------------------
        -  Initialize to write to program/data memory; special treatment for pm(0)
               ---------------------------------------------------------------------------*/

        if ((addr == 0) && (spacePD == 'p'))
        {
            /*  Won't write to pm(0) to delay reset; start from pm(1). */
            outpw(HAWKADDR, 1);
        }
        else
        {
            /*  Setting 0x4000 writes to data memory. */
            outpw(HAWKADDR, addr | ((spacePD == 'p') ? 0 : 0x4000));
        }


        /*---------------------------------------------------------------------------
        -  Read data and download to DSP
        ---------------------------------------------------------------------------*/
        while (fscanf(st, "%lx", &temp) == 1)
        {
            /*  Conuting words. */
            if (spacePD == 'p')
            {
                noPWd++;
            }
            else
            {
                noDWd++;
            }

            if ((addr == 0) && (spacePD == 'p') && (pm0ReadYN == 'n'))
            {
                /*  Won't write to pm(0) to delay reset; save for later. */
                pm0ReadYN = 'y';
                pm0Data = temp;
                continue;
            }

            /*TRACE1("Data is %06lx", temp);*/
            if (spacePD == 'p')
            {
                /*  writing program memory (24 bits). */
                /*  write upper 16 bits of 24 bits. */
                outpw(HAWKDATA, (UWORD)((temp >> 8) & 0xffffUL));
                /*  write lower 8 bits of 24 bits in lower 8 bits. */
                outpw(HAWKDATA, (UWORD)((temp) & 0x00ffUL));
            }
            else
            {
                /*  writing data memory (16 bits). */
                outpw(HAWKDATA, (UWORD)(temp & 0xffffUL));
            }
        }

        /*---------------------------------------------------------------------------
        -  Reading dummy value #dummy
        ---------------------------------------------------------------------------*/
        ch = fgetc(st);

        if (ch != '#')
        {
            TRACE1("Expected kernel end marker '#', not found : %c", ch);
            return (1);
        }

        /*  Skip dummy string. */
        fscanf(st, "%*s");
    }

    if (pm0ReadYN == 'y')
    {
        /*  We have pm(0); write not to reset. */
        outpw(HAWKADDR, 0);

        /*  writing program memory (24 bits). */
        /*  write upper 16 bits of 24 bits. */
        outpw(HAWKDATA, (UWORD)((pm0Data >> 8) & 0xffffUL));
        /*  write lower 8 bits of 24 bits in lower 8 bits. */
        outpw(HAWKDATA, (UWORD)((pm0Data) & 0x00ffUL));
    }
    else
    {
        puts("No instruction for program memory location 0.  DSP will not reset.");
    }

    /*TRACE2("Downloaded %2d program kernels totaling %5d words", noPKnl, noPWd);*/
    /*TRACE2("Downloaded %2d data    kernels totaling %5d words", noDKnl, noDWd);*/

    fclose(st);

    return (0);
}

void Device_Close()
{
}
