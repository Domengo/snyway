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

#if (SUPPORT_V34FAX && SUPPORT_T31_PARSER)
UBYTE CLASS1_F34(UBYTE **pTable)
{
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

    if (RdReg(pAce->FaxClassType, FCLASS_MAJ) == FCLASS1)
    {
        /* command AT+F34=,?,?; shows max,min & cc rate */
        switch (pAce->CurrCommand[pAce->ATidx++])
        {
            case '=':
            {
                T30ToDceInterface *pT30ToDce = pAce->pT30ToDce;
                UBYTE isErr = FALSE, isEnd = FALSE;
                UBYTE initV34PrimRateMax, initV34PrimRateMin = V34_BIT_2400, initV34CntlRate = 1;

                if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &initV34PrimRateMax, &isEnd, FALSE, V34_BIT_33600))
                {
                    if (initV34PrimRateMax > V34_BIT_33600)
                    {
                        isErr = TRUE;
                    }
                    else if (initV34PrimRateMax == 0)
                    {
                        initV34PrimRateMax = V34_BIT_33600;
                    }
                }
                else
                {
                    isErr = TRUE;
                }

                if (!(isErr || isEnd))
                {
                    if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &initV34PrimRateMin, &isEnd, FALSE, V34_BIT_2400))
                    {
                        if (initV34PrimRateMin > V34_BIT_33600)
                        {
                            isErr = TRUE;
                        }
                        else if (initV34PrimRateMin == 0)
                        {
                            initV34PrimRateMax = V34_BIT_2400;
                        }
                    }
                    else
                    {
                        isErr = TRUE;
                    }

                    if (!(isErr || isEnd))
                    {
                        if (COMM_AsciiToUByte((UBYTE *)pAce->CurrCommand, &pAce->ATidx, &initV34CntlRate, &isEnd, FALSE, 1))
                        {
                            if (initV34CntlRate > 1)
                            {
                                isErr = TRUE;
                            }
                            else
                            {
                                initV34CntlRate = 1;
                            }
                        }
                        else
                        {
                            isErr = TRUE;
                        }
                    }
                }

                if (!isErr)
                {
                    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

                    if (pDpcs->V8OPCNTL == A8E_NULL)
                    {
                        pDpcs->V8OPCNTL = A8E_V8O_1 | A8E_V8A_1;
                    }

                    *pT30ToDce->pIsV34Enabled = TRUE;
                    pT30ToDce->initV34CntlRate = initV34CntlRate;
                    pT30ToDce->initV34PrimRateMax = initV34PrimRateMax;
                    pT30ToDce->initV34PrimRateMin = initV34PrimRateMin;
                    return SUCCESS;
                }
            }
        }
    }

    return FAIL;
}
#endif
