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

/*----------------------------------------------------------------------*/
/*      Create the MP sequence from the structure.                      */
/*                                                                      */
/* NOTES: Please refer to ITU V.90 p.31 & ITU V.34 p.34 to set up       */
/*        the structure                                                 */
/*                                                                      */
/* INPUT: UBYTE *info_buf - the output buffer of the INFO0 bit stream   */
/*        MP *I         - the MP structure, user need to setup the MP   */
/*                        in the structure before calling this function.*/
/*        type          - should be TYPE_1 or TYPE_0 (defined in MP.h), */
/*                        which will be used to set bit #19 in the MP   */
/*                        structure                                     */
/*                                                                      */
/* OUTPUT: info_buf is filled with MP bit stream located in the LSB     */
/*         of each data byte.                                           */
/*                                                                      */
/*----------------------------------------------------------------------*/

#include "v34ext.h"

#if !USE_ASM
void  V34_Create_Mp(UBYTE *info_buf, MpStruc *pMpTx)
{
    UWORD data;
    UWORD crc;
    UBYTE shifter;
    UBYTE ubTemp;

    crc = 0xFFFF;

    /* Frame Sync: 1111111111111111 ( total of 17 1's ) */
    /* where the left-most bit is first in time         */
    *info_buf++ = 0xFF;              /* Bit 0:7 */
    *info_buf++ = 0xFF;              /* Bit 8:15 */
    shifter = 1;                     /* Bit 16, End of Frame Sync, S0 */
    /* Bit 17, Start Bit=0        S1 */

    CRC16_Gen(&crc, pMpTx->type);
    shifter |= (pMpTx->type << 2);     /* Bit 18, MP Type,           S2 */

    CRC16_Gen(&crc, 0);              /* Bit 19, Reserved bit=0     S3 */

    data = pMpTx->max_call_to_ans_rate & 0xF;
    CRC16_nBits(&crc, data, 4);
    shifter |= (data << 4);          /* Bit 20:23, Max call bps, S4:S7 */

    *info_buf++ = shifter;     /* Bit 16:23 */

    shifter = pMpTx->max_ans_to_call_rate & 0xF;
    CRC16_nBits(&crc, shifter, 4);  /* Bit 24:27, Max Analogue bps,  S0:S3*/

    ubTemp = (pMpTx->aux_channel_select & 0x1);
    CRC16_Gen(&crc, ubTemp);
    shifter |= ubTemp << 4;         /* Bit 28, Aux channel,       S4 */

    data = pMpTx->TrellisEncodSelect & 0x3;
    CRC16_nBits(&crc, data, 2);
    shifter |= data << 5;           /* Bit 29:30, Trellis bit,  S5:S6*/

    ubTemp = (pMpTx->NonlinearEncodParam & 0x1);
    CRC16_Gen(&crc, ubTemp);
    shifter |= ubTemp << 7;         /* Bit 31, Non-linear encoder S7 */

    *info_buf++ = shifter; /* Bit 24:31 */

    shifter = (pMpTx->const_shap_select_bit & 0x1);
    CRC16_Gen(&crc, shifter);       /* Bit 32, shaping,           S0 */

    ubTemp = (pMpTx->acknowledge_bit & 0x1);
    CRC16_Gen(&crc, ubTemp);
    shifter |= ubTemp << 1;            /* Bit 33, ACK bit,           S1 */

    /* Bit 34, start bit=0,no CRC S2 */

    data = pMpTx->data_rate_cap_mask & 0x3FFF;
    CRC16_nBits(&crc, data, 15);
    shifter |= (data & 0x1F) << 3;   /* Bit 35:39, bps mask,     S3:S7 */

    *info_buf++ = shifter; /* Bit 32:39 */

    data >>= 5;
    *info_buf++ = data & 0xFF;       /* Bit 40:47, bps mask,     S0:S7 */

    data >>= 8;
    shifter = data & 0x3;            /* Bit 48:49, bps mask,     S0:S1 */

    ubTemp = (pMpTx->asymmetric_data_sig_rate & 0x1);
    CRC16_Gen(&crc, ubTemp);
    shifter |= ubTemp << 2;            /* Bit 50, asymmetric bps,    S2 */

    /* Bit 51, Start bit=0,no CRC S3 */

    if (pMpTx->type == MP_TYPE_0)
    {
        /* bit 52-67  reserved for ITU, these are to be set to zero by  */
        /* transmitting modem, not to be set by user                    */
        UpdateCRC_B(&crc, 0);
        UpdateCRC_B(&crc, 0);

        *info_buf++ = shifter;      /* Bit 52:55, Reserved bits=0 S4:S7 */
        *info_buf++ = 0;            /* Bit 56:63, Reserved bits=0 S0:S7 */
        shifter = 0;                /* Bit 64:67, Reserved bits=0 S0:S3 */

        /* Bit 68, start bit=0,no CRC S4 */

        shifter |= (crc & 0x7) << 5;/* Bit 69:71, CRC,          S5:S7 */
        *info_buf++ = shifter;  /* Bit 64:71 */

        crc >>= 3;
        shifter = crc & 0xFF;       /* Bit 72:79, CRC,          S0:S7 */
        *info_buf++ = shifter;

        crc >>= 8;
        shifter = crc & 0x1F;       /* Bit 80:84, CRC,          S0:S4 */
        /* Bit 85:87, Fill bits=0   S5:S7 */
        *info_buf = shifter;
    }/* bit setting for Type 0 is finished */
    else if (pMpTx->type == MP_TYPE_1)
    {
        data = pMpTx->PrecodeCoeff_h[0].r;/* Bit 52:67 Precoding coeffieient h(1) real */

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0xF) << 4; /* Bit 52:55, h(1) real,    S4:S7 */

        *info_buf++ = shifter; /* Bit 48:55 */
        data >>= 4;

        *info_buf++ = (data & 0xFF);/* Bit 56:63, h(1) real,    S0:S7 */
        data >>= 8;

        shifter = data & 0xF;       /* Bit 64:67, h(1) real,    S0:S3 */
        /* Bit 68, Start bit=0,no CRC S4 */

        /* Bit 69:84 Precoding coefficient h(1) imaginary */
        data = pMpTx->PrecodeCoeff_h[0].i;
        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0x7) << 5; /* Bit 69:71, h(1) imag,    S5:S7 */

        *info_buf++ = shifter;  /* Bit 64:71 */
        data >>= 3;

        *info_buf++ = data & 0xFF;  /* Bit 72:79, h(1) imag,    S0:S7 */
        data >>= 8;

        shifter = data & 0x1F;      /* Bit 80:84, h(1) imag,    S0:S4 */
        /* Bit 85, Start bit=0,       S5 */

        data = pMpTx->PrecodeCoeff_h[1].r;/* Bit 86:101 Precoding coefficient h(2) real */

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0x3) << 6; /* Bit 86:87, h(2) real,    S6:S7 */

        *info_buf++ = shifter; /* Bit 80:87 */
        data >>= 2;

        *info_buf++ = data & 0xFF;  /* Bit 88:95, h(2) real,    S0:S7 */
        data >>= 8;

        shifter = data & 0x3F;      /* Bit 96:101, h(2) real,   S0:S5 */
        /* Bit 102, Start bit=0,      S6 */

        /* Bit 103:118 Precoding coefficient h(2) imaginary */
        data = pMpTx->PrecodeCoeff_h[1].i;

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0x1) << 7; /* Bit 103, h(2) imag,        S7 */

        *info_buf++ = shifter;
        data >>= 1;

        *info_buf++ = data & 0xFF;  /* Bit 104:111, h(2) imag,  S0:S7 */
        data >>= 8;

        shifter = data & 0x7F;      /* Bit 112:118, h(2) imag,  S0:S6 */
        /* Bit 119, Start bit=0,      S7 */
        *info_buf++ = shifter;

        /* Bit 120:135 Precoding coefficient h(3) real */
        data = pMpTx->PrecodeCoeff_h[2].r;

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        *info_buf++ = data & 0xFF;  /* Bit 120:127, h(3) real,  S0:S7 */
        data >>= 8;

        *info_buf++ = data & 0xFF;  /* Bit 128:135, h(3) real,  S0:S7 */

        shifter = 0;                /* Bit 136, Start bit=0,      S0 */

        /* Bit 137:152 Precoding coefficient h(3) imaginary */
        data = pMpTx->PrecodeCoeff_h[2].i;

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0x7F) << 1; /* Bit 137:143, h(3) imag,  S1:S7 */

        *info_buf++ = shifter;
        data >>= 7;

        *info_buf++ = data & 0xFF;  /* Bit 144:151, h(3) imag,  S0:S7 */
        data >>= 8;

        shifter = data & 0x1;       /* Bit 152, h(3) imag,        S0 */
        /* Bit 153, Start bit=0,      S1 */

        /* bit 154-169  reserved for ITU, these are to be set to 0    */
        /* by transmitting modem, not to be set by user.              */
        UpdateCRC_B(&crc, 0);
        UpdateCRC_B(&crc, 0);

        *info_buf++ = shifter;      /* Bit 154:159, Reserved    S2:S7 */
        *info_buf++ = 0;            /* Bit 160:167, Reserved    S0:S7 */
        shifter = 0;                /* Bit 168:169, Reserved    S0:S1 */
        /* Bit 170, Start bit=0;      S2 */

        shifter |= (crc & 0x1F) << 3; /* Bit 171:175, CRC,        S3:S7 */

        *info_buf++ = shifter;
        crc >>= 5;

        *info_buf++ = crc & 0xFF;   /* Bit 176:183, CRC,        S0:S7 */
        crc >>= 8;

        shifter = crc & 0x7;        /* Bit 184:186, CRC,        S0:S2 */
        /* Bit 187, Fill bit=0,       S3 */
        *info_buf++ = shifter;
    }  /* bit setting for Type 1 finished here */
}
#endif
