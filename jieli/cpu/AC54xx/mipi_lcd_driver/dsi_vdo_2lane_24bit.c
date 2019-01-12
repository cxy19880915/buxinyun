#include "generic/typedef.h"
#include "asm/cpu.h"
#include "asm/dsi.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#if (LCD_TYPE == LCD_DSI_DCS_2LANE_24BIT)
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
const static u8 init_cmd_list[] = {
    _W, DELAY(1), SIZE(4),  0xB9, 0xFF, 0x83, 0x79,
    _W, DELAY(1), SIZE(21), 0xB1, 0x44, 0x12, 0x12, 0x31, 0x31, 0x50, 0xd0, 0xee, 0x54, 0x80, 0x38, 0x38, 0xf8, 0x33, 0x32, 0x22, 0x00, 0x80, 0x30, 0x00,
    _W, DELAY(1), SIZE(10), 0xB2, 0x80, 0xFE, 0x0A, 0x04, 0x00, 0x50, 0x11, 0x42, 0x1D,
    _W, DELAY(1), SIZE(11), 0xB4, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x22, 0x80, 0x23, 0x80,
    _W, DELAY(1), SIZE(2),  0xCC, 0x02,
    _W, DELAY(1), SIZE(2),  0xD2, 0x11,
    _W, DELAY(1), SIZE(30), 0xD3, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x10, 0x03, 0x00, 0x03, 0x03, 0x60, 0x03, 0x60, 0x00, 0x08, 0x00, 0x08, 0x45, 0x44, 0x08, 0x08, 0x37, 0x08, 0x08, 0x37, 0x09,
    _W, DELAY(1), SIZE(33), 0xD5, 0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x20, 0x21, 0x24, 0x25, 0x18, 0x18, 0x18, 0x18, 0x00, 0x01, 0x04, 0x05, 0x02, 0x03, 0x06, 0x07, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    _W, DELAY(1), SIZE(33), 0xD6, 0x18, 0x18, 0x18, 0x18, 0x19, 0x19, 0x25, 0x24, 0x21, 0x20, 0x18, 0x18, 0x18, 0x18, 0x05, 0x04, 0x01, 0x00, 0x03, 0x02, 0x07, 0x06, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    _W, DELAY(1), SIZE(43), 0xE0, 0x00, 0x00, 0x00, 0x06, 0x08, 0x3f, 0x11, 0x29, 0x05, 0x09, 0x0c, 0x17, 0x10, 0x14, 0x16, 0x15, 0x15, 0x08, 0x13, 0x14, 0x18, 0x00, 0x00, 0x00, 0x06, 0x08, 0x3f, 0x11, 0x29, 0x05, 0x09, 0x0c, 0x17, 0x10, 0x14, 0x16, 0x15, 0x15, 0x08, 0x13, 0x14, 0x18,
    _W, DELAY(1), SIZE(3),  0xB6, 0x42, 0x42,
    _W, DELAY(1), SIZE(2),  0x35, 0x00,
    _W, DELAY(1), SIZE(2),  0x51, 0xFF,
    _W, DELAY(1), SIZE(2),  0x53, 0x24,
    _W, DELAY(1), SIZE(2),  0x55, 0x01,
    /* _W,DELAY(1),SIZE(1), 0x3A,0x70, */
    /* _W,DELAY(1),SIZE(1), 0x26,0x01, */
    _W, DELAY(20), SIZE(1), 0x11,
    _W, DELAY(4), SIZE(1),  0x29,
};

#define freq 321

#define lane_num 2
#define bpp_num  24

//垂直时序要求比较严
#define vsa_line 1
#define vbp_line 10
#define vda_line 854
#define vfp_line 16

#define hsa_cyc  8
#define hbp_cyc  16
#define hda_cyc  480
#define hfp_cyc  16
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
        },
    },
    {
        .x0_lane = lane_en | lane_d0,
        .x1_lane = lane_en | lane_d1,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_dis,
        .x4_lane = lane_dis,
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,//视频模式 类似于dump panel
        .sync_event = SYNC_PULSE_MODE,//同步事件
        .burst_mode = NON_BURST_MODE,//高速模式
        .vfst_lp_en = true,//帧头进LowPower 同步用
        .vlst_lp_en = true,//帧尾进LowPower 同步用
        .vblk_lp_en = false,//blank进LowPower 省电
        .color_mode = COLOR_FORMAT_RGB888,
        .virtual_ch   = 0,
        .hs_eotp_en = false,
        .lpdt_psel0 = true,//选择具体在哪里插入命令
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

        .dsi_vdo_null_v = 0,//插入空包
        .dsi_vdo_bllp_v = hsa_cyc + hbp_cyc + hda_cyc + hda_cyc,
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
    .target_freq = 642,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};

//------------------------------------------------------//
// dsi run
//------------------------------------------------------//
static int dsi_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if (-1 != lcd_reset) {
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
    .enable = IF_ENABLE(LCD_DSI_DCS_2LANE_24BIT),
    .logo = "mipi_2lane_24bit",
    .type = LCD_MIPI,
    .init = dsi_init,
    .dev  = &mipi_dev_t,
    .bl_ctrl = mipi_backlight_ctrl,
};
#endif
