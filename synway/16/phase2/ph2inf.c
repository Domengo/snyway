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
/*  Create the INFO  bit stream from all INFOxx structures.                */
/*                                                                         */
/* NOTES: Please refer to ITU V.34 p.28 to set up the structure            */
/*                                                                         */
/* INPUT: UBYTE *pB         the output buffer of the INFO0 bit stream      */
/*        INFO_0 *pI      the INFO structure, user need to setup the info  */
/*                        in the structure before calling this function.   */
/*                                                                         */
/* OUTPUT: info_buf is filled with INFO0 bit stream located in the LSB     */
/*         of each data word.                                              */
/*                                                                         */
/* Author: Dennis, GAO Research & Consulting Ltd.                          */
/* Date: Jan 17, 1996.                                                     */
/*       Feb 28, 1996. Change to use shifter register, Benjamin Chan       */
/*       April 29, 1996. Jack Liu Porting to VXD lib                       */
/* 99Jun15 Wood    modified from Ph2Inf0.c                                 */
/*                 change to adapt all INFO bit structures                 */
/*                 This change is intended for Assembly implementation     */
/*                 so the 3 structures may need re-organization s.t.       */
/*                 the offsets of related variables match.                 */
/*-------------------------------------------------------------------------*/

#include "ph2ext.h"

void Ph2_CreateInfo(UBYTE *pB, UBYTE *pI, CONST SBYTE *pST)
{
    UWORD crc;
    UBYTE ss;        /* shifter; */
    SWORD sx;        /* shifter 16 */
    SBYTE shift;
    UWORD pattern;
    UBYTE i;
    SWORD *psI;

    /* -- stuff the Fill "1111" and Frame sync bits -- */
#if 1
    pattern = 0x4EFF;                /* 0100,1110,1111,1111 -- Frame sync & Fill bits, 4 more bits to fill PSF delay line*/

    for (i = 0; i < 16; i++)        /* change this loop to sub-routine */
    {
        *pB++ = (pattern & 0x01);
        pattern >>= 1;
    }

#else
    pattern = 0x13BF;                /* 0001,0011,1011,1111 -- Frame sync & Fill bits, 4 more bits to fill PSF delay line*/

    for (i = 0; i < 14; i++)        /* change this loop to sub-routine */
    {
        *pB++ = (pattern & 0x01);
        pattern >>= 1;
    }

#endif

    crc = 0xFFFF;

    while ((shift = *pST++) >= 0)
    {
        if (shift < 8)                /* in byte format */
        {
            ss  = *pI++;

            for (i = 0; i <= shift; i++)
            {
                *pB = (ss & 0x1);

                CRC16_Gen(&crc, *pB++);

                ss >>= 1;
            }
        }
        else
        {
            psI = (SWORD *)pI;
            sx  = *psI;
            /* Must be the last one, otherwise pI += 1 or pI += 2, depending on platform */

            for (i = 0; i <= shift; i++)
            {
                *pB = (UBYTE)(sx & 0x1);

                CRC16_Gen(&crc, *pB++);

                sx >>= 1;
            }
        }
    }

    for (i = 0; i < 16; i++)
    {
        *pB++ = (crc & 0x1);
        crc >>= 1;
    }

    for (i = 0; i < 4; i++)    /* fill bits */
    {
        *pB++ = 1;
    }
}

#if SUPPORT_V90A
void Ph2_SetupInfo1A_V90A(Ph2Struct *pPH2)
{
    Info_1aStruc *pI1a = &(pPH2->I1a);

    if (pPH2->V90_Enable)
    {
        if ((pI1a->sym_rate_call_to_answer >= V34_SYM_3000) && (pPH2->Retrain_V90 != 1))
        {
            pI1a->sym_rate_call_to_answer = V90_SYM_8000;
            pI1a->sym_rate_answer_to_call = V34_SYM_3200;
            pI1a->INFO1a_Bit25            = 78; /* PCM_Ucode_for2pt_train */
            pI1a->min_power_reduction     = 0;
            pI1a->add_power_reduction     = 0;
            pI1a->pre_em_index            = 0;
            pI1a->pro_data_rate           = 0;
        }
        else
        {
            pPH2->V90_Enable = 0;
        }

        Ph2_CreateInfo(pPH2->pInfo_buf1A, (UBYTE *)&(pI1a->min_power_reduction), ShiftTable1a_V90);
    }

#if SUPPORT_V92A

    if (pPH2->V92_Enable)
    {
        pI1a->min_power_reduction = 0;    /* Filter Sections Bits 12:13 */
        pI1a->add_power_reduction = 1;    /* Max Coeffs Bits 14:15 and 16:17 */
        pI1a->length_MD           = 0;  /* Bits 18:24 */

        if (pPH2->ubDownstreamPowerIdx == 0)
        {
            pI1a->INFO1a_Bit25 = 78;
        }
        else if (pPH2->ubDownstreamPowerIdx == 1)
        {
            pI1a->INFO1a_Bit25 = 69;
        }
        else if (pPH2->ubDownstreamPowerIdx == 2)
        {
            pI1a->INFO1a_Bit25 = 62;
        }

#if 0
        //    else if (pPH2->ubDownstreamPowerIdx == 3)
        //        pI1a->INFO1a_Bit25            = 85;
#endif

        pI1a->pre_em_index            = 0;            /* Reserved Bit 32 */
        pI1a->pro_data_rate           = 0;            /* Reserved Bit 33 */
        pI1a->sym_rate_call_to_answer = V90_SYM_8000; /* Bits 34: 36 */
        pI1a->sym_rate_answer_to_call = V90_SYM_8000; /* Bits 37: 39 */

        Ph2_CreateInfo(pPH2->pInfo_buf1A, (UBYTE *)&(pI1a->min_power_reduction), ShiftTable1a_V92);
    }

#endif /* SUPPORT_V92A */
}
#endif /* SUPPORT_V90A */
