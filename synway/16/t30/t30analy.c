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

#if SUPPORT_T30
CONST UBYTE    ST_Table_for_DIS[8] = {5, 7, 3,    2, 1,    6,    4, 0};
//CONST UBYTE    ST_Table_for_DCS[8] = {5, 7, 3, 0xFF, 1, 0xFF, 0xFF, 0};

CONST UBYTE    PageLenTable[4] = {0, 2, 1, 0xFF};
//CONST UBYTE    PageWidTable[4] = {0, 2, 1, 0xFF};

// 1 - compatible receiver
// 0 - not compatible receiver
UBYTE T30_AnalysisDIS_DTC(T30Struct *pT30, UBYTE isDIS)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30SessionSubParm *pRemote = &pT30ToDte->remoteSettings;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pT30->pDceToT30->isV34Selected;
#endif
    UBYTE ch, ret = TRUE;
    SBYTE length;

#if SUPPORT_T38EP
    length = pT30->HDLCLen - 3; // fixing issue 139
#else
    length = pT30->HDLCLen - 5; // fixing issue 139
#endif

    memset(pRemote, 0, sizeof(T30SessionSubParm));

    ch = pT30->HDLC_RX_Buf[3];

    if (ch & BIT5)
    {
        TRACE0(" - Real-time Internet fax (ITU-T T.38)");
    }

    if (ch & BIT2)
    {
        TRACE0(" - V.8 capable");
    }

    /* 2nd fcf byte */
    ch = pT30->HDLC_RX_Buf[4];

    if ((ch & BIT7) && pDteToT30->isRequestingToPoll && pDteToT30->isCapToRcv)/* polling bit is 1 */
    {
        pT30ToDte->isRmtDocToPoll = TRUE; TRACE0("T30: +FPO");
        TRACE0(" - Ready to transmit a facsimile document (polling)");
    }
    else
    {
        pT30ToDte->isRmtDocToPoll = FALSE;
    }

    if (ch & BIT6)
    {
        pT30->isRemoteComptRec = TRUE;
        TRACE0(" - Receiver fax operation");
    }
    else
    {
        pT30->isRemoteComptRec = FALSE;
        ret = FALSE;
    }

    /* bit 11-14 */
    //pT30->RemoteCap    = ch & 0x3C;
    //RemoteCap    = ch & 0x3C;

#if SUPPORT_V34FAX

    if (isV34Selected)
    {
        pRemote->bitRateBR = pDceToT30->priChnlBitRate - 1;
    }
    else
#endif
    {
        //switch (pT30->RemoteCap)
        switch (ch & 0x3C)
        {
            case DIS_V27F:
                pRemote->bitRateBR = BR_2400;
                TRACE0(" - ITU-T V.27 ter fall-back mode");
                break;
            case DIS_V27:
                pRemote->bitRateBR = BR_4800;
                TRACE0(" - ITU-T V.27 ter");
                break;
            case DIS_V29:
                pRemote->bitRateBR = BR_9600;
                TRACE0(" - ITU-T V.29");
                break;
            case DIS_V29F:
            case DIS_V33F:
                pRemote->bitRateBR = BR_9600;
                TRACE0(" - ITU-T V.27 ter and V.29");
                break;
            case DIS_V17F:
                pRemote->bitRateBR = BR_14400;
                TRACE0(" - ITU-T V.27 ter, V.29, and V.17");
                break;
            default:
                pT30->isRemoteComptRec = FALSE;
                TRACE0("T30: ERROR. Invalid data signalling rate");
                ret = FALSE;
                break;
        }
    }

    if (pRemote->bitRateBR < pDteToT30->minPhaseCSpd)
    {
        pT30->isRemoteComptRec = FALSE;
        TRACE0("T30: ERROR. Data signalling rate is too low");
        ret = FALSE;
    }

    pRemote->verticalResolutionVR = VR_200_100 | VR_R8_385;
    TRACE0(" - R8 x 3.85 lines/mm and/or 200 x 100 pels/25.4 mm");

    if (ch & BIT1)
    {
        pRemote->verticalResolutionVR |= VR_200_200 | VR_R8_77;
        TRACE0(" - R8 x 7.7 lines/mm and/or 200 x 200 pels/25.4 mm");
    }

    //    TwoDim = ch & BIT0;
    if (ch & BIT0)
    {
        pRemote->dataFormatDF = DF_MR;    // Supports MR
        TRACE0(" - Two dimensional coding capability (MR)");
    }

    /* 3rd fcf byte */
    ch = pT30->HDLC_RX_Buf[5];

    /* bit 17-18 */
    switch (pRemote->pageWidthWD = PageLenTable[(ch >> 6) & 0x3])
    {
        case WD_0: TRACE0(" - Scan line length 215 mm +/- 1%%"); break;
        case WD_1: TRACE0(" - Scan line length 215 mm +/- 1%%, and 255 mm +/- 1%%"); break;
        default: // fixing issue 143
            TRACE0(" - Scan line length 215 mm +/- 1%%, 255 mm +/- 1%%, and 303 mm +/- 1%%"); break;
    }

    /* bit 19-20 */
    switch (pRemote->pageLengthLN = PageLenTable[(ch >> 4) & 0x3])
    {
        case LN_A4: TRACE0(" - Recording length A4 (297 mm)"); break;
        case LN_B4: TRACE0(" - Recording length A4 (297 mm) and B4 (364 mm)"); break;
        case LN_UNLIMITED: TRACE0(" - Recording length Unlimited"); break;
    }

    /* bit 21-23 */
    switch (pRemote->scanTimeST = ST_Table_for_DIS[(ch >> 1) & 0x7])
    {
        case ST_0: TRACE0(" - Min. scan line time 00 ms at 3.85 l/mm: T7.7 = T3.85"); break;
        case ST_1: TRACE0(" - Min. scan line time 05 ms at 3.85 l/mm: T7.7 = T3.85"); break;
        case ST_2: TRACE0(" - Min. scan line time 10 ms at 3.85 l/mm: T7.7 = 1/2 T3.85"); break;
        case ST_3: TRACE0(" - Min. scan line time 10 ms at 3.85 l/mm: T7.7 = T3.85"); break;
        case ST_4: TRACE0(" - Min. scan line time 20 ms at 3.85 l/mm: T7.7 = 1/2 T3.85"); break;
        case ST_5: TRACE0(" - Min. scan line time 20 ms at 3.85 l/mm: T7.7 = T3.85"); break;
        case ST_6: TRACE0(" - Min. scan line time 40 ms at 3.85 l/mm: T7.7 = 1/2 T3.85"); break;
        case ST_7: TRACE0(" - Min. scan line time 40 ms at 3.85 l/mm: T7.7 = T3.85"); break;
    }

    /* extended bit */
    if (length < 4 || !RdReg(ch, BIT0))
    {
        TRACE0(" - Error correction mode OFF");

        if (pRemote->dataFormatDF == DF_MH)
        {
            TRACE0(" - One dimensional coding capability (MH)");
        }
    }
    else
    {
        /* 4th fcf byte */
        ch = pT30->HDLC_RX_Buf[6];

        /* bit 27 */

        pRemote->errorCorrectionEC = (ch >> 5) & BIT0;

        if ((pRemote->errorCorrectionEC > EC_NON_ECM) && (pT30->pDteToT30->faxParmCapability.errorCorrectionEC > EC_NON_ECM))
        {
            TRACE0(" - Error correction mode ON");

            if (RdReg(pT30->HDLC_RX_Buf[3], BIT1))
            {
                TRACE0(" - 64 octets preferred");
                //pT30->T4ECM.frmSizeBitNum = 6;
            }
            else
            {
                TRACE0(" - 256 octets preferred");
                //pT30->T4ECM.frmSizeBitNum = 8;
            }
        }
        else
        {
            TRACE0(" - Error correction mode OFF");
        }

        //    Uncompressed_Mode = (ch >> 1) & BIT0;
        if (ch & BIT6)
        {
            pRemote->dataFormatDF = DF_2D_UNCOMPRESSED;    // Supports Uncompressed Mode
            TRACE0(" - Uncompressed mode");
        }

        /* bit 31 */
        //pT30->T6Cap &= (ch >> 1) & BIT0;
        //T6Cap = (ch >> 1) & BIT0;
        if (ch & BIT1)
        {
            pRemote->dataFormatDF = DF_MMR;    // Supports MMR
            TRACE0(" - T.6 coding capability (MMR)");
        }

        if (pRemote->dataFormatDF == DF_MH)
        {
            TRACE0(" - One dimensional coding capability (MH)");
        }

        if (length >= 5 && RdReg(ch, BIT0))
        {
            ch = pT30->HDLC_RX_Buf[7];

            if (length >= 6 && RdReg(ch, BIT0))
            {
                ch = pT30->HDLC_RX_Buf[8];

                if (ch & BIT7)
                {
                    pRemote->verticalResolutionVR |= VR_R8_154;
                    TRACE0(" - R8 x 15.4 lines/mm");
                }

                if (ch & BIT6)
                {
                    pRemote->verticalResolutionVR |= VR_300_300;
                    TRACE0(" - 300 x 300 pels/25.4 mm");
                }

                if (ch & BIT5)
                {
                    pRemote->verticalResolutionVR |= VR_R16_154 | VR_400_400;
                    TRACE0(" - R16 x 15.4 lines/mm and/or 400 x 400 pels/25.4 mm");
                }

                if (isDIS && (ch & BIT1))
                {
                    SetReg(pT30->remoteFAPCap, BIT1);
                    TRACE0(" - Selective polling capability");
                }

                if (length >= 7 && RdReg(ch, BIT0))
                {
                    // fixing issue 155
                    ch = pT30->HDLC_RX_Buf[10];

                    if (isDIS && (ch & BIT7))
                    {
                        SetReg(pT30->remoteFAPCap, BIT2);
                        TRACE0(" - Subaddressing capability");
                    }

                    if (isDIS && (ch & BIT6))
                    {
                        SetReg(pT30->remoteFAPCap, BIT0);
                        TRACE0(" - Password capability");
                    }

                    if (length >= 9 && RdReg(pT30->HDLC_RX_Buf[10], BIT0))
                    {
                        ch = pT30->HDLC_RX_Buf[11];

                        if (RdReg(ch, BIT4))
                        {
                            pRemote->jpegOptionJP = JP_ENABLE_JPEG;
                            TRACE0(" - JPEG coding");
                        }

                        if (RdReg(ch, BIT3))
                        {
                            pRemote->jpegOptionJP |= JP_FULL_COLOR;
                            TRACE0(" - Full colour mode");
                        }

                        if (length >= 13 && RdReg(pT30->HDLC_RX_Buf[14], BIT0))
                        {
                            ch = pT30->HDLC_RX_Buf[15];

                            if (RdReg(ch, BIT6))
                            {
                                pRemote->verticalResolutionVR |= VR_100_100;
                                TRACE0(" - 100 x 100 pels/25.4 mm for colour/gray scale");
                            }

                            if (RdReg(ch, BIT7))
                            {
                                if (pRemote->verticalResolutionVR & VR_300_300)
                                {
                                    TRACE0(" - 300 x 300 pels/25.4 mm for colour/gray scale");
                                }

                                if (pRemote->verticalResolutionVR & VR_400_400)
                                {
                                    TRACE0(" - 400 x 400 pels/25.4 mm for colour/gray scale");
                                }
                            }

                            if (length >= 14 && RdReg(ch, BIT0))
                            {
                                ch = pT30->HDLC_RX_Buf[16];

                                if (RdReg(ch, BIT3))
                                {
                                    pRemote->verticalResolutionVR |= VR_600_1200;
                                    TRACE0(" - 600 x 1200 pels/25.4 mm");
                                }

                                if (RdReg(ch, BIT4))
                                {
                                    pRemote->verticalResolutionVR |= VR_400_800;
                                    TRACE0(" - 400 x 800 pels/25.4 mm");
                                }

                                if (RdReg(ch, BIT5))
                                {
                                    pRemote->verticalResolutionVR |= VR_300_600;
                                    TRACE0(" - 300 x 600 pels/25.4 mm");
                                }

                                if (RdReg(ch, BIT6))
                                {
                                    if (RdReg(ch, BIT1))
                                    {
                                        TRACE0(" - 1200 x 1200 pels/25.4 mm for colour/gray scale");
                                    }
                                    else
                                    {
                                        TRACE0(" - 1200 x 1200 pels/25.4 mm");
                                    }

                                    pRemote->verticalResolutionVR |= VR_1200_1200;
                                }

                                if (RdReg(ch, BIT7))
                                {
                                    if (RdReg(ch, BIT2))
                                    {
                                        TRACE0(" - 600 x 600 pels/25.4 mm for colour/gray scale");
                                    }
                                    else
                                    {
                                        TRACE0(" - 600 x 600 pels/25.4 mm");
                                    }

                                    pRemote->verticalResolutionVR |= VR_600_600;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (isDIS)
    {
        pT30ToDte->infoTxtRsp = INFO_TXT_FIS; TRACE("%s", "T30: +FIS:");
    }
    else
    {
        pT30ToDte->infoTxtRsp = INFO_TXT_FTC; TRACE("%s", "T30: +FTC:");
    }

    TRACE("%04x,", pRemote->verticalResolutionVR);
    TRACE("%x,", pRemote->bitRateBR);
    TRACE("%x,", pRemote->pageWidthWD);
    TRACE("%x,", pRemote->pageLengthLN);
    TRACE("%x,", pRemote->dataFormatDF);
    TRACE("%x,", pRemote->errorCorrectionEC);
    TRACE("%02x,", pRemote->fileTransferBF);
    TRACE("%x,", pRemote->scanTimeST);
    TRACE1("%02x", pRemote->jpegOptionJP);

#if SUPPORT_V34FAX

    if (isV34Selected && pRemote->errorCorrectionEC == EC_NON_ECM)
    {
        TRACE0("T30: ERROR. V.34 must come with ECM");
        pT30->isRemoteComptRec = FALSE;
        ret = FALSE;
    }

#endif

    return ret;
}

UBYTE T30_AnalysisDCS_CTC(T30Struct *pT30, UBYTE isCTC)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30SessionSubParm *pStatus = &pT30ToDte->faxParmStatus;
    UBYTE ch, DCS_BitRate, DCS_Rate, tmp, isMetric, sub_jp = 0, ret = TRUE;
    SBYTE length;

#if SUPPORT_T38EP
    length = pT30->HDLCLen - 3; // fixing issue 139
#else
    length = pT30->HDLCLen - 5; // fixing issue 139
#endif

    if (length < 6 || !RdReg(pT30->HDLC_RX_Buf[7], BIT0))
    {
        isMetric = TRUE; // fixing issue 139, the default system is metric-based
    }
    else
    {
        isMetric = !RdReg(pT30->HDLC_RX_Buf[8], BIT4);
    }

#if SUPPORT_T38EP
    ch = pT30->HDLC_RX_Buf[3];

    if (ch & BIT5)
    {
        TRACE0(" - Real-time Internet fax (ITU-T T.38)");
    }

#endif

    /* 2nd fcf byte */
    ch = pT30->HDLC_RX_Buf[4];

    /* Set as receive image mode */
    if (ch & BIT6)
    {
        TRACE0(" - Ordered to receive");
    }

    /* bit 11-14 */
    DCS_BitRate = ch & 0x3C;

#if SUPPORT_V34FAX

    if (pDceToT30->isV34Selected) /* For V34, 11 to 14 are invalid */
    {
        pStatus->bitRateBR = pDceToT30->priChnlBitRate - 1;
        pT30ToDte->faxProtocol = V34FAX;
    }
    else
#endif
    {
        switch (DCS_BitRate)
        {
#if SUPPORT_V17
            case    DCS_V17_14400:
                DCS_Rate = BR_14400;
                TRACE0(" - 14 400 bit/s, ITU-T V.17");
                pT30ToDte->faxProtocol = V17;
                break;
            case    DCS_V17_12000:
                DCS_Rate = BR_12000;
                TRACE0(" - 12 000 bit/s, ITU-T V.17");
                pT30ToDte->faxProtocol = V17;
                break;
#endif
#if SUPPORT_V17 + SUPPORT_V29
            case    DCS_V17_9600:
                DCS_Rate = BR_9600;
                TRACE0(" - 9600 bit/s, ITU-T V.17");
                pT30ToDte->faxProtocol = V17;
                break;
            case    DCS_V29_9600:
                DCS_Rate = BR_9600;
                TRACE0(" - 9600 bit/s, ITU-T V.29");
                pT30ToDte->faxProtocol = V29;
                break;
            case    DCS_V17_7200:
                DCS_Rate = BR_7200;
                TRACE0(" - 7200 bit/s, ITU-T V.17");
                pT30ToDte->faxProtocol = V17;
                break;
            case    DCS_V29_7200:
                DCS_Rate = BR_7200;
                TRACE0(" - 7200 bit/s, ITU-T V.29");
                pT30ToDte->faxProtocol = V29;
                break;
#endif
#if SUPPORT_V27TER
            case    DCS_V27_4800:
                DCS_Rate = BR_4800;
                TRACE0(" - 4800 bit/s, ITU-T V.27 ter");
                pT30ToDte->faxProtocol = V27TER;
                break;
            case    DCS_V27_2400:
                DCS_Rate = BR_2400;
                TRACE0(" - 2400 bit/s, ITU-T V.27 ter");
                pT30ToDte->faxProtocol = V27TER;
                break;
#endif
            default:
                DCS_Rate = BR_2400;
                pT30ToDte->faxProtocol = 0;
                TRACE0("T30: ERROR. Invalid bit rate");
                ret = FALSE;
                break;
        }

        pStatus->bitRateBR = DCS_Rate;

        pT30->T30Speed = DCS_BitRate;
    }

    if (pStatus->bitRateBR < pDteToT30->minPhaseCSpd)
    {
        TRACE0("T30: ERROR. Data signalling rate is too low");
        ret = FALSE;
    }

#if SUPPORT_ECM_MODE

    if (isCTC)
    {
        return ret;
    }

#endif

    if (ch & BIT1)
    {
        if (isMetric)
        {
            pStatus->verticalResolutionVR = VR_R8_77;
            pT30->pgVRforWD = X_R8;
            TRACE0(" - R8 x 7.7 lines/mm");
        }
        else
        {
            pStatus->verticalResolutionVR = VR_200_200;
            pT30->pgVRforWD = X_200DPI;
            TRACE0(" - 200 x 200 pels/25.4 mm");
        }
    }

    if (ch & BIT0)    /* 2D encoding. Could be MR or MMR depending on bit 31 */
    {
        pStatus->dataFormatDF = DF_MR; /* MR */
        TRACE0(" - Two dimensional coding (MR)");
    }

    /* 3rd fcf byte */
    ch = pT30->HDLC_RX_Buf[5];

    /* bit 17-18 */
    switch (pStatus->pageWidthWD = PageLenTable[(ch >> 6) & 0x3])
    {
        case ST_0: TRACE0(" - Scan line length 215 mm +/- 1%%"); break;
        case ST_1: TRACE0(" - Scan line length 255 mm +/- 1%%"); break;
        case ST_2: TRACE0(" - Scan line length 303 mm +/- 1%%"); break;
    }

    /* bit 19-20 */
    switch (pStatus->pageLengthLN = PageLenTable[(ch >> 4) & 0x3])
    {
        case LN_A4: TRACE0(" - Recording length A4 (297 mm)"); break;
        case LN_B4: TRACE0(" - Recording length A4 (297 mm) and B4 (364 mm)"); break;
        case LN_UNLIMITED: TRACE0(" - Recording length Unlimited"); break;
    }

    /* bit 21-23 */
    switch (pStatus->scanTimeST = ST_Table_for_DIS[(ch >> 1) & 0x7])
    {
        case ST_0: TRACE0(" - Min. scan line time 00 ms"); break;
        case ST_1: TRACE0(" - Min. scan line time 05 ms"); break;
        case ST_3: TRACE0(" - Min. scan line time 10 ms"); break;
        case ST_5: TRACE0(" - Min. scan line time 20 ms"); break;
        case ST_7: TRACE0(" - Min. scan line time 40 ms"); break;
    }

    /* extended bit */
    if (length < 4 || !RdReg(ch, BIT0))
    {
#if SUPPORT_ECM_MODE
        pStatus->errorCorrectionEC = EC_NON_ECM;
        TRACE0(" - Error correction mode OFF");
        pT30ToDte->ecmFrmSize = ECM_SIZE_0;
#endif
    }
    else
    {
        /* 4th fcf byte */
        ch = pT30->HDLC_RX_Buf[6];

        /* bit 27 */
#if SUPPORT_ECM_MODE
        pStatus->errorCorrectionEC = (ch >> 5) & BIT0;

        if (pStatus->errorCorrectionEC)
        {
            TRACE0(" - Error correction mode ON");
        }
        else
        {
            TRACE0(" - Error correction mode OFF");
            pT30ToDte->ecmFrmSize = ECM_SIZE_0;
        }

#else
        pStatus->errorCorrectionEC = EC_NON_ECM;
#endif

        tmp = (ch >> 4) & BIT0;

        if (tmp)
        {
            TRACE0(" - Frame size 1 = 064 octets");
            pT30ToDte->ecmFrmSize = ECM_SIZE_64;
            pT30->T4ECM.frmSizeBitNum = 6;
        }
        else if (pStatus->errorCorrectionEC != EC_NON_ECM)
        {
            TRACE0(" - Frame size 0 = 256 octets");
            pT30ToDte->ecmFrmSize = ECM_SIZE_256;
            pT30->T4ECM.frmSizeBitNum = 8;
        }

        /* bit 31 */
        if (ch & BIT1)
        {
            pStatus->dataFormatDF = DF_MMR;    // MMR
            TRACE0(" - T.6 coding enabled (MMR)");
        }

#define CH_RX  pT30->HDLC_RX_Buf

        if (length >= 9 && RdReg(CH_RX[10], BIT0))
        {
            if (CH_RX[11] & BIT4)
            {
                sub_jp = JP_ENABLE_JPEG;
            }

            if (CH_RX[11] & BIT3)
            {
                sub_jp |= JP_FULL_COLOR;
                pStatus->verticalResolutionVR = VR_200_200;
            }

            if (CH_RX[11] & BIT2) { sub_jp |= JP_ENABLE_HUFFMAN; }

            if (CH_RX[11] & BIT1) { sub_jp |= JP_12_BITS_PEL_COMP; }
        }

        if (length >= 5 && RdReg(CH_RX[7], BIT0))
        {
            if (CH_RX[8]&BIT5)
            {
                if (isMetric)
                {
                    pStatus->verticalResolutionVR = VR_R16_154;
                    pT30->pgVRforWD = X_R16;
                    TRACE0(" - R16 x 15.4 lines/mm");
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_400_400;
                    pT30->pgVRforWD = X_400DPI;
                    TRACE0(" - 400 x 400 pels/25.4 mm");
                }
            }
            else if (CH_RX[8]&BIT6)
            {
                pStatus->verticalResolutionVR = VR_300_300;
                pT30->pgVRforWD = X_300DPI; // 300dpi
                TRACE0(" - 300 x 300 pels/25.4 mm");
            }
            else if (CH_RX[8]&BIT7)
            {
                pStatus->verticalResolutionVR = VR_R8_154;
                TRACE0(" - R8 x 15.4 lines/mm");
                pT30->pgVRforWD = X_R8; // R8/200dpi
            }
        }

        if (length >= 12 && RdReg(CH_RX[14], BIT0))
        {
            if (RdReg(CH_RX[15], BIT6))
            {
                pStatus->verticalResolutionVR = VR_100_100;
                TRACE0(" - 100 x 100 pels/25.4 mm for colour/gray scale");
            }
            else if (RdReg(CH_RX[15], BIT7))
            {
                if (pStatus->verticalResolutionVR == VR_300_300)
                {
                    TRACE0(" - 300 x 300 pels/25.4 mm for colour/gray scale");
                }
                else if (pStatus->verticalResolutionVR == VR_400_400)
                {
                    TRACE0(" - 400 x 400 pels/25.4 mm for colour/gray scale");
                }
            }
        }

        if (length >= 13 && RdReg(CH_RX[15], BIT0))
        {
            if (RdReg(CH_RX[16], BIT7))
            {
                pStatus->verticalResolutionVR = VR_600_600;
                pT30->pgVRforWD = X_600DPI;

                if (RdReg(CH_RX[16], BIT2))
                {
                    TRACE0(" - 600 x 600 pels/25.4 mm for colour/gray scale");
                }
                else
                {
                    TRACE0(" - 600 x 600 pels/25.4 mm");
                }
            }
            else if (RdReg(CH_RX[16], BIT6))
            {
                pStatus->verticalResolutionVR = VR_1200_1200;
                pT30->pgVRforWD = X_1200DPI;

                if (RdReg(CH_RX[16], BIT1))
                {
                    TRACE0(" - 1200 x 1200 pels/25.4 mm for colour/gray scale");
                }
                else
                {
                    TRACE0(" - 1200 x 1200 pels/25.4 mm");
                }
            }
            else if (RdReg(CH_RX[16], BIT5))
            {
                pStatus->verticalResolutionVR = VR_300_600;
                pT30->pgVRforWD = X_300DPI;
                TRACE0(" - 300 x 600 pels/25.4 mm");
            }
            else if (RdReg(CH_RX[16], BIT4))
            {
                pT30->pgVRforWD = X_400DPI;
                pStatus->verticalResolutionVR = VR_400_800;
                TRACE0(" - 400 x 800 pels/25.4 mm");
            }
            else if (RdReg(CH_RX[16], BIT3))
            {
                pStatus->verticalResolutionVR = VR_600_1200;
                pT30->pgVRforWD = X_600DPI;
                TRACE0(" - 600 x 1200 pels/25.4 mm");
            }
        }
    }

#undef CH_RX

    if (sub_jp != 0)
    {
        TRACE0(" - Full color mode");
    }
    else if (pStatus->dataFormatDF == DF_MH)
    {
        TRACE0(" - One dimensional coding (MH)");
    }

    pStatus->jpegOptionJP = sub_jp;

    if (isMetric)
    {
        TRACE0(" - Metric based resolution");

        if (pStatus->verticalResolutionVR == VR_R8_385)
        {
            pT30->pgVRforWD = X_R8;
            TRACE0(" - R8 x 3.85 lines/mm");
        }
    }
    else
    {
        TRACE0(" - Inch based resolution");

        if (pStatus->verticalResolutionVR == VR_R8_385)
        {
            pStatus->verticalResolutionVR = VR_200_100;
            pT30->pgVRforWD = X_200DPI;
            TRACE0(" - 200 x 100 pels/25.4 mm");
        }
    }

    // fixing issue 194, reporting FCS even if it's not a compatible transmitor or local DTE is not willing to receive
    pT30ToDte->infoTxtRsp = INFO_TXT_FCS; TRACE("%s", "T30: +FCS:");
    TRACE("%04x,", pStatus->verticalResolutionVR);
    TRACE("%x,", pStatus->bitRateBR);
    TRACE("%x,", pStatus->pageWidthWD);
    TRACE("%x,", pStatus->pageLengthLN);
    TRACE("%x,", pStatus->dataFormatDF);
    TRACE("%x,", pStatus->errorCorrectionEC);
    TRACE("%02x,", pStatus->fileTransferBF);
    TRACE("%x,", pStatus->scanTimeST);
    TRACE1("%02x", pStatus->jpegOptionJP);

#if SUPPORT_V34FAX

    if (pDceToT30->isV34Selected && pStatus->errorCorrectionEC == EC_NON_ECM)
    {
        pT30->wasLastTry = TRUE;
        return FALSE;
    }
    else
#endif
        if (ret && pDteToT30->isCapToRcv)
        {
            pT30->wasLastTry = FALSE;
            pT30->LineCount = 0;

            if (pStatus->dataFormatDF == DF_MMR)
            {
                FXL_T6Init(&(pT30->T6LineStru), ImageWidth_Tiff[pStatus->pageWidthWD][pT30->pgVRforWD]);
            }

            return TRUE;
        }
        else
        {
            pT30->wasLastTry = TRUE;
            return FALSE;
        }
}
#endif
