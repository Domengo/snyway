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

#include "mhsp.h"
#include "aceext.h"
#include "common.h"
#if SUPPORT_V54
#include "v54.h"
#include "gaoapi.h"
#endif

#if SUPPORT_PARSER
#if DUMPSIMDATA
UBYTE ATandF_Flag = 0;
#endif

UBYTE AT_and(UBYTE **pTable)
{
    UBYTE State;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    pAce->ATidx++;

    switch (pAce->CurrCommand[pAce->ATidx])
    {
        case 'F':
            State = AT_andF(pTable);
            break;
        case 'D':
            State = AT_andD(pTable);
            break;
        case 'C':
            State = AT_andC(pTable);
            break;
        case 'H':
            State = AT_andH(pTable);
            break;
        case 'N':
            State = AT_andN(pTable);
            break;
#if (SUPPORT_V42 || SUPPORT_MNP || SUPPORT_FAX)
        case 'K':
            State = AT_andK(pTable);
            break;
#endif
        case 'T':
            State = AT_andT(pTable);
            break;
        case 'U':
            State = AT_andU(pTable);
            break;
            /* result code: "ERROR") */
        default:
            AceSetResponse(pTable, ACE_ERROR);

            return FAIL;
    }

    return State;
}

UBYTE AT_andF(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

#if DUMPSIMDATA

    if (ATandF_Flag == 0)
    {
        ATandF_Flag = 1;
    }

#endif

    if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx+1]) || ((pAce->CurrCommand[pAce->ATidx+1]) == '0'))
    {
        Reset_Init(pTable);

        if ((pAce->CurrCommand[pAce->ATidx+1]) == '0')
        {
            pAce->ATidx++;
        }

        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);

        return SUCCESS;
    }
    else
    {
        /* set result code: "ERROR") */
        AceSetResponse(pTable, ACE_ERROR);

        return FAIL;
    }
}

UBYTE AT_andN(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE BitRate, rate;
    UBYTE isEnd = FALSE, isErr = FALSE;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    pAce->ATidx++;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &BitRate, &isEnd, FALSE, 0))
    {
        rate = RdReg(pDpcs->MBC0, MIN_RATE);/* check minum speed and max speed. */

        if ((BitRate > 39) || (rate > BitRate))
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

    WrReg(pDpcs->MBC1, MAX_RATE, BitRate);

    AceSetResponse(pTable, ACE_OK);/* result code: "OK") */

    return SUCCESS;
}

UBYTE AT_andH(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE protocolNum;
    UBYTE isEnd = FALSE, isErr = FALSE;

    pAce->ATidx++;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &protocolNum, &isEnd, FALSE, 0))
    {
        if (!(protocolNum == 3 || protocolNum == 4))/* AT&H3, AT&H4 by VentaFax */
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

    AceSetResponse(pTable, ACE_OK);
    return SUCCESS;
}

#if (SUPPORT_V42 || SUPPORT_MNP || SUPPORT_FAX)

UBYTE AT_andK(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE protocolNum;
    UBYTE isEnd = FALSE, isErr = FALSE;

    pAce->ATidx++;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &protocolNum, &isEnd, FALSE, 0))
    {
        if (protocolNum == 0)
        {
            ResetReg(pDpcs->MCR1, MNP | V42BIS | V42);
        }

#if SUPPORT_V42
        else if (protocolNum == 1)
        {
            SetReg(pDpcs->MCR1, V42);
        }

#if SUPPORT_V42BIS
        else if (protocolNum == 2)
        {
            SetReg(pDpcs->MCR1, V42 | V42BIS);
        }

#endif
#endif/* Support_V42 */
#if SUPPORT_MNP
        else if (protocolNum == 3)/* MNP4 */
        {
            SetReg(pDpcs->MCR1, MNP);
        }

#endif
#if SUPPORT_FAX
        else if (protocolNum == 4)/* AT&K4+FCLASS=1.0 by Venda Fax */
        {
        }

#endif
        else
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

    AceSetResponse(pTable, ACE_OK);
    return SUCCESS;
}
#endif

UBYTE AT_andU(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE BitRate, rate;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isEnd = FALSE, isErr = FALSE;

    pAce->ATidx++;

    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &BitRate, &isEnd, FALSE, 0))
    {
        rate = RdReg(pDpcs->MBC1, MAX_RATE);/* check minum speed and max speed. */

        if ((BitRate > 39) || (rate < BitRate && rate > 0))
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

    WrReg(pDpcs->MBC0, MIN_RATE, BitRate);

    AceSetResponse(pTable, ACE_OK);

    return SUCCESS;
}

/* ----------------------  A T & T n ---------------------- */

#if SUPPORT_V54
UBYTE ReportSelfTestErr(UBYTE **pTable, SWORD err_cnt)
{
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE *pSreg    = (UBYTE *)pTable[ACESREGDATA_IDX];
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE TempStr[5], err_cnt_UnitsPlace[3];

    pAce->ubSelfTest    = SELFTEST_OFF;
    pAce->ubSelfTest_T0 = ANDT0_UNSELECTED;
    pDpcs->MCR1 = pAce->ubMCR1_BUFFER;

    if (pAce->ubRecv_Sync == 0) /***** Didn't do anything Self Test*****/
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }

    TempStr[0] = pSreg[CR_CHARACTER];
    TempStr[1] = pSreg[LF_CHARACTER];

    if (err_cnt == 0)
    {
        TempStr[2] = 0x30;
        TempStr[3] = 0x30;
        TempStr[4] = 0x30;

        PutStringToCB(DteRd, TempStr, 5);

        /* Added by Chen at 2001/8/20 */
        pAce->CurrCommand[pAce->ATidx+1] = 0;

        pAce->Mandate = 1;
        AceSetResponse(pTable, ACE_OK);

        return SUCCESS;
    }
    else
    {
        if (err_cnt > 999)
        {
            err_cnt = 999;
        }

        Number100_10_1(err_cnt, err_cnt_UnitsPlace);

        TempStr[2] = (UBYTE)(err_cnt_UnitsPlace[0] + 0x30);
        TempStr[3] = (UBYTE)(err_cnt_UnitsPlace[1] + 0x30);
        TempStr[4] = (UBYTE)(err_cnt_UnitsPlace[2] + 0x30);

        PutStringToCB(DteRd, TempStr, 5);

        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
}

void InitSelfTest(ACEStruct *pAce)      /* mainly for pattern test reporting */
{
    pAce->uSelfTestErrorCount = 0;
    pAce->uDataSendCount      = 0;
    pAce->ubDataPosition      = 0;
    pAce->ubRecv_Sync         = 0;
    pAce->ubPre_Receive_data  = 0;
}
#endif

UBYTE AT_andT(UBYTE **pTable)
{
#if SUPPORT_V54
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    UBYTE CommVal, modType, modType1;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UWORD *pClk = (UWORD *)pTable[CLOCKDATA_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE is_test_1_8, is_test_6_7;

    CommVal = (UBYTE)(pAce->CurrCommand[pAce->ATidx+1] - '0');

    if ((CommVal == 0) || (CommVal == 1) || ((CommVal >= 4) && (CommVal <= 8)))
    {
        pAce->ATidx++;

        /* S16, S23 */
        switch (CommVal)
        {
            case 6: /* -- remote digital loop back -- */
            case 7: /***** Remote Self Test *****/
                /* offline always error */
                if (pAce->LineState != ONLINE_COMMAND || pAce->ubSelfTest != SELFTEST_OFF)
                {
                    AceSetResponse(pTable, ACE_ERROR);
                    TRACE1("pAce->ubSelfTest=%d", pAce->ubSelfTest);
                    return FAIL;
                }

                /* online:  MNP/V42/V42bis, V21 / V90 are error */
                modType  = RdReg(pDpcs->MMSR0, MODULATION);
                modType1 = RdReg(pDpcs->MCR1, MNP | V42BIS | V42);

                if ((modType == MODEM_V21) || (modType == MODEM_V90) || modType1)
                {
                    AceSetResponse(pTable, ACE_ERROR);
                    TRACE1("modType=%x", modType);
                    TRACE1("modType1=%x", modType1);
                    return FAIL;
                }

                InitSelfTest(pAce);

                if (pSreg[V54_SELF_TIMER] != 0)
                {
                    pAce->StartTime = *pClk;
                }

                pAce->ubSelfTest = SELF_TEST_6;

                if (CommVal == 6)
                {
                    SetReg(pSreg[V54_SELF_TEST], SREG_RDL);
                    Ace_ChangeLineState(pAce, ONLINE_DATA);
                }
                else
                {
                    SetReg(pSreg[V54_SELF_TEST], SREG_RDL_PATT);
                    PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                    PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
                    Ace_ChangeLineState(pAce, ONLINE_COMMAND);
                }

                SetReg(pDpcs->MTC,  RDL_MODE);

                SetReg(pDpcs->MCR0, TEST_MODE);

                /* No V.42 in loop-back and init to ASC */
                pAce->ubMCR1_BUFFER = pDpcs->MCR1;

                V54_Init(pTable);
                V54_StartRDL(pTable);
                break;

            case 8:
            case 1:

                /***** Self Test *****/
                /* online always error */
                if (pAce->LineState != OFF_LINE || pAce->ubSelfTest != SELFTEST_OFF)
                {
                    AceSetResponse(pTable, ACE_ERROR);
                    TRACE0("pAce->LineState != OFF_LINE || pAce->ubSelfTest != SELFTEST_OFF");
                    return FAIL;
                }

#if SUPPORT_MODEM
                /* offline: MNP\V42\V42bis, V90, AUTO are error */
                modType1 = RdReg(pDpcs->MCR1, MNP | V42BIS | V42);

                TRACE1("modType1=%x", modType1);

#if SUPPORT_AUTO
                TRACE1("pAce->AutoMode=%x", pAce->AutoMode);
#endif

                if ((pAce->ubModemType == MODEM_V90) || modType1
#if SUPPORT_AUTO
                    || pAce->AutoMode
#endif
                   )
                {
                    AceSetResponse(pTable, ACE_ERROR);
                    TRACE0("V54 no support Auto, V90 and V42/V42bis/MNP");
                    return FAIL;
                }

#endif

                InitSelfTest(pAce);

                if (CommVal == 8)
                {
                    SetReg(pSreg[V54_SELF_TEST], SREG_LAL_PATT);
                    pAce->ubSelfTest = SELF_TEST_8;
                    PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                    PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
                    Ace_ChangeLineState(pAce, ONLINE_COMMAND);
                }
                else
                {
                    SetReg(pSreg[V54_SELF_TEST], SREG_LAL);
                    pAce->ubSelfTest  = SELF_TEST_1;
                    Ace_ChangeLineState(pAce, ONLINE_DATA);
                }

                if (pSreg[V54_SELF_TIMER] != 0)
                {
                    pAce->StartTime = *pClk;
                }

                SetReg(pDpcs->MTC,  LDL_MODE);

                /* ClrReg(pDpcs->MCR0); */
                SetReg(pDpcs->MCR0, TEST_MODE);
                MhostConnect(pTable);
                ResetReg(pDpcs->MMR0, CP);

                /* No V.42 in loop-back and init to ASC */
                pAce->ubMCR1_BUFFER = pDpcs->MCR1;
                ResetReg(pDpcs->MCR1, V42);
                break;

            case 0:
                ClrReg(pDpcs->MTC);
                ResetReg(pDpcs->MCR0, TEST_MODE);

                pAce->ubSelfTest = SELFTEST_OFF;

                is_test_1_8 = RdReg(pSreg[V54_SELF_TEST], SREG_LAL | SREG_LAL_PATT);
                is_test_6_7 = RdReg(pSreg[V54_SELF_TEST], SREG_RDL | SREG_RDL_PATT);

                if ((is_test_1_8) && (pAce->LineState == ONLINE_COMMAND))
                {
                    PutReg(pDpcs->MCF, DISCONNECT);
                    Ace_ChangeLineState(pAce, OFF_LINE);
                    pAce->CarrierDetect_Flag = 0;

                    if (pSreg[V54_SELF_TEST] & SREG_LAL)
                    {
                        pAce->ubSelfTest_T0 = ANDT0_UNSELECTED;
                    }
                    else
                    {
                        pAce->ubSelfTest_T0 = SELF_TEST_8_END;
                    }
                }
                else if (is_test_6_7 && (pAce->LineState == ONLINE_COMMAND))
                {
                    V54_TerminateRDL(pTable);

                    pAce->StartTime = *pClk;

                    if (pSreg[V54_SELF_TEST] & SREG_RDL)
                    {
                        pAce->ubSelfTest_T0 = SELF_TEST_6_END;
                    }
                    else
                    {
                        pAce->ubSelfTest_T0 = SELF_TEST_7_END;
                    }
                }

                /*             else pAce->ubSelfTest_T0 = SELFTEST_OFF;  just in case, ... too many bugs!! */

                pDpcs->MCR1 = pAce->ubMCR1_BUFFER;

                /* pAce->LineState = OFF_LINE; */

                ClrReg(pSreg[V54_SELF_TEST]);        /* clear all self test command! */

                break;

            case 4:

                if (pAce->ubSelfTest != SELFTEST_OFF)
                {
                    AceSetResponse(pTable, ACE_ERROR);
                    return FAIL;
                }

                SetReg(pSreg[V54_RESPONSE_CONTROL], RDL_ENABLE);
                SetReg(pDpcs->MCR0, RDLENA);

                V54_Init(pTable);
                break;

            case 5:

                if (pAce->ubSelfTest != SELFTEST_OFF)
                {
                    AceSetResponse(pTable, ACE_ERROR);
                    return FAIL;
                }

                ResetReg(pSreg[V54_RESPONSE_CONTROL], RDL_ENABLE);
                ResetReg(pDpcs->MCR0, RDLENA);

                break;

            default:
                break;

        } /* end switch */

        pAce->CurrCommand[pAce->ATidx] = 0;

        if (pAce->ubSelfTest_T0 == SELF_TEST_8_END)
        {
            return ReportSelfTestErr(pTable, pAce->uSelfTestErrorCount);
        }
        else if ((pAce->ubSelfTest_T0 == SELF_TEST_6_END) || (pAce->ubSelfTest_T0 == SELF_TEST_7_END))
        {
            TRACE1("Test %d End: do nothing here!", pAce->ubSelfTest_T0 + 3);
            return SUCCESS;
        }

#if 0
        else if ((pAce->ubSelfTest == SELFTEST_OFF) ||             /* commands &T0, 4, 5 */
                 (pAce->ubSelfTest == SELF_TEST_1)  || (pAce->ubSelfTest == SELF_TEST_8))
#else
        else if (pAce->ubSelfTest != SELF_TEST_6)
#endif
        {
            /* pAce->CurrCommand[pAce->ATidx+1] = 0; */
            pAce->Mandate = 1;
            AceSetResponse(pTable, ACE_OK);
        }

        return SUCCESS;
    }
    else
#endif
    {
        /* set result code: "ERROR") */
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
}

UBYTE AT_andD(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx+1]))
    {
        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }

    if ((pAce->CurrCommand[pAce->ATidx+1] >= '0') && (pAce->CurrCommand[pAce->ATidx+1] <= '2'))
    {
        pAce->ATidx++;
        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }
    else
    {
        /* set result code: "ERROR") */
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
}

UBYTE AT_andC(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (!COMM_IsNumber(pAce->CurrCommand[pAce->ATidx+1]))
    {
        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }

    if ((pAce->CurrCommand[pAce->ATidx+1] == '1') || (pAce->CurrCommand[pAce->ATidx+1] == '0'))
    {
        pAce->ATidx++;
        /* set result code: "OK") */
        AceSetResponse(pTable, ACE_OK);
        return SUCCESS;
    }
    else
    {
        /* set result code: "ERROR") */
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
}
#endif
