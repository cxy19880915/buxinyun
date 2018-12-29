#ifndef __H62_MIPI_H__
#define __H62_MIPI_H__

#include "typedef.h"
#include "asm/isp_dev.h"

#define H62_MIPI_OUTPUT_W     1280
#define H62_MIPI_OUTPUT_H      720


#define H62_VARIABLE_FPS    0

s32 H62_MIPI_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 H62_MIPI_power_ctl(u8 isp_dev, u8 is_work);

s32 H62_MIPI_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio);
s32 H62_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);

void H62_MIPI_sleep();
void H62_MIPI_wakeup();
void H62_MIPI_W_Reg(u16 addr, u16 val);
u16 H62_MIPI_R_Reg(u16 addr);


#endif


