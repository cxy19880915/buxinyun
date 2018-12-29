#ifndef __SC2303_DVP_H__
#define __SC2303_DVP_H__

#include "typedef.h"

#define SC2303_DVP_OUTPUT_W    1920
#define SC2303_DVP_OUTPUT_H    1088

#define SC2303_FPS_VARIABLE    1


s32 sc2303_dvp_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 sc2303_dvp_power_ctl(u8 isp_dev, u8 is_work);

//s32 GC1004_check(u8 isp_dev);
s32 sc2303_dvp_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void sc2303_dvp_sleep();
void sc2303_dvp_wakeup();
void sc2303_dvp_W_Reg(u16 addr, u16 val);
u16 sc2303_dvp_R_Reg(u16 addr);


#endif


