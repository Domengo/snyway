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

#include "v34fext.h"

#if SUPPORT_V34FAX

#define DUMP_PCM_OUT        0        /* t4 */

void V34Fax_Scramble(V34FaxStruct *pV34Fax)
{
    UBYTE  i, ubNumBits;
    UWORD  out;
    UBYTE  out_stream[4];

    ubNumBits = pV34Fax->ubTxBitsPerSym;

    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        V34_Scram_Bit_GPC((UBYTE)pV34Fax->uScramInbits, &pV34Fax->udScramSReg, out_stream, ubNumBits);
    }
    else
    {
        V34_Scram_Bit_GPA((UBYTE)pV34Fax->uScramInbits, &pV34Fax->udScramSReg, out_stream, ubNumBits);
    }

    out = 0;

    for (i = 0; i < ubNumBits; i++)
    {
        out |= (out_stream[i] << i);
    }

    pV34Fax->uScramOutBits = out;
}


void V34Fax_Encode(V34FaxStruct *pV34Fax)
{
    UBYTE I2I1, Q2Q1I2I1, Q2Q1;

    I2I1 = (pV34Fax->ubEncoderOldPhase + pV34Fax->uScramOutBits) & 3; // Z[n] is the modulo 4 sum of I2I1[n] and I2I1[n-1]. 2*I2+I1 = I2I1

    if (pV34Fax->ubTxBitsPerSym == V34FAX_SYM_BITS_1200)
    {
        Q2Q1 = 0; // 10.2.4
    }
    else
    {
        Q2Q1 = (UBYTE)(pV34Fax->uScramOutBits & 0x000C);
    }

    Q2Q1I2I1 = Q2Q1 + I2I1; // 4 bits

    pV34Fax->IQ = V34FAX_MAP_TAB[Q2Q1I2I1]; // point 0 is Q12 of (1, 1)

    pV34Fax->ubEncoderOldPhase = I2I1; // store Z[n]
}


void V34Fax_PSF_Init(V34FaxStruct *pV34Fax)
{
    DspiFirInit(&pV34Fax->PsfI, (QWORD *)V34Fax_PSF_Coeff, pV34Fax->qPsfDlineI, V34FAX_PSF_DELAY_LEN, V34FAX_SYM_LEN);

    DspiFirInit(&pV34Fax->PsfQ, (QWORD *)V34Fax_PSF_Coeff, pV34Fax->qPsfDlineQ, V34FAX_PSF_DELAY_LEN, V34FAX_SYM_LEN);
}


void V34Fax_PSF(V34FaxStruct *pV34Fax)
{
    DspiFir(&pV34Fax->PsfI, pV34Fax->IQ.r, pV34Fax->qPsfOutBufI);

    DspiFir(&pV34Fax->PsfQ, pV34Fax->IQ.i, pV34Fax->qPsfOutBufQ);
}


void V34Fax_Modulation_Init(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->modem_mode == CALL_MODEM)
    {
        pV34Fax->ModCos = V34Fax_1200COS;
        pV34Fax->ModSin = V34Fax_1200SIN;
    }
    else
    {
        pV34Fax->ModCos = V34Fax_2400COS;
        pV34Fax->ModSin = V34Fax_2400SIN;
    }
}


void V34Fax_Modulation(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);
    V34TxStruct  *pV34Tx  = &(pV34->V34Tx);
    QWORD c, s;
    UBYTE i;

    for (i = 0; i < V34FAX_SYM_LEN; i++)
    {
        c = pV34Fax->ModCos[i];
        s = pV34Fax->ModSin[i];

        pV34Tx->PCMoutPtr[i] = QQMULQ15(pV34Fax->qPsfOutBufI[i], c) - QQMULQ15(pV34Fax->qPsfOutBufQ[i], s); //

        pV34Tx->PCMoutPtr[i] += (pV34Tx->PCMoutPtr[i] >> 1);
#if DUMP_PCM_OUT

        if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = pV34Tx->PCMoutPtr[i]; }

#endif
    }
}

#endif
