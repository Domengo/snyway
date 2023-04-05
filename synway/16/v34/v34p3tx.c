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

/****************************************************************************/
/* Phase3 Tx vectors                                                        */
/****************************************************************************/

#include "v34ext.h"

#if 0 /// LLL temp
void V34_Phase3_Send_Silence_70MS(V34Struct *pV34)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    Phase3_Info *p3     = &(pV34->p3);
    UBYTE i;

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        pV34Tx->PCMoutPtr[i] = 0;
    }

    p3->symbol_counter ++;

    if (p3->symbol_counter >= 224)  /* 70ms in 9.6kHz has 224 symbols */
    {
        (p3->tx_vec_idx) ++;
        p3->symbol_counter = 0;
    }
}

void V34_Phase3_Send_Silence(V34Struct *pV34)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    UBYTE i;

    for (i = 0; i < V34_SYM_SIZE; i++)
    {
        pV34Tx->PCMoutPtr[i] = 0;
    }
}

void V34_Phase3_Mod_Silence(V34Struct *pV34)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    EchoStruc   *pEc    = &(pV34->Echo);
    CQWORD cqOut_sym;

    cqOut_sym.r = 0;
    cqOut_sym.i = 0;
    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOut_sym, (CQWORD *)&pEc->pcBulk_delay);
}

void V34_Phase3_Send_S(V34Struct *pV34)
{
    V34TxStruct *pV34Tx = &(pV34->V34Tx);
    Phase3_Info *p3     = &(pV34->p3);
    EchoStruc   *pEc    = &(pV34->Echo);
    CQWORD cqOut_sym;

    p3->symbol_counter ++;

    if (p3->symbol_counter >= 128)
    {
        (p3->tx_vec_idx) ++;      /* Send S_BAR */
        p3->symbol_counter = 0;
    }

    cqOut_sym = V34_Create_S_or_SBar(&(p3->S_point));
    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);
}


void V34_Phase3_Send_S_Bar(V34Struct *pV34)
{
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    Phase3_Info *p3 = &(pV34->p3);
    EchoStruc  *pEc = &(pV34->Echo);
    CQWORD cqOut_sym;

    cqOut_sym = V34_Create_S_or_SBar(&(p3->S_point_BAR));
    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);

    ++p3->symbol_counter;

    if (p3->symbol_counter >= 16)
    {
        (p3->tx_vec_idx) ++;      /* goto send PP */
        (p3->ec_vec_idx) ++;  /* go from dummy to echo cancellor */
        pEc->ec_adapt = 1;

        if (pV34Rx->modem_mode == CALL_MODEM)
        {
            pEc->qNe_beta = 0;
            pEc->qFe_beta = 0;
        }

#if USE_ANS
        else if (pV34Rx->modem_mode == ANS_MODEM)
        {
            pEc->qNe_beta = q008;
            pEc->qFe_beta = 0;
        }

#endif

        p3->symbol_counter = 0;
        p3->I = 0;               /* Init for PP sequence */
        p3->K = 0;               /* Init for PP sequence */
    }
}

void V34_Phase3_PP_Common(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc             *pEc = &(pV34->Echo);
    Phase3_Info            *p3 = &(pV34->p3);
    CQWORD cqOut_sym;

    if (p3->I > 3)
    {
        p3->I = 0;
        p3->K ++;
    }

    cqOut_sym = V34_Create_PP(p3->K, p3->I);
    (p3->I)++;

    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOut_sym, (CQWORD *)&pEc->pcBulk_delay);

    (p3->symbol_counter)++ ;

    if (p3->symbol_counter >= 288)
    {
        (p3->tx_vec_idx)++;              /* Send TRN sequence */

        pV34Rx->sc = 0;    /* Initial scrambler */
        pShellMap->scram_idx = 16;           /* Prime # scrambled bits to 16 */
        p3->pByte_stream[0] = 0xFF;
        p3->pByte_stream[1] = 0xFF;

        switch (pV34Rx->tx_symbol_rate)
        {
            case V34_SYM_2400:p3->EC_Train_Counter=pV34->fecount+4000; break;
            case V34_SYM_2743:p3->EC_Train_Counter=pV34->fecount+4571; break;
            case V34_SYM_2800:p3->EC_Train_Counter=pV34->fecount+4666; break;
            case V34_SYM_3000:p3->EC_Train_Counter=pV34->fecount+5625; break;
            case V34_SYM_3200:p3->EC_Train_Counter=pV34->fecount+6000; break;
            case V34_SYM_3429:p3->EC_Train_Counter=pV34->fecount+6430; break;
        }
    }
}

void V34_Phase3_Send_PP_Call(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);

    V34_Phase3_PP_Common(pV34);

    if (p3->symbol_counter == ((pV34->fecount << 1) + 200))
    {
        pV34->Echo.qNe_beta = q009;
        pV34->Echo.qFe_beta = q009;
    }
}


void V34_Phase3_Send_PP(V34Struct *pV34)
{
    Phase3_Info *p3 = &(pV34->p3);

    V34_Phase3_PP_Common(pV34);

    if (p3->symbol_counter == ((pV34->fecount << 1) + 200))
    {
        pV34->Echo.qNe_beta = q009;
        pV34->Echo.qFe_beta = q008;
    }
}

void V34_Phase3_Send_TRN(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    ReceiveStruc *pRx = &pV34Rx->RecStruc;
    V34AgcStruc *pAGC = &(pRx->rx_AGC);
    EchoStruc            *pEc = &(pV34->Echo);
    Phase3_Info           *p3 = &(pV34->p3);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    CQWORD cqOut_sym;
    //UBYTE i;

    if (pShellMap->scram_idx >= 16)
    {
        (*pV34Rx->pfScram_byte)(p3->pByte_stream, &(pV34Rx->sc), pShellMap->pScram_buf, 2);
        pShellMap->scram_idx = 0;
    }

    cqOut_sym = V34_Create_TRN(pShellMap->pScram_buf, &(pShellMap->scram_idx), V34_4_POINT_TRN);
    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOut_sym, (CQWORD *)&pEc->pcBulk_delay);

    ++p3->symbol_counter;

    if ((p3->symbol_counter >= p3->EC_Train_Counter) && (pShellMap->scram_idx >= 16))
    {
        pV34Tx->z = pShellMap->pScram_buf[pShellMap->scram_idx-2] + (pShellMap->pScram_buf[pShellMap->scram_idx-1] << 1);

        (p3->tx_vec_idx) ++;      /* Goto send J */
        (p3->rx_vec_idx) ++;      /* Change Rx state from dummy to Det_S_CALL */

        /* init time counter for detect s-to-s bar transition, 10:05 AM 8/29/96 */
        if (pV34Rx->modem_mode == ANS_MODEM)
        {
            p3->Time_cnt_SSBar_start = pV34->Time_cnt;
        }
        else
        {
            pV34->p4.Time_cnt_SSBar_start = pV34->Time_cnt;
            pAGC->freeze = pAGC->qGain; /* Save gain temporary */
            pAGC->qGain  = pAGC->AGC_gain;     /* Use initial gain for 2nd S detection */
        }

        p3->symbol_counter = 0;
        pShellMap->scram_idx = 16;

        pEc->ec_adapt = 0;

        pEc->qNe_beta = 6; /* 0.00005 */
        pEc->qFe_beta = 6; /* 0.00005 */

        p3->pByte_stream[0] = (pV34Rx->J_point) & 0xFF;
        p3->pByte_stream[1] = (pV34Rx->J_point >> 8) & 0xFF;

#if 0 /* Dump echo coef for adjusting System delay */

        for (i = 0; i < 96; i++)
            if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV34->Echo.pcNe_coef[i].r; }

        for (i = 0; i < 96; i++)
            if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV34->Echo.pcNe_coef[i].i; }

        for (i = 0; i < 96; i++)
            if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV34->Echo.pcFe_coef[i].r; }

        for (i = 0; i < 96; i++)
            if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pV34->Echo.pcFe_coef[i].i; }

#endif
    }
    else if (p3->symbol_counter == ((pV34->fecount << 1) + 200))
    {
        pEc->qNe_beta = 2000;/* q009; */
        pEc->qFe_beta = 1500;/* q009; */
    }
    else if (p3->symbol_counter == (pV34->fecount + 3000))
    {
        pEc->qNe_beta = 1000;/* 0.01 */
        pEc->qFe_beta = 1000;/* 0.01 */
    }
    else if (p3->symbol_counter == (pV34->fecount + 4500))
    {
        pEc->qNe_beta = 400;
        pEc->qFe_beta = 400;
    }
}

/* Generating J sequence */
void V34_Phase3_Send_J(V34Struct *pV34)
{
    V34RxStruct *pV34Rx = &(pV34->V34Rx);
    V34TxStruct       *pV34Tx = &(pV34->V34Tx);
    ShellMapStruct *pShellMap = &(pV34Tx->ShellMap);
    EchoStruc            *pEc = &(pV34->Echo);
    Phase3_Info           *p3 = &(pV34->p3);
    CQWORD cqOut_sym;

    if (pShellMap->scram_idx >= 16)
    {
        (*pV34Rx->pfScram_byte)(p3->pByte_stream, &(pV34Rx->sc), pShellMap->pScram_buf, 2);
        pShellMap->scram_idx = 0;
    }

    cqOut_sym = V34_Create_J(pV34);
    V34_Modulate(pV34Tx, &cqOut_sym, &pEc->Ec_Insert);
    V34_EcRef_Insert(&pEc->Ec_Insert, cqOut_sym, (CQWORD *)&pEc->pcBulk_delay);
}
#endif
