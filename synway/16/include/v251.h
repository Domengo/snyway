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

#ifndef _V251_H_
#define _V251_H_

#define DCE_CONTROLLED_V8       (1)

// V.8 indication
#define V8_NULL                 (0x00)
#define V8_A8I                  (0x01)
#define V8_A8C                  (0x02)
#define V8_A8A                  (0x03)
#define V8_A8J                  (0x04)
#define V8_A8M                  (0x05)
#define V8_OK                   (0x06)

// A8E
#define A8E_NULL                (0x00)
#define A8E_V8O_0               (0x00)
#define A8E_V8O_1               (0x01)
#define A8E_V8O_2               (0x02)
#define A8E_V8O_3               (0x03)
#define A8E_V8O_4               (0x04)
#define A8E_V8O_5               (0x05)
#define A8E_V8O_6               (0x06)
#define A8E_V8O                 (0x0F)

#define A8E_V8A_0               (0x00)
#define A8E_V8A_1               (0x10)
#define A8E_V8A_2               (0x20)
#define A8E_V8A_3               (0x30)
#define A8E_V8A_4               (0x40)
#define A8E_V8A_5               (0x50)
#define A8E_V8A                 (0xF0)

#endif
