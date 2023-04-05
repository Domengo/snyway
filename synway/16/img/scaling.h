#ifndef _SCALING_H_
#define _SCALING_H_

#include "imgprodte.h"

#if SUPPORT_CONVERSION_BETWEEN_T4_T6

#include "commtype.h"
#include "t4def.h"
#include "t4info.h"
#include "dsmstru.h"

/********************************\
        Constants
\********************************/

// Scaling Operations
#define IMG_SCALE_NONE                0
#define IMG_SCALE_UP                1
#define IMG_SCALE_DOWN                2


// Return Value
#define SCALER_INIT_SUCCESS        0
#define SCALER_INIT_ERROR        (-1)

/********************************\
       Structure Definition
\********************************/
typedef struct
{
    UBYTE SkipLen1;    // indicates the length of the first skip in pixels, add 1 pixel after these pixels
    UBYTE SkipLen2;    // indicates the length of the second skip in pixels, add 1 pixel after these pixels
    UBYTE SkipTimes;   // indicates the times of the two skips in total, after that time, repeat the two skips
} PaperSizeRatioStruct;

typedef struct
{
    SWORD AddLen1;       // indicates the length of added pixels after one original pixel
    SWORD LoopTimes1;    // the times of repeating addition of AddLen1 pixels after one original pixel
    SWORD AddLen2;       // indicates the length of added pixels after one original pixel
    SWORD LoopTimes2;     // the times of repeating addition of AddLen1 pixels after one original pixel
} ResolutionRatioStrcut;

//  first resolution conversion, then paper size conversion
typedef struct
{
    const PaperSizeRatioStruct *pPaperRatioX;
    const PaperSizeRatioStruct *pPaperRatioY;
    ResolutionRatioStrcut ResoRatioX;
    ResolutionRatioStrcut ResoRatioY;
    UBYTE PaperScale;
    UBYTE ResoScaleX;
    UBYTE ResoScaleY;
    SWORD WidthXBefore;  // before X resolution conversion
    SWORD WidthXAfter;   // after X resolution conversion
    SDWORD LineCntBefore; // before Y resolution conversion
    SDWORD LineCntAfter;  // after Y resolution conversion
    SDWORD LineCntByPSY;  // used by PaperSizeY conversion
} ScalerStruct;


/********************************\
              API
\********************************/
int SCALER_init(ScalerStruct *pScaler, TImgFormat *pImgIn, TImgFormat *pImgOut);
int SCALER_apply(ScalerStruct *pScaler, UWORD *pLineIn, UWORD *pLineOut);

#endif

#endif

