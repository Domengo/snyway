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
#include <string.h>
#include "main.h"
#include "gaomsg.h"
#include "gstdio.h"

extern FILE *fTiffFile;
extern UBYTE AppTiffBuffer[];
extern DTE_InitStruct InitParms;
extern UBYTE IsJpegCompression;
extern void AppSendMsg(GAO_Msg *pMsg, UBYTE MsgType, UWORD MsgCode);

const char *BitRateApp[14] =
{
    "2400bps", "4800bps", "7200bps", "9600bps", "12000bps", "14400bps", "16800bps", "19200bps", "21600bps", "24000bps", "26400bps", "28800bps", "31200bps", "33600bps"

};

const char *ModulationModeApp[4] =
{
    "V.29", "V.27ter", "V.17", "V.34"
};



UWORD AppRecvMsg(GAO_Msg *pMsg) // process report messages
{
    UWORD Rev = 0;
    static UBYTE isHangingUp = 0;

    if (pMsg->Direction == MSG_UP_LINK)
    {
        if (pMsg->MsgType == MSG_TYPE_REPORT)
        {
            Rev = pMsg->MsgCode;

            switch (pMsg->MsgCode)
            {
                case RPT_FAX_STATUS:

                    switch (pMsg->Buf[0])
                    {
                        case 1:

                            if (isHangingUp)
                            {
                                isHangingUp = 0;
                            }
                            else
                            {
                                Rev = 0;
                            }

                            break;
                        case 5:
                            Rev = 0;
                            break;
                        case 2:
                            Rev = 0;
                            TRACE0("APP: Entering fax data stage.");
                            break;
                        case 7:
                            TRACE0("APP: No dial tone!");
                            break;
                        case 8:
                            TRACE0("APP: Remote busy!");
                            break;
                        case 9:
                            TRACE0("APP: No Answer!");
                            break;
                        case 3:
                            Rev = 0;
                            TRACE0("APP: Ringing");
                            break;
                    }

                    break;
                case RPT_HANGUP_CODE:
                    TRACE0("APP: Got FHS.");
                    isHangingUp = 1;
                    break;
                case RPT_TIFF_FILE_OPERATION_MODE:

                    if (pMsg->Buf[0] == 1)
                    {
                        TRACE0("APP: Start Tx Tiff file.");
                    }
                    else if (pMsg->Buf[0] == 3)
                    {
                        TRACE0("APP: Start Rx Tiff file.");

                        if ((fTiffFile = fopen("image.tif", "wb")) == 0)
                        {
                            TRACE0("APP: Cannot create image.tif!\n");
                            return RPT_FAX_STATUS;
                        }

#if APP_FILE_OPERATION
                        //if(FileOperation == 1)
                        {
                            pMsg->Buf[0] = 1; // file pointer
                            pMsg->Buf[1] = (UBYTE)(((unsigned)(fTiffFile) >> 24) & 0xff);
                            pMsg->Buf[2] = (UBYTE)(((unsigned)(fTiffFile) >> 16) & 0xff);
                            pMsg->Buf[3] = (UBYTE)(((unsigned)(fTiffFile) >> 8) & 0xff);
                            pMsg->Buf[4] = (UBYTE)(((unsigned)(fTiffFile)) & 0xff);
                        }
#else
                        //else
                        {
                            pMsg->Buf[0] = 0; // buffer pointer
                            pMsg->Buf[1] = (UBYTE)(((unsigned)(&(AppTiffBuffer[0])) >> 24) & 0xff);
                            pMsg->Buf[2] = (UBYTE)(((unsigned)(&(AppTiffBuffer[0])) >> 16) & 0xff);
                            pMsg->Buf[3] = (UBYTE)(((unsigned)(&(AppTiffBuffer[0])) >> 8) & 0xff);
                            pMsg->Buf[4] = (UBYTE)(((unsigned)(&(AppTiffBuffer[0]))) & 0xff);
                        }
#endif
                        AppSendMsg(pMsg, MSG_TYPE_CONFIG, CFG_TIFF_FILE_PTR_RX);
                    }

                    break;
                case RPT_NEXT_PAGE:

                    if (pMsg->Buf[0] == 1)
                    {
                        TRACE0("APP: Finished current page and start next page.");
                    }
                    else if (pMsg->Buf[0] == 3)
                    {
                        TRACE0("APP: Finished last page.");
                    }

                    break;
                case RPT_NEGOTIATED_PARAS:

                    if (pMsg->Buf[3] == 8)
                    {
                        IsJpegCompression = 1;
                    }
                    else
                    {
                        IsJpegCompression = 0;
                    }

                    break;
            }

            if (pMsg->Direction == MSG_UP_LINK)
            {
                if (pMsg->MsgCode < TEST_FCLASS || pMsg->MsgCode > TEST_FBS)
                {
                    memset(pMsg, 0, sizeof(GAO_Msg));
                }
            }
        }
        else
        {
            memset(pMsg, 0, sizeof(GAO_Msg));
        }
    }

    return Rev;
}
