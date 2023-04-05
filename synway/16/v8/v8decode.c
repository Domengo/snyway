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

void V8_Analy(V8Struct *pV8)
{
    UBYTE i;

    for (i = 2; i < 10; i++)
    {
        if ((pV8->pRecv_string1[i] & V8_MODN0_OCTET_MASK) == V8_MODN0_OCTET)    /* Octet: modn0 */
        {
#if SUPPORT_V90

            if ((pV8->pRecv_string1[i] & V8_MODN0_V90_DISABLE_MASK) == V8_MODN0_V90_DISABLE)
            {
                pV8->V90_Enable = 0;
                pV8->V92_Enable = 0;
            }

#endif
#if SUPPORT_V34

            if ((pV8->pRecv_string1[i] & V8_MODN0_V34_DUPLEX_MASK) == V8_MODN0_V34_DUPLEX)
            {
                pV8->V34_Duplex = 1;
            }

#endif

#if SUPPORT_V34FAX

            if ((pV8->pRecv_string1[i] & V8_MODN0_V34_HDX_MASK) == V8_MODN0_V34_HDX)
            {
                pV8->JM_Octets[38] = 1;
            }

#endif

            i++;

            if ((pV8->pRecv_string1[i] & V8_MODN1_OCTET_MASK) == V8_MODN1_OCTET) /* Octet: modn1 */
            {
                if ((pV8->pRecv_string1[i] & V8_MODN1_V32_ENABLE_MASK) == V8_MODN1_V32_ENABLE)
                {
                    pV8->V32_Enable = 1;
                }

                if ((pV8->pRecv_string1[i] & V8_MODN1_V22_ENABLE_MASK) == V8_MODN1_V22_ENABLE)
                {
                    pV8->V22_Enable = 1;
                }

                i++;

                if ((pV8->pRecv_string1[i] & V8_MODN2_OCTET_MASK) == V8_MODN2_OCTET) /* Octet: modn2 */
                {
                    if ((pV8->pRecv_string1[i] & V8_MODN2_V21_ENABLE_MASK) == V8_MODN2_V21_ENABLE)
                    {
                        pV8->V21_Enable = 1;
                    }

                    i++;
                }
            }
        }

#if SUPPORT_V90A

        if (((pV8->pRecv_string1[i] & V8_V90D_AVAIL_OCTET_MASK) == V8_V90D_AVAIL_OCTET)  /* V90 avai octet.  No digital modem avai ? */
#if SUPPORT_V90D
            && (pV8->modem_mode == CALL_MODEM)
#endif
           )
        {
            pV8->V90_Enable = 0;
            pV8->V92_Enable = 0;
        }

#endif

#if SUPPORT_V90D

        if (((pV8->pRecv_string1[i] & V8_V90A_AVAIL_OCTET_MASK) == V8_V90A_AVAIL_OCTET) /* V90 avai octet.  No analog modem avai ? */
#if SUPPORT_V90A
            && (pV8->modem_mode == ANS_MODEM)
#endif
           )
        {
            pV8->V90_Enable = 0;
            pV8->V92_Enable = 0;
        }

#endif

        if ((pV8->pRecv_string1[i]) == V8_END_OF_RECEIVE_OCTET)
        {
            break;    /* End of Recv_string  */
        }
    }
}
