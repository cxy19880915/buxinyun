#ifndef _GC1004_H
#define _GC1004_H

#include "typedef.h"

#define GC1004_DEBUG printf


#define HOST_MCLK           24
#define REG_SH_DELAY        0x18

#define GC_PLL_DIV2            0
#define GC_DCLK_DIV            0

#define GC_PLL_DIV5B        0x3
#define REG_VBLANK           0x10
#define REG_HBLANK          0x177

#define GC1004_OUTPUT_W    1280
#define GC1004_OUTPUT_H     720
#define REG_WIN_WIDTH       1288
#define REG_WIN_HEIGHT      728


#define IS_GC1014         1   //1014 or 1004



s32 GC1004_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 GC1004_power_ctl(u8 isp_dev, u8 is_work);

//s32 GC1004_check(u8 isp_dev);
s32 GC1004_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void GC1004_sleep();
void GC1004_wakeup();
void GC1004_W_Reg(u16 addr, u16 val);
u16 GC1004_R_Reg(u16 addr);


#endif


