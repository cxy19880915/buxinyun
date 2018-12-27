#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
//porch

#if (LCD_TYPE == LCD_DSI_VDO_2LANE_854x480)
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list
const static u8 init_cmd_list[] = {

 
//put sp  board_init  rw  bta
   _W, DELAY(1), PACKET_DCS,SIZE(6),  0xFF, 0xFF, 0x98, 0x06, 0x04,0x01,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x08, 0x10,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x21, 0x01,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x30, 0x02,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x31, 0x02,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x60, 0x07,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x61, 0x00,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x62, 0x07,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x63, 0x00,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x40, 0x11,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x41, 0x44,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x42, 0x01,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x43, 0x09,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x44, 0x0c,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x45, 0x1b,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x50, 0x70,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x51, 0x70,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x52, 0x00,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x53, 0x59,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x57, 0x50,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa0, 0x00,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa1, 0x0a,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa2, 0x11,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa3, 0x0c,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa4, 0x05,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa5, 0x09,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa6, 0x07,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa7, 0x06,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa8, 0x06,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xa9, 0x0b,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xaa, 0x10,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xab, 0x0a,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xac, 0x0f,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xad, 0x18,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xae, 0x10,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xaf, 0x00,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc0, 0x00,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc1, 0x0a,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc2, 0x11,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc3, 0x0c,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc4, 0x05,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc5, 0x09,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc6, 0x07,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc7, 0x06,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc8, 0x06,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xc9, 0x0b,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xca, 0x10,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xcb, 0x0a,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xcc, 0x0f,
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xCD, 0x18,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xCE, 0x10,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xCF, 0x00,
											
   _W, DELAY(1), PACKET_DCS,SIZE(6),  0xFF, 0xFF,0x98,0x06,0x04,0x06,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x00, 0x20,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x01, 0x05,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x02, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x03, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x04, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x05, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x06, 0x88,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x07, 0x04,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x08, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x09, 0x90,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x0A, 0x04,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x0B, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x0C, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x0D, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x0E, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x0F, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x10, 0x55,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x11, 0x50,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x12, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x13, 0x85,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x14, 0x85,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x15, 0xC0,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x16, 0x0B,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x17, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x18, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x19, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x1A, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x1B, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x1C, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x1D, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x20, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x21, 0x23,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x22, 0x45,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x23, 0x67,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x24, 0x01,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x25, 0x23,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x26, 0x45,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x27, 0x67,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x30, 0x02,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x31, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x32, 0x11,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x33, 0xAA,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x34, 0xBB,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x35, 0x66,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x36, 0x00,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x37, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x38, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x39, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x3A, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x3B, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x3C, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x3D, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x3E, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x3F, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x40, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x52, 0x10, 
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x53, 0x10, 
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x54, 0x13,
   
   _W, DELAY(1), PACKET_DCS,SIZE(6),  0xFF, 0xFF,0x98,0x06,0x04,0x07,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x17, 0x22,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0x02, 0x77,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xE1, 0x79,  
   _W, DELAY(1), PACKET_DCS,SIZE(2),  0xB3, 0x10, 
											
   _W, DELAY(1), PACKET_DCS,SIZE(6),  0xFF, 0xFF,0x98,0x06,0x04,0x00,											
   _W, DELAY(1),PACKET_DCS_WITHOUT_BTA, SIZE(2), 0x11, 0x00,
   _W, DELAY(120), PACKET_DCS_WITHOUT_BTA,SIZE(2),	0x29,0x00,


};

#define freq 321

#define lane_num 2
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24
#if 0
#define vsa_line 2
#define vbp_line 17
#define vda_line 1280
#define vfp_line 10

#define hsa_cyc  4
#define hbp_cyc  12
#define hda_cyc  480
#define hfp_cyc  16

#define vsa_line 1
#define vbp_line 10
#define vda_line 854
#define vfp_line 16

#define hsa_cyc  8
#define hbp_cyc  16
#define hda_cyc  480
#define hfp_cyc  16
#endif
#if 1
#define vsa_line 4//10
#define vbp_line 12//20
#define vda_line 800
#define vfp_line 20//23

#define hsa_cyc  8//6
#define hbp_cyc  32//8
#define hda_cyc  480
#define hfp_cyc   32//16
#else
#define vsa_line 25
#define vbp_line 40
#define vda_line 864
#define vfp_line 32

#define hsa_cyc  8
#define hbp_cyc  16
#define hda_cyc  480
#define hfp_cyc  16
#endif
static struct mipi_dev mipi_dev_t = {
    .info = {
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .buf_addr 		= LCD_DEV_BUF,
        .buf_num 		= LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode 		= 0,
        .color 			= 0x00ff00,
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
        .x3_lane = lane_dis,//lane_en |lane_ex| lane_d2,
        .x4_lane = lane_dis,//lane_en |lane_ex| lane_d3,
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_event = SYNC_EVENT_MODE,
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
    .target_freq = 642,//500
    .pll_division = MIPI_PLL_DIV2,//MIPI_PLL_DIV2,

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
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(10);
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(10);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(10);
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
    T3_PRD = 0x120;

    if(usb_is_charging())
    {
        ui_lcd_light_on();
    }
    else
    {
        //_lcd_light_on_low();
    }
    
    T3_CON |= BIT(0);
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    #if 1
    if (on) 
    {
        imd_dmm_con &= ~BIT(5);
        delay_2ms(100);
        //test_backlight_pwm_int();
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        //ui_lcd_light_off();
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
    #endif
}
//SDRDBG
static void rm68200gai_esd_check(struct esd_deal *esd)
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
//log_flush
REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_DSI_VDO_2LANE_854x480),
    .logo 	 = LCD_NAME,
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_rm68200gai_init,
    .bl_ctrl = mipi_backlight_ctrl,
    ///d = {
    //  .interval = 500,
    //  .esd_check_isr = rm68200gai_esd_check,
    //
};

#endif
