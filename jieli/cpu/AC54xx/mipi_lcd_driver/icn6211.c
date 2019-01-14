//#include "icn2611.h"
#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "asm/isp_alg.h"


/******************IO info*********************/
#define LCD_SDA_OUT()   PORTG_DIR &=~BIT(3);PORTB_PU |= BIT(3)
#define LCD_SDA_IN()    PORTG_DIR |= BIT(3)
#define LCD_SDA_H()     PORTG_OUT |= BIT(3)
#define LCD_SDA_L()     PORTG_OUT &=~BIT(3)
#define LCD_SDA_R()     PORTG_IN  & BIT(3)

#define LCD_SCL_OUT()   PORTG_DIR &=~BIT(5)
#define LCD_SCL_H()     PORTG_OUT |= BIT(5)
#define LCD_SCL_L()     PORTG_OUT &=~BIT(5)

/**********************************************/

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
#define vbp_line 6
#define vda_line 800
#define vfp_line 6
#define hsa_cyc  4
#define hbp_cyc  16
#define hda_cyc  480
#define hfp_cyc  10

#if (LCD_TYPE == LCD_ICN6211_800x480)

#define data_type unsigned char
u8 chip_icn6211 = 0x58;

const static u8 init_cmd_list[] ={
    0x20, 0xF0,
    0x21, 0x40,
    0x22, 0x10,
    0x23, 0x0A,
    0x24, 0x0A,
    0x25, 0x14,
    0x26, 0x00,
    0x27, 0x04,
    0x28, 0x02,
    0x29, 0x02,
    0x34, 0x80,
    0x36, 0x0A,
    0x86, 0x2A,
    0xB5, 0xA0,
    0x5C, 0xFF,
    0x87, 0x10,
    0x2A, 0x01,
    0x56, 0x92,
    0x6B, 0x61,
    0x69, 0x10,
    0x10, 0x10,
    0x11, 0x98,
    0xB6, 0x20,
    0x51, 0x20,
    0x09, 0x10,
};


#if 1
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

//    .cmd_list = init_cmd_list,
//    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = true,
};
#else
static struct mipi_dev icn6211_dev = {
	0,
}
#endif
u8 icn6211_reg_config_dat[25][2] = {
	{0x20, 0xF0},
	{0x21, 0x40},
	{0x22, 0x10},
	{0x23, 0x0A},
	{0x24, 0x0A},
	{0x25, 0x14},
	{0x26, 0x00},
	{0x27, 0x04},
	{0x28, 0x02},
	{0x29, 0x02},
	{0x34, 0x80},
	{0x36, 0x0A},
	{0x86, 0x2A},
	{0xB5, 0xA0},
	{0x5C, 0xFF},
	{0x87, 0x10},
	{0x2A, 0x01},
	{0x56, 0x92},
	{0x6B, 0x61},
	{0x69, 0x10},
	{0x10, 0x10},
	{0x11, 0x98},
	{0xB6, 0x20},
	{0x51, 0x20},
	{0x09, 0x10},
};
static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};

static int ICN_2611_init(void* _data);
static data_type ICN_2611_read_data(data_type icn_addr);
static void ICN_2611_write_cmd(data_type icn_cmd);
static void ICN_2611_write_reg(data_type icn_data);
void test_backlight_pwm_int(void);
static void mipi_backlight_ctrl(void *_data, u8 on);
static void icn2611_init(u8 cmd_list);

static void *iic = NULL;

unsigned char wrICN6211_Reg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;
    u8 icn6211_ID = 0x58;

//    high = regID >> 8;
//    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
//    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x64)) {
//        ret = 0;
//        goto __wend;
//    }
	if (dev_ioctl(iic, IIC_IOCTL_TX, icn6211_ID)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}
/*
void ICN6211_reset(u8 isp_dev)
{
    puts("ICN6211 reset \n");

    u32 reset_gpio;
    u32 pwdn_gpio;

    if (isp_dev == ISP_DEV_0) {
        reset_gpio = reset_gpios[0];
        pwdn_gpio = pwdn_gpios[0];
    } else {
        reset_gpio = reset_gpios[1];
        pwdn_gpio = pwdn_gpios[1];
    }
    //printf("gpio=%d\n", reset_gpio);
    gpio_direction_output(pwdn_gpio, 0);
	gpio_direction_output(reset_gpio, 1);//SET_RESET_PIN(1);
	delay_2ms(5);//MDELAY(20);
	gpio_direction_output(reset_gpio, 0);//SET_RESET_PIN(0);
	delay_2ms(25);//MDELAY(50);
	gpio_direction_output(reset_gpio, 1);//SET_RESET_PIN(1);
	delay_2ms(50);//MDELAY(100);
}
*/

static void icn2611_reg_config(void)
{
	dev_open("iic2",NULL);
	
	wrICN6211_Reg(0x20, 0xF0);
	wrICN6211_Reg(0x21, 0x40);
	wrICN6211_Reg(0x22, 0x10);
	wrICN6211_Reg(0x23, 0x0A);
	wrICN6211_Reg(0x24, 0x0A);
	wrICN6211_Reg(0x25, 0x14);
	wrICN6211_Reg(0x26, 0x00);
	wrICN6211_Reg(0x27, 0x04);
	wrICN6211_Reg(0x28, 0x02);
	wrICN6211_Reg(0x29, 0x02);
	wrICN6211_Reg(0x34, 0x80);
	wrICN6211_Reg(0x36, 0x0A);
	wrICN6211_Reg(0x86, 0x2A);
	wrICN6211_Reg(0xB5, 0xA0);
	wrICN6211_Reg(0x5C, 0xFF);
	wrICN6211_Reg(0x87, 0x10);
	wrICN6211_Reg(0x2A, 0x01);
	wrICN6211_Reg(0x56, 0x92);
	wrICN6211_Reg(0x6B, 0x61);
	wrICN6211_Reg(0x69, 0x10);
	wrICN6211_Reg(0x10, 0x10);
	wrICN6211_Reg(0x11, 0x98);
	wrICN6211_Reg(0xB6, 0x20);
	wrICN6211_Reg(0x51, 0x20);
	wrICN6211_Reg(0x09, 0x10);
	
}

static int ICN_2611_init(void* _data)
{
	struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
	u8 lcd_reset = data->lcd_io.lcd_reset;
	puts("icn2611 init \n");
//	lcm_power(true);
//	MDELAY(2);
	
	gpio_direction_output(lcd_reset, 1);//SET_RESET_PIN(1);
	delay_2ms(5);//MDELAY(20);
	gpio_direction_output(lcd_reset, 0);//SET_RESET_PIN(0);
	delay_2ms(25);//MDELAY(50);
	gpio_direction_output(lcd_reset, 1);//SET_RESET_PIN(1);
	delay_2ms(50);//MDELAY(100);
	icn2611_reg_config();
	dsi_dev_init(&mipi_dev_t);//init_icn6211_registers();
	delay_2ms(1);//MDELAY(2);
	return 0;
#if 0
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
#endif
}


#if 0
void test_backlight_pwm_int(void)
{
    
    IOMC1 &= ~(BIT(23) | BIT(22) | BIT(21) | BIT(20));
    IOMC1 |= BIT(22) | BIT(21) | BIT(20); //TMR3_PWM_OUT

    gpio_direction_output(IO_PORTG_02, 1);
    gpio_set_pull_up(IO_PORTG_02, 1);
    gpio_set_pull_down(IO_PORTG_02, 1);
    gpio_set_die(IO_PORTG_02, 1);

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
    if (on) 
    {
        imd_dmm_con &= ~BIT(5);
        delay_2ms(100);
        test_backlight_pwm_int();
    } else {
        ui_lcd_light_off();
    }
}
#endif

#if 0
REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_ICN6211_800x480),
    .logo 	 = LCD_NAME,
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = ICN_2611_init,
//    .bl_ctrl = mipi_backlight_ctrl,
    .esd = {
        //.interval = 500,
        //.esd_check_isr = rm68200gai_esd_check,
    }
};
#else
REGISTER_LCD_DEVICE_DRIVE(dev) = {//LCM_DRIVER r61526_qvga_lcm_drv = 

    .enable  = IF_ENABLE(LCD_ICN6211_800x480),
	.logo 	 = "icn6211",
    .type 	 = LCD_MIPI,
    .dev	 = &mipi_dev_t,
	.init    = ICN_2611_init,
};
#endif
#endif


