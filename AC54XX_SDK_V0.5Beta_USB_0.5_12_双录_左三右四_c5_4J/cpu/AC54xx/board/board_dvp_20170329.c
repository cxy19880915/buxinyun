#include "system/includes.h"

#include "app_config.h"
#include "gSensor_manage.h"
#include "device/av10_spi.h"

// *INDENT-OFF*

#ifdef CONFIG_BOARD_DVP_20170329


UART0_PLATFORM_DATA_BEGIN(uart0_data)
	.baudrate = 115200,//460800,//115200,
 	.tx_pin = IO_PORTH_13,
  	.flags = UART_DEBUG,
UART0_PLATFORM_DATA_END();

#ifdef CONFIG_SD0_ENABLE

SD0_PLATFORM_DATA_BEGIN(sd0_data)
	.port 					= 'C',
	.priority 				= 3,
	.speed 					= 3,
	.data_width 			= 4,
	.detect_mode 			= SD_CMD_DECT,
	.detect_func 			= sdmmc_0_clk_detect,
SD0_PLATFORM_DATA_END()

#endif //CONFIG_SD0_ENABLE

#ifdef CONFIG_SD1_ENABLE

int sdmmc_1_io_detect(const struct sdmmc_platform_data *data)
{
    int i;
    int val1, val2;

    PORTB_DIR |= BIT(11);
    PORTB_PU  |= BIT(11);
    PORTB_PD  &= ~BIT(11);

    for (i=0; i < 20; i++) {
        delay(100);
        val1 = PORTB_IN & BIT(11);
        delay(100);
        val2 = PORTB_IN & BIT(11);
        if (val1 == val2) {
            return !val1;
        }
    }

    return 0;
}

SD1_PLATFORM_DATA_BEGIN(sd1_data)
	.port 					= 'C',
	.priority 				= 3,
	.speed 					= 3,
	.data_width 			= 4,
	.detect_mode 			= SD_IO_DECT,
	.detect_func 			= sdmmc_1_io_detect,
SD1_PLATFORM_DATA_END()

#endif //CONFIG_SD1_ENABLE

#ifdef CONFIG_SD2_ENABLE

SD2_PLATFORM_DATA_BEGIN(sd2_data)
	.port 					= 'B',
	.priority 				= 3,
	.speed 					= 1,
	.data_width 			= 4,
	.detect_mode 			= SD_CMD_DECT,
	.detect_func 			= sdmmc_2_clk_detect,
SD2_PLATFORM_DATA_END()

#endif //CONFIG_SD2_ENABLE


HW_IIC1_PLATFORM_DATA_BEGIN(hw_iic_data)
	.clk_pin = IO_PORTB_00,//IO_PORTD_14,
	.dat_pin = IO_PORTB_01,//IO_PORTD_15,
	.baudrate = 0x42,//300k  0x50 250k
HW_IIC1_PLATFORM_DATA_END()

SW_IIC_PLATFORM_DATA_BEGIN(sw_iic_data)
	.gpioclk = (struct gpio_reg *)(&PORTB_OUT),
	.gpiodat = (struct gpio_reg *)(&PORTB_OUT),
	.clk_pin = 0,
	.dat_pin = 1,
	.sw_iic_delay = 50,
SW_IIC_PLATFORM_DATA_END()


CAMERA_PLATFORM_DATA_BEGIN(camera0_data)
    .xclk_gpio  = IO_PORTH_09,
    .reset_gpio = IO_PORTH_10,
    .pwdn_gpio  = -1,
    .interface  = SEN_INTERFACE0,
    .dvp={
            .pclk_gpio   = IO_PORTA_15,
            .hsync_gpio  = IO_PORTE_00,
            .vsync_gpio  = IO_PORTE_01,
			.io_function_sel = DVP_SENSOR0(1),
            .data_gpio={
                    IO_PORTA_05,
                    IO_PORTA_06,
                    IO_PORTA_07,
                    IO_PORTA_08,
                    IO_PORTA_09,
                    IO_PORTA_10,
                    IO_PORTA_11,
                    IO_PORTA_12,
                    IO_PORTA_13,
                    IO_PORTA_14,
                    },
CAMERA_PLATFORM_DATA_END()


CAMERA_PLATFORM_DATA_BEGIN(camera1_data)
    .xclk_gpio  = -1,
    .reset_gpio = -1,
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
#define ADC0_04   (0x3ff*04/33) //0x7C
#define ADC0_00   (0)

#define ADKEY_V_0      	((ADC0_33 + ADC0_30)/2)
#define ADKEY_V_1 		((ADC0_30 + ADC0_27)/2)
#define ADKEY_V_2 		((ADC0_27 + ADC0_23)/2)
#define ADKEY_V_3 		((ADC0_23 + ADC0_20)/2)
#define ADKEY_V_4 		((ADC0_20 + ADC0_17)/2)
#define ADKEY_V_5 		((ADC0_17 + ADC0_13)/2)
#define ADKEY_V_6 		((ADC0_13 + ADC0_10)/2)
#define ADKEY_V_7 		((ADC0_10 + ADC0_07)/2)
#define ADKEY_V_8 		((ADC0_07 + ADC0_04)/2)
#define ADKEY_V_9 		((ADC0_04 + ADC0_00)/2)
#define ADKEY_V_10 		(ADC0_00)


ADKEY_PLATFORM_DATA_BEGIN(adkey_data)
	.io 		= IO_PORTB_12,
	.ad_channel = AD_KEY_PA2,
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
			KEY_LEFT,
			KEY_MODE,  /*5*/
			KEY_OK,
			KEY_UP,
			KEY_DOWN,
			KEY_MENU,
			KEY_RIGHT,
		},
	},
ADKEY_PLATFORM_DATA_END()

#endif

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
//以下io为临时配置，还需根据原理图来调整
SW_SPI_PLATFORM_DATA_BEGIN(sw_spi_data)
	.gpio_pin_cs = (struct gpio_reg *)(&PORTG_OUT),
	.gpio_pin_clk = (struct gpio_reg *)(&PORTG_OUT),
	.gpio_pin_in = (struct gpio_reg *)(&PORTG_OUT),
	.gpio_pin_out = (struct gpio_reg *)(&PORTG_OUT),
	.pin_cs = 2,
	.pin_clk = 3,
	.pin_in  = 4,
	.pin_out = 4,
SW_SPI_PLATFORM_DATA_END()
#endif // CONFIG_AV10_SPI_ENABLE


// *INDENT-ON*

const struct dev_node device_table[] = {

    { "iic0",  &iic_dev_ops, (void *) &hw_iic_data },
    { "iic1",  &iic_dev_ops, (void *) &sw_iic_data },

#ifdef CONFIG_AV10_SPI_ENABLE
    { "avin_spi",  &_spi_dev_ops, (void *) &sw_spi_data },
#endif // CONFIG_AV10_SPI_ENABLE

    { "camera0", &camera_dev_ops, (void *) &camera0_data},
    { "camera1", &camera_dev_ops, (void *) &camera1_data},

#ifdef CONFIG_SD0_ENABLE
    { "sd0",  &sd0_dev_ops, (void *) &sd0_data },
#endif

#ifdef CONFIG_SD1_ENABLE
    { "sd1",  &sd1_dev_ops, (void *) &sd1_data },
#endif

#ifdef CONFIG_SD2_ENABLE
    { "sd2",  &sd2_dev_ops, (void *) &sd2_data },
#endif

#ifdef CONFIG_ADKEY_ENABLE
    { "adkey", &adkey_dev_ops, (void *) &adkey_data },
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

    { "spiflash", &spiflash_dev_ops, NULL },

#if 0//just for test
    { "spi1flash", &spiflash_dev_ops, NULL },
    { "spi2flash", &spiflash_dev_ops, NULL },
#endif
#ifdef CONFIG_ETH_PHY_ENABLE
    /* { "net-phy",  &eth_phy_dev_ops, (void *) &net_phy_data}, */
#endif


#ifdef CONFIG_GSENSOR_ENABLE
    {"gsensor", &gsensor_dev_ops, NULL},
#endif

#ifdef CONFIG_PAP_ENABLE
    {"pap", &pap_dev_ops, NULL},
#endif

    {"audio", &audio_dev_ops, NULL},
    {"usb0", &usb_dev_ops, &usb0_data},
};


#ifdef CONFIG_DEBUG_ENABLE
void debug_uart_init()
{
    uart_init(&uart0_data);

    IOMC0 |= BIT(7);
    IOMC1 &= ~(0x0f << 8);
    PORTH_DIR &= ~BIT(13);
    PORTH_PU |= BIT(13);
    PORTH_PD |= BIT(13);
    PORTH_DIE &= ~BIT(13);
}
#endif

void board_init()
{
    /*LDO_CON |= BIT(10) | BIT(9);*/

    puts("board_init\n");
    register_devices(device_table, ARRAY_SIZE(device_table));
    puts("board_int: over\n");

}

#endif


