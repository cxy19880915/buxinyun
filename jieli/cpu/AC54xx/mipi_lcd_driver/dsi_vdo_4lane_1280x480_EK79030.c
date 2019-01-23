#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_1280x480_EK79030)
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list


const static u8 init_cmd_list[] = {
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0xCD,0xAA,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x65,0x08,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x3A,0x14,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x32,0x00,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x36,0x02,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x67,0x82,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x69,0x20,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x6D,0x01,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x68,0x00,

    _W, DELAY(0),PACKET_DCS,SIZE(20), 0x53,0x19,0x17,0x15,0x12,0x12,0x12,0x13,0x15,0x15,0x10,0x0C,0x0A,0x0A,0x0C,0x0B,0x0C,0x09,0x07,0x06,
    _W, DELAY(0),PACKET_DCS,SIZE(20), 0x54,0x19,0x16,0x14,0x11,0x11,0x11,0x13,0x15,0x15,0x10,0x0B,0x09,0x09,0x0B,0x0B,0x0C,0x09,0x07,0x06,

    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x29,0x10,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x2A,0x0C,

    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x6C,0x81,
    _W, DELAY(0),PACKET_DCS,SIZE(9), 0x55,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,
    _W, DELAY(0),PACKET_DCS,SIZE(17), 0x56,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,
    _W, DELAY(0),PACKET_DCS,SIZE(5), 0x57,0x00,0x00,0x00,0x00,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x30,0x00,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x39,0x11,

    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x33,0x08,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x35,0x25,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x4F,0x3D,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x4E,0x35,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x41,0x35,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x73,0x30,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x74,0x10,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x76,0x40,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x77,0x00,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x28,0x31,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x7c,0x80,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x2e,0x04,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x4c,0x80,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x47,0x16,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x48,0x6A,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x50,0xc0,

    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x78,0x6E,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x2D,0x31,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x63,0x04,
    _W, DELAY(0),PACKET_DCS,SIZE(2), 0x4D,0x00,

    _W, DELAY(120), PACKET_DCS,SIZE(1), 0x11,
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
#define vfp_line 10

#define hsa_cyc  8
#define hbp_cyc  42
#define hda_cyc  720
#define hfp_cyc  44

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
    .target_freq = 800,
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
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

static int dsi_vdo_rm68200gai_init(void *_data)
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

void test_backlight_pwm_int()
{
    
    IOMC1 &= ~(BIT(23) | BIT(22) | BIT(21) | BIT(20));
    IOMC1 |= BIT(22) | BIT(21) | BIT(20); //??channel3???????TMR3_PWM_OUT

    gpio_direction_output(IO_PORTG_02, 1);
    gpio_set_pull_up(IO_PORTG_02, 1);
    gpio_set_pull_down(IO_PORTG_02, 1);
    gpio_set_die(IO_PORTG_02, 1);

    /*????PWM????104kHz*/
    T3_CNT = 0;
    T3_CON &= ~(0x3 << 4);
    T3_CON |= (0x0 << 4) | BIT(8); //0???
//	T3_PWM = 0x500;
    T3_PRD = 0x300;

    if(usb_is_charging())
    {
        ui_lcd_light_on();
    }
    else
    {
        ui_lcd_light_on_low();
    }
    
    T3_CON |= BIT(0);
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        imd_dmm_con &= ~BIT(5);
        delay_2ms(100);
        test_backlight_pwm_int();
        //gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        ui_lcd_light_off();
        //gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}



REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_DSI_VDO_4LANE_1280x480_EK79030),
    .logo 	 = LCD_NAME,
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_rm68200gai_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .esd = {
        //.interval = 500,
        //.esd_check_isr = rm68200gai_esd_check,
    }
};

#endif
