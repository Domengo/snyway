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

/*****************************
file      : v21isr.c, for fax
******************************/

#include "v21ext.h"
#include "ptable.h"
#include "mhsp.h"
#include "ioapi.h"

#if SUPPORT_V21CH2FAX

void V21_Ch2_DPCS_isr(UBYTE **pTable)
{
    DpcsStruct *pDpcs = (DpcsStruct *)(pTable[DPCS_STRUC_IDX]);
    QWORD     *pPCMin = (QWORD *)(pTable[PCMINDATA_IDX]);
    QWORD    *pPCMout = (QWORD *)(pTable[PCMOUTDATA_IDX]);
    UBYTE   *pIOTxBuf = (UBYTE *)(pTable[DSPIORDBUFDATA_IDX]);
    UBYTE   *pIORxBuf = (UBYTE *)(pTable[DSPIOWRBUFDATA_IDX]);
    V21Struct   *pV21 = (V21Struct *)(pTable[V21_STRUC_IDX]);
    UBYTE Disc;
    UBYTE i, ubHdlc;
#if SUPPORT_ENERGY_REPORT
    QDWORD qdEnergy;
#endif
#if WITH_DC
    DCStruct *pDC = (DCStruct *)(pV21->pTable[DC_STRUC_IDX]);

    /* Low pass DC estimator filter */
    DcEstimator(pDC, pPCMin, pV21->ubBufferSize, -9);
#endif

    pV21->PCMinPtr = pPCMin;

    Disc = 0;

    /* ========> fax-receive (answer, OR 0) mode <===== */
    if (pV21->modem_mode == ANS_MODEM)
    {
        pV21->pfRxVec(pV21);

        /* If Data mode, send received bits back to IO */
        if (pV21->RxMode)
        {
            ubHdlc = RdReg(pDpcs->MFSR0, FAX_HDLC);

            if (ubHdlc && (pV21->RxMode == 1))
            {
                pV21->RxMode = 2;    /* Hdlc flag detected once */
            }

#if SUPPORT_ENERGY_REPORT

            if (pV21->Detect_FaxTone == 0)
            {
                V21Detect_7E(pV21);
            }

            if (pV21->Detect_FaxTone != 0)
            {
                if (pV21->Detect_FaxTone == 1)
                {
                    pV21->Detect_FaxTone = 2;
                    SetReg(pDpcs->MFSR0, FAX_DETECT);
                }

                qdEnergy = 0;

                for (i = 0; i < pV21->ubBufferSize; i++)
                {
                    qdEnergy += QQMULQD(pV21->PCMinPtr[i], pV21->PCMinPtr[i]) >> 8;
                }

                /**** CHANGE IN CARRIER LEVEL DETECTION ******/
                pV21->qdEnergyAcc += qdEnergy;
                pV21->uEnergyCount++;

                if ((pV21->uEnergyCount == 9) && (pV21->Disconnect_flag == 0))
                {
                    if ((pV21->qdEnergyMax == 0) && (pV21->qdEnergyMin == 0))
                    {
                        pV21->qdEnergyMax = pV21->qdEnergyAcc;
                        pV21->qdEnergyMin = pV21->qdEnergyAcc;
                    }
                    else if (pV21->qdEnergyAcc > pV21->qdEnergyMax)
                    {
                        pV21->qdEnergyMax = pV21->qdEnergyAcc;
                    }
                    else if (pV21->qdEnergyAcc < pV21->qdEnergyMin)
                    {
                        pV21->qdEnergyMin = pV21->qdEnergyAcc;
                    }

                    pV21->qdEnergyAcc = 0;
                    pV21->uEnergyCount = 0;

                    if ((pV21->qdEnergyMax > ((pV21->qdEnergyMin) << 3)))
                    {
                        pV21->Disconnect_flag = 1;
                        TRACE0("V21: Change of Carrier");
                    }
                }

                /**** CHANGE IN CARRIER LEVEL DETECTION ******/

                qdEnergy += GetReg(pDpcs->ENERGY);

                PutReg(pDpcs->ENERGY, qdEnergy);
            }

#endif

            for (i = 0; i < pV21->RxNumBits; i++)
            {
                *pIORxBuf = pV21->pOutBits[i];

#if 0

                if (DumpTone6_Idx < 100000) { DumpTone6[DumpTone6_Idx++] = pV21->pOutBits[i]; }

#endif
                IoWrProcess(pTable, 1);
            }
        }

        if (pV21->Ch2TimeOutCount > V21_TIMEOUT)
        {
            pV21->Ch2TimeOutCount = 0;
            Disc                  = 1;
        }

        if (pV21->Disconnect_flag == 1)
        {
            if ((pV21->RxMode != 2) && (pV21->ubCh2DiscCount == 0))
            {
                V21_Ch2_Init_main(pV21);
                pV21->ubCh2DiscCount = 1;
            }
            else
            {
                if (pV21->RxMode != 0)
                {
                    /* Flush buffered bits to top level application */
                    *pIORxBuf = 0;
                    IoWrProcess(pTable, 7);
                }

                Disc = 1;
            }
        }
    }
    else /* =====modem_mode = 1, CALL/transmit === */
    {
        /* ======== only if fax-transmit (CALL_MODEM, or 1) mode ???========= */
        if (pV21->TxMode) /* Data mode */
        {
            PutReg(pDpcs->MSR0, DATA);

            /* Read one bit from IO */
            IoRdProcess(pTable, 1);

            pV21->TxBits = (UBYTE)((*pIOTxBuf) & 0x01);
        }
    }

    pV21->PCMoutPtr = pPCMout;
    pV21->pfTxVec(pV21);

    pV21->PCMinPtr  += pV21->ubBufferSize;
    pV21->PCMoutPtr += pV21->ubBufferSize;

    /* Look for disconnect possibily */
    Disc |= RdReg(pDpcs->MCF, DISCONNECT);

    if (Disc)
    {
        ClrReg(pDpcs->MCF);
        Disconnect_Init(pTable);
        TRACE0("V21: Disconnect");
    }
}

#if SUPPORT_ENERGY_REPORT
void V21Detect_7E(V21Struct *pV21)
{
    UWORD Temp;
    UBYTE i;

    for (i = 0; i < pV21->RxNumBits; i++)
    {
        Temp = (UWORD)(pV21->pOutBits[i]) & 0x01;
        pV21->uFlagPattern = (pV21->uFlagPattern << 1) | Temp;

        if ((pV21->uFlagPattern & 0xFFFF) == 0x7E7E)
        {
            pV21->Detect_FaxTone = 1;
            TRACE0("V21: Ch2 Tone Detected and received 0x7E");
        }
    }
}
#endif

#endif
