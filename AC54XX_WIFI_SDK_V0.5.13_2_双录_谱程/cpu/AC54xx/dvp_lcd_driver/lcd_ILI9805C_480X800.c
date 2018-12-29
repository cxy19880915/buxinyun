#include "asm/cpu.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"


#ifdef LCD_ILI9805C


/* #define SPI_RST_OUT()   PORTH_DIR &=~ BIT(13) */
#define SPI_RST_H()     gpio_direction_output(IO_PORTH_13, 1);//PORTH_OUT |=  BIT(13)
#define SPI_RST_L()     gpio_direction_output(IO_PORTH_13, 0);//PORTH_OUT &=~ BIT(13)

#define SPI_DCX_OUT()
#define SPI_DCX_L()
#define SPI_DCX_H()

/* #define SPI_CSX_OUT()   PORTE_DIR &=~ BIT(4) */
#define SPI_CSX_H()    gpio_direction_output(IO_PORTE_04, 1);// PORTE_OUT |=  BIT(4)
#define SPI_CSX_L()    gpio_direction_output(IO_PORTE_04, 0);// PORTE_OUT &=~ BIT(4)

/* #define SPI_SCL_OUT()   PORTH_DIR &=~ BIT(12) */
#define SPI_SCL_H()    gpio_direction_output(IO_PORTH_12, 1);// PORTH_OUT |=  BIT(12)
#define SPI_SCL_L()    gpio_direction_output(IO_PORTH_12, 0);// PORTH_OUT &=~ BIT(12)

/* #define SPI_SDA_OUT()   PORTE_DIR &=~ BIT(5) */
/* #define SPI_SDA_IN()    PORTE_DIR |= BIT(5) */
#define SPI_SDA_H()    gpio_direction_output(IO_PORTE_05, 1);// PORTE_OUT |=  BIT(5)
#define SPI_SDA_L()    gpio_direction_output(IO_PORTE_05, 0);// PORTE_OUT &=~ BIT(5)
#define SPI_SDA_R()    gpio_direction_input(IO_PORTE_05); // PORTE_IN & BIT(5)


extern void delay_us(unsigned int us);
extern void delay_2ms(u32 cnt);

static void SPI_WriteComm(u8 cmd)
{
    u8 i;
    SPI_CSX_L();
    ///command
    SPI_SCL_L();
    /* SPI_SDA_OUT(); */
    SPI_SDA_L();
    delay_us(100);
    SPI_SCL_H();
    delay_us(100);
    ///command

    for (i = 0; i < 8; i++) {
        SPI_SCL_L();
        delay_us(100);
        if (cmd & BIT(7)) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }

        cmd <<= 1;
        SPI_SCL_H();
        delay_us(200);
    }
    SPI_CSX_H();
    delay_us(100);
}

static void SPI_WriteData(u8 dat)
{
    u8 i;
    SPI_CSX_L();
    ///data
    SPI_SCL_L();
    /* SPI_SDA_OUT(); */
    SPI_SDA_H();
    delay_us(100);
    SPI_SCL_H();
    delay_us(100);
    ///data

    for (i = 0; i < 8; i++) {
        SPI_SCL_L();
        delay_us(100);
        if (dat & BIT(7)) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }

        dat <<= 1;
        SPI_SCL_H();
        delay_us(500);
    }
    SPI_CSX_H();
    delay_us(100);
}

static void lcd_ili9805c_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    printf("ili9805c_init...\r\n");
    /* spi_io_init(); */

    printf("reset\r\n");
    //RESET
    SPI_RST_H();
    delay_2ms(5);
    SPI_RST_L();
    delay_2ms(5);
    SPI_RST_H();
    delay_2ms(5);

    SPI_WriteComm(0x01); // reset
    delay_2ms(120) ;

    SPI_WriteComm(0x11); // Exit Sleep
    delay_2ms(320) ;

    SPI_WriteComm(0xFF);
    SPI_WriteData(0xFF);
    SPI_WriteData(0x98);
    SPI_WriteData(0x05);

    SPI_WriteComm(0xFD);
    SPI_WriteData(0x0F);
    SPI_WriteData(0x10);
    SPI_WriteData(0x44);
    SPI_WriteData(0x33);

    SPI_WriteComm(0xF8);
    SPI_WriteData(0x10);
    SPI_WriteData(0x02);
    SPI_WriteData(0x02);
    SPI_WriteData(0xb4);
    SPI_WriteData(0x03);
    SPI_WriteData(0x03);
    SPI_WriteData(0x11);
    SPI_WriteData(0x03);

    SPI_WriteComm(0xF1);
    SPI_WriteData(0xc1);
    SPI_WriteData(0x08);
    SPI_WriteData(0x00);
    SPI_WriteData(0x2b);
    SPI_WriteData(0x01);

    SPI_WriteComm(0xE0);
    SPI_WriteData(0x00);
    SPI_WriteData(0x07);
    SPI_WriteData(0x13);
    SPI_WriteData(0x12);
    SPI_WriteData(0x10);
    SPI_WriteData(0x09);
    SPI_WriteData(0x06);
    SPI_WriteData(0x03);
    SPI_WriteData(0x00);
    SPI_WriteData(0x07);
    SPI_WriteData(0x0a);
    SPI_WriteData(0x0b);
    SPI_WriteData(0x0c);
    SPI_WriteData(0x28);
    SPI_WriteData(0x2a);
    SPI_WriteData(0x00);

    SPI_WriteComm(0xE1);
    SPI_WriteData(0x00);
    SPI_WriteData(0x07);
    SPI_WriteData(0x10);
    SPI_WriteData(0x10);
    SPI_WriteData(0x13);
    SPI_WriteData(0x1a);
    SPI_WriteData(0x08);
    SPI_WriteData(0x08);
    SPI_WriteData(0x05);
    SPI_WriteData(0x0a);
    SPI_WriteData(0x05);
    SPI_WriteData(0x13);
    SPI_WriteData(0x10);
    SPI_WriteData(0x30);
    SPI_WriteData(0x25);
    SPI_WriteData(0x00);

    SPI_WriteComm(0xc1);
    SPI_WriteData(0x13);
    SPI_WriteData(0x3D);
    SPI_WriteData(0x1D);
    SPI_WriteData(0x26);

    SPI_WriteComm(0xc7);
    SPI_WriteData(0xAC);

    SPI_WriteComm(0xdf);
    SPI_WriteData(0x23);

    SPI_WriteComm(0xb0);
    SPI_WriteData(0x01);

    SPI_WriteComm(0xb9);
    SPI_WriteData(0x08);
    /* SPI_WriteData(0x00); */
    SPI_WriteData(0x00);

    SPI_WriteComm(0xb4);
    SPI_WriteData(0x00);

#if 0
    SPI_WriteComm(0xb5);// add by zhuo
    SPI_WriteData(0x00);
    SPI_WriteData(0x14);
    SPI_WriteData(0x14);
    SPI_WriteData(0x00);
    SPI_WriteData(0x14);
    SPI_WriteData(0x01);
    SPI_WriteData(0x8F);

#endif

    SPI_WriteComm(0x3A);
    SPI_WriteData(0x70);

    SPI_WriteComm(0xc2);
    SPI_WriteData(0x11);

    SPI_WriteComm(0xbd);
    SPI_WriteData(0x00);
    SPI_WriteData(0x42);
    SPI_WriteData(0x00);

    SPI_WriteComm(0xe8);
    SPI_WriteData(0x24);



    SPI_WriteComm(0x29);
    delay_2ms(50) ;



    printf("initial ok.\r\n");

}


static void lcd_ili9805c_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}


REGISTER_LCD_DEVICE(lcd_ili9805c_dev) = {
    .info = {
        .test_mode 	     = false,
        /* .test_mode 	     = true, */
        .color           = 0x0000FF,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,

        .len 			 = LEN_256,
        .rotate_en       = true,	    // 旋转使能
        .hori_mirror_en  = true,	    // 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能

        .itp_mode_en     = FALSE,
        .sample          = LCD_DEV_SAMPLE,
        .format          = FORMAT_RGB888,
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

    .drive_mode      = MODE_RGB_DE_SYNC,
    .ncycle          = CYCLE_ONE,
    .interlaced_mode = INTERLACED_NONE,
    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN | CLK_EN, // |CLK_NEGATIVE,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
    .data_width      = PORT_24BITS,
    .port_sel        = PORT_GROUP_AA,
    .clk_cfg    	 = PLL3_CLK | DIVA_1 | DIVB_4 | DIVC_4,

    .timing = {
        .hori_interval 		=    646,
        .hori_sync_clk 		=    10,
        .hori_start_clk 	=    20,
        .hori_pixel_width 	=    480,
        .vert_interval 		=    864,
        .vert_sync_clk 		=    2,//14,
        .vert_start_clk 	=    2,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    800,
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_DVP_RGB,
    .init 	 = lcd_ili9805c_init,
    .dev  	 = &lcd_ili9805c_dev,
    .bl_ctrl = lcd_ili9805c_backctrl,
};
#endif

