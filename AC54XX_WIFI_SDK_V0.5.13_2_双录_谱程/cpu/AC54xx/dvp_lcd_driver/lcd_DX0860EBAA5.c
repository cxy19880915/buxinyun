#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/pap.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
/* #if (LCD_TYPE == LCD_RGB_DX0860EBAA5) */
#ifdef LCD_RGB_DX0860EBAA5

#define SPI_RST_OUT()   //gpio_direction_output(IO_PORTG_04, 0);
#define SPI_RST_H()    // gpio_direction_output(IO_PORTG_04, 1)
#define SPI_RST_L()    // gpio_direction_output(IO_PORTG_04, 0)

//CS  PC5
#define SPI_CS_OUT()    //PORTG_DIR &= ~BIT(3)
#define SPI_CS_EN()     gpio_direction_output(IO_PORTH_06, 0)
#define SPI_CS_DIS()    gpio_direction_output(IO_PORTH_06, 1)
//SDA PC6
#define SPI_SDA_OUT()   //PORTG_DIR &= ~BIT(5);PORTB_PU &=~ BIT(5)
#define SPI_SDA_IN()    gpio_direction_input(IO_PORTH_01)//PORTG_DIR |=  BIT(5);PORTB_PU |= BIT(5);PORTB_PD &=~ BIT(5)
#define SPI_SDA_H()     gpio_direction_output(IO_PORTH_01, 1)//PORTG_OUT |=  BIT(5)
#define SPI_SDA_L()     gpio_direction_output(IO_PORTH_01, 0)
#define SPI_SDA_R()     gpio_read(IO_PORTH_01)
//SCL PC7
#define SPI_CLK_OUT()   //PORTG_DIR &= ~BIT(2)
#define SPI_CLK_H()     gpio_direction_output(IO_PORTH_00, 1)
#define SPI_CLK_L()     gpio_direction_output(IO_PORTH_00, 0)
extern void delay_2ms(u32 cnt);
//初始化说明：
//开机：
//1.8V和3.3V电源上电
//初始化VP，DCLK，VSYNC，HSYNC，D0~D7都输出。
//初始化LCD。注意上电到此步保证在0.6秒以上。
//开LCD背光。
//关机：
//关LCD背光
//SPI发送0x04指令
//等待200ms
//SPI的CS，SCLK，MOSI置低电平
//等待50ms
//1.8V和3.3V电源断电
void DX0860EBAA5_init();
void DX0860EBAA5_Poweroff(void);
const u16 init_dat[] = {
    0x0004	,
    0xBE02	,
    0xC240	,

    0x0220,//08	,
    0x0430,//F7
    0x061b,//23	,
    0x083A	,
    0x0A1A	,
    0x0C13	,
    0x0E11	,
    0x2240	,
    0x2440	,
    0x2C1E	,
    0x2E3E	,
    0x3080	,
    0x32C0	,
    0x34E0	,
    0x361E	,
    0x383E	,
    0x3A80	,
    0x3CC0	,
    0x3EE0	,
    0x401E	,
    0x423E	,
    0x4480	,
    0x46C0	,
    0x48E0	,
    0x4A68,//80	,
    0x4C5f,//80	,
    0x4E80	,
    0x8002	,
    0x820B	,
    0x84FF	,
    0x865A	,
    0x88A6	,
    0x8A43	,
    0x8CE7	,
    0x8E07	,
    0x9009	,
    0x9208	,
    0x9408	,
    0x960B	,
    0x98FF	,
    0x9A5A	,
    0x9CA6	,
    0x9E43	,
    0xA0E7	,
    0xA207	,	//亮度变化
    0xA409	,	//亮度变化
    0xA602	,	//亮度变化

    0xBE01	,
    0xC000	,
    0xBE02	,
    0xCA01	,
    0x4A68	,
    0x4C5F	,
    0x4E80	,

#if 1
    //------------
    0x1001,
    0xBE01,
    0xE401,
    0xF020,
#else
    //------------
    0x1000,
    0xBE01,
    0xE401,
    0xF09D,
#endif
    0x01	,
};

void spi_reg_write(u16 reg)
{
    u8 i;
    u16 pctrl;
    SPI_CS_EN();

    pctrl = reg;
    for (i = 0; i < 16; i++) {
        SPI_CLK_L();// = 0;
        if (pctrl & 0x8000) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }
        delay(500);
        pctrl = pctrl << 1;
        SPI_CLK_H();
        delay(500);
    }
    SPI_CS_DIS();
}
static void spi_io_init()
{
    SPI_RST_OUT();
    SPI_CS_OUT();
    SPI_SDA_OUT();
    SPI_CLK_OUT();

    SPI_CS_DIS();
    SPI_CLK_H();
    SPI_SDA_H();

    SPI_RST_H();
    /* delay_2ms(500); */
    delay(500);
    SPI_RST_L();
    delay(500);
    /* delay_2ms(500); */
    SPI_RST_H();
    /* delay_2ms(50); */
    printf("---------DX0860EBAA5 init ----------\n\n");
}

void DX0860EBAA5_init1(void)
{
    spi_io_init();
    /*DX0860EBAA5_init();*/

};
void DX0860EBAA5_Poweroff(void)
{
    spi_reg_write(init_dat[0]);
    delay_2ms(100);
    SPI_CS_DIS();
    SPI_CLK_L();
    SPI_SDA_L();
    delay_2ms(51);

    printf("DX0860EBAA5_Poweroff \n");
};
void DX0860EBAA5_init(void)
{
    u8 i, j;
    j = sizeof(init_dat) / 2;

    //delay_2ms(500);
    // spi_reg_write(init_dat[0]);
    // delay_2ms(100);

    /* delay_2ms(100); */

    delay(500);
    for (i = 1; i < 3; i++) {
        spi_reg_write(init_dat[i]);
    }
    delay_2ms(5);
    for (i = 3; i < j - 5; i++) {
        spi_reg_write(init_dat[i]);
    }
    delay_2ms(5);
    for (; i < j; i++) {
        spi_reg_write(init_dat[i]);
    }
    delay_2ms(100);
    //spi_reg_write(0x0001);
    //delay_2ms(100);



    printf("DX0860EBAA5_init over \n");
}


static void lcd_DX0860EBAA5_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    if (on) {
        printf("\n>>>>>>>>>>>>>>>>>>>lcd_bl_init on\n\n");
        DX0860EBAA5_init();
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        printf("\n>>>>>>>>>>>>>>>>>>>lcd_bl_init onff\n\n");
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }



}

REGISTER_LCD_DEVICE(lcd_DX0860EBAA5_dev) = {
    .info = {
        .test_mode 	     = false,
        .color           = 0xffffff,//BRG
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,

        .rotate_en       = false,	    // ��תʹ��
        .hori_mirror_en  = false,	    // ˮƽ����ʹ��
        .vert_mirror_en  = false,		// ��ֱ����ʹ��
        .len 			 = LEN_256,
        .itp_mode_en     = false,
        .sample          = SAMP_YUV420,
        .format          = FORMAT_YUV422,
        //        .format          = FORMAT_ITU656,

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

    .drive_mode      = MODE_RGB_SYNC,
    .ncycle          = CYCLE_TWO,
    .interlaced_mode = INTERLACED_NONE,
    //        .interlaced_mode = INTERLACED_ALL,
    //        .interlaced_mode = INTERLACED_DATA,
    //    .raw_odd_phase   = PHASE_R,
    //    .raw_even_phase  = PHASE_R,
    //    .raw_even_phase  = PHASE_G,
    //  .raw_mode        = RAW_STR_MODE,
    //    .raw_mode        = RAW_DLT_MODE,
    .dclk_set 		 = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN, //| CLK_EN, //SIGNAL_DEN;
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    //    .sync1_set       = SIGNAL_HSYNC | CLK_EN ,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
    //  .sync2_set       = SIGNAL_VSYNC | CLK_EN ,
    .data_width      = PORT_8BITS,
    .port_sel        = PORT_GROUP_AA,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_1 | DIVC_3,
    //    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_1 | DIVC_3,

    .timing = {
        /*  .hori_interval 		= 1756+100,//1716 */
        /* .hori_sync_clk 		= 15,//4 */

        /* .hori_start_clk 	    = 48, */
        /* .hori_pixel_width 	= 720, */

        /* .vert_interval 		= 535+100,//625 */
        /* .vert_sync_clk 		= 6+6,//1 */
        /* .vert_start_clk 	    = 27, */
        /* .vert_start1_clk 	= 0, */
        /* .vert_pixel_width 	= 480, */

        .hori_interval 		=    1440 + 50, //1716,//1716/2,
        .hori_sync_clk 		=    1,
        .hori_start_clk 	=    48,//240/2,
        .hori_pixel_width 	=    720,//1440/2,

        .vert_interval 		=    480 + 50,
        .vert_sync_clk 		=    12,//42,
        .vert_start_clk 	=    27,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    480,

        //YUV720 720x240 two clock
        //        .hori_interval 		=    1716,//1716/2,
        //        .hori_sync_clk 		=    1,
        //        .hori_start_clk 	=    240,//240/2,
        //        .hori_pixel_width 	=    720,//1440/2,
        //
        //        .vert_interval 		=    263,
        //        .vert_sync_clk 		=    1,//42,
        //        .vert_start_clk 	=    21,
        //        .vert_start1_clk 	=    23,
        //        .vert_pixel_width 	=    240,
    },
};

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    //.logo 	 = "DX0860EBAA5",
    .type 	 = LCD_DVP_RGB,
    .init 	 = DX0860EBAA5_init1,
    .dev  	 = &lcd_DX0860EBAA5_dev,
    .bl_ctrl = lcd_DX0860EBAA5_backctrl,
};
#endif



