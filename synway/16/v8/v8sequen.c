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

#include "v8ext.h"
#include "v251.h"

UWORD V8_Recv_Bits(V8Struct *pV8)
{
    SBYTE bit;

    bit = V8_V21Receive(pV8);

    if (bit == -1)
    {
        return (0xFFFF);
    }
    else
    {
#if 0/* V8 informance BIT */

        if (DumpTone1_Idx < 10000000) { DumpTone1[DumpTone1_Idx++] = bit; }

#endif

        pV8->cur_bit_pattern = (((UDWORD)pV8->cur_bit_pattern << 1) | bit) & 0xFFFF;

        /* Mask for only 11 bits */
        return ((UWORD)(pV8->cur_bit_pattern & 0x07ff));
    }
}

#if USE_ANS
void V8_Detect_CJ(V8Struct *pV8)
{
    UBYTE cjDetected = 0;
    V21Struct *pV21 = &(pV8->v21);
    UBYTE **pTable = pV21->pTable;
    SBYTE bit;

    bit = V8_V21Receive(pV8);

    pV8->CJ_timeout_Count ++;

    if (bit == -1)
    {
        return;
    }

#if 0 /* V8 informance BIT */

    if (DumpTone1_Idx < 10000000) { DumpTone1[DumpTone1_Idx++] = bit; }

    if (DumpTone8_Idx < 10000000) { DumpTone8[DumpTone8_Idx++] = pV8->CJ_Return; }

#endif

    if (bit == 0)
    {
        pV8->CJ_Return++;
    }
    else /* bit == 1 */
    {
        if (pV8->CJ_Return == 9)
        {
            pV8->receive_state++;
        }
        else
        {
            if ((pV8->CJ_Return > 9) && (pV8->receive_state == 0))
            {
                pV8->receive_state++;
            }
            else
            {
                pV8->receive_state = 0;
            }
        }

        pV8->CJ_Return = 0;
    }

    if ((pV8->receive_state == 2) || (pV8->CJ_timeout_Count > 600))
    {
        pV8->CJ_timeout_Count = 0;
        cjDetected = 1;
    }

    if (cjDetected)
    {
        UBYTE a8j = 1;

        V8_SetResponse(pTable, V8_A8J, &a8j, 1);
        pV8->phase1_end = 1;
#if SUPPORT_V34FAX

        if (!pV8->JM_Octets[38])
        {
            pV8->V34fax_Enable = 0;
        }

#endif
        TRACE0("V8: CJ detected");
    }
}
#endif


UBYTE V8_Recv_Seq(V8Struct *pV8, UBYTE which_seq, SWORD *octet_string)
{
    UWORD r_bits;

    r_bits = V8_Recv_Bits(pV8);

    if (r_bits == 0xFFFF)
    {
        return (0);
    }

    switch (pV8->receive_state)
    {
        case 0:

            if (((which_seq == V8_CJ_TYPE) && ((r_bits & 1) == 0)) || ((which_seq != V8_CJ_TYPE) && (r_bits == 0x07FE)))    /* 0x7FE refer Table 1/V.8--Preamble (Ten ONEs preceding each information sequence) */
            {
                pV8->bit_clock     = 9;
                pV8->receive_state = 1;
                pV8->pString_ptr   = octet_string;
            }

            break;

        case 1:

            /* Unframe it to get a byte */
            if ((--pV8->bit_clock) != 0)
            {
                break;
            }

            /* Check the stop bit */
            if ((r_bits & 1) == 0)
            {
                /* Framing error */
                pV8->receive_state = 0;
                return 2;
            }

            /* the sync octet is received */

#if SUPPORT_V92D

            /* because V21 does not detect signal energy, wrong sync byte detected before real Sync */
            /* for USR & Zoom, think later, if check 0xAA only for QC1a */
            /* if (((r_bits & 0x01ff) == 0x01ff) || (r_bits & 0x01ff) == 0x0000)) */
            if ((r_bits & 0x01ff) == 0x01ff)
            {
                pV8->receive_state = 0;
                return 2;
            }

#endif
            *pV8->pString_ptr++ = (SWORD)((r_bits >> 1) & 0xFF);
            pV8->receive_state = 2;

            /* Wait for two more stop bits to confirm and string is ended */
            pV8->bit_clock = 2;
            break;

        case 2:

            /* Look for start bit again */
            if ((r_bits & 1) == 0)
            {
                pV8->bit_clock = 9;
                pV8->receive_state = 1;
                break;
            }
            else   /* String probably is finished */
            {
                if ((--pV8->bit_clock) == 0)
                {
                    pV8->cur_bit_pattern = 0;

                    *pV8->pString_ptr++ = 0x7E;

                    pV8->receive_state = 0;
                    return 1;
                }
            }
    }

    return 0;
}
