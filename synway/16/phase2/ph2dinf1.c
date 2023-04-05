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

/*-------------------------------------------------------------------------*/
/* Detect the INFO1c bit stream from the structure.                        */
/*                                                                         */
/* NOTES: Please refer to ITU V.34 p.29 to set up the structure            */
/*                                                                         */
/* INPUT: UBYTE bit     the input of the INFO_1C bit stream                */
/*                                                                         */
/* OUTPUT: INFO_1C *I the INFO_1C structure, filled with INFO_1C bit stream*/
/*                    received.                                            */
/*                                                                         */
/*         UBYTE Dectct_INFO_1C(UBYTE bit,INFO_1C *I)                      */
/*         - the return value = -1 means the received number of bits is    */
/*           correct, but error occurs in transmitting bit information     */
/*         - the return value = 1 means the received number of bits is     */
/*           correct                                                       */
/*         - the return value = 0 means the received number of bits is     */
/*           not correct, error occurs in transmitting bit information     */
/*-------------------------------------------------------------------------*/

#include "ph2ext.h"

UBYTE Ph2_ShiftData(UBYTE *pData, UBYTE shift)   /* in byte format */
{
    UBYTE result;
    UBYTE i;

    result = *pData++;

    for (i = 1; i <= shift; i++)
    {
        result |= ((*pData++) << i);
    }

    return result;
}

UWORD ShiftWData(UBYTE *pData, UBYTE shift)
{
    /* in word format */
    /* not required for the DSPs */
    /* that are always 16-bit data */

    UWORD result;
    UBYTE i;

    result = *pData++;

    for (i = 1; i <= shift; i++)
    {
        result |= ((UWORD)(*pData++) << i);
    }

    return result;
}

SBYTE Ph2_ChkCrc(UBYTE *pData, UWORD ref)
{
    UWORD check_crc;
    UBYTE i;

    check_crc = 0;

    for (i = 0; i < 16; i++)
    {
        check_crc <<= 1;
        check_crc  |= ((*pData--) & 0x1);
    }

    if (ref == check_crc)
    {
        return 1;
    }
    else
    {
        return -1;    /* (SBYTE)0xFF; */
    }
}

#if SUPPORT_V90A
CONST SBYTE ShiftTable1a_V90[] = { 2, 2, 6, 6, 0, 0, 2, 2, 9, -1 }; /* rectifiec V90 1a structure */
#if SUPPORT_V92A
CONST SBYTE ShiftTable1a_V92[] = { 1, 3, 6, 6, 0, 0, 2, 2, 9, -1 };
#endif
#endif

CONST SBYTE ShiftTable1a[] = { 2, 2, 6, 0, 3, 3, 2, 2, 9, -1 } ;
CONST SBYTE ShiftTable1c[] = { 2, 2, 6, 0, 3, 3,
                               0, 3, 3,
                               0, 3, 3,
                               0, 3, 3,
                               0, 3, 3,
                               0, 3, 3, 9, -1
                             } ;

void Ph2_ShiftDataToInfo(UBYTE *pData, UBYTE *pInfo, CONST SBYTE *pShiftTable)
{
    SBYTE shift;
    SWORD *pIw;

    while ((shift = *pShiftTable++) >= 0)
    {
        if (shift < 8)
        {
            *pInfo++ = Ph2_ShiftData(pData, shift);
        }
        else
        {
            /* !! N.B. this is always the last one, freq_offset */
            pIw  = (SWORD *)pInfo;
            *pIw = (SWORD)ShiftWData(pData, shift);
            *pIw = ((SWORD)(*pIw << 6) >> 6);/* Sign extension for freq_offset */
        }

        pData += (shift + 1);
    }
}

void Ph2_Detect_Info_1c_Init(Info_1cStruc *pI1c)
{
    pI1c->bit_num_cntr_1c = 0;
    pI1c->crc_1c          = 0xFFFF;
}

#if 0 /// LLL temp
SBYTE Ph2_V34Detect_Info_1c(UBYTE bit, Info_1cStruc *pI1c)
{
    SBYTE error;
    UBYTE *pData, *pInfo;

#if 0 /* for dump InfoIc */
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++]  = bit; }
#endif

    if (pI1c->bit_num_cntr_1c < 92)  /* total number of bits from 12-104 is 93 */
    {
        pI1c->pData_sequence_1c[pI1c->bit_num_cntr_1c] = bit;

        pI1c->bit_num_cntr_1c++;

        if (pI1c->bit_num_cntr_1c <= 77)
        {
            CRC16_Gen(&pI1c->crc_1c, bit);
        }

        return (0);
    }
    else if (pI1c->bit_num_cntr_1c == 92)
    {
        pI1c->pData_sequence_1c[pI1c->bit_num_cntr_1c] = bit;
        pData = pI1c->pData_sequence_1c;

        pInfo = &pI1c->min_power_reduction;
        Ph2_ShiftDataToInfo(pData, pInfo, ShiftTable1c);

        pData = &(pI1c->pData_sequence_1c[92]); /* point to the last crc bit */
        error = Ph2_ChkCrc(pData, pI1c->crc_1c);

        Ph2_Detect_Info_1c_Init(pI1c);

        return(error);
    }
    else
    {
        return(0);
    }
}
#endif

void Ph2_Detect_Info_1a_Init(Info_1aStruc *pI1a)
{
    pI1a->bit_num_cntr_1a = 0;
    pI1a->crc_1a          = 0xFFFF;
}

#if 0 /// LLL temp
SBYTE Ph2_V34Detect_Info_1a(UBYTE bit, Info_1aStruc *pI1a)
{
    SBYTE error;
    UBYTE *pData, *pInfo;

    error = 0;

    if (pI1a->bit_num_cntr_1a < 53)  /* total number of bits from 12-65 is 54 */
    {
#if 0 /* To dump Info1a */
        if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = bit; }
#endif
        pI1a->pData_sequence_1a[pI1a->bit_num_cntr_1a] = bit;
        pI1a->bit_num_cntr_1a++;

        if (pI1a->bit_num_cntr_1a <= 38) /* Not < 38 */
        {
            CRC16_Gen(&pI1a->crc_1a, bit);
        }

        return (0);
    }
    else if (pI1a->bit_num_cntr_1a == 53)
    {
        pI1a->pData_sequence_1a[pI1a->bit_num_cntr_1a] = bit;
        pData = pI1a->pData_sequence_1a;

        pInfo = &pI1a->min_power_reduction;   /* beginning of Info content */
        Ph2_ShiftDataToInfo(pData, pInfo, ShiftTable1a);

        pData = &(pI1a->pData_sequence_1a[53]);  /* point to the last crc bit */

        error = Ph2_ChkCrc(pData, pI1a->crc_1a);

        Ph2_Detect_Info_1a_Init(pI1a);
    }

    return(error);
}
#endif

#if SUPPORT_V90D
SBYTE Ph2_V90dDetect_Info_1a(UBYTE bit, Info_1aStruc *pI1a)
{
    SBYTE error;
    UBYTE *pData;

    error = 0;

    if (pI1a->bit_num_cntr_1a < 54) /* was 57, total number of bits from 12-69 is 57 */
    {
        pI1a->pData_sequence_1a[pI1a->bit_num_cntr_1a] = bit;
        pI1a->bit_num_cntr_1a++;

        if (pI1a->bit_num_cntr_1a <= 38) /* Not < 38 */
        {
            CRC16_Gen(&pI1a->crc_1a, bit);
        }

        return (0);
    }
    else if (pI1a->bit_num_cntr_1a == 54)                   /* was 57 */
    {
        pI1a->pData_sequence_1a[pI1a->bit_num_cntr_1a] = bit;
        pData = pI1a->pData_sequence_1a;
        pI1a->min_power_reduction  =  *pData++;                /* bit 12-14 */
        pI1a->min_power_reduction |= (*pData++ << 1);
        pI1a->min_power_reduction |= (*pData++ << 2);

        pI1a->add_power_reduction =   *pData++;                /* bit 15-17 */
        pI1a->add_power_reduction |= (*pData++ << 1);
        pI1a->add_power_reduction |= (*pData++ << 2);

        pI1a->length_MD  =  *pData++;                        /* bit 18-24 */
        pI1a->length_MD |= (*pData++ << 1);
        pI1a->length_MD |= (*pData++ << 2);
        pI1a->length_MD |= (*pData++ << 3);
        pI1a->length_MD |= (*pData++ << 4);
        pI1a->length_MD |= (*pData++ << 5);
        pI1a->length_MD |= (*pData++ << 6);

        pI1a->INFO1a_Bit25  =   *pData++;                    /* bit 25-31 */
        pI1a->INFO1a_Bit25 |= (*pData++ << 1);
        pI1a->INFO1a_Bit25 |= (*pData++ << 2);
        pI1a->INFO1a_Bit25 |= (*pData++ << 3);
        pI1a->INFO1a_Bit25 |= (*pData++ << 4);
        pI1a->INFO1a_Bit25 |= (*pData++ << 5);
        pI1a->INFO1a_Bit25 |= (*pData++ << 6);
        pData++;                                            /* bit 32 */
        pData++;                                            /* bit 33 */
        pI1a->sym_rate_answer_to_call  =  *pData++;            /* bit 34-36 */
        pI1a->sym_rate_answer_to_call |= (*pData++ << 1);
        pI1a->sym_rate_answer_to_call |= (*pData++ << 2);

        pI1a->sym_rate_call_to_answer  =  *pData++;            /* bit 37-39 */
        pI1a->sym_rate_call_to_answer |= (*pData++ << 1);
        pI1a->sym_rate_call_to_answer |= (*pData++ << 2);

        pI1a->freq_offset =   *pData++;                        /* bit 40-49 */
        pI1a->freq_offset |= (*pData++ << 1);
        pI1a->freq_offset |= (*pData++ << 2);
        pI1a->freq_offset |= (*pData++ << 3);
        pI1a->freq_offset |= (*pData++ << 4);
        pI1a->freq_offset |= (*pData++ << 5);
        pI1a->freq_offset |= (*pData++ << 6);
        pI1a->freq_offset |= (*pData++ << 7);
        pI1a->freq_offset |= (*pData++ << 8);
        pI1a->freq_offset |= (*pData   << 9);

        pData = &(pI1a->pData_sequence_1a[53]);                /* point to the last crc bit */

        error = Ph2_ChkCrc(pData, pI1a->crc_1a);

        Ph2_Detect_Info_1a_Init(pI1a);
    }

    return(error);
}
#endif

#if SUPPORT_V34FAX
CONST SBYTE ShiftTableh[] = { 2, 6, 0, 3, 2, 0, -1 };

void  Ph2_Detect_Info_h_Init(Info_hStruc *pIh)
{
    pIh->bit_num_cntr_h = 0;
    pIh->crc_h          = 0xFFFF;
}


SBYTE  Ph2_V34Detect_Info_h(UBYTE bit, Info_hStruc *pIh)
{
    SBYTE error;
    UBYTE *pData, *pInfo;

#if 0
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = bit; }
#endif

    if (pIh->bit_num_cntr_h < 34)    /* total number of bits from 12-46 is 35 */
    {
        pIh->pData_sequence_h[pIh->bit_num_cntr_h] = bit;
        pIh->bit_num_cntr_h++;

        if (pIh->bit_num_cntr_h <= 19) /* 19 is the bits from 12-30 */
        {
            CRC16_Gen(&pIh->crc_h, bit);
        }

        return (0);
    }
    else if (pIh->bit_num_cntr_h == 34)
    {
#if 0
        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = 100; }
#endif
        pIh->pData_sequence_h[pIh->bit_num_cntr_h] = bit;
        pData = &(pIh->pData_sequence_h[34]);   /* point to the last crc bit */
        error = Ph2_ChkCrc(pData, pIh->crc_h);   /* backward to check CRC */

        if (error == 1)  /* CRC ok */
        {
            pData = pIh->pData_sequence_h;
            pInfo = &pIh->power_reduction;   /* beginning of Info content */
            Ph2_ShiftDataToInfo(pData, pInfo, ShiftTableh);
#if 0
            if (DumpTone5_Idx < 10000)
            {
                UWORD i;

                for (i = 0; i < 35; i++)
                {
                    DumpTone5[DumpTone5_Idx++] = pIh->pData_sequence_h[i];
                }
            }
#endif
        }

        Ph2_Detect_Info_h_Init(pIh);

        return(error);
    }

    return(0);
}
#endif
