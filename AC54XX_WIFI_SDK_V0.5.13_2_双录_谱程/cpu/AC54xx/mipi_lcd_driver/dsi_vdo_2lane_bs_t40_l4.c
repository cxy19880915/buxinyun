#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "asm/dsi.h"
#include "asm/port_waked_up.h"


#ifdef LCD_DSI_DCS_2LANE_24BIT_BS_T40_L4
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
const static u8 init_cmd_list[] = {
    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xb0, 0x04,
    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xd6, 0x28,

//20ms
    _W, DELAY(0), PACKET_GENERIC, SIZE(16), 0xc1, 0x63, 0x31, 0x00, 0x27, 0x27, 0x32, 0x12, 0x28,  0xde/* 0xc2~0xC8*/, 0x9e,/*0x82~0x88*//*0x46,0x10,*/ 0xa5, 0x0f, 0x58, 0x21, 0x01,
    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xFA, 0x03,


    _W, DELAY(0), PACKET_DCS, SIZE(5), 0x2A, 0x00, 0x00, 0x01, 0xDF,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0x2B, 0x00, 0x00, 0x03, 0x1f,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A, 0x77,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x36, 0x11,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x35, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0x44, 0x00, 0x00,
    _W, DELAY(120), PACKET_DCS, SIZE(1), 0x11,
    _W, DELAY(50), PACKET_DCS, SIZE(1), 0x29,
};

/* #define freq 321*///49fps-->需要Te 1000/49 = 20.4
#define freq 348    //53fps-->需要Te  1000/53 = 18.8


#define lane_num 2

#define bpp_num  24

//垂直时序要求比较严
#define vsa_line 4//1
#define vbp_line 8//23
#define vda_line 800
#define vfp_line 6

#define hsa_cyc  80
#define hbp_cyc  70
#define hda_cyc  480
#define hfp_cyc  120

static struct mipi_dev mipi_dev_t = {
    .info = {
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .buf_addr 		= LCD_DEV_BUF,
        .buf_num 		= LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode 		= false,
        /* .test_mode 		= true, */
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
        .x0_lane = lane_en | lane_clk,
        .x1_lane = lane_en | lane_d0,
        .x2_lane = lane_en | lane_d1,
        .x3_lane = lane_dis,
        .x4_lane = lane_dis,

        /*      .x0_lane = lane_dis,//lane_en | lane_clk, */
        /* .x1_lane = lane_dis, */
        /* .x2_lane = lane_en | lane_clk, */
        /* .x3_lane = lane_en | lane_d1, */
        /* .x4_lane = lane_en | lane_d0, */


    },
    {

        .video_mode = VIDEO_STREAM_COMMAND,//视频模式 类似于dump panel
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

#if 1
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
#else

        .dsi_vdo_vsa_v  = 0,
        .dsi_vdo_vbp_v  = 16,
        .dsi_vdo_vact_v = 800,
        .dsi_vdo_vfp_v  = 0,

        .dsi_vdo_hsa_v  = 0,
        .dsi_vdo_hbp_v  = 0,
        .dsi_vdo_hact_v = 480 * 3 + 1, //+1 for DCS command
        .dsi_vdo_hfp_v  = 0,

        .dsi_vdo_null_v = 480 * 3, //插入空包
        .dsi_vdo_bllp_v = 480 * 2 * 2,

#endif
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
    .te_mode = {
        .te_mode_en = true,
        /* .te_mode_en = false, */
        .event = EVENT_IO_2,
        .edge = EDGE_POSITIVE,
        /* .edge = EDGE_NEGATIVE, */
        .gpio = IO_PORTG_03,
    },
    .target_freq = (freq * 2),
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};

static int dsi_vdo_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if ((u8) - 1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(100);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(100);
    }

    dsi_dev_init(&mipi_dev_t);

    return 0;
}
#include "os/os_api.h"
static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if ((u8) - 1 == data->lcd_io.backlight) {
        return;
    }
    /* os_time_dly(50); */
    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .bl_ctrl_flags = BL_CTRL_BACKLIGHT,
    /* .te_mode_dbug = true, */
};


#endif

u32  set_retry_cnt()
{
    return 20;
}
