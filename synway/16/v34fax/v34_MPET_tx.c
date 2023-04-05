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

#include "ioapi.h"
#include "v34fext.h"

#if SUPPORT_V34FAX

#define TRANSMITTED_DATA            0       /* t1-bits */
#define TRANSMITTED_PAGE            0       /* page number */

void V34Fax_Symbol_Tx(V34Struct *pV34)
{
    V34FaxStruct *pV34Fax = &(pV34->V34Fax);

    if (pV34Fax->nTxDelayCnt <= 0)
    {
        if (((pV34Fax->modem_mode == CALL_MODEM) && (pV34Fax->MPET_Retrn_Type == 0) && (pV34Fax->pfTxSetUpVec == V34Fax_ALT_Tx_Setup)
             && (pV34Fax->PPhDetected == 0) && (pV34Fax->ShDetected == 0) && (pV34Fax->reRateFlag != RERATE_RESPOND))
            || ((pV34Fax->modem_mode == ANS_MODEM) && (pV34Fax->nTxState == 6)))
        {
            pV34Fax->nTxDelayCnt = 24000;
        }
        else
        {
            pV34Fax->PPhDetected = 0;

            pV34Fax->nTxState++;

            pV34Fax->pfTxSetUpVec = pV34Fax->pfTxSetUpVecTab[pV34Fax->nTxState];
            pV34Fax->nTxDelayCnt  = pV34Fax->nTxDelayCntTab[pV34Fax->nTxState];

            pV34Fax->pfTxSetUpVec(pV34Fax);
        }
    }

    pV34Fax->pfDataInVec(pV34Fax);
    pV34Fax->pfScramVec(pV34Fax);
    pV34Fax->pfEncodeVec(pV34Fax);

    V34Fax_PSF(pV34Fax);

    V34Fax_Modulation(pV34);

    pV34Fax->nTxDelayCnt--;

#if 0

    if (pV34Fax->Enter_CC_Order == TRANSMITTED_PAGE)
        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pV34Fax->IQ.r; }

#endif
}


void V34Fax_Silence_Tx(V34FaxStruct *pV34Fax)
{
    pV34Fax->IQ.r = 0;
    pV34Fax->IQ.i = 0;
}


void V34Fax_Sh_ShB_Tx(V34FaxStruct *pV34Fax)
{
    pV34Fax->IQ = V34FAX_MAP_TAB[pV34Fax->uScramOutBits];

    pV34Fax->uScramOutBits ^= 0x0003;
}


void V34Fax_MPET_AC_Tx(V34FaxStruct *pV34Fax)
{
    pV34Fax->IQ = V34FAX_MAP_TAB[pV34Fax->uScramOutBits];

    pV34Fax->uScramOutBits ^= 0x0002;
}


void V34Fax_PPh_Tx(V34FaxStruct *pV34Fax)
{
    pV34Fax->IQ = V34Fax_Create_PPh(pV34Fax->ubPPh_k, pV34Fax->ubPPh_I);

    pV34Fax->ubPPh_I ++;

    if (pV34Fax->ubPPh_I > 1)
    {
        pV34Fax->ubPPh_I = 0;
        pV34Fax->ubPPh_k++;
    }
}


void V34Fax_ALT_Tx(V34FaxStruct *pV34Fax)
{
    pV34Fax->uScramInbits = 0x2;      /* It should be 0x02, not 0x01 */
    pV34Fax->ubMphSent    = 0;

    if ((pV34Fax->Enter_CC_Order >= 1) && (pV34Fax->reRateFlag == 0) && (pV34Fax->E_detected))
    {
        pV34Fax->nTxDelayCnt = 0;
    }
}


void V34Fax_MPh_Tx(V34FaxStruct *pV34Fax)
{
    UBYTE idx;
    UBYTE Temp;

    pV34Fax->nTxDelayCnt = 10;

    /* Take 2 bits from the MPh bit stream */
    idx  = pV34Fax->ubBitCounter;
    Temp = (pV34Fax->pMPhByteBuf[pV34Fax->uByteCounter] >> idx) & 0x3;

    pV34Fax->uScramInbits = (UWORD)Temp;

    pV34Fax->ubBitCounter += 2;

    if (pV34Fax->ubBitCounter >= 8)
    {
        pV34Fax->ubBitCounter = 0;
        pV34Fax->uByteCounter++;
    }

    /* Reset MPh bit counter if reaching end of the bit buffer */
    /* uMPhSize is setup in MPhSetup function for TYPE0 or TYPE1 length. */
    if ((pV34Fax->ubBitCounter + (pV34Fax->uByteCounter << 3)) >= pV34Fax->uMPhSize)
    {
        pV34Fax->ubBitCounter = 0;
        pV34Fax->uByteCounter = 0;

        pV34Fax->ubMphSent++;

        if ((pV34Fax->ubMphDetect > 0) && (pV34Fax->ubMphSent >= 6))
        {
            /* This logic is not safe, call modem wait for 2 MPh or E detected. */
            pV34Fax->ubMphDetect = 0;
            pV34Fax->nTxDelayCnt = 0;
        }
    }
}


void V34Fax_E_Tx(V34FaxStruct *pV34Fax)
{
    pV34Fax->uScramInbits = 0x03;
}

void V34Fax_T30_Tx(V34FaxStruct *pV34Fax)
{
    if (pV34Fax->ONEs_DetCount > 40 && pV34Fax->modem_mode == ANS_MODEM)
    {
        pV34Fax->ONEs_DetCount  = 0;
        pV34Fax->nTxDelayCnt = 0;
    }
    else
    {
        UBYTE *pIORdBuf = (UBYTE *)pV34Fax->pTable[DSPIORDBUFDATA_IDX];
        UBYTE tx_bits;

        IoRdProcess(pV34Fax->pTable, pV34Fax->ubTxBitsPerSym);

        tx_bits = pIORdBuf[0]; /* the bits to transmit */

#if TRANSMITTED_DATA
        if (pV34Fax->Enter_CC_Order == TRANSMITTED_PAGE)
        {
            if (DumpTone1_Idx < 500000) { DumpTone1[DumpTone1_Idx++] = tx_bits & 1; }
            if (DumpTone1_Idx < 500000) { DumpTone1[DumpTone1_Idx++] = (tx_bits >> 1) & 1; }
        }
#endif

        pV34Fax->uScramInbits = tx_bits;

        pV34Fax->nTxDelayCnt = 100;       /* forever */
    }
}

#endif
