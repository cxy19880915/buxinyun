#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "asm/avo_cfg.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#if (LCD_TYPE == LCD_AVOUT)

static void lcd_avout_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE(lcd_avout_dev) = {
    .info = {
        .test_mode       = false,
        .color           = 0xffff00,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_addr 		 = LCD_DEV_BUF,
        .buf_num  		 = LCD_DEV_BNUM,
        .itp_mode_en     = false,
        .sample          = LCD_DEV_SAMPLE,
        .format          = FORMAT_YUV422,

        .len 			 = LEN_256,
        .rotate_en 		 = false,		// 旋转使能
        .hori_mirror_en  = false,		// 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能

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
        },
    },

    .drive_mode      = MODE_AVOUT,
#if 1//osc clk 12MHz
#if   (AVOUT_MODE == NTSC_720)
    .avout_mode 	 = CVE_NTSC_720/* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_12M | PLL1_CLK | DIVA_5 | DIVB_1 | DIVC_4,
    .pll1_nf         = 60,
    .pll1_nr         = 2700,/* 2700*12M/60 = 540M */
#elif (AVOUT_MODE == NTSC_896)
    .avout_mode 	 = CVE_NTSC_896 /*|CVE_COLOR_BAR*/,
    .clk_cfg    	 = IMD_INTERNAL | OSC_12M | PLL1_CLK | DIVA_1 | DIVB_4 | DIVC_4,
    .pll1_nf         = 55,
    .pll1_nr         = 2460,/* 2460*12M/55 = 536M */
#elif (AVOUT_MODE == PAL_720)
    .avout_mode 	 = CVE_PAL_720 /*|CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_12M | PLL1_CLK | DIVA_5 | DIVB_1 | DIVC_4,
    .pll1_nf         = 60,
    .pll1_nr         = 2700,/* 2700*12M/60 = 540M */
#elif (AVOUT_MODE == PAL_896)
    .avout_mode 	 = CVE_PAL_896 /*|CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_12M | PLL1_CLK | DIVA_1 | DIVB_4 | DIVC_4,
    .pll1_nf         = 60,
    .pll1_nr         = 2680,/* 2680*12M/60 = 536M */
#endif
#else//osc clk 32K
#if   (AVOUT_MODE == NTSC_720)
    .avout_mode 	 = CVE_NTSC_720/* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_32K | PLL1_CLK | DIVA_5 | DIVB_1 | DIVC_4,
    .pll1_nf         = 1,
    .pll1_nr         = 16480,/* 16480*0.032768M/1 = 540M */
#elif (AVOUT_MODE == NTSC_896)
    .avout_mode 	 = CVE_NTSC_896 /*|CVE_COLOR_BAR*/,
    .clk_cfg    	 = IMD_INTERNAL | OSC_32K | PLL1_CLK | DIVA_1 | DIVB_4 | DIVC_4,
    .pll1_nf         = 1,
    .pll1_nr         = 16380,/* 16380*0.032768M/1 = 536M*/
#elif (AVOUT_MODE == PAL_720)
    .avout_mode 	 = CVE_PAL_720 /*|CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_32K | PLL1_CLK | DIVA_5 | DIVB_1 | DIVC_4,
    .pll1_nf         = 1,
    .pll1_nr         = 16480,/* 16480*0.032768M/1 = 540M */
#elif (AVOUT_MODE == PAL_896)
    .avout_mode 	 = CVE_PAL_896 /*|CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_32K | PLL1_CLK | DIVA_1 | DIVB_4 | DIVC_4,
    .pll1_nf         = 1,
    .pll1_nr         = 16357,/* 16357*0.032768M/1 = 536M*/
#endif
#endif
    .ncycle          = CYCLE_ONE,
    .interlaced_mode = INTERLACED_ALL,

    .dclk_set        = /*CLK_EN|*/CLK_NEGATIVE,
    .sync0_set       = /*CLK_EN|*/SIGNAL_DEN | CLK_NEGATIVE,
    .sync1_set       = /*CLK_EN|*/SIGNAL_HSYNC | CLK_NEGATIVE,
    .sync2_set       = /*CLK_EN|*/SIGNAL_VSYNC | CLK_NEGATIVE,

    .data_width      = PORT_1BIT,

    .timing = {
#if (AVOUT_MODE == NTSC_720)
        //NTSC 27MHz 720x480
        .hori_interval 		=     858 * 2,
        .hori_sync_clk 		=     138 * 2,
        .hori_start_clk 	=     138 * 2,
        .hori_pixel_width 	=     720,

        .vert_interval 		=     263,
        .vert_sync_clk 		=     21,
        .vert_start_clk 	=     21,//cve2_wr(51,21);
        .vert_start1_clk 	=     22,
        .vert_pixel_width 	=     240,
#elif (AVOUT_MODE == NTSC_896)
        //NTSC 36MHz 960x480
        .hori_interval 		=     1066 * 2,
        .hori_sync_clk 		=     170 * 2,
        .hori_start_clk 	=     170 * 2,
        .hori_pixel_width 	=     896,

        .vert_interval 		=     263,
        .vert_sync_clk 		=     21,
        .vert_start_clk 	=     21,//cve2_wr(51,21);
        .vert_start1_clk 	=     22,
        .vert_pixel_width 	=     240,
#elif (AVOUT_MODE == PAL_720)
        //PAL 27MHz 720x576
        .hori_interval 		=     864 * 2,
        .hori_sync_clk 		=     138 * 2,
        .hori_start_clk 	=     138 * 2,
        .hori_pixel_width 	=     720,

        .vert_interval 		=     313,
        .vert_sync_clk 		=     21,
        .vert_start_clk 	=     23,//cve2_wr(51,21);
        .vert_start1_clk 	=     24,
        .vert_pixel_width 	=     288,
#elif (AVOUT_MODE == PAL_896)
        //PAL 36MHz 960x576
        .hori_interval 		=     1072 * 2,
        .hori_sync_clk 		=     170 * 2,
        .hori_start_clk 	=     170 * 2,
        .hori_pixel_width 	=     896,

        .vert_interval 		=     313,
        .vert_sync_clk 		=     21,
        .vert_start_clk 	=     23,//cve2_wr(51,21);
        .vert_start1_clk 	=     24,
        .vert_pixel_width 	=     288,
#endif
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable	= IF_ENABLE(LCD_AVOUT),
    .logo = "avout",
    .init = NULL,
    .dev  = &lcd_avout_dev,
    .type = LCD_DVP_RGB,
    .bl_ctrl = lcd_avout_backctrl,
};
#endif
