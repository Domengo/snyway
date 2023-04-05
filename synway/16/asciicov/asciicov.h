
#ifndef _ASCIICOV_H_
#define _ASCIICOV_H_

#include <string.h>
#include "charpool.h"

/********************************\
        Macro Definition
\********************************/

#define MARGIN_WIDTH                (25)
#define LINE_HEIGHT                 (16)

#define MAX_CHARACTER_PER_LINE      (152)       // 2432(A3size)/16(fontsize)

// Format options
#define ASCII2BMP_FORMAT_BOLD       (1)
#define ASCII2BMP_FORMAT_ITALICS    (2)

// Return Values
#define ASCII2BMP_OK                (0)
#define ASCII2BMP_ERROR             (-1)
#define ASCII2BMP_CONTINUE          (0)
#define ASCII2BMP_DONE              (1)


/********************************\
       Structure Definition
\********************************/
typedef struct
{
    fontDescriptionStruct *pFont;

    int lineHeight;             // hieight of a character line in pixels
    int numCharsPerLine;        // number of ASCII characters that fit width of page once margins are accounted for
    int numPixelsInLeftMargin;  // number of pixels in the left margin
    int numPixelsInRightMargin; // number of pixels in the right margin (left margin width plus extra pixels due to character width)
    int format;
    UBYTE *pTail;
    UBYTE *pCurrent;
    int outLineIdx;             // number o fline bing processed by ASCII2BMP_apply
    int hasTwoRows;
    UBYTE asciiLine[MAX_CHARACTER_PER_LINE];
} ascii2BmpStruct;


/********************************\
              API
\********************************/
int ASCII2BMP_init(ascii2BmpStruct *pAscConverter, int pageWidth, int marginWidth, int lineHeight, int format);
int ASCII2BMP_apply(ascii2BmpStruct *pAscConverter, UBYTE *pNewLine);

extern fontDescriptionStruct fontNormal;
extern fontDescriptionStruct fontBold;

#endif
