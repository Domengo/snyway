/*****************************************************************************
*             Copyright (c) 1995 - 2007 by GAO Research Inc.                 *
*                          All rights reserved.                              *
*                                                                            *
*  This program is a confidential and an unpublished work of GAO Research    *
*  Inc. that is protected under international and Canadian copyright laws.   *
*  This program is a trade secret and constitutes valuable property of GAO   *
*  Research Inc. All use, reproduction and/or disclosure in whole or in      *
*  part of this program is strictly prohibited unless expressly authorized   *
*  in writing by GAO Research Inc.                                           *
*****************************************************************************/

#ifndef _TIFF_DEF_H_
#define _TIFF_DEF_H_

#define SUPPORT_FILE_OPERATION                    (1) // 1: TIFF works in the file mode, 0: in the buffer mode

#define MAX_NUM_TX_TIFF_FILES                     (3)
#define MAX_NUM_TX_TIFF_FILES_POLL                (MAX_NUM_TX_TIFF_FILES)

#define TIFF_FILE_IDLE                            (0)
#define TIFF_FILE_TRANSMIT                        (1)
#define TIFF_FILE_RECEIVE                         (2)

#define TIFF_TX_TIFF_FILE                         (1)
#define TIFF_TX_TEXT_FILE                         (2)
#define TIFF_TX_JPEG_CONVERSION_FILE              (3) // local tx jpeg, but remote does not support, so local converted the format

#define TIFF_COVER_PAGE_CALLER_SIDE               (1)
#define TIFF_COVER_PAGE_CALLEE_SIDE               (2)
#define TIFF_COVER_PAGE_SIDE_PATTERN              (3)

#define TIFF_COVER_PAGE_NAME                      (1)
#define TIFF_COVER_PAGE_FAX_NUMBER                (2)
#define TIFF_COVER_PAGE_PHONE_NUMBER              (3)
#define TIFF_COVER_PAGE_STREET                    (4)
#define TIFF_COVER_PAGE_CITYSTATE                 (5)
#define TIFF_COVER_PAGE_COUNTRY_ZIP               (6)
#define TIFF_COVER_PAGE_PATTERN                   (7)

#define TIFF_CPP_NO_COVER_PAGE                    (0)
#define TIFF_CPP_GAO_DEFAULT_1                    (1)
#define TIFF_CPP_CUSTOMER_ONLY_NEXT_CALL          (2)
#define TIFF_CPP_CUSTOMER_ALL_CALLS               (3)

#define TIFF_LEFT_UP                              (127)
#define TIFF_UP_RIGHT                             (128)
#define TIFF_LEFT_DOWN                            (129)
#define TIFF_DOWN_RIGHT                           (130)
#define TIFF_HORIZON                              (131)
#define TIFF_VERTICAL                             (132)
#define TIFF_LEFT_MIDDLE                          (133)
#define TIFF_UP_MIDDLE                            (134)
#define TIFF_MIDDLE_RIGHT                         (135)
#define TIFF_MIDDLE_DOWN                          (136)
#define TIFF_CROSS                                (137)

//TIFF File
#define TIFF_PMC_MIDDLE_OF_PAGE                   (1)
#define TIFF_PMC_NEXT_PAGE                        (2)
#define TIFF_PMC_END_OF_FILE_EOM                  (3)
#define TIFF_PMC_END_OF_FILE_EOP                  (4)
#define TIFF_PMC_COVER_PAGE                       (5)
#define TIFF_PMC_COVER_PAGE_END                   (6)


// states of TIFF
#define TIFF_STATE_INIT                           (0)  //Rx
#define TIFF_STATE_CHECK_PARAS                    (0)  //   Tx
#define TIFF_STATE_FILE_HEAD                      (1)  //Rx
#define TIFF_STATE_WAIT_OK                        (2)  //   Tx
#define TIFF_STATE_WAIT_CONNECT                   (3)  //Rx,Tx
#define TIFF_STATE_DURING_PAGE                    (4)  //Rx,Tx
#define TIFF_STATE_END_OF_PAGE                    (5)  //Rx,Tx
#define TIFF_STATE_LAST_PAGE                      (6)  //Rx
#define TIFF_STATE_END_OF_FILE                    (7)  //Rx,Tx
#define TIFF_STATE_WAIT_OK_COVERP                 (8)
#define TIFF_STATE_WAIT_CONNECT_COVERP            (9)
#define TIFF_STATE_DURING_PAGE_COVERP             (10)
#define TIFF_STATE_END_OF_PAGE_COVERP             (11)

#define TIFF_FILE_WRITE_HEAD                      (1)
#define TIFF_FILE_WRITE_IFD                       (2)
#define TIFF_FILE_WRITE_DATA                      (3)
#define TIFF_FILE_READ_IFD                        (4)
#define TIFF_FILE_READ_DATA                       (5)
#define TIFF_FILE_TX_TEXT_FILE                    (6)
#define TIFF_FILE_TX_COVER_PAGE                   (7)


// X resolution
#define TIFF_X_40D_PER_CM_R4                      (40)
#define TIFF_X_80D_PER_CM_R8                      (80)
#define TIFF_X_160D_PER_CM_R16                    (160)
#define TIFF_X_100D_PER_INCH                      (100)
#define TIFF_X_200D_PER_INCH                      (200)
#define TIFF_X_300D_PER_INCH                      (300)
#define TIFF_X_400D_PER_INCH                      (400)
#define TIFF_X_600D_PER_INCH                      (600)
#define TIFF_X_1200D_PER_INCH                     (1200)


//Vertical resolution
#define TIFF_Y_385L_PER_CM_NORMAL                 (39)
#define TIFF_Y_77L_PER_CM_FINE                    (77)
#define TIFF_Y_154L_PER_CM_PERFECT                (154)
#define TIFF_Y_100L_PER_INCH                      (100)
#define TIFF_Y_200L_PER_INCH                      (200)
#define TIFF_Y_300L_PER_INCH                      (300)
#define TIFF_Y_400L_PER_INCH                      (400)
#define TIFF_Y_600L_PER_INCH                      (600)
#define TIFF_Y_800L_PER_INCH                      (800)
#define TIFF_Y_1200L_PER_INCH                     (1200)

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

#define TIFF_A4_LENGTH_METRIC                     (297) // millimeters
#define TIFF_B4_LENGTH_METRIC                     (364) // millimeters
#define TIFF_A3_LENGTH_METRIC                     (420) // millimeters

#endif
