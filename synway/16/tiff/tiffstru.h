/*****************************************************************************
*             Copyright (c) 1995 - 2007 by GAO Research Inc.                 *
*                          All rights reserved.                              *
*                                                                            *
*  This program is a confidential and an unpublished work of GAO Research    *
*  Inc. that is protected under international and Canadian copyright laws.   *
*  This program is a trade secret and constitutes valuable property of GAO   *
*  Research Inc. All use, reproduction and/or disclosure in whole or in      *
*  part of this program is strictly prohibited unless expressly authorized   *
*  in writing by GAO Research Inc.                                           *
*****************************************************************************/
#ifndef _TIFF_STRUCT_H_
#define _TIFF_STRUCT_H_

#include <stdio.h>
#include "commtype.h"
#include "tiffdef.h"
#include "t4info.h"
#include "tiffrw.h"
#include "gaomsg.h"
#if (SUPPORT_TX_COVER_PAGE)
#include "coverpage.h"
#endif

#if SUPPORT_JPEG_CODEC
typedef struct __xyz_struct
{
    double x;
    double y;
    double z;
} Txyz;

typedef struct __blue_green_red_struct
{
    UBYTE blue;
    UBYTE green;
    UBYTE red;
} TBgr;
#endif

typedef struct
{
    GAO_Msg *pAppTiff;
    GAO_Msg *pTiffDsm;
} TTiffApiStruct;



typedef struct
{
#if SUPPORT_FILE_OPERATION
    FILE *fTiffFileTxPoll[MAX_NUM_TX_TIFF_FILES_POLL];
    FILE *fTiffFileTx[MAX_NUM_TX_TIFF_FILES];
    FILE *fTiffFileRx;
    FILE *fTiffFileTxCurr;
#endif
    UBYTE *pTiffFileBuf;
    UDWORD TxFileTotalLength;// in byte
    SDWORD Offset;
#if (SUPPORT_TX_COVER_PAGE)
    TCoverpageStruct CoverPage;
#endif
    UBYTE FilePointer;
    UBYTE State;
    UBYTE Direction;
    UBYTE LastTiffFileTx;
    UBYTE RemoteHasDocTx; // remote has doc to send after sending ATD command

    // format
    UWORD XRes;
    UWORD YRes;
    UWORD PaperSize;
    TiffStruct TiffCtrl;
    T4InfoStruct pageInfo;
    int pageByteCnt;
    int padbytes;
//#if SUPPORT_FILE_OPERATION
    UBYTE TiffData[1024*1024];
//#endif
    //report
    SWORD TxFileCheckResults[MAX_NUM_TX_TIFF_FILES]; // 1: correct, 0: reported results already, < 0: error code of tiff file
    SWORD ErrorCode;
    UBYTE ResultCode;

    UWORD RxLineNumber;
    UWORD MsgCode;
    UBYTE PostMsgResp;
    UBYTE PostMsgCmd;
    UBYTE TiffFileCmd;
    UBYTE RptPMCTx;
    UBYTE TxFileType; // .tif, .txt
    UBYTE EcmOn;
} TTiffStruct;

#endif
