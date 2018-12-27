#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_DSI_VDO_4LANE_MIPI_ICN9706
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list

#define lane_num 2

const static u8 init_cmd_list[] = {
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xF0, 0x5A, 0x5A,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xF1, 0xA5, 0xA5,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xF0, 0xB4, 0x4B,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB6, 0x3C, 0x3C,
    _W, DELAY(0), PACKET_DCS, SIZE(23), 0xB3, 0x1D, 0x08, 0x1E, 0x1F, 0x10, 0x12, 0x0C, 0x0E, 0x00, 0x1C, 0x04, 0x06, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(23), 0xB4, 0x1D, 0x08, 0x1E, 0x1F, 0x11, 0x13, 0x0D, 0x0F, 0x00, 0x1C, 0x05, 0x07, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(14), 0xB0, 0x76, 0x54, 0xBB, 0xBB, 0x33, 0x33, 0x33, 0x33, 0x22, 0x02, 0x6E, 0x02, 0x0E,
    _W, DELAY(0), PACKET_DCS, SIZE(10), 0xB1, 0x53, 0xA0, 0x00, 0x60, 0x22, 0x02, 0x6E, 0x02, 0x5F,
    _W, DELAY(0), PACKET_DCS, SIZE(11), 0xB2, 0x37, 0x09, 0x08, 0x89, 0x08, 0x00, 0x22, 0x00, 0x44, 0xD9,
    _W, DELAY(0), PACKET_DCS, SIZE(7), 0xBD, 0x4E, 0x0E, 0x41, 0x41, 0x11, 0x1E,
    _W, DELAY(0), PACKET_DCS, SIZE(18), 0xB7, 0x01, 0x01, 0x09, 0x11, 0x0D, 0x15, 0x19, 0x0D, 0x21, 0x1D, 0x00, 0x00, 0x20, 0x00, 0x02, 0xFF, 0x3C,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xB8, 0x23, 0x01, 0x30, 0x34, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBA, 0x13, 0x63,
    _W, DELAY(0), PACKET_DCS, SIZE(7), 0xC1, 0x0F, 0x0E, 0x01, 0x36, 0x3A, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC2, 0x11, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(4), 0xC3, 0x22, 0x31, 0x04,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xC7, 0x05, 0x23, 0x6B, 0x41, 0x00,
    /* _W, DELAY(0), PACKET_DCS, SIZE(39),0xC8,0x7C,0x5E,0x4D,0x41,0x3D,0x2F,0x33,0x1D,0x35,0x35,0x35,0x54,0x43,0x4C,0x3E,0x3C,0x2F,0x1C,0x06,0x7C,0x5E,0x4D,0x41,0x3D,0x2F,0x33,0x1D,0x35,0x35,0x35,0x54,0x43,0x4C,0x3E,0x3C,0x2F,0x1C,0x06,//GAMMA2.2 */
    _W, DELAY(0), PACKET_DCS, SIZE(39), 0xC8, 0x7C, 0x60, 0x50, 0x44, 0x41, 0x33, 0x38, 0x23, 0x3B, 0x3A, 0x3B, 0x5A, 0x4A, 0x55, 0x48, 0x46, 0x39, 0x25, 0x06, 0x7C, 0x60, 0x50, 0x44, 0x41, 0x33, 0x38, 0x23, 0x3B, 0x3A, 0x3B, 0x5A, 0x4A, 0x55, 0x48, 0x46, 0x39, 0x25, 0x06, //GAMMA2.0
    /* _W, DELAY(0), PACKET_DCS, SIZE(39),0xC8,0x7C,0x62,0x53,0x48,0x46,0x38,0x3D,0x28,0x41,0x40,0x41,0x61,0x52,0x5E,0x52,0x50,0x44,0x30,0x06,0x7C,0x62,0x53,0x48,0x46,0x38,0x3D,0x28,0x41,0x40,0x41,0x61,0x52,0x5E,0x52,0x50,0x44,0x30,0x06,//GAMMA1.8  */
    /* _W, DELAY(0), PACKET_DCS, SIZE(39),0xC8,0x7C,0x64,0x56,0x4B,0x49,0x3C,0x43,0x2F,0x48,0x48,0x49,0x6A,0x5C,0x69,0x5B,0x5C,0x50,0x3C,0x06,0x7C,0x64,0x56,0x4B,0x49,0x3C,0x43,0x2F,0x48,0x48,0x49,0x6A,0x5C,0x69,0x5B,0x5C,0x50,0x3C,0x06,//GAMMA1.6 */

#if  (lane_num == 2)
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD2, 0x61, //2lane
#endif

    /* _W, DELAY(0), PACKET_DCS, SIZE( 4),0xD0,0x00,0xFF,0xFF, */
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x35, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x36, 0x14,

    _W, DELAY(120), PACKET_DCS, SIZE(2), 0x11, 0x00,
    _W, DELAY(120), PACKET_DCS, SIZE(2), 0x29, 0x00,
};

#define freq 266

/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

#define vsa_line 2
#define vbp_line 14
#define vda_line 1280
#define vfp_line 16

#define hsa_cyc  8
#define hbp_cyc  32//
#define hda_cyc  480
#define hfp_cyc  32//


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
        .hori_mirror_en = false,//true,			// 水平镜像使能
        .vert_mirror_en = true,//false,		// 垂直镜像使能

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
#if (lane_num == 2)
        //5403
        .x0_lane = lane_dis,
        .x1_lane =  lane_en | lane_clk,
        .x2_lane =  lane_en | lane_d1,
        .x3_lane =  lane_en | lane_d0,
        .x4_lane = lane_dis,
#endif

        //lingxiang
        /* .x0_lane = lane_en | lane_ex | lane_d0, */
        /* .x1_lane = lane_en | lane_ex | lane_d1, */
        /* .x2_lane = lane_en | lane_ex | lane_clk, */
        /* .x3_lane = lane_en | lane_ex | lane_d2, */
        /* .x4_lane = lane_en | lane_ex | lane_d3, */

        //socket
        /* .x0_lane = lane_en | lane_clk, */
        /* .x1_lane = lane_en | lane_d2, */
        /* .x2_lane = lane_en | lane_d1, */
        /* .x3_lane = lane_en | lane_d3, */
        /* .x4_lane = lane_en | lane_d0, */
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_event = SYNC_PULSE_MODE,
        .burst_mode = NON_BURST_MODE,//
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
    .target_freq = 600,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};

static int dsi_vdo_icn9706_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /* gpio_direction_output(IO_PORTH_07, 0); */

    /*
     * lcd reset
     */
    if ((u8) - 1 != lcd_reset) {
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
    static u8 startup = 0;
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if ((u8) - 1 == data->lcd_io.backlight) {
        return;
    }
    if (on) {
        if (!startup) {
            //解决启动瞬间的闪屏
            startup = 1;
            delay_2ms(10);
        }
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

/* void icn9706_esd_check(struct esd_check *esd) */
/* { */
/*     static u32 errcnt = 0; */
/*  */
/*     if (esd->en) { */
/*         lcd_reinit(); */
/*         esd->count ++; */
/*         if (esd->count > 10) { //dsi中断没来 */
/*             #<{(| lcd_reinit(); |)}># */
/*             cpu_reset(); */
/*         } */
/*     } */
/* } */
REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_icn9706_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .esd = {
        /* .interval = 500, */
        /* .esd_check_isr = icn9706_esd_check, */
    }
};

#endif
