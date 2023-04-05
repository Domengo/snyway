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

#include "ph2ext.h"

/***********************************************/
/* biquad tone detection                       */
/* CALL MODEM:    Vector 8, 12                 */
/* ANSWER MODEM:  Vector 3, 10                 */
/***********************************************/
/* Biquads filtering routine, B0 B1 B2 A1 A2 */
void Ph2_Biquad_Tone_Det(Ph2Struct *pPH2)
{
    UBYTE i;
    QWORD biq_out;

    pPH2->qdTotal_egy       = 0;
    pPH2->qdTotal_egy_biqin = 0;

    for (i = 0; i < PH2_SYM_SIZE; i++)
    {
#if 0

        if (DumpTone1_Idx < 100000) { DumpTone1[DumpTone1_Idx++] = pPH2->PCMinPtr[i]; }

#endif

        pPH2->qdTotal_egy_biqin += QQMULQD(pPH2->PCMinPtr[i], pPH2->PCMinPtr[i]) >> 4;

        biq_out = DspIIR_Cas51(&(pPH2->biq), pPH2->PCMinPtr[i]);

        /* If L2 is sending, we want to apply AGC gain to the TONE */
        /* band pass filter output to avoid echo of L2. This is    */
        /* important if the TONE is much smaller than the echo.    */
        if (pPH2->L2_done == 2) /* L1/L2 echo presence */
        {
            biq_out = QQMULQR8(pPH2->AGC_gain, biq_out);
        }

        pPH2->qdTotal_egy += QQMULQD(biq_out, biq_out) >> 4;
    }

    pPH2->qdTotal_egy_biqin >>= 1;
}
