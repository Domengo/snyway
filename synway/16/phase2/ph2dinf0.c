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
/* Detect the INFO0 bit stream from the structure.                         */
/*                                                                         */
/* NOTES: Please refer to ITU V.34 p.28 to set up the structure            */
/*                                                                         */
/* INPUT: UBYTE bit     the input of the INFO_0 bit stream                 */
/*                                                                         */
/* OUTPUT: INFO_0 *I  the INFO_0 structure, filled with INFO_0 bit stream  */
/*                    received.                                            */
/*                                                                         */
/*         SBYTE Dectct_INFO_0(UBYTE bit,INFO_0 *I)                        */
/*         - the return value = -1 means the received number of bits is    */
/*           correct, but error occurs in transmitting bit information     */
/*         - the return value = 1 means the received number of bits is     */
/*           correct                                                       */
/*         - the return value = 0 means the received number of bits is     */
/*           not correct, error occurs in transmitting bit information     */
/*                                                                         */
/* Date: Jan 24, 1996.                                                     */
/* modified on: Feb 19, 1996.                                              */
/*              Mar 05, 1996. Benjamin Chan, take out fill_bits_0 and      */
/*              bit_num_set_0.                                             */
/*              Mar 06, 1996. Benjamin Chan, fixed last bit of CRC call    */
/*              April 29, 1996. Jack Liu, Porting to VXD                   */
/*              99Jun30   Wood  optimize                                   */
/* Author: Dennis, GAO Research & Consulting Ltd.                          */
/*-------------------------------------------------------------------------*/

#include "ph2ext.h"

void Ph2_Detect_Info_0_Init(Info_0Struc *pI0)
{
    pI0->bit_num_cntr_0 = 0;
    pI0->crc_0          = 0xFFFF;
#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)
    pI0->crc_mH = 0xFFFF;
#endif
}

#if SUPPORT_V90
CONST SBYTE ShiftTable0d[] = { 0, 0, 0,  0, 0, 0, 0, 0, 0, 2, 0, 0, 1,  0,  3, 4, 0, 0, 0, 0,  -1};
#endif

CONST SBYTE ShiftTable0[]  = { 0, 0, 0,  0, 0, 0, 0, 0, 0, 2, 0, 0, 1,  0, -1};

SBYTE Ph2_V34Detect_Info_0(UBYTE bit, Info_0Struc *pI0)
{
    SBYTE error;
    UBYTE *pData, *pInfo;

#if 0 /* To dump Info0 */

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = bit; }

#endif

    if (pI0->bit_num_cntr_0 < 32)
    {
        pI0->pData_sequence_0[pI0->bit_num_cntr_0] = bit;
        pI0->bit_num_cntr_0++;

        if (pI0->bit_num_cntr_0 <= 17) /* Not < 17 */
        {
            CRC16_Gen(&pI0->crc_0, bit);
        }

#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)

        if (Ph2_MH_Info_Detect(bit, pI0) == MH_SEQUENCE_DETECTED)
        {
            return(MH_SEQUENCE_DETECTED);
        }

#endif

        return (0);
    }
    else if (pI0->bit_num_cntr_0 == 32)
    {
        pI0->pData_sequence_0[pI0->bit_num_cntr_0] = bit;
        pData = pI0->pData_sequence_0;

        pInfo = (UBYTE *)pI0;           /* pI0->S2743 */
        Ph2_ShiftDataToInfo(pData, pInfo, ShiftTable0);

        pData = &(pI0->pData_sequence_0[32]);   /* point to the last crc bit */
        error = Ph2_ChkCrc(pData, pI0->crc_0);

        Ph2_Detect_Info_0_Init(pI0);
        return(error);
    }
    else
    {
        return (0);
    }
}


#if SUPPORT_V90A

SBYTE Ph2_Detect_Info_0d(UBYTE bit, Info_0Struc *pI0)
{
    SBYTE error;
    UBYTE *pData, *pInfo;

#if 0  /* for to dump INFO0d information */

    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = bit; }

#endif

    if (pI0->bit_num_cntr_0 < 49)
    {
        pI0->pData_sequence_0[pI0->bit_num_cntr_0] = bit;
        pI0->bit_num_cntr_0++;

        if (pI0->bit_num_cntr_0 <= 30)    /* Not < 30 */
        {
            CRC16_Gen(&pI0->crc_0, bit);
        }

#if SUPPORT_V92A_MOH

        if (Ph2_MH_Info_Detect(bit, pI0) == MH_SEQUENCE_DETECTED)
        {
            return(MH_SEQUENCE_DETECTED);
        }

#endif

        return (0);
    }
    else if (pI0->bit_num_cntr_0 == 49)
    {
        pI0->pData_sequence_0[pI0->bit_num_cntr_0] = bit;
        pData = pI0->pData_sequence_0;
        pInfo = (UBYTE *)pI0;            /* pI0->S2743 */
        Ph2_ShiftDataToInfo(pData, pInfo, ShiftTable0d);

        pData = &(pI0->pData_sequence_0[45]);
        error = Ph2_ChkCrc(pData, pI0->crc_0);

        Ph2_Detect_Info_0_Init(pI0);

        return(error);
    }
    else
    {
        return (0);
    }
}

#endif

#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)

UBYTE Ph2_MH_Info_Detect(UBYTE bit, Info_0Struc *pI0)
{
    UBYTE i, crc_check;

    if (pI0->bit_num_cntr_0 <= 8)
    {
        CRC16_Gen(&pI0->crc_mH, bit);
    }

    if (pI0->bit_num_cntr_0 == 28)
    {
        crc_check = Ph2_ChkCrc(&(pI0->pData_sequence_0[23]), pI0->crc_mH);

        if (crc_check == 1)
        {
            for (i = 0; i < 8; i++)
            {
                pI0->mH_byte |= pI0->pData_sequence_0[i] << i;
            }

            return(MH_SEQUENCE_DETECTED);
        }
        else
        {
            return(-1);    /* CRC ERROR */
        }
    }

    return(0);
}
#endif
