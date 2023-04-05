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

#ifndef _HDLCDEF_H
#define _HDLCDEF_H

#define HDLCBUFLEN                      (384)

#define HDLC_INITIALIZED                (0) /* no meaningful bit received yet */
#define HDLC_FRAMESTART_FOUND           (1) /* the flag of frame start is found */
#define HDLC_DATA_FOUND                 (2) /* data is found, the sign is address(0xFF) */
#define HDLC_FRAMEEND_FOUND             (3) /* the flag of frame end is found */
#define HDLC_STREAMEND_FOUND            (4) /* the end of the entire stream */

#define ERROR_BITS                      (0xFC) /* 1111 1100 */
#define TRAN_MODE                       (0xF8) /* 1111 1000 */

#define HDLC_RX_NULL                    (0x00) /* No signal */
#define HDLC_RX_FLAG                    (0x01) /* Receive FLAG */
#define HDLC_RX_DATA                    (0x02) /* Receive Data */
#define HDLC_RX_FCS_OK                  (0x04) /* CRC OK */
#define HDLC_RX_FCS_BAD                 (0x08) /* HDLC Error */
#define HDLC_RX_NO_CARRIER              (0x10) /* No carrier */
#define HDLC_RX_CRC_ERROR               (0x20) /* CRC Error */

/* hdlc type */
#define HDLC_TYPE_TABLE1_LEN            (13)
/******************************************/
#define HDLC_TYPE_INIT                  (0xFFFF)
#define HDLC_TYPE_DIS                   (0x0100)
#define HDLC_TYPE_CSI                   (0x0200)
#define HDLC_TYPE_NSF                   (0x0400)
#define HDLC_TYPE_DTC                   (0x8100)
#define HDLC_TYPE_CIG                   (0x8200)

#define HDLC_TYPE_NSC                   (0x8400)
#define HDLC_TYPE_PWD                   (0x8300)
#define HDLC_TYPE_SEP                   (0x8500)
#define HDLC_TYPE_PSA                   (0x8600)
#define HDLC_TYPE_CIA                   (0x8700)
#define HDLC_TYPE_ISP                   (0x8800)

#define HDLC_TYPE_T4_FCD                (0x6000)
#define HDLC_TYPE_T4_RCP                (0x6100)

#define HDLC_TYPE_TABLE2_LEN            (34)
/******************************************/
#define HDLC_TYPE_DCS                   (0xC100)
#define HDLC_TYPE_TSI                   (0xC200)
#define HDLC_TYPE_SUB                   (0xC300)
#define HDLC_TYPE_NSS                   (0xC400)
#define HDLC_TYPE_SID                   (0xC500)
#define HDLC_TYPE_TSA                   (0xC600)
#define HDLC_TYPE_IRA                   (0xC700)
#define HDLC_TYPE_T4_CTC                (0xC800)

#define HDLC_TYPE_CFR                   (0xA100)
#define HDLC_TYPE_FTT                   (0xA200)
#define HDLC_TYPE_T4_CTR                (0xA300)
#define HDLC_TYPE_CSA                   (0xA400)

#define HDLC_TYPE_EOM                   (0xF100)
#define HDLC_TYPE_MPS                   (0xF200)
#define HDLC_TYPE_EOP                   (0xF400)
#define HDLC_TYPE_T4_RR                 (0xF600)
#define HDLC_TYPE_EOS                   (0xF800)
#define HDLC_TYPE_PRI_EOM               (0xF900)
#define HDLC_TYPE_PRI_MPS               (0xFA00)
#define HDLC_TYPE_PRI_EOP               (0xFC00)

#define HDLC_TYPE_MCF                   (0xB100)
#define HDLC_TYPE_RTN                   (0xB200)
#define HDLC_TYPE_RTP                   (0xB300)
#define HDLC_TYPE_PIN                   (0xB400)
#define HDLC_TYPE_PIP                   (0xB500)
#define HDLC_TYPE_FDM                   (0xBF00)
#define HDLC_TYPE_T4_PPR                (0xBD00)
#define HDLC_TYPE_T4_RNR                (0xB700)
#define HDLC_TYPE_T4_ERR                (0xB800)

#define HDLC_TYPE_DCN                   (0xDF00)
#define HDLC_TYPE_CRP                   (0xD800)
#define HDLC_TYPE_FNV                   (0xD300)
#define HDLC_TYPE_TNR                   (0xD700)
#define HDLC_TYPE_TR                    (0xD600)

#define HDLC_TYPE_EXTABLE_LEN           (14)
/******************************************/
#define PPS_NULL                        (0xFD00)
#define PPS_EOM                         (0xFDF1)
#define PPS_MPS                         (0xFDF2)
#define PPS_EOP                         (0xFDF4)
#define PPS_PRI_EOM                     (0xFDF9)
#define PPS_PRI_MPS                     (0xFDFA)
#define PPS_PRI_EOP                     (0xFDFC)

#define EOR_NULL                        (0xF300)
#define EOR_EOM                         (0xF3F1)
#define EOR_MPS                         (0xF3F2)
#define EOR_EOP                         (0xF3F4)
#define EOR_PRI_EOM                     (0xF3F9)
#define EOR_PRI_MPS                     (0xF3FA)
#define EOR_PRI_EOP                     (0xF3FC)

#define NFLAG_BE                        (34)
#define NFLAG_IN                        (1)
#define NFLAG_EN                        (4)

/* HDLC frame control field flag */
#define HDLC_CONTR_FIELD_LASTFRM        (0xC8)
#define HDLC_CONTR_FIELD_NOTLASTFRM     (0xC0)

#define HDLCCFCERR                      (0x7FFD)

#define T4_PPS                          (PPS_NULL >> 8)    /* 0x7D */
#define T4_EOR                          (EOR_NULL >> 8)    /* 0x73 */
#define T4_RR                           (HDLC_TYPE_T4_RR >> 8)    /* 0x76 */
#define T4_CTC                          (HDLC_TYPE_T4_CTC >> 8)    /* 0x48 */
#define T4_FCD                          (HDLC_TYPE_T4_FCD >> 8)    /* 0x60 */
#define T4_RCP                          (HDLC_TYPE_T4_RCP >> 8)    /* 0x61 */

#define T30_THRESHOLD                   (40) /* less than 64 */

#define FCF2_NULL                       (0x00)
#define FCF2_EOM                        (0xF1)
#define FCF2_MPS                        (0xF2)
#define FCF2_EOP                        (0xF4)
#define FCF2_EOS                        (0xF8)
#define FCF2_PRI_EOM                    (0xF9)
#define FCF2_PRI_MPS                    (0xFA)
#define FCF2_PRI_EOP                    (0xFC)
#define FCF2_INIT                       (0xFF)

#endif
