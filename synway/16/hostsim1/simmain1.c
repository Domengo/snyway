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

/* Main.c for ACE and external modem works */

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dos.h>
#include "gaostru.h"
#include "gaoapi.h"
#include "uartapi.h"
#include "mdmmain.h"/* For real project, can not include "mdmmain.h", define MODEMDATAMEMSIZE as const */
#if TIMER_ON
#include "gaotimer.h"
#endif
#if MC_RECORDER
#include "mc_rec.h"
#include "acestru.h"
#include "circbuff.h"
#endif
#if COMPARE_BYTE
#include "bytecmp.h"
#endif

UWORD DTEDumpSize = 115; /* DTESPEED(115200)/1000 */ /* (38) for DTESPEED(38400)/1000 */

#define BEGIN_SIMU_FILE_NUM      (3)
#define END_SIMU_FILE_NUM        (3)
//#define DATA_FOLD_NAME           "d:\\data\\t.31 33600\\ans"
//#define DATA_FOLD_NAME           "s:\\gaores_tmp\\leoli\\1"
#define DATA_FOLD_NAME           "D:\\synway\\data\\20101101"
//#define DATA_FOLD_NAME           "G:\\gaoresearch\\gr_engineering\\QA_TEST_DATA\\fax01\\class2.1\\send_3Ps\\okidata_class2.1_s"
//#define DATA_FOLD_NAME           "D:\\New_Baseline\\data\\bug 8"
//#define DATA_FOLD_NAME           "."

extern UBYTE DumpOrSimuType;/* 1: only PcmIn; 2: PcmIn & DteIn; 3: multi-file PcmIn & DteIn */

extern UBYTE Case_State;    /* 0: init; 1: case begin; 2: case progressing; */

UBYTE ATA_Flag = 0;

#define PRINT_CHAR_TO_DTE  (0)

#if DUMPDATA_FLAG
SDWORD DumpTone1[5000000];
SDWORD DumpTone2[5000000];
SDWORD DumpTone3[5000000];
SDWORD DumpTone4[5000000];
SDWORD DumpTone5[5000000];
SDWORD DumpTone6[5000000];
SDWORD DumpTone7[5000000];
SDWORD DumpTone8[5000000];

UDWORD DumpTone1_Idx = 0;
UDWORD DumpTone2_Idx = 0;
UDWORD DumpTone3_Idx = 0;
UDWORD DumpTone4_Idx = 0;
UDWORD DumpTone5_Idx = 0;
UDWORD DumpTone6_Idx = 0;
UDWORD DumpTone7_Idx = 0;
UDWORD DumpTone8_Idx = 0;
FILE *DumpFile;
#endif

UBYTE Processor_Side_Initialize(void);
void HwChgState(GAO_ModStatus *pStatus);

GAO_LibStruct LibData;
UBYTE *pubModemDataMem;

GAO_ModStatus ModemStatus;

UBYTE Gprintf_Flag;

#define UBDTE_BUF_SIZE              (BUFSIZE)
UBYTE ubDTE_Buf[UBDTE_BUF_SIZE];

QWORD PCMInBuf[PCM_BUFSIZE];
QWORD PCMOutBuf[PCM_BUFSIZE];

UDWORD Count = 0;

FILE *PCMinRdFile;
FILE *DTEinRdFile;

FILE *PCMinWrFile;
FILE *DTEinWrFile;
FILE *WrFile;

UBYTE  CaseWillEnd_Flag = 0;
UDWORD CaseEnd_Count    = 0;
UBYTE CaseEndFlag       = 0;
UWORD Handup_Count      = 0;

SWORD  DumpPCMinData[MAX_DATA_SIZE];
UDWORD DumpPCMinCount = 0;

UWORD  DumpDTEinData[MAX_DATA_SIZE];/* DTEin number,DTEin data, DTEout number */
UDWORD DumpDTEinCount = 0;

extern SWORD DumpPCMouData[];
extern UDWORD DumpPCMouCount;

UWORD  DumpDTEouData[MAX_DATA_SIZE];
UDWORD DumpDTEouCount = 0;

UWORD  uRingDetTimer = 0;
UWORD  DumpRingData[255];
QWORD  DumpRingCount = 0;

UBYTE  DumpRingCountMax;

UWORD  SimuNameCount;
UWORD  EndCaseNum;

void ReadRingData_ForSimu(void)
{
    UBYTE i;
    SWORD qRing;

    if (CaseEndFlag == 0)
    {
        if (fseek(PCMinRdFile, -1L *(long)sizeof(QWORD), SEEK_END) != 0)
        {
            TRACE0("File read wrong");
        }

        if (fread(&qRing,  sizeof(QWORD), 1, PCMinRdFile) == 0)
        {
            CaseEndFlag = 1;

            TRACE0("PCMinRdFile: Read Data End");
        }

        DumpRingCountMax = (UBYTE)qRing;

        if (fseek(PCMinRdFile, -1L *(long)sizeof(UWORD)*DumpRingCountMax - (long)sizeof(QWORD), SEEK_END) != 0)
        {
            TRACE0("File read wrong");
        }

        for (i = 0; i < DumpRingCountMax; i++)
        {
            if (fread(&qRing,  sizeof(UWORD), 1, PCMinRdFile) == 0)
            {
                CaseEndFlag = 1;

                TRACE0("PCMinRdFile: Read Data End");
            }

            DumpRingData[i] = (UWORD)qRing;
        }

        if (fseek(PCMinRdFile, 0L, SEEK_SET) != 0)
        {
            TRACE0("File read wrong");
        }
    }
}

void OpenDataFileToRead_ForSimu(UWORD NameNum)
{
    CHAR Namebuf[260];

    sprintf(Namebuf, "%s\\Pin%d.dat", DATA_FOLD_NAME, NameNum);

    if ((PCMinRdFile = fopen(Namebuf, "rb")) == NULL)
    {
        TRACE1("The file Pin%d.dat not opened", NameNum);
        CaseEndFlag = 1;
    }

    ReadRingData_ForSimu();

    if (DumpOrSimuType > 1)
    {
        sprintf(Namebuf, "%s\\Din%d.dat", DATA_FOLD_NAME, NameNum);

        if ((DTEinRdFile = fopen(Namebuf, "rb")) == NULL)
        {
            TRACE1("The file Din%d.dat not opened", NameNum);
            CaseEndFlag = 1;
        }
    }
}

void WriteDataFile_Total(UWORD NameNum)
{
    CHAR Namebuf[60];

    sprintf(Namebuf, "Pin%d.dat", NameNum);
    PCMinWrFile = fopen(Namebuf, "wb");

    if (PCMinWrFile != NULL)
    {
        fwrite(&DumpPCMinData, sizeof(QWORD), DumpPCMinCount, PCMinWrFile);
        fwrite(&DumpRingData,  sizeof(UWORD), DumpRingCount,  PCMinWrFile);
        fwrite(&DumpRingCount, sizeof(QWORD), 1,              PCMinWrFile);

        fclose(PCMinWrFile);
    }

    DumpRingCount  = 0;
    DumpPCMinCount = 0;
    uRingDetTimer  = 0;

    sprintf(Namebuf, "Pou%d.dat", NameNum);
    WrFile = fopen(Namebuf, "wb");

    if (WrFile != NULL)
    {
        fwrite(&DumpPCMouData, sizeof(QWORD), DumpPCMouCount, WrFile);

        fclose(WrFile);
    }

    DumpPCMouCount = 0;

    if (DumpOrSimuType > 1)
    {
        sprintf(Namebuf, "Din%d.dat", NameNum);
        DTEinWrFile = fopen(Namebuf, "wb");

        if (DTEinWrFile != NULL)
        {
            fwrite(&DumpDTEinData, sizeof(UWORD), DumpDTEinCount, DTEinWrFile);

            fclose(DTEinWrFile);
        }

        DumpDTEinCount = 0;

        sprintf(Namebuf, "Dou%d.dat", NameNum);
        WrFile = fopen(Namebuf, "wb");

        if (WrFile != NULL)
        {
            fwrite(&DumpDTEouData, sizeof(UWORD), DumpDTEouCount, WrFile);

            fclose(WrFile);
        }

        DumpDTEouCount = 0;
    }

    CaseWillEnd_Flag = 0;
}

void main(UBYTE argc, char *argv[])
{
    char cpstring2[] = {"atx4\r"};
    char cpstring1[] = {"at&k0+m=8\r"};
    char cpstringCall[] = {"atdt1\r"};
    char cpstringAns[] = {"atas0=1\r"};
    char Hangupstring1[] = {"+++"};
    char Hangupstring2[] = {"ath\r"};
    UBYTE sCallFlag = 0;   /* 1 for call, 0 for ans */
    UWORD uAnsCount = 500; /* 500 for 5 sec after the program begins, ATA will be issued */
    UBYTE ATA_Flag  = 0;
    UDWORD i;
    UWORD  uLen = 0, uLenT;
    UBYTE  CaseStart_Flag = 0;
    UBYTE  ch;
#if MC_RECORDER
    DpcsStruct *pDpcs;
    ACEStruct  *pAce;
    CircBuffer *DteWr;
#endif
#if 0/* For debug, initial string */
    char cpstringInit[] = {"ats0=1+m=7\r"};
#endif
    UWORD qDte_Count;
    UWORD qDte_Data;
    UWORD NumberOfDTEOut;
    pubModemDataMem = (UBYTE *)&LibData;

    Gprintf_Flag   = 1;
    DumpOrSimuType = 2;/* 1: only PcmIn; 2: PcmIn & DteIn; 3: multi-file PcmIn & DteIn */
    Case_State     = 1;/* 0: init; 1: case begin; 2: case progressing; */

    if (argc >= 3)
    {
        SimuNameCount = (UWORD)atoi(argv[1]);

        EndCaseNum = (UWORD)atoi(argv[2]);
    }
    else
    {
        SimuNameCount = BEGIN_SIMU_FILE_NUM;
        EndCaseNum    = END_SIMU_FILE_NUM;
    }

    TRACE1("\n\nFollow Case Number = %d", SimuNameCount);
    OpenDataFileToRead_ForSimu(SimuNameCount);

    if (CaseEndFlag == 1)
    {
        return;
    }

    Processor_Side_Initialize();

    for (i = 0; i < PCM_BUFSIZE; i++)
    {
        PCMInBuf[i]  = 0;
        PCMOutBuf[i] = 0;
    }

#if MC_RECORDER
    mcrInit();
#endif
#if TIMER_ON
    mcrTimersInit();
#endif
#if COMPARE_BYTE
    byteCmpInit();
#endif

    /* Initialize Modem system memory & Initialize Modem device driver */
    if (GAO_Lib_Init(pubModemDataMem, sizeof(GAO_LibStruct)) < 0)
    {
        exit(-1);
    }

    /* atTest(); */
    UartInit();
#if 0/* For debug, initial string */
    SendCharToUartRx(cpstringInit, (UWORD)strlen(cpstringInit));
#endif

    if (DumpOrSimuType == 1)
    {
        SendCharToUartRx(cpstring2, (UWORD)strlen(cpstring2));
        SendCharToUartRx(cpstring1, (UWORD)strlen(cpstring1));

        if (sCallFlag)
        {
            SendCharToUartRx(cpstringCall, (UWORD)strlen(cpstringCall));
        }
    }

    /* Initialize status structure */
    if (GAO_ModStatus_Init(&ModemStatus) < 0)
    {
        exit(-1);
    }

#if MC_RECORDER
    pDpcs = (DpcsStruct *)(((UBYTE **)pubModemDataMem)[DPCS_STRUC_IDX]);
    pAce  = (ACEStruct *)(((UBYTE **)pubModemDataMem)[ACE_STRUC_IDX]);
#endif

    /* run host and modem */
    while (1)
    {
#if TIMER_ON
        StartTimer(0);
#endif
        Count ++;

        if (DumpOrSimuType == 1)
        {
            if ((sCallFlag == 0) && uAnsCount-- == 0 && ATA_Flag == 0)
            {
                SendCharToUartRx(cpstringAns, (UWORD)strlen(cpstringAns));
                ATA_Flag = 1;
            }

            if (CaseEndFlag == 1)
            {
                break;
            }
            else if (Handup_Count > 0)
            {
                if (Handup_Count == 500)
                {
                    SendCharToUartRx(Hangupstring1, (UWORD)strlen(Hangupstring1));
                }
                else if (Handup_Count == 200)
                {
                    SendCharToUartRx(Hangupstring2, (UWORD)strlen(Hangupstring2));
                }

                Handup_Count --;
            }
        }

        if (CaseEndFlag == 1)
        {
            break;
        }

        /*  for stop when type Esc key from keyboard */
        if (kbhit())
        {
            ch = (UBYTE)getch();

            if (ch == '$' || ch == 27)
            {
                break;
            }
        }

        /* Check for any data coming from UART driver */
        /* Get how many data modem can accept */
        uLenT = GAO_InBuf_Chk(pubModemDataMem);

        if (uLenT > UBDTE_BUF_SIZE)
        {
            uLenT = UBDTE_BUF_SIZE;
        }

        /* Get the data from terminal */
        /* return got data size, uLen<=uLenT */
        if (DumpOrSimuType > 1)
        {
            if (fread(&qDte_Count, sizeof(UWORD), 1, DTEinRdFile) == 0)
            {
                CaseEndFlag = 1;
                TRACE0("DTEinRdFile: Read Data End");
            }
            else
            {
                uLen = qDte_Count;
            }

            if (qDte_Count == 0x18)
            {
                uLen = qDte_Count;
            }

            for (i = 0; i < uLen; i++)
            {
                if (fread(&qDte_Data, sizeof(UWORD), 1, DTEinRdFile) == 0)
                {
                    CaseEndFlag = 1;
                    TRACE0("DTEinRdFile Error: Read Data End unexpectedly");
                    break;
                }

                ubDTE_Buf[i] = (UBYTE)qDte_Data;
            }

#if 0

            if (fread(&NumberOfDTEOut, sizeof(UWORD), 1, DTEinRdFile) == 0)
            {
                CaseEndFlag = 1;
                TRACE0("DTEinRdFile: Read Data End");
            }
            else
            {
                uLenT = NumberOfDTEOut;
            }

#endif

            if (CaseEndFlag == 0)
            {
                if (DumpDTEinCount < MAX_DATA_SIZE) { DumpDTEinData[DumpDTEinCount++] = uLen; }
            }

            if (uLen > 0)
            {
                for (i = 0; i < uLen; i++)
                {
                    if (DumpDTEinCount < MAX_DATA_SIZE) { DumpDTEinData[DumpDTEinCount++] = ubDTE_Buf[i]; }

#if COMPARE_BYTE

                    if (pAce->LineState == ONLINE_DATA && (pDpcs->MSR1 & V42_DATA))
                    {
                        byteCmpCompare(BYTECMP_CH_A, ubDTE_Buf[i]);
                    }

#endif
                }

                GAO_Buf_In(pubModemDataMem, ubDTE_Buf, uLen);
            }

            if (CaseEndFlag == 0)
            {
                if (DumpDTEinCount < MAX_DATA_SIZE) { DumpDTEinData[DumpDTEinCount++] = uLenT; }
            }
        }
        else
        {
            /* Check for any modem data need to be sent to DTE (UART) */
            /* Get how many data UART can accept */
            uLenT = CheckUartTxBuf();
        }

#if MC_RECORDER
        mcrRecord(5, DteWr->CurrLen);
#endif
        HwChgState(&ModemStatus);
        /* Modem Main function block */
        GAO_PCM_Proc(pubModemDataMem, PCMInBuf, PCMOutBuf);
        GAO_ModStatus_Upd(pubModemDataMem, &ModemStatus);

        if ((ModemStatus.StateChange & HOOK_STATE_CHG) && (ModemStatus.HookState == ON_HOOK) && CaseStart_Flag == 1)
        {
            if (DumpOrSimuType > 2)
            {
                CaseWillEnd_Flag = 1;
                CaseEnd_Count = Count;
            }
            else
            {
                TRACE1("\n\nFollow Case Number = %d", ++SimuNameCount);
                HwInit((UBYTE **)pubModemDataMem);
            }

            CaseStart_Flag = 0;
        }

        if ((ModemStatus.StateChange & HOOK_STATE_CHG) && (ModemStatus.HookState == OFF_HOOK))
        {
            CaseStart_Flag = 1;
        }

        if (uLenT > UBDTE_BUF_SIZE)
        {
            uLenT = UBDTE_BUF_SIZE;
        }

        /* Get the data from modem */
        /* return got data size, uLen<=uLenT */
        if (uLenT > 0)
        {
            uLen = GAO_Buf_Out(pubModemDataMem, ubDTE_Buf, uLenT);
#if COMPARE_BYTE

            if (pAce->LineState == ONLINE_DATA && (pDpcs->MSR1 & V42_DATA))
            {
                for (i = 0; i < uLen; i++)
                {
                    byteCmpCompare(BYTECMP_CH_B, ubDTE_Buf[i]);
                }
            }

#endif
        }
        else
        {
            uLen = 0;
        }

        if (DumpOrSimuType > 1)
        {
            if (DumpDTEouCount < MAX_DATA_SIZE) { DumpDTEouData[DumpDTEouCount++] = uLen; }

            if (uLen > 0)
            {
                for (i = 0; i < uLen; i++)
                {
                    if (DumpDTEouCount < MAX_DATA_SIZE) { DumpDTEouData[DumpDTEouCount++] = ubDTE_Buf[i]; }
                }
            }
        }

#if PRINT_CHAR_TO_DTE/* Print out the characters to DTE */

        if (uLen > 0)
        {
            for (i = 0; i < uLen; i++)
            {
                TRACE("%c", ubDTE_Buf[i]);

                if (ubDTE_Buf[i] == 13)
                {
                    TRACE("%c", 10);
                }
            }
        }

#endif

        if (DumpOrSimuType > 2 && CaseWillEnd_Flag == 1 && (Count == CaseEnd_Count + 10))
        {
            WriteDataFile_Total(SimuNameCount);

            fclose(PCMinRdFile);
            fclose(DTEinRdFile);

            SimuNameCount ++;

            if (SimuNameCount > EndCaseNum)
            {
                TRACE0("\n\nThe Simulation End");
                CaseEndFlag = 1;
            }
            else
            {
                TRACE1("\n\nFollow Case Number = %d", SimuNameCount);
                HwInit((UBYTE **)pubModemDataMem);
                OpenDataFileToRead_ForSimu(SimuNameCount);

                if (CaseEndFlag == 1)
                {
                    break;
                }
            }
        }

#if MC_RECORDER
        mcrRecord(0, StopTimer(0));
        mcrRun();
#endif
    }   /* End while */

    if (DumpOrSimuType > 1 && CaseWillEnd_Flag == 0 && CaseEndFlag == 0)
    {
        fclose(PCMinRdFile);
        fclose(DTEinRdFile);
    }

    if (SimuNameCount <= END_SIMU_FILE_NUM && (CaseEndFlag == 0 || CaseWillEnd_Flag == 0))
    {
        WriteDataFile_Total(SimuNameCount);
    }

#if MC_RECORDER
    mcrSave();
#endif

    TRACE0("BYE");

#if DUMPDATA_FLAG

    DumpFile = fopen("t1.txt", "w");
#if 1 /* Regular print out */

    for (i = 0; i < DumpTone1_Idx; i++)
    {
        fprintf(DumpFile, "%d\n", DumpTone1[i]);
    }

#else /* Special print out for bit */
    {
        UBYTE j;
        j = 0;

        for (i = 0; i < DumpTone1_Idx; i++)
        {
            fprintf(DumpFile, "%d", DumpTone1[i]);
            j++;

            if (j == 10)
            {
                j = 0;
                fprintf(DumpFile, "\n");
            }
        }
    }
#endif

    fclose(DumpFile);

    DumpFile = fopen("t2.txt", "w");

    for (i = 0; i < DumpTone2_Idx; i++)
    {
        fprintf(DumpFile, "%d\n", DumpTone2[i]);
    }

    fclose(DumpFile);

    DumpFile = fopen("t3.txt", "w");

    for (i = 0; i < DumpTone3_Idx; i++)
    {
        fprintf(DumpFile, "%d\n", DumpTone3[i]);
    }

    fclose(DumpFile);

    DumpFile = fopen("t4.txt", "w");

    for (i = 0; i < DumpTone4_Idx; i++)
    {
        fprintf(DumpFile, "%d\n", DumpTone4[i]);
    }

    fclose(DumpFile);

    DumpFile = fopen("t5.txt", "w");

    for (i = 0; i < DumpTone5_Idx; i++)
    {
        fprintf(DumpFile, "%d\n", DumpTone5[i]);
    }

    fclose(DumpFile);

    DumpFile = fopen("t6.txt", "w");

    for (i = 0; i < DumpTone6_Idx; i++)
    {
        fprintf(DumpFile, "%d\n", DumpTone6[i]);
    }

    fclose(DumpFile);

    DumpFile = fopen("t7.txt", "w");

    for (i = 0; i < DumpTone7_Idx; i++)
    {
        fprintf(DumpFile, "%d\n", DumpTone7[i]);
    }

    fclose(DumpFile);

    DumpFile = fopen("t8.txt", "w");

    for (i = 0; i < DumpTone8_Idx; i++)
    {
        fprintf(DumpFile, "%d\n", DumpTone8[i]);
    }

    fclose(DumpFile);
#endif
}

void gprintf(char *str, ...)
{
    va_list ap;

    if (Gprintf_Flag == 0)
    {
        return;
    }

    /* Point to first argument */
    va_start(ap, str);

    vprintf(str, ap);

    va_end(ap);
}

void draw_EPG(UBYTE **pTable)
{
}

void SerialSetCTS(void)
{
}

void SerialClearCTS(void)
{
}

void SerialSetCD(void)
{
}

void SerialClearCD(void)
{
}

UBYTE SerialCheckRTS(void)
{
    return (1);
}

void SerialSetRate(UDWORD rate)
{
}
