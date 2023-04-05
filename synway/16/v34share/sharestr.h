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

#ifndef _SHARESTRU_H
#define _SHARESTRU_H

#include "sharedef.h"
#include "dspdstru.h"

#if (TX_PRECODE_ENABLE + RX_PRECODE_ENABLE)
typedef struct
{
    /* Order=> h2.r h1.r h0.r h2.i h1.i h0.i */
    QWORD pCoef[V34_PRECODE_COEF_LEN * 2];
    /* Double buffer delayline, order=> d0.r d0.i d1.r d1.i d2.r d2.i */
    QWORD pDelay[V34_PRECODE_COEF_LEN * 2 * 2];
    CQWORD cC;
    CQWORD cP;
    UBYTE DelayIdx;
} PrecodeStruct;
#endif

typedef struct
{
    UBYTE b;
    UBYTE K;
    UBYTE Ws;
    UBYTE Js;

    UWORD AMP;
    UWORD SWP;

    UBYTE P;
    UBYTE nbyte;
    UBYTE nbits;
} ModemData; /* This structure can not be changed!!! */

typedef struct
{
    UBYTE   pG2[140];
    UWORD   pG4[140];
    UDWORD  pG8[140];
    UDWORD  pZ8[140];

    SBYTE   K;             /* Mapping parameter, ITU V.34 Table 10 */
    SBYTE   M;             /* Mapping parameter, ITU V.34 Table 10 */
    UBYTE   scram_idx;
    UBYTE   pScram_buf[V34_SCRAM_BUF_SIZE];/* SC_SCRAM_BUF_SIZE=79 */
} ShellMapStruct;

typedef struct
{
    QIFIRStruct iFirI;
    QIFIRStruct iFirQ;

    QWORD DlineQ[V34_PSF_DELAY_LEN << 1];
    QWORD DlineI[V34_PSF_DELAY_LEN << 1];

    UBYTE tx_carrier_idx;   /* current modulation carrier value */
    UBYTE tx_carrier_freq;  /* Tx carrier freq */

    QWORD qTx_carrier_offset;
    QWORD qTx_carrier_offset_idx;

    UBYTE tx_high_freq;   /* use High carrier freq in transmiter */

    CircFilter tx_preemp;
    UBYTE tx_preemp_idx;
    QWORD pqTx_preemp_dline[14];

    UBYTE power_reduction;
    QWORD qTxScale;
} QAMStruct;

typedef struct
{
    QWORD qCos;
    QWORD qSin;

    UWORD max_bulk_len;
    UWORD ne_sublen;
    UWORD bulk_inidx;
} EC_InsertStru;

typedef struct
{
    UWORD current_AMP_bit;
    UWORD AMP;           /* Auxiliary channel multiplexing switching pattern */
    SBYTE Ws;            /* Auxiliary channel multiplexing parameters */
    UBYTE Aux_Flag;      /* Flag to tell using Aux. channel */
} AuxStruc;

typedef struct
{
    SWORD *PCMinPtr;
    SWORD *PCMoutPtr;

    ShellMapStruct ShellMap;
    QAMStruct      QAM;

    QDWORD qdTx_scale;     /* transmit symbol scaling */
    UBYTE  nlinear;        /* Indicated Nonlinear encoder is used */
    QWORD  qTx_nl_scale;   /* Used for non-linear encoder */

    /* Buffering output symbols, size must be power of 2 */
    CQWORD pcTx_sym[V34_SYM_BUF_SIZE];
    UBYTE tx_sym_inidx;  /* Input index for inserting new symbols */
    UBYTE tx_sym_outidx; /* Output index for retreiving old symbols */

#if TX_PRECODE_ENABLE
    PrecodeStruct tx_precode;
#endif

    SBYTE b;             /* Number of bits in one mapping frames */
    SBYTE q;             /* Mapping parameter, 2q=(b-K)/4-3, b>12 */
    UBYTE z;             /* The differential encoder value */
    UWORD tx_COV_state;  /* the convolution state value Q1..Q6 */
    SBYTE Y0;            /* Output bit from Convolution encoder */

    UBYTE current_P;     /* Indicate current mapping frame index */
    UBYTE P;             /* Number of Mapping frames in one Data frame. 0 <= P <= 16 */

    UWORD current_SWP_bit;

    UWORD SWP;           /* Switching pattern, ITU V.34 Table 8 */
    AuxStruc  Aux;

    UBYTE Js;            /* Number of Data frames in one Superframe */
    SBYTE current_J;     /* Indicate current data frame index */

    UWORD current_inv_bit; /* Current Bit inversion pattern, for finding V0 */
    UWORD bit_inv_pat;     /* Bit inversion pattern */

    UBYTE conv_state;    /* Convolution state */

    SBYTE Min_Exp;       /* 1 or 0, 1 means used expanded constellation */
    UBYTE nbyte;         /* Number of byte to scrambler */
    UBYTE nbits;         /* Number of remaining bits to scrambler */
} V34TxStruct;

#endif
