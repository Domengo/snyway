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

#include "aceext.h"
#include "common.h"
#include "v251.h"

#if SUPPORT_PARSER && SUPPORT_FAX
UBYTE AT_plusFCLASS(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE TempStr[20], i = 0;
#if SUPPORT_T32_PARSER
    UBYTE *pIsV34Enabled = &pAce->pDteToT30->isV34Enabled;
#else
    UBYTE *pIsV34Enabled = &pAce->isV34Enabled;
#endif

    /* for command: +fclass?, read current service class setting */
    switch (pAce->CurrCommand[pAce->ATidx++])
    {
        case '?':
            TempStr[i++] = pSreg[CR_CHARACTER];
            TempStr[i++] = pSreg[LF_CHARACTER];

#if SUPPORT_T32_PARSER

            if (pAce->FaxClassType == FCLASS21)
            {
                TempStr[i++] = '2';
                TempStr[i++] = '.';
                TempStr[i++] = '1';
            }
            else if (pAce->FaxClassType == FCLASS20)
            {
                TempStr[i++] = '2';
                TempStr[i++] = '.';
                TempStr[i++] = '0';
            }
            else if (pAce->FaxClassType == FCLASS2)
            {
                TempStr[i++] = '2';
            }
            else
#endif
                if (pAce->FaxClassType == FCLASS10)
                {
                    TempStr[i++] = '1';
                    TempStr[i++] = '.';
                    TempStr[i++] = '0';
                }
                else if (pAce->FaxClassType == FCLASS1)
                {
                    TempStr[i++] = '1';
                }
                else if (pAce->FaxClassType == FCLASS0)
                {
                    TempStr[i++] = '0';
                }

            TempStr[i++] = pSreg[CR_CHARACTER];
            TempStr[i++] = pSreg[LF_CHARACTER];
            PutStringToCB(DteRd, TempStr, i);

            return SUCCESS;
        case '=':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case '?':
                    /* for command: +fclass=?, test/detect */
                    TempStr[i++] = pSreg[CR_CHARACTER];
                    TempStr[i++] = pSreg[LF_CHARACTER];
                    TempStr[i++] = '0';
                    TempStr[i++] = ',';
                    TempStr[i++] = '1';
                    TempStr[i++] = ',';
                    TempStr[i++] = '1';
                    TempStr[i++] = '.';
                    TempStr[i++] = '0';

#if SUPPORT_T32_PARSER
                    TempStr[i++] = ',';
                    TempStr[i++] = '2';
                    TempStr[i++] = '.';
                    TempStr[i++] = '0';
                    TempStr[i++] = ',';
                    TempStr[i++] = '2';
                    TempStr[i++] = '.';
                    TempStr[i++] = '1';
#endif
                    TempStr[i++] = pSreg[CR_CHARACTER];
                    TempStr[i++] = pSreg[LF_CHARACTER];
                    PutStringToCB(DteRd, TempStr, i);
                    return SUCCESS;
                case '0':
                    pAce->FaxClassType = FCLASS0;
#if SUPPORT_V34FAX
                    *pIsV34Enabled = FALSE;
#endif
                    ResetReg(pDpcs->MCR0, FAX);    /* FAX mode */
                    return SUCCESS;
#if SUPPORT_T31_PARSER
                case '1':
#if SUPPORT_V34FAX

                    /* support Class 1.0 */
                    if ((pAce->CurrCommand[pAce->ATidx] == '.')
                        && (pAce->CurrCommand[pAce->ATidx + 1] == '0')
                        && !COMM_IsNumber(pAce->CurrCommand[pAce->ATidx + 2]))
                    {
                        pAce->FaxClassType = FCLASS10;
                        pAce->ATidx += 2;
                        return SUCCESS;
                    }
                    else
#endif
                        if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx + 1]))
                        {
                            pAce->FaxClassType = FCLASS1;
#if SUPPORT_V34FAX
                            *pIsV34Enabled = FALSE;
#endif
                            return SUCCESS;
                        }

                    break;
#endif
#if SUPPORT_T32_PARSER
                case '2':

                    if (pAce->CurrCommand[pAce->ATidx++] == '.')
                    {
                        switch (pAce->CurrCommand[pAce->ATidx++])
                        {
                            case '0':
#if SUPPORT_V34FAX
                                *pIsV34Enabled = FALSE;
#endif
                                pAce->FaxClassType = FCLASS20;
                                pAce->V8Detect_State = V8_PASS_V21;
                                return SUCCESS;
#if SUPPORT_V34FAX
                            case '1':
                                pAce->FaxClassType = FCLASS21;
                                *pIsV34Enabled = TRUE;
                                pAce->pT30ToDce->initV34CntlRate = 1; // always 1200
#if DCE_CONTROLLED_V8
                                pAce->pT30ToDce->v8OpCntl = A8E_V8O_6 | A8E_V8A_5;
#else
                                pAce->pT30ToDce->v8OpCntl = A8E_V8O_3 | A8E_V8A_2;
#endif
                                return SUCCESS;
#endif
                        }
                    }

                    break;
#endif
                case '8':
                    pAce->FaxClassType = FCLASS10;
                    return SUCCESS;
            }
    }

    return FAIL;
}
#endif

