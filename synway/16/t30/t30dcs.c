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
/* Convert ST value of +FCS into actual scan line time (ms) */
static CONST UBYTE ST_ScanTime_TAB[2][8] =
{
    { 0, 5, 10, 10, 20, 20, 40, 40 },    /* normal resolution (T3.85) */
    { 0, 5,  5, 10, 10, 20, 20, 40 }    /* fine resolution (T7.7) */
};

static void T30_UpdatFCS(T30Struct *pT30)
{
    T30SessionSubParm *pStatus = &pT30->pT30ToDte->faxParmStatus;
    T30SessionSubParm *pRemote = &pT30->pT30ToDte->remoteSettings;
    T30SessionSubParm *pSettings = &pT30->pDteToT30->faxParmSettings;
    //T30SessionSubParm *pCapa = &pT30->pDteToT30->faxParmCapability;
    UBYTE isToContinue = TRUE;
    UBYTE isJpeg;

    memcpy(pStatus, pSettings, sizeof(T30SessionSubParm));

    if (pStatus->errorCorrectionEC > pRemote->errorCorrectionEC)
    {
        pStatus->errorCorrectionEC = pRemote->errorCorrectionEC;
    }

    if (pStatus->pageWidthWD > pRemote->pageWidthWD)
    {
        pStatus->pageWidthWD = pRemote->pageWidthWD;
    }

    if (pStatus->pageLengthLN > pRemote->pageLengthLN)
    {
        pStatus->pageLengthLN = pRemote->pageLengthLN;
    }

    if (pStatus->fileTransferBF > pRemote->fileTransferBF)
    {
        pStatus->fileTransferBF = pRemote->fileTransferBF;
    }

    if (pStatus->jpegOptionJP > pRemote->jpegOptionJP)
    {
        pStatus->jpegOptionJP = pRemote->jpegOptionJP;
    }

    if (pStatus->errorCorrectionEC == EC_NON_ECM && pStatus->jpegOptionJP > JP_NON_JPEG)
    {
        TRACE1("T30: ERROR. Invalid JP 0x%x", pStatus->jpegOptionJP);
        pStatus->jpegOptionJP = JP_NON_JPEG;
    }

    if (pStatus->bitRateBR > pRemote->bitRateBR)
    {
        pStatus->bitRateBR = pRemote->bitRateBR;
    }

    if (pStatus->dataFormatDF > pRemote->dataFormatDF)
    {
        pStatus->dataFormatDF = pRemote->dataFormatDF;
    }

    if (pStatus->errorCorrectionEC == EC_NON_ECM && pStatus->dataFormatDF >= DF_MMR)
    {
        TRACE1("T30: ERROR. Invalid DF 0x%x", pStatus->dataFormatDF);
        pStatus->dataFormatDF = DF_MH;
    }

    if (pStatus->errorCorrectionEC != EC_NON_ECM)
    {
        pStatus->scanTimeST = ST_0;    // MSLT = 0 when ECM enabled
    }
    else if (pStatus->scanTimeST < pRemote->scanTimeST)
    {
        pStatus->scanTimeST = pRemote->scanTimeST;
    }

    isJpeg = pStatus->jpegOptionJP != JP_NON_JPEG;

    while (isToContinue)
    {
        switch (pStatus->verticalResolutionVR)
        {
            case VR_R8_385:
                isToContinue = FALSE;
                break;
            case VR_200_100:

                if (isJpeg)
                {
                    pStatus->verticalResolutionVR = VR_100_100;
                }
                else
                {
                    isToContinue = FALSE;
                }

                break;
            case VR_R8_77:

                if (pRemote->verticalResolutionVR & VR_R8_77)
                {
                    isToContinue = FALSE;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_R8_385;
                }

                break;
            case VR_R8_154:

                if (pRemote->verticalResolutionVR & VR_R8_154)
                {
                    isToContinue = FALSE;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_R8_77;
                }

                break;
            case VR_R16_154:

                if (pRemote->verticalResolutionVR & VR_R16_154)
                {
                    isToContinue = FALSE;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_R8_154;
                }

                break;
            case VR_100_100:

                if (!(pRemote->verticalResolutionVR & VR_100_100))
                {
                    TRACE0("T30: ERROR. Remote side doesn't support the resolution");
                }

                isToContinue = FALSE;
                break;
            case VR_200_200:

                if (pRemote->verticalResolutionVR & VR_200_200)
                {
                    isToContinue = FALSE;
                }
                else if (isJpeg)
                {
                    pStatus->verticalResolutionVR = VR_100_100;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_200_100;
                }

                break;
            case VR_200_400:

                if (pRemote->verticalResolutionVR & VR_200_400)
                {
                    isToContinue = FALSE;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_200_200;
                }

                break;
            case VR_300_300:

                if (pRemote->verticalResolutionVR & VR_300_300)
                {
                    isToContinue = FALSE;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_200_200;
                }

                break;
            case VR_300_600:

                if (pRemote->verticalResolutionVR & VR_300_600)
                {
                    isToContinue = FALSE;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_300_300;
                }

                break;
            case VR_400_400:

                if (pRemote->verticalResolutionVR & VR_400_400)
                {
                    isToContinue = FALSE;
                }
                else if (isJpeg)
                {
                    pStatus->verticalResolutionVR = VR_300_300;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_200_400;
                }

                break;
            case VR_400_800:

                if (pRemote->verticalResolutionVR & VR_400_800)
                {
                    isToContinue = FALSE;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_400_400;
                }

                break;
            case VR_600_600:

                if (pRemote->verticalResolutionVR & VR_600_600)
                {
                    isToContinue = FALSE;
                }
                else if (isJpeg)
                {
                    pStatus->verticalResolutionVR = VR_400_400;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_300_600;
                }

                break;
            case VR_600_1200:

                if (pRemote->verticalResolutionVR & VR_600_1200)
                {
                    isToContinue = FALSE;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_600_600;
                }

                break;
            case VR_1200_1200:

                if (pRemote->verticalResolutionVR & VR_1200_1200)
                {
                    isToContinue = FALSE;
                }
                else if (isJpeg)
                {
                    pStatus->verticalResolutionVR = VR_600_600;
                }
                else
                {
                    pStatus->verticalResolutionVR = VR_600_1200;
                }

                break;
            default:
                TRACE1("T30: ERROR. Invalid VR 0x%x", pStatus->verticalResolutionVR);
                isToContinue = FALSE;
                pStatus->verticalResolutionVR = VR_R8_385;
                break;
        }
    }
}

UBYTE T30_MakeDCS_CTC(T30Struct *pT30, UBYTE isCTC)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pT30->pDceToT30->isV34Selected;
#endif
    T30SessionSubParm *pStatus = &pT30->pT30ToDte->faxParmStatus;
    UBYTE temp;
    UBYTE isRspRecError = (pT30->rxErrorTimes == 0) ? FALSE : TRUE;
    UBYTE isToSendSUBorSID = RdReg(pT30->remoteFAPCap, BIT0 | BIT2);
    UBYTE isJpeg, isCG300DpiOr400Dpi, isCG100Dpi, isMetric;
    UDWORD extraResolutions;

#if SUPPORT_ECM_MODE

    if (isCTC)
    {
        TRACE0("T30: HDLC MAKE:     CTC");
    }
    else
#endif
    {
        TRACE0("T30: HDLC MAKE:     DCS");
    }

    /* the first 3 bytes already are set */
    /* 4th    byte */
    pT30->HDLC_TX_Buf[3] = 0x00;
#if SUPPORT_T38EP
    //LOG_writePlain(LOG_LEVEL_DEBUG, " - Real-time Internet fax (ITU-T T.38)", 0);
    //pT30->HDLC_TX_Buf[3] |= BIT5; // enable IAF
#endif

#if SUPPORT_ECM_MODE

    if (isCTC)
    {
        pT30->isFallBackTraining = TRUE;
    }
    else
#endif
    {
        pT30->HDLC_TX_Buf[4] |= BIT6;
        TRACE0(" - Ordering to receive");
    }

    if (!pT30->isFallBackTraining || isRspRecError)
    {
        if (!isRspRecError)
        {
            /* using receiving DIS update FCS[] */
            T30_UpdatFCS(pT30);
        }

#if SUPPORT_V34FAX

        if (isV34Selected)
        {
            if (pStatus->errorCorrectionEC == EC_NON_ECM)
            {
                T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_XMIT);
                TRACE0("T30: ERROR. V.34 must come with ECM");
                return 0;
            }

            pT30->HDLC_TX_Buf[4] |= DCS_V34FAX;
            pT30ToDte->faxProtocol = V34FAX;
            pStatus->bitRateBR = pDceToT30->priChnlBitRate - 1;
        }
        else
#endif
#if SUPPORT_V27TER
            if (pStatus->bitRateBR == BR_2400)
            {
                pT30->HDLC_TX_Buf[4] |= DCS_V27_2400;
                TRACE0(" - 2400 bit/s, ITU-T V.27 ter");
                pT30ToDte->faxProtocol = V27TER;
            }
            else if (pStatus->bitRateBR == BR_4800)
            {
                pT30->HDLC_TX_Buf[4] |= DCS_V27_4800;
                TRACE0(" - 4800 bit/s, ITU-T V.27 ter");
                pT30ToDte->faxProtocol = V27TER;
            }
            else
#endif
#if SUPPORT_V29
                if (pStatus->bitRateBR == BR_7200)
                {
                    pT30->HDLC_TX_Buf[4] |= DCS_V29_7200;
                    TRACE0(" - 7200 bit/s, ITU-T V.29");
                    pT30ToDte->faxProtocol = V29;
                }
                else if (pStatus->bitRateBR == BR_9600)
                {
                    pT30->HDLC_TX_Buf[4] |= DCS_V29_9600;
                    TRACE0(" - 9600 bit/s, ITU-T V.29");
                    pT30ToDte->faxProtocol = V29;
                }
                else
#endif
#if SUPPORT_V17
                    if (pStatus->bitRateBR == BR_12000)
                    {
                        pT30->HDLC_TX_Buf[4] |= DCS_V17_12000;
                        TRACE0(" - 12 000 bit/s, ITU-T V.17");
                        pT30ToDte->faxProtocol = V17;
                    }
                    else if (pStatus->bitRateBR == BR_14400)
                    {
                        TRACE0(" - 14 400 bit/s, ITU-T V.17");
                        pT30->HDLC_TX_Buf[4] |= DCS_V17_14400;
                        pT30ToDte->faxProtocol = V17;
                    }
                    else
#endif
                    {
                        T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_XMIT);
                        TRACE1("T30: ERROR. Invalid bit rate 0x%x", pStatus->bitRateBR);
                        return 0;
                    }

        if (pStatus->bitRateBR == pDteToT30->minPhaseCSpd)
        {
            pT30->wasLastTry = TRUE;
        }
        else if (pStatus->bitRateBR < pDteToT30->minPhaseCSpd)
        {
            T30_GotoPointC(pT30, isCTC ? HANGUP_UNSPECIFIED_PHASE_D_XMIT : HANGUP_FAILURE_TO_TRAIN_AT_FMS); // fixing issue 181
            TRACE0("T30: ERROR. Data signalling rate is too low");
            return 0;
        }
    }
    else
    {
        /* for retraining purpose */
        /* if FTT is received after TCF training stage, DCS bit        */
        /* rate need reduced one step, but still need check remote    */
        /* capability. if RTN is received after image data,            */
        /* do not need modify DCS frame                                */
        UBYTE isToLowerSpeed = TRUE;

        if (isCTC)
        {
            T4ECMstruc *pT4 = &pT30->T4ECM;

            if (pT4->retryCount++ == pDteToT30->ecmRetryCnt)
            {
                pT4->retryCount = 0;
            }
            else
            {
                isToLowerSpeed = FALSE;
            }
        }

        if (isToLowerSpeed)
        {
            switch (pStatus->bitRateBR)
            {
#if SUPPORT_V27TER
                case BR_2400:
                    /* need change phase to send DCN end the process */
                    pT30->HDLC_TX_Buf[4] |=    DCS_V27_2400;
                    TRACE0(" - 2400 bit/s, ITU-T V.27 ter");
                    pT30ToDte->faxProtocol = V27TER;
                    break;
                case BR_4800:
                    /* fall back to 2400 BR case */
                    pT30->HDLC_TX_Buf[4] |=    DCS_V27_2400;
                    TRACE0(" - 2400 bit/s, ITU-T V.27 ter");
                    pStatus->bitRateBR --;
                    pT30ToDte->faxProtocol = V27TER;
                    break;
                case BR_7200:
                    pT30->HDLC_TX_Buf[4] |=    DCS_V27_4800;
                    TRACE0(" - 4800 bit/s, ITU-T V.27 ter");
                    pStatus->bitRateBR --;
                    pT30ToDte->faxProtocol = V27TER;
                    break;
#endif
#if SUPPORT_V29
                case BR_9600:
                    pT30->HDLC_TX_Buf[4] |=    DCS_V29_7200;
                    TRACE0(" - 7200 bit/s, ITU-T V.29");
                    pStatus->bitRateBR --;
                    pT30ToDte->faxProtocol = V29;
                    break;
                case BR_12000:
                    pT30->HDLC_TX_Buf[4] |=    DCS_V29_9600;
                    TRACE0(" - 9600 bit/s, ITU-T V.29");
                    pStatus->bitRateBR --;
                    pT30ToDte->faxProtocol = V29;
                    break;
#endif
#if SUPPORT_V17
                case BR_14400:
                    pT30->HDLC_TX_Buf[4] |=    DCS_V17_12000;
                    TRACE0(" - 12 000 bit/s, ITU-T V.17");
                    pStatus->bitRateBR --;
                    pT30ToDte->faxProtocol = V17;
                    break;
#endif
                default:
                    T30_GotoPointC(pT30, HANGUP_UNSPECIFIED_PHASE_B_XMIT);
                    TRACE1("T30: ERROR. Invalid bit rate 0x%x", pStatus->bitRateBR);
                    return 0;
            }

            if (pStatus->bitRateBR <= pDteToT30->minPhaseCSpd)
            {
                pT30->wasLastTry = TRUE;
            }
        }
        else
        {
            pT30->HDLC_TX_Buf[4] = pT30->T30Speed;
        }
    }

    pT30->T30Speed = (UBYTE)(pT30->HDLC_TX_Buf[4] & 0x3C);

#if SUPPORT_ECM_MODE

    if (isCTC)
    {
        return 5;
    }

#endif

    isJpeg = pStatus->jpegOptionJP != JP_NON_JPEG;
    extraResolutions = pStatus->verticalResolutionVR & (VR_600_600 | VR_1200_1200 | VR_300_600 | VR_400_800 | VR_600_1200);
    isCG300DpiOr400Dpi = (pStatus->verticalResolutionVR & (VR_300_300 | VR_400_400)) && isJpeg;
    isCG100Dpi = (pStatus->verticalResolutionVR & VR_100_100) && isJpeg;

    if (pStatus->verticalResolutionVR & (VR_R8_77 | VR_R8_154 | VR_R16_154) || pStatus->verticalResolutionVR == VR_R8_385)
    {
        isMetric = TRUE;
        TRACE0(" - Metric based resolution"); /* Bit 44, Metric preference, is chosen */
    }
    else
    {
        isMetric = FALSE;
        pT30->HDLC_TX_Buf[8] |= BIT4;    /* Bit 44, Inch preference, is chosen */
        TRACE0(" - Inch based resolution");
    }

    if (pStatus->verticalResolutionVR == VR_R8_385 || pStatus->verticalResolutionVR == VR_200_100)
    {
        if (isMetric)
        {
            TRACE0(" - R8 x 3.85 lines/mm");
        }
        else
        {
            TRACE0(" -  200 x 100 pels/25.4 mm");
        }
    }
    else if (pStatus->verticalResolutionVR & (VR_R8_77 | VR_200_200))    /* fine resolution (R8 x 7.7 lines/mm) */
    {
        pT30->HDLC_TX_Buf[4] |= BIT1;

        if (isMetric)
        {
            TRACE0(" - R8 x 7.7 lines/mm");
        }
        else
        {
            TRACE0(" - 200 x 200 pels/25.4 mm");
        }
    }

    if (pStatus->dataFormatDF == DF_MH)        /* MH */
    {
        TRACE0(" - One dimensional coding (MH)");
    }
    else if (pStatus->dataFormatDF == DF_MR)        /* MR */
    {
        pT30->HDLC_TX_Buf[4] |= BIT0;
        TRACE0(" - Two dimensional coding (MR)");
    }

    /* 6th  byte */
    /* bit 17 , 18 */
    if (pStatus->pageWidthWD == WD_1)
    {
        pT30->HDLC_TX_Buf[5] |= BIT7;
        TRACE0(" - Scan line length 255 mm +/- 1%%");
    }
    else if (pStatus->pageWidthWD == WD_2)
    {
        pT30->HDLC_TX_Buf[5] |= BIT6;
        TRACE0(" - Scan line length 303 mm +/- 1%%");
    }
    else
    {
        TRACE0(" - Scan line length 215 mm +/- 1%%");
    }

    /* bit 19, 20 */
    if (pStatus->pageLengthLN  == LN_B4)
    {
        pT30->HDLC_TX_Buf[5] |= BIT5;
        TRACE0(" - Recording length A4 (297 mm) and B4 (364 mm)");
    }
    else if (pStatus->pageLengthLN == LN_UNLIMITED)
    {
        pT30->HDLC_TX_Buf[5] |= BIT4;
        TRACE0(" - Recording length Unlimited");
    }
    else
    {
        TRACE0(" - Recording length A4 (297 mm)");
    }


    /* Set minimum scan line time */
    /* Bits 21, 22, 23 */

#if SUPPORT_ECM_MODE

    if (pStatus->errorCorrectionEC != EC_NON_ECM)
    {
        pT30->HDLC_TX_Buf[5] |= 0x0E;    /* 0 ms when ECM is used */
        TRACE0(" - Min. scan line time 00 ms");
    }
    else
#endif
    {
        if (pStatus->verticalResolutionVR == VR_R8_385)
        {
            temp = 0;    /* VR = 0 */
        }
        else
        {
            temp = 1;    /* VR > 0 */
        }

        switch (ST_ScanTime_TAB[temp][pStatus->scanTimeST])
        {
            case 0:        /* scan time 0 ms */
            default:
                pT30->msltBitNum = 0;
                pT30->HDLC_TX_Buf[5] |= 0x0E;
                TRACE0(" - Min. scan line time 00 ms");
                break;
            case 5:        /* scan time 5 ms */
                pT30->msltBitNum = (pStatus->bitRateBR + 1) * 12;
                pT30->HDLC_TX_Buf[5] |= BIT3;
                TRACE0(" - Min. scan line time 05 ms");
                break;
            case 10:    /* scan time 10 ms */
                pT30->msltBitNum = (pStatus->bitRateBR + 1) * 24;
                pT30->HDLC_TX_Buf[5] |= BIT2;
                TRACE0(" - Min. scan line time 10 ms");
                break;
            case 20:    /* scan time 20 ms */
                pT30->msltBitNum = (pStatus->bitRateBR + 1) * 48;
                pT30->HDLC_TX_Buf[5] |= 0x00;
                TRACE0(" - Min. scan line time 20 ms");
                break;
            case 40:    /* scan time 40 ms */
                pT30->msltBitNum = (pStatus->bitRateBR + 1) * 96;
                pT30->HDLC_TX_Buf[5] |= BIT1;
                TRACE0(" - Min. scan line time 40 ms");
                break;
        }
    }

    /* The standard FIF field for DCS is 24 bits long */
    if ((pStatus->errorCorrectionEC == EC_NON_ECM) && // non ECM
        (pStatus->verticalResolutionVR == VR_R8_385 || pStatus->verticalResolutionVR == VR_R8_77) && // low metric resolutions
        !isToSendSUBorSID // neither SUB nor SID
       )
    {
        pT30ToDte->ecmFrmSize = ECM_SIZE_0;
        TRACE0(" - Error correction mode OFF");
        return 6;
    }

    /* bit 24 extended bit */
    pT30->HDLC_TX_Buf[5] |= BIT0;

    /* 7th  byte */
#if SUPPORT_ECM_MODE

    if (pStatus->errorCorrectionEC > EC_NON_ECM)
    {
        pT30->HDLC_TX_Buf[6] |= BIT5;
        TRACE0(" - Error correction mode ON");

#if FRAMESIZE == 64
        pT30->HDLC_TX_Buf[6] |= BIT4;
        TRACE0(" - Frame size 1 = 064 octets");
        pT30ToDte->ecmFrmSize = ECM_SIZE_64;
#else
        TRACE0(" - Frame size 0 = 256 octets");
        pT30ToDte->ecmFrmSize = ECM_SIZE_256;
#endif
    }
    else
    {
        pT30ToDte->ecmFrmSize = ECM_SIZE_0;
        TRACE0(" - Error correction mode OFF");
    }

    if (pStatus->dataFormatDF == DF_MMR)
    {
        pT30->HDLC_TX_Buf[6] |= BIT1;
        TRACE0(" - T.6 coding enabled (MMR)");
    }

#else
    pT30ToDte->ecmFrmSize = ECM_SIZE_0;
    TRACE0(" - Error correction mode OFF");
#endif

    pT30->HDLC_TX_Buf[6] |= BIT0;
    pT30->HDLC_TX_Buf[7] |= BIT0;

    if (pStatus->verticalResolutionVR & VR_R8_154)
    {
        pT30->HDLC_TX_Buf[8] |= BIT7;
        TRACE0(" - R8 x 15.4 lines/mm");
    }
    else if (pStatus->verticalResolutionVR & VR_300_300)
    {
        pT30->HDLC_TX_Buf[8] |= BIT6;
        TRACE0(" - 300 x 300 pels/25.4 mm");
    }
    else if (pStatus->verticalResolutionVR & (VR_R16_154 | VR_400_400))
    {
        pT30->HDLC_TX_Buf[8] |= BIT5;
        TRACE0(" - R16 x 15.4 lines/mm and/or 400 x 400 pels/25.4 mm");
    }

    if (!isToSendSUBorSID && !isJpeg && extraResolutions == 0)
    {
        return 9;
    }

    pT30->HDLC_TX_Buf[8] |= BIT0;

    if (RdReg(pT30->remoteFAPCap, BIT2))
    {
        pT30->HDLC_TX_Buf[9] |= BIT7;
        TRACE0(" - Subaddressing transmission");
    }

    if (RdReg(pT30->remoteFAPCap, BIT0))
    {
        pT30->HDLC_TX_Buf[9] |= BIT6;
        TRACE0(" - Sender Identification transmission");
    }

    if (!isJpeg && extraResolutions == 0)
    {
        return (10);
    }

    pT30->HDLC_TX_Buf[9] |= BIT0;
    pT30->HDLC_TX_Buf[10] |= BIT0;

    if (RdReg(pStatus->jpegOptionJP, JP_ENABLE_JPEG))
    {
        pT30->HDLC_TX_Buf[11] |= BIT4;
        TRACE0(" - JPEG mode used");
    }

    if (RdReg(pStatus->jpegOptionJP, JP_FULL_COLOR))
    {
        pT30->HDLC_TX_Buf[11] |= BIT3;
        TRACE0(" - Full color representation in CIELAB space");
    }

    if (!isCG300DpiOr400Dpi && extraResolutions == 0)
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
    else if (extraResolutions & VR_1200_1200)
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
    else if (extraResolutions & VR_300_600)
    {
        pT30->HDLC_TX_Buf[16] |= BIT5;
        TRACE0(" - 300 x 600 pels/25.4 mm");
    }
    else if (extraResolutions & VR_400_800)
    {
        pT30->HDLC_TX_Buf[16] |= BIT4;
        TRACE0(" - 400 x 800 pels/25.4 mm");
    }
    else if (extraResolutions & VR_600_1200)
    {
        pT30->HDLC_TX_Buf[16] |= BIT3;
        TRACE0(" - 600 x 1200 pels/25.4 mm");
    }

    return (17);
}
#endif
