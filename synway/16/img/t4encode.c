/*****************************************************************************
*             Copyright (c) 1995 - 2007 by GAO Research Inc.                 *
*                          All rights reserved.                              *
*                                                                            *
*  This program is a confidential and an unpublished work of GAO Research    *
*  Inc. that is protected under international and Canadian copyright laws.   *
*  This program is a trade secret and constitutes valuable property of GAO   *
*  Research Inc. All use, reproduction and/or disclosure in whole or in      *
*  part of this program is strictly prohibited unless expressly authorized   *
*  in writing by GAO Research Inc.                                           *
*****************************************************************************/

/* ITU-T T.4 1D/2D encoder.
*/

#include "imgprodte.h"
#include "common.h"

#if SUPPORT_CONVERSION_BETWEEN_T4_T6


#include <string.h>
#include <stdio.h>
#include "t4ext.h"
#include "errorcode.h"

//////////////////////////////////////////
// Output Functions
//
// These functions are used to manage the
// output buffer

void T4Output_init(t4OutputStruct *pT4Output)
{
    pT4Output->numBitsFree = 8;
    pT4Output->numBitsOutput = 0;
    pT4Output->partialByte = 0;
}

void T4Output_lineSetup(t4OutputStruct *pT4Output, UBYTE *pOut, int isMMR)
{
    pT4Output->pOut = pOut;

    if (isMMR)
    {
        *pT4Output->pOut = pT4Output->partialByte;
        pT4Output->numBitsOutput = 8 - pT4Output->numBitsFree;
    }
    else
    {
        *pT4Output->pOut = 0;
        pT4Output->numBitsFree = 8;
        pT4Output->numBitsOutput = 0;
    }
}

// 'pattern' must be right aligned, and left padded with zeroes
void T4Output_codeWord(t4OutputStruct *pT4Output, UWORD codeWord, int numBitsCodeWord)
{
    int numBitsFree = pT4Output->numBitsFree;

    pT4Output->numBitsOutput += numBitsCodeWord;

    if (pT4Output->numBitsOutput > T4_MAX_OUTBIT_NUM)
    {
        return;    // This is an error. It will be reported when the output is finished
    }

    while (numBitsCodeWord >= numBitsFree)
    {
        numBitsCodeWord -= numBitsFree;
        *pT4Output->pOut++ |= (UBYTE)(codeWord >> numBitsCodeWord);        // numBitsPattern = numBitsPattern - numBitsFree
        codeWord &= ((UWORD)0xFFFF) >> (16 - numBitsCodeWord);
        *pT4Output->pOut = 0;
        numBitsFree = 8;
    }

    if (numBitsCodeWord > 0)
    {
        *pT4Output->pOut |= (UBYTE)(codeWord << (numBitsFree - numBitsCodeWord));
        numBitsFree -= numBitsCodeWord;
        //numBitsPattern = 0;
    }

    pT4Output->numBitsFree = numBitsFree;
}

int T4Output_lineFinish(t4OutputStruct *pT4Output, int isMMR, int isLSBfirst, int isEOP, int minLineBits)
{
    UBYTE *pOut = pT4Output->pOut;
    int numBitsFree = pT4Output->numBitsFree;
    int numBitsOutput = pT4Output->numBitsOutput;
    int numBytesOutput;
    int i;

    // Check for output buffer overflow condition
    if (numBitsOutput + 8 > T4_MAX_OUTBIT_NUM)
    {
        return ERROR_IMG_ENCODING_ERROR;
    }

    if (isEOP)
    {
        // Make output byte aligned and multiple of 8
        if (numBitsFree < 8)
        {
            pOut++;
            numBitsOutput += numBitsFree;
        }
    }
    else
    {
        if (isMMR)
        {
            // Store partial byte until next line
            // Output will be byte aligned and multiple of 8
            pT4Output->partialByte = *pOut;
            numBitsOutput -= 8 - numBitsFree;
            pT4Output->numBitsFree = numBitsFree;
        }
        else
        {
            // MH and MR line padding: zero fill to minimum scan line time
            // Also makes EOL alignment
            if (minLineBits + 8 > T4_MAX_OUTBIT_NUM)
            {
                return ERROR_IMG_ENCODING_ERROR;
            }

            while (numBitsOutput < minLineBits)
            {
                pOut++;
                numBitsOutput += numBitsFree;

                *pOut = 0;
                numBitsFree = 8;
            }

            // Make output byte aligned and multiple of 8
            if (numBitsFree < 8)
            {
                pOut++;
                numBitsOutput += numBitsFree;
            }
        }
    }

    // At this point:
    // numBitsOutput will be a multiple of 8
    // pOut will point to the empty byte after the data
    numBytesOutput = numBitsOutput >> 3;

    // Reverse bit order if necessary.
    if (isLSBfirst)
    {
        pOut -= numBytesOutput;

        for (i = 0; i < numBytesOutput; i++)
        {
            *pOut = COMM_BitReversal(*pOut);
            pOut++;
        }
    }

    return numBytesOutput;
}

/************************************************************************\
    t4EncodeRun()

    Description:
    This function gives codeword for a given running color. It is
    the core of 1-D encoding. To get complete codewords, the caller
    should keep calling this function with the same color and run
    length until it returns T4_CODEWORD_TERMINATING.
\************************************************************************/
void t4EncodeRun(int color, int runLength, t4OutputStruct *pT4Output)
{
    UWORD pattern;
    int numBits;
    int multiplesOf64;

    // Look for largest makeup codeword
    while (runLength >= MAX_LARGE_RUN)
    {
        pattern = (UWORD)t4EncoderPatternLarge[ENCODER_CODEBOOK_LENGTH_LARGE - 1];
        numBits = (int)t4EncoderShiftLarge[ENCODER_CODEBOOK_LENGTH_LARGE - 1];
        T4Output_codeWord(pT4Output, pattern, numBits);

        runLength -= MAX_LARGE_RUN;
    }

    multiplesOf64 = runLength >> 6;        // divide by 64
    runLength -= multiplesOf64 << 6;    // subtract nultiple of 64 from run length
    // Has no effect on terminating run because mutlipleOf64 = 0

    // Look for multiple of 64 codewords
    if (multiplesOf64 >= MIN_LARGE_MULTIPLE)
    {
        pattern = (UWORD)t4EncoderPatternLarge[multiplesOf64 - MIN_LARGE_MULTIPLE];
        numBits = (int)t4EncoderShiftLarge[multiplesOf64 - MIN_LARGE_MULTIPLE];
        T4Output_codeWord(pT4Output, pattern, numBits);
    }
    else if (multiplesOf64 > 0)
    {
        if (color == WHITE)
        {
            pattern = (UWORD)t4EncoderPatternWhiteMedium[multiplesOf64 - 1];
            numBits = (int)t4EncoderShiftWhiteMedium[multiplesOf64 - 1];
        }
        else
        {
            pattern = (UWORD)t4EncoderPatternBlackMedium[multiplesOf64 - 1];
            numBits = (int)t4EncoderShiftBlackMedium[multiplesOf64 - 1];
        }

        T4Output_codeWord(pT4Output, pattern, numBits);
    }

    // Terminating codeword
    if (color == WHITE)
    {
        pattern = (UWORD)t4EncoderPatternWhiteSmall[runLength];
        numBits = (int)t4EncoderShiftWhiteSmall[runLength];
    }
    else
    {
        pattern = (UWORD)t4EncoderPatternBlackSmall[runLength];
        numBits = (int)t4EncoderShiftBlackSmall[runLength];
    }

    T4Output_codeWord(pT4Output, pattern, numBits);
}




/************************************************************************\
    t4Encoder_init()

    Description:
    T.4 1D encoding initialization function. isLSBfirst is a Boolean variable,
    which indicates the bit order in the output data buffer when encoder
    runs. If isLSBfirst is set to 1, in the output BYTE array, the LSB of
    each byte is the first bit (among all bits in that BYTE) that should
    be transferred on the line. Otherwise, MSB is the first bit.
    Return:
    0:        Success
    Other:    Error (T4_PAGEWIDTH_TOO_WIDE)
\************************************************************************/

int t4Encoder_init(t4EncoderStruct *pT4Encoder, int pageWidth, int isLSBfirst, int compressionMode, int parameterK, int minimumScanLineBits, UWORD *pReferenceLine)
{
    t4EncoderStruct *pEnc = (t4EncoderStruct *) pT4Encoder;

    T4Output_init(&pEnc->t4Output);

    pEnc->pageWidth = pageWidth;
    pEnc->minimumScanLineBits = minimumScanLineBits;
    pEnc->isLSBfirst = (isLSBfirst) ? 1 : 0;
    *pReferenceLine = pageWidth;

    if (compressionMode == T4_1D_MH)
    {
        pEnc->compressionMode = T4_1D_MH;
    }
    else if (compressionMode == T4_2D_MR)
    {
        pEnc->compressionMode = T4_2D_MR;
        pEnc->lineIdx = 0;
        pEnc->K = parameterK;
    }
    else if (compressionMode == T6_2D_MMR)
    {
        pEnc->compressionMode = T6_2D_MMR;
    }
    else
    {
        return T4_COMPRESSION_MODE_NOT_SUPPORTED;
    }

    return 0;
}


/************************************************************************\
    t4Encoder_end()

    Description:
    T.4 1D encoding terminating function. This function MUST be called after
    one page image data is finished encoding. The output buffer will be
    filled in with RTC sequence.
    Return:
    Valid output BYTE number in output array.
\************************************************************************/
int t4Encoder_end(t4EncoderStruct *pT4Encoder, UBYTE *pOut)
{
    t4EncoderStruct *pEnc = pT4Encoder;
    t4OutputStruct *pT4Output = &pEnc->t4Output;
    int i;

    T4Output_lineSetup(pT4Output, pOut, pEnc->compressionMode == T6_2D_MMR);

    if (pEnc->compressionMode == T4_1D_MH)
    {
        for (i = 0; i < 6; i++)
        {
            T4Output_codeWord(pT4Output, 0x1, 12);
        }
    }
    else if (pEnc->compressionMode == T4_2D_MR)
    {
        for (i = 0; i < 6; i++)
        {
            T4Output_codeWord(pT4Output, 0x3, 13);
        }
    }
    else if (pEnc->compressionMode == T6_2D_MMR)
    {
        for (i = 0; i < 2; i++)
        {
            T4Output_codeWord(pT4Output, 0x1, 12);
        }
    }

    return T4Output_lineFinish(pT4Output, pEnc->compressionMode == T6_2D_MMR, pEnc->isLSBfirst, 1, pEnc->minimumScanLineBits);
}



/************************************************************************\
    t4Encoder_apply()

    Description:
    T.4 and T.6 encoding function. Input is one line image data. Note that each
    data bit should be stored in each byte of this input array. The array
    length should not be less than pageWidth.
    Return:
    positive:        valid output BYTE number in output array.
    negative:        Error.
\************************************************************************/
int t4Encoder_apply(t4EncoderStruct *pT4Encoder, UWORD *pLine, UWORD *pReferenceLine, UBYTE *pOut)
{
    t4EncoderStruct *pEnc = pT4Encoder;
    t4OutputStruct *pT4Output = &pEnc->t4Output;
    twoDStruct *p2D = &pEnc->twoD;
    int color;
    int lengthTemp;
    int a1, a2, runIndex;
    UWORD lineLength;


    T4Output_lineSetup(pT4Output, pOut, pEnc->compressionMode == T6_2D_MMR);

    if (pEnc->compressionMode == T4_2D_MR)
    {
        pEnc->lineIdx++;

        if (pEnc->lineIdx > pEnc->K)
        {
            pEnc->lineIdx = 1;
        }
    }

    // Start encoding Line
    color = WHITE;
    lineLength = 0;
    runIndex = 0;

    if ((pEnc->compressionMode == T4_1D_MH) || ((pEnc->compressionMode == T4_2D_MR) && (pEnc->lineIdx == 1)))
    {
        //////////////////
        // 1-D encoding //
        //////////////////

        // EOL at beginning of the line
        if (pEnc->compressionMode == T4_2D_MR)
        {
            T4Output_codeWord(pT4Output, 0x3, 13);
        }
        else
        {
            T4Output_codeWord(pT4Output, 0x1, 12);
        }

        while (lineLength < pEnc->pageWidth)
        {
            lineLength += pLine[runIndex];
            t4EncodeRun(color, pLine[runIndex++], pT4Output);
            color = !color;
        }
    }
    else
    {
        //////////////////
        // 2-D encoding //
        //////////////////

        T4T6_reset2D(p2D);

        a1 = 1;
        p2D->pReferenceLine = pReferenceLine; // Allow T4T6_calculateB1B2 to get reference line without passing it down through all the calls

        if (pEnc->compressionMode == T4_2D_MR)
        {
            // Insert EOL+0 first.
            T4Output_codeWord(pT4Output, 0x2, 13);
        }

        while (p2D->a0 <= pEnc->pageWidth)
        {
            a1 += pLine[runIndex++];

            // Find b1 and b2
            T4T6_calculateB1B2(p2D, pEnc->pageWidth);


            while (p2D->b2 < a1)
            {
                // P mode Codeword: 0001
                T4Output_codeWord(pT4Output, 0x1, 4);
                p2D->a0 = p2D->b2;
                T4T6_calculateB1B2(p2D, pEnc->pageWidth);
            }


            // a1b1
            lengthTemp = (a1 - p2D->b1);

            if (lengthTemp >= -3 && lengthTemp <= 3)
            {
                /* V mode */
                T4Output_codeWord(pT4Output, (UWORD)vModePattern[lengthTemp + 3], (int)vModeNumBits[lengthTemp + 3]);
                p2D->a0 = a1;
                p2D->a0Color = !p2D->a0Color;
            }
            else
            {
                // H mode
                if (a1 > pEnc->pageWidth)
                {
                    a2 = a1;
                }
                else
                {
                    a2 = a1 + pLine[runIndex++];
                }

                // H mode codeword 001 prefix
                T4Output_codeWord(pT4Output, 0x1, 3);

                // 1D run a0->a1
                lengthTemp = ((p2D->a0) ? (a1 - p2D->a0) : (a1 - p2D->a0 - 1));
                t4EncodeRun(p2D->a0Color, lengthTemp, pT4Output);

                // 1D run a1->a2
                // Note that second run is opposite color
                // and overall color does not change after H mode
                t4EncodeRun(!p2D->a0Color, a2 - a1, pT4Output);

                p2D->a0 = a2;
                a1 = a2;
            }
        }
    }

    return T4Output_lineFinish(pT4Output, pEnc->compressionMode == T6_2D_MMR, pEnc->isLSBfirst, 0, pEnc->minimumScanLineBits);
}


#endif

