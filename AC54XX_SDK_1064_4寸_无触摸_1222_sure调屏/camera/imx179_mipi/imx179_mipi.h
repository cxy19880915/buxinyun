#ifndef __IMX179_MIPI_H__
#define __IMX179_MIPI_H__

#include "typedef.h"
#include "asm/isp_dev.h"

/* #define IMX179_MIPI_OUTPUT_W  1280// 1920// 1640   */
/* #define IMX179_MIPI_OUTPUT_H   720// 1080//1232   */
#define IMX179_MIPI_OUTPUT_W   2304
#define IMX179_MIPI_OUTPUT_H   1296
/* #define IMX179_MIPI_OUTPUT_W  1920// 1640    */
/* #define IMX179_MIPI_OUTPUT_H  1080//1232    */



s32 IMX179_MIPI_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 IMX179_MIPI_power_ctl(u8 isp_dev, u8 is_work);

s32 IMX179_MIPI_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio);
s32 IMX179_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);

void IMX179_MIPI_sleep();
void IMX179_MIPI_wakeup();
void IMX179_MIPI_W_Reg(u16 addr, u16 val);
u16 IMX179_MIPI_R_Reg(u16 addr);


#endif

