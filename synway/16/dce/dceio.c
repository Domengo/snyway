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

#include "ptable.h"
#include "dceext.h"

#if BER_CALC
extern UDWORD udZeroCount;
extern UDWORD udTotalCount;
#endif

/***********************************
*        Transfer data to data pump
************************************/
void IoRdProcess(UBYTE **pTable, UWORD NumBits)
{
    UBYTE *pBuf = (UBYTE *)pTable[DSPIORDBUFDATA_IDX];
    DceStruct *pDCE = (DceStruct *)pTable[DCE_STRUC_IDX];
    UWORD BitMask;
    UBYTE RdChar;

    if (pDCE->numBitRd >= 8)
    {
        if (NumBits >= 8)
        {
            *(pBuf++) = (UBYTE)(pDCE->bitBufRd & 0x00FF);

            pDCE->bitBufRd >>= 8;
            pDCE->numBitRd  -= 8;
            NumBits         -= 8;
        }
        else if (NumBits > 0)
        {
            BitMask   = ((UWORD)1 << NumBits) - 1;
            *(pBuf++) = (UBYTE)(pDCE->bitBufRd & BitMask);

            pDCE->bitBufRd >>= NumBits;
            pDCE->numBitRd  -=  NumBits;
            return;
        }
    }

    /* write bytes into modem buffer */
    while (NumBits >= 8)
    {
        /* Get new byte from host: in buffer less than one byte */
        pDCE->pfDCETxVec(pTable, &RdChar);

#if 0/* data from V42 to modem */

        if (DumpTone1_Idx < 10000000) { DumpTone1[DumpTone1_Idx++] = RdChar; }

#endif

        pDCE->bitBufRd  |= (((UWORD)RdChar & 0x00FF) << pDCE->numBitRd);

        /* move data from DCE buffer to modem buffer */
        *(pBuf++) = (UBYTE)(pDCE->bitBufRd  & 0x00FF);

        pDCE->bitBufRd >>= 8;
        NumBits         -= 8;
    }

    /* write remaining bits into modem buffer */
    if (NumBits > 0)
    {
        if (pDCE->numBitRd < NumBits)
        {
            /* get new byte from host */
            pDCE->pfDCETxVec(pTable, &RdChar);

#if 0/* data from V42 to modem */
            if (DumpTone1_Idx < 10000000) { DumpTone1[DumpTone1_Idx++] = RdChar; }
#endif

            pDCE->bitBufRd |= (((UWORD)RdChar & 0x00FF) << pDCE->numBitRd);

            pDCE->numBitRd += 8;
        }

        /* calculate bit-mask for last set of bits */
        BitMask = ((UWORD)1 << NumBits) - 1;

        /* move data from 'DCE' buffer to modem buffer */
        *(pBuf++) = (UBYTE)(pDCE->bitBufRd  & BitMask);

        pDCE->bitBufRd >>= NumBits;
        pDCE->numBitRd -= NumBits;
    }
}

/*******************************
* Transfer data from data pump
********************************/
void IoWrProcess(UBYTE **pTable, UWORD NumBits)
{
    UBYTE *pBuf = (UBYTE *)pTable[DSPIOWRBUFDATA_IDX];
    DceStruct *pDCE = (DceStruct *)pTable[DCE_STRUC_IDX];
    UWORD    BitMask;
    UWORD    Temp;
    UBYTE    WrChar;
#if BER_CALC
    UBYTE ubTemp, i;
#endif

    /* read bytes from modem buffer */
    while (NumBits >= 8)
    {
        if (pDCE->numBitWr < 8)
        {
            /* put byte from modem into buffer */
            Temp = (UWORD)(*pBuf++) & 0x00ff;

#if BER_CALC
            ubTemp = Temp;

            for (i = 0; i < 8; i++)
            {
                if (udTotalCount < 0xfffffffe)
                {
                    udTotalCount++;

                    if ((ubTemp & 0x01) == 0)
                    {
                        udZeroCount++;
                    }
                }

                ubTemp >>= 1;
            }

#endif

            pDCE->bitBufWr |= (Temp << pDCE->numBitWr);
            pDCE->numBitWr += 8;
            NumBits        -= 8;
        }

        /* while 8 bits or more in buffer, send byte(s) to host */
        WrChar = (UBYTE)(pDCE->bitBufWr & 0x00ff);
        /* Write the byte to the host   */

#if 0/* data from modem to V42 */

        if (DumpTone3_Idx < 10000000) { DumpTone3[DumpTone3_Idx++] = WrChar; }

#endif

        pDCE->pfDCERxVec(pTable, WrChar);

        pDCE->bitBufWr   >>= 8;
        pDCE->numBitWr    -= 8;
    }

    /* read remaining bits from modem buffer */
    if (NumBits > 0)
    {
        /* calculate bit-mask for last set of bits */
        BitMask = ((UWORD)1 << NumBits) - 1;

        /* put last bits from modem into buffer */
        Temp = (UWORD)(*pBuf) & BitMask;

#if BER_CALC
        ubTemp = Temp;

        for (i = 0; i < NumBits; i++)
        {
            if (udTotalCount < 0xfffffffe)
            {
                udTotalCount++;

                if ((ubTemp & 0x01) == 0)
                {
                    udZeroCount++;
                }
            }

            ubTemp >>= 1;
        }

#endif

        pDCE->bitBufWr |= (Temp << pDCE->numBitWr);
        pDCE->numBitWr += NumBits;

        /* while 8 bits or more in buffer, send byte(s) to host */
        if (pDCE->numBitWr >= 8)
        {
            WrChar = (UBYTE)(pDCE->bitBufWr & 0x00ff);

#if 0/* data from modem to V42 */
            if (DumpTone4_Idx < 10000000) { DumpTone4[DumpTone4_Idx++] = WrChar; }
#endif
            pDCE->pfDCERxVec(pTable, WrChar);

            pDCE->bitBufWr >>= 8;
            pDCE->numBitWr  -= 8;
        }
    }
}
