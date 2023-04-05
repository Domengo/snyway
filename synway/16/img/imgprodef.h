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


#ifndef _IMG_PRO_DEF_H_
#define _IMG_PRO_DEF_H_
#include "imgprodte.h"

#if SUPPORT_CONVERSION_BETWEEN_T4_T6


#include "t4info.h"
#include "tiffdef.h"

#define IMAGE_BYTES_PER_FRAME                     (400)                    // Must be high enough to finish page before next page

#define MAX_IMAGE_WIDTH                             (TIFF_A3_WIDTH_1200DPI)
#define BITMAP_LENGTH                             (MAX_IMAGE_WIDTH + 4)
#define BITMAP_BUFFER_LENGTH                     (4 * BITMAP_LENGTH)    // UWORDS: Decoder Reference, Decoder Bitmap, Encoder Bitmap, Encoder Reference

#define MAX_IMAGE_LEN_AFTER_ENCODING             (0x4000)               // must be the power of 2, corresponding to 128 bytes of another compression format,
// 128bytes MMR input, output must be less than this value in MH or MR

#define IMG_IN_MIDDLE                            (1)
#endif
#define IMG_NO_CONVERSION                        (0)

#if SUPPORT_JPEG_CODEC
#define IMG_LAB_BLACK_L                          (0)
#define IMG_LAB_BLACK_A                          (128)
#define IMG_LAB_BLACK_B                          (96)
#define IMG_LAB_WHITE_L                          (255)
#define IMG_LAB_WHITE_A                          (128)
#define IMG_LAB_WHITE_B                          (96)
/*#define IMG_LAB_BLACK_L                          (0)
#define IMG_LAB_BLACK_A                          (0)
#define IMG_LAB_BLACK_B                          (0)
#define IMG_LAB_WHITE_L                          (255)
#define IMG_LAB_WHITE_A                          (255)
#define IMG_LAB_WHITE_B                          (255)*/
#endif

#endif
