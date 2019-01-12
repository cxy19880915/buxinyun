#ifndef __IMB_COPY__
#define __IMB_COPY__

#include "generic/typedef.h"
#include "asm/imb_driver.h"
#include "os/os_api.h"

struct imbreq {
    u8 *src;
    u8 *dst;
    u32 src_x;
    u32 src_y;
    u32 src_width;
    u32 src_height;
    u32 dst_x;
    u32 dst_y;
    u32 dst_width;
    u32 dst_height;
    u8 buf_index;
    u8 sta;
    u8 busy;
};

extern struct imbreq ireq;

#define IMB_CONV 1
extern OS_SEM imb_sem;

#endif
