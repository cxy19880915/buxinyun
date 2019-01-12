#ifndef __HM2131_MIPI_H__
#define __HM2131_MIPI_H__

#include "typedef.h"
#include "asm/isp_dev.h"

#define HM2131_MIPI_OUTPUT_W     1928
#define HM2131_MIPI_OUTPUT_H     1088


#define HM2131_VARIABLE_FPS    0

s32 HM2131_MIPI_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 HM2131_MIPI_power_ctl(u8 isp_dev, u8 is_work);

s32 HM2131_MIPI_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio);
s32 HM2131_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);

void HM2131_MIPI_sleep();
void HM2131_MIPI_wakeup();
void HM2131_MIPI_W_Reg(u16 addr, u16 val);
u16 HM2131_MIPI_R_Reg(u16 addr);


#endif

