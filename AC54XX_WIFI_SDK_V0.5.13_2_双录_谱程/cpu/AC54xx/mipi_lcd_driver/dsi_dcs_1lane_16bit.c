#include "generic/typedef.h"
#include "asm/cpu.h"
#include "asm/dsi.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_DSI_DCS_1LANE_16BIT

const static u8 init_cmd_list[] = {
    _W, DELAY(20), PACKET_DCS, SIZE(1), 0x01, //sw reset
    _W, DELAY(20), PACKET_DCS, SIZE(1), 0x11, //exit sleep mode
    _R, DELAY(1), PACKET_DCS, SIZE(1), 0x0a,
    _W, DELAY(1), PACKET_DCS, SIZE(2), 0x3a, 0x55, //set pixel format 16bit
    _W, DELAY(1), PACKET_DCS, SIZE(2), 0x36, 0x01, //set address mode
    _W, DELAY(1), PACKET_DCS, SIZE(2), 0x26, 0x01, //set gamma curve
    _W, DELAY(1), PACKET_DCS, SIZE(2), 0x51, 0xca,
    _W, DELAY(1), PACKET_DCS, SIZE(2), 0x53, 0x2c,
    _W, DELAY(1), PACKET_DCS, SIZE(9), 0x58, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
    _W, DELAY(1), PACKET_DCS, SIZE(10), 0xf9, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x30, 0x30, 0x30,
    _W, DELAY(1), PACKET_DCS, SIZE(1), 0x29, //display on
    _R, DELAY(1), PACKET_DCS, SIZE(1), 0x0a,
};

#define freq   500

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

        .rotate_en 		= true,			// 旋转使能
        .hori_mirror_en = true,			// 水平镜像使能
        .vert_mirror_en = false,		// 垂直镜像使能

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
        .x0_lane = lane_dis,
        .x1_lane = lane_dis,
        .x2_lane = lane_dis,
        .x3_lane = lane_en | lane_d0,
        .x4_lane = lane_en | lane_clk,
    },
    {
        .video_mode = VIDEO_STREAM_COMMAND,//视频模式 类似于dump panel
        .sync_event = SYNC_PULSE_MODE,//同步事件
        .burst_mode = NON_BURST_MODE,//高速模式
        .vfst_lp_en = false,//帧头进LowPower 同步用
        .vlst_lp_en = false,//帧尾进LowPower 同步用
        .vblk_lp_en = false,//blank进LowPower 省电
        .color_mode = COLOR_FORMAT_RGB565,
        .virtual_ch = 0,
        .hs_eotp_en = false,
        .lpdt_psel0 = false,//选择具体在哪里插入命令
        .lpdt_psel1 = false,
        .lpdt_psel2 = false,
        .pixel_type = PIXEL_RGB565_COMMAND,

        .dsi_vdo_vsa_v  = 0,
        .dsi_vdo_vbp_v  = 16,
        .dsi_vdo_vact_v = 640,
        .dsi_vdo_vfp_v  = 0,

        .dsi_vdo_hsa_v  = 0,
        .dsi_vdo_hbp_v  = 0,
        .dsi_vdo_hact_v = 360 * 2 + 1, //+1 for DCS command
        .dsi_vdo_hfp_v  = 0,

        .dsi_vdo_null_v = 360 * 2, //插入空包
        .dsi_vdo_bllp_v = 360 * 2 * 2,
    },
    {
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
    .target_freq = 1000,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};



static int dsi_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if (0xff != lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);
    }

    dsi_dev_init(&mipi_dev_t);

    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type = LCD_MIPI,
    .init = dsi_init,
    .dev  = &mipi_dev_t,
    .bl_ctrl = mipi_backlight_ctrl,
};
#endif
