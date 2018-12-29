#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "os/os_compat.h"

#ifdef LCD_DSI_VDO_4LANE_MIPI_NT35523
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list
//KLD50113XHIA


const static u8 init_cmd_list[] = {

#if 10

    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(1), 0xC0,

    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC8, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB1, 0xE8, 0x21,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB5, 0x05, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBB, 0x93, 0x93,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBC, 0x0F, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xBD, 0x11, 0x30, 0x10, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01,

    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xD7, 0x00, 0xFF,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB7, 0x00, 0x6C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCE, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCA, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB3, 0x23, 0x23,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB4, 0x23, 0x23,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC3, 0x5A, 0x5A,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC4, 0x5A, 0x5A,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC2, 0x5A, 0x5A,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB9, 0x34, 0x34,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBA, 0x34, 0x34,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBC, 0x50, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBD, 0x50, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBE, 0x00, 0x71,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBF, 0x00, 0x71,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xB0, 0x00, 0x00, 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xB1, 0x00, 0x00, 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(8), 0xB2, 0x00, 0x00, 0x0A, 0x06, 0x00, 0xF0, 0x5B,
    _W, DELAY(0), PACKET_DCS, SIZE(8), 0xB3, 0x00, 0x00, 0x09, 0x06, 0x00, 0xF0, 0x5B,
    _W, DELAY(0), PACKET_DCS, SIZE(11), 0xB6, 0xF0, 0x05, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(11), 0xB7, 0xF0, 0x05, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(8), 0xBC, 0xC5, 0x03, 0x00, 0x08, 0x00, 0xF0, 0x5B,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC4, 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xB0, 0x33, 0x04, 0x00, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB1, 0x40, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(4), 0xB2, 0x03, 0x02, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xB3, 0x83, 0x23, 0x42, 0x97,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xB4, 0xC5, 0x35, 0x77, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(8), 0xB5, 0x4C, 0xE5, 0x31, 0x33, 0x33, 0xA3, 0x0A,
    _W, DELAY(0), PACKET_DCS, SIZE(7), 0xB6, 0x00, 0x00, 0xD5, 0x31, 0x77, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB9, 0x00, 0x00, 0x00, 0x05, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xC0, 0x35, 0x33, 0x33, 0x50, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xC6, 0x00, 0x00, 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xCE, 0xF0, 0x1F,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xD2, 0x00, 0x25, 0x02, 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xE7, 0xE8, 0xFF,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xE8, 0xFF, 0xFF,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE9, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEA, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEB, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEC, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEE, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x06,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB0, 0x7D, 0x4A, 0x7D, 0x7D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB1, 0x7D, 0x7D, 0x42, 0x5D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB2, 0x7D, 0x63, 0x61, 0x7D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB3, 0x5F, 0x72, 0x7D, 0x7D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB4, 0x7D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB5, 0x7D, 0x48, 0x7D, 0x7D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB6, 0x7D, 0x7D, 0x40, 0x5C, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB7, 0x7D, 0x62, 0x60, 0x7D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB8, 0x5E, 0x72, 0x7D, 0x7D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB9, 0x7D, 0x7D,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB0, 0x42,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0xD1, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x3F, 0x00, 0x5B, 0x00, 0x71, 0x00, 0x97, 0x00, 0xB5, 0x00, 0xE6,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0xD2, 0x01, 0x0D, 0x01, 0x4A, 0x01, 0x7B, 0x01, 0xC7, 0x02, 0x03, 0x02, 0x05, 0x02, 0x3A, 0x02, 0x73,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0xD3, 0x02, 0x97, 0x02, 0xCB, 0x02, 0xEE, 0x03, 0x20, 0x03, 0x41, 0x03, 0x6D, 0x03, 0x8C, 0x03, 0xAB,

    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xB4, 0x03, 0xBC, 0x03, 0xBE,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xFF, 0xAA, 0x55, 0xA5, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF3, 0xC0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x35, 0x00,
    _W, DELAY(120), PACKET_DCS, SIZE(1), 0x11,
    _W, DELAY(20), PACKET_DCS, SIZE(1), 0x29,
#endif
};

#define freq 400

#define lane_num 4
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

#define vsa_line 20
#define vbp_line 30//17
#define vda_line 1280
#define vfp_line 30//10

#define hsa_cyc  30//8
#define hbp_cyc  40//42
#define hda_cyc  720
#define hfp_cyc  40//44

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

        .rotate_en 		= true,				// 旋转使能
        .hori_mirror_en = true,		// 水平镜像使能
        .vert_mirror_en = false,			// 垂直镜像使能

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
        //touch V1
        /* .x0_lane = lane_en | lane_d3, */
        /* .x1_lane = lane_en | lane_d2, */
        /* .x2_lane = lane_en | lane_clk, */
        /* .x3_lane = lane_en | lane_d1, */
        /* .x4_lane = lane_en | lane_d0, */

        //5403
        /* .x0_lane =  lane_dis, */
        /* .x1_lane =  lane_en | lane_clk, */
        /* .x2_lane =  lane_en | lane_d1, */
        /* .x3_lane =  lane_en | lane_d0, */
        /* .x4_lane =  lane_dis, */
        /* //touch v1 bbm 2 */
        .x0_lane = lane_en | lane_d3,
        .x1_lane = lane_en | lane_d2,
        .x2_lane = lane_en  | lane_clk,
        .x3_lane = lane_en  | lane_d1,
        .x4_lane = lane_en  | lane_d0,

        /* .x4_lane = lane_en  | lane_d1, */
        /* .x3_lane = lane_en  | lane_d0, */

        /* //touch v1 bbm */
        /* .x0_lane = lane_en  | lane_d3, */
        /* .x1_lane = lane_en  | lane_d2, */
        /* .x2_lane = lane_en  | lane_clk, */
        /* .x3_lane = lane_en  | lane_d1, */
        /* .x4_lane = lane_en  | lane_d0, */
        //lingxiang
        /* .x0_lane = lane_en | lane_ex | lane_d0, */
        /* .x1_lane = lane_en | lane_ex | lane_d1, */
        /* .x2_lane = lane_en | lane_ex | lane_clk, */
        /* .x3_lane = lane_en | lane_ex | lane_d2, */
        /* .x4_lane = lane_en | lane_ex | lane_d3, */
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
    .target_freq = 800,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    /* .debug_mode = true, */
    .debug_mode = false,
};

void read_data_func(void *p)
{
    u8 status;
    u8 param;
    static u8 cnt = 0;
    while (1) {
        dsi_task_con |= BIT(7);
        status = dcs_send_short_p1_bta(0xCD, 0xAA);
        printf("staus0=%d\n", status);
        delay(0x100);
        status = dcs_send_short_p0_bta(0x11);
        printf("staus1=%d\n", status);
        delay_10ms(120 / 10 + 1);

        status = dcs_read_parm(0xfa, &param, 0x01);
        delay(0x100);
        printf("\nparam=%d 0x%x\n", param, status);
        if (status == 10) {
            cnt++;
            printf("read data ok %d\n", cnt);
        }
        dsi_task_con &= ~BIT(7);

    }

}
static int dsi_vdo_ili9881c_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if ((u8) - 1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(10);
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(10);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(60);
        /* puts("-------------lcd reset \n"); */
    }

    dsi_dev_init(&mipi_dev_t);

    /* thread_fork("read_data_task",20,4000,0,0,read_data_func,NULL); */

    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if ((u8) - 1 == data->lcd_io.backlight) {
        return;
    }
    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
    /* puts("-------------lcd blacklight \n"); */
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_ili9881c_init,
    .bl_ctrl = mipi_backlight_ctrl,
};

#endif
