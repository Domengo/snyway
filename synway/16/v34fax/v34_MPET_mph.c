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

#include "v34fext.h"

#if SUPPORT_V34FAX

UWORD Mph_BitToByte(UBYTE *data_end, UBYTE n)
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


void V34Fax_Detect_Mph_Init(MphStruc *pMphRx)
{
    pMphRx->bit_num_cntr_mp = -1;
    pMphRx->frame_sync_mp   = 0;
    pMphRx->CRC_mp          = 0xFFFF;
}


UBYTE *Detect_Mph_Common(MphStruc *pMphRx)
{
    UBYTE ubTemp;
    UBYTE *pData;
    UBYTE MPh_max_data_rate;

    pData = pMphRx->pData_sequence_mp;

    /* pData_sequence_mp[0] stored bit 17 of MPh sequence Type 0 */
    /* and Type 1, which is a start bit */

    /* pData_sequence_mp[1] stored bit 18 of MPh sequence Type 0 */
    /* and Type 1, which is the type of the MPh sequence */

    /* pData_sequence_mp[2] stored bit 19 of MPh sequence Type 0 */
    /* and Type 1, a bit reserved for ITU */

    pData += 3; /* bit 17:19 */ /* not used */

    pData += 4; /* bit 20:23 */
    MPh_max_data_rate = (UBYTE)Mph_BitToByte(pData, 4);

    pMphRx->max_data_rate = MPh_max_data_rate;

    pData += 3; /* bit 24:26 */ /* reserved */

    pMphRx->control_channel_rate = *pData++;/* bit 27    */

    pData ++;  /* bit 28    */ /* Reserved */

    ubTemp  = *pData++; /* bit 29:30 */
    pMphRx->TrellisEncodSelect = ubTemp | (*pData++ << 1);

    pMphRx->NonlinearEncodParam = *pData++; /* 31 */

    pMphRx->const_shap_select_bit = *pData++; /* 32 */

    pData ++; /* 33 */ /* Reserved */
    pData ++; /* 34 */ /* pData_sequence_mp[17] is the start bit */

    pData += 14; /* Bit 35:48 */
    pMphRx->data_rate_cap_mask = Mph_BitToByte(pData, 14);

    pData ++;    /* Bit 49: Reserved */
    pMphRx->asymmetric_control_rate = *pData++ ; /* Bit 50 */

    return pData;
}


SBYTE V34Fax_ChkCRC(MphStruc *pMphRx, SWORD idx)
{
    UWORD check_crc;
    SBYTE result_code;

    check_crc = Mph_BitToByte(&(pMphRx->pData_sequence_mp[idx]), 16);

    if (pMphRx->CRC_mp == check_crc)
    {
        result_code =  1;
    }
    else
    {
        result_code = -1;
        TRACE0("V34: CRC error in MPh");
    }

    V34Fax_Detect_Mph_Init(pMphRx);

    return(result_code);
}

SBYTE V34Fax_Detect_Mph(UBYTE bit, MphStruc *pMphRx)
{
    UBYTE flag;
    UBYTE *pData;
    UBYTE U8_temp;
    UBYTE i;
    SWORD *pPrecodingCoef;

    // fixing bug 40, need to constantly check for MPh sync
    pMphRx->frame_sync_mp = (((pMphRx->frame_sync_mp << 1) | bit) & 0x0001ffffL);

    if (pMphRx->frame_sync_mp == 0x0001ffffL)
    {
        V34Fax_Detect_Mph_Init(pMphRx);
        pMphRx->bit_num_cntr_mp = 0;
        return (0);
    }

    switch (pMphRx->bit_num_cntr_mp)
    {
        case -1:
            // fixing bug 40, hasn't found the sync
            break;
        case 0:

            if ((bit & 1) == 0x00)
            {
                /* find start bit */
                pMphRx->pData_sequence_mp[0] = 0x00;
                pMphRx->bit_num_cntr_mp++;
            }

            break;

        case 1:
            pMphRx->pData_sequence_mp[1] = bit & 1;

            CRC16_Gen(&pMphRx->CRC_mp, bit);
            pMphRx->bit_num_cntr_mp++;
            pMphRx->type = bit;

            break;

        default:

            if (pMphRx->type == MP_TYPE_1)
            {
                if (pMphRx->bit_num_cntr_mp < 170)
                {
                    pMphRx->pData_sequence_mp[pMphRx->bit_num_cntr_mp] = bit & 1;

                    switch (pMphRx->bit_num_cntr_mp)
                    {
                            /* Skip all Start bit */
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

                            if (pMphRx->bit_num_cntr_mp < 153)
                            {
                                U8_temp = bit & 0x1;
                                CRC16_Gen(&pMphRx->CRC_mp, U8_temp);
                            }

                            break;
                    }

                    pMphRx->bit_num_cntr_mp++;

                    return(0);
                }
                else if (pMphRx->bit_num_cntr_mp == 170)
                {
                    pMphRx->pData_sequence_mp[pMphRx->bit_num_cntr_mp] = bit & 1;

                    flag = V34Fax_ChkCRC(pMphRx, 169 + 1);

                    if (flag == 1)
                    {
                        pData = Detect_Mph_Common(pMphRx);

                        /* If it is type 1, more incoming bits has to get */
                        /* pData_sequence_mp[35] corresponds to bit #52 in MP sequence */

                        /* assume destination address are continuous!! */
                        pPrecodingCoef = &pMphRx->PrecodeCoeff_h[0].r;

                        for (i = 0; i < 6; i++)
                        {
                            pData++; /* start bit */
                            pData += 16;
                            *pPrecodingCoef++ = Mph_BitToByte(pData, 16);
                        }

#if 0
                        DumpTone6_Idx = 170;    /* not destroy MPh dumped */

                        for (i = 0; i < 170; i++)
                        {
                            DumpTone6[i] = pMphRx->pData_sequence_mp[i];
                        }

#endif
                    }

                    return flag;
                }       /* End if bit_num_cntr_mp < 170 */
            }           /* End TYPE 1 bit extracting */
            else if (pMphRx->type == MP_TYPE_0)
            {
                if (pMphRx->bit_num_cntr_mp < 70)
                {
                    pMphRx->pData_sequence_mp[pMphRx->bit_num_cntr_mp] = bit;

                    switch (pMphRx->bit_num_cntr_mp)
                    {
                        case 17:
                        case 34:
                        case 51:
                            break;

                        default:

                            if (pMphRx->bit_num_cntr_mp < 51)
                            {
                                CRC16_Gen(&pMphRx->CRC_mp, bit);
                            }

                            break;
                    }

                    pMphRx->bit_num_cntr_mp++;

                    return(0);
                }       /* End if bit_num_cntr_mp < 70 */
                else if (pMphRx->bit_num_cntr_mp == 70)
                {
                    pMphRx->pData_sequence_mp[pMphRx->bit_num_cntr_mp] = bit;

                    flag = V34Fax_ChkCRC(pMphRx, 67 + 1);

                    if (flag == 1)
                    {
                        Detect_Mph_Common(pMphRx);

                        /* bit 68 = pData_sequence_mp[51] is the start bit, */
#if 0
                        DumpTone5_Idx = 70;    /* not destroy MPh dumped */

                        for (i = 0; i < 70; i++)
                        {
                            DumpTone5[i] = pMphRx->pData_sequence_mp[i];
                        }

#endif
                        /* do not go into CRC */
                    }

                    return flag;
                }       /* End if bit_num_cntr_mp == 70 */
            }           /* End if TYPE 0 */
    }               /* End Switch */

    return (0);
}


void V34Fax_Create_Mph(UBYTE *info_buf, MphStruc *pMphTx)
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
    CRC16_Gen(&crc, pMphTx->type);
    shifter |= (pMphTx->type << 2);    /* Bit 18, MP Type,           S2 */

    CRC16_Gen(&crc, 0);              /* Bit 19, Reserved bit=0     S3 */

    data = pMphTx->max_data_rate & 0xF;
    CRC16_nBits(&crc, data, 4);
    shifter |= (data << 4);          /* Bit 20:23, Max call bps, S4:S7 */

    *info_buf++ = shifter;           /* Bit 16:23 */

    shifter = 0;                     /* Bit 24:26, Reserved bit=0  S0:S2 */
    CRC16_nBits(&crc, shifter, 3);

    data = pMphTx->control_channel_rate & 1;
    CRC16_Gen(&crc, (UBYTE)data);
    shifter |= (data << 3);          /* Bit 27, Control ch data rate S3 */

    CRC16_Gen(&crc, 0);              /* Bit 28, Reserved bit=0       S4 */

    data = pMphTx->TrellisEncodSelect & 0x3;
    CRC16_nBits(&crc, data, 2);
    shifter |= data << 5;            /* Bit 29:30, Trellis bit,    S5:S6 */

    ubTemp = (pMphTx->NonlinearEncodParam & 0x1);
    CRC16_Gen(&crc, ubTemp);
    shifter |= ubTemp << 7;          /* Bit 31, Non-linear encoder   S7 */

    *info_buf++ = shifter;           /* Bit 24:31 */

    shifter = (pMphTx->const_shap_select_bit & 0x1);
    CRC16_Gen(&crc, shifter);        /* Bit 32, shaping,             S0 */

    CRC16_Gen(&crc, 0);              /* Bit 33, Reserved bit=0       S1 */

    /* Bit 34, start bit=0,  no CRC S2 */

    data = pMphTx->data_rate_cap_mask & 0x3FFF;
    CRC16_nBits(&crc, data, 15);
    shifter |= (data & 0x1F) << 3;   /* Bit 35:39, bps mask,       S3:S7 */

    *info_buf++ = shifter;           /* Bit 32:39 */

    data >>= 5;
    *info_buf++ = data & 0xFF;       /* Bit 40:47, bps mask,       S0:S7 */

    data >>= 8;
    shifter = data & 0x3;            /* Bit 48:49, bps mask,       S0:S1 */

    ubTemp = (pMphTx->asymmetric_control_rate & 0x1);
    CRC16_Gen(&crc, ubTemp);
    shifter |= ubTemp << 2;          /* Bit 50, asymmetric bps,      S2 */

    /* Bit 51, Start bit=0,no CRC S3 */

    if (pMphTx->type == MP_TYPE_0)
    {
        /* bit 52-67  reserved for ITU, these are to be set to zero by  */
        /* transmitting modem, not to be set by user                    */
        UpdateCRC_B(&crc, 0);
        UpdateCRC_B(&crc, 0);

        *info_buf++ = shifter;       /* Bit 52:55, Reserved bits=0 S4:S7 */
        *info_buf++ = 0;             /* Bit 56:63, Reserved bits=0 S0:S7 */
        shifter = 0;                 /* Bit 64:67, Reserved bits=0 S0:S3 */

        /* Bit 68, start bit=0,no CRC S4 */

        shifter |= (crc & 0x7) << 5; /* Bit 69:71, CRC,            S5:S7 */
        *info_buf++ = shifter;       /* Bit 64:71 */

        crc >>= 3;
        shifter = crc & 0xFF;        /* Bit 72:79, CRC,            S0:S7 */
        *info_buf++ = shifter;

        crc >>= 8;
        shifter = crc & 0x1F;        /* Bit 80:84, CRC,            S0:S4 */
        /* Bit 85:87, Fill bits=0     S5:S7 */
        *info_buf = shifter;
    }/* bit setting for Type 0 is finished */
    else if (pMphTx->type == MP_TYPE_1)
    {
        data = pMphTx->PrecodeCoeff_h[0].r;/* Bit 52:67 Precoding coeffieient h(1) real */

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0xF) << 4;   /* Bit 52:55, h(1) real,      S4:S7 */

        *info_buf++ = shifter;          /* Bit 48:55 */
        data >>= 4;

        *info_buf++ = (data & 0xFF);    /* Bit 56:63, h(1) real,      S0:S7 */
        data >>= 8;

        shifter = data & 0xF;           /* Bit 64:67, h(1) real,      S0:S3 */
        /* Bit 68, Start bit=0,  no CRC S4 */

        /* Bit 69:84 Precoding coefficient h(1) imaginary */
        data = pMphTx->PrecodeCoeff_h[0].i;

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));

        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0x7) << 5;  /* Bit 69:71, h(1) imag,      S5:S7 */

        *info_buf++ = shifter;         /* Bit 64:71 */
        data >>= 3;

        *info_buf++ = data & 0xFF;     /* Bit 72:79, h(1) imag,      S0:S7 */
        data >>= 8;

        shifter = data & 0x1F;         /* Bit 80:84, h(1) imag,      S0:S4 */
        /* Bit 85, Start bit=0,         S5 */

        data = pMphTx->PrecodeCoeff_h[1].r;/* Bit 86:101 Precoding coefficient h(2) real */

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0x3) << 6; /* Bit 86:87, h(2) real,      S6:S7 */
        *info_buf++ = shifter;       /* Bit 80:87 */
        data >>= 2;

        *info_buf++ = data & 0xFF;   /* Bit 88:95, h(2) real,      S0:S7 */
        data >>= 8;

        shifter = data & 0x3F;       /* Bit 96:101, h(2) real,     S0:S5 */
        /* Bit 102, Start bit=0,      S6 */

        /* Bit 103:118 Precoding coefficient h(2) imaginary */
        data = pMphTx->PrecodeCoeff_h[1].i;

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0x1) << 7; /* Bit 103, h(2) imag,          S7 */

        *info_buf++ = shifter;
        data >>= 1;

        *info_buf++ = data & 0xFF;   /* Bit 104:111, h(2) imag,    S0:S7 */
        data >>= 8;

        shifter = data & 0x7F;       /* Bit 112:118, h(2) imag,    S0:S6 */
        /* Bit 119, Start bit=0,      S7 */
        *info_buf++ = shifter;

        /* Bit 120:135 Precoding coefficient h(3) real */
        data = pMphTx->PrecodeCoeff_h[2].r;

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        *info_buf++ = data & 0xFF;   /* Bit 120:127, h(3) real,    S0:S7 */
        data >>= 8;

        *info_buf++ = data & 0xFF;   /* Bit 128:135, h(3) real,    S0:S7 */

        shifter = 0;                 /* Bit 136, Start bit=0,        S0 */

        /* Bit 137:152 Precoding coefficient h(3) imaginary */
        data = pMphTx->PrecodeCoeff_h[2].i;

        UpdateCRC_B(&crc, (UBYTE)(data & 0x00FF));
        UpdateCRC_B(&crc, (UBYTE)(data >> 8));

        shifter |= (data & 0x7F) << 1; /* Bit 137:143, h(3) imag,    S1:S7 */

        *info_buf++ = shifter;
        data >>= 7;

        *info_buf++ = data & 0xFF;   /* Bit 144:151, h(3) imag,    S0:S7 */
        data >>= 8;

        shifter = data & 0x1;        /* Bit 152, h(3) imag,          S0 */
        /* Bit 153, Start bit=0,      S1 */

        /* bit 154-169  reserved for ITU, these are to be set to        */
        /* zero by transmitting modem, not to be set by user.           */
        UpdateCRC_B(&crc, 0);
        UpdateCRC_B(&crc, 0);

        *info_buf++ = shifter;       /* Bit 154:159, Reserved      S2:S7 */
        *info_buf++ = 0;             /* Bit 160:167, Reserved      S0:S7 */
        shifter = 0;                 /* Bit 168:169, Reserved      S0:S1 */
        /* Bit 170, Start bit=0;      S2   */

        shifter |= (crc & 0x1F) << 3; /* Bit 171:175, CRC,          S3:S7 */

        *info_buf++ = shifter;
        crc >>= 5;

        *info_buf++ = crc & 0xFF;    /* Bit 176:183, CRC,          S0:S7 */
        crc >>= 8;

        shifter = crc & 0x7;         /* Bit 184:186, CRC,          S0:S2 */
        /* Bit 187, Fill bit=0,       S3   */
        *info_buf++ = shifter;
    }  /* bit setting for Type 1 finished here */
}

#endif
