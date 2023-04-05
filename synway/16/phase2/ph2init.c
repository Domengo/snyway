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

#include <string.h>
#include "ph2ext.h"
#include "mhsp.h"   /* Modem Host Signal Processor */
#include "ptable.h" /* Data memory table defines   */
#include "sregdef.h"

void Ph2_Init(UBYTE **pTable)
{
    MhspStruct *pMhsp = (MhspStruct *)(pTable[MHSP_STRUC_IDX]);
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    Ph2Struct  *pPH2  = (Ph2Struct *)(pTable[PH2_STRUC_IDX]);
#if SUPPORT_V90
    UBYTE      *pSreg = (UBYTE *)pTable[ACESREGDATA_IDX];
#endif
    Info_0Struc *pInfo;
    UBYTE ubRate;
    UBYTE Host_Max_SymRate;

    pPH2->PCMinPtr  = (SWORD *)pTable[PCMINDATA_IDX];
    pPH2->PCMoutPtr = (SWORD *)pTable[PCMOUTDATA_IDX];

    PutReg(pDpcs->MSR0, TRAIN); /* Report Status as TRAINING */

#if SUPPORT_V90
    pPH2->ubDownstreamPowerIdx = pSreg[DOWNSTREAM_POWER_INDEX];

    if (pPH2->V90_Enable)
    {
#if SUPPORT_V90A
#if SUPPORT_V90D

        if (ModemAorB == 0)
#endif
        {
            pPH2->modem_mode = ANS_MODEM;
        }

#endif /* analogue */

#if SUPPORT_V90D
#if SUPPORT_V90A

        if (ModemAorB == 1)
#endif
        {
            pPH2->modem_mode = CALL_MODEM;
        }

#endif /* digital */
    }

#endif /* SUPPORT_V90 */

    if (pPH2->modem_mode == CALL_MODEM)
    {
        pInfo = &(pPH2->I0c);
    }
    else
    {
        pInfo = &(pPH2->I0a);
    }

#if V34_OPTIONAL
    pInfo->S2743 = 1;
    pInfo->S2800 = 1;
    pInfo->S3429 = 1;//0
#else
    pInfo->S2743 = 0;
    pInfo->S2800 = 0;
    pInfo->S3429 = 0;
#endif

    pInfo->S3000_low_carrier  = 1;
    pInfo->S3000_high_carrier = 1;
    pInfo->S3200_low_carrier  = 1;
    pInfo->S3200_high_carrier = 1;
    pInfo->S3429_enable       = 1;

    pInfo->reduce_transmit_power      = 0;
    pInfo->max_allowed_diff              = 0;
    pInfo->transmit_from_CME          = 0;
    pInfo->V34bis                      = 1;
    pInfo->transmit_clock_source      = 0;
    pInfo->acknowledge_correct_recept = 0;

#if SUPPORT_V90D
#if SUPPORT_V90A

    if (pPH2->modem_mode == CALL_MODEM)
#endif
    {
        if (pPH2->V90_Enable)
        {
            pInfo->S2743                    = 0;
            pInfo->S2800                    = 0;
            pInfo->S3429                    = 0;
            pInfo->S3000_low_carrier        = 0;
            pInfo->S3000_high_carrier        = 0;
            pInfo->dig_nom_transmit_power    = 6;
            pInfo->dig_max_transmit_power    = 23;
            pInfo->power_measure            = 1;
            pInfo->dig_S3429_enable            = 0;
#if SUPPORT_V92D
            pInfo->transmit_clock_source    = 2;            /* V.92 Capability = 1; Short phase = 0 */
#endif
        }
    }

#endif

#if SUPPORT_V92A
#if SUPPORT_V90D

    if (pPH2->modem_mode == ANS_MODEM)
#endif
    {
        if (pPH2->V90_Enable)
        {
            pInfo->transmit_clock_source = 3;

            if ((pPH2->drift == 0) || (pSreg[V92_OPTIONS] & QUICK_CONNECT_DISABLE))
            {
                pInfo->transmit_clock_source &= 0x1;    /* Short Phase 2 Disabled */
            }

            if (pSreg[V92_OPTIONS] & V92_DISABLE)
            {
                pInfo->transmit_clock_source &= 0x2;    /* V92 Disabled */
            }
        }
    }

#endif

#if SUPPORT_V34FAX

    if (pPH2->V34fax_Enable == 1)  /* For same settings as multi-tech */
    {
        pInfo->S3000_low_carrier            = 0;
        pInfo->S3200_low_carrier            = 0;
        pInfo->reduce_transmit_power        = 1;
    }

#endif

#if SUPPORT_V90D

    if (pSreg[V90D_PCM_CODING] == MU_LAW)
    {
        pInfo->dig_A_law = 0;
    }
    else
    {
        pInfo->dig_A_law = 1;
    }

#endif

    Host_Max_SymRate = RdReg(pDpcs->MBR, MAX_BAUD_RATE);

    if (pInfo->S3429_enable == 0 && Host_Max_SymRate == V34_SYM_3429)
    {
        pPH2->max_symbol_rate = V34_SYM_3200;
    }
    else
    {
        pPH2->max_symbol_rate = Host_Max_SymRate;
    }

    pPH2->min_symbol_rate = RdReg(pDpcs->MBR, MIN_BAUD_RATE) >> 3;

    ubRate = RdReg(pDpcs->MBC1, MAX_RATE);

    if ((ubRate == 4)
#if (SUPPORT_V90A + SUPPORT_V90D)
        && (pPH2->V90_Enable == 0)
#endif
       )/* if max rate = 2400, limit symbol rate =0 (2400 sym/s) */
    {
        pPH2->max_symbol_rate = 0;
        pPH2->min_symbol_rate = 0;

        pInfo->S2743 = 0;
        pInfo->S2800 = 0;
        pInfo->S3429 = 0;

        pInfo->S3000_low_carrier  = 0;
        pInfo->S3000_high_carrier = 0;
        pInfo->S3200_low_carrier  = 0;
        pInfo->S3200_high_carrier = 0;
        pInfo->S3429_enable       = 0;
    }

    pPH2->Ph2Timer = V34PHASE2_TIMEOUT;

#if SUPPORT_V90

    if (pPH2->V90_Enable)
    {
        pInfo->transmit_from_CME = 1;

#if SUPPORT_V90D

        if (pPH2->modem_mode == CALL_MODEM)
        {
            Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)pInfo, ShiftTable0d);
        }
        else
#endif
            Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)pInfo, ShiftTable0);
    }
    else
#endif
        Ph2_CreateInfo(pPH2->pInfo_buf, (UBYTE *)pInfo, ShiftTable0);

    pPH2->info_idx = 0;
    pPH2->LineProbOn = 0;

    Ph2_StateInit(pPH2);
    Ph2_Prob_Init(&(pPH2->Probing));
    Ph2_RxProbing_Init(&(pPH2->Probing));

    Ph2_DPSK_Init(&(pPH2->Dpsk), pPH2->modem_mode);
    Ph2_Detect_Info_0_Init(&(pPH2->I0c));
    Ph2_Detect_Info_0_Init(&(pPH2->I0a));
    Ph2_Detect_Info_0_Init(&(pPH2->I0check));
    Ph2_Detect_Info_1c_Init(&(pPH2->I1c));
    Ph2_Detect_Info_1a_Init(&(pPH2->I1a));

#if SUPPORT_V34FAX

    if (pPH2->V34fax_Enable == 1)
    {
        Ph2_Detect_Info_h_Init(&(pPH2->Ih));
    }

#endif

    pMhsp->pfRealTime = Ph2_Handler;
    Ph2_HW_Setup(pTable);

    /* This is for retrain only. Optionally using DPCS register to
    check for retrain is also possible.
    */
#if SUPPORT_V90

    if (pPH2->Ph2State.Retrain_Times > 0 || pPH2->Retrain_V90 > 0)
#else
    if (pPH2->Ph2State.Retrain_Times > 0)
#endif
    {
        if (pPH2->Ph2State.Retrain_Times > 2)
        {
            pPH2->max_symbol_rate = pPH2->min_symbol_rate;
        }

        if (pPH2->modem_mode == CALL_MODEM)
        {
#if SUPPORT_V34FAX

            if (pPH2->V34fax_Enable == 1)
            {
                pPH2->tx_vec_idx = 9;    /* rtrn init */
            }
            else
#endif
                pPH2->tx_vec_idx = 16;
        }
        else
        {
#if SUPPORT_V34FAX

            if (pPH2->V34fax_Enable == 1)
            {
                pPH2->tx_vec_idx = 10;    /* rtrn init */
            }
            else
#endif
                pPH2->tx_vec_idx = 18;
        }

        pPH2->rx_vec_idx = 1;
    }

#if SUPPORT_V92A_MOH

    if (pPH2->mH_TxByte != 0) /* This byte is set by V92 modem for cleardown or On-Hold */
    {
        Ph2_Init_MH_Request(pPH2);
    }

#endif
}
