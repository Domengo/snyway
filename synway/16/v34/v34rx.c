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
/* V.34 Receiver -                                                        */
/*                                                                        */
/* INPUT:   SC_DataStrucPtr pSC                                           */
/*          UBYTE *pGlobal                                                */
/**************************************************************************/

#include "ptable.h"
#include "ioapi.h"
#include "v34ext.h"

#if 0 /// LLL temp
void V34_Receiver(ReceiveStruc *pRx, UBYTE **pTable, SWORD qToIO)
{
    DeCodeStruc *DeCode = &(pRx->DeCode);
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    TimeCarrRecovStruc *pTCR = &pRx->TCR;
    V34Struct *pV34 = (V34Struct *)(pTable[V34_STRUC_IDX]);
    V34StateStruc *pV34State = &pRx->V34State;
    Phase4_Info *p4 = &(pV34->p4);
    CQWORD cqSymbol;
    CQWORD pcDsym[3], pcTimingOut[3];
    UBYTE  b, nbyte, hi_frame;
    SBYTE  nbits;
    UBYTE  *pIOBuf;

    V34_Demodulate(pRx, pcDsym);

    V34_Timing_Rec(pTCR, pcDsym, pcTimingOut);

    if (pTCR->Tcount <= 0)
    {
        return;
    }

    cqSymbol = DspcFir_2T3EQ(&(pTCR->eqfir), pcTimingOut);

#if 0

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = cqSymbol.r; }

    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = cqSymbol.i; }

#endif

    V34_Decoder(pRx, &cqSymbol);

    if (pRx->rx_sym_count >= 8)
    {
        nbyte = RxMod->rx_nbyte;
        nbits = RxMod->rx_nbits;

        b = RxMod->rx_b;

        hi_frame = pRx->current_rx_SWP_bit & 0x1;
        pRx->current_rx_SWP_bit >>= 1;

        if (hi_frame == 0)
        {
            --b;

            if (--nbits < 0)
            {
                nbits = 7;
                --nbyte;
            }
        }

        V34_Retrieve_Infobits(pRx, hi_frame);

        /**************** Fill IO Buffer ********************/
        pIOBuf = (UBYTE *)(pTable[DSPIOWRBUFDATA_IDX]);
        (*pRx->pfDescram_byte)(pRx->pDscram_buf, &(pRx->dsc), pIOBuf, nbyte);
        (*pRx->pfDescram_bit)(&(pRx->pDscram_buf[b-nbits]), &(pRx->dsc), &(pIOBuf[nbyte]), nbits);
        /**************** Fill IO Buffer ********************/

#if 0 /* first data must be all 1 */

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pIOBuf[0]; }

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pIOBuf[1]; }

#endif

        /**************** IO Write to terminal ********************/
        if (qToIO)
        {
            IoWrProcess(pTable, b);
        }
        else if (p4->pfRx[p4->rx_vec_idx] == V34_Phase4_Rec_B1_Frame)/* Receive B1 */
        {
            if ((pIOBuf[0] & 0xFF) != 0xFF)
            {
                pRx->ubB1_Err_Count++;
            }

            if (pRx->ubB1_Err_Count > 10)
            {
                TRACE0("Retrain by local, cuase by B1 error");

                pRx->Retrain_flag = RETRAIN_INIT;

                pV34State->retrain_reason = V34_EQ_TRAINING_BAD;

                pRx->ubB1_Err_Count = 0;
            }
        }
    }
}
#endif