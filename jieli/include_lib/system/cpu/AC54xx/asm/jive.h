/*
 * file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library.
 *
 *
*/

#ifndef __IMLIB_H__
#define __IMLIB_H__



#include "typedef.h"
#include "os/os_api.h"

#define JIVE_TASK_MAX                           16

#define JIVE_IFMT_YUV420                        0x2
#define JIVE_IFMT_YUV422                        0x3

#define JIVE_FILTER_OFMT_B                      0x0 //8bits
#define JIVE_FILTER_OFMT_W                      0x1 //16bits

#define JIVE_CANNY_OFMT_AMP_AGL                 0x0 //
#define JIVE_CANNY_OFMT_AMP                     0x1

#define JIVE_SUBOP_COPY_THREE_CHANNEL           0x0
#define JIVE_SUBOP_COPY_SINGLE_CHANNEL          0x1

#define JIVE_SUBOP_CSC_YUV2RGB                  0x0
#define JIVE_SUBOP_CSC_RGB2YUV                  0x1
#define JIVE_SUBOP_CSC_RGB2HSV                  0x2

#define JIVE_SUBOP_AND                          0x0
#define JIVE_SUBOP_OR                           0x1
#define JIVE_SUBOP_SUB_ABS                      0x2
#define JIVE_SUBOP_SUB_SHIFT                    0x3

#define JIVE_SUBOP_INTERGRAL_2D                 0x0
#define JIVE_SUBOP_INTERGRAL_1D_HORIZONTAL      0x1
#define JIVE_SUBOP_INTERGRAL_1D_VERTICAL        0x2
#define JIVE_SUBOP_INTERGRAL_2D_SQR             0x3

#define JIVE_SUBOP_MORPH_DILATE                 0x0
#define JIVE_SUBOP_MORPH_ERODE                  0x1

#define JIVE_SUBOP_BINARY_NORMAL                0x0
#define JIVE_SUBOP_BINARY_TRUNC_HIGH            0x1
#define JIVE_SUBOP_BINARY_TRUNC_LOW             0x2

typedef enum {
    OPTYPE_COPY = 0,
    OPTYPE_FILTER,
    OPTYPE_CSC,
    OPTYPE_CANNY,
    OPTYPE_MORPH,
    OPTYPE_BINARY,
    OPTYPE_MATH,
    OPTYPE_INTERGRAL,
    OPTYPE_HIST,
    OPTYPE_GAMMA,
    OPTYPE_SCALE,
    OPTYPE_MOMENT,
} jive_optype_t;

struct integral_sq_coef {
    unsigned char min_val;
    unsigned char max_val;
    unsigned char offset;
    unsigned char shift;
};

struct jive_task {
    unsigned int next_addr;
    unsigned in_fmt : 2;
    unsigned sub_op : 3;
    unsigned out_fmt : 3;
    unsigned op_type : 8;
    unsigned id : 16;

    unsigned short dst1_stride;
    unsigned short src1_stride;
    unsigned height : 9;
    unsigned reserved0 : 7;
    unsigned width : 10;
    unsigned reserved1 : 6;
    //[2-3]
    unsigned int src1;
    unsigned int src2;
    unsigned int dst1;
    unsigned int dst2;

    //[4-5]
    unsigned short dst2_stride;
    unsigned short src2_stride;
    union {
        unsigned char erode_kernel[9];
        char filter_coef[9];
        unsigned char threshold;
        struct integral_sq_coef itgl_sq_coef;
    };
    unsigned char shift;
    unsigned short reserved2;

    //[6-7]
    unsigned short scale_hcoef : 14;
    unsigned reserved3 : 2;
    unsigned scale_vcoef : 14;
    unsigned reserved4 : 2;
    unsigned scale_height : 9;
    unsigned reserved5 : 7;
    unsigned scale_width : 10;
    unsigned reserved6 : 6;
    unsigned int reserved7[2];
};

struct jive_fh {
    struct jive_task *task_tbl;
    int max_task_num;
    u8  empty_task_idx;
    u16 task_id;

    u32 *integral_sq_count_ptr;
    OS_SEM sem;
    u8 state;
};


#endif

