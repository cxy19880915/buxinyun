#ifndef __sc2363_MIPI_H__
#define __sc2363_MIPI_H__

#include "typedef.h"

#define SC2363_MIPI_OUTPUT_W    1920
#define SC2363_MIPI_OUTPUT_H    1080



s32 sc2363_mipi_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 sc2363_mipi_power_ctl(u8 isp_dev, u8 is_work);

s32 sc2363_mipi_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void sc2363_mipi_sleep();
void sc2363_mipi_wakeup();
void sc2363_mipi_W_Reg(u16 addr, u16 val);
u16 sc2363_mipi_R_Reg(u16 addr);


#endif



