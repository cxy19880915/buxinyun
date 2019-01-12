#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_1280x480_LX68HD)
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list
const static u8 init_cmd_list[] = {
    _W, DELAY(0), PACKET_DCS,SIZE(3), 0xF0, 0x5A,0x5A,
    _W, DELAY(0), PACKET_DCS,SIZE(3), 0xF1, 0xA5,0xA5,
    _W, DELAY(0), PACKET_DCS,SIZE(3), 0xF0, 0xB4,0x4B,
    _W, DELAY(0),PACKET_DCS, SIZE(3), 0xB6, 0x3C,0x3C,
    _W, DELAY(0), PACKET_DCS,SIZE(23), 0xB3, 0x1D,0x08,0x1E,0x1F,0x10,0x12,0x0C,0x0E,0x00,0x1C,0x04,0x06,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    _W, DELAY(0), PACKET_DCS,SIZE(23), 0xB4, 0x1D,0x08,0x1E,0x1F,0x11,0x13,0x0D,0x0F,0x00,0x1C,0x05,0x07,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    _W, DELAY(0), PACKET_DCS,SIZE(14),0xB0,0x76,0x54,0xBB,0xBB,0x33,0x33,0x33,0x33,0x22,0x02,0x6E,0x02,0x0E,
    _W, DELAY(0), PACKET_DCS,SIZE(10),0xB1, 0x53,0xA0,0x00,0x60,0x22,0x02,0x6E,0x02,0x5F,

    _W, DELAY(0), PACKET_DCS,SIZE(11),0xB2,0x37,0x09,0x08,0x89,0x08,0x00,0x22,0x00,0x44,0xD9,
    _W, DELAY(0), PACKET_DCS,SIZE(7), 0xBD,0x4E,0x0E,0x41,0x41,0x11,0x1E,
    _W, DELAY(0), PACKET_DCS,SIZE(18),0xB7,0x01,0x01,0x09,0x11,0x0D,0x15,0x19,0x0D,0x21,0x1D,0x00,0x00,0x20,0x00,0x02,0xFF,0x3C,
    _W, DELAY(0), PACKET_DCS,SIZE(6),0xB8,0x23,0x01,0x30,0x34,0x53,
    _W, DELAY(0), PACKET_DCS,SIZE(3),0xBA,0x13,0x63,
    _W, DELAY(0), PACKET_DCS,SIZE(7),0xC1,0x0F,0x0E,0x01,0x36,0x3A,0x08,
    _W, DELAY(0), PACKET_DCS,SIZE(3),0xC2,0x11,0x40,
    _W, DELAY(0), PACKET_DCS,SIZE(4),0xC3,0x22,0x31,0x04,
    _W, DELAY(0), PACKET_DCS,SIZE(6),0xC7,0x05,0x23,0x6B,0x41,0x00,
    _W, DELAY(0), PACKET_DCS,SIZE(39),0xC8,0x7C,0x63,0x54,0x48,0x47,0x39,0x3F,0x29,0x43,0x41,0x41,0x61,0x53,0x5F,0x53,0x50,0x44,0x30,0x06,0x7C,0x63,0x54,0x48,0x47,0x39,0x3F,0x29,0x43,0x41,0x41,0x61,0x53,0x5F,0x53,0x50,0x44,0x30,0x06,

    _W, DELAY(0), PACKET_DCS,SIZE(2),0x35,0x01,
    _W, DELAY(0), PACKET_DCS,SIZE(2),0x36,0x14,

    _W, DELAY(120), PACKET_DCS,SIZE(2),0x11, 0x00,

    _W, DELAY(0), PACKET_DCS,SIZE(2),0x29, 0x00,
};

#define freq 300

#define lane_num 4
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

#define vsa_line 2
#define vbp_line 17
#define vda_line 1280
#define vfp_line 10

#define hsa_cyc  4
#define hbp_cyc  12
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
        .x0_lane = lane_en |lane_ex| lane_d0,
        .x1_lane = lane_en |lane_ex| lane_d1,
        .x2_lane = lane_en |lane_ex| lane_clk,
        .x3_lane = lane_en |lane_ex| lane_d2,
        .x4_lane = lane_en |lane_ex| lane_d3,
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
    .target_freq = 600,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = true,
};

static int dsi_vdo_rm68200gai_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    //pmsg("LX686 reset\n");
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

    if (-1 == data->lcd_io.backlight) {
        return;
    }
    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

static void rm68200gai_esd_check(struct esd_check *esd)
{
    static u32 errcnt = 0;
    u8 param = 0;
    u8 buf[8];
    return;
    if (esd->en) {
        esd->count ++;
        /* dcs_read_parm(0x04,buf,3); */
        /* dcs_read_parm(0x30,buf,5); */
        /* generic_read_parm(0x45,buf,2); */
        /* generic_read_parm(0x0a,buf,1); */
        param = 0;
        dcs_read_parm(0x0a, &param, 1);
        /* dcs_read_parm(0xa1,buf,5); */
        /* dcs_read_parm(0x45,buf,2); */

        if (param != 0x9c) {
            errcnt ++;
            printf("errcnt : 0x%x\n", errcnt);
            if (errcnt == 6) {
                errcnt = 0;

                lcd_reinit();
            }
        }

        if (esd->count > 10) { //dsi中断没来
            /* lcd_reinit(); */
            cpu_reset();
        }
    }
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_DSI_VDO_4LANE_1280x480_LX68HD),
    .logo 	 = LCD_NAME,
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_rm68200gai_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .esd = {
        .interval = 500,
        .esd_check_isr = rm68200gai_esd_check,
    }
};

#endif
