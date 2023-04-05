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
#include "v251.h"
#include "common.h"

#if SUPPORT_PARSER && SUPPORT_FAX
UBYTE AT_plusF(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE ret = FAIL, isSetting = TRUE;

    // we are here because it's AT+Fxx
    switch (pAce->CurrCommand[pAce->ATidx++])
    {
#if SUPPORT_V34FAX && SUPPORT_T31_PARSER
        case '3':

            if (pAce->CurrCommand[pAce->ATidx++] == '4')
            {
                ret = CLASS1_F34(pTable);
            }

            break;
#endif
        case 'A':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
#if SUPPORT_T32_PARSER
                case 'A': ret = CLASS2_FAA(pTable); break;
                case 'P': ret = CLASS2_FAP(pTable); break;
#endif
#if SUPPORT_T31_PARSER
                case 'R': ret = CLASS1_FAR(pTable); break;
#endif
            }

            break;
        case 'B':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
#if SUPPORT_T32_PARSER
                case 'O': ret = CLASS2_FBO(pTable); break;
                case 'S': ret = CLASS2_FBS(pTable); break;
                case 'U': ret = CLASS2_FBU(pTable); break;
#endif
            }

            break;
        case 'C':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'L':

                    switch (pAce->CurrCommand[pAce->ATidx++])
                    {
                        case 'A':

                            if (pAce->CurrCommand[pAce->ATidx++] == 'S')
                            {
                                if (pAce->CurrCommand[pAce->ATidx++] == 'S')
                                {
                                    ret = AT_plusFCLASS(pTable);
                                }
                            }

                            break;
#if SUPPORT_T31_PARSER
                        case '=':
                            ret = CLASS1_FCL(pTable);
                            break;
#endif
                    }

                    break;
#if SUPPORT_T32_PARSER
                case 'C': ret = CLASS2_FCC(pTable); break;
                case 'Q': ret = CLASS2_FCQ(pTable); break;
                case 'R': ret = CLASS2_FCR(pTable); break;
                case 'S': ret = CLASS2_FCS(pTable); break;
                case 'T': ret = CLASS2_FCT(pTable); break;
#endif
            }

            break;
        case 'D':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
#if SUPPORT_T31_PARSER
                case 'D': ret = CLASS1_FDD(pTable); break;
#endif
#if SUPPORT_T32_PARSER
                case 'R': ret = CLASS2_FDR(pTable); isSetting = FALSE; break;
                case 'T': ret = CLASS2_FDT(pTable); isSetting = FALSE; break;
#endif
            }

            break;
        case 'I':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'T':
#if SUPPORT_T31_PARSER

                    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
                    {
                        ret = CLASS1_FIT(pTable);
                    }

#endif
#if SUPPORT_T32_PARSER

                    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
                    {
                        ret = CLASS2_FIT(pTable);
                    }

#endif
                    break;
#if SUPPORT_T32_PARSER
                case 'E': ret = CLASS2_FIE(pTable); break;
                case 'P': ret = CLASS2_FIP(pTable); isSetting = FALSE; break;
                case 'S': ret = CLASS2_FIS(pTable); break;
#endif
            }

            break;
#if SUPPORT_T32_PARSER
        case 'E':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'A': ret = CLASS2_FEA(pTable); break;
            }

            break;
        case 'F':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'C': ret = CLASS2_FFC(pTable); break;
                case 'D': ret = CLASS2_FFD(pTable); break;
            }

            break;
        case 'H':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'S': ret = CLASS2_FHS(pTable); break;
            }

            break;
        case 'K':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'S': ret = CLASS2_FKS(pTable); isSetting = FALSE; break;
            }

            break;
        case 'N':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'D': ret = CLASS2_FND(pTable); break;
                case 'R': ret = CLASS2_FNR(pTable); break;
                case 'S': ret = CLASS2_FNS(pTable); break;
            }

            break;
        case 'S':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'A': ret = CLASS2_FSA(pTable); break;
                case 'P': ret = CLASS2_FSP(pTable); break;
            }

            break;
#endif
        case 'L':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
#if SUPPORT_T32_PARSER
                case 'I': ret = CLASS2_FLI(pTable); break;
                case 'P': ret = CLASS2_FLP(pTable); break;
#endif
                case 'O': ret = AT_plusFLO(pTable); break;
            }

            break;
        case 'M':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'I': ret = AT_plusFMI(pTable); break;
                case 'M': ret = AT_plusFMM(pTable); break;
                case 'R': ret = AT_plusFMR(pTable); break;
#if SUPPORT_T32_PARSER
                case 'S': ret = CLASS2_FMS(pTable); break;
#endif
            }

            break;
        case 'P':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
#if SUPPORT_T32_PARSER
                case 'A': ret = CLASS2_FPA(pTable); break;
                case 'I': ret = CLASS2_FPI(pTable); break;
                case 'P': ret = CLASS2_FPP(pTable); break;
                case 'S': ret = CLASS2_FPS(pTable); isSetting = FALSE; break;
                case 'W': ret = CLASS2_FPW(pTable); break;
#endif
                case 'R': ret = AT_plusFPR(pTable); break;
            }

            break;
        case 'R':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
#if SUPPORT_T31_PARSER
                case 'H': ret = CLASS1_FRH(pTable); isSetting = FALSE; break;
                case 'S': ret = CLASS1_FRS(pTable); isSetting = FALSE; break;
                case 'M': ret = CLASS1_FRM(pTable); isSetting = FALSE; break;
#endif
#if SUPPORT_T32_PARSER
                case 'Q': ret = CLASS2_FRQ(pTable); break;
                case 'Y': ret = CLASS2_FRY(pTable); break;
#endif
            }

            break;
#if SUPPORT_T31_PARSER
        case 'T':

            switch (pAce->CurrCommand[pAce->ATidx++])
            {
                case 'H': ret = CLASS1_FTH(pTable); isSetting = FALSE; break;
                case 'S': ret = CLASS1_FTS(pTable); isSetting = FALSE; break;
                case 'M': ret = CLASS1_FTM(pTable); isSetting = FALSE; break;
            }

            break;
#endif
    }

    if (ret == FAIL)
    {
        AceSetResponse(pTable, ACE_ERROR);
        return FAIL;
    }
    else if (isSetting)
    {
        if (pAce->CurrCommand[pAce->ATidx] == 0)
        {
            pAce->ATidx--;
        }

        AceSetResponse(pTable, ACE_OK);
    }

    return SUCCESS;
}

#endif
