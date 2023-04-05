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

#ifndef _SMP_H
#define _SMP_H

#define INDE7_9LEN                (36)
#define INDE9_7LEN                (28)

#define INDE9_10LEN               (20)
#define INDE10_9LEN               (18)

#define INDE8_9LEN                (18)
#define INDE9_8LEN                (16)

#define INDE5_6LEN                (30)
#define INDE6_5LEN                (25)

#define INDE3_4LEN                (24)
#define INDE4_3LEN                (18)

#define INDE35_36LEN              (72)
#define INDE36_35LEN              (70)

#define INDE20_21LEN              (42)
#define INDE21_20LEN              (40)

/** For Sampling Rate Converter **/
#if !USE_ASM
typedef struct InDe_struct
{
    QWORD *pCoef;      /* pointer to  filter coefficients  */
    QWORD K_buf[144];  /* delay line    */

    UWORD mode;
    UWORD BufSizeIn;
    UWORD BufSizeOut;
    SWORD Counter;
    SWORD Offset;
    SWORD Index;
    UWORD inter;
    UWORD deci;
    UWORD len;
} InDe_Struct;
#else
typedef struct InDe_struct
{
    QWORD K_buf[36];   /* delay line, circular buffer     */
    QWORD *pCoef;      /* pointer to  filter coefficients  */

    UWORD mode;
    UWORD BufSizeIn;
    UWORD BufSizeOut;
    SWORD Counter;
    QWORD *Offset;
    SWORD Index;
    UWORD inter;
    UWORD deci;
    UWORD len;
} InDe_Struct;
#endif

void Init_InterDeci(InDe_Struct *, UWORD, UWORD, UWORD, QWORD *, UWORD, UWORD);
void SampleRate_Change(InDe_Struct *, QWORD *pIn, QWORD *pOut);

/** For Sampling Rate Converter **/
extern CONST QWORD lpf6_5_h[];
extern CONST QWORD lpf5_6_h[];
extern CONST QWORD lpf10_9_h[];
extern CONST QWORD lpf9_10_h[];
extern CONST QWORD lpf9_7_h[];
extern CONST QWORD lpf7_9_h[];
extern CONST QWORD lpf8_9_h[];
extern CONST QWORD lpf9_8_h[];
extern CONST QWORD lpf3_4_h[];
extern CONST QWORD lpf4_3_h[];
extern CONST QWORD lpf35_36_h[];
extern CONST QWORD lpf36_35_h[];
extern CONST QWORD lpf20_21_h[];
extern CONST QWORD lpf21_20_h[];
#endif
