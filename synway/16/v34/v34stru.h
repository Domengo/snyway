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

#ifndef _V34STRU_H
#define _V34STRU_H

#include "ph2stru.h"
#include "v34def.h"
#include "sharestr.h"

typedef struct
{
    UBYTE update_counter;
    QWORD qAve_egy;
    QWORD qHLK[6];
    QWORD qGain;
    SWORD freeze;
    SBYTE freeze_EQ;
    SWORD AGC_gain;
} V34AgcStruc;

typedef struct
{
    QWORD qTone1;
    QWORD qTone2;
} Retrain_Tone;

typedef struct
{
    CQWORD cS1;
    CQWORD cS2;
} iD4SYMBOL;

typedef struct
{
    DFT_Info tonedet;     /* Detect tone structure */

    QWORD pqQ1[3];
    QWORD pqQ2[3];
    QWORD pqCoef[3];
    QWORD pqMag_sqr[3];

    UBYTE tone_count;
    UBYTE Tone_50ms;
    UBYTE ToneDet_Flag;
} RetrainStruc;

typedef struct
{
    UWORD quadsym1;
    UWORD quadsym2;

    UBYTE S_DetCounter;
    UBYTE clear_down;
    UBYTE renego;
    UBYTE renego_generate;
} RenegoStruc;

typedef struct
{
    CQWORD pcBulk_delay[V34_MAX_BULK_LEN];
    CQDWORD pcNe_coef[V34_ECHO_COEF_LEN];
    CQDWORD pcFe_coef[V34_ECHO_COEF_LEN];

    QWORD qNe_beta;
    QWORD qFe_beta;

    EC_InsertStru Ec_Insert;

    UWORD ne_bulk_outidx;
    UWORD fe_bulk_outidx;

    UBYTE ec_adapt;
} EchoStruc;

typedef struct
{
    CQWORD pcEqcoef[V34_EQ_LENGTH];
    CQWORD pcEqdline[4*V34_EQ_LENGTH];  /* 2/3T Eq */

    CQWORD pcEqScoef[V34_EQ_S_LEN];
    CQWORD pcEqSdline[2*V34_EQ_S_LEN];
} EqStruc;

#if RX_PRECODE_ENABLE
typedef struct
{
    CQWORD cNWCoef[V34_PRECODE_COEF_LEN];
    CQWORD cNWDline[V34_PRECODE_COEF_LEN << 1];
    UBYTE  NWDlineIdx;
    UBYTE  ubBetaShift;
} NoiseWhitenStruct;
#endif

typedef struct
{
    UBYTE pData_sequence_mp[188];
    UDWORD frame_sync_mp;

    UBYTE type;                  /* bit 18 */
    UBYTE max_call_to_ans_rate;  /* bit 20-23, value 1-14 */
    UBYTE max_ans_to_call_rate;  /* bit 24-27, value 1-14 */
    UBYTE aux_channel_select;    /* bit 28 */

    UBYTE TrellisEncodSelect;    /* bit 29-30, value 0-2, value 3 ITU reserved */
    UBYTE NonlinearEncodParam;   /* bit 31 */
    UBYTE const_shap_select_bit; /* bit 32 */
    UBYTE acknowledge_bit;       /* bit 33 */

    UWORD data_rate_cap_mask;
    UBYTE asymmetric_data_sig_rate; /* bit 50 */

    /*************************************************************************/
    /* starting from bit 52, Type 0 and Type 1 has different bit assignment  */
    /*************************************************************************/
    /* For MP sequence Type 1: */
    /* [0].r: bit 52-67; [0].i: bit 69-84; [1].r: bit 86-101; [1].i: bit 103-118; [2].r: bit 120-135; [2].i: bit 137-152 */
    CQWORD PrecodeCoeff_h[V34_PRECODE_COEF_LEN];

    SWORD bit_num_cntr_mp;
    UWORD CRC_mp;
} MpStruc;

typedef struct _V34_struct V34Struct;
typedef void (*V34FnPtr)(V34Struct *);

typedef struct
{
    SBYTE tx_vec_idx;
    SBYTE rx_vec_idx;

    SBYTE result_code;

    UBYTE  S_point;
    UBYTE  S_point_BAR;
    UBYTE  detJ_yet;
    UBYTE  detJ_bit_count;

    UBYTE pByte_stream[2];
    QWORD pqTime_out_value[3];  /* record the time out value for phase3 */

    /* declare the array of pointer to functions */
    V34FnPtr pfTx[V34_CALL_MODEM_TX_VEC_SIZE];
    V34FnPtr pfRx[V34_CALL_MODEM_RX_VEC_SIZE];
    SBYTE ec_vec_idx;
    V34FnPtr pfEc[V34_CALL_MODEM_EC_VEC_SIZE];

    UBYTE K;
    UBYTE I;

    SWORD symbol_counter;
    SWORD detect_symbol_counter;

    SWORD  TimeOut;
    SDWORD Time_cnt_J_start;
    SDWORD Time_cnt_SSBar_start;
    SDWORD EC_Train_Counter;

    //UBYTE DelayIndex;
    //UBYTE EqDelayCount;
} Phase3_Info;

typedef struct
{
    MpStruc MpTx;
    MpStruc MpRx;

    QWORD pqTime_out_value[3];  /* record the time out value for phase4 */

    UBYTE pMp_buf[V34_MP_1_SIZE+1];
    UBYTE pMp_buf_pi[V34_MP_1_SIZE+1];
    UBYTE pByte_stream[10];

    V34FnPtr pfTx[V34_CALL_MODEM_TX_VEC_SIZE];
    V34FnPtr pfRx[V34_CALL_MODEM_RX_VEC_SIZE];
    SBYTE    ec_vec_idx;
    V34FnPtr pfEc[V34_CALL_MODEM_EC_VEC_SIZE];

    UDWORD curr_word;

    SWORD detect_symbol_counter;

    SWORD send_symbol_counter;

    SWORD mp_idx;

    SWORD mp_size;
    SWORD TimeOut;

    SDWORD Time_cnt_E_start;
    SDWORD Time_cnt_Jpi_start;
    SDWORD Time_cnt_SSBar_start;

    SBYTE tx_vec_idx;
    SBYTE rx_vec_idx;

    SBYTE result_code;
    SBYTE mp_scram_size;
    UBYTE S_point;
    UBYTE S_point_BAR;

    UBYTE Phase4_TxEnd;
    UBYTE Phase4_RxEnd;
} Phase4_Info;

/* Each VA_DecoderStr represents one state, eg. if a 16-states decoder */
/* is used, you need 16 of this structure.                          */
typedef struct
{
    /* keep the survived information path */
    iD4SYMBOL pInfo_path[V34_VA_WINTHRESHOLD];/* Info is the decoded 4D symbol */

    UBYTE pPrev_State_idx[V34_VA_WINTHRESHOLD]; /* 25 Back trace state idx */

    SDWORD metric;
} VA_DecoderStr;


typedef struct
{
    UBYTE d4set;
    UBYTE branch;
    UBYTE state;
} VA_NUMBER;


typedef struct
{
    UBYTE *pTrellis_ptr;
    UBYTE **pD4set_ptr;
} VA_DATA_ADDRESS;


typedef struct
{
    VA_NUMBER       number;
    VA_DATA_ADDRESS Addr;
    VA_DecoderStr      pST_ptr[16];/* VA decoder structure of 16 states */

    CQWORD pD2_near1[8];        /* assuming 8 2d subsets */
    CQWORD pD2_near2[8];        /* assuming 8 2d subsets */
    SDWORD pD4_metric[32];      /* considering maximum 32 4d symbsets */

    SWORD count;                /* Accumulated metric normalized counter */
    UBYTE win_size;             /* Number of history windows or Viterbi trace back Window size (the depth of decoder) = 25*/

    UWORD rx_bit_inv_pat;       /* Receiver Bit-inversion pattern */
    UWORD current_rx_inv_bit;

    SBYTE hist_idx;             /* History window index */
    UBYTE D4_sym_idx;           /* 4D symbol index in one mapping frame, 0..3 */
    UBYTE current_rx_P;
    UBYTE current_rx_J;

    UBYTE VA_sym_inidx;
    UBYTE VA_sym_outidx;
    UBYTE VA_sym_count;

    UBYTE rx_COV_type;   /* Which convolution encoder, 0:16 state, 1:32 state, 2:64 state */

    CQWORD pcVA_sym[V34_VA_BUF_SIZE];

    /* For Sync detector */
#if 0
    CQWORD pDec_syn_buf[V34_VA_WINTHRESHOLD];
    UBYTE  sync_size;
    UBYTE  sync_idx;
    SBYTE  sync_Cg;
    SBYTE  sync_Cl;
    UBYTE  sync_TH;
#endif
} VA_INFO;

typedef struct
{
    SBYTE rx_b;
    SBYTE rx_K;
    SBYTE rx_Ws;
    SBYTE rx_Js;

    UWORD rx_AMP;
    UWORD rx_SWP;

    SBYTE rx_P;
    UBYTE rx_nbyte;  /* Number of byte to descrambler */
    UBYTE rx_nbits;  /* Number of remaining bits to descrambler */

    SBYTE rx_M;
    SBYTE rx_q;
} RX_ModemDataStruc;

typedef struct
{
    VA_INFO           Vdec;     /* This is a standard Viterbi algorithm structure */
    RX_ModemDataStruc RxModemData;

    QDWORD  qdScaleAdjust; /* To adjust the power level small difference between phase4 and data mode */
    QDWORD  qdScaleAdjustSum;
    QDWORD  qdScaleAdjustSum1;
    SWORD   nScaleAdjustCount;

    SBYTE dZ;  /* The differential decoder value */
} DeCodeStruc;

typedef struct
{
    SWORD nTimingIdx;

    SWORD baud_adjust_in;
    SWORD baud_adjust_out;

    UWORD offset;

    SBYTE sample_slip_count;

    QWORD qT_Error_Out1;  /* 1.15 format integer */
    QWORD qT_Error_Out2;  /* 1.15 format integer */

    CQWORD pcBaud_adjust[3];
    CQWORD pcBaud_adjust_Dline[V34_BAUD_ADJUST_LEN];
    CQWORD pcT3_delay[4];

    CQWORD pcPolyDline[TIMING_TAPS14 << 1];
    QWORD  *pqCoef;
    CQWORD *pcDline;

    SBYTE  drift_direction;
    UWORD  drift_thresh;
    SWORD  drift_count;
    SWORD  drift_start;

    SBYTE Tcount;

    QDWORD qdError_phase;
    UDWORD udError_phase_out;

    QWORD qS;
    QWORD qC;
    QWORD qCarAcoef;
    QWORD qCarBcoef;

    QCFIRStruct eqfir;
    QWORD qB3;
} TimeCarrRecovStruc; /* Added by Hanyu for Timing/Carrier Recovery */

#if SUPPORT_V34FAX
#include "v34fdef.h"
#include "v34fstru.h"
#endif

#if PHASEJITTER
typedef struct
{
    UBYTE  enable;
    UBYTE  mu_shift;
    QWORD  qdJTout;
    QWORD  qdCoef[V34_JTTAPS];
    QWORD  dl[V34_JTTAPS << 1];
    QWORD  qPde_1;
    QDWORD qdPdey;
    UBYTE  ubOffset;
    QWORD  gain;
    QWORD  counter;
    QWORD  idx;
    QWORD  qC;
    QWORD  qS;
    QDWORD xcorr1;
    QDWORD xcorr2;
    QDWORD pow;
} JTstruct;
#endif

typedef struct
{
    UBYTE  pRx_g2[140];
    UWORD  pRx_g4[140];
    UDWORD pRx_g8[140];
    UDWORD pRx_z8[140];
} RX_ShellMapStruct;

typedef struct
{
    SWORD DetSCount;
    CQDWORD SEgy;
    CQDWORD SCarEgy;
    QDFTPUREStruct Stone;
    QDFTPUREStruct SRef;

    QCFIRStruct EQ_S; /* Equalizer for S to S' transition */
} S_Tone_DetStruc;

typedef struct
{
    QFIRStruct hilbfir;   /*  Hilbert Transform filter structure */
    QWORD pqHilbdline[HILB_TAP_LEN << 1];

    UBYTE rx_carrier_idx;   /* current demodulation carrier value */
    QWORD qRx_carrier_offset_idx;
} DeModulateStruc;

typedef struct
{
    void (*pfDescram_bit)(UBYTE *, UDWORD *, UBYTE *, UBYTE);
    void (*pfDescram_byte)(UBYTE *, UDWORD *, UBYTE *, UBYTE);

    V34StateStruc      V34State;
    RenegoStruc        Renego;
    TimeCarrRecovStruc TCR;
    DeModulateStruc    DeMod;
    S_Tone_DetStruc    S_ToneDet;
    DeCodeStruc        DeCode;
    RX_ShellMapStruct  RxShellMap;
#if RX_PRECODE_ENABLE
    PrecodeStruct      rx_precode;
    PrecodeStruct      VA_precode;
#endif
    V34AgcStruc        rx_AGC;

    CQWORD pcRx_sym[V34_SYM_BUF_SIZE];
    UBYTE rx_sym_outidx; /* Output index for retreiving old symbols */
    UBYTE pDscram_buf[V34_SCRAM_BUF_SIZE];

    QDWORD qdARS_erregy;

    UBYTE rx_sym_count;

    SBYTE current_rx_P;
    SBYTE current_rx_J;

    UWORD current_rx_SWP_bit;

    CQWORD cqEQ_symbol;

    QWORD  qRx_carrier_offset;
    UBYTE  rx_carrier_freq;  /* Rx carrier freq */

    UBYTE  rx_nlinear;    /* Indicated Nonlinear encoder is used */
    QDWORD qRx_nl_scale;
    QDWORD qdRx_scale;    /* 1 / transmit symbol scaling */

    QDWORD qdEq_scale;    /* Equal to tx scale */

#if DRAWEPG
    CQWORD pEpg[2];  /* for EPG use */
#endif

    SWORD  Retrain_Request_Counter;
    SBYTE  Retrain_LO_BER_Counter;
    SBYTE  Retrain_HI_BER_Counter;

    QWORD qEq_nl_scale;

    UBYTE rx_sym_inidx;  /* Input index for inserting new symbols */

    SWORD qPrev_erregy;
    SWORD qSum_erregy;

    UBYTE Retrain_flag;

    UBYTE ubEcDiscCount;

    UDWORD dsc;

    QWORD qRTRN_Req_Thres;

    UBYTE S_trn_point;

#if PHASEJITTER
    JTstruct jtvar;
#endif

    RetrainStruc Rtrn;

    QWORD qEchoCancel_Out[EC_INTER_NUM]; /* Echo-Cancel's output and Demoduale's input */

    //QWORD qTrn_avg;
    //SWORD trn_count;
    QWORD qEq_Beta;
    UBYTE ubB1_Err_Count;

#if SUPPORT_V34FAX
    UBYTE V34fax_Enable;
#endif
} ReceiveStruc;

typedef struct
{
    ReceiveStruc RecStruc;

    void (*pfScram_bit)(UBYTE, UDWORD *, UBYTE *, UBYTE);
    void (*pfScram_byte)(UBYTE *, UDWORD *, UBYTE *, UBYTE);
    void (*pfScramTrn_bit)(UBYTE, UDWORD *, UBYTE *, UBYTE);
    void (*pfScramTrn_byte)(UBYTE *, UDWORD *, UBYTE *, UBYTE);

    QWORD qRx_avg_egy;    /* Used for non-linear decoder */

    SBYTE rx_me;

    /* Scrambler list for training */
    UBYTE train_scram_idx;
    UBYTE md_length;

    UBYTE Num_Trn_Rx;    /* Number of TRN points for receive */
    UBYTE Num_Trn_Tx;    /* Number of TRN points for transmit */

    UBYTE rx_symbol_bits;
    UBYTE tx_bit_rate;   /* Data bit rate */
    UBYTE rx_bit_rate;

    UBYTE rx_symbol_rate;
    UBYTE rx_high_freq;  /* use High carrier freq in receiver */
    UBYTE tx_symbol_rate;/* Modem symbol rate */

    UBYTE Host_maxbitrate;
    UBYTE Host_minbitrate;

    UBYTE modem_mode;    /* Call modem=1  or answer modem= 0 */

    UWORD current_rx_AMP_bit;

    UWORD sequence;        /* Det_J_Sequence */
    UWORD J_point;

    SWORD RTD_PH2_symbol; /* Round-trip delay(symbol) estimate value from PH2 40ms */

    EqStruc Eq;

    /* SC_MpStruc Mp; */
    UBYTE S2743;                 /* bit 12 */
    UBYTE S2800;                 /* bit 13 */
    UBYTE S3429;                 /* bit 14 */
    UBYTE S3000_low_carrier;     /* bit 15 */
    UBYTE S3000_high_carrier;    /* bit 16 */
    UBYTE S3200_low_carrier;     /* bit 17 */
    UBYTE S3200_high_carrier;    /* bit 18 */
    UBYTE S3429_enable;          /* bit 19 */
    UBYTE reduce_transmit_power; /* bit 20 */
    UBYTE max_allowed_diff;      /* bit 21-23, value 0-5 */
    UBYTE transmit_from_CME;     /* bit 24 */
    UBYTE V34bis;                /* bit 25 */
    UBYTE V34bisL;               /* bit 25 */

    CQWORD cC2m;
    CQWORD cP2m;

    UDWORD sc;

    UDWORD train_sc;

    UBYTE pTrain_scram_buf[16];

    CQWORD tTimingOut[3];

    QWORD  qNLDFactor;

#if FREQUENCY_OFFSET
    QWORD  frequencyOffset;
#endif
} V34RxStruct;

struct _V34_struct
{
    UBYTE **pTable;

    V34RxStruct V34Rx;
    V34TxStruct V34Tx;
    //EchoStruc   Echo;
    Phase3_Info p3;   /* Phase 3 info structure */
    Phase4_Info p4;   /* Phase 4 info structure */

    QWORD qRTD_factor;

    SBYTE U0;            /* Trellis encoder output bit */

    SBYTE clear_down;    /* the clear down flag, initial to 0 in phase4 init */

    UWORD fecount;

    SDWORD Time_cnt;

    UBYTE NumSymbol;

#if RX_PRECODE_ENABLE
    NoiseWhitenStruct NoiseWhiten;
#endif

    SBYTE LAL_modem;

#if SUPPORT_V34FAX
    V34FaxStruct V34Fax;
#endif
};

/* Local structure for copy Phase 2 information */
typedef struct
{
    UBYTE bit_rate;
    UBYTE high_carrier_freq;
    UBYTE symbol_rate;
    UBYTE preemp_idx;
    UBYTE power_reduction;
    UBYTE md_length;
} V34SetupStruct;

#endif
