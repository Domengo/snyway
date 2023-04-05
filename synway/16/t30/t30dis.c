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

#include "t30ext.h"

#if SUPPORT_T30
UBYTE T30_MakeDIS_DTC(T30Struct *pT30, UBYTE isDIS)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30SessionSubParm *pSettings = &pDteToT30->faxParmSettings;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    UBYTE isJpeg = pSettings->jpegOptionJP != JP_NON_JPEG;
    UDWORD extraResolutions = pSettings->verticalResolutionVR & (VR_600_600 | VR_1200_1200 | VR_300_600 | VR_400_800 | VR_600_1200);
    UBYTE isCG300DpiOr400Dpi = (pSettings->verticalResolutionVR & (VR_300_300 | VR_400_400)) && isJpeg;
    UBYTE isCG100Dpi = (pSettings->verticalResolutionVR & VR_100_100) && isJpeg;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;
#endif

    if (isDIS)
    {
        TRACE0("T30: HDLC MAKE:     DIS");
    }
    else
    {
        TRACE0("T30: HDLC MAKE:     DTC");
    }

    /* the first 3 bytes already are set */
    /* 4th byte */
    pT30->HDLC_TX_Buf[3] = 0x00;
#if SUPPORT_T38EP
    pT30->HDLC_TX_Buf[3] |= BIT5; // enable IAF
    TRACE0(" - Real-time Internet fax (ITU-T T.38)");
#endif

#if FRAMESIZE_256_64
    SetReg(pT30->HDLC_TX_Buf[3], BIT1);
    TRACE0(" - 64 octets preferred");
#else
    TRACE0(" - 256 octets preferred");
#endif

    /* 5th  byte */
    if (pDteToT30->isDocForPolling)
    {
        pT30->HDLC_TX_Buf[4] |= BIT7;
        TRACE0(" - Ready to transmit a facsimile document (polling)");
    }

    if (pDteToT30->isCapToRcv)
    {
        pT30->HDLC_TX_Buf[4] |= BIT6;
        TRACE0(" - Receiver fax operation");
    }

#if SUPPORT_V34FAX

    if (isV34Selected)
    {
        pT30->HDLC_TX_Buf[4] |= DIS_V34F;
    }
    else
#endif
    {
        switch (pSettings->bitRateBR)
        {
            case BR_2400:
                pT30->HDLC_TX_Buf[4] |= DIS_V27F;
                TRACE0(" - ITU-T V.27 ter fall-back mode");
                break;
            case BR_4800:
                pT30->HDLC_TX_Buf[4] |= DIS_V27;
                TRACE0(" - ITU-T V.27 ter");
                break;
            case BR_7200:
            case BR_9600:
                pT30->HDLC_TX_Buf[4] |= DIS_V29F;
                TRACE0(" - ITU-T V.27 ter and V.29");
                break;
            default:
                pT30->HDLC_TX_Buf[4] |= DIS_V17F;
                TRACE0(" - ITU-T V.27 ter, V.29, and V.17");
                break;
        }
    }

    TRACE0(" - R8 x 3.85 lines/mm and/or 200 x 100 pels/25.4 mm");

    if (pSettings->verticalResolutionVR & (VR_R8_77 | VR_200_200))
    {
        pT30->HDLC_TX_Buf[4] |= BIT1;
        TRACE0(" - R8 x 7.7 lines/mm and/or 200 x 200 pels/25.4 mm");
    }

    if (pSettings->dataFormatDF > DF_MH)        /* MR or MMR */
    {
        pT30->HDLC_TX_Buf[4] |= BIT0;
        TRACE0(" - Two dimensional coding capability (MR)");
    }
    else
    {
        TRACE0(" - One dimensional coding capability (MH)");
    }

    /* 6th  byte */
    /* bit 17 , 18 */
    if (pSettings->pageWidthWD == WD_1)
    {
        pT30->HDLC_TX_Buf[5] |=    BIT7;
        TRACE0(" - Scan line length 215 mm +/- 1%%, and 255 mm +/- 1%%");
    }
    else if (pSettings->pageWidthWD == WD_2)
    {
        pT30->HDLC_TX_Buf[5] |=    BIT6;
        TRACE0(" - Scan line length 215 mm +/- 1%%, 255 mm +/- 1%%, and 303 mm +/- 1%%");
    }
    else
    {
        TRACE0(" - Scan line length 215 mm +/- 1%%");
    }

    /* bit 19 , 20 */
    if (pSettings->pageLengthLN == LN_B4)
    {
        pT30->HDLC_TX_Buf[5] |=    BIT5;
        TRACE0(" - Recording length A4 (297 mm) and B4 (364 mm)");
    }
    else if (pSettings->pageLengthLN == LN_UNLIMITED)
    {
        pT30->HDLC_TX_Buf[5] |=    BIT4;
        TRACE0(" - Recording length Unlimited");
    }
    else
    {
        TRACE0(" - Recording length A4 (297 mm)");
    }

    /* bit 21, 22, 23 */
#if SUPPORT_ECM_MODE

    if (pSettings->errorCorrectionEC != EC_NON_ECM)
    {
        pT30->HDLC_TX_Buf[5] |= 0x0E;    /* 0 ms when ECM is used */
    }
    else
#endif
    {
        switch (pSettings->scanTimeST)
        {
            case ST_0:
                /* 0ms */
                pT30->HDLC_TX_Buf[5] |=    0x0E;
                TRACE0(" - Min. scan line time 00 ms at 3.85 l/mm: T7.7 = T3.85");
                break;
            case ST_1:
                /* 5 ms */
                pT30->HDLC_TX_Buf[5] |=    0x08;
                TRACE0(" - Min. scan line time 05 ms at 3.85 l/mm: T7.7 = T3.85");
                break;
            case ST_2:

                /* 10ms */
                if (pSettings->verticalResolutionVR == 0)
                {
                    pT30->HDLC_TX_Buf[5] |=    0x06;
                    TRACE0(" - Min. scan line time 10 ms at 3.85 l/mm: T7.7 = T3.85");
                }
                else
                {
                    pT30->HDLC_TX_Buf[5] |=    0x04;
                    TRACE0(" - Min. scan line time 10 ms at 3.85 l/mm: T7.7 = 1/2 T3.85");
                }

                break;
            case ST_3:
                /* 10ms */
                pT30->HDLC_TX_Buf[5] |=    0x06;
                TRACE0(" - Min. scan line time 10 ms at 3.85 l/mm: T7.7 = T3.85");
                break;
            case ST_4:

                /* 20ms */
                if (pSettings->verticalResolutionVR == 0)
                {
                    pT30->HDLC_TX_Buf[5] |=    0x00;
                    TRACE0(" - Min. scan line time 20 ms at 3.85 l/mm: T7.7 = T3.85");
                }
                else
                {
                    pT30->HDLC_TX_Buf[5] |=    0x0C;
                    TRACE0(" - Min. scan line time 20 ms at 3.85 l/mm: T7.7 = 1/2 T3.85");
                }

                break;
            case ST_5:
                /* 20ms */
                pT30->HDLC_TX_Buf[5] |=    0x00;
                TRACE0(" - Min. scan line time 20 ms at 3.85 l/mm: T7.7 = T3.85");
                break;
            case ST_6:

                /* 40 ms */
                if (pSettings->verticalResolutionVR == 0)
                {
                    pT30->HDLC_TX_Buf[5] |=    0x02;
                    TRACE0(" - Min. scan line time 40 ms at 3.85 l/mm: T7.7 = T3.85");
                }
                else
                {
                    pT30->HDLC_TX_Buf[5] |=    0x0A;
                    TRACE0(" - Min. scan line time 40 ms at 3.85 l/mm: T7.7 = 1/2 T3.85");
                }

                break;
            case ST_7:
                /* 40 ms */
                pT30->HDLC_TX_Buf[5] |=    0x02;
                TRACE0(" - Min. scan line time 40 ms at 3.85 l/mm: T7.7 = T3.85");
                break;
        }
    }

#if SUPPORT_ECM_MODE
    /* bit 24 */
    pT30->HDLC_TX_Buf[5] |=    BIT0;

    /* 7th  byte */
    if (pSettings->dataFormatDF == DF_2D_UNCOMPRESSED)
    {
        pT30->HDLC_TX_Buf[6] |= BIT6;
    }

    if (pSettings->errorCorrectionEC)
    {
        pT30->HDLC_TX_Buf[6] |= BIT5;
        TRACE0(" - Error correction mode ON");

        if (pSettings->dataFormatDF >= DF_MMR) /* MMR or better, and ECM enabled */
        {
            pT30->HDLC_TX_Buf[6] |= BIT1;
            TRACE0(" - T.6 coding capability (MMR)");
        }
    }
    else
    {
        TRACE0(" - Error correction mode OFF");
    }

    pT30->HDLC_TX_Buf[6] |= BIT0;

    pT30->HDLC_TX_Buf[7] |= BIT0;

    /* DK: Bits 44 and 45 can not be both zero */
    if (pSettings->verticalResolutionVR & VR_R8_154)
    {
        pT30->HDLC_TX_Buf[8] |= BIT7;
        TRACE0(" - R8 x 15.4 lines/mm");
    }

    if (pSettings->verticalResolutionVR & VR_300_300)
    {
        pT30->HDLC_TX_Buf[8] |= BIT6;
        TRACE0(" - 300 x 300 pels/25.4 mm");
    }

    if (pSettings->verticalResolutionVR & (VR_R16_154 | VR_400_400))
    {
        pT30->HDLC_TX_Buf[8] |= BIT5;
        TRACE0(" - R16 x 15.4 lines/mm and/or 400 x 400 pels/25.4 mm");
    }

    if (pSettings->verticalResolutionVR & (VR_R8_77 | VR_R8_154 | VR_R16_154))
    {
        pT30->HDLC_TX_Buf[8] |= BIT3;    /* Bit 45, Metric preference, is chosen */
        TRACE0(" - Metric based resolution preferred");
    }
    else
    {
        pT30->HDLC_TX_Buf[8] |= BIT4;    /* Bit 44, Inch preference, is chosen */
        TRACE0(" - Inch based resolution preferred");
    }

    if (isDIS)
    {
        if (RdReg(pDteToT30->addressPollingCap, BIT1))
        {
            pT30->HDLC_TX_Buf[8] |= BIT1;
            TRACE0(" - Selective polling capability");
        }
    }
    else if (RdReg(pT30->remoteFAPCap, BIT1))
    {
        pT30->HDLC_TX_Buf[8] |= BIT1;
        TRACE0(" - Selective polling transmission");
    }

    pT30->HDLC_TX_Buf[8] |= BIT0;

    if (isDIS && RdReg(pDteToT30->addressPollingCap, BIT2))
    {
        pT30->HDLC_TX_Buf[9] |= BIT7;
        TRACE0(" - Subaddressing capability");
    }

    if (isDIS)
    {
        if (RdReg(pDteToT30->addressPollingCap, BIT0))
        {
            pT30->HDLC_TX_Buf[9] |= BIT6;
            TRACE0(" - Password");
        }
    }
    else if (RdReg(pT30->remoteFAPCap, BIT0))
    {
        pT30->HDLC_TX_Buf[9] |= BIT6;
        TRACE0(" - Password transmission");
    }

    if (!isJpeg && extraResolutions == 0)
    {
        return (10);
    }

    pT30->HDLC_TX_Buf[9] |= BIT0;
    pT30->HDLC_TX_Buf[10] |= BIT0;

    if (RdReg(pSettings->jpegOptionJP, JP_ENABLE_JPEG))
    {
        pT30->HDLC_TX_Buf[11] |= BIT4;
        TRACE0(" - JPEG coding");
    }

    if (RdReg(pSettings->jpegOptionJP, JP_FULL_COLOR))
    {
        pT30->HDLC_TX_Buf[11] |= BIT3;
        TRACE0(" - Full colour mode");
    }

    if (!isCG300DpiOr400Dpi && !isCG100Dpi && extraResolutions == 0)
    {
        return (12);
    }

    pT30->HDLC_TX_Buf[11] |= BIT0;
    pT30->HDLC_TX_Buf[12] |= BIT0;
    pT30->HDLC_TX_Buf[13] |= BIT0;
    pT30->HDLC_TX_Buf[14] |= BIT0;

    if (isCG300DpiOr400Dpi)
    {
        pT30->HDLC_TX_Buf[15] |= BIT7;
        TRACE0(" - 300 x 300 pels/25.4 mm or 400 x 400 pels/25.4 mm for colour/gray scale");
    }

    if (isCG100Dpi)
    {
        pT30->HDLC_TX_Buf[15] |= BIT6;
        TRACE0(" - 100 x 100 pels/25.4 mm for colour/gray scale");
    }

    if (extraResolutions == 0)
    {
        return (16);
    }

    pT30->HDLC_TX_Buf[15] |= BIT0;

    if (extraResolutions & VR_600_600)
    {
        pT30->HDLC_TX_Buf[16] |= BIT7;

        if (isJpeg)
        {
            pT30->HDLC_TX_Buf[16] |= BIT2;
            TRACE0(" - 600 x 600 pels/25.4 mm for colour/gray scale");
        }
        else
        {
            TRACE0(" - 600 x 600 pels/25.4 mm");
        }
    }

    if (extraResolutions & VR_1200_1200)
    {
        pT30->HDLC_TX_Buf[16] |= BIT6;

        if (isJpeg)
        {
            pT30->HDLC_TX_Buf[16] |= BIT1;
            TRACE0(" - 1200 x 1200 pels/25.4 mm for colour/gray scale");
        }
        else
        {
            TRACE0(" - 1200 x 1200 pels/25.4 mm");
        }
    }

    if (extraResolutions & VR_300_600)
    {
        pT30->HDLC_TX_Buf[16] |= BIT5;
        TRACE0(" - 300 x 600 pels/25.4 mm");
    }

    if (extraResolutions & VR_400_800)
    {
        pT30->HDLC_TX_Buf[16] |= BIT4;
        TRACE0(" - 400 x 800 pels/25.4 mm");
    }

    if (extraResolutions & VR_600_1200)
    {
        pT30->HDLC_TX_Buf[16] |= BIT3;
        TRACE0(" - 600 x 1200 pels/25.4 mm");
    }

    return (17);

#else /* without ECM */

    pT30->HDLC_TX_Buf[5] &= 0xFFFE;
    TRACE0(" - Error correction mode OFF");

    return (6);

#endif
}
#endif
