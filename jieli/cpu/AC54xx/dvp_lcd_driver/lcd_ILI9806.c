#include "asm/cpu.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"


#if (LCD_TYPE == LCD_ILI9806)

//PC6->PV6 RESET
#define SPI_RST_OUT()   PORTB_DIR &=~ BIT(6)
#define SPI_RST_H()     PORTB_OUT |=  BIT(6)
#define SPI_RST_L()     PORTB_OUT &=~ BIT(6)

#define SPI_DCX_OUT()
#define SPI_DCX_L()
#define SPI_DCX_H()

//PH4->PH2 CS
#define SPI_CSX_OUT()   PORTD_DIR &=~ BIT(12)
#define SPI_CSX_H()     PORTD_OUT |=  BIT(12)
#define SPI_CSX_L()     PORTD_OUT &=~ BIT(12)

//PC5->PV5 SCL
#define SPI_SCL_OUT()   PORTB_DIR &=~ BIT(5)
#define SPI_SCL_H()     PORTB_OUT |=  BIT(5)
#define SPI_SCL_L()     PORTB_OUT &=~ BIT(5)

//PC7->PV7 SDA
#define SPI_SDA_OUT()   PORTB_DIR &=~ BIT(7)
#define SPI_SDA_IN()    PORTB_DIR |= BIT(7)
#define SPI_SDA_H()     PORTB_OUT |=  BIT(7)
#define SPI_SDA_L()     PORTB_OUT &=~ BIT(7)
#define SPI_SDA_R()     PORTB_IN & BIT(7)

/* #define MOVE_MODE   //部分显示 */

extern void delay_us(unsigned int us);

static void spi_io_init()
{
    SPI_DCX_OUT();
    SPI_SCL_OUT();
    SPI_CSX_OUT();
    SPI_SDA_OUT();
    SPI_RST_OUT();

    SPI_SDA_H();
    SPI_CSX_H();
    SPI_DCX_L();
    SPI_SCL_L();
}

static void spi_write_cmd(u8 cmd)
{
    u8 i;
    SPI_CSX_L();
    ///command
    SPI_SCL_L();
    SPI_SDA_OUT();
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

static void spi_write_dat(u8 dat)
{
    u8 i;
    SPI_CSX_L();
    ///data
    SPI_SCL_L();
    SPI_SDA_OUT();
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

static void set_display_area(unsigned int Xstart, unsigned int Xend, unsigned int Ystart, unsigned int Yend)
{
    spi_write_cmd(0x2a);
    spi_write_dat(Xstart >> 8);
    spi_write_dat(Xstart & 0xff);
    spi_write_dat(Xend >> 8);
    spi_write_dat(Xend & 0xff);

    spi_write_cmd(0x2b);
    spi_write_dat(Ystart >> 8);
    spi_write_dat(Ystart & 0xff);
    spi_write_dat(Yend >> 8);
    spi_write_dat(Yend & 0xff);

    spi_write_cmd(0x2c);
}

static void lcd_ili9806e_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    printf("ili9806e_init...\r\n");
    spi_io_init();

    printf("reset\r\n");
    //RESET
    SPI_RST_H();
    delay_2ms(5);
    SPI_RST_L();
    delay_2ms(5);
    SPI_RST_H();
    delay_2ms(5);

    spi_write_cmd(0x01);
    delay_2ms(5);

    spi_write_cmd(0xFF);
    spi_write_dat(0xFF);
    spi_write_dat(0x98);
    spi_write_dat(0x06);

    spi_write_cmd(0xBA);
    spi_write_dat(0xe0);

    spi_write_cmd(0x3A);
    spi_write_dat(0x57);

    spi_write_cmd(0xEC);
    spi_write_dat(0x24);
    spi_write_dat(0x4F);
    spi_write_dat(0x00);

    spi_write_cmd(0xBA);
    spi_write_dat(0x60);

    spi_write_cmd(0xBC);
    spi_write_dat(0x01);
    spi_write_dat(0x12);
    spi_write_dat(0x61);
    spi_write_dat(0xFF);
    spi_write_dat(0x10);
    spi_write_dat(0x10);
    spi_write_dat(0x0B);
    spi_write_dat(0x13);
    spi_write_dat(0x32);
    spi_write_dat(0x73);
    spi_write_dat(0xFF);
    spi_write_dat(0xFF);
    spi_write_dat(0x0E);
    spi_write_dat(0x0E);
    spi_write_dat(0x00);
    spi_write_dat(0x03);
    spi_write_dat(0x66);
    spi_write_dat(0x63);
    spi_write_dat(0x01);
    spi_write_dat(0x00);
    spi_write_dat(0x00);

    spi_write_cmd(0xBD);
    spi_write_dat(0x01);
    spi_write_dat(0x23);
    spi_write_dat(0x45);
    spi_write_dat(0x67);
    spi_write_dat(0x01);
    spi_write_dat(0x23);
    spi_write_dat(0x45);
    spi_write_dat(0x67);

    spi_write_cmd(0xBE);
    spi_write_dat(0x00);
    spi_write_dat(0x21);
    spi_write_dat(0xAB);
    spi_write_dat(0x60);
    spi_write_dat(0x22);
    spi_write_dat(0x22);
    spi_write_dat(0x22);
    spi_write_dat(0x22);
    spi_write_dat(0x22);

    spi_write_cmd(0xC7);
    spi_write_dat(0x6F);
    spi_write_dat(0x80);

    spi_write_cmd(0xC7);
    spi_write_dat(0x78);
    spi_write_dat(0x80);

    spi_write_cmd(0xED);
    spi_write_dat(0x7F);
    spi_write_dat(0x0F);
    spi_write_dat(0x00);

    spi_write_cmd(0xB6);
    spi_write_dat(0xa3);//RGB/SYNC mode

    spi_write_cmd(0xB0);
    spi_write_dat(0xc0);//


    u16 HBP = 40;

    //800+26+26
    u8 VFP = 26;
    u8 VBP = 20;//24;
    spi_write_cmd(0xB5);
    spi_write_dat(VFP);
    spi_write_dat(VBP);
    spi_write_dat(HBP);
    spi_write_dat(HBP >> 8);

//	spi_write_cmd(0xC0);
//    spi_write_dat(0xAB);
//    spi_write_dat(0x0B);
//    spi_write_dat(0x0A);

    spi_write_cmd(0xC0);
    spi_write_dat(0xAB);
    spi_write_dat(0x0B);
    spi_write_dat(0x0A);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x88);
    spi_write_dat(0x2C);
    spi_write_dat(0x50);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0xFF);

    spi_write_cmd(0xFC);
    spi_write_dat(0x09);

    spi_write_cmd(0xDF);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x20);

    spi_write_cmd(0xF3);
    spi_write_dat(0x74);

    //480x800
    spi_write_cmd(0xB1);
    spi_write_dat(0x00);
    spi_write_dat(0x12);
    spi_write_dat(0x14);

    spi_write_cmd(0xF2);
    spi_write_dat(0x00);
    spi_write_dat(0x59);
    spi_write_dat(0x40);
    spi_write_dat(0x28);

    spi_write_cmd(0xC1);
    spi_write_dat(0x07);
    spi_write_dat(0x80);
    spi_write_dat(0x80);
    spi_write_dat(0x20);

    spi_write_cmd(0xE0);
    spi_write_dat(0x04);
    spi_write_dat(0x08);
    spi_write_dat(0x0D);
    spi_write_dat(0x0C);
    spi_write_dat(0x05);
    spi_write_dat(0x09);
    spi_write_dat(0x07);
    spi_write_dat(0x04);
    spi_write_dat(0x08);
    spi_write_dat(0x0C);
    spi_write_dat(0x11);
    spi_write_dat(0x04);
    spi_write_dat(0x07);
    spi_write_dat(0x1C);
    spi_write_dat(0x15);
    spi_write_dat(0x00);

    spi_write_cmd(0xE1);
    spi_write_dat(0x00);
    spi_write_dat(0x06);
    spi_write_dat(0x0B);
    spi_write_dat(0x0A);
    spi_write_dat(0x04);
    spi_write_dat(0x07);
    spi_write_dat(0x06);
    spi_write_dat(0x04);
    spi_write_dat(0x08);
    spi_write_dat(0x0C);
    spi_write_dat(0x11);
    spi_write_dat(0x04);
    spi_write_dat(0x06);
    spi_write_dat(0x1A);
    spi_write_dat(0x13);
    spi_write_dat(0x00);

    spi_write_cmd(0xF7);
    spi_write_dat(0x82);//480*800 resolution

    spi_write_cmd(0x44);
    spi_write_dat(0x03);
    spi_write_dat(0x56);

    spi_write_cmd(0x35);
    spi_write_dat(0x00);

    spi_write_cmd(0x36);
    spi_write_dat(0x00);

//    spi_write_cmd(0xb9);
//    spi_write_dat(0x00);
//    spi_write_dat(0x00);

    spi_write_cmd(0x51);
    spi_write_dat(0x83);

    set_display_area(0, 480 - 1, 0,/*784*/800 - 1);

    spi_write_cmd(0x13);
    spi_write_cmd(0x11);//Sleep Out
    spi_write_cmd(0x29);//Display ON

    printf("initial ok.\r\n");

}


static void lcd_ili9806e_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}


REGISTER_LCD_DEVICE(lcd_ili9806e_dev) = {
    .info = {
        .test_mode 	     = false,
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
        .sample          = SAMP_YUV422,
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
        },
    },

    .drive_mode      = MODE_RGB_DE_SYNC,
    .ncycle          = CYCLE_ONE,
    .interlaced_mode = INTERLACED_NONE,
    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN  | CLK_EN/*|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
    .data_width      = PORT_24BITS,
    .port_sel        = PORT_GROUP_B,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_2,

    .timing = {
        .hori_interval 		=    646,
        .hori_sync_clk 		=    2,
        .hori_start_clk 	=    40,
        .hori_pixel_width 	=    480,
        .vert_interval 		=    856,
        .vert_sync_clk 		=    4,//14,
        .vert_start_clk 	=    24,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    800,
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_ILI9806),
    .logo 	 = "ili9806e",
    .type 	 = LCD_DVP_RGB,
    .init 	 = lcd_ili9806e_init,
    .dev  	 = &lcd_ili9806e_dev,
    .bl_ctrl = lcd_ili9806e_backctrl,
};
#endif

