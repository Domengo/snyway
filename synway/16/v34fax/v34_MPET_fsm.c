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

#include "v34fext.h"

#if SUPPORT_V34FAX

CONST V34FaxFnptr V34Fax_Call_Tx_Init_VecTab[] =
{
    V34Fax_Silence_Tx_Setup,
    V34Fax_PPh_Tx_Setup,
    V34Fax_ALT_Tx_Setup,
    V34Fax_MPh_Tx_Setup,
    V34Fax_E_Tx_Setup,
    V34Fax_T30_Tx_SetUp,
    V34Fax_MPET_fsm_StateProtect
};

CONST QWORD V34Fax_Call_Tx_Init_CntTab[] =
{
    42,      /* Silence 70ms */
    32,      /* PPh for 32T */
    1800,    /* Max ALT time to wait for PPh */
    2400,    /* Max MPh time to wait for MPh */
    10,      /* E is a 20bit sequence, 10T */
    2400,    /* HDLC frame Tx */
    24000
};


CONST V34FaxFnptr V34Fax_Call_Rx_Init_VecTab[] =
{
    V34Fax_PPh_Rx_SetUp,
    V34Fax_PPh_Train_SetUp,
    V34Fax_ALT_Rx_SetUp,
    V34Fax_MPh_Rx_SetUp,
    V34Fax_E_Rx_SetUp,
    V34Fax_T30_Rx_SetUp,
    V34Fax_Silence_Rx_SetUp,
    V34Fax_MPET_fsm_StateProtect
};


CONST QWORD V34Fax_Call_Rx_Init_CntTab[] =
{
    2400,     /* PPh detect */
    32,       /* Forced Train for 32T during PPh signal */
    1800,     /* 3sec timeout */
    1800,     /* 3sec timeout */
    1800,     /* 3sec timeout */
    2400,     /* HDLC frame Rx */
    24000,    /* Silence Rx */
    24000
};


CONST V34FaxFnptr V34Fax_Call_Tx_NoFirstBlock_Init_VecTab[] =
{
    V34Fax_Silence_Tx_Setup,
    V34Fax_Sh_Tx_Setup,
    V34Fax_ShB_Tx_Setup,
    V34Fax_ALT_Tx_Setup,
    V34Fax_E_Tx_Setup,
    V34Fax_T30_Tx_SetUp,
    V34Fax_Dummy,
    V34Fax_MPET_fsm_StateProtect
};


CONST QWORD V34Fax_Call_Tx_NoFirstBlock_Init_CntTab[] =
{
    42,     /* Silence 70ms */
    24,     /* Sh for 24T */
    8,      /* Sh bar for 8T */
    115,     /* ALT >= 16T */
    10,     /* E is a 20bit sequence, 10T */
    24000,  /* EOP & Flags & DCN & "1" */
    24000,  /* wait for next page only */
    24000
};


CONST V34FaxFnptr V34Fax_Call_Rx_NoFirstBlock_Init_VecTab[] =
{
    V34Fax_PPh_Sh_Rx_SetUp,
    V34Fax_ShB_Rx_SetUp,
    V34Fax_ALT_Rx_SetUp,
    V34Fax_E_Rx_SetUp,
    V34Fax_T30_Rx_SetUp,
    V34Fax_Silence_Rx_SetUp,
    V34Fax_MPET_fsm_StateProtect
};


CONST QWORD V34Fax_Call_Rx_NoFirstBlock_Init_CntTab[] =
{
    2400,     /* Sh detect */
    32,       /* Forced Train for 32T during ShB signal */
    120,      /* 200ms timeout */
    1800,     /* 3s timeout */
    2400,     /* Flags & MCF & Flags */
    2400,     /* Silence Rx */
    24000
};


/******************************************************/
/* FSM for Call when rerate initiated by remote modem */
/* This is setup when PPh is detected */
CONST V34FaxFnptr V34Fax_Call_Tx_Rerate_by_Remote_Init_VecTab[] =
{
    V34Fax_Silence_Tx_Setup,
    V34Fax_Sh_Tx_Setup,
    V34Fax_ShB_Tx_Setup,
    V34Fax_ALT_Tx_Setup,
    V34Fax_PPh_Tx_Setup,
    V34Fax_ALT_Tx_Setup,
    V34Fax_MPh_Tx_Setup,
    V34Fax_E_Tx_Setup,
    V34Fax_T30_Tx_SetUp,
    V34Fax_MPET_fsm_StateProtect
};


CONST QWORD V34Fax_Call_Tx_Rerate_by_Remote_Init_CntTab[] =
{
    42,      /* Silence 70ms */
    24,      /* Sh for 24T */
    8,       /* Sh bar for 8T */
    2400,    /* Wait for Receive end PPh */
    32,      /* PPh for 32T */
    115,     /* ALT > 16T, ALT < 120T  */
    2400,    /* Max MPh time to wait for MPh */
    10,      /* E is a 20bit sequence, 10T */
    2400,    /* HDLC frame Tx */
    24000
};
/*********************************************/


CONST V34FaxFnptr V34Fax_Ans_Tx_Init_VecTab[] =
{
    V34Fax_Silence_Tx_Setup,
    V34Fax_PPh_Tx_Setup,
    V34Fax_ALT_Tx_Setup,
    V34Fax_MPh_Tx_Setup,
    V34Fax_E_Tx_Setup,
    V34Fax_T30_Tx_SetUp,
    V34Fax_Silence_Tx_Setup,
    V34Fax_MPET_fsm_StateProtect
};


CONST QWORD V34Fax_Ans_Tx_Init_CntTab[] =
{
    18000,   /* wait for Rx signal */
    32,      /* PPh for 32T */
    1800,    /* Max ALT time to wait for PPh */
    1800,    /* Max MPh time to wait for MPh */
    10,      /* E is a 20bit sequence, 10T */
    100,     /* HDLC frame Tx */
    18000,   /* wait for next page only */
    24000
};


CONST V34FaxFnptr V34Fax_MPET_Retrn_by_Local_Init_VecTab[] =
{
    V34Fax_MPET_AC_Tx_Setup,
    V34Fax_PPh_Tx_Setup,
    V34Fax_ALT_Tx_Setup,
    V34Fax_MPh_Tx_Setup,
    V34Fax_E_Tx_Setup,
    V34Fax_T30_Tx_SetUp,
    V34Fax_Silence_Tx_Setup,
    V34Fax_MPET_fsm_StateProtect
};


CONST QWORD V34Fax_MPET_Retrn_by_Local_Init_CntTab[] =
{
    18000,   /* wait for Rx signal */
    32,      /* PPh for 32T */
    120,     /* Max ALT time to wait for PPh */
    1800,    /* Max MPh time to wait for first MPh */
    10,      /* E is a 20bit sequence, 10T */
    100,     /* HDLC frame Tx */
    18000,   /* wait for next page only */
    24000
};


CONST V34FaxFnptr V34Fax_Ans_Rx_Init_VecTab[] =
{
    V34Fax_PPh_Rx_SetUp,
    V34Fax_PPh_Train_SetUp,
    V34Fax_ALT_Rx_SetUp,
    V34Fax_MPh_Rx_SetUp,
    V34Fax_E_Rx_SetUp,
    V34Fax_T30_Rx_SetUp,
    V34Fax_ONEs_Rx_SetUp,
    V34Fax_Silence_Rx_SetUp,
    V34Fax_Rx_SetUp,
    V34Fax_MPET_fsm_StateProtect
};


CONST QWORD V34Fax_Ans_Rx_Init_CntTab[] =
{
    18000,     /* PPh detect */
    32,       /* Forced Train for 32 T during PPh signal */
    1800,     /* ALT detect */
    1800,     /* MPh detect */
    1800,     /* E detect */
    100,      /* HDLC frame Rx */
    1800,     /* ONEs detect */
    18000,    /* Silence Rx */
    18000,    /* wait only */
    24000
};


CONST V34FaxFnptr V34Fax_Ans_Tx_NoFirstBlock_Init_VecTab[] =
{
    V34Fax_Silence_Tx_Setup,
    V34Fax_Sh_Tx_Setup,
    V34Fax_ShB_Tx_Setup,
    V34Fax_ALT_Tx_Setup,
    V34Fax_E_Tx_Setup,
    V34Fax_T30_Tx_SetUp,
    V34Fax_Silence_Tx_Setup,
    V34Fax_Rx_SetUp,
    V34Fax_MPET_fsm_StateProtect
};


CONST QWORD V34Fax_Ans_Tx_NoFirstBlock_Init_CntTab[] =
{
    2400,   /* Silence Tx */
    24,     /* 24,Sh = 24T */
    8,      /* Sh bar = 8T */
    110,     /* ALT >= 16T < 120T *//* changed from 96. */
    10,     /* E is a 20bit sequence, = 10T */
    2400,   /* Flags & MCF & Flags */
    2400,   /* Silence Tx forever */
    2400,   /* wait only */
    24000
};

CONST V34FaxFnptr V34Fax_Ans_Rx_NoFirstBlock_Init_VecTab[] =
{
    V34Fax_PPh_Sh_Rx_SetUp,
    V34Fax_ShB_Rx_SetUp,
    V34Fax_ALT_Rx_SetUp,
    V34Fax_E_Rx_SetUp,
    V34Fax_T30_Rx_SetUp,
    V34Fax_ONEs_Rx_SetUp,
    V34Fax_Silence_Rx_SetUp,
    V34Fax_Rx_SetUp,
    V34Fax_MPET_fsm_StateProtect
};

CONST QWORD V34Fax_Ans_Rx_NoFirstBlock_Init_CntTab[] =
{
    1800,     /* Sh detect */
    32,       /* Forced Train for 32 T during ShB signal */
    1800,     /* ALT detect */
    1800,     /* E detect */
    100,      /* HDLC frame Rx */
    1800,     /* ONEs detect  */
    1800,     /* silence detect */
    1800,     /* wait only */
    24000
};

/********************************************************/
/* FSM for Answer when rerate initiated by local modem */
/* This is setup on init  */

CONST V34FaxFnptr V34Fax_Ans_Rx_Rerate_by_Local_Init_VecTab[] =
{
    V34Fax_PPh_Sh_Rx_SetUp,
    V34Fax_ShB_Rx_SetUp,
    V34Fax_ALT_Rx_SetUp,
    V34Fax_MPh_Rx_SetUp,
    V34Fax_E_Rx_SetUp,
    V34Fax_T30_Rx_SetUp,
    V34Fax_ONEs_Rx_SetUp,
    V34Fax_Silence_Rx_SetUp,
    V34Fax_Rx_SetUp,
    V34Fax_MPET_fsm_StateProtect
};

CONST QWORD V34Fax_Ans_Rx_Rerate_by_Local_Init_CntTab[] =
{
    1800,     /* Sh detect */
    32,       /* Forced Train for 32 T during ShB signal */
    1800,     /* ALT detect */
    1800,     /* MPh detect */
    1800,     /* E detect */
    100,      /* HDLC frame Rx */
    1800,     /* ONEs detect  */
    1800,     /* silence detect */
    1800,     /* wait only */
    24000
};

/*******************************************/

void V34Fax_MPET_fsm_StateProtect(V34FaxStruct *pV34Fax)
{
    pV34Fax->timeout_count = V34FAX_MPET_TIMEOUT;

    TRACE0("Crash: MPET fsm State over!");
}

#endif
