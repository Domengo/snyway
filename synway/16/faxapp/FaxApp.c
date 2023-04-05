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

// This file for simulation

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "gaoapi.h"
#include "gaomsg.h"

#define TIME_MEASURE  (0)

#ifndef _WIN32
#include <pthread.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <sys/times.h>
#include <spandsp.h>
#include <zaptel.h>
#include <unistd.h>
#elif TIME_MEASURE
#include <wtypes.h>
#endif

#include "main.h"

#define MAX_TIFF_FILE_LENGTH                (1024*1024)
#define FAX_DATA_STRUCT_SIZE                (0x148f94)//0x50000
#define PCM_BUF_SIZE_INIT                   (80)
#define PCM_BUF_LENGTH_MAX                  (0x1000)

#define CHANNEL_NO    (1)
#define DATA_RECORD   (0)
#define DATA_REPLAY   (!DATA_RECORD)

#ifdef _WIN32
#define FOLDER_NAME "z:\\var\\tmp\\"
//#define FOLDER_NAME "G:\\gaoresearch\\gr_engineering\\QA_TEST_DATA\\bieta\\Bieta19f\\Interruption\\R1\\"
//#define FOLDER_NAME "D:\\data\\issue179\\"
//#define FOLDER_NAME ""
#else
#define FOLDER_NAME "/var/tmp/"
#endif

#define TEST_CASE ""

extern UWORD AppRecvMsg(GAO_Msg *pMsg);

GAO_ModStatus ModemStatus;
int FaxDataStruct[FAX_DATA_STRUCT_SIZE/4 + 1];
GAO_Msg AppGaoMsg;
char Namebuf[20];
#define ZX_READ_DEFAULT (80)
#define ZX_READ_SAMPLES (96)
#define ZX_READ_BYTES   (ZX_READ_SAMPLES << 1)
#define ZX_SUCCESS (0)
#define ZX_ERROR   (-1)

#ifndef _WIN32
#if DATA_RECORD
// REFERENCE LINKS
//   http://linux.die.net
//   http://wiki.freeswitch.org/wiki/Zapata_zaptel_interface

// BLACKFIN TOOLCHAIN DOWNLOAD
//   wget http://blackfin.uclinux.org/gf/download/frsrelease/375/4037/blackfin-toolchain-08r1-8.i386.tar.gz
//   wget http://blackfin.uclinux.org/gf/download/frsrelease/375/4041/blackfin-toolchain-uclibc-default-08r1-8.i386.tar.gz

// SET ENVIRONMENT PATH TO INCLUDE BLACKFIN TOOLCHAIN
//   PATH=$PATH:$HOME/bin:/opt/uClinux/bfin-uclinux/bin:/opt/uClinux/bfin-linux-uclibc/bin

// SLINEAR = Raw 16-bit Signed Linear (8000 Hz) PCM

// Sample configured to pickup 4 DTMF digits

int zxTest(int nChannel);

static void xquit(int num)
{
    exit(0);
}


void zxTonePlay(int nFD, int nTone)
{
    ioctl(nFD, ZT_SENDTONE, &nTone);
}

void zxLogSz(const char *psz)
{
    printf("%s\n", psz);
}

void zxLogSzn(const char *psz, int n)
{
    printf("%s%d\n", psz, n);
}

void zxLogEvent(int n)
{
    switch (n)
    {
        case ZT_EVENT_NONE:
            printf("  ZT_EVENT_NONE \n");
            break;
        case ZT_EVENT_ONHOOK:
            printf("  ZT_EVENT_ONHOOK \n");
            break;
        case ZT_EVENT_RINGOFFHOOK:
            printf("  ZT_EVENT_RINGOFFHOOK \n");
            break;
        default:
            printf("  EVENT = %d \n", n);
            break;
    }
}

int zxOptionSet(int nFD, int nRequest, int nData)
{
    int nOPT;

    nOPT = nData;

    return ioctl(nFD, nRequest, &nOPT);
}

void zxHangup(int nFD)
{
    zxOptionSet(nFD, ZT_HOOK, ZT_ONHOOK);
    zxTonePlay(nFD, -1);
}

int zxEventGet(int nFD)
{
    int nEvent;
    int nRC;

    nRC = ioctl(nFD, ZT_GETEVENT, &nEvent);

    if (nRC == -1)
    {
        return -1;
    }

    return nEvent;
}

int zxOpen(int nChannel)
{
    int nFD;
    int nRC;
    ZT_PARAMS xParams;

    struct zt_bufferinfo xBI;

    nFD = open("/dev/zap/channel", O_RDWR);

    if (nFD < 0)
    {
        return -1;
    }

    nRC = ioctl(nFD, ZT_SPECIFY, &nChannel);

    if (nRC == -1)
    {
        close(nFD);
        return -1;
    }

    zxOptionSet(nFD, ZT_SET_BLOCKSIZE, ZX_READ_SAMPLES);

    memset(&xParams, 0, sizeof(xParams));
    nRC = ioctl(nFD, ZT_GET_PARAMS, &xParams);
    xParams.channo = nChannel;
    nRC = ioctl(nFD, ZT_SET_PARAMS, &xParams);

    memset(&xBI, 0, sizeof(xBI));
    nRC = ioctl(nFD, ZT_GET_BUFINFO, &xBI);
    xBI.rxbufpolicy = ZT_POLICY_IMMEDIATE;
    xBI.txbufpolicy = ZT_POLICY_IMMEDIATE;
    xBI.numbufs = 4;
    xBI.bufsize = ZX_READ_SAMPLES;
    nRC = ioctl(nFD, ZT_SET_BUFINFO, &xBI);

    zxOptionSet(nFD, ZT_SETLINEAR, 1);
    zxOptionSet(nFD, ZT_ECHOCANCEL, 0);
    zxOptionSet(nFD, ZT_TIMERCONFIG, ZX_READ_SAMPLES);

    return nFD;
}

int zxClose(int nFD)
{
    if (nFD == -1)
    {
        return ZX_ERROR;
    }

    close(nFD);

    return ZX_SUCCESS;
}

int zxDTMF(int nFD, int nDIGITS)
{
    dtmf_rx_state_t xDRS;
    char sz[256];
    QWORD xData[ZX_READ_SAMPLES];
    int nx;
    int nc;
    char ch;
    int n;

    nc = 0;

    dtmf_rx_init(&xDRS, NULL, NULL);
    dtmf_rx_parms(&xDRS, FALSE, 8, 4);

    for (n = 0; n < 1000; n++)
    {
        usleep(1);

        nx = read(nFD, xData, ZX_READ_BYTES);

        if (nx != ZX_READ_BYTES)
        {
            return nc;
        }

        dtmf_rx(&xDRS, xData, ZX_READ_SAMPLES);

        nx = dtmf_rx_get(&xDRS, sz, 1);

        if (nx < 1)
        {
            continue;
        }

        zxTonePlay(nFD, -1);

        ch = sz[0];
        nc = nc + 1;

        if (nc == 1)
        {
            printf("  DTMF=%c", ch);
        }
        else
        {
            printf("%c", ch);
        }

        if (nc == nDIGITS)
        {
            break;
        }

    }

    printf("\n");
    return nc;
}

int zxTickCount()
{
    struct tms xTMS;
    return times(&xTMS);
}

#endif

#if TIME_MEASURE
static void diff(struct timespec start, struct timespec end, struct timespec *diff)
{
    if ((end.tv_nsec - start.tv_nsec) < 0)
    {
        diff->tv_sec = end.tv_sec - start.tv_sec - 1;
        diff->tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        diff->tv_sec = end.tv_sec - start.tv_sec;
        diff->tv_nsec = end.tv_nsec - start.tv_nsec;
    }
}
#endif
#endif

void AppSendMsg(GAO_Msg *pMsg, UBYTE MsgType, UWORD MsgCode)
{
    pMsg->Direction = MSG_DOWN_LINK;
    pMsg->MsgType = MsgType;
    pMsg->MsgCode = MsgCode;
}

UDWORD DumpPCMinCount = 0L;
FILE *fTiffFile, *fBmpFile;
UBYTE AppTiffBuffer[MAX_TIFF_FILE_LENGTH + 4 + 10];
UBYTE IsJpegCompression = 0;

#if APP_SENT_COVER_PAGE
const char *gCallerSide[6] = {"John Chiney", "+1 416-234-5678", "+1 647-234-5678", "1600 Amphitheatre Parkway", "Mountain View, CA", "US 94043"};
const char *gCalleeSide[6] = {"Dan Kiney", "+1 416-432-8765", "+1 647-432-8765", "100 Amphitheatre Parkway", "Mountain View, CA", "US 94067"};
char gDateTimeStr[] = {"08/20/2010 14:50"};
UBYTE CoverPagePattern = 1; // default value
#endif

int main(int argc, char *argv[]) // first recv, then send
{
    FILE *fdPcmIn, *fdPcmOut;
    SWORD PCMInBuf[PCM_BUF_LENGTH_MAX];
    SWORD PCMOutBuf[PCM_BUF_LENGTH_MAX];
    char PhoneNumber[20] = "1234";
    int i = 0;
    //int RxTiffCount = 0;
    int SendFaxDoc = 0; // 0: receive a fax, 1: send a fax
    SWORD inBuffer[ZX_READ_SAMPLES], outBuffer[ZX_READ_SAMPLES + ZX_READ_DEFAULT];
    SWORD *pPCMIn = PCMInBuf, *pPCMLibIn = PCMInBuf, *pPCMOut = PCMOutBuf, *pPCMLibOut = PCMOutBuf;
    UBYTE isHangUp = 0, isEndOfSession = 0;
    int nWordsIn = 0, nWordsOut = 0;
    SWORD bufferSize = ZX_READ_DEFAULT;
    //char TifffileName[40];

#ifndef _WIN32
#if DATA_RECORD
    sigset_t sigs;
    int nPID;
    int nChannel;
    int nFD;
    int nRC;
    int nEvent;
    int nBytes;
    struct pollfd xFDS;
#endif
#if TIME_MEASURE
    struct timespec time1, time2, timeTemp;
    FILE *fTimeMeasure = fopen("timemeasure.dat", "wb");
#endif
#elif TIME_MEASURE
    LARGE_INTEGER litmp;
    LONGLONG QPart1, QPart2;
    double dfMinus;
    FILE *fTimeMeasure = fopen("timemeasure.dat", "wb");
#endif

    if (argc < 2)
    {
        printf("APP: miss parameter!\n");
        exit(-1);
    }

    if ((strncmp(argv[1], "-r", 2) == 0) || (strncmp(argv[1], "-R", 2) == 0))
    {
        SendFaxDoc = 0;
    }
    else if ((strncmp(argv[1], "-t", 2) == 0) || (strncmp(argv[1], "-T", 2) == 0))
    {
        SendFaxDoc = 1;
    }

    if (SendFaxDoc == 0)// receive a fax
    {
#if DATA_RECORD
        fdPcmIn  = fopen(FOLDER_NAME "pin_r.dat",  "wb");
#else
        fdPcmIn  = fopen(FOLDER_NAME "pin_r" TEST_CASE ".dat",  "rb");
#endif

        if (fdPcmIn == NULL)
        {
            printf("APP: recv fax, can't open the PCM input data file!\n");
            exit(-2);
        }

#if DATA_RECORD
        fdPcmOut  = fopen(FOLDER_NAME "pout_r.dat",  "wb");
#else
        fdPcmOut  = fopen("simpout.dat",  "wb");
#endif

        if (fdPcmOut == NULL)
        {
            printf("APP: recv fax, failed to create the PCM output file!\n");
            exit(-2);
        }
    }
    else // send a fax
    {
#if DATA_RECORD
        fdPcmIn  = fopen(FOLDER_NAME "pin_t.dat",  "wb");
#else
        fdPcmIn  = fopen(FOLDER_NAME "pin_t" TEST_CASE ".dat",  "rb");
#endif

        if (fdPcmIn == NULL)
        {
            printf("APP: send fax, can't open PCM input data file!\n");
            exit(-2);
        }

#if DATA_RECORD
        fdPcmOut  = fopen(FOLDER_NAME "pout_t.dat",  "wb");
#else
        fdPcmOut  = fopen("simpout.dat",  "wb");
#endif

        if (fdPcmOut == NULL)
        {
            printf("APP: send fax, failed to create the PCM output file!\n");
            exit(-2);
        }
    }

    printf("APP: Program starting here.\n");

    for (i = 0; i < PCM_BUF_LENGTH_MAX; i++)
    {
        PCMInBuf[i]  = 0;
        PCMOutBuf[i] = 0;
    }

#if !defined(_WIN32) && DATA_RECORD
    nPID = getpid();

    sigemptyset(&sigs);
    sigaddset(&sigs, SIGHUP);
    sigaddset(&sigs, SIGTERM);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGPIPE);
    sigaddset(&sigs, SIGWINCH);
    pthread_sigmask(SIG_BLOCK, &sigs, NULL);

    signal(SIGINT, xquit);
    signal(SIGTERM, xquit);
    signal(SIGPIPE, SIG_IGN);

    // seed the random number generators
    srand((unsigned int) nPID + (unsigned int) time(NULL));
    srandom((unsigned int) nPID + (unsigned int) time(NULL));

    printf("****** PID = %d ****** \n\n", nPID);

    pthread_sigmask(SIG_UNBLOCK, &sigs, NULL);

    nChannel = CHANNEL_NO;
#endif

#if !defined(_WIN32) && DATA_RECORD
    nFD = zxOpen(nChannel);

    if (nFD == -1)
    {
        zxLogSz("FAILED TO OPEN CHANNEL");
        exit(-3);
    }

    if (SendFaxDoc)
    {
        zxOptionSet(nFD, ZT_HOOK, ZT_RING);
    }

    for (i = 0; i < 1000; i++)
    {
        printf("%d: wait for event [%d] \n", nChannel, i);

        xFDS.fd = nFD;
        xFDS.events = POLLPRI;
        xFDS.revents = 0;
        poll(&xFDS, 1, -1);
        nEvent = zxEventGet(nFD);
        zxLogEvent(nEvent);

        switch (nEvent)
        {
            case ZT_EVENT_RINGOFFHOOK:

                if (SendFaxDoc)
                {
                    zxOptionSet(nFD, ZT_HOOK, ZT_RINGOFF);
                }
                else
                {
                    // setup initial dialtone on line
                    zxTonePlay(nFD, ZT_TONE_DIALTONE);

                    // pickup 4 dtmf digits
                    nRC = zxDTMF(nFD, 3);

                    if (nRC < 0)
                    {
                        exit(-3);
                    }
                }

                i = 1000; // DTMF detection finished
                break;

            case ZT_EVENT_ONHOOK:
                zxTonePlay(nFD, -1);
                break;
            default:
                break;
        }

        usleep(1);
    }

#endif

    printf("APP: Initializing system.\n");

    memset(&AppGaoMsg, 0, sizeof(GAO_Msg));

    if (GAO_Lib_Init(FaxDataStruct, sizeof(FaxDataStruct)) < 0)
    {
        exit(-1);
    }

    GAO_ModStatus_Init(&ModemStatus);
    AppSendMsg(&AppGaoMsg, MSG_TYPE_MAINTENANCE, MTN_REQUEST_FAXLIB_VERSION);

    while (AppGaoMsg.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
    }

    if ((AppGaoMsg.MsgType == MSG_TYPE_MAINTENANCE) && (AppGaoMsg.MsgCode == MTN_RESP_FAXLIB_VERSION))
    {
        printf("Gao Library Version is: %d.%d\n", AppGaoMsg.Buf[0], AppGaoMsg.Buf[1]);
    }
    else
    {
        printf("Wrong response to request version number!\n");
    }

    memset(&AppGaoMsg, 0, sizeof(GAO_Msg));

    memcpy(AppGaoMsg.Buf, "22222222", 8);

    AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_LOCAL_ID);

    while (AppGaoMsg.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
    }

    memset(&AppGaoMsg, 0, sizeof(GAO_Msg));
    AppGaoMsg.Buf[0] = DCE_RX_IMAGE_QUALITY_CHECK_CAPA;
    AppGaoMsg.Buf[1] = DCE_TX_IMAGE_QUALITY_CHECK_CAPA;
    AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_COPY_QUALITY_CHECK);

    while (AppGaoMsg.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
    }

    memset(&AppGaoMsg, 0, sizeof(GAO_Msg));
    AppGaoMsg.Buf[0] = MIN_GOODLINES_PERCENTAGE_IN_A_PAGE;
    AppGaoMsg.Buf[1] = MAX_CONSECU_BADLINES_IN_A_PAGE;
    AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_RECV_QUALITY_THRESHOLD);

    while (AppGaoMsg.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
    }

    memset(&AppGaoMsg, 0, sizeof(GAO_Msg));

    if (SendFaxDoc == 0) // receive a fax
    {
        //FaxApp.HookState = APP_OFF_HOOK;
        AppGaoMsg.Buf[0] = 1;
        AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_RING_NUMBER_FOR_AUTO_ANS);
        ModemStatus.RingDetected = 1;
    }
    else // send a fax
    {
#if APP_SENT_COVER_PAGE

        // cover page features
        for (i = 0; i < 6; i++)
        {
            AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_COVER_PAGE_ATTRIB);
            AppGaoMsg.Buf[0] = 1;
            AppGaoMsg.Buf[1] = i + 1;
            AppGaoMsg.Buf[2] = strlen(gCallerSide[i]);
            memcpy(&(AppGaoMsg.Buf[3]), gCallerSide[i], AppGaoMsg.Buf[2]);

            while (AppGaoMsg.Direction != MSG_DIR_IDLE)
            {
                GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
            }
        }

        for (i = 0; i < 6; i++)
        {
            AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_COVER_PAGE_ATTRIB);
            AppGaoMsg.Buf[0] = 2;
            AppGaoMsg.Buf[1] = i + 1;
            AppGaoMsg.Buf[2] = strlen(gCalleeSide[i]);
            memcpy(&(AppGaoMsg.Buf[3]), gCalleeSide[i], AppGaoMsg.Buf[2]);

            while (AppGaoMsg.Direction != MSG_DIR_IDLE)
            {
                GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
            }
        }

        AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_COVER_PAGE_ATTRIB);
        AppGaoMsg.Buf[0] = 3;
        AppGaoMsg.Buf[1] = 7;
        AppGaoMsg.Buf[2] = 1;
        AppGaoMsg.Buf[3] = CoverPagePattern;

        while (AppGaoMsg.Direction != MSG_DIR_IDLE)
        {
            GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
        }

        AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_COVER_PAGE_ATTRIB);
        AppGaoMsg.Buf[0] = 1;
        AppGaoMsg.Buf[1] = 0;
        AppGaoMsg.Buf[2] = strlen(gDateTimeStr);
        memcpy(&(AppGaoMsg.Buf[3]), gDateTimeStr, AppGaoMsg.Buf[2]);

        while (AppGaoMsg.Direction != MSG_DIR_IDLE)
        {
            GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
        }

        printf("APP: cover page cfg done.\n\n");
#endif
        // start to transmit
        // open the tif file which is being transmitting
#ifndef APP_SEND_FILE_TYPE
        printf("APP: APP_SEND_FILE_TYPE is not defined.\n");
        exit(-7);
#elif APP_SEND_FILE_TYPE == 0
        printf("APP: Transmit image.tif.\n");

        if ((fTiffFile = fopen("image.tif", "rb")) == 0)
        {
            printf("image.tif does not exist!\n");
            exit(-1);
        }

#elif APP_SEND_FILE_TYPE == 1

        printf("APP: Transmit image.bmp.\n");

        if ((fBmpFile = fopen("image.bmp", "rb")) == 0)
        {
            printf("image.bmp does not exist!\n");
            exit(-4);
        }

        if ((fTiffFile = fopen("image.tif", "wb")) == 0)
        {
            printf("Can note create image.tif!\n");
            exit(-5);
        }

        FileConversionFromBmpToTiff(fBmpFile, fTiffFile);
        fclose(fBmpFile);
        fclose(fTiffFile);
        printf("done with the conversion from bmp to tiff.\n");

        if ((fTiffFile = fopen("image.tif", "rb")) == 0)
        {
            printf("image.tif does not exist!\n");
            exit(-1);
        }

#elif APP_SEND_FILE_TYPE == 2
        printf("APP: Transmit test.txt.\n");

        if ((fTiffFile = fopen("test.txt", "rb")) == 0)
        {
            printf("test.txt does not exist!\n");
            exit(-1);
        }

#else
        printf("APP: APP_SEND_FILE_TYPE is not set correctly.\n");
        exit(-6);
#endif

#if APP_FILE_OPERATION

        AppGaoMsg.Buf[0] = 1; // file pointer
        AppGaoMsg.Buf[1] = 1; // 1 Tx tiff file
        AppGaoMsg.Buf[2] = (UBYTE)(((unsigned)(fTiffFile) >> 24) & 0xff);
        AppGaoMsg.Buf[3] = (UBYTE)(((unsigned)(fTiffFile) >> 16) & 0xff);
        AppGaoMsg.Buf[4] = (UBYTE)(((unsigned)(fTiffFile) >> 8) & 0xff);
        AppGaoMsg.Buf[5] = (UBYTE)(((unsigned)(fTiffFile)) & 0xff);

#else
        //fseek(fTiffFile, 0, SEEK_END);
        //i = ftell(fTiffFile);
        //fseek(fTiffFile, 0, SEEK_SET);
        i = fread(&(AppTiffBuffer[4]), 1, MAX_TIFF_FILE_LENGTH + 5, fTiffFile);

        if (i > MAX_TIFF_FILE_LENGTH)
        {
            printf("tiff file too big!\n");
            exit(-1);
        }

        //fread(&(AppTiffBuffer[4]), 1, i, fTiffFile);
        AppTiffBuffer[0] = (i >> 24) & 0xff;
        AppTiffBuffer[1] = (i >> 16) & 0xff;
        AppTiffBuffer[2] = (i >> 8) & 0xff;
        AppTiffBuffer[3] = (i >> 0) & 0xff;

        AppGaoMsg.Buf[0] = 0; // buffer pointer
        AppGaoMsg.Buf[1] = 1; // 1 Tx tiff file
        AppGaoMsg.Buf[2] = (UBYTE)(((unsigned)(&(AppTiffBuffer[0])) >> 24) & 0xff);
        AppGaoMsg.Buf[3] = (UBYTE)(((unsigned)(&(AppTiffBuffer[0])) >> 16) & 0xff);
        AppGaoMsg.Buf[4] = (UBYTE)(((unsigned)(&(AppTiffBuffer[0])) >> 8) & 0xff);
        AppGaoMsg.Buf[5] = (UBYTE)(((unsigned)(&(AppTiffBuffer[0]))) & 0xff);
#endif
        AppSendMsg(&AppGaoMsg, MSG_TYPE_CONFIG, CFG_TIFF_FILE_PTR_TX);

        while (AppGaoMsg.Direction != MSG_UP_LINK)
        {
            GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
        }

        if ((AppGaoMsg.MsgType == MSG_TYPE_REPORT) && (AppGaoMsg.MsgCode == RPT_VALIDITY_OF_TX_TIFF_FILE) && (AppGaoMsg.Buf[1] == 0))
        {
            printf("Check Tx Tiff File is Passed.\n");
        }
        else
        {
            printf("check Tx Tiff file failed!, error code is: %d\n", AppGaoMsg.Buf[1]);
            exit(-2);
        }

        memset(&AppGaoMsg, 0, sizeof(GAO_Msg));

        AppSendMsg(&AppGaoMsg, MSG_TYPE_CONTROL, CTL_FAX_CALL_ORG);
        AppGaoMsg.Buf[0] = 2; // tone dialling mode
        AppGaoMsg.Buf[1] = strlen(PhoneNumber); // length of phone number
        memcpy(&(AppGaoMsg.Buf[2]), PhoneNumber, AppGaoMsg.Buf[1]);
        printf("APP: SEND ATD COMMAND.\n");
        //FaxApp.HookState = APP_OFF_HOOK;
    }

    while (!isEndOfSession)
    {
#if !defined(_WIN32) && DATA_RECORD
        usleep(1);
#endif

        if (!isHangUp)
        {
#if DATA_RECORD
            // read data from channel
            nBytes = read(nFD, inBuffer, ZX_READ_BYTES);

            if (nBytes != ZX_READ_BYTES)
            {
                printf("  CHANNEL READ FAILED\n");
                isHangUp = 1;
            }

            if (nBytes != 0)
#endif
            {
#if DATA_REPLAY

                if (fread(inBuffer, 1, ZX_READ_BYTES, fdPcmIn) != ZX_READ_BYTES)
                {
                    isHangUp = 1;
                }

#endif
            }

            for (i = 0; i < ZX_READ_SAMPLES; i++)
            {
                *pPCMIn++ = inBuffer[i];

                if (pPCMIn >= &PCMInBuf[PCM_BUF_LENGTH_MAX])
                {
                    pPCMIn = PCMInBuf;
                }
            }

            nWordsIn += ZX_READ_SAMPLES;
        }
        else
        {
            memset(inBuffer, 0, sizeof(inBuffer));
        }

        while (nWordsIn >= bufferSize || isHangUp)
        {
            SWORD sTemp;
            UWORD Rev;

            if (nWordsIn >= bufferSize)
            {
                //TRACE("%s", "In: ")
                for (i = 0; i < bufferSize; i++)
                {
                    inBuffer[i] = *pPCMLibIn++;

                    //TRACE("%02x ", inBuffer[i]);
                    if (pPCMLibIn >= &PCMInBuf[PCM_BUF_LENGTH_MAX])
                    {
                        pPCMLibIn = PCMInBuf;
                    }
                }

                //TRACE0("");

#if DATA_RECORD
                fwrite(inBuffer, sizeof(SWORD), bufferSize, fdPcmIn);
                fflush(fdPcmIn);
#endif
            }

#if TIME_MEASURE
#ifdef _WIN32
            QueryPerformanceCounter(&litmp);
            QPart1 = litmp.QuadPart;
#else
            clock_gettime(CLOCK_MONOTONIC, &time1);
#endif
#endif

            // GAO FAX LIB
            GAO_PCM_Proc(FaxDataStruct, inBuffer, outBuffer);
            GAO_ModStatus_Upd(FaxDataStruct, &ModemStatus);
            sTemp = ModemStatus.BufferSize;

            GAO_Msg_Proc(FaxDataStruct, &AppGaoMsg);
            Rev = AppRecvMsg(&AppGaoMsg);

            if (Rev == RPT_FAX_STATUS)
            {
#if !APP_FILE_OPERATION

                if (SendFaxDoc == 0) // Rx
                {
                    i = (int)((((AppTiffBuffer[0]) << 24) & 0xff000000) + (((AppTiffBuffer[1]) << 16) & 0x00ff0000) + (((AppTiffBuffer[2]) << 8) & 0x0000ff00) + ((AppTiffBuffer[3]) & 0xff));
                    fwrite(&(AppTiffBuffer[4]), 1, i, fTiffFile);
                    printf("APP: Write tiff buffer into a file.\n");
                }

#endif

                if (fTiffFile != 0)
                {
                    fclose(fTiffFile);
                }

                isEndOfSession = 1;
                break;
            }

#if TIME_MEASURE
#ifdef _WIN32
            QueryPerformanceCounter(&litmp);
            QPart2 = litmp.QuadPart;
            dfMinus = (double)((QPart2 - QPart1) * 4);
            {
                SWORD temp = (SWORD)(dfMinus / bufferSize);

                for (i = 0; i < bufferSize; i ++)
                {
                    fwrite(&temp, sizeof(temp), 1, fTimeMeasure);
                }
            }
#else

            if (0)
            {
                volatile temp;

                for (i = 0; i < 100000; i ++)
                {
                    temp = i;
                }
            }

            clock_gettime(CLOCK_MONOTONIC, &time2);

            diff(time1, time2, &timeTemp);
            {
                SWORD temp = (SWORD)(timeTemp.tv_nsec * 8 / bufferSize);

                for (i = 0; i < bufferSize; i ++)
                {
                    fwrite(&temp, sizeof(temp), 1, fTimeMeasure);
                }
            }
#endif
#endif

            DumpPCMinCount += bufferSize;

            //printf("DumpPCMinCount=%lu\n", DumpPCMinCount);
            for (i = 0; i < bufferSize; i++)
            {
                *pPCMLibOut++ = outBuffer[i];

                if (pPCMLibOut >= &PCMOutBuf[PCM_BUF_LENGTH_MAX])
                {
                    pPCMLibOut = PCMOutBuf;
                }
            }

            nWordsIn -= bufferSize;
            nWordsOut += bufferSize;

            if (sTemp > 0 && (sTemp != bufferSize))
            {
                bufferSize = sTemp;
                printf("bufferSize: %d\n", bufferSize);
            }

#if !defined(_WIN32) && DATA_RECORD
            usleep(1);
#endif
        }

        if (!isHangUp)
        {
            int temp = nWordsOut;

            if (temp > ZX_READ_SAMPLES)
            {
                temp = ZX_READ_SAMPLES;
            }

            for (i = 0; i < temp; i++)
            {
                outBuffer[i] = *pPCMOut++;

                if (pPCMOut >= &PCMOutBuf[PCM_BUF_LENGTH_MAX])
                {
                    pPCMOut = PCMOutBuf;
                }
            }

            //printf("nWordsOut: %d, temp: %d, left: %d\n", nWordsOut, temp, nWordsOut-temp);
            nWordsOut -= temp;
#if DATA_RECORD
            // write data to channel
            nRC = write(nFD, outBuffer, temp << 1);

            if (nRC != (temp << 1))
            {
                printf("  CHANNEL WRITE FAILED [%d] \n", nRC);
                //return 0; // Sometimes it fails, but it's not a fatal failure
            }

            if (nRC != 0)
            {
                fwrite(outBuffer, 1, temp << 1, fdPcmOut);
                fflush(fdPcmOut);
            }

#endif
        }
    }

#if SUPPORT_JPEG_CODEC
    if ((SendFaxDoc == 0) && (IsJpegCompression == 1))
    {
        short ReValue = 1;
        i = 0;        

        if ((fTiffFile = fopen("image.tif", "rb")) == 0)
        {
            printf("image.tif does not exist!\n");
            exit(-8);
        }

        while (ReValue == 1)
        {
            memset(Namebuf, 0, sizeof(Namebuf));
            sprintf(Namebuf, "image%d.bmp", i++);

            if ((fBmpFile = fopen(Namebuf, "wb")) == 0)
            {
                printf("Can not create %s!\n", Namebuf);
                exit(-9);
            }

            ReValue = FileConversionFromTiffToBmp(fTiffFile, fBmpFile);
            fclose(fBmpFile);
        }

        fclose(fTiffFile);
    }
#endif

#if !defined(_WIN32) && DATA_RECORD
    close(nFD);
#endif

    fcloseall();
    return 0;
}

void gprintf(char *str, ...)
{
    va_list ap;

    /* Point to first argument */
    va_start(ap, str);

    vprintf(str, ap);

    va_end(ap);
}
