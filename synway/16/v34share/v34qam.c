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
#include "shareext.h"

#if USE_ASM
#include "ptable.h"
#endif

/*F:V34_Modulate_Init()**********************************************/
/* Modulation - Including transmit filtering                        */
/*                                                                  */
/* INPUT: V34Struct * pV34 - The V.34 info structure                */
/*                                                                  */
/* OUTPUT: PCMoutPtr is filled with modulated samples               */
/*                                                                  */
/* Last Updated: Feb 13, 1996.                                      */
/*               Feb 16, 1996.                                      */
/*               Feb 25, 1996. Fix Modulation bug, I*cos - Q*sin,   */
/*                             Benjamin                             */
/*               Feb 29, 1996. Use Interpolate filter, Benjamin     */
/*               Mar 08, 1996. Modified for double buffering        */
/*               Apr 19, 1996. Take away SYMBOL_SCALE_DOWN          */
/*               Apr 30, 1996. Jack Liu Porting to VXD lib          */
/* Author: Benjamin Chan, GAO Research & Consulting Ltd.            */
/*         Xian, GAO Research & Consulting Ltd.                     */
/*F!*****************************************************************/

void V34_CircFIR_Filter_Init(CircFilter *fir, QWORD *pqHist, QDWORD *pqCoef, UBYTE coef_len)
{
    fir->tap_length = coef_len;
    fir->offset = 0;
    fir->pqDline = pqHist;
    fir->pqCoef = pqCoef;
}

void  V34_Modulate_Init(UBYTE **pTable, QAMStruct *pQAM)
{
#if USE_ASM
    QWORD *PsfDlineQ = (QWORD *)(pTable[CIRCDATA_IDX]);
    QWORD *PsfDlineI = (QWORD *)(pTable[CIRCDATA_IDX] + 32);

    Dspi3Fir_PsfInit(&(pQAM->iFirQ), (QWORD *)V34_PsfCoef, PsfDlineQ, V34_PSF_DELAY_LEN, V34_SYM_SIZE);
    Dspi3Fir_PsfInit(&(pQAM->iFirI), (QWORD *)V34_PsfCoef, PsfDlineI, V34_PSF_DELAY_LEN, V34_SYM_SIZE);
#else
    Dspi3Fir_PsfInit(&(pQAM->iFirQ), (QWORD *)V34_PsfCoef, pQAM->DlineQ, V34_PSF_DELAY_LEN, V34_SYM_SIZE);
    Dspi3Fir_PsfInit(&(pQAM->iFirI), (QWORD *)V34_PsfCoef, pQAM->DlineI, V34_PSF_DELAY_LEN, V34_SYM_SIZE);
#endif

    pQAM->tx_carrier_idx = 0;
    pQAM->qTx_carrier_offset_idx = 0;

    memset(pQAM->pqTx_preemp_dline, 0, sizeof(pQAM->pqTx_preemp_dline));
}


/********************************************************************/
/* Modulation - Including transmit filtering                        */
/*                                                                  */
/* INPUT:  COMPLEX *u - Symbol to send out                          */
/*         V34Struct * pV34 - The V.34 info structure               */
/*         S16 n - Number of symbols                                */
/* OUTPUT: PCMoutPtr is filled with modulated samples               */
/********************************************************************/

#if !USE_ASM
#if 0
void  V34_Modulate(V34TxStruct *pV34Tx, CQWORD *u, EC_InsertStru *Ec_Insert)
#else
void  V34_Modulate(V34TxStruct *pV34Tx, CQWORD *u)
#endif
{
    QAMStruct *pQAM = &(pV34Tx->QAM);
    QWORD pqFirx[3];
    QWORD pqFiry[3];
    QWORD qC1, qS1, qOut;
    QDWORD qdTemp;
    QWORD qOffset;
    UBYTE  i, j, idxc, idxs;
    QDWORD qOutput_r;
    QWORD *pqOut_bufr;

    /* ----------------------------- Transmit filter---------------------------- */
    //pqOut_bufr = pqFirx;

    Dspi3Fir_Psf(&pQAM->iFirQ, u->r, pqFirx);
    Dspi3Fir_Psf(&pQAM->iFirI, u->i, pqFiry);
    /* ------------------------------ Modulation ------------------------------- */
    idxc = pQAM->tx_carrier_idx;
    idxs = (idxc - 64) & COS_SAMPLE_NUM; /* Step back Pi/2 for Sine value */
    qOffset = pQAM->qTx_carrier_offset_idx;

    for (j = 0; j < V34_SYM_SIZE; j++)
    {
        qC1 = DSP_tCOS_TABLE[idxc] + QQMULQR15(DSP_tCOS_OFFSET[idxc], qOffset);
        qS1 = DSP_tCOS_TABLE[idxs] + QQMULQR15(DSP_tCOS_OFFSET[idxs], qOffset);

        qdTemp  = QQMULQD(pqFirx[j], qC1);
        qdTemp -= QQMULQD(pqFiry[j], qS1);
        qOut   = QDR15Q(qdTemp);

        /* V34_CircFIR_Filter */
        pqOut_bufr = &(pQAM->tx_preemp.pqDline[pQAM->tx_preemp.offset]);

        *pqOut_bufr   = qOut;  /* Overwrite the oldest sample */
        pqOut_bufr[7] = qOut; /* Double buffering */

        ++pqOut_bufr;

        /* form output accumulation */
        qOutput_r = 0;

        for (i = 0; i < 7; i++)
        {
            qOutput_r += QDQMULQD0(pQAM->tx_preemp.pqCoef[i], pqOut_bufr[i]);
        }

        qOut = QDR15Q(qOutput_r);

        (pQAM->tx_preemp.offset)++;

        if (pQAM->tx_preemp.offset >= pQAM->tx_preemp.tap_length)
        {
            pQAM->tx_preemp.offset = 0;
        }

        /* Circfir end */

        pV34Tx->PCMoutPtr[j] = QQMULQR8(qOut, pQAM->qTxScale);/* qTxScale in Q7.8 */

        qdTemp = (QDWORD)qOffset + pQAM->qTx_carrier_offset;

        if (qdTemp > qONE)
        {
            qdTemp -= qONE;
            ++idxc;
            ++idxs;
        }

        qOffset = (QWORD)qdTemp;

        idxc = (idxc + pQAM->tx_carrier_freq) & COS_SAMPLE_NUM;
        idxs = (idxs + pQAM->tx_carrier_freq) & COS_SAMPLE_NUM;
    }

    pQAM->tx_carrier_idx = idxc;
    pQAM->qTx_carrier_offset_idx = qOffset;

#if 0
    Ec_Insert->qCos = qC1;
    Ec_Insert->qSin = qS1;
#endif
}
#endif
