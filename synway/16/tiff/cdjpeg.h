/*
 * cdjpeg.h
 *
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains common declarations for the sample applications
 * cjpeg and djpeg.  It is NOT used by the core JPEG library.
 */

#ifndef _CDJPEG_H_
#define _CDJPEG_H_


#define JPEG_CJPEG_DJPEG    /* define proper options in jconfig.h */
#define JPEG_INTERNAL_OPTIONS   /* cjpeg.c,djpeg.c need to see xxx_SUPPORTED */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"     /* get library error codes too */
#include "cderror.h"        /* get application-specific error codes */
//#include "studio.h"
//#include "magick-type.h"
//#include "quantize.h"
//#include "image-private.h"
//#include "geometry.h"

/*
#define CacheShift  2
#define ErrorQueueLength  16
#define MaxNodes  266817
#define MaxTreeDepth  8
#define NodesInAList  1920




typedef struct _RealPixelPacket
{
  MagickRealType
    red,
    green,
    blue,
    opacity;
} RealPixelPacket;

typedef struct _NodeInfo
{
  struct _NodeInfo
    *parent,
    *child[16];

  MagickSizeType
    number_unique;

  RealPixelPacket
    total_color;

  MagickRealType
    quantize_error;

  unsigned long
    color_number,
    id,
    level;
} NodeInfo;

typedef struct _Nodes
{
  NodeInfo
    *nodes;

  struct _Nodes
    *next;
} Nodes;

typedef struct _CubeInfo
{
  NodeInfo
    *root;

  unsigned long
    colors,
    maximum_colors;

  long
    transparent_index;

  MagickSizeType
    transparent_pixels;

  RealPixelPacket
    target;

  MagickRealType
    distance,
    pruning_threshold,
    next_threshold;

  unsigned long
    nodes,
    free_nodes,
    color_number;

  NodeInfo
    *next_node;

  Nodes
    *node_queue;

  long
    *cache;

  RealPixelPacket
    error[ErrorQueueLength];

  MagickRealType
    weights[ErrorQueueLength];

  QuantizeInfo
    *quantize_info;

  MagickBooleanType
    associate_alpha;

  long
    x,
    y;

  unsigned long
    depth;

  MagickOffsetType
    offset;

  MagickSizeType
    span;
} CubeInfo;



typedef struct _PrimaryInfo
{
  double
    x,
    y,
    z;
} PrimaryInfo;


typedef struct _TransformPacket
{
  MagickRealType
    x,
    y,
    z;
} TransformPacket;*/


/*
 * Object interface for cjpeg's source file decoding modules
 */

typedef struct cjpeg_source_struct *cjpeg_source_ptr;

struct cjpeg_source_struct
{
    JMETHOD(void, start_input, (j_compress_ptr cinfo,
                                cjpeg_source_ptr sinfo));
    JMETHOD(JDIMENSION, get_pixel_rows, (j_compress_ptr cinfo,
                                         cjpeg_source_ptr sinfo));
    JMETHOD(void, finish_input, (j_compress_ptr cinfo,
                                 cjpeg_source_ptr sinfo));

    FILE *input_file;

    JSAMPARRAY buffer;
    JDIMENSION buffer_height;
};


/*
 * Object interface for djpeg's output file encoding modules
 */

typedef struct djpeg_dest_struct *djpeg_dest_ptr;

struct djpeg_dest_struct
{
    /* start_output is called after jpeg_start_decompress finishes.
     * The color map will be ready at this time, if one is needed.
     */
    JMETHOD(void, start_output, (j_decompress_ptr cinfo,
                                 djpeg_dest_ptr dinfo));
    /* Emit the specified number of pixel rows from the buffer. */
    JMETHOD(void, put_pixel_rows, (j_decompress_ptr cinfo,
                                   djpeg_dest_ptr dinfo,
                                   JDIMENSION rows_supplied));
    /* Finish up at the end of the image. */
    JMETHOD(void, finish_output, (j_decompress_ptr cinfo,
                                  djpeg_dest_ptr dinfo));

    /* Target file spec; filled in by djpeg.c after object is created. */
    FILE *output_file;

    /* Output pixel-row buffer.  Created by module init or start_output.
     * Width is cinfo->output_width * cinfo->output_components;
     * height is buffer_height.
     */
    JSAMPARRAY buffer;
    JDIMENSION buffer_height;
};

/* Private version of data destination object */

typedef struct
{
    struct djpeg_dest_struct pub; /* public fields */

    boolean is_os2;       /* saves the OS2 format request flag */

    jvirt_sarray_ptr whole_image; /* needed to reverse row order */
    JDIMENSION data_width;    /* JSAMPLEs per row */
    JDIMENSION row_width;     /* physical width of one row in the BMP file */
    int pad_bytes;        /* number of padding bytes needed per row */
    JDIMENSION cur_output_row;    /* next row# to write to virtual array */
} bmp_dest_struct;

typedef bmp_dest_struct *bmp_dest_ptr;



/*
 * cjpeg/djpeg may need to perform extra passes to convert to or from
 * the source/destination file format.  The JPEG library does not know
 * about these passes, but we'd like them to be counted by the progress
 * monitor.  We use an expanded progress monitor object to hold the
 * additional pass count.
 */

struct cdjpeg_progress_mgr
{
    struct jpeg_progress_mgr pub; /* fields known to JPEG library */
    int completed_extra_passes;   /* extra passes completed */
    int total_extra_passes;   /* total extra */
    /* last printed percentage stored here to avoid multiple printouts */
    int percent_done;
};

typedef struct cdjpeg_progress_mgr *cd_progress_ptr;


/* Short forms of external names for systems with brain-damaged linkers. */

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jinit_read_bmp      jIRdBMP
#define jinit_write_bmp     jIWrBMP
#define jinit_read_gif      jIRdGIF
#define jinit_write_gif     jIWrGIF
#define jinit_read_ppm      jIRdPPM
#define jinit_write_ppm     jIWrPPM
#define jinit_read_rle      jIRdRLE
#define jinit_write_rle     jIWrRLE
#define jinit_read_targa    jIRdTarga
#define jinit_write_targa   jIWrTarga
#define read_quant_tables   RdQTables
#define read_scan_script    RdScnScript
#define set_quality_ratings     SetQRates
#define set_quant_slots     SetQSlots
#define set_sample_factors  SetSFacts
#define read_color_map      RdCMap
#define enable_signal_catcher   EnSigCatcher
#define start_progress_monitor  StProgMon
#define end_progress_monitor    EnProgMon
#define read_stdin      RdStdin
#define write_stdout        WrStdout
#endif /* NEED_SHORT_EXTERNAL_NAMES */

/* Module selection routines for I/O modules. */

EXTERN(cjpeg_source_ptr) jinit_read_bmp JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_bmp JPP((j_decompress_ptr cinfo,
        boolean is_os2));
EXTERN(cjpeg_source_ptr) jinit_read_gif JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_gif JPP((j_decompress_ptr cinfo));
EXTERN(cjpeg_source_ptr) jinit_read_ppm JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_ppm JPP((j_decompress_ptr cinfo));
EXTERN(cjpeg_source_ptr) jinit_read_rle JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_rle JPP((j_decompress_ptr cinfo));
EXTERN(cjpeg_source_ptr) jinit_read_targa JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_targa JPP((j_decompress_ptr cinfo));

/* cjpeg support routines (in rdswitch.c) */

EXTERN(boolean) read_quant_tables JPP((j_compress_ptr cinfo, char *filename,
                                       boolean force_baseline));
EXTERN(boolean) read_scan_script JPP((j_compress_ptr cinfo, char *filename));
EXTERN(boolean) set_quality_ratings JPP((j_compress_ptr cinfo, char *arg,
                                        boolean force_baseline));
EXTERN(boolean) set_quant_slots JPP((j_compress_ptr cinfo, char *arg));
EXTERN(boolean) set_sample_factors JPP((j_compress_ptr cinfo, char *arg));

/* djpeg support routines (in rdcolmap.c) */

EXTERN(void) read_color_map JPP((j_decompress_ptr cinfo, FILE *infile));

/* common support routines (in cdjpeg.c) */

EXTERN(void) enable_signal_catcher JPP((j_common_ptr cinfo));
EXTERN(void) start_progress_monitor JPP((j_common_ptr cinfo,
                                        cd_progress_ptr progress));
EXTERN(void) end_progress_monitor JPP((j_common_ptr cinfo));
EXTERN(boolean) keymatch JPP((char *arg, const char *keyword, int minchars));
EXTERN(FILE *) read_stdin JPP((void));
EXTERN(FILE *) write_stdout JPP((void));

/* miscellaneous useful macros */

#ifdef DONT_USE_B_MODE      /* define mode parameters for fopen() */
#define READ_BINARY "r"
#define WRITE_BINARY    "w"
#else
#ifdef VMS          /* VMS is very nonstandard */
#define READ_BINARY "rb", "ctx=stm"
#define WRITE_BINARY    "wb", "ctx=stm"
#else               /* standard ANSI-compliant case */
#define READ_BINARY "rb"
#define WRITE_BINARY    "wb"
#endif
#endif

#ifndef EXIT_FAILURE        /* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#ifdef VMS
#define EXIT_SUCCESS  1     /* VMS is very nonstandard */
#else
#define EXIT_SUCCESS  0
#endif
#endif
#ifndef EXIT_WARNING
#ifdef VMS
#define EXIT_WARNING  1     /* VMS is very nonstandard */
#else
#define EXIT_WARNING  2
#endif
#endif

#endif


