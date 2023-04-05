


#ifndef _COVER_PAGE_H_
#define _COVER_PAGE_H_

#define    MAX_LEN_TERMINAL_NAME                     (20)
#define    MAX_LEN_TERMINAKL_PHONE_NUM               (20)
#define    MAX_LEN_TERMINAKL_STREET                  (100)
#define    MAX_LEN_TERMINAKL_CITY                    (50)
#define    MAX_LEN_DATE_TIME_STRING                  (50)


typedef struct
{
    CHAR Name[MAX_LEN_TERMINAL_NAME + 1];
    CHAR FaxNumber[MAX_LEN_TERMINAKL_PHONE_NUM + 1];
    CHAR PhoneNumber[MAX_LEN_TERMINAKL_PHONE_NUM + 1];
    CHAR Street[MAX_LEN_TERMINAKL_STREET + 1];
    CHAR CityState[MAX_LEN_TERMINAKL_CITY + 1];
    CHAR CountryZipCode[MAX_LEN_TERMINAKL_CITY + 1];
} TTerminalDetailsStruct;

typedef struct
{
    CHAR DateTimeStr[MAX_LEN_DATE_TIME_STRING + 1];
    UBYTE Pattern;
    TTerminalDetailsStruct CallerSide;
    TTerminalDetailsStruct CalleeSide;
    UBYTE RptNoCoverPageData;
} TCoverpageStruct;



#endif




