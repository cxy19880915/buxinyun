#include "system/includes.h"
#include "eth/eth_phy.h"
#include "eth/ethmac.h"

#include "app_config.h"
#include "gSensor_manage.h"
#include "device/av10_spi.h"
#include "power_ctrl.h"

// *INDENT-OFF*

#ifdef CONFIG_BOARD_MIPI_V3

/* #define UART0 */

UART0_PLATFORM_DATA_BEGIN(uart0_data)               //使用串口URATx x: 0 1 2 3,请看uart.h头文件 
    .baudrate = 115200,				                //配置窗口波特率
    .tx_pin = IO_PORTG_14,			                //配置串口TX脚，如果该脚位普通io,则需要使用output channel的形式 请见 void debug_uart_init()
    .flags = UART_DEBUG,			                //使能串口调试功能
UART0_PLATFORM_DATA_END();


#ifdef CONFIG_SD0_ENABLE

SD0_PLATFORM_DATA_BEGIN(sd0_data)
	.port 					= 'C',                  
	.priority 				= 3,
	.data_width 			= 4,
	.speed 					= 30000000,
	.detect_mode 			= SD_CMD_DECT,
	.detect_func 			= sdmmc_0_clk_detect,
SD0_PLATFORM_DATA_END()

#endif //CONFIG_SD0_ENABLE

#ifdef CONFIG_SD1_ENABLE

int sdmmc_1_io_detect(const struct sdmmc_platform_data *data)
{
    static u8 init = 0;

    if (!init) {
        init = 1;
        gpio_direction_input(IO_PORTA_11);
        gpio_set_pull_up(IO_PORTA_11, 1);
        gpio_set_pull_down(IO_PORTA_11, 0);
    }

    return !gpio_read(IO_PORTA_11);
}

SD1_PLATFORM_DATA_BEGIN(sd1_data)
	.port 					= 'C',                  //使用对应组IO口此处使用 PORTC
	.priority 				= 3,
	.data_width 			= 4,                    //配置数据线sy
	.speed 					= 30000000,             //配置速度
	.detect_mode 			= SD_IO_DECT,           //配置检测方式 此处使用io检测
	.detect_func 			= sdmmc_1_io_detect,    //io检测需要配置对应io的读操作 函数
SD1_PLATFORM_DATA_END()

#endif //CONFIG_SD1_ENABLE

#ifdef CONFIG_SD2_ENABLE

SD2_PLATFORM_DATA_BEGIN(sd2_data)
	.port 					= 'B',
	.priority 				= 3,
	.data_width 			= 4,
	.speed 					= 30000000,
	.detect_mode 			= SD_CMD_DECT,
	.detect_func 			= sdmmc_2_clk_detect,
SD2_PLATFORM_DATA_END()

#endif //CONFIG_SD2_ENABLE

HW_IIC0_PLATFORM_DATA_BEGIN(hw_iic0_data)
	.clk_pin = IO_PORTB_04,//IO_PORTD_14,           //配置硬件iic的clk脚
	.dat_pin = IO_PORTB_03,//IO_PORTD_15,           //配置硬件iic的dat脚
	.baudrate = 0x1f,                               //配置iic波特率
HW_IIC0_PLATFORM_DATA_END()

HW_IIC1_PLATFORM_DATA_BEGIN(hw_iic1_data)
	.clk_pin = IO_PORTB_00,//IO_PORTD_14,
	.dat_pin = IO_PORTB_01,//IO_PORTD_15,
	.baudrate = 0xff,
HW_IIC1_PLATFORM_DATA_END()

SW_IIC_PLATFORM_DATA_BEGIN(sw_iic_data)
	.gpioclk = (struct gpio_reg *)(&PORTB_OUT),     //配置软件iic的clk脚是portx
	.gpiodat = (struct gpio_reg *)(&PORTB_OUT),     //配置软件iic的dat脚是portx
	.clk_pin = 0,                                   //配置clk的是对应port的第几脚
	.dat_pin = 1,                                   //配置dat的是对应port的第几脚
SW_IIC_PLATFORM_DATA_END()


LCD_PLATFORM_DATA_BEGIN(lcd_data)
	.lcd_name = (u8*)"mipi_4lane_720p_rm68200",     //给屏设备命名
	.interface = LCD_MIPI,                          //配置改屏的接口类型 是LCD_MIPI, LCD_DVP_MCU, LCD_DVP_RGB
	.lcd_io = {
		.lcd_reset = IO_PORTG_04,                   //配置屏幕的reset脚
        .backlight = IO_PORTG_02,                   //配置屏幕的背光控制io
        .backlight_value = 1,                       //背光默认值
	}
LCD_PLATFORM_DATA_END()





#ifdef CONFIG_VIDEO0_ENABLE

static const struct camera_platform_data camera0_data = {
    .xclk_gpio      = IO_PORTA_14,                                 	//配置摄像头的xclk脚
	.reset_gpio     = IO_PORTA_15,                                  //配置摄像头的reset脚
    .online_detect  = NULL,
	.pwdn_gpio      = IO_PORTE_01,                                  //pwdn脚配置
    .power_value    = 1,                                            	//pwdn脚默认值配置
	.interface      = SEN_INTERFACE_CSI2,                           //该摄像头接口类型 SEN_INTERFACE_CSI2（·SEN_INTERFACE0, SEN_INTERFACE1, SEN_INTERFACE_CSI2） 
    .csi2 = {
        .data_lane_num = 2,                                         //配置mipi的通道数 总通道数位4,根据硬件来选，此处使用2通道 
        .clk_inv = 1,
        .d0_rmap = CSI2_X4_LANE,                                    //data0 选择CSI_D4P、CSI_D4N 
        .d0_inv = 1,                                                //data0差分脚N->P  或者P->N 该值为1   N->N  P->P  
        .d1_rmap = CSI2_X3_LANE,                                    //data1 选择CSI_D3P、CSI_D3N 
        .d1_inv = 1,                                                //data0差分脚N->P  或者P->N 该值为1   N->N  P->P 
        .tval_hstt = 12,                                            //与时序相关 需根据mipi摄像头手册来配置
        .tval_stto = 30,
    }
};

static const struct video_subdevice_data video0_subdev_data[] = {
    { VIDEO_TAG_CAMERA, (void *)&camera0_data },
};
static const struct video_platform_data video0_data = {
    .data = video0_subdev_data,
    .num = ARRAY_SIZE(video0_subdev_data),
};

#endif



#ifdef CONFIG_VIDEO1_ENABLE

static bool camera1_online_detect()
{//后拉摄像头 在线检测 IO配置
    static u8 init = 0;

    if (!init) {
        init = 1;
        gpio_direction_input(IO_PORTA_10);
        gpio_set_pull_up(IO_PORTA_10, 0);
        gpio_set_pull_down(IO_PORTA_10, 0);
    }

    return !gpio_read(IO_PORTA_10);
}

static const struct camera_platform_data camera1_data = {
    .xclk_gpio      = -1,                                           //摄像头xclk脚配置,此处av10使用外部晶振提供，因此给-1
	.reset_gpio     = IO_PORTA_06,                              //摄像头reset脚配置
	.pwdn_gpio      = IO_PORTB_12,                              //此处的pwdn为后拉头的电源控制，并非av10的pwdn
    .power_value    = 1,                                            //默认值为1
	.interface      = SEN_INTERFACE1,                           //该摄像头接口类型 SEN_INTERFACE0（SEN_INTERFACE0, SEN_INTERFACE1, SEN_INTERFACE_CSI2）,
    .online_detect  = camera1_online_detect,                        //后拉头在线检测
    .dvp = {
        .pclk_gpio  = IO_PORTH_00,                                  //配置dvp摄像头对应的pclk脚
        .hsync_gpio = -1,                                           //配置dvp摄像头对应的行型号脚
        .vsync_gpio = -1,                                           //配置dvp摄像头对应的场型号脚
		.io_function_sel = DVP_SENSOR1(0),                  //配置dvp摄像头占用的io脚，选择 
        .data_gpio  = {                                             //配置dvp摄像头对应的数据脚 D0~D9
            -1,
            -1,
            IO_PORTH_08,
            IO_PORTH_07,
            IO_PORTH_06,
            IO_PORTH_05,
            IO_PORTH_04,
            IO_PORTH_03,
            IO_PORTH_02,
            IO_PORTH_01,
        },
    }
};

static const struct video_subdevice_data video1_subdev_data[] = {
    { VIDEO_TAG_CAMERA, (void *)&camera1_data },
};
static const struct video_platform_data video1_data = {
    .data = video1_subdev_data,
    .num = ARRAY_SIZE(video1_subdev_data),
};
#endif



USB_PLATFORM_DATA_BEGIN(usb0_data)                                  //usb设备 相关配置
    .id = 0,
    .online_check_cnt = 5,                                          //上线消抖计数
    .offline_check_cnt = 100,//250                                  //离线消抖计数
    .isr_priority = 4,                                              //中断优先级
    .host_ot = 20,                                          
    .host_speed = 1,
    .slave_ot = 10,
    .ctl_irq_int = HUSB0_CTL_INT,
USB_PLATFORM_DATA_END()



#ifdef CONFIG_ADKEY_ENABLE
/*-------------ADKEY GROUP 1----------------*/
#define ADC0_33   (0x3FF)
#define ADC0_30   (0x3ff*30/33) //0x3A2
#define ADC0_27   (0x3ff*27/33) //0x345
#define ADC0_23   (0x3ff*23/33) //0x2C9
#define ADC0_20   (0x3ff*20/33) //0x26C
#define ADC0_17   (0x3ff*17/33) //0x1F0
#define ADC0_13   (0x3ff*13/33) //0x193
#define ADC0_07   (0x3ff*07/33) //0x136
#define ADC0_03   (0x3ff*03/33) //0xD9
#define ADC0_02   (0x3ff*02/33) //0xD9
#define ADC0_01   (0x3ff*01/33) //0x7C
#define ADC0_00   (0)

#define ADKEY_V_0      	((ADC0_33 + ADC0_30)/2)
#define ADKEY_V_1 		((ADC0_30 + ADC0_27)/2)
#define ADKEY_V_2 		((ADC0_27 + ADC0_23)/2)
#define ADKEY_V_3 		((ADC0_23 + ADC0_20)/2)
#define ADKEY_V_4 		((ADC0_20 + ADC0_17)/2)
#define ADKEY_V_5 		((ADC0_17 + ADC0_13)/2)
#define ADKEY_V_6 		((ADC0_07 + ADC0_03)/2)//up 0.7
#define ADKEY_V_7 		((ADC0_03 + ADC0_03)/2)//down 0.4
#define ADKEY_V_8 		((ADC0_01 + ADC0_01)/2)//ok 0.2
#define ADKEY_V_9 		((ADC0_01 + ADC0_00)/2)//menu/mode 0.1
#define ADKEY_V_10 		(ADC0_00)

//ad按键使用  每个ad_value（ad电压值）对应一个key_value（消息）例如 ADKEY_V_8按键值对应KEY_OK  
//五个按键：OK ,  MEN/MODE, POWER,  UP,  DOWN
ADKEY_PLATFORM_DATA_BEGIN(adkey_data)
	.io 		= IO_PORTB_14,
	.ad_channel = 4,
	.table 	= {
		.ad_value = {
			ADKEY_V_0,
			ADKEY_V_1,
			ADKEY_V_2,
			ADKEY_V_3,
			ADKEY_V_4,
			ADKEY_V_5,
			ADKEY_V_6,
			ADKEY_V_7,
			ADKEY_V_8,
			ADKEY_V_9,
			ADKEY_V_10,
		},
		.key_value = {
			NO_KEY,    /*0*/
			NO_KEY,
			NO_KEY,
			NO_KEY,
			NO_KEY,
			NO_KEY,  /*5*/
			KEY_UP,
			KEY_DOWN,
			KEY_OK,
			KEY_MODE,//long KEY_MENU,// kick KEY_MODE
			NO_KEY,//,
		},
	},
ADKEY_PLATFORM_DATA_END()
    
//menu/mode按键服用  按键重映射,短按位mode,长按位menu
int key_event_remap(struct sys_event *e)
{
	static u8 m_flag = 0;

    if (e->u.key.value == KEY_MODE) {
	   	if(e->u.key.event == KEY_EVENT_HOLD) {
			return false;
		}


   		 	if (e->u.key.event == KEY_EVENT_LONG) {
				m_flag = 1;
				e->u.key.value = KEY_MENU;
				e->u.key.event = KEY_EVENT_CLICK;
			}

    }
	return true;
}


#endif


#define PWR_VALUE_3_50		0x14f//0x15a//0x16A  // 3.5V
#define PWR_VALUE_3_55		0x155//0x160//0x16F  // 3.55V
#define PWR_VALUE_3_60		0x15b//0x165//0x174  // 3.6V
#define PWR_VALUE_3_65		0x16a//0x16a//0x179  // 3.65V
#define PWR_VALUE_3_70		0x166//0x16f//0x17E  // 3.7V
#define PWR_VALUE_3_75		0x16c//0x175//0x183  // 3.75V
#define PWR_VALUE_3_80		0x171//0x179//0x189  // 3.8V
#define PWR_VALUE_3_85		0x177//0x17f//0x18E  // 3.85V
#define PWR_VALUE_3_90		0x17c//0x183//0x18E  // 3.90V
#define PWR_VALUE_3_95		0x181//0x188//0x18E  // 3.90V
#define PWR_VALUE_4_00		0x186//0x18e//0x19D  // 4.00V
#define PWR_VALUE_4_05		0x18c//0x193//0x1A2  // 4.05V
#define PWR_VALUE_4_10		0x191//0x198//0x1A8  // 4.1V
#define PWR_VALUE_4_15		0x196//0x198//0x1A8  // 4.15V
#define PWR_VALUE_4_20		0x19c//0x1a3//0x1B2  // 4.2V

POWERDET_PLATFORM_DATA_BEGIN(pwrdet_data)
	.nlevel = 5,                                        //电压检测等级 
	.power_level = {                                   //每个等级对应的电压 
		PWR_VALUE_3_85,
		PWR_VALUE_3_90,
		PWR_VALUE_3_95,
		PWR_VALUE_4_05,
		PWR_VALUE_4_15,
	},
POWERDET_PLATFORM_DATA_END()

#ifdef CONFIG_IOKEY_ENABLE
/*
 * power键
 */
const struct iokey_port iokey_list[] = {
    {
        .port = IO_PORTA_13,                            //配置start_key  
        .press_value = 1,
        .key_value = KEY_POWER,                         //电源按键消息
    }
};

const struct iokey_platform_data iokey_data = {
    .num = ARRAY_SIZE(iokey_list),
    .port = iokey_list,
};

//start_key 状态读取，用于开机状态判断
unsigned char read_power_key()
{
	gpio_direction_input(IO_PORTA_13);
	return (gpio_read(IO_PORTA_13));
}

#endif


/*
 * spi0接falsh
 */
const struct spi_platform_data spi0_data = {
    .clk    = 60000000,
    .mode   = SPI_DUAL_MODE,
    .port   = SPI_PORTA_0_4_PORTH15,
};
const struct spiflash_platform_data spiflash_data = {
    .name           = "spi0",
    .mode           = FAST_READ_IO_MODE,
    .private_data   = (void *)&spi0_data,
};



const struct dac_platform_data dac_data = {
    .ldo_id = 1,
};
const struct audio_platform_data audio_data = {
    .private_data = (void *)&dac_data,
};


NET_PLATFORM_DATA_BEGIN(net_phy_data)
	.name = (u8 *)"rtl8201",
 	.speed = PHY_FULLDUPLEX_100M,
  	.mode  = RMII_MODE,
	.check_link_time = 100, //100ms
    .mac_addr = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88},
	.rmii_bus = {
		.phy_addr = 0x1,
		.gpioclk = (struct gpio_reg *)(&PORTB_OUT),
		.gpiodat = (struct gpio_reg *)(&PORTB_OUT),
		.clk_pin = 1,
		.dat_pin = 0,
	},
NET_PLATFORM_DATA_END()


#ifdef CONFIG_GSENSOR_ENABLE
extern const struct device_operations gsensor_dev_ops;
#endif // CONFIG_GSENSOR_ENABLE


#ifdef CONFIG_AV10_SPI_ENABLE
extern const struct device_operations _spi_dev_ops;
//av10 spi通讯io口配置
SW_SPI_PLATFORM_DATA_BEGIN(sw_spi_data)
	.gpio_pin_cs = (struct gpio_reg *)(&PORTG_OUT),
	.gpio_pin_clk = (struct gpio_reg *)(&PORTE_OUT),
	.gpio_pin_in = (struct gpio_reg *)(&PORTE_OUT),
	.gpio_pin_out = (struct gpio_reg *)(&PORTE_OUT),
	.pin_cs = 15,
	.pin_clk = 4,
	.pin_in  = 5,
	.pin_out = 5,
SW_SPI_PLATFORM_DATA_END()
#endif // CONFIG_AV10_SPI_ENABLE


//倒车检测初始化
void AV_PARKING_DET_INIT()
{
    gpio_direction_input(IO_PORTA_09);
}
//倒车 io状态检测
unsigned char AV_PARKING_DET_STATUS()
{
    return (!gpio_read(IO_PORTA_09));
}

//用于普通io作为电源控制脚
unsigned char PWR_CTL(unsigned char on_off)
{
    return 0;
}

//usb io状态检测
unsigned char USB_IS_CHARGING()
{
	gpio_direction_input(IO_PORT_PR_01);
	gpio_set_pull_up(IO_PORT_PR_01, 0);
	gpio_set_pull_down(IO_PORT_PR_01, 0);
	gpio_set_die(IO_PORT_PR_01, 1);
	delay(5);
	return (gpio_read(IO_PORT_PR_01));//no usb charing == false
}


//关机唤醒相关io配置
POWER_PLATFORM_DATA_BEGIN(power_data)
	.usb_wkup = {
		.gpio = IO_PORT_PR_01,                      //usb充电唤醒io
		.edge = 0,                                  //0 上升沿唤醒     1 下降沿唤醒 
		.port_en  = 1,                              //io唤醒使能
	},
	.gsen_int_wkup = {
		.gpio = IO_PORT_PR_03,                      //重力感应唤醒io
		.edge = 0,                                  //0 上升沿唤醒     1 下降沿唤醒 
		.port_en = 1,                               //io唤醒使能
	},
	.en_pr = {
		.wkup_en = 1,                               //总唤醒使能
		.wkup_power_gpio = IO_PORT_PR_00,           //唤醒时候，由PRO输出高电平
	},
POWER_PLATFORM_DATA_END()





REGISTER_DEVICES(device_table) = {

#ifdef CONFIG_PAP_ENABLE
    { "pap",   &pap_dev_ops, NULL},
#endif
	{ "lcd",   &lcd_dev_ops, (void*)&lcd_data},

    { "iic1",  &iic_dev_ops, (void *)&hw_iic0_data },
    { "iic0",  &iic_dev_ops, (void *)&sw_iic_data },

    { "audio", &audio_dev_ops, (void *)&audio_data },

#ifdef CONFIG_AV10_SPI_ENABLE
    { "avin_spi",  &_spi_dev_ops, (void *)&sw_spi_data },
#endif

#ifdef CONFIG_SD0_ENABLE
    { "sd0",  &sd0_dev_ops, (void *)&sd0_data },
#endif

#ifdef CONFIG_SD1_ENABLE
    { "sd1",  &sd1_dev_ops, (void *)&sd1_data },
#endif

#ifdef CONFIG_SD2_ENABLE
    { "sd2",  &sd2_dev_ops, (void *)&sd2_data },
#endif

#ifdef CONFIG_ADKEY_ENABLE
    { "adkey", &key_dev_ops, (void *)&adkey_data },
#endif
	{ "powerdet", &powerdet_dev_ops, (void *)&pwrdet_data },
#ifdef CONFIG_IOKEY_ENABLE
    { "iokey", &key_dev_ops, (void *)&iokey_data },
#endif

#ifdef CONFIG_VIDEO0_ENABLE
    { "video0",  &video_dev_ops, (void *)&video0_data},
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    { "video1",  &video_dev_ops, (void *)&video1_data },
#endif

#ifdef CONFIG_VIDEO_DEC_ENABLE
    { "video2",  &video_dev_ops, NULL },
#endif

    { "fb0",  &fb_dev_ops, NULL },
    { "fb1",  &fb_dev_ops, NULL },
    { "fb2",  &fb_dev_ops, NULL },

    { "videoengine",  &video_engine_ops, NULL },

    { "spiflash", &spiflash_dev_ops, (void *)&spiflash_data },

#ifdef CONFIG_ETH_PHY_ENABLE
    /* { "net-phy",  &eth_phy_dev_ops, (void *) &net_phy_data}, */
#endif


#ifdef CONFIG_GSENSOR_ENABLE
    {"gsensor", &gsensor_dev_ops, NULL},
#endif

    {"rtc", &rtc_dev_ops, NULL},

    {"usb0", &usb_dev_ops, (void *)&usb0_data},
    {"power", &power_dev_ops, (void *)&power_data},
};

// *INDENT-ON*

#ifdef CONFIG_DEBUG_ENABLE
void debug_uart_init()
{

    uart_init(&uart0_data);
    if (uart0_data.tx_pin == IO_PORTG_14) {
        //io为普通io，则使用output channel 方式来 输出串口
        IOMC1 &= ~(0x0f << 8);
        PORTG_DIR &= ~BIT(14);
        PORTG_PU |= BIT(14);
        PORTG_PD |= BIT(14);
        PORTG_DIE &= ~BIT(14);
    }
    IOMC3 &= ~BIT(25);//解除串口uart3占用io PB3 and PB4,否则 g_sensor iic无法通讯
}
#endif

void board_init()
{
    CLK_CON0 &=  ~BIT(13);//uboot某测试时钟io pe2输出关断


    // sd power ctrl pin
    PORTB_DIR &= ~ BIT(11);
    PORTB_OUT &= ~BIT(11) ; //
    mipi_phy_con0 &= ~BIT(23);//增加这一句 关闭mipi ldo

    puts("board_init\n");
    devices_init();//设备注册
    puts("board_int: over\n");

    first_time_power_on();//电源控制
}

#endif


