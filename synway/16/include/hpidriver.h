#ifndef __GAO_Driver_H__
#define __GAO_Driver_H__

#define CMD_TIDSP_RESET                     1
#define CMD_TIDSP_POWER_STATUS              2
#define CMD_TIDSP_BOOT_READY                3
#define CMD_TIDSP_HPI_READ                  4
#define CMD_TIDSP_HPI_WRITE                 5
#define CMD_TIDSP_HRDY                      6
#define CMD_TIDSP_HPI_BUF_READ              7
#define CMD_TIDSP_HPI_BUF_WRITE             8
#define CMD_HPI_READ                        9
#define CMD_HPI_WRITE                       10
#define LENGTH_TIDSP_REGISTERS              0x8

typedef struct
{
    unsigned short  offset;
    int     size;
    unsigned char   data[LENGTH_TIDSP_REGISTERS];
} sTiDspInfo;

//#define MAX_TIDSP_BUF_SIZE    2
#define MAX_TIDSP_BUF_SIZE  300
typedef struct
{
    int     size;
    unsigned char   data[MAX_TIDSP_BUF_SIZE];
} sTiDspBufInfo;

typedef struct
{
    unsigned short *pBuff;
    unsigned int HPIWordaddr;
    int WordLen;
} sHPIRWPARA;


#define BIT_0                               0x00000001
#define BIT_1                               0x00000002
#define BIT_2                               0x00000004
#define BIT_3                               0x00000008
#define BIT_4                               0x00000010
#define BIT_5                               0x00000020
#define BIT_6                               0x00000040
#define BIT_7                               0x00000080
#define BIT_8                               0x00000100
#define BIT_9                               0x00000200
#define BIT_10                              0x00000400
#define BIT_11                              0x00000800
#define BIT_12                              0x00001000
#define BIT_13                              0x00002000
#define BIT_14                              0x00004000
#define BIT_15                              0x00008000


// HPIC register
#define BOB                                 BIT_0
#define DSPINT                              BIT_1
#define HINT                                BIT_2
#define HRDY                                BIT_3   /* read only */
#define FETCH                               BIT_4
#define HPIRST                              BIT_7   /* read only */
#define BOBSTAT                             BIT_8   /* read only */
#define DUALHPIA                            BIT_9
#define HPIASEL                             BIT_11

#define TI_DSP_HPIC_LOW                     (0)
#define TI_DSP_HPIC_HIGH                    (1)
#define TI_DSP_HPID_WITH_AUTOINC_LOW        (2)
#define TI_DSP_HPID_WITH_AUTOINC_HIGH       (3)
#define TI_DSP_HPIA_LOW                     (4)
#define TI_DSP_HPIA_HIGH                    (5)
#define TI_DSP_HPID_WITHOUT_AUTOINC_LOW     (6)
#define TI_DSP_HPID_WITHOUT_AUTOINC_HIGH    (7)

#define FPGA_IO_ADDR_BASE                   0x11800000
#define FPGA_IO_ADDR_LEN                    0x00001000
#define TIDSP_ADDR_BASE                     0x11c00000
#define TIDSP_ADDR_LEN                      0x00000008

#define RSTCNT_OFFSET                       0x12
#define VMONI_OFFSET                        0x14
#define BRDY_OFFSET                         0x16

#define HPIC_REG_RESET                      HRDY


#endif
