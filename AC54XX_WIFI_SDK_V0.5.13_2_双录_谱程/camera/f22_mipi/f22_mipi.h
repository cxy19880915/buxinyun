
#ifndef __F22_MIPI_H__
#define __F22_MIPI_H__

#include "typedef.h"
#include "asm/isp_dev.h"

#define F22_MIPI_OUTPUT_W     1920 //-Crop
#define F22_MIPI_OUTPUT_H     1080


#define F22_VARIABLE_FPS    0

s32 F22_MIPI_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 F22_MIPI_power_ctl(u8 isp_dev, u8 is_work);

s32 F22_MIPI_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio);
s32 F22_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);

void F22_MIPI_sleep();
void F22_MIPI_wakeup();
void F22_MIPI_W_Reg(u16 addr, u16 val);
u16 F22_MIPI_R_Reg(u16 addr);


#endif

