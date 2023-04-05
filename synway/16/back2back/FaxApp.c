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

#include "main.h"

#define MAX_TIFF_FILE_LENGTH                (1024*1024)
#define FAX_DATA_STRUCT_SIZE                (1328444)
#define PCM_BUF_SIZE_INIT                   (80)
#define PCM_BUF_LENGTH_MAX                  (0x1000)

#define ZX_READ_DEFAULT                     (80)
#define ZX_READ_SAMPLES                     (96)
#define ZX_READ_BYTES                       (ZX_READ_SAMPLES << 1)

extern void AppSendMsg(GAO_Msg *pMsg, UBYTE MsgType, UWORD MsgCode);
extern UWORD AppRecvMsg(GAO_Msg *pMsg);

GAO_ModStatus ModemStatusA;
int FaxDataStructA[FAX_DATA_STRUCT_SIZE/4 + 1];
GAO_Msg AppGaoMsgA;
FILE *fTiffFileA;

UBYTE AppTiffBufferA[MAX_TIFF_FILE_LENGTH + 4 + 10];

GAO_ModStatus ModemStatusB;
int FaxDataStructB[FAX_DATA_STRUCT_SIZE/4 + 1];
GAO_Msg AppGaoMsgB;
FILE *fTiffFileB;
UBYTE AppTiffBufferB[MAX_TIFF_FILE_LENGTH + 4 + 10];

UDWORD DumpPCMinCountA, DumpPCMinCount;

int main()
{
    FILE *fdPCMA2B = fopen("pin_r.dat", "wb");
    FILE *fdPCMB2A = fopen("pin_t.dat", "wb");
    SWORD PCMA2B[PCM_BUF_LENGTH_MAX];
    SWORD PCMB2A[PCM_BUF_LENGTH_MAX];
    char PhoneNumber[20] = "1234";
    int i = 0;
    SWORD inBuffer[ZX_READ_SAMPLES], outBuffer[ZX_READ_SAMPLES + ZX_READ_DEFAULT];
    UBYTE isEndOfSessionA = 0, isEndOfSessionB = 0;
    SWORD bufferSizeA = ZX_READ_DEFAULT, bufferSizeB = ZX_READ_DEFAULT;
    int nSamplesA2B = 1000, nSamplesB2A = 1000;
    SWORD *pPCMA2BRead = PCMA2B, *pPCMA2BWrite = &PCMA2B[nSamplesA2B];
    SWORD *pPCMB2ARead = PCMB2A, *pPCMB2AWrite = &PCMB2A[nSamplesB2A];

    DumpPCMinCountA = 0L, DumpPCMinCount = 0L;

    memset(&PCMA2B, 0, sizeof(PCMA2B));
    memset(&PCMB2A, 0, sizeof(PCMB2A));
    memset(inBuffer, 0, sizeof(inBuffer));
    memset(outBuffer, 0, sizeof(outBuffer));
    memset(&AppGaoMsgA, 0, sizeof(GAO_Msg));
    memset(&AppGaoMsgB, 0, sizeof(GAO_Msg));
    memset(AppTiffBufferA, 0, sizeof(AppTiffBufferA));
    memset(AppTiffBufferB, 0, sizeof(AppTiffBufferB));

    // A send, B receive
    printf("APP: Initializing system.\n");

    if (GAO_Lib_Init(FaxDataStructA, sizeof(FaxDataStructA)) < 0)
    {
        exit(-1);
    }

    if (GAO_Lib_Init(FaxDataStructB, sizeof(FaxDataStructB)) < 0)
    {
        exit(-1);
    }

    GAO_ModStatus_Init(&ModemStatusA);
    GAO_ModStatus_Init(&ModemStatusB);

    AppSendMsg(&AppGaoMsgA, MSG_TYPE_MAINTENANCE, MTN_REQUEST_FAXLIB_VERSION);

    while (AppGaoMsgA.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStructA, &AppGaoMsgA);
    }

    if ((AppGaoMsgA.MsgType == MSG_TYPE_MAINTENANCE) && (AppGaoMsgA.MsgCode == MTN_RESP_FAXLIB_VERSION))
    {
        printf("Gao Library Version is: %d.%d\n", AppGaoMsgA.Buf[0], AppGaoMsgA.Buf[1]);
    }
    else
    {
        printf("Wrong response to request version number!\n");
    }

    memset(&AppGaoMsgA, 0, sizeof(GAO_Msg));

    AppSendMsg(&AppGaoMsgB, MSG_TYPE_MAINTENANCE, MTN_REQUEST_FAXLIB_VERSION);

    while (AppGaoMsgB.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStructB, &AppGaoMsgB);
    }

    if ((AppGaoMsgB.MsgType == MSG_TYPE_MAINTENANCE) && (AppGaoMsgB.MsgCode == MTN_RESP_FAXLIB_VERSION))
    {
        printf("Gao Library Version is: %d.%d\n", AppGaoMsgB.Buf[0], AppGaoMsgB.Buf[1]);
    }
    else
    {
        printf("Wrong response to request version number!\n");
    }

    memset(&AppGaoMsgB, 0, sizeof(GAO_Msg));

    memcpy(AppGaoMsgA.Buf, "11111111", 8);

    AppSendMsg(&AppGaoMsgA, MSG_TYPE_CONFIG, CFG_LOCAL_ID);

    while (AppGaoMsgA.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStructA, &AppGaoMsgA);
    }

    memset(&AppGaoMsgA, 0, sizeof(GAO_Msg));

    memcpy(AppGaoMsgB.Buf, "22222222", 8);

    AppSendMsg(&AppGaoMsgB, MSG_TYPE_CONFIG, CFG_LOCAL_ID);

    while (AppGaoMsgB.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStructB, &AppGaoMsgB);
    }

    memset(&AppGaoMsgB, 0, sizeof(GAO_Msg));

    // A part
    if ((fTiffFileA = fopen("imageA.tif", "rb")) == 0)
    {
        printf("image.tif does not exist!\n");
        exit(-1);
    }

    i = fread(&(AppTiffBufferA[4]), 1, MAX_TIFF_FILE_LENGTH + 5, fTiffFileA);

    if (i > MAX_TIFF_FILE_LENGTH)
    {
        printf("tiff file too big!\n");
        exit(-1);
    }

    AppTiffBufferA[0] = (i >> 24) & 0xff;
    AppTiffBufferA[1] = (i >> 16) & 0xff;
    AppTiffBufferA[2] = (i >>  8) & 0xff;
    AppTiffBufferA[3] = (i >>  0) & 0xff;

    AppGaoMsgA.Buf[0] = 0; // buffer pointer
    AppGaoMsgA.Buf[1] = 1; // 1 Tx tiff file
    AppGaoMsgA.Buf[2] = (UBYTE)(((unsigned)(&(AppTiffBufferA[0])) >> 24) & 0xff);
    AppGaoMsgA.Buf[3] = (UBYTE)(((unsigned)(&(AppTiffBufferA[0])) >> 16) & 0xff);
    AppGaoMsgA.Buf[4] = (UBYTE)(((unsigned)(&(AppTiffBufferA[0])) >>  8) & 0xff);
    AppGaoMsgA.Buf[5] = (UBYTE)(((unsigned)(&(AppTiffBufferA[0]))) & 0xff);

    AppSendMsg(&AppGaoMsgA, MSG_TYPE_CONFIG, CFG_TIFF_FILE_PTR_TX);

    while (AppGaoMsgA.Direction != MSG_UP_LINK)
    {
        GAO_Msg_Proc(FaxDataStructA, &AppGaoMsgA);
    }

    if ((AppGaoMsgA.MsgType == MSG_TYPE_REPORT) && (AppGaoMsgA.MsgCode == RPT_VALIDITY_OF_TX_TIFF_FILE) && (AppGaoMsgA.Buf[1] == 0))
    {
        printf("Check Tx Tiff File is Passed.\n");
    }
    else
    {
        printf("check Tx Tiff file failed!, error code is: %d\n", AppGaoMsgA.Buf[1]);
        exit(-2);
    }

    memset(&AppGaoMsgA, 0, sizeof(GAO_Msg));

    AppSendMsg(&AppGaoMsgA, MSG_TYPE_CONTROL, CTL_FAX_CALL_ORG);
    AppGaoMsgA.Buf[0] = 2; // tone dialling mode
    AppGaoMsgA.Buf[1] = strlen(PhoneNumber); // length of phone number
    memcpy(&(AppGaoMsgA.Buf[2]), PhoneNumber, AppGaoMsgA.Buf[1]);
    printf("APP: SEND ATD COMMAND.\n");

    // B part
    AppGaoMsgB.Buf[0] = 1;
    AppSendMsg(&AppGaoMsgB, MSG_TYPE_CONFIG, CFG_RING_NUMBER_FOR_AUTO_ANS);
    ModemStatusB.RingDetected = 1;

    while (!(isEndOfSessionA && isEndOfSessionB))
    {
        // A send
        while (nSamplesB2A >= bufferSizeA)
        {
            SWORD sTemp;
            UWORD Rev;

            for (i = 0; i < bufferSizeA; i++)
            {
                if (pPCMB2ARead == pPCMB2AWrite)
                {
                    TRACE0("ERROR 1");
                }

                inBuffer[i] = *pPCMB2ARead++;

                if (pPCMB2ARead >= &PCMB2A[PCM_BUF_LENGTH_MAX])
                {
                    pPCMB2ARead = PCMB2A;
                }
            }

            fwrite(inBuffer, sizeof(SWORD), bufferSizeA, fdPCMB2A);
            fflush(fdPCMB2A);

            // GAO FAX LIB
            GAO_PCM_Proc(FaxDataStructA, inBuffer, outBuffer);
            GAO_ModStatus_Upd(FaxDataStructA, &ModemStatusA);
            sTemp = ModemStatusA.BufferSize;

            GAO_Msg_Proc(FaxDataStructA, &AppGaoMsgA);
            Rev = AppRecvMsg(&AppGaoMsgA);

            if (Rev == RPT_FAX_STATUS)
            {
                if (fTiffFileA != 0)
                {
                    fclose(fTiffFileA);
                }

                isEndOfSessionA = 1;
                TRACE0("End of A");
                break;
            }

            DumpPCMinCountA += bufferSizeA;

            //printf("DumpPCMinCountA=%lu\n", DumpPCMinCountA);
            for (i = 0; i < bufferSizeA; i++)
            {
                *pPCMA2BWrite++ = outBuffer[i];

                if (pPCMA2BWrite >= &PCMA2B[PCM_BUF_LENGTH_MAX])
                {
                    pPCMA2BWrite = PCMA2B;
                }

                if (pPCMA2BRead == pPCMA2BWrite)
                {
                    TRACE0("ERROR 2");
                }
            }

            nSamplesB2A -= bufferSizeA;
            nSamplesA2B += bufferSizeA;

            if (sTemp > 0 && (sTemp != bufferSizeA))
            {
                bufferSizeA = sTemp;
                printf("bufferSizeA: %d\n", bufferSizeA);
            }
        }

        // B receive
        while (nSamplesA2B >= bufferSizeB)
        {
            SWORD sTemp;
            UWORD Rev;

            //TRACE("%s", "In: ")
            for (i = 0; i < bufferSizeB; i++)
            {
                if (pPCMA2BRead == pPCMA2BWrite)
                {
                    TRACE0("ERROR 3");
                }

                inBuffer[i] = *pPCMA2BRead++;

                //TRACE("%02x ", inBuffer[i]);
                if (pPCMA2BRead >= &PCMA2B[PCM_BUF_LENGTH_MAX])
                {
                    pPCMA2BRead = PCMA2B;
                }
            }

            //TRACE0("");

            fwrite(inBuffer, sizeof(SWORD), bufferSizeB, fdPCMA2B);
            fflush(fdPCMA2B);

            // GAO FAX LIB
            GAO_PCM_Proc(FaxDataStructB, inBuffer, outBuffer);
            GAO_ModStatus_Upd(FaxDataStructB, &ModemStatusB);
            sTemp = ModemStatusB.BufferSize;

            GAO_Msg_Proc(FaxDataStructB, &AppGaoMsgB);
            Rev = AppRecvMsg(&AppGaoMsgB);

            if (Rev == RPT_FAX_STATUS)
            {
                if (fTiffFileB != 0)
                {
                    printf("APP: Write tiff buffer into a file.\n");
                    i = (int)((((AppTiffBufferB[0]) << 24) & 0xff000000) +
                              (((AppTiffBufferB[1]) << 16) & 0x00ff0000) +
                              (((AppTiffBufferB[2]) <<  8) & 0x0000ff00) +
                              ((AppTiffBufferB[3]) & 0xff));

                    fwrite(&(AppTiffBufferB[4]), 1, i, fTiffFileB);

                    fclose(fTiffFileB);
                }

                isEndOfSessionB = 1;
                TRACE0("End of B");
                break;
            }

            DumpPCMinCount += bufferSizeB;

            //printf("DumpPCMinCount=%lu\n", DumpPCMinCount);
            for (i = 0; i < bufferSizeB; i++)
            {
                *pPCMB2AWrite++ = outBuffer[i];

                if (pPCMB2AWrite >= &PCMB2A[PCM_BUF_LENGTH_MAX])
                {
                    pPCMB2AWrite = PCMB2A;
                }

                if (pPCMB2ARead == pPCMB2AWrite)
                {
                    TRACE0("ERROR 4");
                }
            }

            nSamplesA2B -= bufferSizeB;
            nSamplesB2A += bufferSizeB;

            if (sTemp > 0 && (sTemp != bufferSizeB))
            {
                bufferSizeB = sTemp;
                printf("bufferSize: %d\n", bufferSizeB);
            }
        }
    }

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
