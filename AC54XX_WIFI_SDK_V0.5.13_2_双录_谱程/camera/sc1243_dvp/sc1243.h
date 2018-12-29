#ifndef __SC1243_DVP_H__
#define __SC1243_DVP_H__

#include "typedef.h"

#define SC1243_DVP_OUTPUT_W    1280
#define SC1243_DVP_OUTPUT_H    720

#define SC1243_FPS_VARIABLE    0


s32 sc1243_dvp_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 sc1243_dvp_power_ctl(u8 isp_dev, u8 is_work);

//s32 GC1004_check(u8 isp_dev);
s32 sc1243_dvp_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void sc1243_dvp_sleep();
void sc1243_dvp_wakeup();
void sc1243_dvp_W_Reg(u16 addr, u16 val);
u16 sc1243_dvp_R_Reg(u16 addr);


#endif



