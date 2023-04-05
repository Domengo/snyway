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

#if defined (_FAXAPI_H)
/* Reentry! */
#else
#define _FAXAPI_H

#include "porting.h"
#include "commtype.h"
#include "commdef.h"
#include "circbuff.h"

#define INTERFACE_IMG_LENGTH    (128) // enough to hold 10ms
#define INTERFACE_PARM_LEN_L    (100)
#define INTERFACE_PARM_LEN_S    (20)
#define NUMERIC_DIGIT_LENGTH    (21)
#define T30FRAMELEN             (64)

#define FCLASS_REV              (BIT0 | BIT1 | BIT2 | BIT3)
#define FCLASS_MAJ              (BIT4 | BIT5 | BIT6 | BIT7)
#define FCLASS_MAJ_SHIFT        (4)
#define FCLASS_REV_0            (0x1)
#define FCLASS_REV_1            (0x2)

#define FCLASS0                 (0x00)                   /* fax class 0   */
#define FCLASS1                 (0x10)                   /* fax class 1   */
#define FCLASS10                (FCLASS1 | FCLASS_REV_0) /* fax class 1.0 */
#define FCLASS2                 (0x20)                   /* fax class 2   */
#define FCLASS20                (FCLASS2 | FCLASS_REV_0) /* fax class 2.0 */
#define FCLASS21                (FCLASS2 | FCLASS_REV_1) /* fax class 2.1 */
#define FCLASS3                 (0x30)                   /* integrated fax */
#define FCLASS8                 (0x80)                   /* integrated fax */

// T.30 session subparameters
#define VR_R8_385               (0)
#define VR_R8_77                (BIT0)
#define VR_R8_154               (BIT1)
#define VR_R16_154              (BIT2)
#define VR_200_100              (BIT3)
#define VR_200_200              (BIT4)
#define VR_200_400              (BIT5)
#define VR_300_300              (BIT6)
#define VR_400_400              (BIT7)
#define VR_600_600              (BIT0 << 8)
#define VR_1200_1200            (BIT1 << 8)
#define VR_300_600              (BIT2 << 8)
#define VR_400_800              (BIT3 << 8)
#define VR_600_1200             (BIT4 << 8)
#define VR_100_100              (BIT5 << 8)

#define BR_2400                 (0x00)
#define BR_4800                 (0x01)
#define BR_7200                 (0x02)
#define BR_9600                 (0x03)
#define BR_12000                (0x04)
#define BR_14400                (0x05)
#define BR_16800                (0x06)
#define BR_19200                (0x07)
#define BR_21600                (0x08)
#define BR_24000                (0x09)
#define BR_26400                (0x0a)
#define BR_28800                (0x0b)
#define BR_31200                (0x0c)
#define BR_33600                (0x0d)

#define WD_0                    (0x00)
#define WD_1                    (0x01)
#define WD_2                    (0x02)
#define WD_3                    (0x03)
#define WD_4                    (0x04)

#define LN_A4                   (0x00)
#define LN_B4                   (0x01)
#define LN_UNLIMITED            (0x02)

#define DF_MH                   (0x00)
#define DF_MR                   (0x01)
#define DF_2D_UNCOMPRESSED      (0x02)
#define DF_MMR                  (0x03)
//... more DF to come

#define EC_NON_ECM              (0x00)
#define EC_ECM                  (0x01)
//... more EC to come

#define BF_NON_FILE_TRANSFER    (0x00)
//... more BF to come

#define ST_0                    (0x00)
#define ST_1                    (0x01)
#define ST_2                    (0x02)
#define ST_3                    (0x03)
#define ST_4                    (0x04)
#define ST_5                    (0x05)
#define ST_6                    (0x06)
#define ST_7                    (0x07)

#define JP_NON_JPEG             (0x00)
#define JP_ENABLE_JPEG          (0x01)
#define JP_FULL_COLOR           (0x02)
#define JP_ENABLE_HUFFMAN       (0x04)
#define JP_12_BITS_PEL_COMP     (0x08)
#define JP_NO_SUBSAMPLING       (0x10)
#define JP_CUSTOM_ILLUMINANT    (0x20)
#define JP_CUSTOM_GAMUT         (0x40)

// ECM Block Size
#define ECM_SIZE_0              (0)
#define ECM_SIZE_64             (1)
#define ECM_SIZE_256            (2)

// DTE->T.30 action commands
#define DTE_T30_NULL            (0x00)
#define DTE_T30_ATD             (0x01)
#define DTE_T30_ATA             (0x02)
#define DTE_T30_FDT             (0x03)
#define DTE_T30_FDR             (0x04)
#define DTE_T30_FIP             (0x05)
#define DTE_T30_FPS             (0x06)
// DTE->T.30 parameter commands
#define DTE_T30_FCLASS          (0x01)
#define DTE_T30_FCC             (0x02)
#define DTE_T30_FIS             (0x03)
#define DTE_T30_FCS             (0x04)
#define DTE_T30_FLI             (0x05)
#define DTE_T30_FPI             (0x06)
#define DTE_T30_FLP             (0x07)
#define DTE_T30_FSP             (0x08)
#define DTE_T30_FNR             (0x09)
#define DTE_T30_FIE             (0x0a)
#define DTE_T30_FPS_PARM        (0x0b)
#define DTE_T30_FCQ             (0x0c)
#define DTE_T30_FIT             (0x0d)
#define DTE_T30_FPP             (0x0e)
#define DTE_T30_FBO             (0x0f)
#define DTE_T30_FEA             (0x10)
#define DTE_T30_FMI             (0x11)
#define DTE_T30_FMM             (0x12)
#define DTE_T30_FMR             (0x13)
#define DTE_T30_FPR             (0x14)
#define DTE_T30_FLO             (0x15)
#define DTE_T30_FNS             (0x16)
#define DTE_T30_FCR             (0x17)
#define DTE_T30_FBU             (0x18)
#define DTE_T30_FAP             (0x19)
#define DTE_T30_FRQ             (0x1a)
#define DTE_T30_FAA             (0x1b)
#define DTE_T30_FCT             (0x1c)
#define DTE_T30_FHS             (0x1d)
#define DTE_T30_FRY             (0x1e)
#define DTE_T30_FMS             (0x1f)
#define DTE_T30_FND             (0x20)
#define DTE_T30_FFC             (0x21)
#define DTE_T30_FBS             (0x22)

// T.30->DCE action commands
#define T30_DCE_NULL            (0x00)
#define T30_DCE_ATD             (0x01)
#define T30_DCE_ATA             (0x02)
#define T30_DCE_ATH             (0x03)
#define T30_DCE_ATO             (0x04)
#define T30_DCE_FTS             (0x05)
#define T30_DCE_FRS             (0x06)
#define T30_DCE_FTM             (0x07)
#define T30_DCE_FRM             (0x08)
#define T30_DCE_FTH             (0x09)
#define T30_DCE_FRH             (0x0a)
#define T30_DCE_A8M             (0x0b)
#define T30_DCE_DLE_PRI         (0x0c)
#define T30_DCE_DLE_CTRL        (0x0d)
#define T30_DCE_DLE_RTNC        (0x0e)
#define T30_DCE_DLE_EOT         (0x0f)

#define MOD_NULL                (0)
#define MOD_V21_300             (3)
#define MOD_V27TER_2400         (24)
#define MOD_V27TER_4800         (48)
#define MOD_V29_7200            (72)
#define MOD_V17_L_7200          (73)
#define MOD_V17_S_7200          (74)
#define MOD_V29_9600            (96)
#define MOD_V17_L_9600          (97)
#define MOD_V17_S_9600          (98)
#define MOD_V17_L_12000         (121)
#define MOD_V17_S_12000         (122)
#define MOD_V17_L_14400         (145)
#define MOD_V17_S_14400         (146)

// DCE->T.30 V.34 commands
#define DCE_T30_NULL            (0x00)
#define DCE_T30_DLE_PRI         (0x01)
#define DCE_T30_DLE_CTRL        (0x02)
#define DCE_T30_DLE_EOT         (0x03)

// T.30->DCE data types
#define T30_DCE_DATA_NULL       (0x00)
#define T30_DCE_HDLC_DATA       (0x01)
#define T30_DCE_HDLC_OK         (0x02)
#define T30_DCE_HDLC_END        (0x03)
#define T30_DCE_HDLC_BAD_OPT    (0x04)
#define T30_DCE_HDLC_BAD_LAST   (0x05)
#define T30_DCE_HS_DATA         (0x06)
#define T30_DCE_HS_END          (0x07)

// DCE->T.30 data types
#define DCE_T30_DATA_NULL       (0x00)
#define DCE_T30_HDLC_DATA       (0x01)
#define DCE_T30_HDLC_END        (0x02)
#define DCE_T30_HDLC_BAD        (0x03)
#define DCE_T30_HS_DATA         (0x04)
#define DCE_T30_HS_END          (0x05)

// Result code responses
#define RESULT_CODE_NULL        (0)
#define RESULT_CODE_OK          (1)
#define RESULT_CODE_CONNECT     (2)
#define RESULT_CODE_RING        (3)
#define RESULT_CODE_NOCARRIER   (4)
#define RESULT_CODE_ERROR       (5)
#define RESULT_CODE_NODIALTN    (7)
#define RESULT_CODE_BUSY        (8)
#define RESULT_CODE_NOANS       (9)

// Information text responses
#define INFO_TXT_NULL           (0x00)
#define INFO_TXT_FCO            (0x01)
#define INFO_TXT_FCS            (0x02)
#define INFO_TXT_FIS            (0x03)
#define INFO_TXT_FTC            (0x04)
#define INFO_TXT_FTI            (0x05)
#define INFO_TXT_FCI            (0x06)
#define INFO_TXT_FPI            (0x07)
#define INFO_TXT_FNF            (0x08)
#define INFO_TXT_FNS            (0x09)
#define INFO_TXT_FNC            (0x0a)
#define INFO_TXT_FSA            (0x0b)
#define INFO_TXT_FPA            (0x0c)
#define INFO_TXT_FPW            (0x0d)
#define INFO_TXT_FPS            (0x0e)
#define INFO_TXT_FET            (0x0f)
#define INFO_TXT_FPS_FET        (0x10)
#define INFO_TXT_FHS            (0x11)
#define INFO_TXT_FPO            (0x12)
#define INFO_TXT_FHT            (0x13)
#define INFO_TXT_FHR            (0x14)
// DCE->T.30
#define INFO_TXT_FCERROR        (0x01)
#define INFO_TXT_F34            (0x02)

#define POST_PG_MSG_NULL        (0x00)
#define POST_PG_MSG_MPS         (0x01)
#define POST_PG_MSG_EOM         (0x02)
#define POST_PG_MSG_EOP         (0x03)

#define POST_PG_RSP_MCF         (0x01)
#define POST_PG_RSP_RTN         (0x02)
#define POST_PG_RSP_RTP         (0x03)
#define POST_PG_RSP_PIN         (0x04)
#define POST_PG_RSP_PIP         (0x05)

#define FAP_SUB                 (BIT0)
#define FAP_SEP                 (BIT1)
#define FAP_PWD                 (BIT2)

#define FCQ_RQ_1                (BIT0)
#define FCQ_RQ_2                (BIT1 | BIT0)
#define FCQ_TQ_1                (BIT2)
#define FCQ_TQ_2                (BIT3 | BIT2)

#define FNR_RPR                 (BIT0)
#define FNR_TPR                 (BIT1)
#define FNR_IDR                 (BIT2)
#define FNR_NSR                 (BIT3)

/* definition for +FBO command */
#define DIRECT_C_DIRECT_BD      (0)
#define REVERSED_C_DIRECT_BD    (1)
#define DIRECT_C_REVERSED_BD    (2)
#define REVERSED_C_REVERSED_BD  (3)

// single character command or transparent data command
#define NULL_DATA_CMD           (0)
#define DC1_XON                 (1)
#define DC2_READY               (2)
#define DC3_XOFF                (3)
#define CAN_STOP                (4)

#define V34_CONTROL_CHNL        (1)
#define V34_PRIMARY_CHNL        (2)

#define T31_DC1                 (0x51)
#define T31_DC3                 (0x53)
#define T31_FERR                (0x07)
#define T31_PRI                 (0x6b)
#define T31_CTRL                (0x6d)
#define T31_PPH                 (0x6c)
#define T31_RTN                 (0x6a)
#define T31_MARK                (0x68)
#define T31_RTNC                (0x69)
#define T31_C12                 (0x6e)
#define T31_C24                 (0x6f)
#define T31_P24                 (0x70)
#define T31_P48                 (0x71)
#define T31_P72                 (0x72)
#define T31_P96                 (0x73)
#define T31_P120                (0x74)
#define T31_P144                (0x75)
#define T31_P168                (0x76)
#define T31_P192                (0x77)
#define T31_P216                (0x78)
#define T31_P240                (0x79)
#define T31_P264                (0x7a)
#define T31_P288                (0x7b)
#define T31_P312                (0x7c)
#define T31_P336                (0x7d)

#define T32_MPS                 (0x2c)
#define T32_EOM                 (0x3b)
#define T32_EOP                 (0x2e)
#define T32_PRI                 (0x21)
#define T32_BC_Q                (0x3f)
#define T32_OVR                 (0x4f)
#define T32_VR0                 (0x61)
#define T32_VR1                 (0x62)
#define T32_LN0                 (0x63)
#define T32_LN1                 (0x64)
#define T32_LN2                 (0x65)
#define T32_WD0                 (0x66)
#define T32_WD1                 (0x67)
#define T32_WD2                 (0x68)
#define T32_WD3                 (0x69)
#define T32_WD4                 (0x6a)
#define T32_DF0                 (0x6b)
#define T32_DF1                 (0x6c)
#define T32_DF2                 (0x6d)
#define T32_DF3                 (0x6e)
#define T32_RB0                 (0x30)
#define T32_RB1                 (0x31)
#define T32_RB2                 (0x32)
#define T32_RB3                 (0x33)
#define T32_RB4                 (0x34)
#define T32_RB5                 (0x35)
#define T32_RB6                 (0x36)
#define T32_RB7                 (0x37)
#define T32_RB8                 (0x38)
#define T32_RB9                 (0x39)

// image width in pixels at different X resolution
#define TIFF_A4_WIDTH_R8                          (1728)
#define TIFF_A4_WIDTH_R16                         (3456)
#define TIFF_A4_WIDTH_100DPI                      (864)
#define TIFF_A4_WIDTH_200DPI                      (1728)
#define TIFF_A4_WIDTH_300DPI                      (2592)
#define TIFF_A4_WIDTH_400DPI                      (3456)
#define TIFF_A4_WIDTH_600DPI                      (5184)
#define TIFF_A4_WIDTH_1200DPI                     (10368)

#define TIFF_B4_WIDTH_R8                          (2048)
#define TIFF_B4_WIDTH_R16                         (4096)
#define TIFF_B4_WIDTH_100DPI                      (1024)
#define TIFF_B4_WIDTH_200DPI                      (2048)
#define TIFF_B4_WIDTH_300DPI                      (3072)
#define TIFF_B4_WIDTH_400DPI                      (4096)
#define TIFF_B4_WIDTH_600DPI                      (6144)
#define TIFF_B4_WIDTH_1200DPI                     (12288)

#define TIFF_A3_WIDTH_R8                          (2432)
#define TIFF_A3_WIDTH_R16                         (4864)
#define TIFF_A3_WIDTH_100DPI                      (1216)
#define TIFF_A3_WIDTH_200DPI                      (2432)
#define TIFF_A3_WIDTH_300DPI                      (3648)
#define TIFF_A3_WIDTH_400DPI                      (4864)
#define TIFF_A3_WIDTH_600DPI                      (7296)
#define TIFF_A3_WIDTH_1200DPI                     (14592)

typedef struct
{
    UDWORD                      verticalResolutionVR;
    UBYTE                       bitRateBR;
    UBYTE                       pageWidthWD;
    UBYTE                       pageLengthLN;
    UBYTE                       dataFormatDF;
    UBYTE                       errorCorrectionEC;
    UBYTE                       fileTransferBF;
    UBYTE                       scanTimeST;
    UBYTE                       jpegOptionJP;
} T30SessionSubParm;

typedef struct
{
    UBYTE                       isTransmit;
    UBYTE                       isV34Enabled;
    UBYTE                       actionCmd;
    UBYTE                       currentDteT30Cmd; // saved action cmd + FKS
    UBYTE                       isDiscCmd;            // +FKS or CAN
    UBYTE                       dataCmd;
    UBYTE                       postPgMsg;        // MPS/EOP/EOM
    UBYTE                       tempData[INTERFACE_IMG_LENGTH];
    UWORD                       tempDataLen;
    T30SessionSubParm           faxParmCapability;    // +FCC, fixing issue 138
    T30SessionSubParm           faxParmSettings;    // +FIS
    CHAR                        localID[NUMERIC_DIGIT_LENGTH];        // +FLI for CSI/TSI
    CHAR                        localPollingID[NUMERIC_DIGIT_LENGTH];    // +FPI for CIG
    CHAR                        nonStandardFrm[T30FRAMELEN - 4];    // +FNS
    UBYTE                       nonStandardFrmLen;
    UBYTE                       isDocForPolling;    // +FLP
    UBYTE                       isRequestingToPoll;    // +FSP
    UBYTE                       isCapToRcv;    // +FCR
    UBYTE                       isHdlcRepEnabled;    // +FBU
    UBYTE                       negoReporting; // +FNR
    UBYTE                       addressPollingCap;    // +FAP
    CHAR                        destSubAddr[NUMERIC_DIGIT_LENGTH];    // +FSA
    CHAR                        selectivePollingAddr[NUMERIC_DIGIT_LENGTH];    // +FPA
    CHAR                        passWord[NUMERIC_DIGIT_LENGTH];    // +FPW
    UBYTE                       copyQtyChk;    // +FCQ
    UBYTE                       rcvQtyPglThrh;    // +FRQ.pgl
    UBYTE                       rcvQtyCblThrh;    // +FRQ.cbl
    UBYTE                       phaseCRspTimeout;    // +FCT
    UBYTE                       ecmRetryCnt;    // +FRY
    UBYTE                       minPhaseCSpd;    // +FMS
    UBYTE                       inactTimeout;    // +FIT.time
    UBYTE                       inactAction;    // +FIT.action
    UBYTE                       dataBitOrder;    // +FBO
} DteToT30Interface;

typedef struct
{
    UBYTE                       isV34Capable; // +F34?
    UBYTE                       resultCodeRsp;
    UBYTE                       infoTxtRsp;
    T30SessionSubParm           faxParmStatus;    // +FCS:
    T30SessionSubParm           remoteSettings; // DIS/DTC received
    UBYTE                       isHdlcXmitted;    // +FHT:
    UBYTE                       isHdlcRcved;    // +FHR:
    UBYTE                       hangupCode;    // +FHS:
    UBYTE                       isRmtDocToPoll;    // +FPO:
    UBYTE                       ecmFrmSize;
    UBYTE                       faxProtocol;
    UBYTE                       tempParm[INTERFACE_PARM_LEN_L];
    UBYTE                       tempParmLen;
    UBYTE                       dataCmd;
    UBYTE                       tempData[INTERFACE_IMG_LENGTH];
    UBYTE                       tempDataLen;
} T30ToDteInterface;

typedef struct
{
    UBYTE                       *pInactTimeout;    // +FIT.time
    UBYTE                       *pInactAction;    // +FIT.action
    UBYTE                       actionCmd;
    UBYTE                       currentT30DceCmd; // saved action cmd
    UBYTE                       flowCntlCmd;
    UBYTE                       carrierLossTimeout;
    UBYTE                       tempParm[INTERFACE_PARM_LEN_S];
    UBYTE                       tempParmLen;
    UBYTE                       tempData[270];
    UBYTE                       tempDataType;
    UWORD                       tempDataLen;
#if SUPPORT_V34FAX
    UBYTE                       isReqRateReduction; // <DLE><pph>
    UBYTE                       v8OpCntl; // +A8E
    UBYTE                       *pIsV34Enabled;
    UBYTE                       initV34PrimRateMax;
    UBYTE                       initV34PrimRateMin;
    UBYTE                       initV34CntlRate;
#endif
} T30ToDceInterface;

typedef struct
{
    UBYTE                       resultCodeRsp;
    UBYTE                       infoTxtRsp;
    UBYTE                       flowCntlCmd;
    SWORD                       dataTotalLen;
    UBYTE                       tempData[270];
    UBYTE                       tempDataType;
    UWORD                       tempDataLen;
#if SUPPORT_V34FAX
    UBYTE                       selectedV34Chnl;
    UBYTE                       priChnlBitRate;
    UBYTE                       ctrlChnlBitRate;
    UBYTE                       v8IndicationType;
    UBYTE                       v8IndicationData[INTERFACE_PARM_LEN_S];
    UBYTE                       v8IndicationLen;
    UBYTE                       isV34Selected;
    UBYTE                       isDleEot;
#endif
} DceToT30Interface;

extern CONST UBYTE T30_BytesPer10ms[];
extern CONST CHAR *T31_Action_Command[];
extern CONST CHAR *T32_Action_Command[];
extern CONST CHAR *T32_Info_Txt[];
extern CONST UWORD ImageWidth_Tiff[3][8];
extern CONST CHAR *T32_Parm_Command[];
extern CONST CHAR AT_PLUS_F[];
extern CONST CHAR PLUS_F[];

void COMM_ParseSubParmOut(T30SessionSubParm *pSubParm, CircBuffer *pCBOut);
UBYTE COMM_ParseSubParmIn(UBYTE *pBufIn, UBYTE *pIndex, T30SessionSubParm *pSubParm);

#endif
