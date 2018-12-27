#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_480x272_8BITS

static void lcd_480x272_8bits_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

/* #define INTERPOLATION_MODE_TEST //插值模式测试 */

REGISTER_LCD_DEVICE(lcd_480x272_8bits_dev) = {
    .info = {
        .test_mode 	     = false,
        .color          = 0x00FF00,
        .xres 			= LCD_DEV_WIDTH,
        .yres 	        = LCD_DEV_HIGHT,
        .buf_num  	    = LCD_DEV_BNUM,
        .buf_addr 		= LCD_DEV_BUF,

        .rotate_en      = false,		// 旋转使能
        .hori_mirror_en = false,		// 水平镜像使能
        .vert_mirror_en = false,		// 垂直镜像使能
        .len 			 = LEN_256,

        .adjust = {
            .y_gain = 0x80,
            .u_gain = 0x80,
            .v_gain = 0x80,
            .r_gain = 0x80,
            .g_gain = 0x80,
            .b_gain = 0x80,
            .r_coe0 = 0x40,
            .g_coe1 = 0x40,
            .b_coe2 = 0x40,
            .r_gma  = 100,
            .g_gma  = 100,
            .b_gma  = 100,
        },
#ifdef INTERPOLATION_MODE_TEST
        .itp_mode_en = true,
#else
        .itp_mode_en = false,
#endif
        .sample      = LCD_DEV_SAMPLE,
        .format      = FORMAT_RGB888,
    },

    .drive_mode      = MODE_RGB_DE_SYNC,
    .ncycle          = CYCLE_THREE,
    .interlaced_mode = INTERLACED_NONE,
    .dclk_set 		 = CLK_EN,
    .sync0_set       = SIGNAL_DEN | CLK_EN, //SIGNAL_DEN;
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
    .data_width      = PORT_8BITS,
    .port_sel        = PORT_GROUP_B,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_4,


    .timing = {
#ifdef INTERPOLATION_MODE_TEST
        .hori_interval 		= 625,
        .hori_sync_clk 		= 1,
        .hori_start_clk 	= 40,
        .hori_pixel_width 	= 480,
        .vert_interval 		= 288,

        .vert_sync_clk 		= 1,
        .vert_start_clk 	= 8,
        .vert_start1_clk 	= 0,
        .vert_pixel_width 	= 264,
#else
        .hori_interval 		= 625 * 3, //12500000/60/288,//
        .hori_sync_clk 		= 1 * 3,
        .hori_start_clk 	= 40 * 3,
        .hori_pixel_width 	= 480,

        .vert_interval 		= 288,
        .vert_sync_clk 		= 1,
        .vert_start_clk 	= 8,
        .vert_start1_clk 	= 0,
        .vert_pixel_width 	= 272,
#endif
    },
};


REGISTER_LCD_DEVICE_DRIVE(lcd_480x272_8bits_dev)  = {
    .type = LCD_DVP_RGB,
    .init = NULL,
    .dev  = &lcd_480x272_8bits_dev,
    .bl_ctrl = lcd_480x272_8bits_backctrl,
};
#endif
