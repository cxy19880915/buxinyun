#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#ifdef LCD_DSI_VDO_4LANE_MIPI

#define freq 300

#define lane_num 4
#define bpp_num  24

#define vsa_line 20
#define vbp_line 30
#define vda_line 1280
#define vfp_line 30

#define hsa_cyc  30
#define hbp_cyc  40
#define hda_cyc  480
#define hfp_cyc  40

static struct mipi_dev mipi_dev_t = {
    .info = {
        .xres = LCD_DEV_WIDTH,
        .yres = LCD_DEV_HIGHT,
        .buf_addr = LCD_DEV_BUF,
        .buf_num = LCD_DEV_BNUM,
        .sample = LCD_DEV_SAMPLE,
        .test_mode = false,//测试模式
        .color = 0x0000ff,
        .itp_mode_en = false,
        .format = FORMAT_RGB888,

        .rotate_en = true,				// 旋转使能
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
        .x0_lane = lane_en | lane_d3,
        .x1_lane = lane_en | lane_d2,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_en | lane_d1,
        .x4_lane = lane_en | lane_d0,
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_event = SYNC_PULSE_MODE,
        .burst_mode = NON_BURST_MODE,
        .vfst_lp_en = true,
        .vlst_lp_en = true,
        .vblk_lp_en = false,
        .color_mode = COLOR_FORMAT_RGB888,
        .virtual_ch	= 0,
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
        .dsi_vdo_bllp_v = hsa_cyc + hbp_cyc + hda_cyc + hfp_cyc,// + hda_cyc,
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
    .target_freq = 600,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = NULL,
    .cmd_list_item = 0,
    .debug_mode = false,
};

static int dsi_vdo_mipi_init(void *_data)
{
    printf("dsi_vdo_mipi init...\n");
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u32 lcd_reset = data->lcd_io.lcd_reset;
    printf("lcd_reset : %d\n", lcd_reset);
    //reset pin
    gpio_direction_output(lcd_reset, 0);
    delay(0xffff);
    gpio_direction_output(lcd_reset, 1);
    delay(0xffff);
    delay(0xffff);
    delay(0xffff);
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
    .init = dsi_vdo_mipi_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .dev  = &mipi_dev_t,
};

#endif
