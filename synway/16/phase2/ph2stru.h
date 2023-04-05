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

#ifndef _PH2STRU_H
#define _PH2STRU_H

#include "dspdstru.h"
#include "ph2def.h"

typedef struct
{
    UBYTE hang_up;
    UBYTE hangup_reason;

    UBYTE retrain_reason;
    UBYTE Retrain_Times;
} V34StateStruc;

typedef struct
{
    QWORD  *pqQ1;                        /* Each Freq has 1 Q1, Q(n-1) */
    QWORD  *pqQ2;                        /* Each Freq has 1 Q2, Q(n-2) */
    QWORD  *pqCoef;                        /* Each Freq has 1 coef, coef = 2*COS(2*PI*k/N) */
    QWORD  *pqMag_sqr;                    /* Each Freq has 1 magnitude square result */

    QDWORD qEgy;
    SWORD  N;                            /* Number of samples to accumulate */

    SWORD  cur_N;                        /* Current sample number */
    UBYTE  Num_Freq;                    /* Total number of freq. need to detect */
} DFT_Info;

typedef struct
{
    UBYTE carrier_freq_idx;                /* these 3 MUST be first & in sequence */
    UBYTE preemphasis_idx;
    UBYTE project_data_rate;

    UBYTE symbol_rate_idx;
    UBYTE max_data_rate;
} SymRate_Info;                           /* changed from SC_SRATE_Info1c */

typedef struct
{
    UBYTE S2743;                      /* bit 12 */    /* !!!! MUST be first !!!! */
    UBYTE S2800;                      /* bit 13 */
    UBYTE S3429;                      /* bit 14 */
    UBYTE S3000_low_carrier;          /* bit 15 */
    UBYTE S3000_high_carrier;         /* bit 16 */
    UBYTE S3200_low_carrier;          /* bit 17 */
    UBYTE S3200_high_carrier;         /* bit 18 */
    UBYTE S3429_enable;               /* bit 19 */
    UBYTE reduce_transmit_power;      /* bit 20 */
    UBYTE max_allowed_diff;           /* bit 21-23, value 0-5 */
    UBYTE transmit_from_CME;          /* bit 24 */
    UBYTE V34bis;                     /* bit 25 */

    UBYTE transmit_clock_source;      /* bit 26-27, value 0-2 */
    UBYTE acknowledge_correct_recept; /* bit 28 */
#if SUPPORT_V90
    /* V90 Info0d parameters */
    UBYTE dig_nom_transmit_power;        /* 29-32  -6dBm0  to -21 dBm0, step 1   */
    UBYTE dig_max_transmit_power;        /* 33-37  0.5dBm0 to -16 dBm0, step 0.5 */
    UBYTE power_measure;                /* 38     1: codec output, 0: terminal  */
    UBYTE dig_A_law;                    /* 39     0: u-law */
    UBYTE dig_S3429_enable;                /* 40 */
    /* V90 Info0d parameters */            /* 41     not applicable to analog modem */
#endif

    UBYTE bit_num_cntr_0;

    UBYTE pData_sequence_0[V34_DATA_SEQUENCE_0];
    UWORD crc_0;
#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)
    UWORD crc_mH;
    UBYTE mH_byte;
#endif
} Info_0Struc;

typedef struct
{
    UBYTE pData_sequence_1a[V34_DATA_SEQUENCE_1A];
    UBYTE bit_num_cntr_1a;

    UBYTE ubAlignmentForfreq_offset;

    /* -- these 9 must be in sequence -- */
    UBYTE min_power_reduction;            /* bit 12-14, value 0-7 */
    UBYTE add_power_reduction;            /* bit 15-17, value 0-7 */
    UBYTE length_MD;                    /* bit 18-24, value 0-127 */

    UBYTE INFO1a_Bit25;                    /* In V34, it is bit25: high_carrier_freq; In V90a, it is bits25-31: PCM_Ucode_for2pt_train */
    UBYTE pre_em_index;                    /* bit 26-29, value 0-10 */
    UBYTE pro_data_rate;                /* bit 30-33, value 0-12 */
    UBYTE sym_rate_answer_to_call;        /* bit 34-36, value 0-5 */
    UBYTE sym_rate_call_to_answer;        /* bit 37-39, value 0-5 */
    SWORD freq_offset;                    /* bit 40-49, value -512 to 511 */

    UWORD crc_1a;
} Info_1aStruc;

#if SUPPORT_V34FAX
typedef struct
{
    UBYTE pData_sequence_h[V34_DATA_SEQUENCE_H];
    UBYTE bit_num_cntr_h;

    UBYTE ubAlignmentForfreq_offset;

    /* -- these 7 must be in sequence -- */
    UBYTE power_reduction;         /* bit 12-14, value 0-7   */
    UBYTE trn_length;              /* bit 15-21, value 0-127 */

    UBYTE high_carrier_freq;       /* bit 22,    value 0-1   */
    UBYTE pre_em_index;            /* bit 23-26, value 0-10  */
    UBYTE sym_rate_call_to_answer; /* bit 27-29, value 0-5   */

    UBYTE TRN_constellation;       /* bit 30,     value 0-1  */
    UWORD crc_h;
} Info_hStruc;
#endif

typedef struct
{
    UBYTE pData_sequence_1c[V34_DATA_SEQUENCE_1C]; /* DATA_SEQUENCE_1C 97 */
    UBYTE bit_num_cntr_1c;

    UBYTE ubAlignmentForfreq_offset;

    /* -- must be in sequence, down to ..S3429 -- */
    UBYTE min_power_reduction;            /* bit 12-14, value 0-7 */
    UBYTE add_power_reduction;            /* bit 15-17, value 0-7 */
    UBYTE length_MD;                    /* bit 18-24, value 0-127 */

    UBYTE high_carrier_freq_S2400;        /* bit 25,    value 0-1  */
    UBYTE pre_em_index_S2400;            /* bit 26-29, value 0-10 */
    UBYTE pro_data_rate_S2400;            /* bit 30-33, value 0-12 */

    UBYTE high_carrier_freq_S2743;        /* bit 34,    value 0-1  */
    UBYTE pre_em_index_S2743;            /* bit 35-38, value 0-10 */
    UBYTE pro_data_rate_S2743;            /* bit 39-42, value 0-12 */

    UBYTE high_carrier_freq_S2800;        /* bit 43,    value 0-1  */
    UBYTE pre_em_index_S2800;            /* bit 44-47, value 0-10 */
    UBYTE pro_data_rate_S2800;            /* bit 48-51, value 0-12 */

    UBYTE high_carrier_freq_S3000;        /* bit 52,    value 0-1  */
    UBYTE pre_em_index_S3000;            /* bit 53-56  value 0-10 */
    UBYTE pro_data_rate_S3000;            /* bit 57-60, value 0-12 */

    UBYTE high_carrier_freq_S3200;        /* bit 61,    value 0-1  */
    UBYTE pre_em_index_S3200;            /* bit 62-65, value 0-10 */
    UBYTE pro_data_rate_S3200;            /* bit 66-69, value 0-12 */

    UBYTE high_carrier_freq_S3429;        /* bit 70,    value 0-1  */
    UBYTE pre_em_index_S3429;            /* bit 71-74, value 0-10 */
    UBYTE pro_data_rate_S3429;            /* bit 75-78, value 0-12 */

    SWORD freq_offset;                    /* bit 79-88 */

    UWORD crc_1c;
} Info_1cStruc;

typedef struct
{
    QIFIRStruct dpskfir;
    QFIRStruct  LPfir;
    QFIRStruct  BPfir;

    QWORD pqFilter_delay_1[2 * V34_DELAYLINE_LENGTH];    /* Double buffering */
    QWORD pqFilter_delay_3[2 * V34_DPSK_CALL_LEN];        /* Double buffering */
    QWORD pqFilter_delay_4[2 * 48];                        /* Double buffering */

    QWORD pqSum_buf[PH2_SYM_SIZE];
    QWORD pqSdelay[PH2_SYM_SIZE];

    QWORD qEncode_bit;

    QWORD qWc;

    UBYTE DPSK_Flag;
    UBYTE DPSK_begin_flag;
    UBYTE DPSK_bit_cnt;

    UBYTE out_offset;

    UBYTE ubFrameSync;
    UBYTE ubFrameSyncCount;
} DPSKInfo;

typedef struct
{
    CQWORD *pcBuf;

    SWORD idx;
    SWORD size;
} cCIRC_BUF;

typedef struct
{
    CQWORD *cqW_vec;
    UBYTE *pPermute_idx;

    UBYTE num_step;
    UBYTE num_points;
} FFT_Data;

typedef struct
{
    QWORD qPrev_offset;
    QWORD qCurrent_offset;
} Freq_Offset;

typedef struct
{
    QWORD *pqPower_Spectrum;        /* point to Power_Spectrum_idx */

    QWORD qNoise_power;
    QWORD RESERVE;

    UBYTE p_idx;
    UBYTE lo_preemp;
    UBYTE hi_preemp;
    UBYTE pRESV;

    SWORD current_sample;

    SWORD power_counter;            /* Used for calculating the average power */

    /* 1--- 3100 Hz tone is detected, 0----This tone is not detected; */
    SWORD ptone_counter;
    SWORD L1_counter;
    SWORD L2_counter;
    UWORD error;                    /* exceptional error in probing results */

    UBYTE pFFT_permute_idx[64];

    CQWORD pcFFT_inbuf[64];
    CQWORD pcFFT_w_vec[64];

    QWORD pqL1_Power_Spectrum[32];
    QWORD pqL2_Power_Spectrum[32];

    /* total number of different frequency in the output signal */
    UWORD puCurrent_phase[V34_MAX_TONE_SIZE];

    FFT_Data FFT32_Info;
    cCIRC_BUF FFT_in;

    QDWORD pqdGain[6][11][25];        /* precalculated Gains to reduce MIPS */
} ProbStruc;

typedef struct Phase2Info Ph2Struct;
typedef void (*V34Ph2FnPtr)(Ph2Struct *);

struct Phase2Info
{
    SWORD *PCMinPtr;
    SWORD *PCMoutPtr;

    Info_0Struc I0check;
    Info_0Struc I0a;                /* INFO 0a structure */
    Info_0Struc I0c;                /* INFO 0c structure */
    Info_1aStruc I1a;                /* INFO 1a structure */
    Info_1cStruc I1c;                /* INFO 1c structure */

#if SUPPORT_V34FAX
    Info_hStruc Ih;       /* INFO h  structure */
    UBYTE V34fax_Enable;
    UBYTE V34fax_EQ_Mode;

    UBYTE FreqOffset_Enable;

    QWORD pPower_Spect[25];
    QDWORD drift1;
#endif

    DFT_Info tonedet;                /* Detect tone structure */
    ProbStruc Probing;                /* Probing structure */
    SymRate_Info pSymRate_info[6];

    SWORD pCounter_vec_tx[8];

#if SUPPORT_V90D
    UBYTE pInfo_buf[V34_INFO_0d_SIZE];
#else
    UBYTE pInfo_buf[V34_INFO_0_SIZE];
#endif

    UBYTE pInfo_buf1[V34_INFO_1C_SIZE];        /* INFO_1C_SIZE 109 */
    UBYTE pInfo_buf1A[V34_INFO_1A_SIZE];    /* SC_INFO_1A_SIZE 70 */

    QWORD pqQ1[3];
    QWORD pqQ2[3];
    QWORD pqCoef[3];
    QWORD pqMag_sqr[3];

    SWORD delay_counter_tx;
    SWORD delay_counter_rx;

    SWORD L1_counter;
    SWORD L2_counter;

    UBYTE RTD_Status;                        /* enable/disable (1/0) RTD counter */
    SWORD RTD_Symbol;                       /* Round-trip delay estimate value */

    SWORD Time_cnt_info1_start;
    SWORD Time_cnt_tone_start;

    SWORD Time_cnt_tonea1_start;
    SWORD Time_cnt;
    SWORD Time_cnt_B_PR_40MS;

    UBYTE init_recovery;
    UBYTE recovery_info0_detected;

    SBYTE info_idx;
    SBYTE tx_vec_idx;
    SBYTE rx_vec_idx;
    SBYTE counter_vec_tx_idx;

    UBYTE dettone;
    UBYTE Phase2_End;
    UBYTE CRC;

    UBYTE bit_0_count;
    UBYTE tone_count;

    UBYTE recovery;                            /* 1: in recovery mechanism, 0: error free */
    UBYTE send_info1c_done;
    UBYTE toneb1_detected;
    UBYTE toneA2_detected;

    SBYTE L2_done;
    SBYTE LineProbOn;
    QDWORD qdTotal_egy;
    QDWORD qdTotal_egy_biqin;

    IIR_Cas5Struct  biq;
    QWORD biq_dline[2*(3+1)];
    SWORD AGC_gain;

    DPSKInfo Dpsk;
    V34StateStruc Ph2State;

    UBYTE TimingDrift_Enable;                /* enable calculation of timing drift */

    QDWORD all_angles[PH2_ANGLE_NUM + 1];    /* L2 results for timing drift calculation */
    QDWORD prev_angle;
    QDWORD drift;

    QDWORD all_angles1[PH2_ANGLE_NUM + 1];
    QDWORD prev_angle1;

    UBYTE modem_mode;                        /* Call modem=1  or answer modem= 0 */

    UBYTE min_symbol_rate;
    UBYTE max_symbol_rate;

    UBYTE Retrain_flag;

    CONST V34Ph2FnPtr *pfTx;
    CONST V34Ph2FnPtr *pfRx;

    SWORD Ph2Timer;
    QWORD qNLDFactor;

    UBYTE V90_Enable;
    UBYTE V92_Enable;

#if SUPPORT_V90
    UBYTE Retrain_V90;
    UBYTE num_retrain;
    UBYTE ubDownstreamPowerIdx;
#endif
#if SUPPORT_V92
    UBYTE V92_Capability;
    UBYTE Short_Phase2;
#endif
#if (SUPPORT_V92A_MOH + SUPPORT_V92D_MOH)
    UBYTE mH_TxByte;
    UBYTE mH_RxByte;
#endif
#if SUPPORT_V92D
    UBYTE filter_sections;
    UWORD max_total_coefficients;
    UWORD max_each_coefficients;
#endif
#if SUPPORT_V92A
    UBYTE PCM_support;
#endif
#if DRAWEPG
    UWORD epg_pos;
#endif

    UBYTE  Silence_Count1;
    UBYTE  Silence_Count2;
    QDWORD Silence_Egy_Ref;
    QDWORD Silence_Egy;
    UBYTE  Silence_Detect_Count;
    QWORD  PrevOut;

    UBYTE  SNR_Beg_Count;
    UBYTE  SNR_test_End;
    UBYTE  Sym_Count;
    QDWORD qdNoise_Egy;
    QDWORD qdSNR_Egy;

    UWORD  uToneAorB_Timeout_Count;
    UWORD  uINFO0_detect_Timeout_Count;
    UWORD  uTimeout_Count;
};

#endif
