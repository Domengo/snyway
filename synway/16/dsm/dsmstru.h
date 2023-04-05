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


#ifndef _DSM_STRUCT_H_DTE_
#define _DSM_STRUCT_H_DTE_

#include "commtype.h"
#include "gaomsg.h"


typedef struct
{
    GAO_Msg *pTiffDsm;
    GAO_Msg *pDsmDce;
    GAO_Msg *pDsmImg;
} TDsmApiStruct;



typedef struct
{
    UBYTE PaperSize;
    UBYTE XRes;
    UBYTE YRes;
    UBYTE Compression;
    UBYTE FillOrder;
    UBYTE ScanTimePerLine;
    UBYTE BitRate;
} TImgFormat;



typedef struct
{
    UBYTE ServiceClass;
    UBYTE CallType; //FAX_CALL_ORG or FAX_CALL_ANS
    UBYTE Direction; //DSM_FAX_TRANSMISSION or DSM_FAX_RECEIVING
    UBYTE State;
    UBYTE RemoteHasDocTx; // remote has doc to send after sending ATD command
    UBYTE RxPolledDocCapa;
#if (SUPPORT_TX_COVER_PAGE)
    UBYTE TxCoverP;
#endif
    UBYTE DocNumForPoll; // local has a doc to send after rx a doc
    UBYTE DocNumForTx;   // local has a doc to send when sending ATD command

    UBYTE PostMsgCmd;
    UBYTE RespCode;
    UBYTE GotImgFormat; //
    UBYTE RxTiffPtrReady;
    UBYTE ReTxCapaNoECM;
    UBYTE NumOfTiffFile;
    UBYTE RxNextDoc;
    UBYTE RingNumber;
    UBYTE RingCount;
    UBYTE SendATA;

    UBYTE LocalRequestPRI;
    UBYTE RemoteRequestPRI;
    UBYTE PRI_Voice; // it is 1 during the PRI, +FVO
    UBYTE ActionAfterPRI;
    UBYTE ResetDte;

    UBYTE TiffFileMode;//DSM_TX_TIFF_FILE,DSM_TX_TIFF_FILE_BY_POLL,DSM_RX_TIFF_FILE,DSM_RX_TIFF_FILE_BY_POLL
    UBYTE RptTiffFileMode;//DSM_TX_TIFF_FILE,DSM_TX_TIFF_FILE_BY_POLL,DSM_RX_TIFF_FILE,DSM_RX_TIFF_FILE_BY_POLL
    UWORD ActionCmd;    // CTL_TRANSMIT_PAGE,CTL_RECEIVE_PAGE,CTL_AT_FKS
    UWORD FcsFtcRpt;   //RPT_NEGOTIATED_PARAS,RPT_FTC_OF_REMOTE_SIDE

    TImgFormat TxTifFormat; // tif file format when tx
    TImgFormat DcsFormat;    // negotiated parameters
    UBYTE SendImgConCmd; // 1: send image conversion cmd to IMG
    UWORD RespTimer;

#if SUPPORT_PARSER
    UBYTE OperationStage;
    UBYTE PrevByte;
    UWORD CurrCnfgCmd;
    UBYTE CurrCnfgParm;
#if SUPPORT_T31_PARSER
    UBYTE CurrT30DCECmd;
    UBYTE WasDleEtx;
#endif
#endif
    UBYTE DceRxImageQualityCheckCapa; // 0: no, 1: can check, 2: can correct
    UBYTE DceTxImageQualityCheckCapa; // 0: no, 1: can check, 2: can correct
} TDsmStrcut;




#endif


