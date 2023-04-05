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

/*
 * ACE command processing for EC control
 *
 * COMMAND SUPPORTED:
 *  AT+EC=0 - Disable EC.
 *  AT+EC=1 - Enable/Start EC.
 *  AT+EC=2 - restart EC training/adaptation
 *            (reset all filter coefs if not previously freezed))
 *  AT+EC=3 - freeze/resume EC training (no update of filter coefs).
 *
 *  AT+EWIND? - Get V.42 transmit (kTx) and receive (kRx) window size
 *  AT+EWIND=? - Get V.42 transmit (kTx) and receive (kRx) window size
 *  AT+EWIND=<kTx>,<kRx> - Set V.42 transmit (kTx) and receive (kRx) window size
 */

#include "aceext.h"
#include "common.h"

#if SUPPORT_V42
#include "v42ext.h"
#endif

#if SUPPORT_PARSER
UBYTE AT_plusE(UBYTE **pTable)
{
    UBYTE EcType;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    ACEStruct   *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
#if V42_IMPLEMENTED
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE      *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    int result;
    UBYTE stringBuf[V42_PARAM_STRING_LEN];
#endif
    UBYTE isErr = FALSE, isEnd = FALSE;

    /* Look for ID command after +E */
#if V42_IMPLEMENTED

    if ((pAce->CurrCommand[pAce->ATidx    ] == 'W')
        && (pAce->CurrCommand[pAce->ATidx + 1] == 'I')
        && (pAce->CurrCommand[pAce->ATidx + 2] == 'N')
        && (pAce->CurrCommand[pAce->ATidx + 3] == 'D'))
    {
        pAce->ATidx += 4;

        if (pAce->CurrCommand[pAce->ATidx] == '=')
        {
            pAce->ATidx++;

            if (pAce->CurrCommand[pAce->ATidx] == '?')
            {
                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                V42_ParamGetWinSizeRange(&(pAce->m_V42Parameter), stringBuf);
                PutStringToCB(DteRd, stringBuf, (UWORD)strlen(stringBuf));

                AceSetResponse(pTable, ACE_OK);
                return SUCCESS;
            }
            else
            {
                result = V42_ParamSetWinSize(&(pAce->m_V42Parameter), pAce->CurrCommand, &(pAce->ATidx));

                if (V42_PARAM_OK == result)
                {
                    pAce->ATidx--;
                    AceSetResponse(pTable, ACE_OK);
                    return SUCCESS;
                }
            }
        }
        else if (pAce->CurrCommand[pAce->ATidx] == '?')
        {
            PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
            PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

            V42_ParamGetWinSizeValue(&(pAce->m_V42Parameter), stringBuf);
            PutStringToCB(DteRd, stringBuf, (UWORD)strlen(stringBuf));

            AceSetResponse(pTable, ACE_OK);
            return SUCCESS;
        }

        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

#endif

    if (pAce->CurrCommand[pAce->ATidx] != 'C')
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    /* Look for "=" or "?" */
    pAce->ATidx++;

    if (pAce->CurrCommand[pAce->ATidx] == '?')
    {
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }
    else if (pAce->CurrCommand[pAce->ATidx] != '=')
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    /* Read value */
    pAce->ATidx++;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &EcType, &isEnd, FALSE, 0))
    {
        if ((EcType == 0xFF) || (EcType > 3))
        {
            isErr = TRUE;
        }
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

    switch (EcType)
    {
        case 0: ResetReg(pDpcs->LECR, LEC_ENABLE);  /* disable */
            ResetReg(pDpcs->AECR, AEC_ENABLE);  /* disable */
            PutReg(pDpcs->MCF,    DISCONNECT);  /* If running, stop it */
            break;

        case 1: SetReg(pDpcs->LECR, LEC_ENABLE);    /* Enable */
            PutReg(pDpcs->MCF,  CONNECT);       /* Start LEC */
            break;                              /*** same for AEC ***/

        case 2: SetReg(pDpcs->LECR, LEC_RESTART);   /* Enable */
            SetReg(pDpcs->AECR, AEC_RESTART);   /* Enable */
            PutReg(pDpcs->MCF,  CONNECT);       /* Start LEC */
            break;

        case 3: SetReg(pDpcs->LECR, LEC_FREEZE);    /* Enable */
            SetReg(pDpcs->AECR, AEC_FREEZE);    /* Enable */
            PutReg(pDpcs->MCF,  CONNECT);       /* Start LEC */
            break;
    }

    AceSetResponse(pTable, ACE_OK);

    return SUCCESS;
}
#endif
