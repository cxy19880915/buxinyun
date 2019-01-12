#include "generic/irq.h"
#include "sys/event.h"
#include "device/device.h"
#include "fs/fs.h"

#include "asm/adkey.h"
#include "asm/uart.h"
#include "asm/sdmmc.h"
#include "asm/video.h"
#include "asm/fb.h"
#include "asm/spiflash.h"


#define CONFIG_SD1_ENABLE
#define CONFIG_VIDEO0_ENABLE
#define CONFIG_ADKEY_ENABLE




UART1_PLATFORM_DATA_BEGIN()
.baudrate = 115200,
 .tx_pin = 2,
  .rx_pin = 3,
   .port_dir = &PORTC_DIR,
    .flags = UART_DEBUG | UART_DMA_SUPPORT,
     UART1_PLATFORM_DATA_END()



#ifdef CONFIG_SD1_ENABLE

     SD1_PLATFORM_DATA_BEGIN()
     .port 					= 'A',
                   .priority 				= 3,
                              .speed 					= 3,
                                           .data_width 			= 4,
                                                     .detect_mode 			= SD_CMD_DECT,
                                                              .detect_func 			= sdmmc_1_clk_detect,
                                                                       SD1_PLATFORM_DATA_END()

#endif //CONFIG_SD1_ENABLE


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

                                                                       ADKEY_PLATFORM_DATA_BEGIN()
                                                                       .io 		= 2,
                                                                              .ad_channel = AD_KEY_PA2,
                                                                               .port 		= &PORTA_OUT,
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
        KEY_MODE,
        KEY_UP,    /*5*/
        KEY_DOWN,
        KEY_OK,
        KEY_LEFT,
        KEY_RIGHT,
        KEY_MENU,   /*10*/
    },
},
ADKEY_PLATFORM_DATA_END()

#endif



const struct dev_node device_table[] = {
    {
        .name = "uart1",
        .ops = &uart_dev_ops,
        .priv_data = &uart1_data,
    },

#ifdef CONFIG_SD1_ENABLE
    {
        .name = "sd1",
        .ops = &sd1_dev_ops,
        .priv_data = &sd1_data,
    },
#endif


#ifdef CONFIG_ADKEY_ENABLE
    {
        .name 	= "adkey",
        .ops 	= &adkey_dev_ops,
        .priv_data = &adkey_data,
    },
#endif

#ifdef CONFIG_VIDEO0_ENABLE
    {
        .name = "video0",
        .ops = &video_dev_ops,
    },
#endif

    {
        .name = "fb0",
        .ops = &fb_dev_ops,
    },

    {
        .name = "fb1",
        .ops = &fb_dev_ops,
    },

    {
        .name = "spiflash",
        .ops = &spiflash_dev_ops,
    },
};




#define ALCD_BL_ON()     PORTF_DIR &= ~BIT(0);PORTF_OUT |= BIT(0)

void board_init()
{
    LDO_CON |= (BIT(10) | BIT(9));//2.8V 1.8V输出使能
    ALCD_BL_ON();


    register_devices(device_table, ARRAY_SIZE(device_table));
    first_time_power_on();
}




