
#include "gstdio.h"
#if (SUPPORT_TX_PURE_TEXT_FILE || SUPPORT_TX_COVER_PAGE)
#include "commtype.h"
#include "t4def.h"
#include "asciicov.h"

#define COLOR(x)    (int)((x)^(WHITE))
/* x: the bit color defined in the fontPool: 0:white, 1:black
** In order to be compatible with the color defined in t4.def, use the
** following macro to get the actual color with the definition in t4.def
*/

/***********************************************************************************\
    Ascii Converter Init
    This function initialize Ascii Converter structure.

    Input:
    pAscConverter:  pointer to the structure
    pageWidth:      page width in pixel
    marginWidth:    margin width in pixel
    lineHeight:     height of one line in pixel (this is used to control line space.)
    bBold:          if non-zero, bold font is used

    Return Value:
    ASCII2BMP_ERROR:    Error
    > 0:                number of characters in a line
\***********************************************************************************/
int ASCII2BMP_init(ascii2BmpStruct *pAscConverter, int pageWidth, int marginWidth, int lineHeight, int format)
{
    ascii2BmpStruct *pac = pAscConverter;
    const fontDescriptionStruct *pFont;

    memset(pac, 0, sizeof(ascii2BmpStruct)); //is this needed?

    pac->format = format;
    pac->pFont = ((format & ASCII2BMP_FORMAT_BOLD) ? &fontBold : &fontNormal);
    pFont = pac->pFont;

    pac->lineHeight = lineHeight;
    pac->numCharsPerLine = (pageWidth - marginWidth - marginWidth) / pFont->width;
    pac->numPixelsInLeftMargin = marginWidth;
    pac->numPixelsInRightMargin = pageWidth - (pac->numCharsPerLine * pFont->width) - marginWidth;

    //if (pac->lineHeight < pFont->height + 3)
    //  return ASCII2BMP_ERROR;
    if (pac->numCharsPerLine > MAX_CHARACTER_PER_LINE || pac->numCharsPerLine < 0)
    {
        return ASCII2BMP_ERROR;
    }

    pac->pTail = pac->asciiLine + pac->numCharsPerLine;
    pac->pCurrent = pac->asciiLine;

    return pac->numCharsPerLine;
}


/***********************************************************************************\
    Conversion Function
    This function outputs bitmap bit stream of the loaded one line characters.

    Input:
    pAscConverter:      pointer to the structure
    pNewLine:           array to store image bit of one pixel line

    Return Value:
    ASCII2BMP_CONTINUE: Continue to call this function to get next pixel line.
    ASCII2BMP_DONE:     Finished output bit for this line character.
\***********************************************************************************/
int ASCII2BMP_apply(ascii2BmpStruct *pAscConverter, UBYTE *pNewLine)
{
    ascii2BmpStruct *pac = pAscConverter;
    const fontDescriptionStruct *pFont = pac->pFont;
    UDWORD pattern;
    int i, j;
    UBYTE *pBit = pNewLine;
    UBYTE bit;

    pac->outLineIdx++;

    // left white margin
    for (j = 0; j < pac->numPixelsInLeftMargin; j++)
    {
        *pBit++ = WHITE;
    }

    // page content
    if (pac->outLineIdx <= pFont->height)
    {
        //Font character line
        for (i = 0; i < pac->numCharsPerLine; i++)
        {
            pattern = pFont->glyphs[pac->outLineIdx - 1][pac->asciiLine[i] - START_CHAR];

            for (j = pFont->width - 1; j >= 0; j--)
            {
                *pBit++ = COLOR((pattern >> j) & 0x01);
            }
        }
    }
    else if ((pac->outLineIdx == pFont->height + 2) && (pac->format & ASCII2BMP_FORMAT_ITALICS))
    {
        // Underline
        for (i = 0; i < pac->numCharsPerLine; i++)
        {
            bit = (pac->asciiLine[i] == ' ') ? (WHITE) : (BLACK);
            //bit = BLACK;
            // bit = WHITE;
            //printf("%c", pac->asciiLine[i]);
            //printf("%d\n", pac->numCharsPerLine);

            for (j = 0; j < pFont->width; j++)
            {
                *pBit++ = bit;
            }
        }
    }
    else
    {
        // line spacing
        for (j = 0; j < pac->numCharsPerLine * pFont->width; j++)
        {
            *pBit++ = WHITE;
        }
    }

    // right margin and extra leftover pixels
    for (j = 0; j < pac->numPixelsInRightMargin; j++)
    {
        *pBit++ = WHITE;
    }

    // check if final line
    if (pac->outLineIdx >= pac->lineHeight)
    {
        pac->outLineIdx = 0;
        return ASCII2BMP_DONE;
    }

    return ASCII2BMP_CONTINUE;
}

#endif
