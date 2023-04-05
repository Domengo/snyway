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

/******************************************************************************
*
*         FILE NAME:        DCEMAIN.C
*        AUTHOR:            SPARK LEE
*        DATE:            May. 2002
*        DISCRIPTION:    Schedule ASC, Fax, V42 and MNP modules
*
*****************************************************************************/

#include <string.h>
#include "ptable.h"
#include "dceext.h"
#include "aceext.h"

/*************************************/
/* Initialize DCE when system starts */
/*************************************/
void DCEInit(UBYTE **pTable)
{
    DceStruct *pDCE = (DceStruct *)pTable[DCE_STRUC_IDX];

    memset(pDCE, 0, sizeof(DceStruct));
}

/******************/
/*    Set DCE mode  */
/******************/
static void DCESetMode(UBYTE **pTable)
{
    DceStruct  *pDCE  = (DceStruct *)pTable[DCE_STRUC_IDX];
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    pDCE->pfDCETxVec    = FAXIOTx;
    pDCE->pfDCERxVec    = FAXIORx;

    SetReg(pDpcs->MCR0, FAX);
    ResetReg(pDpcs->MFCF0, CHAR_OR_HDLC);

    pDCE->numBitRd = 0;
    pDCE->numBitWr = 0;
    pDCE->bitBufRd = 0;
    pDCE->bitBufWr = 0;
    pDCE->state    = DCE_STATE_ESTABLISH;

    FAXInit(pTable);

    TRACE0("DCE: Mode set");
}

#if SUPPORT_V34FAX

/*****************************/
/*    Set DCE mode for V34Fax  */
/*****************************/
static void DCESetMode_V34Fax(UBYTE **pTable)
{
    UBYTE code;
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

    code = RdReg(pDpcs->MMSR0, MODULATION);

    if (code == AUTOMODE)
    {
        return;
    }

    DCESetMode(pTable);
}

#endif

void DCE_SelectMode_Set(UBYTE **pTable)
{
    DCESetMode(pTable);
}

/*************************************************************
DCE(ASC V.42 Fax and MNP) interface function for ModemMain
*************************************************************/
void DCEMain(UBYTE **pTable)
{
    DceStruct  *pDCE  = (DceStruct *)pTable[DCE_STRUC_IDX];
#if SUPPORT_V34FAX
    ACEStruct *pAce = (ACEStruct *)pTable[ACE_STRUC_IDX];
    UBYTE isV34Selected = pAce->pDceToT30->isV34Selected;
#endif
    DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];
    UBYTE      code;

#if SUPPORT_V34FAX

    if (isV34Selected)
    {
        switch (pDCE->state)
        {
            case DCE_STATE_INIT:
                DCESetMode_V34Fax(pTable);

                break;

            case DCE_STATE_ESTABLISH:

                if (RdReg(pDpcs->MSR0, DATA))
                {
                    pDCE->state  = DCE_STATE_DATA;
                }

                break;

            case DCE_STATE_DATA:

                if (RdReg(pDpcs->MSR0, DISC_DET))
                {
                    pDCE->state  = DCE_STATE_DISCONNECT;
                }
                else
                {
                    FAXMain(pTable);
                }

                break;

            case DCE_STATE_DISCONNECT:
                pDCE->state  = DCE_STATE_INIT;
                break;

            default:
                break;
        }
    }
    else

#endif

    {
        if (pDCE->state == DCE_STATE_INIT)
        {
            code = RdReg(pDpcs->MMSR0, MODULATION);

            if (code != MODEM_NULL)
            {
                DCE_SelectMode_Set(pTable);
            }
        }
        else
        {
            if (RdReg(pDpcs->MSR0, DISC_DET))
            {
                pDCE->state = DCE_STATE_INIT;
            }
            else if (RdReg(pDpcs->MSR0, DATA))
            {
                FAXMain(pTable);
            }
        }
    }
}

