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
#include <time.h>
#include <conio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dos.h>
#include "gaostru.h"
#include "gaoapi.h"
#include "uartapi.h"
#include "drvapi.h"
#include "mdmmain.h"/* For real project, can not include "mdmmain.h", define MODEMDATAMEMSIZE as const*/
#include "types.h"
#include "serutil.h"
#if TIMER_ON
#include "gaotimer.h"
#endif
#if MC_RECORDER
#include "mc_rec.h"
#endif
#if COMPARE_BYTE
#include "bytecmp.h"
#endif
#include "common.h"

#if DRAWEPG
void init_graf(void);
void done_graf(void);
#endif

UBYTE InitHW_Flag = 1;
int iSerPort;

int iSerIRQ;
int iSerBaud;
int iSerParity;
int iSerData;
int iSerStop;

UWORD DTEDumpSize = 115; /*iSerBaud/1000;*/ /*DTESPEED(115200)/1000*/ /*(38) for DTESPEED(38400)/1000*/

#define DTE_OUT_FILE       (0) /* if DUMPOTHERDTEOUTFILE = 1, DTE_OUT_FILE = 0 */
#define PRINT_CHAR_TO_DTE  (0)

UBYTE SerPortInit(void);
void SerRestorOldHandler(void);

GAO_LibStruct LibData;
UBYTE *pubModemDataMem;

#if DUMPDATA_FLAG
SDWORD DumpTone1[10000];
SDWORD DumpTone2[10000];
SDWORD DumpTone3[10000];
SDWORD DumpTone4[10000];
SDWORD DumpTone5[10000];
SDWORD DumpTone6[10000];
SDWORD DumpTone7[10000];
SDWORD DumpTone8[10000];

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

GAO_ModStatus ModemStatus;

UBYTE TxIntEnd;
UBYTE Gprintf_Flag;

#define UBDTE_BUF_SIZE              (BUFSIZE)
UBYTE ubDTE_Buf[UBDTE_BUF_SIZE];

QDWORD Input_Value = 0;

UDWORD Count = 0;

#if DTE_OUT_FILE
UBYTE DTEoutFile_StartFlag = 0;
FILE *DTEout_Ifp;
#endif

UWORD  CaseNumber       = 1;
UBYTE  ESC_flag         = 0;
UBYTE  CaseStart_Flag   = 0;
UBYTE  CaseWillEnd_Flag = 0;
UBYTE  CaseEnd_Flag     = 0;
UDWORD CaseEnd_Count    = 0;

#if DUMPSIMDATA

/* 0: No dump;                     1: Dump PcmIn;            2: Dump PcmIn & DteIn; */
/* 3: Dump multi-file PcmIn & DteIn; 4: Dump multi-file PcmIn & DteIn, and no stop. */
extern UBYTE DumpOrSimuType;

extern UBYTE Case_State;/*0: init; 1: case begin; 2: case progressing;*/
extern UBYTE ATandF_Flag;

FILE *PCMinWrFile;
FILE *DTEinWrFile;

SWORD  DumpPCMinData[MAX_DATA_SIZE];
UDWORD DumpPCMinCount = 0;

UWORD  DumpDTEinData[MAX_DATA_SIZE];/*DTEin number,DTEin data, DTEout number*/
UDWORD DumpDTEinCount = 0;

#if DUMP_OUT_DATA_ENABLE
FILE   *PCMouWrFile;
FILE   *DTEouWrFile;

extern SWORD DumpPCMouData[];
extern UDWORD DumpPCMouCount;

UWORD  DumpDTEouData[MAX_DATA_SIZE];
UDWORD DumpDTEouCount = 0;
#endif

UWORD Handup_Count = 0;

UWORD  uRingDetTimer = 0;
UWORD  DumpRingData[255];
QWORD  DumpRingCount = 0;

UBYTE  DiskFull_Flag = 0;
SDWORD DiskFreeSpace = 532000000;

void Check_DiskSpace(void)/*No used*/
{
    struct diskfree_t disk_data;

    /* get information about drive 5 (the E drive) */
    if (_dos_getdiskfree(5, &disk_data) == 0)
    {
        DiskFreeSpace = ((UDWORD)disk_data.avail_clusters) * ((UDWORD)disk_data.sectors_per_cluster) * ((UDWORD)disk_data.bytes_per_sector);

        if (DiskFreeSpace < 1024)
        {
            DiskFull_Flag = 1;
        }
    }
    else
    {
        TRACE0("Invalid drive specified");
    }
}

void GetRingData()
{
    if (DumpOrSimuType > 0)
    {
        if (ModemStatus.RingDetected == 1)
        {
            if (DumpRingCount < 255) { DumpRingData[DumpRingCount++] = uRingDetTimer; }
        }

        uRingDetTimer++;
    }
}

void WriteDataFile(UWORD NameNum)
{
    CHAR  PCMinNamebuf[60];
    CHAR  DTEinNamebuf[60];
#if DUMP_OUT_DATA_ENABLE
    CHAR  PCMouNamebuf[60];
    CHAR  DTEouNamebuf[60];
#endif

    if (Case_State == 1)
    {
        sprintf(PCMinNamebuf, "D:\\dumpdata\\PCMin\\Pin%d.dat", NameNum);
        sprintf(DTEinNamebuf, "D:\\dumpdata\\DTEin\\Din%d.dat", NameNum);

        PCMinWrFile = fopen(PCMinNamebuf, "wb");
        DTEinWrFile = fopen(DTEinNamebuf, "wb");

#if DUMP_OUT_DATA_ENABLE
        sprintf(PCMouNamebuf, "D:\\dumpdata\\PCMou\\Pou%d.dat", NameNum);
        sprintf(DTEouNamebuf, "D:\\dumpdata\\DTEou\\Dou%d.dat", NameNum);

        PCMouWrFile = fopen(PCMouNamebuf, "wb");
        DTEouWrFile = fopen(DTEouNamebuf, "wb");
#endif

        if (PCMinWrFile == NULL || DTEinWrFile == NULL
#if DUMP_OUT_DATA_ENABLE
            || PCMouWrFile == NULL || DTEouWrFile == NULL
#endif
           )
        {
            TRACE0("Disk1 Full!!!");
            DiskFull_Flag = 1;
        }

        Case_State = 2;
    }

    if (Case_State == 2)
    {
#if DUMP_OUT_DATA_ENABLE
        DiskFreeSpace -= ((DumpPCMinCount + DumpDTEinCount + DumpPCMouCount + DumpDTEouCount) << 1);
#else
        DiskFreeSpace -= ((DumpPCMinCount + DumpDTEinCount) << 1);
#endif

        if (DiskFreeSpace > 1024)
        {
            fwrite(&DumpPCMinData, sizeof(QWORD), DumpPCMinCount, PCMinWrFile);
            fwrite(&DumpDTEinData, sizeof(UWORD), DumpDTEinCount, DTEinWrFile);

            DumpPCMinCount = 0;
            DumpDTEinCount = 0;

#if DUMP_OUT_DATA_ENABLE
            fwrite(&DumpPCMouData, sizeof(QWORD), DumpPCMouCount, PCMouWrFile);
            fwrite(&DumpDTEouData, sizeof(UWORD), DumpDTEouCount, DTEouWrFile);

            DumpPCMouCount = 0;
            DumpDTEouCount = 0;
#endif
        }
        else
        {
            TRACE1("DiskFreeSpace=%d, Disk2 Full!!!", DiskFreeSpace);
            DiskFull_Flag = 1;
        }
    }

    if (Case_State > 0 && ((DiskFull_Flag == 1) || (ESC_flag == 1) || (CaseEnd_Flag == 1)))
    {
        if (PCMinWrFile != NULL)
        {
            fwrite(&DumpRingData,  sizeof(UWORD), DumpRingCount, PCMinWrFile);
            fwrite(&DumpRingCount, sizeof(QWORD), 1,             PCMinWrFile);

            DumpRingCount = 0;
            uRingDetTimer = 0;
        }

        fclose(PCMinWrFile);
        fclose(DTEinWrFile);
#if DUMP_OUT_DATA_ENABLE
        fclose(PCMouWrFile);
        fclose(DTEouWrFile);
#endif

        DiskFreeSpace -= ((DumpRingCount + 1) << 1);

        DiskFreeSpace &= 0xFFFFFE00;

        if (DiskFreeSpace < 8192)
        {
            TRACE0("Disk3 Full!!!");
            DiskFull_Flag = 1;
        }
    }
}

void WriteDataFile_Total(UWORD NameNum)
{
    CHAR  PCMinNamebuf[60];
    CHAR  DTEinNamebuf[60];
#if DUMP_OUT_DATA_ENABLE
    CHAR  PCMouNamebuf[60];
    CHAR  DTEouNamebuf[60];
#endif

    if (DumpOrSimuType > 2)
    {
        sprintf(PCMinNamebuf, "M:\\PCMin\\Pin%d.dat", NameNum);
        sprintf(DTEinNamebuf, "M:\\DTEin\\Din%d.dat", NameNum);
#if DUMP_OUT_DATA_ENABLE
        sprintf(PCMouNamebuf, "M:\\PCMou\\Pou%d.dat", NameNum);
        sprintf(DTEouNamebuf, "M:\\DTEou\\Dou%d.dat", NameNum);
#endif
    }
    else
    {
        sprintf(PCMinNamebuf, "Pin%d.dat", NameNum);

#if DUMP_OUT_DATA_ENABLE
        sprintf(PCMouNamebuf, "Pou%d.dat", NameNum);
        sprintf(DTEouNamebuf, "Dou%d.dat", NameNum);
#endif

        if (DumpOrSimuType == 2)
        {
            sprintf(DTEinNamebuf, "Din%d.dat", NameNum);
        }
    }

    PCMinWrFile = fopen(PCMinNamebuf, "wb");

#if DUMP_OUT_DATA_ENABLE
    PCMouWrFile = fopen(PCMouNamebuf, "wb");
    DTEouWrFile = fopen(DTEouNamebuf, "wb");

    if (PCMinWrFile != NULL && PCMouWrFile != NULL)
#else
    if (PCMinWrFile != NULL)
#endif
    {
        fwrite(&DumpPCMinData, sizeof(QWORD), DumpPCMinCount, PCMinWrFile);
        fwrite(&DumpRingData,  sizeof(UWORD), DumpRingCount,  PCMinWrFile);
        fwrite(&DumpRingCount, sizeof(QWORD), 1,              PCMinWrFile);

        fclose(PCMinWrFile);

#if DUMP_OUT_DATA_ENABLE
        fwrite(&DumpPCMouData, sizeof(QWORD), DumpPCMouCount, PCMouWrFile);

        fclose(PCMouWrFile);
#endif
    }

    if (DumpOrSimuType >= 2)
    {
        DTEinWrFile = fopen(DTEinNamebuf, "wb");

        if (DTEinWrFile != NULL)
        {
            fwrite(&DumpDTEinData, sizeof(UWORD), DumpDTEinCount, DTEinWrFile);

            fclose(DTEinWrFile);
        }

#if DUMP_OUT_DATA_ENABLE
        DTEouWrFile = fopen(DTEouNamebuf, "wb");

        if (DTEouWrFile != NULL)
        {
            fwrite(&DumpDTEouData, sizeof(UWORD), DumpDTEouCount, DTEouWrFile);

            fclose(DTEouWrFile);
        }

#endif
    }
}

#endif

void PrintHelp(void)
{
    TRACE0("HOST [Option]");
    TRACE0("Option:  -c[n]    n=1,2 COM1 or COM2");
    TRACE0("Option:  -b[n]    n=1152,576,384,192,96,48,24 for different baudrate");
    TRACE0("Option:  -p[n]    n=no, odd, even for parity type");
    TRACE0("Option:  -data[n] n=8,7 for data bits");
    TRACE0("Option:  -stop[n] n=1,2 for stop bits");
    TRACE0("Option:  -s[nm]   n=0,1,2,3,4: NO, PCM, +DTE, MULTI(AT&F), MULTI");
    TRACE0("                  m=01 ~ 10: 1M ~ 10M DiskFreeSpace");
    TRACE0("Option:  -noprint (Disable debugging print)");
    TRACE0("Dafault: -c1 -b1152 -pno -data8 -stop1 -s0");
    TRACE0("Option:  -h       (This help screen)");
}

UBYTE  SER_SUPPORT;

#if 0/*For debug, initial string*/
char cpstringInit[] = {"ats0=1+m=7\r"};
#endif

#if DUMPSIMDATA
char cpstring2[] = {"atx4\r"};
char cpstring1[] = {"at&k0+m=8\r"};
char cpstringCall[] = {"atdt1\r"};
char cpstringAns[] = {"atas0=1\r"};
char Hangupstring1[] = {"+++"};
char Hangupstring2[] = {"ath\r"};
UBYTE sCallFlag = 0;   /*1 for call, 0 for ans*/
UWORD uAnsCount = 500; /*500 for 5 sec after the program begins, ATA will be issued*/
UBYTE ATA_Flag  = 0;
#endif

#if DTE_OUT_FILE
UWORD qDte_Count;
UWORD qDte_Data;
#endif

void Switch_Log();

void main(UBYTE argc, char *argv[])
{
    UDWORD i;
    time_t time_of_day;
    auto char buf[26];

#if MC_RECORDER
    DpcsStruct *pDpcs;
    ACEStruct  *pAce;
    CircBuffer *DteWr;
#endif

#if DUMPSIMDATA
    DumpOrSimuType = 0;
    Case_State     = 0;
#endif

    pubModemDataMem = (UBYTE *)&LibData;

#if DTE_OUT_FILE

    if ((DTEout_Ifp = fopen("data_in\\Dou.dat", "r+b")) == NULL)
    {
        TRACE0("The file DTEout.dat not opened");
    }

#endif

#if MC_RECORDER
    mcrInit();

#if TIMER_ON
    mcrTimersInit();
#endif
#endif

#if COMPARE_BYTE
    byteCmpInit();
#endif

    Gprintf_Flag = 1;

    /* Default support debugging printing, and defualt COM port */
    iSerPort   = SER_COM2;
    iSerIRQ    = SER_IRQ_COM2;
    iSerBaud   = 115200;
    iSerParity = SER_LCR_NOPARITY;
    iSerData   = SER_LCR_8BITS;
    iSerStop   = SER_LCR_1STOPBIT;

    /* Parsing input argument */
    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            if (strncmp(argv[i], "-c", 2) == 0)
            {
                if (argv[i][2] == '1')
                {
                    iSerPort = SER_COM1;
                    iSerIRQ = SER_IRQ_COM1;
                }
                else if (argv[i][2] == '2')
                {
                    iSerPort = SER_COM2;
                    iSerIRQ = SER_IRQ_COM2;
                }
            }
            else if (strncmp(argv[i], "-h", 2) == 0)
            {
                PrintHelp();
                return;
            }
            else if (strncmp(argv[i], "-noprint", 8) == 0)
            {
                TRACE0("!! Debugging Printout DISABLE  !!");
                Gprintf_Flag = 0;
            }
            else if (strncmp(argv[i], "-b", 2) == 0)
            {
                if (strncmp(argv[i], "-b1152", 6) == 0)
                {
                    iSerBaud = 115200;
                }
                else if (strncmp(argv[i], "-b576",  5) == 0)
                {
                    iSerBaud = 57600;
                }
                else if (strncmp(argv[i], "-b384",  5) == 0)
                {
                    iSerBaud = 38400;
                }
                else if (strncmp(argv[i], "-b192",  5) == 0)
                {
                    iSerBaud = 19200;
                }
                else if (strncmp(argv[i], "-b96",   4) == 0)
                {
                    iSerBaud = 9600;
                }
                else if (strncmp(argv[i], "-b48",   4) == 0)
                {
                    iSerBaud = 4800;
                }
                else if (strncmp(argv[i], "-b24",   4) == 0)
                {
                    iSerBaud = 2400;
                }
            }
            else if (strncmp(argv[i], "-p", 2) == 0)
            {
                if (strncmp(argv[i], "-pno", 4) == 0)
                {
                    iSerParity = SER_LCR_NOPARITY;
                }
                else if (strncmp(argv[i], "-podd",  5) == 0)
                {
                    iSerParity = SER_LCR_ODDPARITY;
                }
                else if (strncmp(argv[i], "-peven", 6) == 0)
                {
                    iSerParity = SER_LCR_EVENPARITY;
                }
            }
            else if (strncmp(argv[i], "-data", 5) == 0)
            {
                if (strncmp(argv[i], "-data8", 6) == 0)
                {
                    iSerData = SER_LCR_8BITS;
                }
                else if (strncmp(argv[i], "-data7", 6) == 0)
                {
                    iSerData = SER_LCR_7BITS;
                }
            }
            else if (strncmp(argv[i], "-stop", 5) == 0)
            {
                if (strncmp(argv[i], "-stop1", 6) == 0)
                {
                    iSerStop = SER_LCR_1STOPBIT;
                }
                else if (strncmp(argv[i], "-stop2", 6) == 0)
                {
                    iSerStop = SER_LCR_2STOPBITS;
                }
            }

#if DUMPSIMDATA
            else if (strncmp(argv[i], "-s", 2) == 0)
            {
                if (argv[i][2] == '1')
                {
                    DumpOrSimuType = 1;

                    if (argv[i][3] == 'c')/*c: Call*/
                    {
                        sCallFlag = 1;
                    }
                    else if (argv[i][3] == 'a')/*a: Answer*/
                    {
                        sCallFlag = 0;
                    }
                }
                else if (argv[i][2] == '2')
                {
                    DumpOrSimuType = 2;
                }
                else if (argv[i][2] == '3' || argv[i][2] == '4')
                {
                    if (argv[i][2] == '3')
                    {
                        DumpOrSimuType = 3;
                    }
                    else if (argv[i][2] == '4')
                    {
                        DumpOrSimuType = 4;
                    }

                    if (COMM_IsNumber(argv[i][3]) && COMM_IsNumber(argv[i][4]))
                    {
                        DiskFreeSpace = (argv[i][4] - '0') * 10 + (argv[i][3] - '0');
                        DiskFreeSpace *= 100000000;
                    }
                }
                else
                {
                    DumpOrSimuType = 0;
                }
            }

#else
            else if (strncmp(argv[i], "-s", 2) == 0)
            {
            }

#endif
            else if (strncmp(argv[i], "-d", 2) == 0)
            {
                TRACE0("!! Input Debug Parameter  !!");
                scanf("%d", &Input_Value);
                TRACE1("Input_Value = %d", Input_Value);
            }
            else
            {
                PrintHelp();
                return;
            }
        }
    }

    /* Open COM port */
    SER_SUPPORT = SerPortInit();

    if (SER_SUPPORT)
    {
        TxIntEnd = 1;

        if (iSerPort == SER_COM1)
        {
            TRACE1("Opening COM 1 with Baudrate of %d", iSerBaud);
        }
        else
        {
            TRACE1("Opening COM 2 with Baudrate of %d", iSerBaud);
        }

        DTEDumpSize = iSerBaud / 1000; /*DTESPEED(115200)/1000*/ /*(38) for DTESPEED(38400)/1000*/
    }

    /* Initialize Modem system memory & Initialize Modem device driver */
    if (GAO_Lib_Init(pubModemDataMem, sizeof(GAO_LibStruct)) < 0)
    {
        exit(-1);
    }

#if DRAWEPG
    init_graf();
#endif

    UartInit();

#if 0/*For debug, initial string*/
    SendCharToUartRx(cpstringInit, (UWORD)strlen(cpstringInit));
#endif

#if DUMPSIMDATA

    if (DumpOrSimuType == 1)
    {
        Hook(ON_HOOK);

        SendCharToUartRx(cpstring2, (UWORD)strlen(cpstring2));

        SendCharToUartRx(cpstring1, (UWORD)strlen(cpstring1));

        if (sCallFlag)
        {
            SendCharToUartRx(cpstringCall, (UWORD)strlen(cpstringCall));
        }
    }

#endif

    /* Initialize status structure */
    if (GAO_ModStatus_Init(&ModemStatus) < 0)
    {
        exit(-1);
    }

#if MC_RECORDER
    pDpcs = (DpcsStruct *)(((UBYTE **)pubModemDataMem)[DPCS_STRUC_IDX]);
    pAce  = (ACEStruct *)(((UBYTE **)pubModemDataMem)[ACE_STRUC_IDX]);
#endif

    Switch_Log();

    TRACE0("\n\nFollow Case Number = 1");

    time_of_day = time(NULL);
    TRACE1("%s", _ctime(&time_of_day, buf));

#if SUPPORT_PCI_BOARD
    Check_Telephony_function(&ModemStatus);
    ModemStatus.FunctionReq = (UWORD)((ModemStatus.DriverFunc) & 0x0000FFFF);
    Enable_Telephony_function(&ModemStatus);
#endif

    Processor_Side_Initialize();

    while (ESC_flag == 0)
    {
#if SUPPORT_PCI_BOARD

        if (Interrupt_Status & 0x0f)
#endif
        {
            DriverXferIn();

            HwCheckState(&ModemStatus);

#if SUPPORT_PCI_BOARD
            Interrupt_Status = 0x00;

            MdmSwiHandler((QWORD *)pPCMIn, (QWORD *)pPCMOut);/* run host and modem */
#else /*SUPPORT_HAWKBOARD*/

            MdmSwiHandler(gPCMInBuf, gPCMOutBuf);/* run host and modem */

#endif

            HwChgState(&ModemStatus);

            Data_Send_Prepare();

#if SUPPORT_PCI_BOARD

            if (Interrupt_Status != 0x00)
            {
                TRACE1("Out of MIPs, Interrupt_Status=%d", Interrupt_Status);
            }

#endif
        }
    }

    Device_Close();

    Hook(ON_HOOK);

    outp(iSerPort + SER_MODEM_CONTROL, inp(iSerPort + SER_MODEM_CONTROL) & ~SER_MCR_DTR);
    TRACE0("DSR reset");

    /* restore old serial interrupt handler */
    SerRestorOldHandler();

#if MC_RECORDER
    mcrSave();
#endif

#if DUMPSIMDATA

    if (DumpOrSimuType == 1 || DumpOrSimuType == 2)
    {
        WriteDataFile_Total(1);
    }
    else if (DumpOrSimuType > 2)
    {
        if (DiskFull_Flag == 0)
        {
            WriteDataFile(CaseNumber);
        }
    }

#endif

    TRACE0("BYE");

#if DRAWEPG
    /* Reset graphics if supported */
    done_graf();
#endif

#if DTE_OUT_FILE
    fclose(DTEout_Ifp);
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
        j = 6;

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

void GlobalVariable_EachCase_Init()
{
    UartTxIn  = 0;
    UartTxOut = 0;
    UartRxIn  = 0;
    UartRxOut = 0;
}

/* Interrupt (Thread) handler */
void MdmSwiHandler(QWORD *PCMInBuf, QWORD *PCMOutBuf)
{
    UWORD  uLen, uLenT;
    int    i;
    UBYTE  ch;
    time_t time_of_day;
    auto char buf[26];

    if (InitHW_Flag == 1)
    {
        GlobalVariable_EachCase_Init();

#if SUPPORT_HAWKBOARD
        Processor_Side_Initialize();
#endif

        InitHW_Flag = 0;
    }

    Count ++;

#if DUMPSIMDATA

    if (DumpOrSimuType == 1)
    {
        if ((sCallFlag == 0) && uAnsCount-- == 0 && ATA_Flag == 0)
        {
            SendCharToUartRx(cpstringAns, (UWORD)strlen(cpstringAns));

            ATA_Flag = 1;
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

#endif

    /*  for input from keyboard */
    if (kbhit())
    {
        ch = (UBYTE)getch();

        if (ch == '$' || ch == 27)
        {
            ESC_flag = 1;
        }
        else
        {
            /* Copy console input char to modem */
            GAO_Buf_In(pubModemDataMem, &ch, 1);

            /*if(!SER_SUPPORT) putch(ch);*/
        }
    }

    /* Check for any data coming from UART driver */

    /*Get how many data modem can accept*/
    uLenT = GAO_InBuf_Chk(pubModemDataMem);

    if (uLenT > UBDTE_BUF_SIZE)
    {
        uLenT = UBDTE_BUF_SIZE;
    }

    /*Get the data from terminal*/
    /*return got data size, uLen<=uLenT*/
    if (uLenT > 0)
    {
        uLen = GetCharFrUart(ubDTE_Buf, uLenT);
    }
    else
    {
        uLen = 0;
    }

#if MC_RECORDER
    mcrRecord(3, uLen);
#endif

#if DUMPSIMDATA

    if (Case_State == 0)
    {
        if ((DumpOrSimuType == 3 && ATandF_Flag == 1) || (DumpOrSimuType == 4))
        {
            Case_State     = 1;
            DumpRingCount  = 0;
            DumpPCMinCount = 0;
            uRingDetTimer  = 0;
            DumpDTEinCount = 0;
#if DUMP_OUT_DATA_ENABLE
            DumpPCMouCount = 0;
            DumpDTEouCount = 0;
#endif
        }
        else if (DumpOrSimuType == 1 || DumpOrSimuType == 2)
        {
            Case_State = 1;
        }
    }

    if (Case_State > 0)
    {
        if (DumpDTEinCount < MAX_DATA_SIZE) { DumpDTEinData[DumpDTEinCount++] = uLen; }

        if (uLen > 0)
        {
            for (i = 0; i < uLen; i++)
            {
                if (DumpDTEinCount < MAX_DATA_SIZE) { DumpDTEinData[DumpDTEinCount++] = ubDTE_Buf[i]; }
            }
        }
    }

    GetRingData();
#endif

#if !DUMPOTHERDTEOUTFILE

    /*Send the data to modem*/
    if (uLen > 0)
    {
#if COMPARE_BYTE

        if (pAce->LineState == ONLINE_DATA)
        {
            for (i = 0; i < uLen; i++)
            {
                byteCmpCompare(BYTECMP_CH_A, ubDTE_Buf[i]);
            }
        }

#endif

        GAO_Buf_In(pubModemDataMem, ubDTE_Buf, uLen);
    }

#endif

#if 0//MC_RECORDER
    mcrRecord(5, DteWr->CurrLen);
#endif

    UartChgState(&ModemStatus);

#if TIMER_ON
    StartTimer(0);
#endif

    GAO_PCM_Proc(pubModemDataMem, PCMInBuf, PCMOutBuf); /* Modem Main function block */

    GAO_ModStatus_Upd(pubModemDataMem, &ModemStatus);

#if TIMER_ON
    mcrRecord(0, StopTimer(0));
#endif

#if MC_RECORDER
    mcrRun();
#endif

    if ((ModemStatus.StateChange & HOOK_STATE_CHG) && (ModemStatus.HookState == ON_HOOK) && CaseStart_Flag == 1)
    {
        CaseEnd_Count    = Count;
        CaseWillEnd_Flag = 1;
        CaseStart_Flag   = 0;
    }

    if ((ModemStatus.StateChange & HOOK_STATE_CHG) && (ModemStatus.HookState == OFF_HOOK))
    {
        CaseStart_Flag = 1;
    }

    if (CaseWillEnd_Flag == 1 && (Count == CaseEnd_Count + 10))
    {
        CaseEnd_Flag = 1;
    }

    /* Check for any modem data need to be sent to DTE (UART) */

    /*Get how many data UART can accept*/
    uLenT = CheckUartTxBuf();

    if (uLenT > UBDTE_BUF_SIZE)
    {
        uLenT = UBDTE_BUF_SIZE;
    }

    uLen = 0;

#if DTE_OUT_FILE

    if (DTEoutFile_StartFlag == 0)
    {
#endif

        /*Get the data from modem*/
        /*return got data size, uLen<=uLenT*/
        if (uLenT > 0)
        {
            uLen = GAO_Buf_Out(pubModemDataMem, ubDTE_Buf, uLenT);
        }

#if DUMPSIMDATA

        if (DumpOrSimuType >= 2)
        {
            if (DumpDTEinCount < MAX_DATA_SIZE) { DumpDTEinData[DumpDTEinCount++] = uLen; }

#if DUMP_OUT_DATA_ENABLE

            if (DumpDTEouCount < MAX_DATA_SIZE) { DumpDTEouData[DumpDTEouCount++] = uLen; }

            if (uLen > 0)
            {
                for (i = 0; i < uLen; i++)
                {
                    if (DumpDTEouCount < MAX_DATA_SIZE) { DumpDTEouData[DumpDTEouCount++] = ubDTE_Buf[i]; }
                }
            }

#endif
        }

#endif

#if DTE_OUT_FILE

        if (uLen > 0)
        {
            for (i = 0; i < uLen; i++)
            {
                if (ubDTE_Buf[i] == 255) /* 255; 181; 230; 237; 209 */
                {
                    DTEoutFile_StartFlag = 1;
                }
            }
        }
    }
    else
    {
        if (fread(&qDte_Count, sizeof(UWORD), 1, DTEout_Ifp) == 0)
        {
            TRACE0("DTEout_Ifp: Read Data End");
        }

        /* Check for any modem data need to be sent to DTE (UART) */
        if (qDte_Count > 0)
        {
            if (qDte_Count > uLenT)
            {
                TRACE0("Error: no enough space in UART buffer");
                TRACE2("qDte_Count=%d; uLenT=%d", qDte_Count, uLenT);
                qDte_Count = uLenT;
            }

            for (i = 0; i < qDte_Count; i++)
            {
                if (fread(&qDte_Data, sizeof(UWORD), 1, DTEout_Ifp) == 0)
                {
                    TRACE0("DTEout_Ifp Error: Read Data and Write data End unexpectedly");
                }

                ubDTE_Buf[i] = (UBYTE)qDte_Data;
            }

            uLen = qDte_Count;
        }
    }

#endif

#if PRINT_CHAR_TO_DTE/*Print out the characters to DTE*/

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

    /*Send the data to terminal*/
    if (uLen > 0)
    {
        _disable();

        SendCharToUart(ubDTE_Buf, uLen);

        /* If there is no Tx buffer empty interrupt, we will have to
        transmit at least one char to generate the new interrupt.
        */

        if (TxIntEnd)
        {
            if (SER_SUPPORT)
            {
                ch = GetOneCharFrUartTx();
                TxIntEnd = 0;

                /* This is always the last step when transmitting data
                to SER_TXBUFFER because Tx buffer empty interrupt
                can be generated from UART chip very quickly.
                */
                outp(iSerPort + SER_TXBUFFER, ch);
            }
        }

        _enable();
    }

#if DUMPSIMDATA

    if (DumpOrSimuType > 2 && DiskFull_Flag == 0)
    {
        WriteDataFile(CaseNumber);
    }

#endif

    if (CaseEnd_Flag == 1)
    {
#if DUMPSIMDATA
        ATandF_Flag      = 0;
        Case_State       = 0;
#endif
        CaseEnd_Count    = 0;
        CaseWillEnd_Flag = 0;
        CaseEnd_Flag     = 0;
        Count            = 0;

        CaseNumber++;
        TRACE1("\n\nFollow Case Number = %d", CaseNumber);
        HwInit((UBYTE **)pubModemDataMem);

        time_of_day = time(NULL);
        TRACE1("%s", _ctime(&time_of_day, buf));

        InitHW_Flag      = 1;

#if DRAWEPG
        init_graf();
#endif
    }
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

void Switch_Log()
{
    TRACE1("VERSION_LABEL                = %s", VERSION_LABEL);

    TRACE1("SUPPORT_REALTIME             = %d", SUPPORT_REALTIME);
    TRACE1("SUPPORT_SIMULATION           = %d", SUPPORT_SIMULATION);

    TRACE1("SUPPORT_T31                  = %d", SUPPORT_T31);
    TRACE1("SUPPORT_T32                  = %d", SUPPORT_T32);

    TRACE1("SUPPORT_V17                  = %d", SUPPORT_V17);
    TRACE1("SUPPORT_V27TER               = %d", SUPPORT_V27TER);
    TRACE1("SUPPORT_V29                  = %d", SUPPORT_V29);

    TRACE1("SUPPORT_V21M                 = %d", SUPPORT_V21M);
    TRACE1("SUPPORT_V23                  = %d", SUPPORT_V23);
    TRACE1("SUPPORT_V22BIS               = %d", SUPPORT_V22BIS);
    TRACE1("SUPPORT_V32                  = %d", SUPPORT_V32);
    TRACE1("SUPPORT_V32BIS               = %d", SUPPORT_V32BIS);
    TRACE1("SUPPORT_V34                  = %d", SUPPORT_V34);

    TRACE1("SUPPORT_V90A                 = %d", SUPPORT_V90A);
    TRACE1("SUPPORT_V90D                 = %d", SUPPORT_V90D);

    TRACE1("SUPPORT_B103                 = %d", SUPPORT_B103);
    TRACE1("SUPPORT_B212A                = %d", SUPPORT_B212A);

    TRACE1("SUPPORT_MNP                  = %d", SUPPORT_MNP);
    TRACE1("SUPPORT_V42                  = %d", SUPPORT_V42);

    TRACE1("SUPPORT_AUTO                 = %d", SUPPORT_AUTO);

    TRACE1("SUPPORT_V42INFO              = %d", SUPPORT_V42INFO);
    TRACE1("SUPPORT_V42BIS               = %d", SUPPORT_V42BIS);
    TRACE1("SUPPORT_V44                  = %d", SUPPORT_V44);
    TRACE1("SUPPORT_V42_SREJ             = %d", SUPPORT_V42_SREJ);
    TRACE1("SUPPORT_V42_MSREJ            = %d", SUPPORT_V42_MSREJ);

#if SUPPORT_V42
    TRACE1("PROTOCOL_DEFAULT             = %d", PROTOCOL_DEFAULT);
#endif

    TRACE1("SUPPORT_V54                  = %d", SUPPORT_V54);

    TRACE1("SAMCONV                      = %d", SAMCONV);

#if SUPPORT_V90D
    TRACE1("A_MU_LAW_PCM                 = %d", A_MU_LAW_PCM);
#endif

    TRACE1("SAMPLE_RATE_INIT_7200        = %d", SAMPLE_RATE_INIT_7200);
    TRACE1("SAMPLE_RATE_INIT_9600        = %d", SAMPLE_RATE_INIT_9600);

    TRACE1("SUPPORT_CID_DET              = %d", SUPPORT_CID_DET);
    TRACE1("SUPPORT_CID_GEN              = %d", SUPPORT_CID_GEN);

#if SUPPORT_CID_DET
    TRACE1("CID_SNOOP_MODE               = %d", CID_SNOOP_MODE);
#endif

    TRACE1("CID_HW7200SAMPLERATE_SUPPORT = %d", CID_HW7200SAMPLERATE_SUPPORT);
    TRACE1("V27_HW7200SAMPLERATE_SUPPORT = %d", V27_HW7200SAMPLERATE_SUPPORT);

    TRACE1("SUPPORT_DTMF                 = %d", SUPPORT_DTMF);

    TRACE1("SUPPORT_ENERGY_REPORT        = %d", SUPPORT_ENERGY_REPORT);

    TRACE1("DUALMODEMSIM                 = %d", DUALMODEMSIM);

    TRACE1("SUPPORT_PCI_BOARD            = %d", SUPPORT_PCI_BOARD);
    TRACE1("SUPPORT_HAWKBOARD            = %d", SUPPORT_HAWKBOARD);

    TRACE1("HAWK_ADI                     = %d", HAWK_ADI);
    TRACE1("HAWK_SLAB                    = %d", HAWK_SLAB);
    TRACE1("HAWK_ST7550                  = %d", HAWK_ST7550);

    TRACE1("T1FRAMER                     = %d", T1FRAMER);

    TRACE1("DUMPSIMDATA                  = %d", DUMPSIMDATA);
    TRACE1("DUMP_OUT_DATA_ENABLE         = %d", DUMP_OUT_DATA_ENABLE);
}
