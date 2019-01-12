#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
/**
 * @Synopsis 图层内存分配
 */
REGISTER_LAYER_BUF(layer_yuv420_buf, FB_COLOR_FORMAT_YUV420, LCD_DEV_WIDTH, LCD_DEV_HIGHT, 2);
/* REGISTER_LAYER_BUF(layer_yuv420_buf, FB_COLOR_FORMAT_YUV420, 176, 128, 2); */
/* REGISTER_LAYER_BUF(layer_osd8_buf, FB_COLOR_FORMAT_256, 480, 124, 1); */
/* REGISTER_LAYER_BUF(layer_osd8_buf, FB_COLOR_FORMAT_256, 480, 52, 1); */
/* REGISTER_LAYER_BUF(layer_osd8_buf, FB_COLOR_FORMAT_256, 480, 60, 1); */

/**
 * @Synopsis  定义图层静态显存
 *
 * @Param layer_yuv420_buf
 * @Param 图层类型:FB_COLOR_FORMAT_YUV420
 * @Param 图层宽度:480
 * @Param 图层高度:272
 * @Param 图层buf数量:2
 */
REGISTER_LCD_DEVICE_BUF(LCD_DEV_WIDTH, LCD_DEV_HIGHT, LCD_DEV_SAMPLE, LCD_DEV_BNUM);
