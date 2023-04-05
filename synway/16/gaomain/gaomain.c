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

#include <string.h>
#include <stdio.h>
#include "common.h"
#include "gaoapi.h"
#include "gaostru.h"
#if SUPPORT_DTE
#include "gaomsg.h"
#include "dsmstru.h"
#include "dsmdef.h"
#include "t4info.h"
#include "tiffstru.h"
#endif
#if SUPPORT_PARSER
#include "circbuff.h"
#endif
#if SUPPORT_T30
#include "t30ext.h"
#endif
#if SUPPORT_VXX_DCE
#include "aceext.h"
#endif
#if SUPPORT_T38
#include "t38ext.h"
#endif

#if SUPPORT_VXX_DCE
extern void MDM_Init(UBYTE *pModemDataMem);
#endif

#if SUPPORT_DTE
extern void DsmSendCtlMsg(GAO_Msg *pMsg, UWORD MsgCode);
extern void DsmMain(TDsmApiStruct *pApi, TDsmStrcut *pDsm);
extern void ImageMain(TImgApiStruct *pApiImg, TImgProDteStruct *pImg);
extern void DsmImgT30InterfaceMain(GAO_LibStruct *pFax);
extern void DsmInit(TDsmStrcut *pDsm);
extern void ImageProcessInit(TImgProDteStruct *pImg);
extern void TiffInit(TTiffStruct *pTiff);
extern void TiffMain(TTiffApiStruct *pTiffApi, TTiffStruct *pTiff);
extern void FaxlibMaintenance(GAO_Msg *pMsg);

// T.31 parser
void DTE_ParseDataToClass1Dce(GAO_LibStruct *pFax);
void DTE_ParseRspOrIndFromClass1Dce(GAO_LibStruct *pFax);
void DTE_ParseDataFromClass1Dce(GAO_LibStruct *pFax);

// T.32 parser
void DTE_ParseRspOrIndFromDce(GAO_LibStruct *pFax);
void DTE_ParseDataFromClass2Dce(GAO_LibStruct *pFax);

void SendImgConversionCommand(TDsmStrcut *pDsm, GAO_Msg *pDsmImg)
{
    TImgFormat *pTif = &(pDsm->TxTifFormat);
    TImgFormat *pDcs = &(pDsm->DcsFormat);

    if (pDsm->SendImgConCmd != 0)
    {
        if (pDsmImg->Direction == MSG_DIR_IDLE)
        {
            DsmSendCtlMsg(pDsmImg, CTL_IMAGE_CONVERSION);
            pDsmImg->Buf[0] = pDsm->Direction;

            if (pDsm->SendImgConCmd == 1)
            {
                pDsmImg->Buf[1] = 1; // need to convert image format

                if (pDsm->Direction == DSM_FAX_TRANSMISSION)
                {
                    memcpy(&(pDsmImg->Buf[2]), pTif, sizeof(TImgFormat)); // format before conversion
                    memcpy(&(pDsmImg->Buf[2 + sizeof(TImgFormat)]), pDcs, sizeof(TImgFormat)); // format after conversion
                }
                else
                {
                    memcpy(&(pDsmImg->Buf[2]), pDcs, sizeof(TImgFormat)); // format before conversion
                    memcpy(&(pDsmImg->Buf[2 + sizeof(TImgFormat)]), pTif, sizeof(TImgFormat)); // format after conversion
                }
            }

#if SUPPORT_TX_PURE_TEXT_FILE
            else if (pDsm->SendImgConCmd == 3)
            {
                pDsmImg->Buf[1] = 2; // tx a text file
                memcpy(&(pDsmImg->Buf[2 + sizeof(TImgFormat)]), pDcs, sizeof(TImgFormat)); // format after conversion
            }

#endif
            else
            {
                pDsmImg->Buf[1] = 0;    // do not need to convert image format
            }

#if (SUPPORT_TX_COVER_PAGE)

            if (pDsm->TxCoverP == 1)
            {
                pDsmImg->Buf[0] = DSM_FAX_TRANSMISSION_WITH_COVERP;

                if (pDsm->SendImgConCmd == 2)
                {
                    memcpy(&(pDsmImg->Buf[2 + sizeof(TImgFormat)]), pDcs, sizeof(TImgFormat));    // format after conversion
                }
            }

#endif
            pDsm->SendImgConCmd = 0;
        }
    }
}

SWORD GAO_Msg_Proc(void *pLibDataMem, GAO_Msg *pMsg)
{
    GAO_LibStruct *pLib = (GAO_LibStruct *)pLibDataMem;
    TDsmStrcut *pDsm = &(pLib->m_Dsm);
    TTiffApiStruct TiffMsg;
    TDsmApiStruct DsmMsg;
    TImgApiStruct ImgMsg;
    TTiffStruct *pTiff = &(pLib->m_Tiff);
    TImgProDteStruct *pImg = &(pLib->m_Img);

    DsmMsg.pTiffDsm = &(pLib->m_TiffDsm);
    DsmMsg.pDsmDce = &(pLib->m_ImgDce);
    DsmMsg.pDsmImg = &(pLib->m_DsmImg);
    TiffMsg.pAppTiff = pMsg;
    TiffMsg.pTiffDsm = &(pLib->m_TiffDsm);
    ImgMsg.pDsmImg = &(pLib->m_DsmImg);
    ImgMsg.pImgDce = &(pLib->m_ImgDce);

    FaxlibMaintenance(pMsg);

    TiffMain(&TiffMsg, pTiff);
    DsmMain(&DsmMsg, pDsm);   // DSM main function
    SendImgConversionCommand(pDsm, &(pLib->m_DsmImg));

    if (pDsm->State == DSM_STATE_IMAGE)
    {
        ImageMain(&ImgMsg, pImg);   // Image main function
    }
    else if (pDsm->State == DSM_STATE_PRI) // voice during PRI
    {
    }

#if SUPPORT_PARSER

    if (pDsm->OperationStage == DTE_OS_DATA)
    {
#if SUPPORT_T31_PARSER

        if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS1)
        {
            DTE_ParseDataFromClass1Dce(pLib);
        }

#endif
#if SUPPORT_T32_PARSER

        if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS2)
        {
            DTE_ParseDataFromClass2Dce(pLib);
        }

#endif
    }

#endif

    DsmImgT30InterfaceMain(pLib); // interface between DSM or IMG and (T32)T30

    if (pDsm->State == DSM_STATE_IMAGE)
    {
        ImageMain(&ImgMsg, pImg);   // Image main function
    }
    else if (pDsm->State == DSM_STATE_PRI) // voice during PRI
    {
    }

    DsmMain(&DsmMsg, pDsm);   // DSM main function
    SendImgConversionCommand(pDsm, &(pLib->m_DsmImg));
    TiffMain(&TiffMsg, pTiff);

#if SUPPORT_PARSER
#if SUPPORT_T31_PARSER

    if (RdReg(pDsm->ServiceClass, FCLASS_MAJ) == FCLASS1)
    {
        if (pDsm->OperationStage != DTE_OS_CONFIG)
        {
            DTE_ParseDataToClass1Dce(pLib);
            T30_Main(&pLib->m_T30);
        }

        if (pDsm->OperationStage != DTE_OS_DATA)
        {
            DTE_ParseRspOrIndFromClass1Dce(pLib);
        }
    }
    else
#endif
        if (pDsm->OperationStage != DTE_OS_DATA)
        {
            DTE_ParseRspOrIndFromDce(pLib);
        }

#endif

    return 0;
}
#endif

SWORD GAO_Lib_Init(void *pLibDataMem, UDWORD udLenBuf)
{
    if (udLenBuf >= sizeof(GAO_LibStruct))
    {
        GAO_LibStruct *pLib = (GAO_LibStruct *)pLibDataMem;
#if SUPPORT_VXX_DCE
        GaoModemStruct *pModem = &(pLib->m_Modem);
#endif

        memset(pLib, 0, udLenBuf);
        TRACE1("GAO: Data structure size is 0x%x bytes.", sizeof(GAO_LibStruct));

#if SUPPORT_DTE
#if SUPPORT_T32
        // moved cfg msgs by app to here, if app does not want to do these cfg
        pLib->dteToT30.isCapToRcv = TRUE;
        pLib->dteToT30.minPhaseCSpd = BR_2400;
#if SUPPORT_V34FAX
        pLib->dteToT30.isV34Enabled = TRUE;
#else
        pLib->dteToT30.isV34Enabled = FALSE;
#endif
#endif

        TiffInit(&(pLib->m_Tiff));
        DsmInit(&(pLib->m_Dsm));
        ImageProcessInit(&(pLib->m_Img));
#endif

#if SUPPORT_PARSER
        CB_InitCircBuffer(&pLib->m_BufferOut, pLib->m_MemoryOut, BUFSIZE);
        CB_InitCircBuffer(&pLib->m_BufferIn, pLib->m_MemoryIn, BUFSIZE);
#endif

#if SUPPORT_VXX_DCE
        MDM_Init((UBYTE *)pModem);
#if SUPPORT_FAX
        pModem->m_Ace.pT30ToDce = &(pLib->t30ToDce);
        pModem->m_Ace.pDceToT30 = &(pLib->dceToT30);
#endif
#if SUPPORT_T32_PARSER
        pModem->m_Ace.pT30ToDte = &(pLib->t30ToDte);
        pModem->m_Ace.pDteToT30 = &(pLib->dteToT30);
#endif
#if SUPPORT_PARSER
        pModem->m_Ace.pCBIn = &pLib->m_BufferIn;
        pModem->m_Ace.pCBOut = &pLib->m_BufferOut;
#endif
        ACEInit((UBYTE **)pModem);
#endif

#if SUPPORT_T30
        pLib->m_T30.pT30ToDte = &(pLib->t30ToDte);
        pLib->m_T30.pT30ToDce = &(pLib->t30ToDce);
        pLib->m_T30.pDteToT30 = &(pLib->dteToT30);
        pLib->m_T30.pDceToT30 = &(pLib->dceToT30);
        T30_Init(&pLib->m_T30); // must be after setting of default fax capabilities
#endif
        return VERSION_NUMBER;
    }
    else
    {
        TRACE2("GAO: ERROR. Buffer size %u. Need %u", udLenBuf, sizeof(GAO_LibStruct));
        return -1;
    }
}

#if SUPPORT_PARSER
/* for Customer to check how many bytes data modem can receive from UART */
/* i.e., empty space of DteWr buffer */
SWORD GAO_InBuf_Chk(void *pLibDataMem)
{
    CircBuffer *pCBIn = &((GAO_LibStruct *)pLibDataMem)->m_BufferIn;

    return (SWORD)GetAvailLenInCB(pCBIn);
}

/* for Customer to check how many bytes data in modem is ready to send out */
/* i.e., non-empty space of DteRd buffer */
/* This function is for convenience only. It is not necessary and currently not called. */
SWORD GAO_OutBuf_Chk(void *pLibDataMem)
{
    CircBuffer *pCBOut = &((GAO_LibStruct *)pLibDataMem)->m_BufferOut;

    return (SWORD)(pCBOut->CurrLen);
}

#define DEBUG_OU (0)
#define DEBUG_IN (0)

#if DEBUG_OU || DEBUG_IN
FILE *fGaoMain = NULL;
#endif

/* transmit the data from remote equipment */
SWORD GAO_Buf_In(void *pLibDataMem, UBYTE *pBuf, UWORD uLenIn)
{
    CircBuffer *pCBIn = &((GAO_LibStruct *)pLibDataMem)->m_BufferIn;

    if (uLenIn > GetAvailLenInCB(pCBIn))
    {
        TRACE0("API: ERROR. Buffer overflow");
        return -1;
    }
    else if (uLenIn > 0)
    {
#if DEBUG_IN
        UWORD i;

        TRACE("%s", "Fr Remote: ");

        for (i = 0; i < uLenIn; i++)
        {
            TRACE("%02x ", pBuf[i]);
        }

        TRACE0("");

        if (fGaoMain == NULL)
        {
#if SUPPORT_DTE
            fGaoMain = fopen("dtein.dat", "wb");
#else
            fGaoMain = fopen("dcein.dat", "wb");
#endif
        }

        fwrite(pBuf, 1, uLenIn, fGaoMain);
#endif

        PutStringToCB(pCBIn, pBuf, uLenIn);
    }

    return 0;
}

/* Transmit the data to remote equipment */
/* return accutally how many bytes data received */
SWORD GAO_Buf_Out(void *pLibDataMem, UBYTE *pBuf, UWORD uLenBuf)
{
    CircBuffer *pCBOut = &((GAO_LibStruct *)pLibDataMem)->m_BufferOut;

    if (uLenBuf > pCBOut->CurrLen)
    {
        uLenBuf = pCBOut->CurrLen;
    }

    if (uLenBuf > 0)
    {
        GetStringFromCB(pCBOut, pBuf, uLenBuf);

#if DEBUG_OU
        {
            UWORD i;

            TRACE("%s", "To Remote: ");

            for (i = 0; i < uLenBuf; i++)
            {
                TRACE("%02x ", pBuf[i]);
            }

            TRACE0("");
        }

        if (!fGaoMain)
        {
#if SUPPORT_DTE
            fGaoMain = fopen("dteout.dat", "wb");
#else
            fGaoMain = fopen("dceout.dat", "wb");
#endif
        }

        fwrite(pBuf, 1, uLenBuf, fGaoMain);
#endif
    }

    return uLenBuf;
}
#endif

#if SUPPORT_T38
SWORD GAO_IP_Dec(void *pLibDataMem, UBYTE *pIPBuf, UWORD uLenIn) // returns the SUCCESS or FAIL
{
    GAO_LibStruct *pLib = (GAO_LibStruct *)pLibDataMem;
    T38Struct *pT38 = &pLib->m_T38;

    return T38_Decode(pT38, pIPBuf, uLenIn);
}

SWORD GAO_IP_Enc(void *pLibDataMem, UBYTE *pIPBuf, UWORD uLenBuf) // returns the length of encoded IP packet in *pIPPacket
{
    UBYTE isPacketReady = FALSE;
    GAO_LibStruct *pLib = (GAO_LibStruct *)pLibDataMem;
    T38Struct *pT38 = &pLib->m_T38;
    T38EncoderStru *pT38Enc = &pT38->t38_encoder;
    CircBuffer *pT38CBRd = &pT38->T38_TxCB;

    T38_Main(pT38);
    T30_Main(&pLib->m_T30);

    if (pT38->timer_counter_10ms == 0)
    {
        if (pT38CBRd->CurrLen || pT38->t38_encoder.ifp_len)
        {
            isPacketReady = TRUE;
            pT38->isDuplicate = FALSE;
        }
        else if (pT38->t38_protocol == T38_UDP && pT38->t38_encoder.per_data.size && pT38->isDuplicate < UDP_RESEND_TIMES) // fixing issue 15
        {
            isPacketReady = TRUE;
            pT38->isDuplicate++;
        }
    }
    else
    {
#if T38_TRACE
        TRACE0("T38: TX Ticking...");
#endif
        pT38->timer_counter_10ms--;
    }

#if SUPPORT_T38EP

    if (pT38Enc->isData)
#endif
    {
        pT38Enc->txCapability += pT38Enc->bitsPerTick;
    }

#if T38_JITTER_BUFFER
    pT38->CycleCounter++;
    pT38->CycleCounter &= 0xffff;
#endif

    if (isPacketReady)
    {
        return T38_Encode(pT38, pIPBuf);
    }

    return 0;
}
#endif




