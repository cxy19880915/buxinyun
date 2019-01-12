#ifndef __SC2143_H__
#define __SC2143_H__

#include "typedef.h"

#define SC2143_OUTPUT_W    1920
#define SC2143_OUTPUT_H    1080




s32 sc2143_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 sc2143_power_ctl(u8 isp_dev, u8 is_work);

//s32 GC1004_check(u8 isp_dev);
s32 sc2143_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void sc2143_sleep();
void sc2143_wakeup();
void sc2143_W_Reg(u16 addr, u16 val);
u16 sc2143_R_Reg(u16 addr);


#endif


