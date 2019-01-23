
//#include "icn2311.h"
#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "asm/isp_alg.h"

#define LCD_SDA_OUT()   gpio_direction_output(IO_PORTG_03,0);gpio_set_pull_up(IO_PORTG_03,1)
#define LCD_SDA_IN()    gpio_direction_input(IO_PORTG_03)
#define LCD_SDA_H()     gpio_direction_output(IO_PORTG_03,1)
#define LCD_SDA_L()     gpio_direction_output(IO_PORTG_03,0)
#define LCD_SDA_R()     gpio_read(IO_PORTG_03)

#define LCD_SCL_OUT()   gpio_direction_output(IO_PORTG_05,0)
#define LCD_SCL_H()     gpio_direction_output(IO_PORTG_05,1)
#define LCD_SCL_L()     gpio_direction_output(IO_PORTG_05,0)

static void *iic = NULL;

#define freq 300
#define lane_num 4
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24
#define vsa_line 3///2
#define vbp_line 32//6
#define vda_line 800
#define vfp_line 13//6
#define hsa_cyc  48
#define hbp_cyc  88//16
#define hda_cyc  480
#define hfp_cyc  40//10

#if (LCD_TYPE == LCD_ICN6211_800x480)

u8 chip_icn6211 = 0x58;

const static u8 init_cmd_list[] ={
   0
};

static struct mipi_dev mipi_dev_t = {
    .info = {
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .buf_addr 		= LCD_DEV_BUF,
        .buf_num 		= LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode 		= true,//false,
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
    .debug_mode = true,
};

static unsigned char ICN_2611_init(void* _data);
static unsigned char ICN_2611_write_reg(unsigned char icn_data);
//static unsigned char icn2611_init(u8 cmd_list);
static unsigned char _icn6211_write_one_byte(unsigned char w_chip_id, unsigned char register_address, unsigned char buf);
static void wr_reg(u8 w_chip_id,u8 reg, u8 dat);

static void lcd_io_init()
{
    LCD_SDA_OUT();
    LCD_SCL_OUT();

    LCD_SDA_H();
    LCD_SCL_H();
}

static void delay_50ns(u16 cnt)//380ns
{
    while (cnt--) {
        delay(100);//50ns
    }
}
static void wr_start(void)
{
    LCD_SDA_OUT();
	LCD_SCL_OUT();
    LCD_SDA_H();//拉高数据线
    delay_us(250);//delay_50ns(20);
    LCD_SCL_H();//拉高时钟线
    delay_us(250);//delay_50ns(20);
    LCD_SDA_L();//在时钟线为高电平时，拉低数据线，产生起始信号。
    delay_us(50);//delay_50ns(20);
    LCD_SCL_L();//拉低时钟线
}
static void wr_stop(void)
{
	LCD_SCL_L();//拉低时钟线
	delay_us(50);//delay_50ns(20);
    LCD_SDA_L();//拉低数据线
    delay_us(50);//delay_50ns(20);
    LCD_SCL_H();//拉高时钟线。
    delay_us(50);//delay_50ns(20);
    LCD_SDA_H();//时钟时线为高电平时，拉高数据线，产生停止信号。
    delay_us(50);//delay_50ns(20);
}

static void wr_dat(u8 dat)
{
    char i;
    for (i = 7; i >= 0; i--) {
        LCD_SCL_L();
        delay_us(50);//delay_us(1);//delay_50ns(1);

        if (dat& BIT(7)) {
            LCD_SDA_H();
        } else {
            LCD_SDA_L();
        }
		delay_us(50);//delay_us(1);//delay_50ns(1);
        LCD_SCL_H();
        delay_us(100);//delay_50ns(1);
        dat <<= 1;		
    }	
}
static void wr_Rack(void)
{
	LCD_SCL_L();
//	LCD_SDA_IN();
	delay_us(50);//delay_50ns(1);
	LCD_SDA_H();//释放数据总线，准备接收应答信号。
    delay_us(50);//delay_50ns(1);

	LCD_SCL_H();//拉高时钟线。
    delay_us(50);//delay_50ns(1);
	//while(LCD_SDA_R());//读取应答信号的状态。
    delay_us(50);//delay_50ns(10);
}


static void wr_reg(u8 w_chip_id,u8 reg, u8 dat)
{
	wr_start();
	wr_dat(w_chip_id);
	wr_Rack();
	wr_dat(reg);
	wr_Rack();
	wr_dat(dat);
	wr_Rack();
	wr_stop();
	puts("+++++++++iic++");
}

static unsigned char _icn6211_write_one_byte(unsigned char w_chip_id, unsigned char register_address, unsigned char buf)
{
    unsigned char ret = true;
    if (!iic) {
        ret = false;
        puts("\n icn6211 iic wr err -1");
        goto __gcend;
    }
    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts("\n icn6211 iic wr err 0\n");
        ret = false;
        goto __gcend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address)) {
        puts("\n icn6211 iic wr err 1\n");
        goto __gcend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, buf)) {
        puts("\n icn6211 iic wr err 3\n");
        ret = false;
        goto __gcend;
    }

__gcend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;
}

//u8 buf[10];
static unsigned char _icn6211_read_byte(unsigned char w_chip_id, unsigned char register_address)
{
    unsigned char ret = true;
	unsigned char regDat;
	int i=0;
    if (!iic) {
        ret = false;
        puts("\n icn6211 iic r err -1");
        goto __gcend;
    }
    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts("\n icn6211 iic r err 0\n");
        ret = false;
        goto __gcend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address)) {
        puts("\n icn6211 iic r err 1\n");
		ret = false;
        goto __gcend;
    }

    delay(10);

//	dev_ioctl(iic, IIC_IOCTL_START, 0);
	
	if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id+1)) {
        puts("\n icn6211 iic r err 2\n");
        ret = false;
        goto __gcend;
    }

	delay(10);
#if 0
	for(i=0;i<9;i++){
	    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_ACK, buf[i])) {
	        puts("\n icn6211 iic wr err 3\n");
	        goto __gcend;
	    }
	}
    delay(10);
#endif
#if 0
    if (dev_ioctl(iic, IIC_IOCTL_RX, ret)) {
        puts("\n icn6211 iic r err 4\n");
        ret = false;
        goto __gcend;
    }
#endif
	if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)&regDat)) {
        regDat = 0;
        puts("\n gsen iic rd err 3\n");
        goto __gcend;
    }

__gcend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
	
//	return buf;
    return regDat;
}


static unsigned char wrICN6211_Reg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;
    u8 icn6211_ID = 0x58;
	u8	*recieve = NULL;
	//while(1){
	//	wr_reg(icn6211_ID,regID,regDat);
		_icn6211_write_one_byte(icn6211_ID,regID,regDat);
	//	printf("\n regID = %d,regDat =%d\n",regID,regDat);
		delay_2ms(10);
	//	}
//	recieve = _icn6211_read_one_byte(icn6211_ID,regID);
//	printf("\n recieve = %d,regDat =%d\n",recieve,regDat);
    return ret;
}
static unsigned char rICN6211_Reg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;
    u8 icn6211_ID = 0x58;
	u8	*recieve = NULL;
	regDat = _icn6211_read_byte(icn6211_ID,regID);
	//for(int i=0;i<10;i++){
		printf("\n recieve = %x,regID =%x\n",regDat,regID);
	//}
    return ret;
}


static struct sw_iic_platform_data *_IIC = NULL;

static unsigned char icn2611_reg_config(void)
{
#if 1
	if (!iic) {
		iic = dev_open("iic2",0);//("iic2", 0);
        if (!iic) {
            puts("\n  open iic dev for icn6211 err\n");
            return -EINVAL;
        }
    }	
#endif
#if 1
	gpio_set_pull_up(IO_PORTG_05,1);
	gpio_set_pull_up(IO_PORTG_03,1);
	delay_2ms(100);
	wrICN6211_Reg(0x20, 0x20);
	wrICN6211_Reg(0x21, 0xE0);
	wrICN6211_Reg(0x22, 0x13);
	wrICN6211_Reg(0x23, 0x28);
	wrICN6211_Reg(0x24, 0x30);
	wrICN6211_Reg(0x25, 0x58);
	wrICN6211_Reg(0x26, 0x00);
	wrICN6211_Reg(0x27, 0x0D);
	wrICN6211_Reg(0x28, 0x03);
	wrICN6211_Reg(0x29, 0x20);
	wrICN6211_Reg(0x34, 0x80);
	wrICN6211_Reg(0x36, 0x28);
//	wrICN6211_Reg(0x86, 0xA0);
	wrICN6211_Reg(0xB5, 0xA0);//0xFF);
	wrICN6211_Reg(0x5C, 0xFF);//0x01);
//	wrICN6211_Reg(0x87, 0x92);
	wrICN6211_Reg(0x2A, 0x01);//0x71);
	wrICN6211_Reg(0x56, 0x93);//0x90);//0x19);
	wrICN6211_Reg(0x6B, 0x71);//0x40);
	wrICN6211_Reg(0x69, 0x25);//0x88);
	wrICN6211_Reg(0x10, 0x40);//0x20);
	wrICN6211_Reg(0x11, 0x88);//0x20);
	wrICN6211_Reg(0xB6, 0x20);//0x10);
	wrICN6211_Reg(0x51, 0x20);
	wrICN6211_Reg(0x14, 0x43);
	wrICN6211_Reg(0x2A, 0x49);
	wrICN6211_Reg(0x09, 0x10);


	delay_2ms(10);
	rICN6211_Reg(0x20, 0xFF);
	rICN6211_Reg(0x21, 0xFF);
	rICN6211_Reg(0x22, 0xFF);
	rICN6211_Reg(0x23, 0xFF);
	rICN6211_Reg(0x24, 0xFF);
	rICN6211_Reg(0x25, 0xFF);
	rICN6211_Reg(0x26, 0xFF);
	rICN6211_Reg(0x27, 0xFF);
	rICN6211_Reg(0x28, 0xFF);
	rICN6211_Reg(0x29, 0xFF);
	rICN6211_Reg(0x34, 0xFF);
	rICN6211_Reg(0x36, 0xFF);
	rICN6211_Reg(0xB5, 0xFF);//0xFF);
	rICN6211_Reg(0x5C, 0xFF);//0x01);
	rICN6211_Reg(0x2A, 0xFF);//0x71);
	rICN6211_Reg(0x56, 0xFF);//0x90);//0x19);
	rICN6211_Reg(0x6B, 0xFF);//0x40);
	rICN6211_Reg(0x69, 0xFF);//0x88);
	rICN6211_Reg(0x10, 0xFF);//0x20);
	rICN6211_Reg(0x11, 0xFF);//0x20);
	rICN6211_Reg(0xB6, 0xFF);//0x10);
	rICN6211_Reg(0x51, 0xFF);
	rICN6211_Reg(0x14, 0xFF);
	rICN6211_Reg(0x2A, 0xFF);
	rICN6211_Reg(0x09, 0xFF);


	#endif
	#if 0
	rICN6211_Reg(0x21, 0x40);
	rICN6211_Reg(0x22, 0x10);
	rICN6211_Reg(0x23, 0x0A);
	rICN6211_Reg(0x24, 0x0A);
	rICN6211_Reg(0x25, 0x14);
	rICN6211_Reg(0x26, 0x00);
	rICN6211_Reg(0x27, 0x04);
	rICN6211_Reg(0x28, 0x02);
	rICN6211_Reg(0x29, 0x02);
	rICN6211_Reg(0x34, 0x80);
	rICN6211_Reg(0x36, 0x0A);
	rICN6211_Reg(0x86, 0x2A);
	rICN6211_Reg(0xB5, 0xA0);
	rICN6211_Reg(0x5C, 0xFF);
	rICN6211_Reg(0x87, 0x10);
	rICN6211_Reg(0x2A, 0x01);
	rICN6211_Reg(0x56, 0x92);
	rICN6211_Reg(0x6B, 0x61);
	rICN6211_Reg(0x69, 0x10);
	rICN6211_Reg(0x10, 0x10);
	rICN6211_Reg(0x11, 0x98);
	rICN6211_Reg(0xB6, 0x20);
	rICN6211_Reg(0x51, 0x20);
	rICN6211_Reg(0x09, 0x10);
	#endif
	return 0;
	
}

static unsigned char ICN_2611_init(void* _data)
{
	struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
	u8 lcd_reset = data->lcd_io.lcd_reset;
	puts("icn2611 init \n");
//	lcm_power(true);
//	MDELAY(2);
	
	gpio_direction_output(lcd_reset, 1);//SET_RESET_PIN(1);
	delay_us(5);//MDELAY(20);
//	gpio_direction_output(lcd_reset, 0);//SET_RESET_PIN(0);
//	delay_us(25);//MDELAY(50);
//	gpio_direction_output(lcd_reset, 1);//SET_RESET_PIN(1);
	delay_us(50);//MDELAY(100);
	icn2611_reg_config();
	dsi_dev_init(&mipi_dev_t);//init_icn6211_registers();
	delay_us(1);//MDELAY(2);
	return 0;

}


#if 1
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

#if 1
REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_ICN6211_800x480),
    .logo 	 = LCD_NAME,
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = ICN_2611_init,
    .bl_ctrl = mipi_backlight_ctrl,
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

