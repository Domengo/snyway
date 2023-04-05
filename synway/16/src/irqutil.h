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

/**********************************************************************/
/*                   I R Q U T I L  . H                               */
/**------------------------------------------------------------------**/
/* task           : Constants and prototypes for  the interrupt       */
/*                  functions.                                        */
/**------------------------------------------------------------------**/
/* Author          : Michael Tischer / Bruno Jennrich                 */
/* developed on  : 3/12/1994                                          */
/* last update : 4/06/1995                                            */
/**------------------------------------------------------------------**/
/* COMPILER       : Borland C++ 3.1, Microsoft Visual C++  1.5        */
/**********************************************************************/
#ifndef _IRQUTIL_H
#define _IRQUTIL_H

#include "types.h"

/*- IRQ-Controller Port addresses --------------------------------------*/
#define MASTER_PIC          0x20       /* master-PIC base address*/
#define SLAVE_PIC           0xA0        /* slave-PIC base address*/
#define IRQ_MASK            0x01         /* offset to masking port */

/*- IRQ-commandos ----------------------------------------------------*/
#define EOI                 0x20 /* not specified End of Interrupt */

#define MASTER_FIRST_VECTOR 0x08   /* hardware software vectors- */
#define SLAVE_FIRST_VECTOR  0x70   /* interrupts                      */

/*- Prototypes -------------------------------------------------------*/
void irq_Enable(int iIRQ);
void irq_Disable(int iIRQ);
void irq_SendEOI(int iIRQ);
void (_interrupt _FP *irq_SetHandler(int iIRQ, void (_interrupt _FP *lpHandler)()))();

#endif
