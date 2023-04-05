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
#include "aceext.h"
#include "gaoapi.h"
#include "common.h"
#include "faxapi.h"

#if SUPPORT_PARSER
UBYTE Ace_ParseUnsupportedParm(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case '=':

                switch (pAce->CurrCommand[pAce->ATidx++])
                {
                    case '0': return SUCCESS;
                    case '?': AceSetResponse(pTable, ACE_0_CAP); return SUCCESS;
                }

                break;
            case '?':
            {
                CircBuffer *DteRd = pAce->pCBOut;
                UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                PutByteToCB(DteRd, '0');

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                return SUCCESS;
            }
        }
    }

    return FAIL;
}

void Ace_ReportFlowControl(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

    PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
    PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

    PutByteToCB(DteRd, pAce->ubFlowControlType + '0');

    PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
    PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
}

UBYTE Ace_SetReportFlowControl(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    switch (pAce->CurrCommand[pAce->ATidx++])
    {
        case '0': pAce->ubFlowControlType = FLOWCONTROL_NO; break;
        case '1': pAce->ubFlowControlType = FLOWCONTROL_SW; break;
        case '2': pAce->ubFlowControlType = FLOWCONTROL_HW; break;
        case '?': AceSetResponse(pTable, ACE_FC_CAP); return SUCCESS;
        default: return FAIL;
    }

    TRACE1("ACE: Flow Control %d", pAce->ubFlowControlType);

    return SUCCESS;
}

void Ace_ReportSerialPortRate(UBYTE **pTable, UBYTE format)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

    PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
    PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

    switch (format)
    {
        case 0:
            // +T? format
            break;
        case 1:
            // +IPR? format
            PutStringToCB(DteRd, "+IPR:", 5);

            switch (pAce->ubSerPortRate)
            {
                case 0: PutByteToCB(DteRd, '0'); break;
                case 4: PutStringToCB(DteRd, "2400", 4); break;
                case 5: PutStringToCB(DteRd, "4800", 4); break;
                case 7: PutStringToCB(DteRd, "9600", 4); break;
                case 10: PutStringToCB(DteRd, "19200", 5); break;
                case 11: PutStringToCB(DteRd, "38400", 5); break;
                case 12: PutStringToCB(DteRd, "57600", 5); break;
            }

            break;
#if SUPPORT_FAX
        case 2:

            // +FPR? format
            switch (pAce->ubSerPortRate)
            {
                case 0: PutByteToCB(DteRd, '0'); break;
                case 4: PutByteToCB(DteRd, '1'); break;
                case 5: PutByteToCB(DteRd, '2'); break;
                case 7: PutByteToCB(DteRd, '4'); break;
                case 10: PutByteToCB(DteRd, '8'); break;
                case 11: PutStringToCB(DteRd, "10", 2); break;
                case 12: PutStringToCB(DteRd, "18", 2); break;
            }

            break;
#endif
    }

    PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
    PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
}

UBYTE Ace_SetReportSerialPortRate(UBYTE **pTable, UBYTE format)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE SerRate;
    UBYTE isEnd = FALSE, isErr = FALSE;

    switch (format)
    {
        case 0:

            // +T=<value> format
            if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &SerRate, &isEnd, FALSE, 0))
            {
                if ((SerRate >= 1) && (SerRate <= 13))
                {
                    pAce->ubSerPortRate = SerRate;
                }
                else
                {
                    isErr = TRUE;
                }
            }
            else
            {
                isErr = TRUE;
            }

            break;
        case 1:

            // +IPR=<value> format
            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case '?': AceSetResponse(pTable, ACE_IPR_CAP); return SUCCESS;
                case '0': pAce->ubSerPortRate = 12; break;
                case '2': pAce->ubSerPortRate = 4; pAce->ATidx += 3; break;
                case '4': pAce->ubSerPortRate = 5; pAce->ATidx += 3; break;
                case '9': pAce->ubSerPortRate = 7; pAce->ATidx += 3; break;
                case '1': pAce->ubSerPortRate = 10; pAce->ATidx += 4; break;
                case '3': pAce->ubSerPortRate = 11; pAce->ATidx += 4; break;
                case '5': pAce->ubSerPortRate = 12; pAce->ATidx += 4; break;
                default: isErr = TRUE; break;
            }

            break;
#if SUPPORT_FAX
        case 2:

            // +FPR=<value> format
            if (pAce->CurrCommand[pAce->ATidx] == '?')
            {
                pAce->ATidx++;
                AceSetResponse(pTable, ACE_FPR_CAP); return SUCCESS;
            }
            else if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &SerRate, &isEnd, TRUE, 0))
            {
                pAce->ATidx++;

                switch (SerRate)
                {
                    case 0x00: pAce->ubSerPortRate = 12; break;
                    case 0x01: pAce->ubSerPortRate = 4; break;
                    case 0x02: pAce->ubSerPortRate = 5; break;
                    case 0x04: pAce->ubSerPortRate = 7; break;
                    case 0x08: pAce->ubSerPortRate = 10; break;
                    case 0x10: pAce->ubSerPortRate = 11; break;
                    case 0x18: pAce->ubSerPortRate = 12; break;
                    default: isErr = TRUE; break;
                }
            }

            break;
#endif
    }

    if (isErr)
    {
        return FAIL;
    }
    else
    {
        TRACE1("ACE: Serial Port Rate %d", pAce->ubSerPortRate);
        return SUCCESS;
    }
}

void AceSetResponse(UBYTE **pTable, UBYTE Number)
{
    UBYTE Code0, Code, Mode, Proto, MMR;
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

    pAce->ubStringFlag = 0; /* EPR331 keep ' ' for string in AT command. */

    /* check for quiet */
    if (pAce->Quiet)
    {
        return;
    }

    /* for "OK" check if the last command */
    if ((Number == ACE_OK) && ((pAce->ATidx + 1) != (UBYTE)strlen((CHAR *)pAce->CurrCommand)) && (!pAce->Mandate))
    {
        return;
    }

    /* check for verbose code */
    if (pAce->Verbose)
    {
        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

        PutStringToCB(DteRd, (UBYTE *)(ResponseCode[Number]), (UWORD)(strlen((CHAR *)ResponseCode[Number])));

#if SUPPORT_MODEM

        /* display connect speed, modulation, protocol after "CONNECT" */
        if (Number == ACE_CONNECT_MODEM)
        {
            /* Display Bitrate */
            Code0 = GetReg(pDpcs->MBSC);

            PutStringToCB(DteRd, (UBYTE *)(RateCode[Code0]), (UWORD)(strlen((CHAR *)RateCode[Code0])));

            /* Display modulation */
            Code = RdReg(pDpcs->MMSR0, MODULATION);

            switch (Code)
            {
#if SUPPORT_V21M
                case MODEM_V21:     MMR = MMRCODE_V21;    break;
#endif
#if SUPPORT_V23
                case MODEM_V23:     MMR = MMRCODE_V23;    break;
#endif
#if SUPPORT_V22
                case MODEM_V22:     MMR = MMRCODE_V22;    break;
#endif
#if SUPPORT_V22BIS
                case MODEM_V22bis:  MMR = MMRCODE_V22BIS; break;
#endif
#if SUPPORT_V32
                case MODEM_V32:     MMR = MMRCODE_V32;    break;
#endif
#if SUPPORT_V32BIS
                case MODEM_V32bis:  MMR = MMRCODE_V32BIS; break;
#endif
#if SUPPORT_V34
                case MODEM_V34:     MMR = MMRCODE_V34;    break;
#endif
#if (SUPPORT_V90A + SUPPORT_V90D)
                case MODEM_V90:     MMR = MMRCODE_V90;    break;
#endif
#if SUPPORT_V92A
                case MODEM_V92:     MMR = MMRCODE_V92;      break;
#endif
#if SUPPORT_B103
                case BELL103:       MMR = MMRCODE_B103;   break;
#endif
#if SUPPORT_B212A
                case BELL212A:      MMR = MMRCODE_B212A;  break;
#endif
                default:            MMR = MMRCODE_NONE;
            }

            PutStringToCB(DteRd, (UBYTE *)(ModulationCode[MMR]), (UWORD)(strlen((CHAR *)ModulationCode[MMR])));

            /* Display Protocol */
            Mode = RdReg(pDpcs->MCR1, V42 | V42BIS | MNP);
            Code = RdReg(pDpcs->MSR1, V44_DATA | V42bis_DATA | V42_DATA | V42_FAIL | MNP_DATA | MNP_DATA1 | MNP_FAIL);

            if (Mode == 0)
            {
                Proto = PROTOCODE_NONE;
            }
            else
            {
                switch (Code)
                {
                    case V42_DATA:                    Proto = PROTOCODE_V42;    break;
                    case V44_DATA|V42_DATA:           Proto = PROTOCODE_V44;    break;
                    case V42bis_DATA|V42_DATA:        Proto = PROTOCODE_V42BIS; break;
                    case V42_FAIL|MNP_DATA1|MNP_DATA: Proto = PROTOCODE_MNP4;   break;
                    case V42_FAIL|MNP_DATA1:          Proto = PROTOCODE_MNP3;   break;
                    case V42_FAIL|MNP_DATA:           Proto = PROTOCODE_MNP2;   break;
                    default:                          Proto = PROTOCODE_NONE;   break;
                }
            }

            PutStringToCB(DteRd, (UBYTE *)(ProtocolCode[Proto]), (UWORD)(strlen((CHAR *)ProtocolCode[Proto])));

            TRACE3("CONNECT%s%s%s", RateCode[Code0], ModulationCode[MMR], ProtocolCode[Proto]);
        }

#endif
        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
        PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
    }
    else/* for numeric code */
    {
        PutByteToCB(DteRd, (UBYTE)(Number + 0x30));
        PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
    }

    if (Number != ACE_CONNECT_MODEM)
    {
        TRACE1("%s", ResponseCode[Number]);
    }

#if SUPPORT_FAX

    if (pAce->FaxClassType != FCLASS0)
    {
        switch (Number)
        {
            case ACE_CONNECT:

                if (pAce->LineState != ONLINE_FAX_DATA)
                {
                    Ace_ChangeLineState(pAce, ONLINE_FAX_DATA);
#if SUPPORT_T32_PARSER
                    pAce->prevDataCmd = NULL_DATA_CMD;
#endif
                }

                break;
            case ACE_OK:
            case ACE_ERROR:
            case ACE_NO_CARRIER:
                pAce->Mandate = FALSE;
                pAce->WasETX = FALSE;

                if (pAce->LineState == OFF_LINE)
                {
#if SUPPORT_T32_PARSER
                    pAce->pDteToT30->isDiscCmd = FALSE; // fixing bug 21, clear the command when any command is ACKed under OFF_LINE state
#endif
                }
                else if (pAce->LineState != ONLINE_FAX_COMMAND)
                {
                    Ace_ChangeLineState(pAce, ONLINE_FAX_COMMAND);
                }

                break;
        }
    }
    else
#endif
    {
        pAce->Mandate = FALSE;
    }
}

static void AT_parser(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE CommNameIdx, AtState, CurrCommandLen;
    UBYTE i;

    memcpy(pAce->LastCommand, pAce->CurrCommand, ((strlen((CHAR *)pAce->CurrCommand)) + 1) * sizeof(UBYTE));

    for (i = 1; ((pAce->CurrCommand[i] != 0) && (i < AT_SIZE)); i++)
    {
        if ((pAce->CurrCommand[i-1] == 'A')
            && (pAce->CurrCommand[i  ] == 'T'))
        {
            pAce->ATidx = (UBYTE)(i + 1);

            break;
        }
    }

    if (pAce->CurrCommand[i] == 0)
    {
        return;
    }

    if (pAce->CurrCommand[pAce->ATidx] == 0)
    {
        pAce->ATidx--;
        AceSetResponse(pTable, ACE_OK);
    }
    else
    {
        CurrCommandLen = (UBYTE)strlen((CHAR *)pAce->CurrCommand);

        /* check the command line */
        for (; pAce->ATidx < CurrCommandLen; pAce->ATidx++)
        {
            /* looking for command name */
            for (CommNameIdx = 0; CommNameIdx < AT_COMMAND_NAME_LEN; CommNameIdx++)
            {
                if (pAce->CurrCommand[pAce->ATidx] == AtCommandName_UpCase[CommNameIdx])
                {
                    AtState = pAce->AtActions[CommNameIdx](pTable);

                    if (pAce->Comma) /* if there is a comma in dial string */
                    {
                        return;
                    }

                    if (AtState) /* if implement the command successful, go to next command */
                    {
                        break;
                    }
                    else
                    {
                        return;
                    }
                }
                else if (CommNameIdx == (AT_COMMAND_NAME_LEN - 1))/* no this command */
                {
                    /* set result code: "ERROR" */
                    AceSetResponse(pTable, ACE_ERROR);
                    return;
                }
            }
        }
    }
}

UBYTE AT_plus(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    pAce->ATidx++;
    /* check for "at+a/m/t/c/d/e/f/i" command */

    switch (pAce->CurrCommand[pAce->ATidx++])
    {
#if SUPPORT_V251
        case 'A': return AT_plusA(pTable);
#endif
#if (SUPPORT_CID_DET + SUPPORT_CID_GEN)
        case 'C': return AT_plusC(pTable);
#endif
#if (SUPPORT_DTMF + SUPPORT_V44)
        case 'D': return AT_plusD(pTable);
#endif
        case 'E': return AT_plusE(pTable);
#if SUPPORT_FAX
        case 'F': return AT_plusF(pTable);
        case 'I': return AT_plusI(pTable);
#endif
#if SUPPORT_MODEM
        case 'M': return AT_plusM(pTable);
#endif
        case 'T': return AT_plusT(pTable);
        default: AceSetResponse(pTable, ACE_ERROR); return FAIL;
    }
}

/* AT command processing */
void AceAt(UBYTE **pTable, UBYTE ch)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    CircBuffer *DteRd = pAce->pCBOut;
    CircBuffer *DteWr = pAce->pCBIn;
    UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
    UBYTE temp, Keyhit = 1;

    do
    {
        if (Keyhit == 0)
        {
            if (!GetByteFromCB(DteWr, &ch))
            {
                break;
            }

#if FAX_DEBUG
            TRACE13("~%c", ch);
#endif
        }

        /* Echo char back to DTE if neccessary */
        if (pAce->Echo)
        {
            PutByteToCB(DteRd, ch);

            if (ch == pSreg[CR_CHARACTER])
            {
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);
            }
        }

        /* Backspace processing */
        if (ch == pSreg[BS_CHARACTER])
        {
            if (pAce->ATidx > 0)
            {
                pAce->ATidx--;
            }

            if (pAce->Echo)
            {
                temp = ' ';
                PutByteToCB(DteRd, temp);
                PutByteToCB(DteRd, pSreg[BS_CHARACTER]);
                return;
            }
        }

        /* check if command line ends, CARRIAGE RETURN */
        if (ch == pSreg[CR_CHARACTER])
        {
            if (pAce->ATidx < AT_SIZE)
            {
                pAce->CurrCommand[pAce->ATidx] = 0; /* End of string */
            }
            else
            {
                pAce->ATidx = 0;

                if ((pAce->CurrCommand[0] == 'A')
                    && (pAce->CurrCommand[1] == 'T'))
                {
                    AceSetResponse(pTable, ACE_ERROR);
                }
            }

            if (pAce->ATidx == 0)
            {
                return;
            }
            else
            {
                AT_parser(pTable);

                if (!pAce->Comma)
                {
                    pAce->ATidx = 0;
                }

                return;
            }
        }

        pAce->ubStringFlag ^= (ch == '"'); /* EPR 331 flag for " " */

        /* Skip all space in AT command if it is not a string */
        if (((ch + pAce->ubStringFlag) > ' ') && (ch < 128) && (pAce->ATidx < AT_SIZE))
        {
            pAce->CurrCommand[pAce->ATidx++] = pAce->ubStringFlag ? ch : COMM_ToUpperCase(ch);

            if ((pAce->ATidx == 2)
                && (pAce->CurrCommand[0] == 'A')
                && (pAce->CurrCommand[1] == 'A'))
            {
                pAce->ATidx = 1;
            }
            else if ((pAce->CurrCommand[0] != 'A')
                     || (pAce->ATidx > 1 && ((pAce->CurrCommand[0] != 'A') || (pAce->CurrCommand[1] != 'T'))))
            {
                pAce->ATidx = 0;
            }
        }

        if ((pAce->CurrCommand[0] == 'A') && (pAce->CurrCommand[1] == '/'))
        {
            temp = (UBYTE)strlen((CHAR *)pAce->LastCommand);

            if (temp > 0)
            {
                memcpy(pAce->CurrCommand, pAce->LastCommand, (temp + 1) * sizeof(UBYTE));
            }
            else /* No preceding command, OK will be issued */
            {
                pAce->CurrCommand[0] = 'A';
                pAce->CurrCommand[1] = 'T';
                pAce->CurrCommand[2] =  0 ;
            }

            AT_parser(pTable);

            if (!pAce->Comma)
            {
                pAce->ATidx = 0;
            }

            return;
        }

        Keyhit = 0;

    }
    while (DteWr->CurrLen);
}
#endif
