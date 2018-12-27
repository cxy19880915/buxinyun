#ifndef __C2392_MIPI_H__
#define __C2392_MIPI_H__

#include "typedef.h"
#include "asm/isp_dev.h"

#define C2392_MIPI_OUTPUT_W     1920
#define C2392_MIPI_OUTPUT_H     1080



s32 C2392_MIPI_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 C2392_MIPI_power_ctl(u8 isp_dev, u8 is_work);

s32 C2392_MIPI_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio);
s32 C2392_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);

void C2392_MIPI_sleep();
void C2392_MIPI_wakeup();
void C2392_MIPI_W_Reg(u16 addr, u16 val);
u16 C2392_MIPI_R_Reg(u16 addr);


#endif

