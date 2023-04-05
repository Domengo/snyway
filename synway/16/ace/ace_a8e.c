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
#include "v251.h"
#include "common.h"
#include "mhsp.h"
#include "gaoapi.h"

#if SUPPORT_V251 && SUPPORT_V8 && SUPPORT_T31 && SUPPORT_V34FAX
void Ace_FsmA8E(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    MhspStruct *pMhsp = (MhspStruct *)pTable[MHSP_STRUC_IDX];
    UBYTE isOffline = pAce->LineState == OFF_LINE;
    UBYTE isIdle = pMhsp->pfRealTime == MhspIdle;
    UBYTE isV8Enabled = pT30ToDce->v8OpCntl != A8E_V8O_0;
    UBYTE mode = RdReg(pDpcs->MSR0, DISC_DET);

    if (isV8Enabled)
    {
        pDpcs->V8OPCNTL = pT30ToDce->v8OpCntl;
    }
    else
    {
        isV8Enabled = pDpcs->V8OPCNTL != A8E_NULL;
    }

    if (isOffline)
    {
        Ace_ChangeState(pAce, ACE_NOF);
    }
    else
    {
        if (isIdle)
        {
            if (mode)
            {
                /* clear DISC_DET bit */
                ClrReg(pDpcs->MSR0);
            }

            if (RdReg(pDpcs->MCR0, ANSWER))
            {
                SetReg(pDpcs->MCR0, RECEIVE_IMAGE);
            }
            else
            {
                ResetReg(pDpcs->MCR0, RECEIVE_IMAGE);
            }

            MhostConnect(pTable);

            if (isV8Enabled)
            {
                ResetReg(pDpcs->MMR0, CP);
                WrReg(pDpcs->MBC0, MIN_RATE, (pT30ToDce->initV34PrimRateMin + 3));
                WrReg(pDpcs->MBC1, MAX_RATE, (pT30ToDce->initV34PrimRateMax + 3));
            }

            Ace_ChangeState(pAce, ACE_NOF);
        }
        else
        {
            PutReg(pDpcs->MCF, DISCONNECT);
        }
    }

    pT30ToDce->v8OpCntl = A8E_V8O_0;
}

#if SUPPORT_PARSER
UBYTE AT_A8E(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE isErr = FALSE, isEnd = FALSE;
    UBYTE v8OpCntl = pDpcs->V8OPCNTL;
#if SUPPORT_CI
    UBYTE v8cf = pDpcs->V8CF == 0x00 ? 0x81 : pDpcs->V8CF;
#endif

    if (pAce->CurrCommand[pAce->ATidx] == '=')
    {
        pAce->ATidx++;

        if (pAce->CurrCommand[pAce->ATidx] == '?')
        {
            AceSetResponse(pTable, ACE_A8E_CAP);
            isEnd = TRUE;
        }
        else if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &v8OpCntl, &isEnd, FALSE, A8E_V8O_1))
        {
            if (v8OpCntl > A8E_V8O_6
#if !SUPPORT_CI
                || v8OpCntl == A8E_V8O_2
#endif
#if !SUPPORT_CT
                || v8OpCntl == A8E_V8O_4
#endif
               )
            {
                isErr = TRUE;
            }
        }
        else
        {
            isErr = TRUE;
        }

        if (!(isErr || isEnd))
        {
            UBYTE v8a;

            if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &v8a, &isEnd, FALSE, A8E_V8A_1 >> 4))
            {
                v8a <<= 4;

                if (v8a > A8E_V8A_5)
                {
                    isErr = TRUE;
                }
                else
                {
                    v8OpCntl |= v8a;
                }
            }
            else
            {
                isErr = TRUE;
            }

            if (!(isErr || isEnd))
            {
#if SUPPORT_CI

                if (!COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &v8cf, &isEnd, TRUE, 0x81))
                {
                    isErr = TRUE;
                }

#else
                isErr = TRUE;
#endif
            }
        }
    }
    else if (pAce->CurrCommand[pAce->ATidx] == '?')
    {
        UBYTE tempStr[20], length = 0;

        tempStr[length++] = pSreg[CR_CHARACTER];
        tempStr[length++] = pSreg[LF_CHARACTER];
        tempStr[length++] = (v8OpCntl & 0xF) + '0';
        tempStr[length++] = ',';
        tempStr[length++] = ((v8OpCntl >> 4) & 0xF) + '0';
        tempStr[length++] = ',';
        tempStr[length++] = '0';
        tempStr[length++] = '0';
        tempStr[length++] = ',';
        tempStr[length++] = '0';
        tempStr[length++] = pSreg[CR_CHARACTER];
        tempStr[length++] = pSreg[LF_CHARACTER];
        PutStringToCB(DteRd, tempStr, length);
    }
    else
    {
        isErr = TRUE;
    }

    if (isErr)
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    if ((v8OpCntl & 0xF0) == A8E_V8A_4)
    {
        v8OpCntl = A8E_V8O_0;
    }

    pAce->pT30ToDce->v8OpCntl = v8OpCntl;

#if SUPPORT_CI
    pDpcs->V8CF = v8cf;
#endif

    AceSetResponse(pTable, ACE_OK);

    return SUCCESS;
}

#endif

#endif
