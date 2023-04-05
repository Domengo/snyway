
#include "imgprodte.h"

#if SUPPORT_CONVERSION_BETWEEN_T4_T6
#include "dsmdef.h"
#include "scaling.h"

extern CONST int XResoValue_Tiff[9];
extern CONST int YResoValue_Tiff[10];

const PaperSizeRatioStruct X_Ratio_A4_B4 = {   5, 6, 5 };    // A4 <-> B4: Add 5 pixels/27 pixels for each 27 to get 32
const PaperSizeRatioStruct X_Ratio_A4_A3 = {   2, 3, 11};    // A4 <-> A3: Add 11 pixels/27 pixels for each 27 to get 38
const PaperSizeRatioStruct X_Ratio_B4_A3 = {   5, 6, 3 };    // B4 <-> A3: Add 3 pixels/16 pixels for each 16 to get 19

const PaperSizeRatioStruct Y_Ratio_A4_B4 = {   4, 5, 2 };    // A4 <-> B4:
const PaperSizeRatioStruct Y_Ratio_A4_A3 = {   2, 3, 2};    // A4 <-> A3:
const PaperSizeRatioStruct Y_Ratio_B4_A3 = {   6, 7, 2 };    // B4 <-> A3:


void PaperScaleInit(ScalerStruct *pScaler, UBYTE PaperSizeIn, UBYTE PaperSizeOut)
{
    switch (PaperSizeIn)
    {
        case DSM_A4_LETTER_LEGAL:

            switch (PaperSizeOut)
            {
                case DSM_A4_LETTER_LEGAL:
                    pScaler->PaperScale = IMG_SCALE_NONE;
                    break;
                case DSM_B4:
                    pScaler->pPaperRatioX = &X_Ratio_A4_B4;
                    pScaler->pPaperRatioY = &Y_Ratio_A4_B4;
                    pScaler->PaperScale = IMG_SCALE_UP;
                    break;
                case DSM_A3:
                    pScaler->pPaperRatioX = &X_Ratio_A4_A3;
                    pScaler->pPaperRatioY = &Y_Ratio_A4_A3;
                    pScaler->PaperScale = IMG_SCALE_UP;
                    break;
                default:
                    TRACE1("IMG: Scaler out paper size Error1!, %d", PaperSizeOut);
                    break;
            }

            break;
        case DSM_B4:

            switch (PaperSizeOut)
            {
                case DSM_A4_LETTER_LEGAL:
                    pScaler->pPaperRatioX = &X_Ratio_A4_B4;
                    pScaler->pPaperRatioY = &Y_Ratio_A4_B4;
                    pScaler->PaperScale = IMG_SCALE_DOWN;
                    break;
                case DSM_B4:
                    pScaler->PaperScale = IMG_SCALE_NONE;
                    break;
                case DSM_A3:
                    pScaler->pPaperRatioX = &X_Ratio_B4_A3;
                    pScaler->pPaperRatioY = &Y_Ratio_B4_A3;
                    pScaler->PaperScale = IMG_SCALE_UP;
                    break;
                default:
                    TRACE1("IMG: Scaler out paper size Error2!, %d", PaperSizeOut);
                    break;
            }

            break;
        case DSM_A3:

            switch (PaperSizeOut)
            {
                case DSM_A4_LETTER_LEGAL:
                    pScaler->pPaperRatioX = &X_Ratio_A4_A3;
                    pScaler->pPaperRatioY = &Y_Ratio_A4_A3;
                    pScaler->PaperScale = IMG_SCALE_DOWN;
                    break;
                case DSM_B4:
                    pScaler->pPaperRatioX = &X_Ratio_B4_A3;
                    pScaler->pPaperRatioY = &Y_Ratio_B4_A3;
                    pScaler->PaperScale = IMG_SCALE_DOWN;
                    break;
                case DSM_A3:
                    pScaler->PaperScale = IMG_SCALE_NONE;
                    break;
                default:
                    TRACE1("IMG: Scaler out paper size Error3!, %d", PaperSizeOut);
                    break;
            }

            break;
        default:
            TRACE1("IMG: Scaler in paper size Error!, %d", PaperSizeIn);
            break;
    }
}


SWORD XResolutionConversion(SWORD XRes)
{
    SWORD Rev = 0;

    if (XRes == R8_X_RESOLUTION)
    {
        XRes = X_RESOLUTION_200DPI;
    }
    else if (XRes == R16_X_RESOLUTION)
    {
        XRes = X_RESOLUTION_400DPI;
    }

    if ((XRes >= X_RESOLUTION_200DPI) && (XRes <= X_RESOLUTION_1200DPI))
    {
        Rev = (UBYTE)((XResoValue_Tiff[XRes - 1]) / 100);
    }
    else
    {
        TRACE1("IMG: Scaler X Resolution in Error!, %d", XRes);
    }

    return Rev;
}

SWORD YResolutionConversion(SWORD YRes)
{
    SWORD Rev = 0;

    if (YRes == VR_NORMAL_385)
    {
        YRes = VR_100L_INCH;
    }
    else if (YRes == VR_FINE_77)
    {
        YRes = VR_200L_INCH;
    }
    else if (YRes == VR_PERFECT_154)
    {
        YRes = VR_400L_INCH;
    }

    if ((YRes >= VR_100L_INCH) && (YRes <= VR_1200L_INCH))
    {
        Rev = (UBYTE)((YResoValue_Tiff[YRes - 1]) / 100);
    }
    else
    {
        TRACE1("IMG: Scaler Y Resolution in Error!, %d", YRes);
    }

    return Rev;
}


void GenResolutionRatio(ResolutionRatioStrcut *pResoRatio, SWORD ResLow, SWORD ResHigh)
{
    SWORD Quotient, Remainder;

    memset(pResoRatio, 0, sizeof(ResolutionRatioStrcut));

    Quotient = (ResHigh - ResLow) / ResLow;
    Remainder = (ResHigh - ResLow) % ResLow;

    pResoRatio->AddLen1 = Quotient;
    pResoRatio->LoopTimes1 = Remainder;
    pResoRatio->AddLen2 = Quotient;
    pResoRatio->LoopTimes2 = ResLow - Remainder;

    if (Remainder != 0)
    {
        pResoRatio->AddLen1 += 1;
    }

}

UBYTE ResoScaleCalculate(ResolutionRatioStrcut *pResoRatio, SWORD ResIn, SWORD ResOut)
{
    UBYTE Rev = 0;

    if ((ResIn > 0) && (ResOut > 0))
    {
        if (ResIn > ResOut)
        {
            GenResolutionRatio(pResoRatio, ResOut, ResIn);
            Rev = IMG_SCALE_DOWN;
        }
        else if (ResIn < ResOut)
        {
            GenResolutionRatio(pResoRatio, ResIn, ResOut);
            Rev = IMG_SCALE_UP;
        }
        else
        {
            memset(pResoRatio, 0, sizeof(ResolutionRatioStrcut));
            Rev = IMG_SCALE_NONE;
        }
    }

    return Rev;
}

void ResoScaleInit(ScalerStruct *pScaler, SWORD XResIn, SWORD YResIn, SWORD XResOut, SWORD YResOut)
{
    ResolutionRatioStrcut *pResoRatioX = &(pScaler->ResoRatioX);
    ResolutionRatioStrcut *pResoRatioY = &(pScaler->ResoRatioY);

    XResIn = XResolutionConversion(XResIn);
    XResOut = XResolutionConversion(XResOut);
    pScaler->ResoScaleX = ResoScaleCalculate(pResoRatioX, XResIn, XResOut);

    YResIn = YResolutionConversion(YResIn);
    YResOut = YResolutionConversion(YResOut);
    pScaler->ResoScaleY = ResoScaleCalculate(pResoRatioY, YResIn, YResOut);
}

SWORD GetScanLineLengthInPixels(UBYTE PaperSize, SWORD XRes)
{
    SWORD Rev = 0;

    PaperSize -= 1;

    if (PaperSize == 3)
    {
        PaperSize = 2;
    }

    XRes -= 2;

    if (((PaperSize >= 0) && (PaperSize <= 2)) && ((XRes >= 0) && (XRes <= 7)))
    {
        Rev = (SWORD)(ImageWidth_Tiff[PaperSize][XRes]);
    }
    else
    {
        TRACE0("IMG: PaperSizeIn or XResIn Error!");
    }

    return Rev;
}

int SCALER_init(ScalerStruct *pScaler, TImgFormat *pImgIn, TImgFormat *pImgOut)
{

    memset(&(pScaler->ResoRatioX), 0, sizeof(ResolutionRatioStrcut));
    memset(&(pScaler->ResoRatioY), 0, sizeof(ResolutionRatioStrcut));

    pScaler->LineCntBefore = 0;
    pScaler->PaperScale = 0;
    pScaler->ResoScaleX = 0;
    pScaler->ResoScaleY = 0;
    pScaler->WidthXBefore = GetScanLineLengthInPixels(pImgIn->PaperSize, pImgIn->XRes);
    pScaler->WidthXAfter = GetScanLineLengthInPixels(pImgIn->PaperSize, pImgOut->XRes);
    PaperScaleInit(pScaler, pImgIn->PaperSize, pImgOut->PaperSize);
    ResoScaleInit(pScaler, pImgIn->XRes, pImgIn->YRes, pImgOut->XRes, pImgOut->YRes);

    return SCALER_INIT_SUCCESS;
}



void ImgScaleResolutionX(ScalerStruct *pScaler, UWORD *pLineIn, UWORD *pLineOut)
{
    ResolutionRatioStrcut *pResoRatioX = &(pScaler->ResoRatioX);
    int runIndexIn = 0, runIndexOut = 0, lineLength = 0, AddLen = 0, number = 0;
    int NextStart = 0, Len1, Len2;
    UWORD runLengthOut;
    int i;

    switch (pScaler->ResoScaleX)
    {
        case IMG_SCALE_NONE:

            while (lineLength < pScaler->WidthXBefore)
            {
                runLengthOut = pLineIn[runIndexIn++];
                lineLength += runLengthOut;
                pLineOut[runIndexOut++] = runLengthOut;
            }

            break;
        case IMG_SCALE_UP:

            while (lineLength < pScaler->WidthXBefore)
            {
                AddLen = 0;
                runLengthOut = pLineIn[runIndexIn++];
                lineLength += runLengthOut;

                for (i = 0; i < runLengthOut; i++)
                {
                    if (NextStart < pResoRatioX->LoopTimes1)
                    {
                        AddLen += pResoRatioX->AddLen1;
                    }
                    else if (NextStart < (pResoRatioX->LoopTimes1 + pResoRatioX->LoopTimes2))
                    {
                        AddLen += pResoRatioX->AddLen2;
                    }

                    NextStart++;
                    NextStart = NextStart % (pResoRatioX->LoopTimes1 + pResoRatioX->LoopTimes2);
                }

                pLineOut[runIndexOut++] = runLengthOut + AddLen;
            }

            break;
        case IMG_SCALE_DOWN:
            number = 0;
            Len1 = (pResoRatioX->AddLen1 + 1) * pResoRatioX->LoopTimes1 + (pResoRatioX->AddLen2 + 1) * pResoRatioX->LoopTimes2;

            while (lineLength < pScaler->WidthXBefore)
            {
                AddLen = 0;
                runLengthOut = pLineIn[runIndexIn++];
                lineLength += runLengthOut;

                for (i = 0; i < runLengthOut; i++)
                {
                    Len2 = number % Len1;

                    if ((Len2 / (pResoRatioX->AddLen1 + 1)) >= pResoRatioX->LoopTimes1)
                    {
                        if ((Len2 % (pResoRatioX->AddLen2 + 1)) != 0)
                        {
                            AddLen--;
                        }
                    }
                    else
                    {
                        if ((Len2 % (pResoRatioX->AddLen1 + 1)) != 0)
                        {
                            AddLen--;
                        }
                    }

                    number++; // indicate the pixel number
                }

                pLineOut[runIndexOut++] = runLengthOut + AddLen;
            }

            break;
    }
}


void ImgScalePaperSizeX(ScalerStruct *pScaler, UWORD *pLineIn, UWORD *pLineOut)
{
    const PaperSizeRatioStruct *pPaperRatioX = pScaler->pPaperRatioX;
    int runIndexIn = 0, runIndexOut = 0, lineLength = 0, number = 0;
    int NextStart = 0;
    UWORD runLengthOut;
    int zeroFlag;

    switch (pScaler->PaperScale)
    {
        case IMG_SCALE_NONE:

            while (lineLength < pScaler->WidthXAfter)
            {
                runLengthOut = pLineIn[runIndexIn++];
                lineLength += runLengthOut;
                pLineOut[runIndexOut++] = runLengthOut;
            }

            break;
        case IMG_SCALE_UP:
            number = pPaperRatioX->SkipLen1;

            while (lineLength < pScaler->WidthXAfter)
            {
                runLengthOut = pLineIn[runIndexIn++];
                lineLength += runLengthOut;

                while (lineLength >= number)
                {
                    number += (NextStart & 0x1) ? pPaperRatioX->SkipLen2 : pPaperRatioX->SkipLen1;
                    NextStart++;

                    if (NextStart >= pPaperRatioX->SkipTimes)
                    {
                        NextStart = 0;
                    }

                    runLengthOut++;
                }

                pLineOut[runIndexOut++] = runLengthOut;
            }

            break;
        case IMG_SCALE_DOWN:
            number = pPaperRatioX->SkipLen1 + 1;
            zeroFlag = 0;

            while (lineLength < pScaler->WidthXAfter)
            {
                runLengthOut = pLineIn[runIndexIn++];
                lineLength += runLengthOut;

                while (lineLength >= number)
                {
                    number += 1 + ((NextStart & 0x1) ? pPaperRatioX->SkipLen2 : pPaperRatioX->SkipLen1); // odd is run1, even is run2
                    NextStart++;

                    if (NextStart >= pPaperRatioX->SkipTimes)
                    {
                        NextStart = 0;
                    }

                    runLengthOut--;
                }

                if (runLengthOut)
                {
                    if (zeroFlag)
                    {
                        zeroFlag = 0;
                        pLineOut[runIndexOut - 1] += runLengthOut;
                    }
                    else
                    {
                        pLineOut[runIndexOut++] = runLengthOut;
                    }
                }
                else
                {
                    zeroFlag = 1;
                }
            }

            break;
    }
}



int ImgScaleResolutionY(ScalerStruct *pScaler)
{
    ResolutionRatioStrcut *pResoRatioY = &(pScaler->ResoRatioY);
    int VerticalLineCnt = 1;
    int Len1, Len2;

    switch (pScaler->ResoScaleY)
    {
        case IMG_SCALE_NONE:
            break;
        case IMG_SCALE_UP:
            Len1 = pResoRatioY->LoopTimes1 + pResoRatioY->LoopTimes2;
            Len2 = pScaler->LineCntBefore % Len1;

            if (Len2 < pResoRatioY->LoopTimes1)
            {
                VerticalLineCnt = pResoRatioY->AddLen1 + 1;
            }
            else
            {
                VerticalLineCnt = pResoRatioY->AddLen2 + 1;
            }

            pScaler->LineCntBefore++;
            break;
        case IMG_SCALE_DOWN:
            Len1 = (pResoRatioY->AddLen1 + 1) * pResoRatioY->LoopTimes1 + (pResoRatioY->AddLen2 + 1) * pResoRatioY->LoopTimes2;
            Len2 = pScaler->LineCntBefore % Len1;

            if ((Len2 / (pResoRatioY->AddLen1 + 1)) >= pResoRatioY->LoopTimes1)
            {
                if ((Len2 % (pResoRatioY->AddLen2 + 1)) != 0)
                {
                    VerticalLineCnt = 0;
                }
            }
            else
            {
                if ((Len2 % (pResoRatioY->AddLen1 + 1)) != 0)
                {
                    VerticalLineCnt = 0;
                }
            }

            pScaler->LineCntBefore++;
            break;
    }

    return VerticalLineCnt;
}



int ImgScalePaperSizeY(ScalerStruct *pScaler)
{
    const PaperSizeRatioStruct *pPaperRatioY = pScaler->pPaperRatioY;
    int VerticalLineCnt = 1, i;
    int Len1, Len2;


    switch (pScaler->PaperScale)
    {
        case IMG_SCALE_NONE:
            break;
        case IMG_SCALE_UP:
            Len1 = 0;

            for (i = 0; i < pPaperRatioY->SkipTimes; i++)
            {
                if (i & 1)
                {
                    Len1 += pPaperRatioY->SkipLen2;
                }
                else
                {
                    Len1 += pPaperRatioY->SkipLen1;
                }
            }

            for (i = pScaler->LineCntByPSY; i < pScaler->LineCntAfter; i++)
            {
                Len2 = i % Len1;
                Len2 = Len2 % (pPaperRatioY->SkipLen1 + pPaperRatioY->SkipLen2);

                if ((Len2 == (pPaperRatioY->SkipLen1 - 1)) || (Len2 == (pPaperRatioY->SkipLen1 + pPaperRatioY->SkipLen2 - 1)))
                {
                    VerticalLineCnt++;
                }
            }

            pScaler->LineCntByPSY = pScaler->LineCntAfter;
            break;
        case IMG_SCALE_DOWN:
            Len1 = 0;

            for (i = 0; i < pPaperRatioY->SkipTimes; i++)
            {
                if (i & 1)
                {
                    Len1 += (pPaperRatioY->SkipLen2 + 1);
                }
                else
                {
                    Len1 += (pPaperRatioY->SkipLen1 + 1);
                }
            }

            for (i = pScaler->LineCntByPSY; i < pScaler->LineCntAfter; i++)
            {
                Len2 = i % Len1;
                Len2 = Len2 % (pPaperRatioY->SkipLen1 + 1 + pPaperRatioY->SkipLen2 + 1);

                if ((Len2 == (pPaperRatioY->SkipLen1)) || (Len2 == (pPaperRatioY->SkipLen1 + pPaperRatioY->SkipLen2 + 1)))
                {
                    VerticalLineCnt = 0;
                }
            }

            pScaler->LineCntByPSY = pScaler->LineCntAfter;
            break;
    }

    return VerticalLineCnt;
}


/************************************************************************\
    SCALER_apply()
    Performs the scaling function on lines of bitmapped data. This can
    only be called after the scaler has been initialized and then is
    called for each line of the input bitmap.

    Scaling ratios are specified by ScalerRatioStruct which describes the
    periodicity of adding or dropping bits/lines. Whether to drop or add
    a bit/line depends on whether the sclaing is scaling up (add bits/
    lines) or scaling down (dropping bits/lines).

    The scaling occurs as an integral number of operations (adding or
    dropping) for an integral number of bits/lines. In all cases, the
    periodicity works as cycleCount number of cycles of runCount number
    of runs of length run1, then run2, followed by a run of length run1.

    For the example of A4->B4, 5 bits/lines are added for every 27 bits/
    lines. The following cycle occurs 64 times to process 1728 bits per
    line:

    12345(+)123456(+)12345(+)123456(+)12345(+)
    |<->|   |<-->|                            12345(+)123456(+)12...
    run1=5  run2=6                   | run1  |
    |<---------- runCount=2 -------->|<----->|
    |<---------------- 1 cycle ------------->|<---- 63 more times--->
\************************************************************************/
int SCALER_apply(ScalerStruct *pScaler, UWORD *pLineIn, UWORD *pLineOut)
{
    //const PaperSizeRatioStruct *pPaperRatioX = pScaler->pPaperRatioX;
    //const PaperSizeRatioStruct *pPaperRatioY = pScaler->pPaperRatioY;
    //ResolutionRatioStrcut *pResoRatioX = &(pScaler->ResoRatioX);
    //ResolutionRatioStrcut *pResoRatioY = &(pScaler->ResoRatioY);
    int VerticalLineCnt = 0, VerticalLineCnt1;
    UWORD OutData[BITMAP_LENGTH];

    VerticalLineCnt1 = ImgScaleResolutionY(pScaler);
    pScaler->LineCntAfter += VerticalLineCnt1;

    if (VerticalLineCnt1 > 0)
    {
        VerticalLineCnt = ImgScalePaperSizeY(pScaler);
    }

    if (VerticalLineCnt > 0)
    {
        ImgScaleResolutionX(pScaler, pLineIn, OutData);
        ImgScalePaperSizeX(pScaler, OutData, pLineOut);
    }

    return VerticalLineCnt;
}

#endif

