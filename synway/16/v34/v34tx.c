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

/**************************************************************************/
/* V.34 Transmiter - inluding scrambling, encoding and modulation         */
/*          SWORD *PCMoutPtr    Buffer for modulation output              */
/**************************************************************************/

#include "ptable.h"
#include "ioapi.h"
#include "v34ext.h"

#if 0 /// LLL temp
void V34_Transmiter(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc *pEc = &(pV34->Echo);
    UBYTE *pIOBuf;
    CQWORD cqOutsymbol;
    SBYTE nbyte, nbits, b;

    cqOutsymbol.r = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].r;
    cqOutsymbol.i = pV34Tx->pcTx_sym[pV34Tx->tx_sym_outidx].i;

#if 0

    if (ModemAorB == 0)
    {
        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = cqOutsymbol.r; }

        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = cqOutsymbol.i; }
    }

#endif

    V34_Modulate(pV34Tx, &cqOutsymbol, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOutsymbol, (CQWORD *)&pEc->pcBulk_delay);

    pV34Tx->tx_sym_outidx ++;
    pV34Tx->tx_sym_outidx &= V34_SYM_BUF_MASK;

    if (pV34Tx->tx_sym_outidx == pV34Tx->tx_sym_inidx)
    {
        nbyte = pV34Tx->nbyte;
        nbits = pV34Tx->nbits;
        b     = pV34Tx->b;

        if ((pV34Tx->current_SWP_bit & 0x1) == 0)
        {
            --b;

            if (--nbits < 0)
            {
                nbits = 7;
                --nbyte;
            }
        }

#if 1
        /**************** IO Read from terminal ********************/
        /* IO_ReadV34Data(b, pV34->DTE_in, pGlobal);              */
        /**************** IO Read from terminal ********************/
        pIOBuf = (UBYTE *)(pV34->pTable[DSPIORDBUFDATA_IDX]);
        /**************** IO Read from terminal ********************/
        IoRdProcess(pV34->pTable, b);
        /**************** IO Read from terminal ********************/
#endif

        (*pV34Rx->pfScram_byte)(pIOBuf, &(pV34Rx->sc), pShellMap->pScram_buf, nbyte);
        (*pV34Rx->pfScram_bit)(pIOBuf[nbyte], &(pV34Rx->sc), &(pShellMap->pScram_buf[b - nbits]), nbits);

        pShellMap->scram_idx = 0;
        V34_Send_Mapping_Frame(pV34);
    }
}
#endif