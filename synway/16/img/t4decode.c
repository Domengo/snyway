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

/* ITU-T T.4 1D/2D decoder.
*/
#include "imgprodte.h"

#if SUPPORT_CONVERSION_BETWEEN_T4_T6

#include "t4ext.h"
#include <string.h>
#include <stdio.h>

// Resets the MH component of the decoder for use in MR and MMR decoding.
// Specify the color to start with for H mode (new lines must be WHITE)
void T4T6_reset1D(t4DecoderStruct *pT4Decoder, int color)
{
    t4DecoderStruct *pDec = pT4Decoder;

    pDec->treeIndex = 0;
    pDec->isLineError = 0;

    if (color == BLACK)
    {
        pDec->color = BLACK;
        pDec->tree = t4DecoderTreeBlack;
    }
    else
    {
        pDec->color = WHITE;
        pDec->tree = t4DecoderTreeWhite;
    }
}

void T4T6_reset2D(twoDStruct *p2D)
{
    // Initialize for new 2D line.
    p2D->a0 = 0;
    p2D->b1 = 1;
    p2D->a0Color = WHITE;
    p2D->b1Color = WHITE;
    p2D->b1Index = 0;
}

/*************************************************************************************\
    T.4 Decoder Structure Initialization

    This function must be called on a T.4 2D decoder structure before it is
    used in any other functions. The state is initialized to the beginning
    of a line or page. MH starts off with WHITE.

    Inputs:
        pT4Decoder:    Pointer to t4Decoder2DStruct for particular channel
    Return:
        0:                Initialization successful

\*************************************************************************************/

int t4Decoder_init(t4DecoderStruct *pT4Decoder, int pageWidth, int compressionMode, UWORD *pReferenceLine)
{
    t4DecoderStruct *pDec = pT4Decoder;
    twoDStruct *p2D = &pDec->twoD;

    *pReferenceLine = (UWORD) pageWidth;    // set 'first' line to be white run of pagewidth

    pDec->pageWidth = pageWidth;
    pDec->numZeros = 0;
    pDec->runLengthAccumulated = 0;
    pDec->runIndex = 0;
    pDec->numEOL = 0;
    pDec->isLineError = 0;
    pDec->compressionMode = compressionMode;

    switch (compressionMode)
    {
        case T4_1D_MH:
        case T4_2D_MR:
            pDec->isFirstEOL = 1;
            pDec->pFnProcess = T4T6_detectEOL;
            pDec->lineLengthCurrent = pageWidth;    // Prevent bad line from being indicated in first EOL
            break;
        case T6_2D_MMR:
            //f = fopen("t6dump.txt", "w");
            pDec->isFirstEOL = 0;        // Bypass looking for first EOL
            pDec->pFnProcess = T4T6_decodeLine2D;
            pDec->lineLengthCurrent = 0;
            T4T6_reset2D(p2D);
            break;
        default:
            return T4_COMPRESSION_MODE_NOT_SUPPORTED;
    }

    return 0;
}




/*************************************************************************************\
    T.4 Bitwise decoder.

    Inputs:
        t4Decoder:    Pointer to t4DecoderStruct for particular channel
        bit:        Next bit to decode (0..1)
    Outputs:
        runLength:    If bit finishes a codeword, length of decoded run
        color:        If bit finishes a codeword, color of decoded run (WHITE or BLACK)
    Return:
        T4_DECODER_ERROR    Bit not valid. Error in decoder. Output not valid.
                            Application pads missing pixel until new code word generated.
        T4_DECODER_CONTINUE    Bit does not finish a codeword, outputs not valid
        T4_DECODER_CODEWORD    Bit finishes a codeword, outputs valid
        T4_DECODER_EOL        Bit finishes EOL, outputs not valid
        T4_DECODER_RTC        Bit finishes RTC, outputs not valid

\*************************************************************************************/

int T4T6_decodeLine1D(t4DecoderStruct *pT4Decoder, int bit, int *pRunLength, int *pColor)
{
    t4DecoderStruct *pDec = (t4DecoderStruct *) pT4Decoder;

    // Count number of consecutive zeros for EOL detection
    if (bit)
    {
        if (pDec->numZeros >= T4_MIN_ZEROS_IN_EOL)            // EOL
        {
            return T4_DECODER_EOL;
        }
        else if (pDec->numZeros > T4_MAX_ZEROS_IN_CODEWORD)    // Error: Invalid number of zeros
        {
            return T4_DECODER_ERROR;
        }
    }
    else
    {
        if (pDec->numZeros > T4_MAX_ZEROS_IN_CODEWORD)
        {
            return T4_DECODER_CONTINUE;    // Beyond valid codewords. Extension or EOL so do not process codewords
        }
    }

    // Regular codeword parsing
    pDec->treeIndex += bit;
    pDec->treeIndex = pDec->tree[pDec->treeIndex];

    if (pDec->treeIndex > 0)
    {
        // Next offset found. Wait for next bit
        return T4_DECODER_CONTINUE;
    }
    else if (pDec->treeIndex == T4_TREE_NEXT)
    {
        // Move to tree with large length runs
        pDec->treeIndex = 0;
        pDec->tree = t4DecoderTreeLarge;

        return T4_DECODER_CONTINUE;
    }
    else
    {
        // Reached the end of the codeword
        *pRunLength = -(pDec->treeIndex);
        *pColor = pDec->color;

        pDec->treeIndex = 0;

        // resume tree after large tree search.
        pDec->tree = (pDec->color == WHITE) ? t4DecoderTreeWhite : t4DecoderTreeBlack;

        if (*pRunLength <= T4_MAX_TERMINATING_CODEWORD_LENGTH)
        {
            //change colors after terminating codeword
            if (pDec->color == WHITE)
            {
                pDec->color = BLACK;
                pDec->tree = t4DecoderTreeBlack;
            }
            else
            {
                pDec->color = WHITE;
                pDec->tree = t4DecoderTreeWhite;
            }

            return T4_DECODER_CODEWORD;
        }

        return T4_DECODER_MAKEUP_CODEWORD;
    }
}


//////////////////////////////
// 1D Functions (MH and MR)

// Detect the initial EOL to start MH and MR pages, or find EOL after error
// Ignores data before valid EOL.
int T4T6_detectEOL(t4DecoderStruct *pDec, int bit, int *pRunLength, int *pColor)
{
    if (bit) // end of zeros detected.
    {
        if (pDec->numZeros >= T4_MIN_ZEROS_IN_EOL)
        {
            return T4_DECODER_EOL;
        }
    }

    return T4_DECODER_CONTINUE;
}

int T4T6_decodeEOLtag(t4DecoderStruct *pDec, int bit, int *pRunLength, int *pColor)
{
    // detect EOL tag
    if (bit)
    {
        // EOL + 1: 1D encoded line or RTC
        T4T6_reset1D(pDec, WHITE);
        pDec->pFnProcess = T4T6_decodeLine1D;
    }
    else
    {
        // EOL + 0: 2D encoded line
        T4T6_reset2D(&pDec->twoD);

        if (pDec->isLineError)
        {
            pDec->pFnProcess = T4T6_detectEOL;    // Back to looking for EOL
        }
        else
        {
            pDec->pFnProcess = T4T6_decodeLine2D;    // No error, so decode line
        }

        pDec->numEOL = 0;
        pDec->numZeros = 0;
    }

    return T4_DECODER_CONTINUE;
}


//////////////////////////////////////////
// 2D Functions (MR and MMR)

int T4T6_decodeHmode(t4DecoderStruct *pDec, int bit, int *pRunLength, int *pColor)
{
    twoDStruct *p2D = &pDec->twoD;
    int returnValue = T4T6_decodeLine1D(pDec, bit, pRunLength, pColor);

    switch (returnValue)
    {
        case T4_DECODER_CONTINUE:
            break;
        case T4_DECODER_CODEWORD:
            p2D->numCodeword++;
            p2D->Hrun += *pRunLength;
            //fprintf(f, "[%d%c]", *pRunLength, *pColor == WHITE ? 'W' : 'B');

            if (p2D->numCodeword >= 2)
            {
                //fprintf(f, "\n");
                // H mode finished.
                pDec->pFnProcess = T4T6_decodeLine2D;

                /*
                // Error check
                if (*pColor == pDec->a0Color)
                    return T4_DECODER_ERROR;
                */
                // Why is it enabled for MMR and not MR?
                if (pDec->compressionMode == T6_2D_MMR)
                {
                    if (*pColor == p2D->a0Color)
                    {
                        //fclose(f);
                        returnValue = T4_DECODER_ERROR;
                    }
                }

                p2D->a0 = ((p2D->a0) ? (p2D->a0) : 1);
                p2D->a0 += p2D->Hrun;
            }

            break;
        case T4_DECODER_MAKEUP_CODEWORD:
            p2D->Hrun += *pRunLength;
            break;
        default:
            returnValue = T4_DECODER_ERROR;
    }

    return returnValue;
}

int T4T6_decodeVmode(t4DecoderStruct *pDec, int bit, int *pRunLength, int *pColor)
{
    twoDStruct *p2D = &pDec->twoD;
    int a1;

    T4T6_calculateB1B2(p2D, pDec->pageWidth);
    *pColor = p2D->a0Color;
    a1 = ((bit) ? (p2D->b1 + p2D->vx) : (p2D->b1 - p2D->vx));
    *pRunLength = ((p2D->a0) ? (a1 - p2D->a0) : (a1 - 1));
    p2D->a0 = a1;
    p2D->a0Color = !(p2D->a0Color);
    pDec->pFnProcess = T4T6_decodeLine2D;
    //fprintf(f, "V(%c%d)[%d%c]\n", bit ? '+' : '-', p2D->vx, *pRunLength, *pColor == WHITE ? 'W' : 'B');
    return T4_DECODER_CODEWORD;
}

int T4T6_decodeLine2D(t4DecoderStruct *pDec, int bit, int *pRunLength, int *pColor)
{
    twoDStruct *p2D = &pDec->twoD;

    if (bit)
    {
        // non-zero found
        switch (pDec->numZeros)
        {
            case 0:
                // V(0)
                T4T6_calculateB1B2(p2D, pDec->pageWidth);
                *pColor = p2D->a0Color;
                *pRunLength = ((p2D->a0) ? (p2D->b1 - p2D->a0) : (p2D->b1 - 1));
                p2D->a0 = p2D->b1;
                p2D->a0Color = !(p2D->a0Color);
                //fprintf(f, "V(0)[%d%c]\n", *pRunLength, *pColor == WHITE ? 'W' : 'B');
                return T4_DECODER_CODEWORD;
            case 1:
                // V(1)
                p2D->vx = 1;
                pDec->pFnProcess = T4T6_decodeVmode;
                return T4_DECODER_CONTINUE;
            case 2:
                // H mode
                T4T6_reset1D(pDec, p2D->a0Color);
                p2D->numCodeword = 0;
                p2D->Hrun = 0;
                pDec->pFnProcess = T4T6_decodeHmode;
                //fprintf(f, "H");
                return T4_DECODER_CONTINUE;
            case 3:
                // P mode
                T4T6_calculateB1B2(p2D, pDec->pageWidth);
                *pColor = p2D->a0Color;
                *pRunLength = ((p2D->a0) ? (p2D->b2 - p2D->a0) : (p2D->b2 - 1));
                p2D->a0 = p2D->b2;
                //fprintf(f, "PM[%d%c]\n", *pRunLength, *pColor == WHITE ? 'W' : 'B');
                return T4_DECODER_MAKEUP_CODEWORD;// Doesn't switch color on P mode, so rest of run will follow.
            case 4:
                // V(2)
                p2D->vx = 2;
                pDec->pFnProcess = T4T6_decodeVmode;
                return T4_DECODER_CONTINUE;
            case 5:
                // V(3)
                p2D->vx = 3;
                pDec->pFnProcess = T4T6_decodeVmode;
                return T4_DECODER_CONTINUE;
            default:

                // Fuzzy EOL detection.
                // MR EOL must have 1D tag,
                // MMR EOL must be == T4_MIN_ZEROS_IN_EOL, not >= T4_MIN_ZEROS_IN_EOL
                if (pDec->numZeros >= T4_MIN_ZEROS_IN_EOL)
                {
                    return T4_DECODER_EOL;
                }
        }
    }

    return T4_DECODER_CONTINUE;
}



/* Before call this function, the reference line must be filled in. */
/* This function is shared by encoder and decoder */
void T4T6_calculateB1B2(twoDStruct *p2D, int pageWidth)
{
    UWORD *pRef = p2D->pReferenceLine;
    int a0        = p2D->a0;
    int a0Color = p2D->a0Color;
    int b1        = p2D->b1;
    int b1Color = p2D->b1Color;
    int b1Index = p2D->b1Index;

    // Back up to make sure b1 starts at a0 + 1 location
    // In case where a0 changes color but doesn't reach b1,
    // location of b1 can go backwards
    while (b1 > (a0 + 1))
    {
        b1Index--;
        b1 -= pRef[b1Index];
        b1Color = !b1Color;
    }

    while (((b1 <= a0) || (a0Color == b1Color)) && (b1 <= pageWidth))
    {
        b1 += pRef[b1Index++];
        b1Color = !b1Color;
    }

    if (b1 >= pageWidth)
    {
        p2D->b2 = b1 + 1;
    }
    else
    {
        p2D->b2 = b1 + pRef[b1Index];
    }

    p2D->b1 = b1;
    p2D->b1Color = b1Color;
    p2D->b1Index = b1Index;
}

/*************************************************************************************\
    T.4/T.6 Bitwise decoder.

    Inputs:
        t4Decoder2D:    Pointer to t4Decoder2DStruct for particular channel
        bit:            Next bit to decode (0..1)
    Outputs:
        runLength:        If bit finishes a codeword, length of decoded run
        color:            If bit finishes a codeword, color of decoded run (WHITE or BLACK)
    Return:
        T4_DECODER_ERROR    Bit not valid. Error in decoder. Output not valid.
                            Application pads missing pixel until new code word generated.
        T4_DECODER_CONTINUE    Bit does not finish a codeword, outputs not valid
        T4_DECODER_CODEWORD    Bit finishes a codeword, outputs valid
        T4_DECODER_EOL        Bit finishes EOL, outputs not valid
        T4_DECODER_RTC        Bit finishes RTC, outputs not valid

\*************************************************************************************/

int t4Decoder_apply(t4DecoderStruct *pT4Decoder, int bit, UWORD *pLine, UWORD *pReferenceLine)
{
    t4DecoderStruct *pDec = (t4DecoderStruct *) pT4Decoder;
    twoDStruct *p2D = &pDec->twoD;
    int returnValue;
    int isError = 0;
    int runLength, color;

    if (bit == 0)
    {
        pDec->numZeros++;
    }
    else
    {
        bit = 1;    // Bit must be 0 or 1 after this point
    }

    p2D->pReferenceLine = pReferenceLine;

    returnValue = pDec->pFnProcess(pDec, bit, &runLength, &color);

    if (bit != 0)
    {
        pDec->numZeros = 0;
    }

    switch (returnValue)
    {
        case T4_DECODER_ERROR:
            isError = 1;
            pDec->numEOL = 0;
            returnValue = T4_DECODER_CONTINUE;
            break;

        case T4_DECODER_MAKEUP_CODEWORD:
            // Non-terminating code word for 1D. Accumulate total run length of same color runs
            pDec->numEOL = 0;
            pDec->runLengthAccumulated += runLength;
            pDec->numZeros = 0;
            returnValue = T4_DECODER_CONTINUE;
            break;

        case T4_DECODER_CODEWORD:
            // Terminating Code Word: Save decoded data in buffer for reference
            pDec->numEOL = 0;
            pDec->numZeros = 0;
            runLength += pDec->runLengthAccumulated;
            pDec->runLengthAccumulated = 0;

            if (runLength < 0)    // Caused by errors being interpretted by 2D algorithms
            {
                isError = 1;
                break;
            }

            // Check for line overflows
            if (pDec->lineLengthCurrent + runLength <= pDec->pageWidth)
            {
                pDec->lineLengthCurrent += runLength;
                pLine[pDec->runIndex++] = (UWORD) runLength;
            }
            else
            {
                isError = 1;
                break;
            }

            // Check if end of line for MMR
            if (!((pDec->compressionMode == T6_2D_MMR) && (pDec->lineLengthCurrent >= pDec->pageWidth) && (pDec->pFnProcess != T4T6_decodeHmode)))
            {
                break;
            }

            // Setup for EOL handling of MMR
            returnValue = T4_DECODER_EOL;    // Make sure that EOL is ready to be returned

        case T4_DECODER_EOL:
            pDec->numEOL++;

            if ((pDec->numEOL == T4_NUM_EOL_IN_RTC) && (pDec->compressionMode != T6_2D_MMR))
            {
                returnValue = T4_DECODER_RTC;        // RTC
            }
            else if ((pDec->numEOL == T4_NUM_EOL_IN_EOFB) && (pDec->compressionMode == T6_2D_MMR))
            {
                if (pDec->isLineError)
                {
                    returnValue = T4_DECODER_EOFB_BAD;    // EOFB with page errors
                }
                else
                {
                    returnValue = T4_DECODER_EOFB;    // EOFB
                }
            }
            else if (pDec->numEOL == 1)
            {
                // Process line for application

                // Check for line underflow
                if (pDec->lineLengthCurrent < pDec->pageWidth)
                {
                    isError = 1;

                    if (pDec->runIndex & 0x01)
                    {
                        // Current color is black. Increase previous white run to end of line
                        pLine[pDec->runIndex - 1] += (UWORD)(pDec->pageWidth - pDec->lineLengthCurrent);
                    }
                    else
                    {
                        // Current color is white. Make white run to end of line
                        pLine[pDec->runIndex] = (UWORD)(pDec->pageWidth - pDec->lineLengthCurrent);
                    }
                }

                pDec->runIndex = 0;
                pDec->lineLengthCurrent = 0;
                pDec->numZeros = 0;
            }

            break;
    }

    if (isError)
    {
        pDec->isLineError = 1;
        pDec->pFnProcess = T4T6_detectEOL;
    }

    if (returnValue == T4_DECODER_EOL)
    {
        if (pDec->isLineError)
        {
            returnValue = T4_DECODER_EOL_BAD_LINE;
        }

        switch (pDec->compressionMode)
        {
            case T4_1D_MH:
                T4T6_reset1D(pDec, WHITE);
                pDec->pFnProcess = T4T6_decodeLine1D;
                break;
            case T4_2D_MR:
                // Reset postponed until tag decoding
                pDec->pFnProcess = T4T6_decodeEOLtag;
                break;
            case T6_2D_MMR:
                T4T6_reset2D(p2D);

                if (pDec->isLineError)
                {
                    pDec->pFnProcess = T4T6_detectEOL;
                }
                else
                {
                    pDec->pFnProcess = T4T6_decodeLine2D;
                }

                break;
        }

        if (pDec->isFirstEOL)
        {
            pDec->isFirstEOL = 0;
            returnValue = T4_DECODER_CONTINUE;
        }

        if (pDec->numEOL > 1)
        {
            returnValue = T4_DECODER_CONTINUE;
        }

    }

    return returnValue;
}

#endif

