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

#if SUPPORT_T32_PARSER
UBYTE CLASS2_FPS(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS2)
    {
        switch (pAce->CurrCommand[pAce->ATidx])
        {
            case '=':
                pAce->ATidx++;

                if (pAce->CurrCommand[pAce->ATidx] == '?')
                {
                    pAce->ATidx++;
                    AceSetResponse(pTable, ACE_FPS_CAP);
                    return SUCCESS;
                }
                else
                {
                    UBYTE fps;
                    UBYTE isEnd = FALSE;

                    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &fps, &isEnd, FALSE, POST_PG_RSP_MCF))
                    {
                        if (fps <= POST_PG_RSP_RTP)
                        {
                            Ace_SetT32Command(pAce, DTE_T30_FPS);
                            pAce->pDteToT30->tempData[0] = fps;
                            return SUCCESS;
                        }
                    }
                }

            case '?':
            {
                CircBuffer *DteRd = pAce->pCBOut;
                UBYTE *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                PutByteToCB(DteRd, '1');

                PutByteToCB(DteRd, pSreg[CR_CHARACTER]);
                PutByteToCB(DteRd, pSreg[LF_CHARACTER]);

                AceSetResponse(pTable, ACE_OK);

                return SUCCESS;
            }
        }
    }

    return FAIL;
}
#endif
