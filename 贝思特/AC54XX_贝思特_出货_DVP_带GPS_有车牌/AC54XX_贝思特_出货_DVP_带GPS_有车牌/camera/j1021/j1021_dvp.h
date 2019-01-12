#ifndef __J1021_DVP_H__
#define __J1021_DVP_H__

#include "typedef.h"

#define J1021_DVP_OUTPUT_W    1280
#define J1021_DVP_OUTPUT_H    720

#define J1021_VARIABLE_FPS    1 // DECREASE BUTTON


s32 J1021_dvp_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 J1021_dvp_power_ctl(u8 isp_dev, u8 is_work);

//s32 GC1004_check(u8 isp_dev);
s32 J1021_dvp_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void J1021_dvp_sleep();
void J1021_dvp_wakeup();
void J1021_dvp_W_Reg(u16 addr, u16 val);
u16 J1021_dvp_R_Reg(u16 addr);


#endif


