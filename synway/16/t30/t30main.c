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
#include "t30ext.h"
#include "v251.h"

#if SUPPORT_T30
void T30_PhaseA(T30Struct *pT30)
{
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
#if SUPPORT_V34FAX
    UBYTE isV34Enabled = pDteToT30->isV34Enabled;
#if !SUPPORT_PARSER
    UBYTE isToConfigV34 = FALSE;
#endif

    if (pDceToT30->isDleEot)
    {
        T30_GotoPointB(pT30, HANGUP_RING_WITHOUT_HANDSHAKE); return;
    }

#endif

    switch (pDteToT30->actionCmd)
    {
        case DTE_T30_ATD:
            pT30->isCall = TRUE;
            pT30->isTransmit = pDteToT30->isTransmit;
            T30_SetCommand(pT30ToDce, T30_DCE_ATD);
            pT30ToDce->tempParmLen = (UBYTE)pDteToT30->tempDataLen;

            if (pDteToT30->tempData[0] == 1)//pulse dial
            {
            }
            else if (pDteToT30->tempData[0] == 2)//tone dial
            {
            }

            memcpy(&(pT30ToDce->tempParm[0]), &(pDteToT30->tempData[1]), pT30ToDce->tempParmLen);
            pDteToT30->tempDataLen = 0;
#if !SUPPORT_PARSER && SUPPORT_V34FAX
            isToConfigV34 = TRUE;
#endif
            break;
        case DTE_T30_ATA:
            pT30->isCall = pT30->isTransmit = FALSE;
            T30_SetCommand(pT30ToDce, T30_DCE_ATA);
#if !SUPPORT_PARSER && SUPPORT_V34FAX
            isToConfigV34 = TRUE;
#endif
            break;
    }

    pDteToT30->actionCmd = DTE_T30_NULL;

#if SUPPORT_V34FAX

    if (isV34Enabled)
    {
#if !SUPPORT_PARSER

        if (isToConfigV34)
        {
            T30_UpdateT31Parms(pT30);
        }

#endif

        switch (pDceToT30->v8IndicationType)
        {
            case V8_A8A:
            case V8_A8J:
            case V8_A8C:
            case V8_A8I:
                break;
            case V8_A8M:
                memcpy(pT30->remoteV8Menu, pDceToT30->v8IndicationData, sizeof(pT30->remoteV8Menu));
                break;
        }

        pDceToT30->v8IndicationType = V8_NULL;
        pDceToT30->v8IndicationLen = 0;

#if !DCE_CONTROLLED_V8

        if (pDceToT30->resultCodeRsp == RESULT_CODE_OK)
        {
            DpcsStruct *pDpcs = (DpcsStruct *)pTable[DPCS_STRUC_IDX];

            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;

            if (!RdReg(pDpcs->MCR0, ANSWER))
            {
                if (pT30->isMenuSent)
                {
                    T30_SetCommand(pT30ToDce, T30_DCE_ATO);
                    pT30->isMenuSent = FALSE;
                }
                else
                {
                    UBYTE localV8Menu[4] = {0x81, 0x85, 0xd4, 0x90};

                    if (!pT30->isTransmit)
                    {
                        // initial polling
                        localV8Menu[0] = 0xa1;
                    }

                    memcpy(pT30ToDce->tempParm, localV8Menu, 4);
                    pT30ToDce->tempParmLen = 4;
                    T30_SetCommand(pT30ToDce, T30_DCE_A8M);
                    {
                        int i;

                        for (i = 0; i < 4; i ++)
                        {
                            TRACE("%02x", pT30ToDce->tempParm[i]);
                        }
                    }
                    TRACE0("");
                    pT30->isMenuSent = TRUE;
                }
            }
            else
            {
                if (pT30->isMenuSent)
                {
                    T30_SetCommand(pT30ToDce, T30_DCE_ATO);
                    pT30->isMenuSent = FALSE;
                }
                else
                {
                    memcpy(pT30ToDce->tempParm, pT30->remoteV8Menu, sizeof(pT30->remoteV8Menu));

                    if (pT30->remoteV8Menu[0] == 0xa1 && pDteToT30->isDocForPolling)
                    {
                        // CM with RX, initial polling
                        pT30->isTransmit = TRUE;
                    }

                    pT30ToDce->tempParmLen = sizeof(pT30->remoteV8Menu);
                    T30_SetCommand(pT30ToDce, T30_DCE_A8M);
                    {
                        int i;

                        for (i = 0; i < 4; i ++)
                        {
                            TRACE("%02x", pT30ToDce->tempParm[i]);
                        }
                    }
                    TRACE0("");
                    pT30->isMenuSent = TRUE;
                }
            }
        }

#endif
    }

#endif

    switch (pDceToT30->infoTxtRsp)
    {
        case INFO_TXT_NULL:
        case INFO_TXT_F34: break;
        default: TRACE1("T30: Invalid Info Txt Response %d", pDceToT30->infoTxtRsp); break;
    }

    pDceToT30->infoTxtRsp = INFO_TXT_NULL;

    switch (pDceToT30->resultCodeRsp)
    {
        case RESULT_CODE_CONNECT:
            pT30ToDte->infoTxtRsp = INFO_TXT_FCO; TRACE0("T30: +FCO");

            if (pT30->isCall)
            {
                pT30->Timer_T1_Counter = 1; TRACE0("T30: T1 starts /Sender.");
                T30_GetHDLCfromRemote(pT30, TRUE);
                pT30->pfT30Vec = T30_PhaseA_Call;
            }
            else
            {
                pT30->Timer_T1_Counter = 1; TRACE0("T30: T1 starts /Receiver.");
                pT30->pfT30Vec = T30_PhaseA_Ans; pT30->txStage = LS_TX_STAGE_CRC_OK;
            }

            break;
        case RESULT_CODE_NOCARRIER:
        case RESULT_CODE_ERROR:

            if (pT30->isCall)
            {
                pT30ToDte->hangupCode = pDceToT30->resultCodeRsp == RESULT_CODE_ERROR ? HANGUP_RINGBACK_NO_CED : HANGUP_RINGBACK_NO_ANSWER;
            }
            else
            {
                pT30ToDte->hangupCode = HANGUP_RING_WITHOUT_HANDSHAKE;
            }

            T30_GotoPointB(pT30, pT30ToDte->hangupCode); break;
        case RESULT_CODE_NODIALTN:
            T30_SetResponse(pT30, RESULT_CODE_NODIALTN);
            T30_GotoPointB(pT30, HANGUP_NO_LOOP_CURRENT); break;
        case RESULT_CODE_BUSY:
            T30_SetResponse(pT30, RESULT_CODE_BUSY);
            T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_A_ERROR); break;
#if DCE_CONTROLLED_V8
        case RESULT_CODE_OK:
            T30_SetCommand(pT30ToDce, T30_DCE_ATO);
            break;
#endif
        case RESULT_CODE_NULL:
            break;
        case RESULT_CODE_RING:
            T30_SetResponse(pT30, RESULT_CODE_RING);
            break;
        default:
            TRACE1("T30: ERROR. Invalid result code response %d", pDceToT30->resultCodeRsp);
            T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_A_ERROR); break;
    }

    pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
}

#if SUPPORT_V34FAX
void T30_PhaseB_AfterEOM(T30Struct *pT30)
{
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;

    if (!pDceToT30->isDleEot)
    {
        // wait for the <DLE><EOT> back from DCE
        return;
    }

    switch (pDceToT30->resultCodeRsp)
    {
        case RESULT_CODE_OK:

            // this could be OK to the previous ATO, or to A8M
            if (pT30ToDce->currentT30DceCmd == T30_DCE_ATO)
            {
                // +F34
                if (pDteToT30->minPhaseCSpd > pDteToT30->faxParmSettings.bitRateBR)
                {
                    pDteToT30->minPhaseCSpd = pDteToT30->faxParmSettings.bitRateBR;
                }

                pT30ToDce->initV34PrimRateMax = pDteToT30->faxParmSettings.bitRateBR + 1;
                pT30ToDce->initV34PrimRateMin = pDteToT30->minPhaseCSpd + 1;
                pT30ToDce->initV34CntlRate = V34_BIT_DEFAULT;

                if (pT30->isCall)
                {
                    UBYTE localV8Menu[4] = {0x81, 0x85, 0xd4, 0x90};

                    if (!pT30->isTransmit)
                    {
                        // polling
                        localV8Menu[0] = 0xa1;
                    }

                    pT30ToDce->v8OpCntl = A8E_V8O_5; TRACE1("T30: +A8E=%d,", pT30ToDce->v8OpCntl);
                    memcpy(pT30ToDce->tempParm, localV8Menu, 4);
                    pT30ToDce->tempParmLen = 4;
                    T30_SetCommand(pT30ToDce, T30_DCE_A8M);
                    {
                        int i;

                        for (i = 0; i < 4; i ++)
                        {
                            TRACE("%02x", pT30ToDce->tempParm[i]);
                        }
                    }
                    TRACE0("");
                }
                else
                {
                    pT30ToDce->v8OpCntl = A8E_V8A_3; TRACE1("T30: +A8E=,%d", pT30ToDce->v8OpCntl);
                    pT30ToDce->currentT30DceCmd = T30_DCE_NULL;
                }
            }
            else if (pT30ToDce->currentT30DceCmd == T30_DCE_A8M)
            {
                T30_SetCommand(pT30ToDce, T30_DCE_ATO);
            }

            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            break;
        case RESULT_CODE_CONNECT:
            pT30ToDte->infoTxtRsp = INFO_TXT_FCO; TRACE0("T30: +FCO");

            if (pT30->isTransmit)
            {
                pT30->pfT30Vec = T30_PhaseB_SendDCS; pT30->txStage = LS_TX_STAGE_DATA;
            }
            else
            {
                pT30->pfT30Vec = T30_PhaseB_GetDCS; pT30->isDcsAsCmd = TRUE;
            }

            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            break;
        case RESULT_CODE_NOCARRIER:
        case RESULT_CODE_ERROR:

            if (pT30->isCall)
            {
                pT30ToDte->hangupCode = pDceToT30->resultCodeRsp == RESULT_CODE_ERROR ? HANGUP_RINGBACK_NO_CED : HANGUP_RINGBACK_NO_ANSWER;
            }
            else
            {
                pT30ToDte->hangupCode = HANGUP_RING_WITHOUT_HANDSHAKE;
            }

            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            T30_GotoPointB(pT30, pT30ToDte->hangupCode); return;
        case RESULT_CODE_NULL:
            break;
        default:
            TRACE1("T30: ERROR. Invalid result code response %d", pDceToT30->resultCodeRsp);
            pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
            T30_GotoPointB(pT30, HANGUP_UNSPECIFIED_PHASE_A_ERROR); return;
    }

    switch (pDceToT30->v8IndicationType)
    {
        case V8_A8A:
            break;
        case V8_A8M:
            memcpy(pT30->remoteV8Menu, pDceToT30->v8IndicationData, sizeof(pT30->remoteV8Menu));

            if (!pT30->isCall)
            {
                memcpy(pT30ToDce->tempParm, pT30->remoteV8Menu, sizeof(pT30->remoteV8Menu));
                pT30ToDce->tempParmLen = sizeof(pT30->remoteV8Menu);

                if (pT30->remoteV8Menu[0] == 0xa1 && pDteToT30->isDocForPolling)
                {
                    pT30ToDce->tempParm[0] = 0x81;
                }
                else
                {
                    pT30ToDce->tempParm[0] = 0xa1;
                }

                T30_SetCommand(pT30ToDce, T30_DCE_A8M);
                {
                    int i;

                    for (i = 0; i < 4; i ++)
                    {
                        TRACE("%02x", pT30ToDce->tempParm[i]);
                    }
                }
                TRACE0("");
            }

            break;
        case V8_A8J:
            break;
        case V8_A8C:
            break;
        case V8_A8I:
            break;
    }

    pDceToT30->v8IndicationType = V8_NULL;
    pDceToT30->v8IndicationLen = 0;
}
#endif

void T30_PhaseHangUp(T30Struct *pT30)
{
    //DteToT30Interface *pDteToT30 = pT30->pDteToT30;
    T30ToDteInterface *pT30ToDte = pT30->pT30ToDte;
    DceToT30Interface *pDceToT30 = pT30->pDceToT30;
    T30ToDceInterface *pT30ToDce = pT30->pT30ToDce;
#if SUPPORT_V34FAX
    UBYTE isV34Selected = pDceToT30->isV34Selected;

    if (isV34Selected)
    {
        if (pDceToT30->isDleEot && pDceToT30->resultCodeRsp == RESULT_CODE_OK)
        {
            pDceToT30->isDleEot = FALSE;

            if (pT30ToDce->currentT30DceCmd != T30_DCE_ATH)
            {
                T30_SetCommand(pT30ToDce, T30_DCE_ATH);
                pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
                pT30ToDce->tempParm[0] = 0;
            }
        }
    }

#endif

    if ((pT30ToDce->currentT30DceCmd == T30_DCE_ATH) && (pDceToT30->resultCodeRsp == RESULT_CODE_OK))
    {
        UBYTE hangupCode = pT30ToDte->hangupCode;
#if !SUPPORT_DCE
        UBYTE initStep = pT30->InitStep;
#endif

        pDceToT30->resultCodeRsp = RESULT_CODE_NULL;
        T30_SetResponse(pT30, RESULT_CODE_OK); // always OK

        T30_Init(pT30);

        if (pT30ToDte->dataCmd == DC3_XOFF)
        {
            pT30ToDte->dataCmd = DC1_XON; // fixing issue 184, temporarily
        }

        pT30ToDte->infoTxtRsp = INFO_TXT_FHS; TRACE1("T30: +FHS:%02x", hangupCode);
        pT30ToDte->hangupCode = hangupCode;
        pT30ToDte->resultCodeRsp = RESULT_CODE_OK;
#if !SUPPORT_DCE
        pT30->InitStep = initStep;
#endif
    }
}


static void T30_TimeCounter(T30Struct *pT30)
{
    if (pT30->Timer_T1_Counter)
    {
        pT30->Timer_T1_Counter++;
    }

    if (pT30->Timer_T2_Counter)
    {
        pT30->Timer_T2_Counter++;
    }

    if (pT30->Timer_T4_Counter)
    {
        pT30->Timer_T4_Counter++;
    }

    if (pT30->Timer_T5_Counter)
    {
        pT30->Timer_T5_Counter++;
    }

    if (pT30->Timer_3s_Counter)
    {
        pT30->Timer_3s_Counter++;
    }

    if (pT30->Timer_TCF)
    {
        pT30->Timer_TCF++;
    }
}

/*    the main interface function of T.30 module */
void T30_Main(T30Struct *pT30)
{
    DteToT30Interface *pDteToT30 = pT30->pDteToT30;

    T30_TimeCounter(pT30);

    if (pDteToT30->actionCmd == DTE_T30_FIP)
    {
        T30_InitT32Parms(pDteToT30);
        T30_SetResponse(pT30, RESULT_CODE_OK);
    }

    pT30->pfT30Vec(pT30);
}
#endif