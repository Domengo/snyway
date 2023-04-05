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

#if USE_ANS

#if SUPPORT_V92D
void V8_Pre_QCA1d(V8Struct *pV8)      /* -- prepare QCA1d -- */
{
    SBYTE i;
    UBYTE j;
    UBYTE oct_idx;         /* index for local octet      */
    UWORD QCA1d_Octets[7];

    QCA1d_Octets[0] = 0x3FF;
    QCA1d_Octets[1] = 0x2AA;
    QCA1d_Octets[2] = 0x206;     /* -9.5dB */
    QCA1d_Octets[3] = 0x3FF;
    QCA1d_Octets[4] = 0x2AA;
    QCA1d_Octets[5] = 0x206;     /* -9.5dB */
    QCA1d_Octets[6] = 0x3FF;

    pV8->V8_QCA1d_Length = V8_QCA1d_LENGTH;
    pV8->cur_send_bit_index = 0;

    oct_idx = 0;

    for (i = 0; i < 7; i++)
    {
        for (j = 0; j < 10; j++)
        {
            pV8->V8_QCA1d_Octets[oct_idx++] = QCA1d_Octets[i] & 1;
            QCA1d_Octets[i] >>= 1;
        }
    }
}
#endif

void V8_Pre_JM(V8Struct *pV8)      /* -- prepare JM -- */
{
    SWORD rsj;
    UBYTE j, oct_idx, jm_idx, temp;
    UBYTE octet[3];
    SBYTE i;

    for (i = 0; i < 30; i++)                        /* duplicate leading bits, */
    {
        pV8->JM_Octets[i] = pV8->CM_Octets[i];    /* sync bits and call      */
    }

    /* Modulation modes */
    for (j = 0; j < 3; j++)                            /* duplicate modulation modes octects */
    {
        /* shift them into bytes       */
        octet[j] = 0;

        temp = UBUBMULUB(3 + j, 10);

        /* !! note a trick below !!    */
        for (i = 1; i < 9; i++)
        {
            octet[j] = (UBYTE)((octet[j] << 1) + pV8->CM_Octets[i + temp]);
        }
    }

    oct_idx = 0;

    for (j = 0; j < 8; j++)
    {
        rsj = pV8->pRecv_string1[j];

        if ((rsj & 0xf8) == 0xa0) /* modulation octet */
        {
#if SUPPORT_V90

            if (!pV8->V90_Enable && !pV8->V92_Enable)
            {
                rsj &= 0xfb;    /* mask V.90/V.92 avai */
            }
            else
            {
                octet[0] |= 0x04;
            }

#endif
            octet[oct_idx++] &= rsj;

            j++;
            rsj = pV8->pRecv_string1[j];

            if ((rsj & 0x1c) == 0x08) /* extension octet */
            {
                octet[oct_idx++] &= rsj;

                j++;
                rsj = pV8->pRecv_string1[j];

                if ((rsj & 0x1c) == 0x08) /* extension octet */
                {
                    octet[oct_idx++] &= rsj;
                }
            }
        }
    }

    jm_idx = 3;

    for (j = 0; j < oct_idx; j++)            /* -- shift octet into jm -- */
    {
        temp = UBUBMULUB(jm_idx, 10);

        for (i = 8; i >= 0; i--)                /* N.B. actually only 8 bits !! assume the 9th bit is 0 !! */
        {
            pV8->JM_Octets[i + temp] = (UBYTE)(octet[j] & 0x1);
            octet[j] >>= 1;
        }

        pV8->JM_Octets[temp + 9] = 1;        /* stop bit */
        jm_idx++;
    }

    temp = UBUBMULUB(jm_idx, 10);

    pV8->ubJMLen = temp;

#if SUPPORT_V90

    if (pV8->V90_Enable || pV8->V92_Enable)
    {
        for (i = 0; i < 20; i++)
        {
            pV8->JM_Octets[i + temp] = pV8->CM_Octets[70 + i];
        }

        pV8->ubJMLen += 20;
    }

#endif
}

#endif
