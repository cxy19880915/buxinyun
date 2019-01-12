#include "generic/typedef.h"
#include "asm/cpu.h"
#include "asm/dsi.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_1920x1080_55)
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list
const static u8 init_cmd_list[] = {

    #if 0  //05需要打开
    _W, DELAY(1),   PACKET_GENERIC,SIZE(2), 0xff, 0x05,
    _W, DELAY(1),   PACKET_GENERIC,SIZE(2), 0xff, 0x00,
    _W, DELAY(1),   PACKET_GENERIC,SIZE(2), 0xfb, 0x01,

    _W, DELAY(20),  PACKET_GENERIC,SIZE(2), 0xff, 0x00,
    _W, DELAY(1),   PACKET_GENERIC,SIZE(2), 0xd3, 0x08,
    _W, DELAY(1),   PACKET_GENERIC,SIZE(2), 0xd4, 0x0E,
    #endif
    _W, DELAY(120), PACKET_GENERIC,SIZE(2),0X11,0x00, 
    _W, DELAY(160), PACKET_GENERIC,SIZE(2),0X29,0x00,  //Display On
};
#if 0
#define freq 800

#define lane_num 4
#define bpp_num  24

#define vsa_line 2
#define vbp_line 4
#define vda_line 1920
#define vfp_line 4

#define hsa_cyc  10
#define hbp_cyc  50
#define hda_cyc  1080
#define hfp_cyc  142
#else
#define freq 800

#define lane_num 4
#define bpp_num  24

#define vsa_line 11
#define vbp_line 6
#define vda_line 1920
#define vfp_line 14

#define hsa_cyc  8
#define hbp_cyc  16
#define hda_cyc  1080
#define hfp_cyc  72
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
        .itp_mode_en 	= true,            // 720p鎻掑�煎埌1080p
        .format 		= FORMAT_RGB888,
        .len 			= LEN_256,

        .rotate_en 		= true,			// 鏃嬭浆浣胯兘
        .hori_mirror_en = true,			// 姘村钩闀滃儚浣胯兘
        .vert_mirror_en = false,		// 鍨傜洿闀滃儚浣胯兘

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
        #if 1
        .x0_lane = lane_en|lane_ex|lane_d0,
        .x1_lane = lane_en|lane_ex|lane_d1,
        .x2_lane = lane_en|lane_ex|lane_clk,
        .x3_lane = lane_en|lane_ex|lane_d2,
        .x4_lane = lane_en|lane_ex|lane_d3,
        #endif
        #if 0  //02
        .x0_lane = lane_en | lane_d0,
        .x1_lane = lane_en | lane_d1,
        .x2_lane = lane_en | lane_ex|lane_clk,
        .x3_lane = lane_en | lane_d2,
        .x4_lane = lane_en | lane_d3,
        #endif
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_event = SYNC_PULSE_MODE,
        .burst_mode = NON_BURST_MODE,
        .vfst_lp_en = true,
        .vlst_lp_en = true,
        .vblk_lp_en = false,
        .color_mode = COLOR_FORMAT_RGB888,
        .virtual_ch   = 0,
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
        .dsi_vdo_bllp_v = hsa_cyc + hbp_cyc + hda_cyc + hfp_cyc,
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
    .target_freq = 800,
    .pll_division = MIPI_PLL_DIV1,

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
    /**************************/
    /*  PG3 1.8v 低有电
    /*  PG5 +-5v 高有电
    /*  PG4 复位
    /**************************/
    if (-1 != lcd_reset) {

        gpio_direction_output(lcd_reset, 1);
        delay_2ms(80);
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(20);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(20);
        
        gpio_direction_output(IO_PORTG_03, 1);
        delay_2ms(10);
        gpio_direction_output(IO_PORTG_05, 1);
        delay_2ms(10);
            
    }

    dsi_dev_init(&mipi_dev_t);

    return 0;
}

void test_backlight_pwm_int()
{
    
    IOMC1 &= ~(BIT(23) | BIT(22) | BIT(21) | BIT(20));
    IOMC1 |= BIT(22) | BIT(21) | BIT(20); //将channel3输出选择为TMR3_PWM_OUT

    gpio_direction_output(IO_PORTG_02, 1);
    gpio_set_pull_up(IO_PORTG_02, 1);
    gpio_set_pull_down(IO_PORTG_02, 1);
    gpio_set_die(IO_PORTG_02, 1);

    /*计算PWM频率为104kHz*/
    T3_CNT = 0;
    T3_CON &= ~(0x3 << 4);
    T3_CON |= (0x0 << 4) | BIT(8); //0分频
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
    .enable = IF_ENABLE(LCD_DSI_VDO_4LANE_1920x1080_55),
    .logo =LCD_NAME,// "mipi_4lane_1080_55",
    .type = LCD_MIPI,
    .init = dsi_init,
    .dev  = &mipi_dev_t,
    .bl_ctrl = mipi_backlight_ctrl,
};
#endif
