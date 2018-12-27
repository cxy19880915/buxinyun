#ifndef __SC2143_MIPI_H__
#define __SC2143_MIPI_H__

#include "typedef.h"

#define SC2143_MIPI_OUTPUT_W    1920
#define SC2143_MIPI_OUTPUT_H    1088




s32 sc2143_mipi_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 sc2143_mipi_power_ctl(u8 isp_dev, u8 is_work);

//s32 GC1004_check(u8 isp_dev);
s32 sc2143_mipi_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void sc2143_mipi_sleep();
void sc2143_mipi_wakeup();
void sc2143_mipi_W_Reg(u16 addr, u16 val);
u16 sc2143_mipi_R_Reg(u16 addr);


#endif


