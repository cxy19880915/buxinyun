#ifndef __MIS2003_H__
#define __MIS2003_H__

#include "typedef.h"

#define MIS2003_OUTPUT_W    1920
#define MIS2003_OUTPUT_H    1080

// u32 mis2003_set_shutter(isp_ae_shutter_t *shutter);

// u32 mis2003_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain);
#if 0

s32 mis2003_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 mis2003_power_ctl(u8 isp_dev, u8 is_work);


s32 mis2003_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void mis2003_sleep();
void mis2003_wakeup();
void mis2003_W_Reg(u16 addr, u16 val);
u16 mis2003_R_Reg(u16 addr);

#endif

#endif
