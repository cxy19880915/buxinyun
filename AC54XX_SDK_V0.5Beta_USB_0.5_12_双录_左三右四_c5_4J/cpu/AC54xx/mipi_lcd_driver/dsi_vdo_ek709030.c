#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "app_config.h"

#if (LCD_TYPE == LCD_DSI_VDO_EK79030)
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list
#if 0
const static u8 init_cmd_list[] = {
	_W, DELAY(0), PACKET_DCS, SIZE(1), 0x10,	
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD,0xAA,	
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x30,0x00,	
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x39,0x11,		
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x32,0x00,	
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x33,0x38,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x35,0x24,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x4F,0x35,//26
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x4E,0x27,//13
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x41,0x32,//VCOM36
	_W, DELAY(0), PACKET_DCS, SIZE(9), 0x55,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,	
	_W, DELAY(0), PACKET_DCS, SIZE(17), 0x56,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x65,0x08,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A,0x08,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x36,0x49,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x67,0x82,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x69,0x20,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x6C,0x80,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x6D,0x01,
	_W, DELAY(0), PACKET_DCS, SIZE(20), 0x53,0x1F,0x19,0x15,0x11,0x11,0x11,0x12,0x14,0x15,0x11,0x0D,0x0B,0x0B,0x0D,0x0C,0x0C,0x08,0x04,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(20), 0x54,0x1F,0x19,0x15,0x11,0x11,0x11,0x13,0x15,0x16,0x11,0x0D,0x0C,0x0C,0x0E,0x0C,0x0C,0x08,0x04,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x6B,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x58,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x73,0xF0,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x76,0x40,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x77,0x04,//00
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x74,0x17,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x5E,0x03,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x68,0x10,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x6A,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x28,0x31,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x29,0x21,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x63,0x04,//08
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x27,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x7c,0x80,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x2e,0x05,//PWM
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x4c,0x80,//VCSW1
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x50,0xc0,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x78,0x6E,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x2D,0x31,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x49,0x00,//
	_W, DELAY(120), PACKET_DCS, SIZE(2), 0x4D,0x00,
	_W, DELAY(200), PACKET_DCS, SIZE(1), 0x11,
};

#define freq 350

#define lane_num 4
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

#define vsa_line 2
#define vbp_line 10
#define vda_line 1280
#define vfp_line 12

#define hsa_cyc  24
#define hbp_cyc  161
#define hda_cyc  400
#define hfp_cyc  160
#endif

#if 1
const static u8 init_cmd_list[] = {
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD,0x99,
	_W, DELAY(0), PACKET_DCS, SIZE(23), 0x24,0x0A,0x06,0x09,0x05,0x07,0x0B,0x08,0x0C,0x01,0x14,0x14,0x14,0x14,0x14,0x14,0x11,0x12,0x14,0x14,0x13,0x10,0x14,
	_W, DELAY(0), PACKET_DCS, SIZE(23), 0x25,0x0A,0x06,0x09,0x05,0x07,0x0B,0x08,0x0C,0x01,0x14,0x14,0x14,0x14,0x14,0x14,0x11,0x12,0x14,0x14,0x14,0x13,0x10,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x2B,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(3), 0x72,0x00,0x80,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A,0x10,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x29,0x91,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x36,0x40,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x67,0x82,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x69,0x47,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x6C,0x08,
	_W, DELAY(0), PACKET_DCS, SIZE(20), 0x53,0x1F,0x1C,0x1B,0x18,0x19,0x1A,0x1B,0x1E,0x1E,0x19,0x14,0x12,0x10,0x12,0x10,0x11,0x0E,0x0D,0x0C,
	_W, DELAY(0), PACKET_DCS, SIZE(20), 0x54,0x1F,0x1C,0x1B,0x18,0x19,0x1A,0x1B,0x1E,0x1E,0x19,0x14,0x12,0x10,0x12,0x10,0x11,0x0E,0x0D,0x0C,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x6D,0x05,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x58,0x01,
	_W, DELAY(0), PACKET_DCS, SIZE(9), 0x55,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x33,0x38,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x32,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x35,0x25,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x63,0x05,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x4F,0x3A,
	_W, DELAY(0), PACKET_DCS, SIZE(17), 0x56,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x4E,0x3A,
	_W, DELAY(0), PACKET_DCS, SIZE(9), 0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x02,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x41,0x4B,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x73,0x30,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x74,0x10,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x76,0x36,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x77,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x28,0x31,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x7C,0x80,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x2E,0x04,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x4C,0x80,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x47,0x1F,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x48,0x09,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x50,0xC0,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x78,0x6E,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x2D,0x31,
	_W, DELAY(120), PACKET_DCS, SIZE(2), 0x4D,0x00,
	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD,0xAA,
	_W, DELAY(200), PACKET_DCS, SIZE(1), 0x11,
};

#define freq 350

#define lane_num 4
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

#define vsa_line 2
#define vbp_line 10
#define vda_line 1280
#define vfp_line 12

#define hsa_cyc  24
#define hbp_cyc  160
#define hda_cyc  400
#define hfp_cyc  160
#endif

static struct mipi_dev mipi_dev_t = {
    .info = {
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .buf_addr 		= LCD_DEV_BUF,
        .buf_num 		= LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode 		= false,
        .color 			= 0x0000ff,
        .itp_mode_en 	= false,
        .format 		= FORMAT_RGB888,
        .len 			= LEN_256,

        .rotate_en = true,				// 旋转使能
        .hori_mirror_en = false,			// 水平镜像使能
        .vert_mirror_en = true,		// 垂直镜像使能

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
    },
    {
        .x0_lane = lane_en | lane_d0 | lane_ex,
        .x1_lane = lane_en | lane_d1 | lane_ex,
        .x2_lane = lane_en | lane_clk | lane_ex,
        .x3_lane = lane_en | lane_d2 | lane_ex,
        .x4_lane = lane_en | lane_d3 | lane_ex,
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_event = SYNC_PULSE_MODE,
        .burst_mode = NON_BURST_MODE,
        .vfst_lp_en = true,
        .vlst_lp_en = true,
        .vblk_lp_en = false,
        .color_mode = COLOR_FORMAT_RGB888,
        .virtual_ch = 0,
        .hs_eotp_en = true,
        .lpdt_psel0 = true,
        .lpdt_psel1 = false,
        .lpdt_psel2 = false,
        .pixel_type = PIXEL_RGB888,

        .dsi_vdo_vsa_v  = vsa_line,
        .dsi_vdo_vbp_v  = vbp_line,
        .dsi_vdo_vact_v = vda_line,
        .dsi_vdo_vfp_v  = vfp_line,

        .dsi_vdo_hsa_v  = (lane_num * hsa_cyc) - 10,
        .dsi_vdo_hbp_v  = (lane_num * hbp_cyc) - 10,
        .dsi_vdo_hact_v = (bpp_num  * hda_cyc) / 8,
        .dsi_vdo_hfp_v  = (lane_num * hfp_cyc) - 6,

        .dsi_vdo_null_v = 0,
        .dsi_vdo_bllp_v = hsa_cyc + hbp_cyc + hda_cyc + hfp_cyc,//?+hda_cyc
    },
    {
        /* 以下参数只需修改freq */
        .tval_lpx   = ((80     * freq / 1000) / 2 - 1),
        .tval_wkup  = ((100000 * freq / 1000) / 8 - 1),
        .tval_c_pre = ((40     * freq / 1000) / 2 - 1),
        .tval_c_sot = ((300    * freq / 1000) / 2 - 1),
        .tval_c_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_c_brk = ((150    * freq / 1000) / 2 - 1),
        .tval_d_pre = ((60     * freq / 1000) / 2 - 1),
        .tval_d_sot = ((160    * freq / 1000) / 2 - 1),
        .tval_d_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_d_brk = ((150    * freq / 1000) / 2 - 1),
    },
    .target_freq = 700,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};


static int dsi_vdo_ek79030_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    printf("dsi_vdo_ek79030_init ...\n");
    //printf("lcd_reset : %d\n", lcd_reset);
    /*
     * lcd reset
     */
    if (-1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(10);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(20);
    }

    dsi_dev_init(&mipi_dev_t);

    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (-1 == data->lcd_io.backlight) {
        return;
    }
    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_DSI_VDO_EK79030),
    .logo 	 = "mipi_lcd",
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_ek79030_init,
    .bl_ctrl = mipi_backlight_ctrl,
};

#endif
