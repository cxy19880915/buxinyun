#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_HX839D_BOE686)
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list


const static u8 init_cmd_list[] = {
_W, DELAY(0),PACKET_DCS,SIZE(4), 0xB9, 0xFF, 0x83, 0x94,

_W, DELAY(0),PACKET_DCS,SIZE(3), 0xBA, 0x73, 0x83,

// Set Power HX5186 Mode /External Power Mode
_W, DELAY(0),PACKET_DCS,SIZE(16), 0xB1, 0x6C, 0x0C, 0x0D, 0x25, 0x04, 0x11, 0xF1, 0x81, 0x5C, 0xE6, 0x23, 0x80, 0xC0, 0xD2, 0x58,

_W, DELAY(0),PACKET_DCS,SIZE(13), 0xB2, 0x00, 0x64, 0x0F, 0x09, 0x24, 0x1C, 0x08, 0x08, 0x1C, 0x4D, 0x00, 0x00,

_W, DELAY(0),PACKET_DCS,SIZE(13), 0xB4, 0x00, 0xFF, 0x01, 0x5A, 0x01, 0x5A, 0x01, 0x5A, 0x01, 0x6C, 0x01, 0x6C,


// Set Power Option HX5186 Mode
_W, DELAY(0),PACKET_DCS,SIZE(4), 0xBF, 0x41, 0x0E, 0x01,

_W, DELAY(0),PACKET_DCS,SIZE(33), 0xD3, 0x00, 0x07, 0x00, 0x64, 0x07, 0x08, 0x08, 0x32, 0x10, 0x07, 0x00, 0x07, 0x32, 0x10, 0x03, 0x00, 0x03, 0x00, 0x32, 0x10, 0x08, 0x00, 0x35, 0x33, 0x09, 0x09, 0x37, 0x0D, 0x07, 0x37, 0x0E, 0x08,

// Set GIP

_W, DELAY(0),PACKET_DCS,SIZE(45), 0xD5, 0x18, 0x18, 0x24, 0x24, 0x1A, 0x1A, 0x1B, 0x1B, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x18, 0x18, 0x19, 0x19, 0x20, 0x21, 0x22,
0x23, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,



_W, DELAY(0),PACKET_DCS,SIZE(45), 0xD6, 0x19, 0x19, 0x24, 0x24, 0x1A, 0x1A, 0x1B, 0x1B, 0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04, 0x18, 0x18, 0x18, 0x18, 0x23, 0x22, 0x21,
0x20, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,



// Set Gamma
_W, DELAY(0),PACKET_DCS,SIZE(43), 0xE0, 0x00, 0x00, 0x02, 0x28, 0x2D, 0x3D, 0x0F, 0x32, 0x06, 0x09, 0x0C, 0x17, 0x0E, 0x12, 0x14, 0x12, 0x14, 0x07, 0x11, 0x12, 0x18, 0x00, 0x00,
0x03, 0x28, 0x2C, 0x3D, 0x0F, 0x32, 0x06, 0x09, 0x0B, 0x16, 0x0F, 0x11, 0x14, 0x13, 0x13, 0x07, 0x11, 0x11, 0x17,

// Set Panel
_W, DELAY(0),PACKET_DCS,SIZE(2), 0xCC, 0x01, //05 fan sao


// Set TCON Option
_W, DELAY(0),PACKET_DCS,SIZE(5), 0xC7, 0x00, 0xC0, 0x40, 0xC0,

// Set C0
_W, DELAY(0),PACKET_DCS,SIZE(3), 0xC0, 0x30, 0x14,

// Set VCOM
_W, DELAY(0),PACKET_DCS,SIZE(3), 0xB6, 0x46, 0x46,

// Set ECO
_W, DELAY(0),PACKET_DCS,SIZE(3), 0xC6, 0x3D, 0x00,

_W, DELAY(120),PACKET_DCS,SIZE(1), 0x11,

// Display ON
_W, DELAY(60),PACKET_DCS,SIZE(1), 0x29,

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

#define vsa_line 2
#define vbp_line 17
#define vda_line 1280
#define vfp_line 50

#define hsa_cyc  8
#define hbp_cyc  42
#define hda_cyc  480
#define hfp_cyc  80

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

        .hori_mirror_en = false,		// 水平镜像使能
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

    .target_freq = 800,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = true,
};
/* 屏复位 */
void rm68200gai_reset(unsigned int reset_gpio)
{
    gpio_direction_output(reset_gpio, 0);
    delay_2ms(5);
}
/* 屏释放复位 */
void rm68200gai_reset_release(unsigned int reset_gpio)
{
    delay_2ms(5);
    gpio_direction_output(reset_gpio, 1);
}

static int dsi_vdo_hx839d_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if (-1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(20);
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(20);
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

//void rm68200gai_esd_check(struct esd_check *esd)
//{
//    static u32 errcnt = 0;
//    u8 param = 0;
//    u8 buf[8];
//    return;
//    if (esd->en) {
//        esd->count ++;
//        /* dcs_read_parm(0x04,buf,3); */
//        /* dcs_read_parm(0x30,buf,5); */
//        /* generic_read_parm(0x45,buf,2); */
//        /* generic_read_parm(0x0a,buf,1); */
//        param = 0;
//        dcs_read_parm(0x0a, &param, 1);
//        /* dcs_read_parm(0xa1,buf,5); */
//        /* dcs_read_parm(0x45,buf,2); */
//
//        if (param != 0x9c) {
//            errcnt ++;
//            printf("errcnt : 0x%x\n", errcnt);
//            if (errcnt == 6) {
//                errcnt = 0;
//
//                lcd_reinit();
//            }
//        }
//
//        if (esd->count > 10) { //dsi中断没来
//            /* lcd_reinit(); */
//            cpu_reset();
//        }
//    }
//}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_DSI_VDO_4LANE_HX839D_BOE686),
    .logo 	 = LCD_NAME,
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_hx839d_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .esd = {
        .interval = 500,
        .esd_check_isr = NULL,
    }
};

#endif
