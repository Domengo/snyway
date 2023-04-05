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

/******************************************************************************
        File Name : GSTDIO.H

        Description : Header file of GAO Stdio library for cross
                      platform debugging port.

        Authors :  Benjamin Chan
*****************************************************************************/

#ifndef _GSTDIO_H
#define _GSTDIO_H

#include "porting.h"

#define TRACEON (1 && !CUSTOMER_BUILD)

#if defined(__TMS320C55X__)
#define FORMAT_DWORD_PREFIX "l"
#else
#define FORMAT_DWORD_PREFIX ""
#endif

#if TRACEON
#include <string.h>

void gprintf(char *, ...);

#define TRACE(str,  para1)                        {gprintf((str), (para1));}
#define TRACE13(str,para1)                        {if (para1 == 0x0d) gprintf("\n"); else gprintf((str), (para1));}
#define TRACE0(str)                               {if (strstr(str, "ERROR") == NULL && strstr(str, "WARNING") == NULL) gprintf(str "\n"); else gprintf(str " %s %d\n", __FILE__, __LINE__);}
#define TRACE1(str, para1)                        {if (strstr(str, "ERROR") == NULL && strstr(str, "WARNING") == NULL) gprintf(str "\n", (para1)); else gprintf(str " %s %d\n", (para1), __FILE__, __LINE__);}
#define TRACE2(str, para1, para2)                 {if (strstr(str, "ERROR") == NULL && strstr(str, "WARNING") == NULL) gprintf(str "\n", (para1), (para2)); else gprintf(str " %s %d\n", (para1), (para2), __FILE__, __LINE__);}
#define TRACE3(str, para1, para2, para3)          {if (strstr(str, "ERROR") == NULL && strstr(str, "WARNING") == NULL) gprintf(str "\n", (para1), (para2), (para3)); else gprintf(str " %s %d\n", (para1), (para2), (para3), __FILE__, __LINE__);}
#define TRACE4(str, para1, para2, para3, para4)   {if (strstr(str, "ERROR") == NULL && strstr(str, "WARNING") == NULL) gprintf(str "\n", (para1), (para2), (para3), (para4)); else gprintf(str " %s %d\n", (para1), (para2), (para3), (para4), __FILE__, __LINE__);}
#else
#define TRACE(str,  para1) {}
#define TRACE13(str,para1) {}
#define TRACE0(str) {}
#define TRACE1(str, para1) {}
#define TRACE2(str, para1, para2) {}
#define TRACE3(str, para1, para2, para3) {}
#define TRACE4(str, para1, para2, para3, para4) {}
#endif
#endif
