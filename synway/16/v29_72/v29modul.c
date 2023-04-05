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

/*************************************************************************
void V29_Modulate(V29Struct *pV29)

  This program defines some functions related to moduator off V29 modem.
  It accepts the input from the pulse shape filter and modualtes according
  to the standard V29 and determines the modulated sample.
**************************************************************************/

#include "v29ext.h"

void V29_TX_Modulate(V29Struct *pV29)
{
    UBYTE  i;
    UWORD  Phase;
    QWORD  qCf, qSf;
    QWORD  qC, qS;
    QDWORD qdTemp;

    Phase = pV29->ModPhase;

    for (i = 0; i < V29_SYM_SIZE; i++)
    {
        /* find cos, sin values */
        SinCos_Lookup_Fine(Phase, &qS, &qC);
        Phase_Update(&Phase, V29_CARRIER_1700_DELTA_PHASE);

        /* modulate and store I and Q value for point */
        qCf = pV29->pqPsfOutIptr[i];
        qSf = pV29->pqPsfOutQptr[i];

        qdTemp = QQMULQD(qCf, qC) - QQMULQD(qSf, qS);
        pV29->PCMoutPtr[i] = QDR15Q(qdTemp);
    }

    pV29->ModPhase = Phase;
}
