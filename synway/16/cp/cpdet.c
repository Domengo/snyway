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

/*-------------------------------------------------------------------------*/
/* Tone Detection Routine                                                  */
/*                                                                         */
/* Using Goertzel algorithm                                                */
/*                                                                         */
/* INPUT: CpStruct *pCp - Call Progress structure                          */
/*-------------------------------------------------------------------------*/

#include "cpext.h"

void CP_Detect_init(CpStruct *pCp)
{
    SBYTE i, j;
    SWORD sTemp;

    DSPD_DFT_Init(&(pCp->Dft), (CP_BUF_SIZE << 2), TONE_MAX_FREQ, (QWORD *)CP_tToneGcoefs, pCp->qdMagSq, pCp->qQ1Q2, 7);

    pCp->AnsCount     = 0;
    pCp->BellAnsCount = 0;

    Cadence_init(pCp);

    pCp->LastDetTones = 0;
    pCp->DetTones = 0;

    pCp->ubDialToneDetected = 0;
    pCp->ubBusyToneDetected = 0;
    pCp->ubCongToneDetected = 0;
    pCp->ubRingToneDetected = 0;
    /* this allow 3 kind of dialtone */

#if SUPPORT_AUTO
    pCp->ubBusyToneFreq = 0;
#endif

    /***  set cadence pattern for countries ***/
    switch (pCp->ubCountry)
    {
        case CP_CANADA:
            CP_Tone_init_CANADA(pCp);
            TRACE0("CANADA");
            break;
        case CP_CHINA:
            CP_Tone_init_CHINA(pCp);
            TRACE0("CHINA");
            break;
        case CP_FRANCE:
            CP_Tone_init_FRANCE(pCp);
            TRACE0("FRANCE");
            break;
        case CP_GERMANY:
            CP_Tone_init_GERMANY(pCp);
            TRACE0("GERMANY");
            break;
        case CP_ISRAEL:
            CP_Tone_init_ISRAEL(pCp);
            TRACE0("ISRAEL");
            break;
        case CP_KOREA:
            CP_Tone_init_KOREA(pCp);
            TRACE0("KOREA");
            break;
        case CP_NETHERLANDS:
            CP_Tone_init_NETHERLANDS(pCp);
            TRACE0("NETHERLANDS");
            break;
        case CP_SPAIN:
            CP_Tone_init_SPAIN(pCp);
            TRACE0("SPAIN");
            break;
        case CP_UK:
            CP_Tone_init_UK(pCp);
            TRACE0("UK");
            break;
        case CP_VANUATU:
            CP_Tone_init_VANUATU(pCp);
            TRACE0("VANUATU");
            break;
        case CP_AUSTRALIA:
            CP_Tone_init_AUSTRALIA(pCp);
            TRACE0("AUSTRALIA");
            break;
        case CP_JAPAN:
            CP_Tone_init_JAPAN(pCp);
            TRACE0("JAPAN");
            break;
        default:
            CP_Tone_init_CANADA(pCp);
            TRACE0("Default: CANADA");
            break;
    }

    /****** error tolerance ******/
    /* the following calculation can be replaced by a table for reducing mips */
    for (i = 0; i < GROUPTONENUMBER; i++)
    {
        /* dial tone */
        for (j = 0; j < pCp->DialToneReference[i].ubCadenceNum; j++)
        {
            sTemp = pCp->DialToneReference[i].nCadenceOn[j];

            sTemp = ((SDWORD)sTemp * 111 + 999) / 1000;

            pCp->DialToneReference[i].nCadenOnErr[j] = sTemp;

            sTemp = pCp->DialToneReference[i].nCadenceOff[j];

            if (sTemp == CP_Cadence_OffTimeMS(50))
            {
                sTemp = ((SDWORD)sTemp * 333 + 999) / 1000;
            }
            else
            {
                sTemp = ((SDWORD)sTemp * 111 + 999) / 1000;
            }

            pCp->DialToneReference[i].nCadenOffErr[j] = sTemp;
        }

        /* busy tone */
        for (j = 0; j < pCp->BusyToneReference[i].ubCadenceNum; j++)
        {
            sTemp = pCp->BusyToneReference[i].nCadenceOn[j];

            sTemp = ((SDWORD)sTemp * 111 + 999) / 1000;

            pCp->BusyToneReference[i].nCadenOnErr[j] = sTemp;

            sTemp = pCp->BusyToneReference[i].nCadenceOff[j];

            sTemp = ((SDWORD)sTemp * 111 + 999) / 1000;

            pCp->BusyToneReference[i].nCadenOffErr[j] = sTemp;
        }

        /* congestion tone */
        for (j = 0; j < pCp->CongestionToneReference[i].ubCadenceNum; j++)
        {
            sTemp = pCp->CongestionToneReference[i].nCadenceOn[j];

            sTemp = ((SDWORD)sTemp * 111 + 999) / 1000;

            pCp->CongestionToneReference[i].nCadenOnErr[j] = sTemp;

            sTemp = pCp->CongestionToneReference[i].nCadenceOff[j];

            sTemp = ((SDWORD)sTemp * 111 + 999) / 1000;

            pCp->CongestionToneReference[i].nCadenOffErr[j] = sTemp;
        }

        /* ring back tone */
        for (j = 0; j < pCp->RingBackToneReference[i].ubCadenceNum; j++)
        {
            sTemp = pCp->RingBackToneReference[i].nCadenceOn[j];

            sTemp = ((SDWORD)sTemp * 111 + 999) / 1000;

            pCp->RingBackToneReference[i].nCadenOnErr[j] = sTemp;

            sTemp = pCp->RingBackToneReference[i].nCadenceOff[j];

            sTemp = ((SDWORD)sTemp * 111 + 999) / 1000;

            pCp->RingBackToneReference[i].nCadenOffErr[j] = sTemp;
        }
    }

#if SUPPORT_FAX
    pCp->V21ToneCount  = 0;
#endif
}

void CP_Detect(CpStruct *pCp)
{
    DftStruct *pDft = &(pCp->Dft);
    SBYTE  i, j, iMax1, iMax2;
    UBYTE  ubStartOn;
    SWORD  *pPCMin;
    SDWORD sdMnsqrMaxTemp, sdMnsqrMax1, sdMnsqrMax2;
    QDWORD qdAvg_Energy;
    UWORD  uBusyReferenceTone;

    qdAvg_Energy = DSPD_DFT_Detect(pDft, pCp->PCMinPtr, CP_BUF_SIZE);

    if (qdAvg_Energy > 0)
    {
        qdAvg_Energy >>= 5;

#if 0

        if (DumpTone1_Idx < 1000000) { DumpTone1[DumpTone1_Idx++] = pCp->qdMagSq[0]; }

        if (DumpTone2_Idx < 1000000) { DumpTone2[DumpTone2_Idx++] = pCp->qdMagSq[1]; }

        if (DumpTone3_Idx < 1000000) { DumpTone3[DumpTone3_Idx++] = pCp->qdMagSq[2]; }

        if (DumpTone4_Idx < 1000000) { DumpTone4[DumpTone4_Idx++] = pCp->qdMagSq[3]; }

        if (DumpTone5_Idx < 1000000) { DumpTone5[DumpTone5_Idx++] = pCp->qdMagSq[10]; }

        if (DumpTone6_Idx < 1000000) { DumpTone6[DumpTone6_Idx++] = qdAvg_Energy; }

#endif
        sdMnsqrMax1 = 0x80000000;
        sdMnsqrMax2 = 0x80000000;
        iMax1 = -1;
        iMax2 = -1;

        /********* detecting one or two tones **********/
        for (i = 0; i < (TONE_MAX_FREQ - 2); i++)
        {
            sdMnsqrMaxTemp = pCp->qdMagSq[i];

            if ((sdMnsqrMaxTemp > DET_TONETHRES) && (sdMnsqrMaxTemp >= qdAvg_Energy))
            {
                if (sdMnsqrMax1 < sdMnsqrMaxTemp)
                {
                    sdMnsqrMax2 = sdMnsqrMax1;
                    iMax2 = iMax1;

                    sdMnsqrMax1 = sdMnsqrMaxTemp;
                    iMax1 = i;
                }
                else if (sdMnsqrMax2 < sdMnsqrMaxTemp)
                {
                    sdMnsqrMax2 = sdMnsqrMaxTemp;
                    iMax2 = i;
                }
            }
        }

        if (iMax1 != -1)     /* at least one tone is detected */
        {
            if (pCp->nTone_First & (1 << iMax1))   /* max1 is same as one of the previous detected tones (however, the previous max1 may fade away but the max2 remains */
            {
                if (pCp->nTone & (1 << iMax1))
                {
                    pCp->nTone |= (1 << iMax1);
                }
                else
                {
                    pCp->nTone  = (1 << iMax1);
                }
            }

            if ((iMax2 != -1) && (pCp->nTone_First & (1 << iMax2)))
            {
                pCp->nTone |= (1 << iMax2);
            }

            pCp->nTone_First = (1 << iMax1);

            if (iMax2 != -1)
            {
                pCp->nTone_First |= (1 << iMax2);
            }
        }

        for (i = 4; i < 8; i++)
        {
            if (pCp->qdMagSq[i] > DET_TONETHRES && (pCp->qdMagSq[i] >= qdAvg_Energy))
            {
                pCp->DetTones = RECEIVEROFFHOOKTONE;
            }
        }

        if ((pCp->qdMagSq[12] > DET_ANSTONETHRES) && (pCp->qdMagSq[12] >= qdAvg_Energy))
        {
            pCp->AnsCount++;

#if SUPPORT_FAX_SEND_CNG

            if (pCp->Fax_Callmode &&
                (((pCp->V8Mode == 1) && (pCp->AnsCount == DET_ANSAM_CNT_THRES)) || (pCp->AnsCount == DET_CED_CNT_THRES)))
            {
                pCp->RemoteAnsToneDetected = 1;
                pCp->AnsCount = 0;
            }

#endif
        }

#if SUPPORT_FAX

        /* Consider V21 tone as answer tone. */
        if ((pCp->Fax_Callmode) && (pCp->qdMagSq[TONE_MAX_FREQ - 1] > DET_ANSTONETHRES) && (pCp->qdMagSq[TONE_MAX_FREQ - 1] >= qdAvg_Energy))
        {
            pCp->V21ToneCount++;
        }

#endif

        if ((pCp->qdMagSq[13] > DET_ANSTONETHRES) && (pCp->qdMagSq[13] >= qdAvg_Energy))
        {
            pCp->BellAnsCount++;
        }

        /* return with success (for now) */
        /* Clean up the qQ1Q2 buffer */
        for (i = 0; i < TONE_Q1Q2SIZE; i++)
        {
            pCp->qQ1Q2[i] = 0;
        }
    }

    pPCMin = pCp->PCMinPtr;

    for (i = 0; i < CP_BUF_SIZE; i += CP_WINDOW_SIZE)
    {
        /********************************************/
        /*          calculating the cadence         */
        /********************************************/
        ubStartOn = Ca_Calculate(pCp, &pPCMin[i]);

        if (pCp->nTone)
        {
            if (pCp->ubDialToneDetected == 0)
            {
                /****************************************/
                /*            dial tone                 */
                /****************************************/
                for (j = 0; j < GROUPTONENUMBER; j++)
                {
                    if (pCp->DialToneReference[j].ubCadenceNum == 0)
                    {
                        if (CheckPureTone(pCp, &pCp->DialToneReference[j]))
                        {
                            pCp->DetTones = DIALTONE;
                            pCp->ubDialToneDetected = 1;

                            TRACE1("DIALTONE %d", j + 1);
                        }
                    }
                    else if ((ubStartOn == 1) && (pCp->DialToneReference[j].ubCadenceNum > 0))
                    {
                        if (CheckToneCadence(pCp, &pCp->DialToneReference[j]))
                        {
                            pCp->DetTones = DIALTONE;
                            pCp->ubDialToneDetected = 1;

                            TRACE1("DIALTONE %d", j + 1);
                        }
                    }
                }
            }

            if (pCp->ubBusyToneDetected == 0)
            {
                /****************************************/
                /*              busy tone               */
                /****************************************/
                for (j = 0; j < 2; j++)
                {
                    uBusyReferenceTone = pCp->BusyToneReference[j].nTone;

                    if (uBusyReferenceTone != 0 && (pCp->nTone & uBusyReferenceTone) == uBusyReferenceTone)
                    {
#if SUPPORT_AUTO
                        pCp->ubBusyToneFreq = 1;
#endif

                        if ((ubStartOn == 1) && (pCp->BusyToneReference[j].ubCadenceNum > 0))
                        {
                            if (CheckToneCadence(pCp, &pCp->BusyToneReference[j]))
                            {
                                pCp->DetTones = BUSYTONE;
                                pCp->ubBusyToneDetected = 1;

                                TRACE1("BUSYTONE %d", j + 1);
                            }
                        }
                    }
                }
            }

            if (pCp->ubCongToneDetected == 0)
            {
                /****************************************/
                /*           Congestion tone            */
                /****************************************/
                for (j = 0; j < 2; j++)
                {
                    if ((ubStartOn == 1) && (pCp->CongestionToneReference[j].ubCadenceNum > 0))
                    {
                        if (CheckToneCadence(pCp, &pCp->CongestionToneReference[j]))
                        {
                            pCp->DetTones = CONGESTIONTONE;
                            pCp->ubCongToneDetected = 1;

                            TRACE1("CONGESTIONTONE %d", j + 1);
                        }
                    }
                }
            }

            if (pCp->ubRingToneDetected == 0)
            {
                /****************************************/
                /*            Ring tone                 */
                /****************************************/
                for (j = 0; j < 2; j++)
                {
                    if ((ubStartOn == 1) && (pCp->RingBackToneReference[j].ubCadenceNum > 0))
                    {
                        if (CheckToneCadence(pCp, &pCp->RingBackToneReference[j]))
                        {
                            pCp->DetTones = RINGBACKTONE;
                            pCp->ubRingToneDetected = 1;

                            TRACE1("RINGBACKTONE %d", j + 1);
                        }
                    }
                }
            }
        }
    }
}

void Cadence_init(CpStruct *pCp)
{
    UBYTE i;

    pCp->nTone       = 0;
    pCp->nTone_First = 0;

    for (i = 0; i < 16; i++)
    {
        pCp->nHistoryCadenceOn[i]  = 0;
        pCp->nHistoryCadenceOff[i] = 0;
    }

    pCp->ubHistoryIndex = 0;

    pCp->ubDetected = 0;

    pCp->nVadTotalEgy  = 0;
    pCp->nVadTotalEgy1 = 0;
    pCp->ubLastVadDet  = 0;
    pCp->nCurrCadence  = 0;
}

UBYTE Ca_Calculate(CpStruct *pCp, SWORD *pPCMin)
{
    SDWORD curr_egy = 0;
    SWORD  qIn;
    SWORD  sTmp;
    UBYTE  i;
    UBYTE  ubRet = 0;

    for (i = 0; i < CP_WINDOW_SIZE; i++)
    {
        qIn = pPCMin[i];

        if (qIn < 0)
        {
            qIn = -qIn;
        }

        curr_egy = pCp->nVadTotalEgy1 + ((qIn - pCp->nVadTotalEgy1) >> CP_WINDOW_SIZE);

#if 0

        if (DumpTone1_Idx < 1000000) { DumpTone1[DumpTone1_Idx++] = curr_egy; }

#endif

        if (curr_egy > 32767)
        {
            curr_egy = 32767;
        }
        else if (curr_egy < -32768)
        {
            curr_egy = -32768;
        }

        pCp->nVadTotalEgy1 = (SWORD)curr_egy;

        curr_egy = pCp->nVadTotalEgy + ((curr_egy - pCp->nVadTotalEgy) >> CP_WINDOW_SIZE);

        if (curr_egy > 32767)
        {
            curr_egy = 32767;
        }
        else if (curr_egy < -32768)
        {
            curr_egy = -32768;
        }

        pCp->nVadTotalEgy = (SWORD)curr_egy;
    }

#if 0

    if (DumpTone2_Idx < 1000000) { DumpTone2[DumpTone2_Idx++] = pCp->nVadTotalEgy; }

#endif

    if (curr_egy > (SDWORD)CP_VAD_THRESHOLD)
    {
        /* Detect VAD, CP_VAD_ON */
        if (pCp->ubLastVadDet != CP_VAD_ON && pCp->nCurrCadence > 25) /**** OFF to ON transition ****/
        {
            /* TRACE1("off:%d", pCp->nCurrCadence); */
            pCp->nHistoryCadenceOff[pCp->ubHistoryIndex] = pCp->nCurrCadence; /* Record OFF time */
            pCp->nCurrCadence = 0;
            pCp->ubLastVadDet = CP_VAD_ON;

            pCp->ubHistoryIndex ++;
            ubRet = 1;
        }

        pCp->nHistoryCadenceOn[pCp->ubHistoryIndex] = pCp->nCurrCadence;
    }
    else
    {
        /* Detect silence */
        if (pCp->ubLastVadDet != CP_VAD_OFF && pCp->nCurrCadence > 25) /**** ON to OFF transition ****/
        {
            pCp->nHistoryCadenceOn[pCp->ubHistoryIndex] = pCp->nCurrCadence; /* Record ON time */

            /* TRACE1("on:%d", pCp->nCurrCadence); */
            pCp->nCurrCadence = 0;
            pCp->ubLastVadDet = CP_VAD_OFF;
        }
        else
        {
            /* pCp->nHistoryCadenceOff[pCp->ubHistoryIndex] = pCp->nCurrCadence; */   /** OFF-OFF **/

            if (pCp->ubDetected && (pCp->nHistoryCadenceOff[pCp->ubHistoryIndex] == CP_Cadence_OffTimeMS(5000)))
            {
                /*************  long silent *************/
                Cadence_init(pCp);
            }
        }
    }

    pCp->nCurrCadence ++;

    sTmp = CP_Cadence_OnTimeMS(5000);

    if (pCp->nCurrCadence >= sTmp)
    {
        pCp->nCurrCadence = sTmp;
    }

    return ubRet;
}

UBYTE CheckToneCadence(CpStruct *pCp, CaStruct *pCaRef)
{
    UBYTE i;
    SBYTE sbIndex;
    SWORD qToleranceOn, qToleranceOff;

    sbIndex = (SBYTE)pCp->ubHistoryIndex;

    /* on off tone */
    sbIndex -= pCaRef->ubCadenceNum;

    if (pCaRef->nTone == 0 || sbIndex <= 0 || (pCp->nTone & pCaRef->nTone) != pCaRef->nTone)
    {
        return (0);
    }

    for (i = 0; i < pCaRef->ubCadenceNum; i++)
    {
        /* on */
        qToleranceOn = pCp->nHistoryCadenceOn[sbIndex] - pCaRef->nCadenceOn[i];
        qToleranceOn = QABS(qToleranceOn);

        /* off */
        qToleranceOff = pCp->nHistoryCadenceOff[sbIndex] - pCaRef->nCadenceOff[i];
        qToleranceOff = QABS(qToleranceOff);

        if (qToleranceOn > pCaRef->nCadenOnErr[i] || qToleranceOff > pCaRef->nCadenOffErr[i])
        {
            return (0);
        }
        else
        {
            sbIndex ++;
        }
    }

    /*  clean history record */
    Cadence_init(pCp);

    return (1);
}

UBYTE CheckPureTone(CpStruct *pCp, CaStruct *pCaRef)
{
    UWORD uIndex;
    SWORD qTolerance;

    if (pCaRef->nTone == 0 || (pCp->nTone & pCaRef->nTone) != pCaRef->nTone)
    {
        return 0;
    }

    uIndex = pCp->ubHistoryIndex;

    /* pure tone */
    qTolerance = pCp->nHistoryCadenceOn[uIndex] - pCaRef->nCadenceOn[0];

    if (qTolerance > 0)
    {
        /*  clean history record */
        Cadence_init(pCp);

        return 1;
    }

    return 0;
}
