#ifndef _CHARPOOL_H_
#define _CHARPOOL_H_

#include "commtype.h"
#include "porting.h"

#define START_CHAR          32  // "space"
//#define END_CHAR          126 // "~"
#define END_CHAR            137 // 127 - 137 self-defined, refer to line43-53 of tiffdef.h
#define CHAR_TOTAL_NUM      (END_CHAR - START_CHAR + 1)

typedef struct
{
    int height;
    int width;
    const UDWORD(*glyphs)[CHAR_TOTAL_NUM];
} fontDescriptionStruct;


#endif
