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
#include "t30ext.h"
#include "common.h"
#include "hdlcext.h"

#if SUPPORT_T30 && SUPPORT_ECM_MODE

static void T30_RestoreLeftover_ECM(T4ECMstruc *pT4)
{
    UWORD i;

    for (i = 0; i < LEFTOVER_BUFSIZE; i++)
    {
        pT4->pblock[i] = pT4->leftover_buf[i];
    }

    pT4->udImageDataByteNum = pT4->leftover_len;

    T30_T4InitLeftoverBuf(pT4);
}

static void T30_T4MakeFrameStart_ECM(HdlcStruct *pHDLC_TX, UBYTE fcf)
{
    HDLC_ResetWB(pHDLC_TX);

    pHDLC_TX->WorkingBuff[0] = 0xFF;
    pHDLC_TX->WorkingBuff[1] = HDLC_CONTR_FIELD_NOTLASTFRM;
    pHDLC_TX->WorkingBuff[2] = fcf;
}

#if !SUPPORT_T38EP
static void T30_T4MakeFrameEnd_ECM(HdlcStruct *pHDLC_TX)
{
    pHDLC_TX->CRC = 0xFFFF;
    pHDLC_TX->OnesCount = 0;

    HDLC_UpdateCRC16(&pHDLC_TX->CRC, pHDLC_TX->WorkingBuff, (UWORD)(pHDLC_TX->WorkingBuffEnd >> 3));

    HDLC_AppendCRC(pHDLC_TX);

    HDLC_Encode(pHDLC_TX);

    HDLC_AppendFlags(pHDLC_TX, 2);
}
#endif

/*
** In this part we poll T32 for image data. Input DataTypes can be:
**    - T4DATA          : data is avaialable : get the data and return suitable return value
**    - T4PAGE_END_FLAG : data ends : proceed to next phase when data sent
**    - T30MESNULL      : enquiry check : return 64k BLOCK status
*/
static void T30_GetImageData_ECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    //DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    //T30ToDceInterface *pT30ToDce = &pT30->t30ToDce;
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UWORD    len, i, uDiff, uLen;
    UDWORD    eof;

    if (pT30ToDte->dataCmd == DC3_XOFF)
    {
        return;
    }

    pT30ToDte->dataCmd = DC1_XON;

    uLen = pDteToT30->tempDataLen;

    if (uLen != 0)
    {
        pDteToT30->tempDataLen = 0;

        if (uLen <= (BLOCKSIZE - pT4->udImageDataByteNum))
        {
            /* transfer all data to BLOCK buf */
            memcpy(&pT4->pblock[pT4->udImageDataByteNum], pDteToT30->tempData, uLen);
            pT4->udImageDataByteNum += uLen;
        }
        else if (pT4->udImageDataByteNum < BLOCKSIZE)
        {
            /* part of data to the end of BLOCK buf, the rest to leftover buf */
            len = (UWORD)(BLOCKSIZE - pT4->udImageDataByteNum);

            memcpy(&pT4->pblock[pT4->udImageDataByteNum], pDteToT30->tempData, len);
            pT4->udImageDataByteNum += len;

            pT4->leftover_len = uLen - len;
            memcpy(pT4->leftover_buf, &pDteToT30->tempData[len], pT4->leftover_len);

            pT30ToDte->dataCmd = DC3_XOFF;
            TRACE0("T30: BUFFER FULL returned to T32-3");
        }
        else
        {
            /* no transfer, BLOCK buf is full */
            pT4->leftover_len = uLen;
            memcpy(pT4->leftover_buf, pDteToT30->tempData, pT4->leftover_len);

            pT30ToDte->dataCmd = DC3_XOFF;
            TRACE0("T30: BUFFER FULL returned to T32-4");
        }
    }
    else if (pDteToT30->postPgMsg != POST_PG_MSG_NULL)
    {
        switch (pDteToT30->postPgMsg)
        {
            case POST_PG_MSG_EOP:
                pT4->FCF2 = FCF2_EOP;
                break;
            case POST_PG_MSG_MPS:
                pT4->FCF2 = FCF2_MPS;
                break;
            case POST_PG_MSG_EOM:
                pT4->FCF2 = FCF2_EOM;
                pT30->wasEOM = TRUE;
                break;
            default:
                TRACE0("T30: ERROR. pDteToT30->postPgMsg");
                T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT);
                break;
        }

        TRACE1("T30: PPS_what received from DTE=%x", pT4->FCF2);
        pDteToT30->postPgMsg = POST_PG_MSG_NULL;

        /* end of data by DTE */
        /* in the end of a BLOCK and PAGEENDs */
        if ((pT4->udImageDataByteNum == BLOCKSIZE) && pT4->leftover_len)  /* RTC is split? */
        {
            T30_RestoreLeftover_ECM(pT4);
        }

        /* zero fill to the end of frame */
        eof = ((pT4->udImageDataByteNum + (FRAMESIZE - 1)) >> FRAMESIZE_BIT_NUM) << FRAMESIZE_BIT_NUM;

        uDiff = (UWORD)(eof - pT4->udImageDataByteNum);

        for (i = 0; i < uDiff; i++)
        {
            pT4->pblock[pT4->udImageDataByteNum ++] = 0;
        }

        /* send necessary frames only */
        for (i = (UWORD)(pT4->udImageDataByteNum >> FRAMESIZE_BIT_NUM); i < BLOCKNUM; i++) // fixing issue 157
        {
            ResetBit(pT4->status[i>>3], 7 - (i & 0x7));
        }

        pT4->frmNumInBlock = (UBYTE)((eof >> FRAMESIZE_BIT_NUM) - 1);
    }
}

static void T30_touchT4Struc_ECM(T30Struct *pT30)
{
    T4ECMstruc *pT4 = &pT30->T4ECM;
    HdlcStruct *pHDLC_TX = &pT30->T30HDLC;
    UWORD    i;

    /* sending a BLOCK for the first time or retransmitting the last BLOCK again? */
    if (pT4->PPR_count == 0)/* the first time */
    {
        /* check if there is any leftover from previous BLOCK */
        /* at the beginning of a new BLOCK */
        pT4->udImageDataByteNum = 0;

        /* any leftover from previous BLOCK? */
        if (pT4->leftover_len)
        {
            T30_RestoreLeftover_ECM(pT4);
        }

        for (i = 0; i < BLOCKNUM_BYTESIZE; i++)
        {
            pT4->status[i] = 0xFF;    /* send all 256 bits frames */
        }

        pT4->FCF1 = T4_PPS;                    /* send PPS command in PhaseD */
        pT4->FCF2 = FCF2_INIT;

        pT4->frmNumInBlock = BLOCKNUM - 1;    /* 256 frames */

        pT30->pT30ToDte->dataCmd = DC1_XON;
        T30_GetImageData_ECM(pT30);/* check if T32 already has something */
    }

    pT4->isRcpExchanged = FALSE;
    pT4->frmNumInPP = 0;

    HDLC_Init(pHDLC_TX);

#if !SUPPORT_T38EP
    /* put 20 ms FLAG in RawData buffer */
    HDLC_AppendFlags(pHDLC_TX, (UBYTE)(2 * pT30->TCFByteNum));
#endif
}

static UBYTE T30_SendImageData_ToModem_ECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif
    T4ECMstruc *pT4 = &pT30->T4ECM;
    HdlcStruct *pHDLC_TX = &pT30->T30HDLC;
    UBYTE byte;
    UWORD i, j, frame_available = FALSE;
    UBYTE doNotEnc;

#if SUPPORT_T38EP
    doNotEnc = TRUE;
#elif SUPPORT_V34FAX
    doNotEnc = pDceToT30->isV34Selected;
#else
    doNotEnc = FALSE;
#endif

    if (pDceToT30->flowCntlCmd == DC3_XOFF || pT30ToDce->tempDataLen != 0)
    {
        return FALSE;
    }

    /* check RawData buffer */
    if ((pHDLC_TX->RawDataEnd >> 3) < T30_THRESHOLD || doNotEnc)
    {
        /* fill RawData buffer */
        for (i = 0; i < (pT4->udImageDataByteNum >> FRAMESIZE_BIT_NUM); i++)
        {
            if (ReadBit(pT4->status[i>>3], 7 - (i & 0x7)) == 0)
            {
                continue;
            }

            TRACE1("T30: Making frame number %d", i);
            pT4->frmNumInPP++;

            frame_available = TRUE;

            T30_T4MakeFrameStart_ECM(pHDLC_TX, T4_FCD);
            /* reverse all FIF including frame_number and image_data */
            pHDLC_TX->WorkingBuff[3] = COMM_BitReversal((UBYTE)i);

            if (RdReg(pDteToT30->dataBitOrder, BIT0) != 0) // fixing issue 137
            {
                // Reversed bit order for Phase C
                memcpy(&pHDLC_TX->WorkingBuff[4], &pT4->pblock[i << FRAMESIZE_BIT_NUM], FRAMESIZE);
            }
            else
            {
                // Direct bit order for Phase C
                for (j = 0; j < FRAMESIZE; j++)
                {
                    pHDLC_TX->WorkingBuff[4 + j] = COMM_BitReversal(pT4->pblock[(i << FRAMESIZE_BIT_NUM) + j]);
                }
            }

            pHDLC_TX->WorkingBuffEnd = ((4 + FRAMESIZE) << 3) | (pHDLC_TX->WorkingBuffEnd & 0x07);

            if (doNotEnc)
            {
                // T38 requires all bit stuffing, CRC and flags be removed
                UWORD uLen = pHDLC_TX->WorkingBuffEnd >> 3;

                memcpy(pT30ToDce->tempData, pHDLC_TX->WorkingBuff, uLen);
                pT30ToDce->tempDataLen = uLen;
                pT30ToDce->tempDataType = T30_DCE_HDLC_OK;
            }

#if !SUPPORT_T38EP
            else
            {
                T30_T4MakeFrameEnd_ECM(pHDLC_TX);
            }

#endif

            ResetBit(pT4->status[i>>3], 7 - (i & 0x7));

            break;
        }

        if (!frame_available && !pT4->isRcpExchanged)
        {
            /* whole block examined? */
            if (i == (pT4->frmNumInBlock + 1))/* YES, either T4PAGE_END_FLAG or whole BLOCK, append RCP */
            {
                if (doNotEnc)
                {
                    T30_T4MakeFrameStart_ECM(pHDLC_TX, T4_RCP);
                    pHDLC_TX->WorkingBuffEnd = (3 << 3) | (pHDLC_TX->WorkingBuffEnd & 0x07);
                    memcpy(pT30ToDce->tempData, pHDLC_TX->WorkingBuff, 3);
                    pT30ToDce->tempDataLen = 3;

                    if (++pT30->rxErrorTimes == 3) // fixing issue 149
                    {
                        pT30->rxErrorTimes = 0;
                        pT30ToDce->tempDataType = T30_DCE_HDLC_END;
                        pT4->isRcpExchanged = TRUE;
                    }
                    else
                    {
                        pT30ToDce->tempDataType = T30_DCE_HDLC_OK;
                    }
                }

#if !SUPPORT_T38EP
                else
                {
                    for (j = 0; j < 3; j ++)
                    {
                        T30_T4MakeFrameStart_ECM(pHDLC_TX, T4_RCP);
                        pHDLC_TX->WorkingBuffEnd = (3 << 3) | (pHDLC_TX->WorkingBuffEnd & 0x07);
                        T30_T4MakeFrameEnd_ECM(pHDLC_TX);
                    }

                    pT4->isRcpExchanged = TRUE;
                }

#endif
            }
            else /* NO, maybe DTE is slow in sending data */
            {
                frame_available = TRUE;
            }

#if !SUPPORT_T38EP

            if (!doNotEnc)
            {
                /* append 20 ms FLAGs after RCP, or in the gaps of DTE no data */
                HDLC_AppendFlags(pHDLC_TX, (UBYTE)(2 * pT30->TCFByteNum));/* 20 ms */
            }

#endif
        }
    }

    if (!doNotEnc)
    {
        UWORD rdLen = pHDLC_TX->RawDataEnd >> 3, txLen;

        if (pT4->isRcpExchanged)
        {
            // send everything in the raw data buffer
            txLen = rdLen + ((pHDLC_TX->RawDataEnd & 7) ? 1 : 0);
        }
        else if (rdLen == 0)
        {
            txLen = 0;
        }
        else
        {
            txLen = rdLen - 1;
        }

#if SUPPORT_T31_PARSER
        byte = (UBYTE)MIN16(txLen, 3 * pT30->TCFByteNum);
#else
        byte = (UBYTE)MIN16(txLen, pT30->TCFByteNum);
#endif

        if (byte)
        {
            /* MODEM will reverse the data, so reverse it here to make it direct. */
            //TRACE("%s", "T30 Tx:");
            for (i = 0; i <= byte; i++)
            {
                pT30ToDce->tempData[i] = COMM_BitReversal(pHDLC_TX->RawData[i]);
                //TRACE("%02x ", pT30ToDce->tempData[i]);
            }

            //TRACE0("");

            pT30ToDce->tempDataLen = byte;

            if (pT4->isRcpExchanged && txLen == byte)
            {
                pT30ToDce->tempDataType = T30_DCE_HS_END;
                pHDLC_TX->RawDataEnd = 0;
            }
            else
            {
                pT30ToDce->tempDataType = T30_DCE_HS_DATA;

                /* adjust Rawdata buffer; shift left */
                for (i = 0; i <= ((pHDLC_TX->RawDataEnd >> 3) - byte); i++)
                {
                    pHDLC_TX->RawData[i] = pHDLC_TX->RawData[i + byte];
                }

                for (j = i; j < HDLCBUFLEN; j++)
                {
                    pHDLC_TX->RawData[j] = 0;
                }

                pHDLC_TX->RawDataEnd -= byte << 3;
            }
        }
    }

    if ((!frame_available) &&
        ((pDceToT30->resultCodeRsp == RESULT_CODE_OK)
#if SUPPORT_V34FAX
         || (isV34Selected && pT30ToDce->tempDataType == T30_DCE_HDLC_END)
#endif
        ))
    {
        pDceToT30->resultCodeRsp = RESULT_CODE_NULL;

        return TRUE;
    }

    return FALSE;
}

static UBYTE T30_CommandTx_ECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UWORD len = 0;

    // fixing issue 180
    if (pDteToT30->isDiscCmd)
    {
        T30_GotoPointC(pT30, HANGUP_ABORTED_FROM_FKS_OR_CAN);
        return 2;
    }

    if (!pT30->isHdlcMade)
    {
        pT30->HDLC_TX_Buf[0] = 0xff;
        pT30->HDLC_TX_Buf[1] = HDLC_CONTR_FIELD_LASTFRM;
        pT30->HDLC_TX_Buf[2] = (UBYTE)(HDLC_CorrT30FCF(pT30->wasDisRcved, pT4->FCF1 << 8) >> 8);

        switch (pT4->FCF1)
        {
            case T4_PPS :
                TRACE1("T30: Sending PPS_%x command", pT4->FCF2);
                pT30->HDLC_TX_Buf[3] = pT4->FCF2;
                pT30->HDLC_TX_Buf[4] = COMM_BitReversal(pT4->pgNum);
                pT30->HDLC_TX_Buf[5] = COMM_BitReversal(pT4->blkNum);
                pT30->HDLC_TX_Buf[6] = pT4->frmNumInPP == 0 ? 0 : COMM_BitReversal((UBYTE)(pT4->frmNumInPP - 1));

                len = 7;
                break;
            case T4_EOR :
                TRACE1("T30: Sending EOR_%x command", pT4->FCF2);
                pT30->HDLC_TX_Buf[3] = pT4->FCF2;

                len = 4;
                break;
            case T4_CTC :
                TRACE0("T30: Sending CTC command");

                if (T30_MakeDCS_CTC(pT30, TRUE))
                {
                    len = 5;
                }
                else
                {
                    // fixing issue 181
                    return 2;
                }

                break;
            case T4_RR :
                TRACE0("T30: Sending RR command");

                len = 3;
                break;
        }
        pT30->isHdlcMade = TRUE;    /* len is still zero, which prevents multi frame making */
#if SUPPORT_T38EP
        T30_LogHdlc(pT30->HDLC_TX_Buf, len, TRUE, TRUE);
#else
        T30_LogHdlc(pT30->HDLC_TX_Buf, len, TRUE, FALSE);
#endif
    }

#if SUPPORT_V34FAX
    else if (isV34Selected)
    {
        return 1;
    }

#endif
    else if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
    {
        pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
        return 1;
    }
    else
    {
        return 2;
    }

    pT30ToDce->tempDataLen = (UBYTE)len;

    if (len != 0)
    {
        memcpy(pT30ToDce->tempData, pT30->HDLC_TX_Buf, len);
        pT30ToDce->tempDataType = T30_DCE_HDLC_OK;
    }

    return 2;
}

/* [4]
** T30_PhaseC_ImageDataTx in ECM mode: (MODEM speed HIGH:T4)
** This phase can be entered from :
**    T30_PhaseB_GetConf (previous phase) to start a new transmission or from
**  T30_PhaseC_GetConf (next 2 phase) either to continue to next BLOCK (PPS_NUL)
**                                     or to continue to next page  (PPS_MPS)
**                                     or to retransmit a bad page  (PPR case)
** This phase advances to T30_PhaseD_SendPE (next phase) to send a PPS command;
** PPS_NUL in case a BLOCK has been transmitted and T4PAGE_END_FLAG has NOT come yet, or
** PPS_MPS or PPS_EOP if T4PAGE_END_FLAG has been received from T32.
*/
void T30_PhaseC_ImageDataTx_ECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UBYTE BlockFinish_Flag;
    UBYTE doNotEnc, isV34Selected;

#if SUPPORT_T38EP
    doNotEnc = TRUE;
#elif SUPPORT_V34FAX
    doNotEnc = pDceToT30->isV34Selected;
#else
    doNotEnc = FALSE;
#endif

#if SUPPORT_V34FAX
    isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_C_XMIT); return;
    }
    else
#endif
        if (pDteToT30->actionCmd != DTE_T30_FDT)
        {
            return;
        }

    if (pDteToT30->isDiscCmd)
    {
        pT30ToDce->tempDataLen = 0;
        pT30ToDce->tempDataType = T30_DCE_HS_END;
        T30_GotoPointC(pT30, HANGUP_ABORTED_FROM_FKS_OR_CAN); return;
    }

    switch (pT30->txStage)
    {
        case HS_TX_STAGE_SILENCE:
#if SUPPORT_V34FAX

            if (isV34Selected)
            {
                T30_SetCommand(pT30ToDce, T30_DCE_DLE_PRI);
                pT30->txStage = HS_TX_STAGE_SYNC;
            }
            else
#endif
                if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
                {
                    UBYTE isLongTraining = FALSE;

                    pT30->txStage = HS_TX_STAGE_SYNC;
                    pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                    T30_SetCommand(pT30ToDce, T30_DCE_FTM);

                    if (pT4->FCF1 == T4_CTC) // fixing issue 110
                    {
                        isLongTraining = TRUE;
                        pT4->FCF1 = T4_PPS;
                    }

                    pT30ToDce->tempParm[0] = T30_ConvertSpdToT31Mod(pT30->T30Speed, isLongTraining);
                }

            break;
        case HS_TX_STAGE_SYNC:

            if ((pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
#if SUPPORT_V34FAX
                || (pDceToT30->selectedV34Chnl == V34_PRIMARY_CHNL)
#endif
               )
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = HS_TX_STAGE_DATA;

                if (pT4->FCF2 != FCF2_NULL && pT4->PPR_count == 0) // fixing issue 158
                {
                    T30_SetResponse(pT30, RESULT_CODE_CONNECT);
                }

#if SUPPORT_V34FAX

                if (isV34Selected)
                {
                    pT30ToDte->faxParmStatus.bitRateBR = pDceToT30->priChnlBitRate - 1;
                }

#endif
                pT30->TCFByteNum = T30_BytesPer10ms[pT30ToDte->faxParmStatus.bitRateBR];

                pT30->TCFResult  = 18;    /* 180 ms flag, another 20 ms will follow */
                /* do some initialization on T4Struc */
                T30_touchT4Struc_ECM(pT30); // 20 ms flags

                TRACE0("T30: Start to send Fax image with ECM");
            }

            break;
        default:

            if (pT4->PPR_count)
            {
                /* last BLOCK not transmitted yet? */
                //TRACE0("Buffer Full returned to T32-0");
            }
            else
            {
                /* every time check T32 for image data */
                T30_GetImageData_ECM(pT30);
            }

            /* During the first 18 times send FLAGs, and during this time accumulate */
            /* the data from T32, and send them down the line after sending the FLAGs */
            if (pT30->TCFResult)
            {

                if (doNotEnc)
                {
                    pT30->TCFResult--; // T.38 doesn't send out the flags
                }
                else if (!pT30ToDce->tempDataLen)
                {
                    pT30->TCFResult--;
                    memset(pT30ToDce->tempData, 0x7E, 45);
                    pT30ToDce->tempDataLen = (UBYTE)(pT30->TCFByteNum);
                    pT30ToDce->tempDataType = T30_DCE_HS_DATA;
                }
            }
            else
            {
                /* send image data down the line until the BLOCK finishes */
                BlockFinish_Flag = T30_SendImageData_ToModem_ECM(pT30);

                if (BlockFinish_Flag == 1)
                {
#if SUPPORT_V34FAX

                    if (isV34Selected)
                    {
                        if ((pT4->PPR_count & 0x1) != 0x0)
                        {
                            pT30ToDce->initV34PrimRateMax = pDceToT30->priChnlBitRate - 1;
                            pT30ToDce->isReqRateReduction = TRUE; TRACE1("T30: Command <DLE><P%d><DLE><PPH>", pT30ToDce->initV34PrimRateMax * 24);
                        }
                        else
                        {
                            pT30ToDce->isReqRateReduction = FALSE;
                        }
                    }
                    else
#endif
                    {
                        T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                        pT30ToDce->tempParm[0] = SILENCE_MOD_CHG;
                    }

                    pT30->pfT30Vec = T30_PhaseD_SendPE_ECM; pT30->txStage = LS_TX_STAGE_SILENCE;

                    TRACE0("T30: PhaseC. MODEM finished sending the BLOCK");

                    if (pT4->FCF2 == FCF2_INIT) // if FCF2 hasn't been updated at the end of a block, it should be a partial page. Send PPS_NULL
                    {
                        // init
                        pT4->FCF2 = FCF2_NULL; // fixing issue 158
                    }
                }
            }
    }
}

/* [5]
** T30_PhaseD_SendPE in ECM mode: (MODEM speed LOW:T30)
** This phase can be entered from :
**        T30_PhaseC_ImageDataTx  (previous phase) to send a PPS command; or from
**      T30_PhaseC_GetConf (next phase)     either to send a EOR command
**                                          or to send a CTC command
**                                          or to send a RR  command
** This phase advances to T30_PhaseC_GetConf (next phase) to get the response.
*/
void T30_PhaseD_SendPE_ECM(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    T4ECMstruc *pT4 = &pT30->T4ECM;
    UWORD re = 0;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT); return;
    }

#endif

    switch (pT30->txStage)
    {
        case LS_TX_STAGE_SILENCE:
#if SUPPORT_V34FAX

            if (isV34Selected)
            {
                T30_SetCommand(pT30ToDce, T30_DCE_DLE_CTRL);
                pT30->txStage = LS_TX_STAGE_DATA;
            }
            else
#endif
                if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
                {
                    pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                    pT30->txStage = LS_TX_STAGE_PREAMBLE;
                }

            break;
        case LS_TX_STAGE_PREAMBLE:
            T30_SetCommand(pT30ToDce, T30_DCE_FTH);
            pT30ToDce->tempParm[0] = MOD_V21_300;
            pT30->txStage = LS_TX_STAGE_DATA;
            break;
        case LS_TX_STAGE_DATA:

            if ((pDceToT30->resultCodeRsp == RESULT_CODE_CONNECT)
#if SUPPORT_V34FAX
                || (pDceToT30->selectedV34Chnl == V34_CONTROL_CHNL)
#endif
               )
            {
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30->txStage = LS_TX_STAGE_CRC_OK;
            }

            break;
        default:
            /* send the command to the remote and wait until sent */
            re = T30_CommandTx_ECM(pT30);

            if (re == 1)
            {
                /* all sent? */
                T30_StopHDLCSending(pT30);
                pT30->pfT30Vec = T30_PhaseC_GetConf_ECM;
                TRACE0("T30: PhaseD. MODEM finished sending the COMMAND");

                pT30->hdlcTypeTx = ((UWORD)pT4->FCF2) << 8;
            }
    }
}

/* [6]
** T30_PhaseC_GetConf in ECM mode: (MODEM speed LOW:T30)
** This phase can be entered from :
**      T30_PhaseD_SendPE (previous phase) to recive a response :
**                    either PPR to retransmit the already sent BLOCK again
**                      or RNR to do flow control
**                      or MCF for confirmation to a PPS command
**                      or ERR for confirmation to a EOR command
**                        (CTR  case not handled now)
** This phase advances to T30_PhaseC_ImageDataTx (previous 2 phase) to repeat the BLOCK (PPR)
**                                               or to send next BLOCK/PAGE (MCF,ERR)
**                or to T30_PhaseD_SendPE (previous phase) to send a command (EOR,RR)
**                or to T30_PhaseE_Call (next phase) to disconnect.
*/
void T30_PhaseC_GetConf_ECM(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    //T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    T4ECMstruc *pT4      = &pT30->T4ECM;
    UBYTE hdlcRxStatus;
    int i;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT); return;
    }

#endif

    /* get the response from remote */
    hdlcRxStatus = T30_GetHDLCfromRemote(pT30, FALSE);

    switch (hdlcRxStatus)
    {
        case HDLC_3S_TIMEOUT:
            T30_GotoPointC(pT30, HANGUP_RSPREC_ERROR_PHASE_D_XMIT);
            break;
        case HDLC_RX_ERROR:

            if (pT30->rxErrorTimes < T30_MAX_REPEAT_TIMES)
            {
                if (pT4->RNR_flag == 1)
                {
                    pT4->FCF1 = T4_RR;
                }

                // fixing issue 67
                pT30->pfT30Vec = T30_PhaseD_SendPE_ECM;
#if SUPPORT_V34FAX
                pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                pT30->rxErrorTimes++;
            }
            else
            {
                UBYTE hangupCode;
                pT30->rxErrorTimes = 0;

                switch (pT4->FCF2)
                {
                    case FCF2_NULL:
                    case FCF2_MPS:
                        hangupCode = HANGUP_NO_RSP_TO_MPS_3_TIMES;
                        break;
                    case FCF2_EOP:
                        hangupCode = HANGUP_NO_RSP_TO_EOP_3_TIMES;
                        break;
                    case FCF2_EOM:
                        hangupCode = HANGUP_NO_RSP_TO_EOM_3_TIMES;
                        break;
                    default:
                        hangupCode = HANGUP_RSPREC_ERROR_PHASE_D_XMIT;
                }

                T30_GotoPointC(pT30, hangupCode);
                TRACE0("T30: Failed to receive response three times, disconnect the line. 1");
            }

            break;
        case HDLC_LASTFRM_RXED:

            //fixing issue 110
            if (pT4->FCF1 == T4_CTC)
            {
                if (pT30->hdlcTypeRx == HDLC_TYPE_T4_CTR)
                {
#if SUPPORT_V34FAX

                    if (!isV34Selected)
#endif
                    {
                        T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                        pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 5; // fixing issue 187
                    }

                    pT30->pfT30Vec = T30_PhaseC_ImageDataTx_ECM; pT30->txStage = HS_TX_STAGE_SILENCE;
                }
                else
                {
                    T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT);
                }

                pT4->RNR_flag = 0;
            }
            else
            {
                switch (pT30->hdlcTypeRx)
                {
                    case HDLC_TYPE_T4_PPR:
                    {
                        if (pT4->RNR_flag == 1)
                        {
                            T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT);
                            break;
                        }

                        pT4->PPR_count++;
                        TRACE1("T30: PPR received, %d time(s)", pT4->PPR_count);

                        for (i = 0; i < BLOCKNUM_BYTESIZE; i++)
                        {
                            pT4->status[i] = pT30->HDLC_RX_Buf[3 + i];
                            TRACE("%02x ", pT4->status[i]);
                        }

                        TRACE0(".");

                        /* discard unnecessary bits in response */
                        for (i = (UWORD)(pT4->udImageDataByteNum >> FRAMESIZE_BIT_NUM); i < BLOCKNUM; i++) // fixing issue 157
                        {
                            ResetBit(pT4->status[i>>3], 7 - (i & 0x7));
                        }

                        for (i = 0; i < BLOCKNUM_BYTESIZE; i++)
                        {
                            TRACE("%02x ", pT4->status[i]);
                        }

                        TRACE0(".");

#if SUPPORT_V34FAX

                        if (isV34Selected && pDteToT30->minPhaseCSpd > pDceToT30->priChnlBitRate)
                        {
                            pT30->wasLastTry = TRUE;
                        }

#endif

                        // 1, 2, or 3, not V34: send image
                        // 4, not V34, not last try: CTC
                        // 4, not V34, last try: EOR
                        // any, V34, not last try: send image
                        // any, V34, last try: EOR
                        if (((((pT4->PPR_count & 0x3) == 0x0) && (pT4->retryCount >= pDteToT30->ecmRetryCnt))
#if SUPPORT_V34FAX
                             || isV34Selected
#endif
                            ) && pT30->wasLastTry)
                        {
                            // fixing issue 110
                            /* end of retransmission */
                            pT4->FCF1 = T4_EOR;
                            pT30->pfT30Vec = T30_PhaseD_SendPE_ECM;
#if SUPPORT_V34FAX
                            pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                            pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                        }
#if SUPPORT_V34FAX
                        else if (isV34Selected)
                        {
                            pT4->FCF1 = T4_PPS;

                            ////fixing issue 173, 
                            //if ((pT4->PPR_count & 0x3) == 0x3)
                            ////if (pT4->PPR_count == 0x1) // LLL debug
                            //{
                            //    T30_SetCommand(pT30ToDce, T30_DCE_DLE_RTNC);
                            //    pT30ToDce->initV34PrimRateMax = pDceToT30->priChnlBitRate - 1;
                            //    pT30->pfT30Vec = T30_PhaseD_SendPE_ECM; pT30->txStage = LS_TX_STAGE_DATA;
                            //    pT4->PPR_count--; // LLL debug
                            //}
                            //else
                            {
                                /* repeat the bad frames in the BLOCK and when finished send */
                                /* the same PPS command as the previous time (as of FCF2:MPS or EOP) */
                                pT30->pfT30Vec = T30_PhaseC_ImageDataTx_ECM; pT30->txStage = HS_TX_STAGE_SILENCE;
                            }
                        }
#endif
                        else if ((pT4->PPR_count & 0x3) != 0x0)
                        {
                            /* repeat the bad frames in the BLOCK and when finished send */
                            /* the same PPS command as the previous time (as of FCF2:MPS or EOP) */
                            pT4->FCF1 = T4_PPS;
                            T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                            pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 5; // fixing issue 187
                            pT30->pfT30Vec = T30_PhaseC_ImageDataTx_ECM; pT30->txStage = HS_TX_STAGE_SILENCE;
                        }
                        else
                        {
                            pT4->FCF1 = T4_CTC;
                            pT30->pfT30Vec = T30_PhaseD_SendPE_ECM; pT30->txStage = LS_TX_STAGE_PREAMBLE;
                        }

                        pT4->RNR_flag = 0;
                    }
                    break;
                    case HDLC_TYPE_T4_RNR:

                        /* if for the first time, activate the timer */
                        if (pT4->RNR_flag == 0)
                        {
                            pT4->RNR_flag = 1;
                            pT30->Timer_T5_Counter = 1; TRACE0("T30: T5 starts");
                        }

                        /* and send RR command */
                        pT4->FCF1 = T4_RR;

                        if (pT4->RNR_flag && (pT30->Timer_T5_Counter > TIMER_T5_LENGTH))
                        {
                            /* if timed out, disconnect */
                            pT30->Timer_T5_Counter = 0; TRACE0("T30: T5 elapsed. Send DCN and disconnet the line.");
                            pT4->RNR_flag = 0;
                            T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT);
                        }
                        else
                        {
                            pT30->pfT30Vec = T30_PhaseD_SendPE_ECM;
#if SUPPORT_V34FAX
                            pT30->txStage = isV34Selected ? LS_TX_STAGE_DATA : LS_TX_STAGE_PREAMBLE;
#else
                            pT30->txStage = LS_TX_STAGE_PREAMBLE;
#endif
                        }

                        break;
                    case HDLC_TYPE_MCF:
                    case HDLC_TYPE_T4_ERR:

                        if (pT4->RNR_flag == 1)
                        {
                            pT30->Timer_T5_Counter = 0; TRACE0("T30: T5 reset");

                            pT4->RNR_flag = 0;
                        }

                        pT4->PPR_count = 0;/* PPR count reset. */

                        if (pT30->wasLastTry && pT30->hdlcTypeRx == HDLC_TYPE_T4_ERR)
                        {
                            // cannot further downgrade the speed because it's already 2400 or FMS speed
                            T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_D_XMIT);
                        }
                        else if (pT4->FCF2 == FCF2_EOM)
                        {
                            T30_InitForPhaseBReEntry(pT30);
                            pT30->pfT30Vec = T30_PhaseA_Call;
                        }
                        /* based on what this MCF/ERR is received for, change phase */
                        else if (pT4->FCF2 == FCF2_EOP)
                        {
                            T30_GotoPointC(pT30, HANGUP_NORMAL);
                        }
                        else if ((pT4->FCF2 == FCF2_NULL) || (pT4->FCF2 == FCF2_MPS))
                        {
#if SUPPORT_V34FAX

                            if (!isV34Selected)
#endif
                            {
                                T30_SetCommand(pT30ToDce, T30_DCE_FTS);
                                pT30ToDce->tempParm[0] = SILENCE_MOD_CHG + 5; // fixing issue 187
                            }

                            pT30->pfT30Vec = T30_PhaseC_ImageDataTx_ECM; pT30->txStage = HS_TX_STAGE_SILENCE;
                        }

                        /* update T4Struc counters */
                        if (pT4->FCF2 == FCF2_NULL)
                        {
                            pT4->blkNum++;
                        }
                        else
                        {
                            if (pT4->FCF2 != FCF2_EOP)
                            {
                                T30_SetResponse(pT30, RESULT_CODE_OK);
                            }

                            pT4->pgNum ++;
                            pT4->blkNum = 0;
                        }

                        break;
                    case HDLC_TYPE_DCN:
                        TRACE0("T30: DCN instead of response-to-PE is received");
                        break;
                    default:
                    {
                        UBYTE hangupCode;

                        switch (pT4->FCF2)
                        {
                            case FCF2_NULL:
                            case FCF2_MPS:
                                hangupCode = HANGUP_INVALID_RSP_TO_MPS;
                                break;
                            case FCF2_EOP:
                                hangupCode = HANGUP_INVALID_RSP_TO_EOP;
                                break;
                            case FCF2_EOM:
                                hangupCode = HANGUP_INVALID_RSP_TO_EOM;
                                break;
                            default:
                                hangupCode = HANGUP_UNSPECIFIED_PHASE_D_XMIT;
                        }

                        TRACE1("T30: ERROR. Invalid response 0x%x", pT30->hdlcTypeRx);
                        pT4->PPR_count = 0;
                        T30_GotoPointC(pT30, hangupCode);
                        break;
                    }
                }
            }

            T30_StopHDLCReceiving(pT30);
            break;
    }
}

#endif
