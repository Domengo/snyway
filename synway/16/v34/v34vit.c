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

/******** Fix Point Viterbi Algorithm (VA) decoding ***********************/
/*      MODULE:                                                           */
/*  void  V34_VA_Decode(DeCodeStruc *DeCode, CQWORD *pOut9_7)             */
/*  void VA_init(VA_NUMBER *, VA_DATA_ADDRESS *, VA_INFO *)               */
/*      FileName:                                                         */
/*       viterbi.c                                                        */
/*      INPUT:                                                            */
/*      cDspDATA *in         - Input a 4D symbol                          */
/*      OUTPUT:                                                           */
/*        CQWORD *out       - Output a 4D symbol                          */
/*   NOTES:                                                               */
/*      1.  You should use VA_init() before using this Viterbi            */
/*          Algorithm.                                                    */
/*      2.  external function: D4Min() and ArrayMin() are general for     */
/*          all cases; however, different application may require to      */
/*          modify the external function D2Min();                         */
/*      3.  VA_StateNum             -Number of states                     */
/*          VA_BranchNum            -Number of branches leading           */
/*          to a state                                                    */
/*          VA_D2SetNum             -Number of D2 subsets                 */
/*          VA_D4SetNum             -Number of D4 subsets                 */
/*          *They are all defined in "viterbi.h"                          */
/*      4.  Use 16-bit integer to represent the input, the range would    */
/*          be:  -32768<=...<=32767                                       */
/*      5.  the input range: -256<= input <= 255                          */
/**************************************************************************/

#include "v34ext.h"

extern UWORD Flag_VA;

#if !USE_ASM

#if VITERBI_ON
void  V34_VA_Decode(DeCodeStruc *DeCode, CQWORD *pOut9_7)
{
    RX_ModemDataStruc *RxMod = &(DeCode->RxModemData);
    VA_DecoderStr *pST_ptr;
    iD4SYMBOL D4_near[V34_VA_D4SETNUM];
    CQWORD *pD2_near1, *pD2_near2;
    iD4SYMBOL d4in;
    VA_INFO *dec = &(DeCode->Vdec);    /* VA INFO structure */
    SDWORD *pD4_metric;
    SDWORD pNew_metric[V34_VA_STATENUM];
    SDWORD min_branch_metric;
    SDWORD branch_metric;
    SDWORD *pMetric;
    SWORD *pInfoPath;
    SWORD *pD2;
    UWORD m, m_mod;
    SBYTE hist_idx;
    UBYTE i;
    UBYTE j;
    UBYTE state_idx = 0, v34idx;
    UBYTE *pTrellis, *pPrevStateIdx, *pMinptr;
    UBYTE v0;

    pST_ptr = dec->pST_ptr;
    pD4_metric = dec->pD4_metric;

    pD2_near1 = dec->pD2_near1;
    pD2_near2 = dec->pD2_near2;

    /* branch_num = dec->number.branch; suppose that We always use 16 states */
    /*      d4set_num = dec->number.d4set; */

    /********************Begin Perform decoding**************** */

    /* 1. For each D2 symbol, find a nearest D2 symbol in each of the eight D2 subsets  */
    d4in.cS1 = dec->pcVA_sym[dec->VA_sym_outidx]; /* 1st 2D symbol in a 4D symbol*/   /*Viterbi Input*/

    dec->VA_sym_outidx ++;
    dec->VA_sym_outidx &= V34_VA_BUF_MASK;

    d4in.cS2 = dec->pcVA_sym[dec->VA_sym_outidx]; /* 2nd 2D symbol in a 4D symbol*/   /*Viterbi Input*/

    dec->VA_sym_outidx ++;
    dec->VA_sym_outidx &= V34_VA_BUF_MASK;

#if 0
    Flag_VA++;

    if (Flag_VA == 100)
    {
        d4in.cS1.r = QQMULQR8(d4in.cS1.r, 272);/* 362 */
        d4in.cS1.i = QQMULQR8(d4in.cS1.i, 272);

        d4in.cS2.r = QQMULQR8(d4in.cS2.r, 272);
        d4in.cS2.i = QQMULQR8(d4in.cS2.i, 272);
    }

    if (Flag_VA > 100)
    {
        Flag_VA = 101;
    }

#endif

#if 0  /* for dump data. */
    if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = d4in.cS1.r; }
    if (DumpTone2_Idx < 100000) { DumpTone2[DumpTone2_Idx++] = d4in.cS1.i; }
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = d4in.cS2.r; }
    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = d4in.cS2.i; }
#endif

    /* First 2D symbol */
    V34_State16_D2Min(&d4in.cS1, pD2_near1); /* output of pD2_near1 is 9.7 format */

    /* for the second 2D symbol */
    V34_State16_D2Min(&d4in.cS2, pD2_near2); /* output of pD2_near1 is 9.7 format */

    /* ------------------------------------------------------------------ */
    /* 2.Find the nearest D4 symbol to the input D4 symbol in each D4 subset */

    /* Output of D4_near is 9.7 format */
    /* For each 4D symbol, it associates with a minimum metric */
    /* which is stored in pD4_metric.                           */
    V34_State16_D4Min(&d4in, D4_near, (UBYTE *)dec);

#if 0
    for (i = 0; i < 16; i++)
    {
        if (DumpTone5_Idx < 1000000) { DumpTone5[DumpTone5_Idx++] = D4_near[i].cS1.r; }
        if (DumpTone6_Idx < 1000000) { DumpTone6[DumpTone6_Idx++] = D4_near[i].cS1.i; }
        if (DumpTone7_Idx < 1000000) { DumpTone7[DumpTone7_Idx++] = D4_near[i].cS2.r; }
        if (DumpTone8_Idx < 1000000) { DumpTone8[DumpTone8_Idx++] = D4_near[i].cS2.i; }
    }
#endif

    /* ---------------------------------------------------------------------- */
    /* Calculate v0 */
    v0 = 0;
    /* check_sync = 0; */
    m = (dec->current_rx_P << 2) + dec->D4_sym_idx;

    m_mod = m / (RxMod->rx_P << 1);

    if ((m - (m_mod *(RxMod->rx_P << 1))) == 0)
    {
        v0 = (dec->current_rx_inv_bit & 0x1) << 2;

        dec->current_rx_inv_bit >>= 1;

        /* check_sync = 1; */
    }

    /* Assigned pointer point to Trellis table */
    pTrellis = dec->Addr.pTrellis_ptr;

    /* 3. Calculate the survived metric according to the new 4D sysmbol */
    pInfoPath     = (SWORD *)(&(pST_ptr[0].pInfo_path[dec->hist_idx]));
    pPrevStateIdx = &(pST_ptr[0].pPrev_State_idx[dec->hist_idx]);

    for (i = 0; i < V34_VA_STATENUM ; i++) // 16
    {
        /* For all associated branches of current ending state, i, */
        /* find out the minimum accumulated distance.              */
        pMinptr = pTrellis;

        min_branch_metric = Q31_MAX;

        pMetric = &(pST_ptr[*pTrellis].metric);
        pTrellis++; /* Advance pointer to get label index of */

        for (j = 0; j < VITERBI_BRANCH_NUM; j++) // 4
        {
            // choose in 4 possible path (label) which gives the least accumulated distance
            /* Retrieve old accumulated branch metric */
            /* current starting state */
            v34idx = (*pTrellis) + v0; /* D4_metric has to be double buffered */
            branch_metric = *pMetric + pD4_metric[v34idx];

            pMetric += ((sizeof(VA_DecoderStr) << 2) / sizeof(SDWORD));

            if (branch_metric < min_branch_metric)
            {
                min_branch_metric = branch_metric;

                pMinptr = pTrellis;
            }

            pTrellis += 2;
        }

        pTrellis--; // back to "starting state"

        pNew_metric[i] = min_branch_metric;

        v34idx = ((*pMinptr) + v0) & 7;/* 7 = d4set_num - 1*/ // the path/label that has minimum accumulated distance

        pMinptr--; // the "starting point" that has minimum accumulated distance

        /* Cast to double word for faster copy, provided that */
        /* D4_near is aligned to 32bit boundary               */
        pD2 = (SWORD *)(&D4_near[v34idx]);
        pInfoPath[0] = pD2[0]; // S1.r
        pInfoPath[1] = pD2[1]; // S1.i
        pInfoPath[2] = pD2[2]; // S2.r
        pInfoPath[3] = pD2[3]; // S2.i

        pInfoPath += (sizeof(VA_DecoderStr) / sizeof(SWORD)); // next "current state"

        *pPrevStateIdx = *pMinptr; // store the "starting point" that has the minimum metric
        pPrevStateIdx += (sizeof(VA_DecoderStr) / sizeof(UBYTE)); // next "current state"
    }

    /* 4. Renew ST[i] where i=0,1,....15. */
    dec->count --;

    pMetric = &(pST_ptr[0].metric); // from "current state" 0

    if (dec->count <= 0)
    {
        /// LLL debug
        SDWORD minLast = Q31_MAX;

        dec->count = dec->win_size;

        for (i = 0; i < V34_VA_STATENUM ; i++)
        {
            /// LLL debug
            if (pNew_metric[i] < minLast)
            {
                minLast = pNew_metric[i];
                state_idx = i;
            }

            *pMetric = pNew_metric[i] - pNew_metric[0]; // normalization?
            pMetric += (sizeof(VA_DecoderStr) / sizeof(SDWORD));
        }
    }
    else
    {
        /// LLL debug
        SDWORD minLast = Q31_MAX;

        for (i = 0; i < V34_VA_STATENUM ; i++)
        {
            /// LLL debug
            if (pNew_metric[i] < minLast)
            {
                minLast = pNew_metric[i];
                state_idx = i;
            }

            *pMetric = pNew_metric[i]; // update the real store with the minimum metric
            pMetric += (sizeof(VA_DecoderStr) / sizeof(SDWORD));
        }
    }

    /* ________________________________________________________________ */
    /* 5. Decide the output 4D symbol */
    /* Trace back surviving path      */
    hist_idx = dec->hist_idx;
    /// LLL debug
    ///state_idx = 0;
    //printf("state_idx changes: %d ", state_idx);

    i = hist_idx;

    while (hist_idx > 0)
    {
        state_idx = pST_ptr[state_idx].pPrev_State_idx[hist_idx--]; // the minimum "starting state"
        /// LLL debug
        //printf("%d ", state_idx);
    }

    if (i < dec->win_size - 1)
    {
        i++;
        state_idx = pST_ptr[state_idx].pPrev_State_idx[hist_idx--];
        hist_idx  = dec->win_size - 1;
        j         = hist_idx;

        while (i < j)
        {
            /// LLL debug
            //printf("%d ", state_idx);
            state_idx = pST_ptr[state_idx].pPrev_State_idx[hist_idx--];
            i++;
        }
    }

    /// LLL debug
    //printf("\n");

    dec->hist_idx = hist_idx;

    (*pOut9_7).r = pST_ptr[state_idx].pInfo_path[hist_idx].cS1.r;
    (*pOut9_7).i = pST_ptr[state_idx].pInfo_path[hist_idx].cS1.i;

    pOut9_7++;

    (*pOut9_7).r = pST_ptr[state_idx].pInfo_path[hist_idx].cS2.r;
    (*pOut9_7).i = pST_ptr[state_idx].pInfo_path[hist_idx].cS2.i;

    /// LLL debug
    //printf("state_idx=%d, S1.r=%d, S1.i=%d, S2.r=%d, S2.i=%d\n", state_idx, pST_ptr[state_idx].pInfo_path[hist_idx].cS1.r,
    //pST_ptr[state_idx].pInfo_path[hist_idx].cS1.i, pST_ptr[state_idx].pInfo_path[hist_idx].cS2.r,
    //pST_ptr[state_idx].pInfo_path[hist_idx].cS2.i);
#if 0
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pST_ptr[state_idx].pInfo_path[hist_idx].cS1.r; }
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pST_ptr[state_idx].pInfo_path[hist_idx].cS1.i; }
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pST_ptr[state_idx].pInfo_path[hist_idx].cS2.r; }
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = pST_ptr[state_idx].pInfo_path[hist_idx].cS2.i; }
#endif

#if 0
    V34_Sync_Loss_Detector(dec, d4in.cS2, *pOut9_7, check_sync);
#endif

    dec->D4_sym_idx ++;

    if (dec->D4_sym_idx > 3)      /* Finish one mapping frame? */
    {
        dec->D4_sym_idx = 0;

        dec->current_rx_P ++;

        if (dec->current_rx_P >= RxMod->rx_P)
        {
            dec->current_rx_P = 0;

            /* Check to see if finish one superframe */
            /* Maybe have to synchronize the superframe????? */
            dec->current_rx_J ++;

            if (dec->current_rx_J >= RxMod->rx_Js)
            {
                dec->current_rx_J = 0;
                dec->current_rx_inv_bit = dec->rx_bit_inv_pat;
            }
        }
    }
}

#else

void  V34_VA_Decode(DeCodeStruc *DeCode, CQWORD *pOut9_7)
{
    VA_DecoderStr *pST_ptr;
    iD4SYMBOL D4_near[V34_VA_D4SETNUM];
    CQWORD *pD2_near1, *pD2_near2;
    iD4SYMBOL d4in;
    VA_INFO *dec = &(DeCode->Vdec);    /* VA INFO structure */

    SDWORD *pD4_metric;

    pST_ptr = dec->pST_ptr;
    pD4_metric = dec->pD4_metric;
    pD2_near1 = dec->pD2_near1;
    pD2_near2 = dec->pD2_near2;

    /* 1. For each D2 symbol, find a nearest D2 symbol in each of the */
    /*    eight D2 subsets  */
    d4in.cS1 = dec->pcVA_sym[dec->VA_sym_outidx]; /* 1st 2D symbol in a 4D symbol*/   /*Viterbi Input*/

    dec->VA_sym_outidx ++;
    dec->VA_sym_outidx &= V34_VA_BUF_MASK;

    d4in.cS2 = dec->pcVA_sym[dec->VA_sym_outidx]; /* 2nd 2D symbol in a 4D symbol*/   /*Viterbi Input*/

    dec->VA_sym_outidx ++;
    dec->VA_sym_outidx &= V34_VA_BUF_MASK;

#if 0
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = d4in.cS1.r; }
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = d4in.cS1.i; }
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = d4in.cS2.r; }
    if (DumpTone3_Idx < 100000) { DumpTone3[DumpTone3_Idx++] = d4in.cS2.i; }
#endif

    /* First 2D symbol */
    V34_State16_D2Min(&d4in.cS1, pD2_near1); /* output of pD2_near1 is 9.7 format */
    /* for the second 2D symbol */
    V34_State16_D2Min(&d4in.cS2, pD2_near2); /* output of pD2_near2 is 9.7 format */

    /* ------------------------------------------------------------------ */
    /* 2.Find the nearest D4 symbol to the input D4 symbol in each D4 subset */

    /* Output of D4_near is 9.7 format */
    /* For each 4D symbol, it associates with a minimum metric */
    /* which is stored in pD4_metric.                           */
    V34_State16_D4Min(&d4in, D4_near, (UBYTE *)dec);

    (*pOut9_7).r = D4_near[0].cS1.r;
    (*pOut9_7).i = D4_near[0].cS1.i;

    pOut9_7++;

    (*pOut9_7).r   = D4_near[0].cS2.r;
    (*pOut9_7).i   = D4_near[0].cS2.i;

#if 0
    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = D4_near[0].cS1.r; }
    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = D4_near[0].cS1.i; }
    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = D4_near[0].cS2.r; }
    if (DumpTone4_Idx < 100000) { DumpTone4[DumpTone4_Idx++] = D4_near[0].cS2.i; }
#endif
}
#endif

#endif

/***************************************************************************/
/* D4in and D4out are the second input and output 2d symbols of Viterbi    */
/***************************************************************************/

#if 0

void  V34_Init_Sync_Detector(VA_INFO *dec)
{
    UBYTE i;

    dec->sync_idx = 0;
    dec->sync_size = dec->win_size;
    dec->sync_Cg = 0;
    dec->sync_Cl = 0;
    dec->sync_TH = 13;

    for (i = 0; i < dec->sync_size; i++)
    {
        dec->pDec_syn_buf[i].r = 0;
        dec->pDec_syn_buf[i].i = 0;
    }
}


void V34_Sync_Loss_Detector(VA_INFO *V, CQWORD d4in9_7, CQWORD d4out9_7, UBYTE check_sync)
{
    CQWORD d2;
    SWORD d4metric;
    SBYTE p;

    d2 = V->pDec_syn_buf[V->sync_idx];

    V->pDec_syn_buf[V->sync_idx++] = d4in9_7;

    if (V->sync_idx >= V->sync_size)
    {
        V->sync_idx = 0;
    }

    d4metric = ((SWORD)d2.r - (SWORD)d4out9_7.r) * ((SWORD)d2.r - (SWORD)d4out9_7.r) +
               ((SWORD)d2.i - (SWORD)d4out9_7.i) * ((SWORD)d2.i - (SWORD)d4out9_7.i);

    p = (((UWORD)d4metric & 0x8000U) == 0) ? -1 : 1; /* check if d4metric >= 2.0 */

    V->sync_Cl += p;

    if (check_sync)
    {
        p = V->sync_Cl > 0 ? 1 : -1;

        V->sync_Cg += p;

        if (V->sync_Cg < 0)
        {
            V->sync_Cg = 0;
        }

        if (V->sync_Cg > V->sync_TH)
        {
            V->sync_Cg = V->sync_TH;
        }

        V->sync_Cl = 0;
    }
}

#endif


/***** Initilization for Viterbi Algorithm (VA) decoding****************/
/*      OUTPUT:                                                        */
/*              VA_INFO *dec    - Initialize every element in the      */
/*                                structure (*dec)                     */
/*      Notes:                                                         */
/*      1. This file should be created prior to the use of the viterbi */
/*         decoder;                                                    */
/***********************************************************************/
void  V34_VA_Init(VA_INFO *dec)
{
    UWORD i, j;
    iD4SYMBOL zero;
    VA_DecoderStr *pST_ptr;

    zero.cS1.r = 0;
    zero.cS1.i = 0;

    zero.cS2.r = 0;
    zero.cS2.i = 0;

    /* ------------------Initilization for the DECODER-------------------- */
    /* Init for 16 state Viterbi decoder only */
    for (i = 0; i < V34_VA_STATENUM; i++)
    {
        pST_ptr = &(dec->pST_ptr[i]);
        pST_ptr->metric = 0xFFFFF;

        for (j = 0; j < V34_VA_WINTHRESHOLD; j++)
        {
            pST_ptr->pInfo_path[j] = zero;

            pST_ptr->pPrev_State_idx[j] = 0;
        }
    }

    dec->pST_ptr[0].metric = 0; /* only state 0 is the starting state */
}
