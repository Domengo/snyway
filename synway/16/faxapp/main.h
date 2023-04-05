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

#ifndef _MAIN_H_
#define _MAIN_H_

#include "commtype.h"

#define APP_FILE_OPERATION                               (1) // 1: file, 0:buffer
#define APP_SENT_COVER_PAGE                              (0) // 0: no cover page, 1: tx cover page
#define APP_SEND_FILE_TYPE                               (0) // 0: send "image.tif" file, 1: send "image.bmp" file, 2: send "test.txt" file

#define MAX_TIFF_FILE_LENGTH                             (1024*1024)
#define DCE_RX_IMAGE_QUALITY_CHECK_CAPA                  (1) // 0: no, 1: can check, 2: can correct
#define DCE_TX_IMAGE_QUALITY_CHECK_CAPA                  (0) // 0: no, 1: can check, 2: can correct
#define MIN_GOODLINES_PERCENTAGE_IN_A_PAGE               (95)// minimum tolerable percentage of good lines
#define MAX_CONSECU_BADLINES_IN_A_PAGE                   (5) // maximum tolerable number of consecutive bad lines

// Configuration Stages
#define APP_CS_TST_GEN                                   (0x00) // Testing non-class-specific DCE capabilities/information, e.g. Fax Class, version
#define APP_CS_TST_GEN_RSP                               (0x01)
#define APP_CS_INI_DCE                                   (0x02) // Initialization string
#define APP_CS_INI_DCE_RSP                               (0x03)
#define APP_CS_SET_FCLASS                                (0x04) // Setting Fax Class
#define APP_CS_SET_FCLASS_RSP                            (0x05)
#define APP_CS_TST_FCC                                   (0x06) // Testing Class 2 DCE capabilities. Skipped in Class1.
#define APP_CS_TST_FCC_RSP                               (0x07)
#define APP_CS_SET_SUB_PARMS                             (0x08) // Setting Class 2 sub-parameters
#define APP_CS_SET_SUB_PARMS_RSP                         (0x09)
#define APP_CS_SET_LOCAL_ID                              (0x0A) // Setting Class 2 local ID
#define APP_CS_SET_LOCAL_ID_RSP                          (0x0B)
#define APP_CS_SET_FMS                                   (0x0C) // Setting Class 2 minimum Phase C speed
#define APP_CS_SET_FMS_RSP                               (0x0D)
#define APP_CS_SET_FCR                                   (0x0E) // Setting Class 2 local receiving capability
#define APP_CS_SET_FCR_RSP                               (0x0F)
#define APP_CS_SET_FBO                                   (0x10) // Setting Class 2 bit order
#define APP_CS_SET_FBO_RSP                               (0x11)
#define APP_CS_SET_FNR                                   (0x12) // Setting Class 2 negotiation reporting
#define APP_CS_SET_FNR_RSP                               (0x13)
#define APP_CS_WRT_FCC                                   (0x14) // Writing Class 2 DCE capabilities
#define APP_CS_WRT_FCC_RSP                               (0x15)
#define APP_CS_CFG_LAST                                  (0x16) // Tx file pointer, or setting auto-answer ringing number
#define APP_CS_CFG_LAST_RSP                              (0x17)
#define APP_CS_END                                       (0x18)

typedef struct
{
    UBYTE         FClass;
    UBYTE         isDial;
    UBYTE         isV34Enabled;
    UBYTE         isEcmEnabled;
    UBYTE         ringNum;
    UBYTE         maxBitRate;
    UBYTE         minBitRate;
    UBYTE         IsJPEGEnabled;
    UBYTE         dialNum[21];
    UBYTE         localID[21];
    UBYTE         tiffFile[100];
} DTE_InitStruct;

void FileConversionFromBmpToTiff(FILE *fBmp, FILE *fTif);
SWORD FileConversionFromTiffToBmp(FILE *fTif, FILE *fBmp);

#endif
