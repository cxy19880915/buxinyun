#ifndef __SC3033_MIPI_H__
#define __SC3033_MIPI_H__

#include "typedef.h"


#define SC3033_1080P 1
#define SC3033_1296P 0
#define SC3033_2048x1536 0

#if SC3033_1080P
#define SC3033_MIPI_OUTPUT_W    1920
#define SC3033_MIPI_OUTPUT_H    1088
#endif

#if SC3033_1296P
#define SC3033_MIPI_OUTPUT_W    2304
#define SC3033_MIPI_OUTPUT_H    1296
#endif


#if SC3033_2048x1536
#define SC3033_MIPI_OUTPUT_W    2048
#define SC3033_MIPI_OUTPUT_H    1536
#endif


s32 sc3033_mipi_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 sc3033_mipi_power_ctl(u8 isp_dev, u8 is_work);

//s32 GC1004_check(u8 isp_dev);
s32 sc3033_mipi_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void sc3033_mipi_sleep();
void sc3033_mipi_wakeup();
void sc3033_mipi_W_Reg(u16 addr, u16 val);
u16 sc3033_mipi_R_Reg(u16 addr);


#endif


