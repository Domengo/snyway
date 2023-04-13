/*****************************************************************************
*             Copyright (c) 1995 - 2010 by GAO Research Inc.                 *
*                          All rights reserved.                              *
*                                                                            *
*  This program is a confidential and an unpublished work of GAO Research    *
*  Inc. that is protected under international and Canadian copyright laws.   *
*  This program is a trade secret and constitutes valuable property of GAO   *
*  Research Inc. All use, reproduction and/or disclosure in whole or in      *
*  part of this program is strictly prohibited unless expressly authorized   *
*  in writing by GAO Research Inc.                                           *
*****************************************************************************/

#include "commtype.h"
#include "gaoapi.h"
#include "hdlcstru.h"



/*Turn this on to do simulation from recorded PCM and DTE data files*/
#define Modem_Data_SIZE              (14100)
/*We will give the exact size with the software delivery*/
#define Modem_Channel_Number         (1)
/*This can be changed to any number as long as there is enough MIPS*/
#define PCMSIZE 100

//----------------- fax modem data -----------------------------------
UBYTE ModemDataMem[Modem_Channel_Number][Modem_Data_SIZE];
/*Assume array store as [0][0]--[0][n];[1][0]--[1][n];...;[m][0]--[m][n]*/
UBYTE *pModemDataMem[Modem_Channel_Number];

UBYTE MODEM_Status Status[Modem_Channel_Number];

SWORD pPCMIn[PCMSIZE];
SWORD pPCMOut[PCMSIZE];
//---------------- DTE buffer -------------------------------
#define UBDTE_BUF_SIZE               (512)
UBYTE ubDTE_Buf[UBDTE_BUF_SIZE];
//---------------------- globe signal with system -------------------------
UBYTE DataReady[Modem_Channel_Number];
/*When system gets #i channel PCM samples ready, set this globe variable DataReady[i] to 1 */
UBYTE ESC_flag = 0;
/*when system quits fax application, set this globe variable ESC_flag to 1*/


//---------------- functions Synway should provide ----------------------
void Driver_Initialize(void)
{
/*Synway initializes all other stuff */
}
void HwIn(SWORD* pPCM,UWORD Channel,UBYTE Buffersize)
{
/*Synway puts the specific channel, indicated by "Channel", PCM samples to Buffer that starts at pPCM,
the PCM number is defined by Buffersize*/
}
void HwOut(SWORD* pPCM,UWORD Channel,UBYTE Buffersize)
{
/*Synway puts PCM samples that store in Buffer starting at pPCM to the specific channel, indicated by "Channel",
the PCM number is defined by Buffersize*/
}

UWORD GetCharFrT30(UBYTE *ubDTE_Buf, UWORD uLenT, UWORD Channel)
{
    UWORD Ret =0;
/*Synway fills the buffer ubDTE_Buf with no more than uLenT T30/class1 characters, channel is Channel,
and return the exact number of filling characters.*/
    return Ret;
}

UWORD CheckT30TxBuf(UWORD Channel)
{
    UWORD Ret =0;
/*Synway returns how many characters that T30/class1 can accept at specific channel indicated by "Channel"*/
    return Ret;
}
void SendCharToT30(UBYTE *ubDTE_Buf, UWORD uLen,UWORD Channel)
{
/*Synway puts those characters that store in ubDTE_Buf and size is uLen to T30,channel number is Channel.*/
}

void DTE_Functions(UWORD Channel)
{
/*Synway DTE Functions handler for channel*/
}

void Device_Close(void)
{
/*Synway releases/closes all other stuff */
}



void main()
{
    UWORD i;
    UWORD uLen, uLenT;



    for (i = 0; i < PCMSIZE; i++)
    {
        pPCMIn[i] = 0;
        pPCMOut[i] = 0;
    }

    for (i = 0; i < Modem_Channel_Number; i++)
    {
        pModemDataMem[i] = (UBYTE *)&ModemDataMem[i][0];
        ModemInit(pModemDataMem[i]);
        ModemStateInit(&Status[i]);
    }

    ESC_flag = 0;


    for (i = 0; i < Modem_Channel_Number; i++)
    {
        DataReady[i] = 0;
    }
    /*      low level driver and DTE device initialization here*/

    Driver_Initialize();


    while (ESC_flag == 0)
    {
        for (i = 0; i < Modem_Channel_Number; i++)
        {
  
            if (DataReady[i])

            {
            
                DataReady[i] = 0;
                HwIn(pPCMIn, i, Status[i].BufferSize);


//----------------- fax modem  DTE/T30 IN ---------------------------
                /*Get how many data fax modem can accept*/
                uLenT = CheckTxBuf(pModemDataMem[i]);

                if (uLenT > UBDTE_BUF_SIZE)
                    uLenT = UBDTE_BUF_SIZE;

                /*Get the data from DTE device*/
                /*return got data size, uLen<=uLenT*/
                if (uLenT > 0)
                    uLen = GetCharFrT30(ubDTE_Buf, uLenT, i);
                else
                    uLen = 0;
                   

                /*Send the data to fax modem*/
                if (uLen > 0)
                {
                    DTE_Transmit(pModemDataMem[i], ubDTE_Buf, uLen);
                }
//---------------------- fax modem -----------------------------------


                ModemMain(pModemDataMem[i], pPCMIn, pPCMOut);



                Modem_StatusChange(pModemDataMem[i], &Status[i]);

                if(Status[i].ModemState == GO_V17FAX)
                {
                    /*i channel fall back V17 processing*/
        			ModemInit(pModemDataMem[i]);
       				ModemStateInit(&Status[i]);
                }

//------------------- fax modem  DTE/T30 out -----------------------------------------
				/*if application detect a ring
				the modem status member RingDetected should be set to 1

				Status[i].RingDetected = 1;
				*/

                /*Get how many data T30 can accept*/
                uLenT = CheckT30TxBuf(i);


                if (uLenT > UBDTE_BUF_SIZE)
                    uLenT = UBDTE_BUF_SIZE;

                uLen = 0;

                /*Get the data from fax modem*/
                /*return got data size, uLen<=uLenT*/
                if (uLenT > 0)
                    uLen = DTE_Receive(pModemDataMem[i], ubDTE_Buf, uLenT);


                /*Send the data to terminal*/
                if (uLen > 0)
                {
                    SendCharToT30(ubDTE_Buf, uLen,i);
                }


                DTE_Functions(i);

                HwOut(pPCMOut,i,Status[i].BufferSize);
			
			}

		}
	}



}
