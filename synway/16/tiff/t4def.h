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
#ifndef T4_DEF_H
#define T4_DEF_H

#include "commtype.h"

// Color Space
#define CS_ITUYCC                           (6)
#define CS_CIELAB8                          (8)  // TIFF V6.0, June 03, 1992
#define CS_CIELAB9                          (9)  // TTN1, September 14, 1995
#define CS_CIELAB10                         (10) // T.42, July, 2003



// Build Options
#define SUPPORT_MR                          1   // T.4 2D //Currently must be enabled
#define SUPPORT_MMR                         1   // T.6 2D //Currently must be enabled

// Compression Modes
#define T4_1D_MH                            1
#define T4_2D_MR                            2
#define T6_2D_MMR                           3
#define JPEG_COLOR                          4
#define JPEG_GRAY                           5

// T.4 Codec Constants
#define T4_TREE_NEXT                        (-16000)
#define WHITE                               1
#define BLACK                               (!WHITE)
#define MAX_TERMINATING_CODE_WORD_LENGTH    63
#define COLOR_TREE_LENGTH                   182
#define COMBINED_TREE_LENGTH                26
#define ENCODER_CODEBOOK_LENGTH_SMALL       64
#define ENCODER_CODEBOOK_LENGTH_MEDIUM      27
#define ENCODER_CODEBOOK_LENGTH_LARGE       13
#define MAX_LARGE_RUN                       2560
#define MIN_LARGE_RUN                       1792
#define MIN_LARGE_MULTIPLE                  28

#define MAX_BYTES_IN_LINE                   320


#define T4_MAX_ZEROS_IN_CODEWORD            7
#define T4_MIN_ZEROS_IN_EOL                 11
#define T4_NUM_EOL_IN_RTC                   6
#define T4_NUM_EOL_IN_EOFB                  3   // 2 for EOFB + 1 for implied EOL at pagewidth
#define T4_MAX_TERMINATING_CODEWORD_LENGTH  63

// Return Value
#define T4_DECODER_ERROR                    (-1)
#define T4_PAGEWIDTH_TOO_WIDE               (-2)
#define T4_OUTPUT_BUFFER_OVERFLOW           (-3)
#define T4_COMPRESSION_MODE_NOT_SUPPORTED   (-4)
#define T4_DECODER_CONTINUE                 (0)
#define T4_DECODER_CODEWORD                 (1)
#define T4_DECODER_EOL                      (2)
#define T4_DECODER_RTC                      (3)
#define T4_DECODER_EOFB                     (4)
#define T4_DECODER_MAKEUP_CODEWORD          (5)
#define T4_DECODER_EOL_BAD_LINE             (6)
#define T4_DECODER_EOFB_BAD                 (7)

#define T4_ENCODER_STATE_EOL                0
#define T4_ENCODER_STATE_CODEWORD           1
#define T4_ENCODER_STATE_NEXT_BIT           2

#define T4_ENCODER_CONTINUE                 0
#define T4_ENCODER_DATA                     1
#define T4_ENCODER_ERROR                    (-1)

#define T4_CODEWORD_TERMINATING             0
#define T4_CODEWORD_MAKEUP                  1

#define T4_EOL_NUM_BITS                     12
#define T4_EOL_FILL_BITS                    16
#define T4_EOL_PATTERN                      0x0001
#define T4_EOL_OFFSET                       12
#define T4_EOL_MOD                          7

/* 2D */
#define BIT_MSB                             7
#define BIT_LSB                             0

// General
#define T4_MAX_OUTBIT_NUM                   12280
#define OUTBUF_BYTE_LENGTH                  ((T4_MAX_OUTBIT_NUM>>3)+1)

#endif //T4_DEF_H

