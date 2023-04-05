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

#if SUPPORT_PARSER
/* for command Sn?, read the S reg */
static UBYTE AceReadValue(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE Reg, TempStr[7], s_UnitsPlace[3];
    UBYTE isEnd = FALSE;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &Reg, &isEnd, FALSE, 0))
    {
        if (Reg == 0 || (Reg >= 2 && Reg <= 8) || (Reg >= 10 && Reg <= 13) || Reg == 18 || Reg == 21 || Reg == 23 || Reg == 30 || Reg == 33 || Reg == 34 || Reg == 35 || Reg == 36)
        {
            pAce->ATidx++;

            Number100_10_1(pSreg[Reg], s_UnitsPlace);

            TempStr[0] = pSreg[CR_CHARACTER];
            TempStr[1] = pSreg[LF_CHARACTER];
            TempStr[2] = (UBYTE)(s_UnitsPlace[0] + '0');
            TempStr[3] = (UBYTE)(s_UnitsPlace[1] + '0');
            TempStr[4] = (UBYTE)(s_UnitsPlace[2] + '0');
            TempStr[5] = pSreg[CR_CHARACTER];
            TempStr[6] = pSreg[LF_CHARACTER];

            PutStringToCB(DteRd, TempStr, 7);

            AceSetResponse(pTable, ACE_OK);
            return SUCCESS;
        }
    }

    AceSetResponse(pTable, ACE_ERROR);

    return FAIL;
}

/* for command Sn=xxx */
static UBYTE AceWriteValue(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    UBYTE Reg, max_sym_rate, min_sym_rate;
    UBYTE uValue;
    UWORD Value = 0xFFFF;
    UBYTE isEnd = FALSE;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &Reg, &isEnd, FALSE, 0))
    {
        pAce->ATidx += 2;

        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &uValue, &isEnd, FALSE, 0))
        {
            switch (Reg)
            {
                case AUTO_ANS_RING_NUMBER:
                case COMMA_DIAL_PAUSE_TIME:
#if SUPPORT_V54
                case V54_SELF_TIMER:
#endif
                    Value = uValue;

                    if (Reg == AUTO_ANS_RING_NUMBER)
                    {
                        pAce->RingCount = 0;
                    }

                    break;
                case AT_ESCAPE_CHARACTER:
                case CR_CHARACTER:
                case LF_CHARACTER:
                case BS_CHARACTER:

                    if (uValue <= 127)
                    {
                        Value = uValue;
                    }

                    break;
                case BLIND_DIAL_PAUSE_TIME:

                    if (uValue <= 10 && uValue >= 2)
                    {
                        Value = uValue;
                    }

                    break;
                case NO_ANSWER_TIMEOUT:
                case DIAL_TONE_DET_TIMEOUT:

                    if (uValue >= 1)
                    {
                        Value = uValue;
                    }

                    break;
                case NO_CARRIER_DISCONNECT:

                    if (uValue <= 254 && uValue >= 1)
                    {
                        Value = uValue;
                    }

                    break;
                case TONE_DIAL_LENGTH:

                    if (uValue >= 50)
                    {
                        Value = uValue;
                    }

                    break;
                case ESCAPE_GUARD_TIME:

                    if (uValue <= 254 && uValue >= 20)
                    {
                        Value = uValue;
                    }

                    break;
                case AUTO_RETRAIN_CONTROL:

                    if (uValue == 0 || uValue == 2)
                    {
                        Value = uValue;

                        if (Value & 0x02)/* BIT1 of Sreg21 for auto retrain enable */
                        {
                            SetReg(pDpcs->MCR0, RTRND);
                        }
                        else
                        {
                            ResetReg(pDpcs->MCR0, RTRND);
                        }
                    }

                    break;
#if SUPPORT_V54
                case V54_RESPONSE_CONTROL:          /* parity & digital loopback */

                    if (uValue == 0 || uValue == 1)
                    {
                        Value = uValue;

                        if (Value & 1)
                        {
                            SetReg(pDpcs->MCR0, RDLENA);
                        }
                        else
                        {
                            ResetReg(pDpcs->MCR0, RDLENA);
                        }
                    }

                    break;
#endif
                case V90D_PCM_CODING:

                    if (uValue <= 1)
                    {
                        Value = uValue;
                    }

                    break;
                case V34_SYMBOL_RATE:
                    Value = uValue;

                    max_sym_rate = (UBYTE)(Value & 0x07);
                    min_sym_rate = (UBYTE)((Value >> 3) & 0x07);

                    if ((max_sym_rate > 5) || (min_sym_rate > 5) || (max_sym_rate < min_sym_rate))
                    {
                        break;
                    }

                    PutReg(pDpcs->MBR, (UBYTE)Value);

                    break;
                case TRANSMIT_POWER:

                    if (uValue <= 23)
                    {
                        Value = uValue;
                    }

                    break;
                case CP_COUNTRY_CODE:

                    if (uValue > 0 && uValue <= 12)
                    {
                        Value = uValue;
                    }

                    break;
                case DTMF_TRANSMIT_POWER:

                    if (uValue >= 5 && uValue <= 18)
                    {
                        Value = uValue;
                    }

                    break;
                case V92_OPTIONS:

                    if (uValue <= 15)
                    {
                        Value = uValue;
                    }

                    break;
                case DOWNSTREAM_POWER_INDEX:

                    if (uValue <= 3)
                    {
                        Value = uValue;
                    }

                    break;
                    /* case 50 to 63 reserve for driver */
                case DRIVER_PARAM1:

                    if ((uValue >= 7) && (uValue <= 14))
                    {
                        Value = uValue;
                    }

                    break;
                case DRIVER_PARAM2:

                    if ((uValue >= 7) && (uValue <= 14))
                    {
                        Value = uValue;
                    }

                    break;
                case DRIVER_PARAM3:

                    if (uValue <= 47)
                    {
                        Value = uValue;
                    }

                    break;
                case DRIVER_PARAM4:

                    if (uValue <= 47)
                    {
                        Value = uValue;
                    }

                    break;
            }
        }
    }

    if (Value <= 255)
    {
        pSreg[Reg] = (UBYTE)Value;

        AceSetResponse(pTable, ACE_OK);

        return SUCCESS;
    }
    else
    {
        AceSetResponse(pTable, ACE_ERROR);

        return FAIL;
    }
}

UBYTE AT_S(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (COMM_IsNumber(pAce->CurrCommand[pAce->ATidx+1]))
    {
        pAce->ATidx++;

        // 1 or 2 digits
        if (pAce->CurrCommand[pAce->ATidx+1] == '=' || pAce->CurrCommand[pAce->ATidx+2] == '=')
        {
            return (AceWriteValue(pTable));
        }

        if ((pAce->CurrCommand[pAce->ATidx+1] == '?') || (pAce->CurrCommand[pAce->ATidx+2] == '?'))
        {
            return (AceReadValue(pTable));
        }
    }

    AceSetResponse(pTable, ACE_ERROR);

    return FAIL;
}
#endif
