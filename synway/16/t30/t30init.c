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
#include <stdlib.h>
#include "t30ext.h"
#include "v251.h"

#if SUPPORT_T30
#if 0
CONST UBYTE ID[OPTFRAMELEN] =
{
    0x2B, 0x31, 0x20, 0x34, 0x31, 0x36, 0x32, 0x39, 0x32, 0x30,
    0x30, 0x33, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};
#endif

/* ----------------------- */
/*  T.30 initialization    */
/* ----------------------- */
void T30_Init(T30Struct *pT30)
{
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_ECM_MODE
    T4ECMstruc *pT4 = &pT30->T4ECM;

#if PC_ECM
    memset((UBYTE *)&pT4->pblock, 0, sizeof(T30Struct) - sizeof(pT4->ubECM_Block));
#else
    memset(pT30, 0, sizeof(T30Struct));
#endif
#else
    memset(pT30, 0, sizeof(T30Struct));
#endif

    pT30->pDteToT30 = pDteToT30;
    pT30->pDceToT30 = pDceToT30;
    pT30->pT30ToDte = pT30ToDte;
    pT30->pT30ToDce = pT30ToDce;

    memset(&pT30ToDte->faxParmStatus, 0, sizeof(pT30ToDte->faxParmStatus)); // fixing bug 19, need to get rid of the information of the previous call
    T30_InitT32Parms(pDteToT30);
    T30_UpdateT31Parms(pT30);

#if SUPPORT_V34FAX
    pT30ToDce->pIsV34Enabled = &pDteToT30->isV34Enabled;
    pT30ToDte->isV34Capable = TRUE;
#if DCE_CONTROLLED_V8
    pT30ToDce->v8OpCntl = A8E_V8O_6 | A8E_V8A_5;
#else
    pT30ToDce->v8OpCntl = A8E_V8O_3 | A8E_V8A_2;
#endif
#else
    pT30ToDte->isV34Capable = FALSE;
#endif

    pT30->pfT30Vec = T30_PhaseA;

    pT30->T30Speed = BR_V21_300;
    pT30->hdlcTypeRx = T30MESNULL;

    TRACE0("T30: ****** START ******");

#if SUPPORT_ECM_MODE
    {
#if PC_ECM
        TRACE0("T30: DYNAMICALLY");

        if ((pT4->pblock = malloc(BLOCKSIZE)) == NULL)
        {
            TRACE0("T30: ERROR. Malloc can not assign memory in ECM mode");
            pT4->pblock = (UBYTE *) - 1;
            /*    0: without ECM         OK */
            /* else: with    ECM, malloc OK */
            /*   -1: with    ECM, malloc ERROR */
        }

#else
        TRACE0("T30: STATICALLY");

        pT4->pblock = pT4->pRead = pT4->pWrite = pT4->pLast_EOL = pT4->ubECM_Block;
        pT4->FCF2 = FCF2_INIT; // fixing issue 158
#endif
    }
#endif
}

void T30_UpdateT31Parms(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;

#if SUPPORT_V34FAX

    if (pDteToT30->isV34Enabled)
    {
        pT30ToDce->initV34PrimRateMax = pDteToT30->faxParmSettings.bitRateBR + 1;
        pT30ToDce->initV34PrimRateMin = pDteToT30->minPhaseCSpd + 1;
        pT30ToDce->initV34CntlRate = 1;
    }
    else
#endif
    {
        pT30ToDce->carrierLossTimeout = 2; // 20 ms
    }
}

void T30_InitT32Parms(DteToT30Interface *pDteToT30)
{
    // +FCLASS
    //pDteToT30->isV34Enabled = FALSE;

    // +FCC
    pDteToT30->faxParmCapability.verticalResolutionVR = VR_R8_385 | VR_R8_77 | VR_R8_154 | VR_R16_154 | VR_200_100 | VR_200_200 | VR_200_400;
#if SUPPORT_V34FAX
    pDteToT30->faxParmCapability.bitRateBR = BR_33600;
#else
    pDteToT30->faxParmCapability.bitRateBR = BR_14400;
#endif
    pDteToT30->faxParmCapability.pageWidthWD = WD_0;
    pDteToT30->faxParmCapability.pageLengthLN = LN_UNLIMITED;
    pDteToT30->faxParmCapability.dataFormatDF = DF_MMR;
#if SUPPORT_ECM_MODE
    pDteToT30->faxParmCapability.errorCorrectionEC = EC_ECM;
#else
    pDteToT30->faxParmCapability.errorCorrectionEC = EC_NON_ECM;
#endif
    pDteToT30->faxParmCapability.fileTransferBF = BF_NON_FILE_TRANSFER;
    pDteToT30->faxParmCapability.scanTimeST = ST_0;
    pDteToT30->faxParmCapability.jpegOptionJP = JP_ENABLE_JPEG | JP_FULL_COLOR; // jpeg color and gray

    // +FIS
    memcpy(&pDteToT30->faxParmSettings, &pDteToT30->faxParmCapability, sizeof(pDteToT30->faxParmCapability));

    // +FLI
    memset(pDteToT30->localID, 0, sizeof(pDteToT30->localID));

    // +FPI
    memset(pDteToT30->localPollingID, 0, sizeof(pDteToT30->localPollingID));

    // +FNS
    memset(pDteToT30->nonStandardFrm, 0, sizeof(pDteToT30->nonStandardFrm));

    // +FLP
    pDteToT30->isDocForPolling = FALSE;

    // +FSP
    pDteToT30->isRequestingToPoll = FALSE;

    // +FCR
    pDteToT30->isCapToRcv = TRUE;

    // +FBU
    pDteToT30->isHdlcRepEnabled = FALSE;

    // +FNR
    SetReg(pDteToT30->negoReporting, FNR_RPR | FNR_TPR | FNR_IDR); /// LLL temp

    // +FAP
    pDteToT30->addressPollingCap = 0;

    // +FSA
    memset(pDteToT30->destSubAddr, 0, sizeof(pDteToT30->destSubAddr));

    // +FPA
    memset(pDteToT30->selectivePollingAddr, 0, sizeof(pDteToT30->selectivePollingAddr));

    // +FPW
    memset(pDteToT30->passWord, 0, sizeof(pDteToT30->passWord));

    // +FFD, +FIE, +FAA, +FCT, +FND, +FIT, +FPP, +FEA, +FFC are not supported

    // +FPS is set elsewhere

    // +FCQ
    pDteToT30->copyQtyChk = 0; /// LLL temp

    // +FRQ
    pDteToT30->rcvQtyPglThrh = 0;
    pDteToT30->rcvQtyPglThrh = 0;

    // +FRY
    pDteToT30->ecmRetryCnt = 0;

    // +FMS
    pDteToT30->minPhaseCSpd = 0;

    // +FBO
    pDteToT30->dataBitOrder = DIRECT_C_DIRECT_BD;
}

void T30_InitForPhaseBReEntry(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;

    pT30->wasDisRcved = FALSE;
    memset(&pT30ToDte->remoteSettings, 0, sizeof(pT30ToDte->remoteSettings));
    memcpy(&pDteToT30->faxParmSettings, &pDteToT30->faxParmCapability, sizeof(pDteToT30->faxParmCapability));
    pT30->Timer_T1_Counter = 1; TRACE0("T30: Go back to Phase B. T1 starts.");
}
#endif
