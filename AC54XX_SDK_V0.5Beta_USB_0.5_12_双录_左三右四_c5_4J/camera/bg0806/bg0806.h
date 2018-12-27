#ifndef __BG0806_H__
#define __BG0806_H__

#include "typedef.h"
#include "asm/isp_dev.h"

#define BG0806_OUTPUT_W     1920
#define BG0806_OUTPUT_H     1080



s32 BG0806_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 BG0806_power_ctl(u8 isp_dev, u8 is_work);

s32 BG0806_check(u8 isp_dev, u32 _reset_gpio);
s32 BG0806_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);

void BG0806_sleep();
void BG0806_wakeup();
void BG0806_W_Reg(u16 addr, u16 val);
u16 BG0806_R_Reg(u16 addr);


#endif

