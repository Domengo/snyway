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

#include <string.h>
#include "commmac.h"
#include "hdlcext.h"
#include "common.h"
#include "dspdext.h"

CONST UWORD HDLCType_Table1[HDLC_TYPE_TABLE1_LEN] =
{
    HDLC_TYPE_DIS,     HDLC_TYPE_CSI,       HDLC_TYPE_NSF,      HDLC_TYPE_DTC,
    HDLC_TYPE_CIG,     HDLC_TYPE_NSC,       HDLC_TYPE_PWD,      HDLC_TYPE_SEP,
    HDLC_TYPE_PSA,     HDLC_TYPE_CIA,       HDLC_TYPE_ISP,      HDLC_TYPE_T4_FCD,
    HDLC_TYPE_T4_RCP
};

static CONST UWORD HDLCType_Table2[HDLC_TYPE_TABLE2_LEN] =
{
    HDLC_TYPE_DCS,       HDLC_TYPE_TSI,      HDLC_TYPE_NSS,      HDLC_TYPE_SUB,
    HDLC_TYPE_FTT,       HDLC_TYPE_EOM,      HDLC_TYPE_PRI_EOM,  HDLC_TYPE_PRI_MPS,
    HDLC_TYPE_PRI_EOP,   HDLC_TYPE_EOS,      HDLC_TYPE_T4_RR,    HDLC_TYPE_TNR,
    HDLC_TYPE_MCF,       HDLC_TYPE_RTP,      HDLC_TYPE_RTN,      HDLC_TYPE_PIP,
    HDLC_TYPE_PIN,       HDLC_TYPE_FDM,      HDLC_TYPE_T4_PPR,   HDLC_TYPE_T4_RNR,
    HDLC_TYPE_FNV,       HDLC_TYPE_T4_ERR,   HDLC_TYPE_DCN,      HDLC_TYPE_CRP,
    HDLC_TYPE_SID,       HDLC_TYPE_TSA,      HDLC_TYPE_CFR,      HDLC_TYPE_T4_CTC,
    HDLC_TYPE_T4_CTR,    HDLC_TYPE_MPS,      HDLC_TYPE_EOP,      HDLC_TYPE_IRA,
    HDLC_TYPE_CSA,       HDLC_TYPE_TR
};

static CONST UWORD HDLCType_ExTable[HDLC_TYPE_EXTABLE_LEN] =
{
    PPS_NULL, PPS_EOM, PPS_MPS, PPS_EOP, PPS_PRI_EOM, PPS_PRI_MPS, PPS_PRI_EOP,
    EOR_NULL, EOR_EOM, EOR_MPS, EOR_EOP, EOR_PRI_EOM, EOR_PRI_MPS, EOR_PRI_EOP
};

UWORD HDLC_GetT30Type(UBYTE *p)
{
    UWORD    fcf, i;
    UBYTE    ch;

    ch = *p;

    fcf = ((UWORD)ch << 8);

    for (i = 0; i < HDLC_TYPE_TABLE1_LEN; i++)
    {
        if (fcf == HDLCType_Table1[i])
        {
            break;
        }
    }

    if (i == HDLC_TYPE_TABLE1_LEN)
    {
        for (i = 0; i < HDLC_TYPE_TABLE2_LEN; i++)
        {
            if ((fcf & 0x7FFF) == (HDLCType_Table2[i] & 0x7FFF))
            {
                fcf = HDLCType_Table2[i];
                break;
            }
        }

        if (i == HDLC_TYPE_TABLE2_LEN)
        {
            fcf |= (*(p + 1));

            for (i = 0; i < HDLC_TYPE_EXTABLE_LEN; i++)
            {
                if ((fcf & 0x7FFF) == (HDLCType_ExTable[i] & 0x7FFF))
                {
                    fcf = HDLCType_ExTable[i];
                    break;
                }
            }

            if (i == HDLC_TYPE_EXTABLE_LEN)
            {
                fcf    = HDLCCFCERR;
            }
        }
    }

    return fcf;
}

UWORD HDLC_CorrT30FCF(UBYTE isDisRcved, UWORD type)
{
    if (isDisRcved)
    {
        return type;
    }
    else
    {
        UWORD i;

        for (i = 0; i < HDLC_TYPE_TABLE1_LEN; i++)
        {
            if (type == HDLCType_Table1[i])
            {
                return type;
            }
        }

        return (type & 0x7fff);
    }
}

/******************************************
* the initialization of T30_HDLC_STRUCT
*******************************************/
void HDLC_Init(HdlcStruct *pHDLC)
{
    memset(pHDLC, 0, sizeof(HdlcStruct));

    pHDLC->Status = HDLC_INITIALIZED;
    pHDLC->CRC = 0xFFFF;
}

/***********************************
* Remove the data in working buffer,
* and reset associated attributes
***********************************/
void HDLC_ResetWB(HdlcStruct *pHDLC)
{
    memset(pHDLC->WorkingBuff, 0, sizeof(pHDLC->WorkingBuff));

    pHDLC->WorkingBuffEnd = pHDLC->WorkingBuffLoc = 0;
}

#if !SUPPORT_T38EP
static CONST UBYTE Flag_Patterns[8] = {0x7E, 0x3F, 0x9F, 0xCF, 0xE7, 0xF3, 0xF9, 0xFC};
static CONST UBYTE Mask[8] = {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01};

/***********************************************************************
* Remove data from working buffer. Remain the bits that are not aligned.
* Reset associated indexes as well.
***********************************************************************/
static void HDLC_CleanWB_Rx(HdlcStruct *pHDLC)
{
    pHDLC->WorkingBuff[0] = pHDLC->WorkingBuff[pHDLC->WorkingBuffEnd >> 3];

    memset(&pHDLC->WorkingBuff[1], 0, sizeof(pHDLC->WorkingBuff) - sizeof(UBYTE));

    pHDLC->WorkingBuffEnd &= 0x07;
    pHDLC->WorkingBuffLoc &= 0x07;
}

/******************************************************
* Remove frames from Raw data buffer to make room for
* more data, but remain the last flag.
* Reset associated indexes as well.
*******************************************************/
void HDLC_CleanRD(HdlcStruct *pHDLC_RX)
{
    UBYTE byte;
    SWORD length;
    int i;

    switch (pHDLC_RX->Status)
    {
        case HDLC_INITIALIZED:

            /* just keep the last byte, because only the last byte is meaningful    */
            if (pHDLC_RX->RawDataEnd >= 8)
            {
                byte = pHDLC_RX->RawData[(pHDLC_RX->RawDataEnd >> 3) - 1];

                memset(pHDLC_RX->RawData, 0, sizeof(pHDLC_RX->RawData));

                pHDLC_RX->RawData[0] = byte;
                pHDLC_RX->FrameStart = 0;
                pHDLC_RX->WhereToStart = 0;
                pHDLC_RX->RawDataEnd = (UWORD)((1 << 3) | (pHDLC_RX->RawDataEnd & 0x07));
            }

            pHDLC_RX->OnesCount = 0;

            break;

        case HDLC_FRAMESTART_FOUND:
        case HDLC_DATA_FOUND:
            length = (pHDLC_RX->RawDataEnd >> 3) - (pHDLC_RX->FrameStart >> 3);

            if (length < 0)
            {
                TRACE0("HDLC: ERROR. Invalid length");
            }

            for (i = 0; i < length; i++)
            {
                pHDLC_RX->RawData[i] = pHDLC_RX->RawData[(pHDLC_RX->FrameStart >> 3) + i];
            }

            for (i = length; i < HDLCBUFLEN; i++)
            {
                pHDLC_RX->RawData[i] = 0;
            }

            /* move WhereToStart foreward    */
            if (pHDLC_RX->WhereToStart > (pHDLC_RX->FrameStart & 0xFFF8)) // fixing bug 36, avoid negtive WhereToStart
            {
                pHDLC_RX->WhereToStart -= (pHDLC_RX->FrameStart & 0xFFF8);
            }
            else
            {
                pHDLC_RX->WhereToStart = 0;
            }

            /* bitno remains unchanged        */
            pHDLC_RX->FrameStart &= 0x07;
            pHDLC_RX->RawDataEnd = (UWORD)((length << 3) | (pHDLC_RX->RawDataEnd & 0x07));

            break;

        case HDLC_FRAMEEND_FOUND:

            if (pHDLC_RX->WhereToStart >= pHDLC_RX->FrameEnd - 8)
            {
                /* we've already decode the whole frame and sent it to upper layer    */
                length = (pHDLC_RX->RawDataEnd >> 3) - (pHDLC_RX->FrameEnd >> 3);

                if (length < 0)
                {
                    TRACE0("HDLC: ERROR. Invalid length");
                }

                for (i = 0; i < length; i++)
                {
                    pHDLC_RX->RawData[i] = pHDLC_RX->RawData[(pHDLC_RX->FrameEnd >> 3) + i];
                }

                for (i = length; i < HDLCBUFLEN; i++)
                {
                    pHDLC_RX->RawData[i] = 0;
                }

                /* bitno remains unchanged    */
                pHDLC_RX->FrameEnd &= 0x07;

                /* set byteno and bitno of FrameStart to byteno and bitno of FrameEnd seperately    */
                pHDLC_RX->FrameStart = pHDLC_RX->FrameEnd;
                pHDLC_RX->WhereToStart = pHDLC_RX->FrameStart;
                pHDLC_RX->Status = HDLC_FRAMESTART_FOUND;
                pHDLC_RX->RawDataEnd = (UWORD)((length << 3) | (pHDLC_RX->RawDataEnd & 0x07));
                pHDLC_RX->OnesCount = 0;
            }

            break;
    }
}


/******************************************************************
* search flag between "start" and "end" in Raw Data
* return the number of bits that skiped for searching the first flag.
* Note: the 8 bits of flag is included in the returned number.
* if 0 is returned, means no flag is found.
* this function changes nothing in HDLC structure.
*********************************************************************/
static UWORD HDLC_SearchFlag(HdlcStruct *pHDLC_RX)
{
    UWORD loc;
    UBYTE bitno, *q;

    loc = pHDLC_RX->FrameStart;

    while (loc + 8 <= pHDLC_RX->RawDataEnd)
    {
        bitno = (UBYTE)(loc & 0x07);
        q     =    pHDLC_RX->RawData + (loc >> 3);

        if ((((*q++) ^ Flag_Patterns[bitno]) & Mask[bitno]) == 0)
        {
            if ((((*q) ^ Flag_Patterns[bitno]) & ~Mask[bitno]) == 0)
            {
                return ((UWORD)(loc - pHDLC_RX->FrameStart + 8));
            }
        }

        loc++;
    }

    return 0;
}


/*****************************************************
* Skip all continuous flags between "start" and "end"
* return the number of bits before the first non-flag.
* if FALSE is returned, means no flag is found or the left
* bits(before "end") are not enough to contain a flag.
******************************************************/
static UBYTE HDLC_SkipFlag(HdlcStruct *pHDLC_RX)
{
    UBYTE ret = FALSE;
    UBYTE pattern, next_pattern, mask, *p;

    pattern      = Flag_Patterns[pHDLC_RX->FrameStart & 0x07];
    mask         = (UBYTE)(~(0xFF >> (pHDLC_RX->FrameStart & 0x07)));
    next_pattern = (UBYTE)(pattern & mask);

    p = pHDLC_RX->RawData + (pHDLC_RX->FrameStart >> 3);

    while (pHDLC_RX->FrameStart <= pHDLC_RX->RawDataEnd - 8)
    {
        if ((*p++ == pattern) && ((*p & mask) == next_pattern))
        {
            /* if it is a flag, move to check the next 8 bits */
            pHDLC_RX->FrameStart += 1 << 3;

            ret = TRUE;
        }
        else
        {
            return ret;
        }
    }

    return ret;
}

/******************************************************************
* Check if there is an address (0xFF) start from "start".
* Note: actually, the address shoud be 1111 1011 1XXX before
* transperancy bits are taken out.
* return 0, means no address is detected, otherwise, 1 is returned.
******************************************************************/
static UBYTE HDLC_CheckAddress(HdlcStruct *pHDLC_RX)
{
    UWORD word;

    if ((pHDLC_RX->RawDataEnd - pHDLC_RX->FrameStart) <= 9)/* not enough bits to contain an address */
    {
        return 2;
    }

    word = (UWORD)(pHDLC_RX->RawData[pHDLC_RX->FrameStart >> 3] << 8);

    word |= pHDLC_RX->RawData[(pHDLC_RX->FrameStart >> 3) + 1];

    word <<= pHDLC_RX->FrameStart & 0x07;

    if ((word & 0xFF80) == 0xFB80)/* flag is found */
    {
        return 1;
    }
    else
    {
        TRACE0("HDLC: WARNING. Address of HDLC is not right");

        return 0;
    }
}

UBYTE HDLC_DataFound(HdlcStruct *pHDLC_RX, UBYTE *pBuff, UWORD *Len)
{
    UBYTE ret;

    if (HDLC_Decode(pHDLC_RX))
    {
        UWORD length = pHDLC_RX->WorkingBuffEnd >> 3;

        memcpy(pBuff, pHDLC_RX->WorkingBuff, length * sizeof(UBYTE));

        *Len = length & 0x1FF;

        ret = HDLC_RX_DATA | HDLC_RX_FLAG;

        HDLC_UpdateCRC16(&pHDLC_RX->CRC, pHDLC_RX->WorkingBuff, length);

        HDLC_CleanWB_Rx(pHDLC_RX);
    }
    else
    {
        ret = HDLC_RX_FCS_BAD;
    }

    return (ret);
}

/******************************************************/
/* try to identify frame from the raw data            */
/******************************************************/
void HDLC_IdentifyFrame(HdlcStruct *pHDLC_RX, UBYTE mustRepFrmStart)
{
    UWORD ret;

    if (pHDLC_RX->Status == HDLC_INITIALIZED)
    {
        if (pHDLC_RX->RawDataEnd > 0)
        {
            pHDLC_RX->FrameStart = 0;

            ret = HDLC_SearchFlag(pHDLC_RX);

            if (ret == 0)
            {
                /* if no flag is found */
                HDLC_CleanRD(pHDLC_RX);
            }
            else
            {
                /* if flag is found, move FrameStart after the flag found    */
                pHDLC_RX->FrameStart = ret;
                pHDLC_RX->Status = HDLC_FRAMESTART_FOUND;

                if (mustRepFrmStart)
                {
                    return;
                }
            }
        }
    }

    if (pHDLC_RX->Status == HDLC_FRAMESTART_FOUND)
    {
        /* if the first flag is already found */
        if (!HDLC_SkipFlag(pHDLC_RX))
        {
            // not all data are flags
            ret = HDLC_CheckAddress(pHDLC_RX);

            if (ret == 1)
            {
                /* if address is found */
                pHDLC_RX->Status = HDLC_DATA_FOUND;
                pHDLC_RX->WhereToStart = pHDLC_RX->FrameStart;
            }
            else if (ret == 0)
            {
                // wrong frame, skip it
                ret = HDLC_SearchFlag(pHDLC_RX);

                if (ret == 0)
                {
                    /* if no flag is found */
                    pHDLC_RX->Status = HDLC_INITIALIZED; // fixing bug 36, need to wipe out everything but the last byte
                    HDLC_CleanRD(pHDLC_RX);
                }
                else
                {
                    /* if flag is found, move FrameStart to the flag that found    */
                    pHDLC_RX->FrameStart += ret;
                    pHDLC_RX->WhereToStart = pHDLC_RX->FrameStart;
                }
            }
        }
    }

    if (pHDLC_RX->Status == HDLC_DATA_FOUND)
    {
        /* next, to find the end flag */
        ret = HDLC_SearchFlag(pHDLC_RX);

        if (ret > 0)
        {
            pHDLC_RX->FrameEnd = pHDLC_RX->FrameStart + ret;
            pHDLC_RX->Status = HDLC_FRAMEEND_FOUND;
        }
    }
}

void HDLC_FillWB(HdlcStruct *pHDLC_TX, UBYTE *inBuf, UWORD inLen)
{
    UWORD buffLen = pHDLC_TX->WorkingBuffEnd >> 3;

    HDLC_UpdateCRC16(&pHDLC_TX->CRC, inBuf, inLen);

    /* copy data to HDLC struct */
    memcpy(&pHDLC_TX->WorkingBuff[buffLen], inBuf, inLen * sizeof(UBYTE));

    pHDLC_TX->WorkingBuffEnd += inLen << 3;
}

/****************************************
* Append num Flags to the end of Raw Data
****************************************/
void HDLC_AppendFlags(HdlcStruct *pHDLC_TX, UBYTE num)
{
    UWORD word, uTemp;
    UBYTE i, byte1, byte2;

    word  = 0x7E00 >> (pHDLC_TX->RawDataEnd & 0x07);
    byte1 = word >> 8;
    byte2 = word & 0xFF;

    uTemp = pHDLC_TX->RawDataEnd >> 3;

    for (i = 0; i < num; i++)
    {
        pHDLC_TX->RawData[uTemp + i    ] |= byte1;

        pHDLC_TX->RawData[uTemp + i + 1]  = byte2;
    }

    pHDLC_TX->RawDataEnd += (num << 3);
}


/****************************************************************
* get rid of the flag and transparency bits from the Raw Data and
* copy it to working buffer.
* return 1 if frame is correctly decoded, otherwise, return 0
****************************************************************/
UBYTE HDLC_Decode(HdlcStruct *pHDLC_RX)
{
    UWORD end;

    if (pHDLC_RX->Status < HDLC_FRAMEEND_FOUND)
    {
        end = pHDLC_RX->RawDataEnd;
    }
    else
    {
        if (pHDLC_RX->FrameEnd > 8)
        {
            end = pHDLC_RX->FrameEnd - 8;
        }
        else
        {
            return 0;    /* no enough bits */
        }
    }

    while (pHDLC_RX->WhereToStart < end)
    {
        UBYTE byte = pHDLC_RX->RawData[(pHDLC_RX->WhereToStart >> 3)];
        UBYTE temp = (byte << (pHDLC_RX->WhereToStart & 0x07)) & 0xFF;
        int i;

        for (i = pHDLC_RX->WhereToStart & 0x7; i < 8; i++)
        {
            if (temp & 0x80)
            {
                pHDLC_RX->OnesCount++;
                SetBit(pHDLC_RX->WorkingBuff[(pHDLC_RX->WorkingBuffEnd >> 3)], 7 - (pHDLC_RX->WorkingBuffEnd & 0x7));
                pHDLC_RX->WorkingBuffEnd++;
            }
            else
            {
                if (pHDLC_RX->OnesCount < 5)
                {
                    ResetBit(pHDLC_RX->WorkingBuff[(pHDLC_RX->WorkingBuffEnd >> 3)], 7 - (pHDLC_RX->WorkingBuffEnd & 0x7));
                    pHDLC_RX->WorkingBuffEnd++;
                }
                else if (pHDLC_RX->OnesCount > 5)
                {
                    TRACE0("HDLC: ERROR. Corrupted frame");
                    return 0;
                }

                pHDLC_RX->OnesCount = 0;
            }

            temp <<= 1;
        }

        pHDLC_RX->WhereToStart += 8 - (pHDLC_RX->WhereToStart & 0x7);
    }

    return 1;
}

/**********************************************************************
* Encode the original frame data so that it could be sent through HDLC.
**********************************************************************/
void HDLC_Encode(HdlcStruct *pHDLC_TX)
{
    while (pHDLC_TX->WorkingBuffLoc < pHDLC_TX->WorkingBuffEnd)
    {
        UBYTE byte = pHDLC_TX->WorkingBuff[(pHDLC_TX->WorkingBuffLoc >> 3)];
        UBYTE temp = byte;
        int i;

        pHDLC_TX->WorkingBuffLoc += 8;

        for (i = 0; i < 8; i++)
        {
            if (temp & 0x80)
            {
                pHDLC_TX->OnesCount++;
                SetBit(pHDLC_TX->RawData[(pHDLC_TX->RawDataEnd >> 3)], 7 - (pHDLC_TX->RawDataEnd & 0x7));
            }
            else
            {
                pHDLC_TX->OnesCount = 0;
                ResetBit(pHDLC_TX->RawData[(pHDLC_TX->RawDataEnd >> 3)], 7 - (pHDLC_TX->RawDataEnd & 0x7));
            }

            pHDLC_TX->RawDataEnd++;

            if (pHDLC_TX->OnesCount >= 5)
            {
                pHDLC_TX->OnesCount = 0;
                ResetBit(pHDLC_TX->RawData[(pHDLC_TX->RawDataEnd >> 3)], 7 - (pHDLC_TX->RawDataEnd & 0x7));
                pHDLC_TX->RawDataEnd++;
            }

            temp <<= 1;
        }
    }
}

void HDLC_UpdateCRC16(UWORD *reg, UBYTE *msg, UWORD len)
{
    UBYTE byte;

    while (len--)
    {
        byte = COMM_BitReversal(*(msg++));

        UpdateCRC_B(reg, byte);
    }
}

/********************************
* CRC check
* return 1 if correct, 0 if not.
********************************/
UBYTE HDLC_CheckCRC(HdlcStruct *pHDLC)
{
    UWORD crc;

    HDLC_UpdateCRC16(&pHDLC->CRC, pHDLC->WorkingBuff, (UWORD)(pHDLC->WorkingBuffEnd >> 3));

    crc = pHDLC->CRC;

    pHDLC->CRC = 0xFFFF; /* reset CRC */

    if (crc == 0xF0B8)   /* CRC correct */
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************
* Append CRC to the end of Working Buffer.
*****************************************/
void HDLC_AppendCRC(HdlcStruct *pHDLC)
{
    UWORD crc, BuffLen;
    UBYTE byte;

    BuffLen = pHDLC->WorkingBuffEnd >> 3;

    /* append CRC to the end of data */
    crc = ~(pHDLC->CRC);

    byte = crc & 0x00FF;
    pHDLC->WorkingBuff[BuffLen] = COMM_BitReversal(byte);

    byte = (crc >> 8) & 0x00FF;
    pHDLC->WorkingBuff[BuffLen + 1] = COMM_BitReversal(byte);

    pHDLC->WorkingBuffEnd += 2 << 3;

    /* reset the CRC initial value */
    pHDLC->CRC = 0xFFFF;
}
#endif
