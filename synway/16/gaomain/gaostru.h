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

#ifndef _GAO_STRU_H
#define _GAO_STRU_H

#include "porting.h"

#if SUPPORT_FAX
#include "faxapi.h"
#endif
#if SUPPORT_DTE
#include "tiffstru.h"
#include "dsmstru.h"
#include "imgprodte.h"
#endif
#if SUPPORT_T38EP
#include "t38ext.h"
#endif
#if SUPPORT_VXX_DCE
#include "mdmmain.h"
#endif
#if SUPPORT_T30
#include "t30ext.h"
#endif
#if SUPPORT_PARSER
#include "circbuff.h"
#endif

typedef struct
{
#if SUPPORT_VXX_DCE
    GaoModemStruct m_Modem;
#endif
#if SUPPORT_PARSER
    UBYTE m_MemoryIn[BUFSIZE];
    CircBuffer m_BufferIn;
    UBYTE m_MemoryOut[BUFSIZE];
    CircBuffer m_BufferOut;
#endif
#if SUPPORT_DTE
    TTiffStruct m_Tiff;
    TDsmStrcut m_Dsm;
    TImgProDteStruct m_Img;

    GAO_Msg m_TiffDsm;
    GAO_Msg m_DsmImg;
    GAO_Msg m_ImgDce;
#endif

#if SUPPORT_T38_DCE
    T38Struct m_T38;
#endif
#if SUPPORT_T30
    T30Struct m_T30;
#endif

#if SUPPORT_T31
    DceToT30Interface dceToT30;
    T30ToDceInterface t30ToDce;
#endif

#if SUPPORT_T32
    DteToT30Interface dteToT30;
    T30ToDteInterface t30ToDte;
#endif

} GAO_LibStruct;

#endif
