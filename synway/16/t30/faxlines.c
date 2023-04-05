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

#include "t30ext.h"
#include "faxlinecnt.h"

#if SUPPORT_T30
void FXL_CountFaxLines(T30Struct *pT30, UBYTE *ch)
{
    UBYTE i, tmp;
    UBYTE isMMR = pT30->pT30ToDte->faxParmStatus.dataFormatDF == DF_MMR;
    UBYTE isMR = pT30->pT30ToDte->faxParmStatus.dataFormatDF == DF_MR;

    tmp = 0x01;

    for (i = 0; i < 8; i++)
    {
        if ((tmp & *ch) == 0)
        {
            pT30->ZeroCount_EOL++;
        }
        else
        {
            if (isMR && pT30->ImageState == IMAGE_STATE_EOL)
            {
                // ignore the +0/1 bit of EOL
                if (pT30->EOLCount == 6)
                {
                    pT30->ImageState = IMAGE_STATE_RTC_EOB;
                    *ch &= 0xff >> (7 - i);
                    pT30->EOLCount = 0;
                    return;
                }

                pT30->ImageState = IMAGE_STATE_DATA;
            }
            else if (pT30->ZeroCount_EOL < 11)
            {
                pT30->EOLCount = 0;
            }
            else
            {
                /* this bit is 1 and at least 11 zero bit ahead this bit. */
                pT30->LineCount ++;
                pT30->ImageState = IMAGE_STATE_EOL;

                if ((pT30->EOLCount != 0) && (pT30->ZeroCount_EOL >= 12))
                {
                    pT30->EOLCount = 1;
                }
                else
                {
                    pT30->EOLCount ++;
                }

                /* RTC = 6 x EOL for MH and MR */
                /* EOFB = 2 x EOL for MMR */
                if ((pT30->EOLCount == 6) || (isMMR && (pT30->EOLCount == 2)))
                {
                    pT30->ZeroCount_EOL = 0;
                    pT30->LineCount -= 6; // fixing issue 146

                    if (isMMR)
                    {
                        pT30->LineCount = pT30->T6LineStru.udLinesCount;
                    }

                    if (!isMR)
                    {
                        pT30->ImageState = IMAGE_STATE_RTC_EOB;
                        *ch &= 0xff >> (7 - i);
                        pT30->EOLCount = 0;
                        return;
                    }
                }
            }

            pT30->ZeroCount_EOL = 0;
        }

        tmp <<= 1;
    }
}
#endif
