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

/*************************************************************************/
/* Detect the MP TYPE 0 OR TYPE 1 bit stream from the structure.         */
/*                                                                       */
/* NOTES: Please refer to ITU V.34 p.34-35 to set up the structure       */
/*                                                                       */
/* INPUT: UBYTE bit     the input of the MP bit stream                   */
/*                                                                       */
/* OUTPUT: MP *I the MP structure, filled with MP bit stream             */
/*         received.                                                     */
/*                                                                       */
/*         SBYTE  V34_Detect_Mp(UBYTE bit, MpStruc *pMp)                 */
/*         - the return value = -1 means the received number of bits is  */
/*           correct, but error occurs in transmitting bit information   */
/*         - the return value = 1 means the received number of bits is   */
/*           correct                                                     */
/*         - the return value = 0 means the received number of bits is   */
/*           not correct, error occurs in transmitting bit information   */
/*                                                                       */
/*         - If there is no error in receiving the sequence up to it #19, */
/*           the pointer *MP_type_info contains the information about the */
/*           MP, which is 1 for TYPE_1, and 0 for TYPE_0                 */
/*************************************************************************/

#include "v34ext.h"

#if 0 /// LLL temp
UWORD Mp_BitToByte(UBYTE *data_end, UBYTE n)
{
    UBYTE i;
    UWORD value;

    value = 0;

    for (i = 0; i < n; i++)
    {
        value = (value << 1) | *(--data_end);
    }

    return(value);
}

void V34_Detect_Mp_Init(MpStruc *pMp)
{
    pMp->bit_num_cntr_mp = 0;
    pMp->frame_sync_mp = 0;
    pMp->CRC_mp = 0xFFFF;
}


UBYTE *Detect_Mp_Common(MpStruc *pMp)
{
    UBYTE ubTemp;
    UBYTE *pData, *pDst;

    pData = pMp->pData_sequence_mp;
    pDst  = &pMp->max_call_to_ans_rate;  /* assume data are in order! */

    /* pData_sequence_mp[0] stored bit 17 of MP sequence Type 0 */
    /* and Type 1, which is a start bit */

    /* pData_sequence_mp[1] stored bit 18 of MP sequence Type 0 */
    /* and Type 1, which is the type of the MP sequence */

    /* pData_sequence_mp[2] stored bit 19 of MP sequence Type 0 */
    /* and Type 1, a bit reserved for ITU */
    /* bits */
    /* 17:19 */  pData += 3;                                    /* not used */
    /* 20:23 */  pData += 4;  *pDst++ = (UBYTE)Mp_BitToByte(pData, 4);
    /* 24:27 */  pData += 4;  *pDst++ = (UBYTE)Mp_BitToByte(pData, 4);
    /* 28    */               *pDst++ = *pData++;
    /* 29:30 */                ubTemp = *pData++;
    *pDst++ = ubTemp | (*pData++ << 1);
    /* 31 */                  *pDst++ = *pData++;
    /* 32 */                  *pDst++ = *pData++;
    /* 33 */                  *pDst++ = *pData++;
    /* 34 */     pData++ ;    /* pData_sequence_mp[17] is the start bit */

    /* 35:49 */  pData += 14; pMp->data_rate_cap_mask = Mp_BitToByte(pData, 14);
    /* 47:49 */  pData ++;                                    /* not used */
    /* 50 */                  pMp->asymmetric_data_sig_rate =  *pData++ ;

    return pData;
}

SBYTE V34_ChkCRC(MpStruc *pMp, SWORD idx)
{
    UWORD check_crc;
    SBYTE result_code;

    check_crc = Mp_BitToByte(&(pMp->pData_sequence_mp[idx]), 16);

    if (pMp->CRC_mp == check_crc)
    {
        result_code =  1;
    }
    else
    {
        result_code = -1;
    }

    V34_Detect_Mp_Init(pMp);

    return(result_code);
}

SBYTE  V34_Detect_Mp(UBYTE bit, MpStruc *pMp)
{
    UBYTE *pData;
    UBYTE U8_temp;
    UBYTE i;
    SWORD *pPrecodingCoef;

    if (pMp->frame_sync_mp != 0x0001ffffL)
    {
        pMp->frame_sync_mp = (((pMp->frame_sync_mp << 1) | bit) & 0x0001ffffL);

        return (0);
    }

    switch (pMp->bit_num_cntr_mp)
    {
        case 0:
            pMp->pData_sequence_mp[0] = bit & 1;
            pMp->bit_num_cntr_mp++;
            break;

        case 1:
            pMp->pData_sequence_mp[1] = bit & 1;
            CRC16_Gen(&pMp->CRC_mp, bit);
            pMp->bit_num_cntr_mp++;
            pMp->type = bit;
            break;

        default:

            if (pMp->type == MP_TYPE_1)
            {
                if (pMp->bit_num_cntr_mp < 170)
                {
                    pMp->pData_sequence_mp[pMp->bit_num_cntr_mp] = bit & 1;

                    switch (pMp->bit_num_cntr_mp)
                    {
                        case 17:
                        case 34:
                        case 51:
                        case 68:
                        case 85:
                        case 102:
                        case 119:
                        case 136:
                        case 153:
                            break;

                        default:

                            if (pMp->bit_num_cntr_mp < 153)
                            {
                                U8_temp = bit & 0x1;
                                CRC16_Gen(&pMp->CRC_mp, U8_temp);
                            }

                            break;
                    }   /* End switch */

                    pMp->bit_num_cntr_mp++;

                    return(0);
                }       /* End if bit_num_cntr_mp < 170 */
                else if (pMp->bit_num_cntr_mp == 170)
                {
                    pMp->pData_sequence_mp[pMp->bit_num_cntr_mp] = bit & 1;

                    pData = Detect_Mp_Common(pMp);

                    /* If it is type 1, more incoming bits has to get */
                    /* pData_sequence_mp[35] corresponds to bit #52 in MP sequence */

                    /* assume destination address are continuous!! */
                    pPrecodingCoef = &pMp->PrecodeCoeff_h[0].r;

                    for (i = 0; i < 6; i++)
                    {
                        pData++; /* start bit */
                        pData += 16;
                        *pPrecodingCoef++  = Mp_BitToByte(pData, 16);
                    }

                    return (V34_ChkCRC(pMp, 169 + 1));
                }       /* End if bit_num_cntr_mp < 170 */
            }           /* End TYPE 1 bit extracting */
            else if (pMp->type == MP_TYPE_0)
            {
                if (pMp->bit_num_cntr_mp < 70)
                {
                    pMp->pData_sequence_mp[pMp->bit_num_cntr_mp] = bit;

                    switch (pMp->bit_num_cntr_mp)
                    {
                        case 17:
                        case 34:
                        case 51:
                            break;

                        default:

                            if (pMp->bit_num_cntr_mp < 51)
                            {
                                CRC16_Gen(&pMp->CRC_mp, bit);
                            }

                            break;
                    }

                    pMp->bit_num_cntr_mp++;

                    return(0);
                }       /* End if bit_num_cntr_mp < 70 */
                else if (pMp->bit_num_cntr_mp == 70)
                {
                    pMp->pData_sequence_mp[pMp->bit_num_cntr_mp] = bit;
                    pMp->bit_num_cntr_mp++;

                    Detect_Mp_Common(pMp);

                    /* bit 68 = pData_sequence_mp[51] is the start bit, */
                    /* do not go into CRC */

                    return V34_ChkCRC(pMp, 67 + 1);

                }       /* End if bit_num_cntr_mp == 70 */
            }           /* End if TYPE 0 */
    }               /* End Switch */

    return (0);
}
#endif