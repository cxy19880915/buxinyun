#include "system/includes.h"

#include "app_config.h"
#include "gSensor_manage.h"
#include "device/av10_spi.h"

#include "power_ctrl.h"
// *INDENT-OFF*

#ifdef CONFIG_BOARD_MIPI_V2

/* #define UART0 */

UART0_PLATFORM_DATA_BEGIN(uart0_data)
    .baudrate = 115200,
    .tx_pin = IO_PORTE_00,
    .flags = UART_DEBUG,
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
    int i;
    int val1, val2;

    PORTA_DIR |= BIT(11);
    PORTA_PU  |= BIT(11);
    PORTA_PD  &= ~BIT(11);

    for (i=0; i < 20; i++) {
        delay(100);
        val1 = PORTA_IN & BIT(11);
        delay(100);
        val2 = PORTA_IN & BIT(11);
        if (val1 == val2) {
            return !val1;
        }
    }

    return 0;
}

SD1_PLATFORM_DATA_BEGIN(sd1_data)
	.port 					= 'C',
	.priority 				= 3,
	.data_width 			= 4,
	.speed 					= 30000000,
	.detect_mode 			= SD_IO_DECT,
	.detect_func 			= sdmmc_1_io_detect,
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
	.clk_pin = IO_PORTB_04,//IO_PORTD_14,
	.dat_pin = IO_PORTB_03,//IO_PORTD_15,
	.baudrate = 23,
HW_IIC0_PLATFORM_DATA_END()

HW_IIC1_PLATFORM_DATA_BEGIN(hw_iic1_data)
	.clk_pin = IO_PORTB_00,//IO_PORTD_14,
	.dat_pin = IO_PORTB_01,//IO_PORTD_15,
	.baudrate = 0xff,
HW_IIC1_PLATFORM_DATA_END()

SW_IIC_PLATFORM_DATA_BEGIN(sw_iic_data)
	.gpioclk = (struct gpio_reg *)(&PORTB_OUT),
	.gpiodat = (struct gpio_reg *)(&PORTB_OUT),
	.clk_pin = 0,
	.dat_pin = 1,
	.sw_iic_delay = 50,
SW_IIC_PLATFORM_DATA_END()


LCD_PLATFORM_DATA_BEGIN(lcd_data)
	.lcd_name = (u8*)"mipi_4lane_720p_rm68200",
	.interface = LCD_MIPI,
	.lcd_io = {
		.lcd_reset = IO_PORTG_04,
		/* .lcd_backlight = IO_PORTG_02, */
	}
LCD_PLATFORM_DATA_END()


CAMERA_PLATFORM_DATA_BEGIN(camera0_data)
    .xclk_gpio  = IO_PORTA_14,
	.reset_gpio = IO_PORTA_15,
	.pwdn_gpio  = IO_PORTE_01,
	.interface  = SEN_INTERFACE_CSI2,
    .csi2 = {
        .data_lane_num = 2,
        .clk_inv = 1,
        .d0_rmap = CSI2_X4_LANE,
        .d0_inv = 1,
        .d1_rmap = CSI2_X3_LANE,
        .d1_inv = 1,
        .tval_hstt = 12,
        .tval_stto = 12,
    }
CAMERA_PLATFORM_DATA_END()


CAMERA_PLATFORM_DATA_BEGIN(camera1_data)
    .xclk_gpio  = -1,
	.reset_gpio = IO_PORTA_06,
	.pwdn_gpio  = -1,
	.interface  = SEN_INTERFACE1,
    .dvp={
        .pclk_gpio  = IO_PORTH_00,
        .hsync_gpio  = -1,
        .vsync_gpio  = -1,
		.io_function_sel = DVP_SENSOR1(0),
        .data_gpio={
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
CAMERA_PLATFORM_DATA_END()



VIDEO_PLATFORM_DATA_BEGIN(video_data)
    .data = (void *)VIDEO_REC_NUM,
VIDEO_PLATFORM_DATA_END()

USB_PLATFORM_DATA_BEGIN(usb0_data)
    .id = 0,
    .online_check_cnt = 5,
    .offline_check_cnt = 250,
    .isr_priority = 4,
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
#define ADC0_10   (0x3ff*10/33) //0x136
#define ADC0_07   (0x3ff*07/33) //0xD9
#define ADC0_05   (0x3ff*05/33) //0x7C
#define ADC0_00   (0)

#define ADKEY_V_0      	((ADC0_33 + ADC0_30)/2)
#define ADKEY_V_1 		((ADC0_30 + ADC0_27)/2)
#define ADKEY_V_2 		((ADC0_27 + ADC0_23)/2)
#define ADKEY_V_3 		((ADC0_23 + ADC0_20)/2)
#define ADKEY_V_4 		((ADC0_20 + ADC0_17)/2)
#define ADKEY_V_5 		((ADC0_17 + ADC0_13)/2)
#define ADKEY_V_6 		((ADC0_13 + ADC0_10)/2)
#define ADKEY_V_7 		((ADC0_10 + ADC0_07)/2)
#define ADKEY_V_8 		((ADC0_07 + ADC0_05)/2)
#define ADKEY_V_9 		((ADC0_05 + ADC0_00)/2)
#define ADKEY_V_10 		(ADC0_00)


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
			KEY_MENU,
			KEY_MENU,  /*5*/
			NO_KEY,
			KEY_MODE,
			KEY_UP,
			KEY_DOWN,
			KEY_OK,
		},
	},
ADKEY_PLATFORM_DATA_END()


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
	.nlevel = 5,
	.power_level = {
		PWR_VALUE_3_50,
		PWR_VALUE_3_75,
		PWR_VALUE_3_85,
		PWR_VALUE_3_95,
		PWR_VALUE_4_05,
	},
POWERDET_PLATFORM_DATA_END()

#ifdef CONFIG_IOKEY_ENABLE
/*
 * power键
 */
const struct iokey_port iokey_list[] = {
    {
        .port = IO_PORTA_13,
        .press_value = 1,
        .key_value = KEY_POWER,
    }
};

const struct iokey_platform_data iokey_data = {
    .num = ARRAY_SIZE(iokey_list),
    .port = iokey_list,
};


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
    .mode   = SPI_ODD_MODE,
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



#ifdef CONFIG_GSENSOR_ENABLE
extern const struct device_operations gsensor_dev_ops;
#endif // CONFIG_GSENSOR_ENABLE


#ifdef CONFIG_AV10_SPI_ENABLE
extern const struct device_operations _spi_dev_ops;
//以下io为临时配置，还需根据原理图来调整
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


void AV_PCTL()
{
    gpio_direction_output(IO_PORTB_12, 1);
}

void av_in_det_init()
{
    gpio_direction_input(IO_PORTA_10);
}

unsigned char av_in_status()
{
    return (!gpio_read(IO_PORTA_10));
}

void av_parking_det_init()
{
    gpio_direction_input(IO_PORTA_09);
}

unsigned char av_parking_det_status()
{
    return (!gpio_read(IO_PORTA_09));
}
unsigned char PWR_CTL(unsigned char on_off)
{
    on_off = on_off? 1:0;
    /* gpio_direction_output(IO_PORTE_04, on_off); */
    return 0;
}

unsigned char DSI_BL(unsigned char value)
{
    static u8 flag = 0;
    if (!flag)
    {
        flag = 1;
		CLK_CON0 &=  ~BIT(13);//uboot某测试时钟io pe2输出关断
    }

    value = value? 1:0;
    gpio_direction_output(IO_PORTG_02, value);
    return 0;
}
#define USB_WKUP_IO 	IO_PORT_PR_01
#define GSEN_WKUP_IO 	IO_PORT_PR_02
POWER_PLATFORM_DATA_BEGIN(power_data)
	.usb_wkup = {
		.gpio = USB_WKUP_IO,
		.edge = 0,
		.port_en  = 1,
	},
	.gsen_int_wkup = {
		.gpio = GSEN_WKUP_IO,
		.edge = 0,
		.port_en = 1,
	},
	.en_pr = {
		.wkup_en = 1,
		.wkup_power_gpio = IO_PORT_PR_00,
	},
POWER_PLATFORM_DATA_END()


unsigned char usb_is_charging()
{
	static unsigned char init = 0;
	if (!init){
		init = 1;
		gpio_direction_input(USB_WKUP_IO);
		gpio_set_pull_up(USB_WKUP_IO, 0);
		gpio_set_pull_down(USB_WKUP_IO, 0);
		gpio_set_die(USB_WKUP_IO, 1);
		delay(10);
	}

	return (gpio_read(USB_WKUP_IO));//no usb charing == false
}


unsigned int get_usb_wkup_gpio()
{
	return (USB_WKUP_IO);
}



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

    { "camera0", &camera_dev_ops, (void *)&camera0_data},
    { "camera1", &camera_dev_ops, (void *)&camera1_data},

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
    { "video0",  &video_dev_ops, (void *) &video_data},
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    { "video1",  &video_dev_ops, NULL },
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
    if (uart0_data.tx_pin == IO_PORTE_00) {
        IOMC1 &= ~(0x0f << 8);
        PORTE_DIR &= ~BIT(0);
        PORTE_PU |= BIT(0);
        PORTE_PD |= BIT(0);
        PORTE_DIE &= ~BIT(0);
    }
    IOMC3 &= ~BIT(25);//解除串口uart3占用io PB3 and PB4,否则 g_sensor iic无法通讯
}
#endif

void board_init()
{

    // sd power ctrl pin
    PORTB_DIR &= ~ BIT(11);
    PORTB_OUT &= ~BIT(11) ; //
    mipi_phy_con0 &= ~BIT(23);//增加这一句 关闭mipi ldo

    puts("board_init\n");
    devices_init();
    puts("board_int: over\n");

    first_time_power_on();
}

#endif


