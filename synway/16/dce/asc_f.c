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

/****************************************************************************/
/*        DISCRIPTION:    Transfer data for character and fax mode            */
/****************************************************************************/

#include <string.h>
#include "dceext.h"
#include "common.h"
#include "ptable.h"
#if TRACEON
#include "aceext.h"
#endif

#if SUPPORT_FAX

void FAXInit(UBYTE **pTable)
{
    /* Keep the interface for the future */
}

void FAXMain(UBYTE **pTable)
{
    /* Keep the interface for the future */
}

/******************************************************************************
*
*        Transmit data to data pump in direct Fax mode
*
******************************************************************************/
extern UDWORD DumpPCMinCount;
void FAXIOTx(UBYTE **pTable, UBYTE *ch)
{
    CircBuffer *pAscCBRd = (CircBuffer *)pTable[ASCCBRDDATA_IDX];

    if (!GetByteFromCB(pAscCBRd, ch))
    {
#if TRACEON
        ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];

        if (pAce->classState == ACE_FTH ||
            pAce->classState == ACE_FTH_HS ||
            pAce->classState == ACE_FTM)
        {
            //TRACE0("DCE: WARNING. ASC buffer empty.");
        }

#endif
        *ch = 0;/* Pay attention to: Must not insert any data at any time. */
        /* At the moment, insert 0 when CB no data due to function structure. */
    }

    /// LLL debug
    //TRACE2("%8d: ASC: Tx: %02x ", DumpPCMinCount, *ch);
}

/******************************************************************************
*
*        Receive data from data pump in direct Fax mode
*
******************************************************************************/
void FAXIORx(UBYTE **pTable, UBYTE ch)
{
    CircBuffer *pAscCBWr = (CircBuffer *)pTable[ASCCBWRDATA_IDX];

    PutByteToCB(pAscCBWr, ch);
}

#endif
