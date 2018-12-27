#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/pap.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_LYWS030GQ

//CS  PC5
#define SPI_CS_OUT()    PORTB_DIR &= ~BIT(5)
#define SPI_CS_EN()     PORTB_OUT &= ~BIT(5)
#define SPI_CS_DIS()    PORTB_OUT |= BIT(5)
//SDA PC6
#define SPI_SDA_OUT()   PORTB_DIR &= ~BIT(6);PORTB_PU &=~ BIT(6)
#define SPI_SDA_IN()    PORTB_DIR |=  BIT(6);PORTB_PU |= BIT(6);PORTB_PD &=~ BIT(6)
#define SPI_SDA_H()     PORTB_OUT |=  BIT(6)
#define SPI_SDA_L()     PORTB_OUT &= ~BIT(6)
#define SPI_SDA_R()     PORTB_IN&BIT(6)
//SCL PC7
#define SPI_CLK_OUT()   PORTB_DIR &= ~BIT(7)
#define SPI_CLK_H()     PORTB_OUT |=  BIT(7)
#define SPI_CLK_L()     PORTB_OUT &= ~BIT(7)

#define DELAY_CNT 500

static void spi_init()
{
    SPI_CS_OUT();
    SPI_SDA_OUT();
    SPI_CLK_OUT();

    SPI_CS_EN();
    SPI_CLK_H();
    SPI_SDA_H();
}

static void spi_tx_byte(u8 dat)
{
    u8 i;

    SPI_SDA_OUT();

    for (i = 0; i < 8; i++) {

        if (dat & BIT(7)) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }
        dat <<= 1;
        delay(DELAY_CNT);
        SPI_CLK_L();

        delay(DELAY_CNT * 2);

        SPI_CLK_H();
        delay(DELAY_CNT);
    }

}

static u8 spi_rx_byte()
{
    u8 dat = 0;
    u8 i;

    SPI_SDA_IN();
    delay(100);

    for (i = 0; i < 8; i++) {
        SPI_CLK_L();
        delay(DELAY_CNT);

        dat <<= 1;
        if (SPI_SDA_R()) {
            dat |= BIT(0);
        }

        SPI_CLK_H();
        delay(DELAY_CNT);
    }


    return dat;
}


static void wr_reg(u8 addr, u8 val)
{
    SPI_CS_EN();
    spi_tx_byte(addr << 1);
    spi_tx_byte(val);
    SPI_CS_DIS();
}

static u8 rd_reg(u8 addr)
{
    u8 ret;
    SPI_CS_EN();
    spi_tx_byte((addr << 1) | BIT(0));
    ret = spi_rx_byte();
    SPI_CS_DIS();

    return ret;
}

static void lyws030gq_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u16 clk = 100;

    spi_init();

    /*
    while(clk--)
    {
        SPI_CLK_L();
        delay(DELAY_CNT);
        SPI_CLK_H();
        delay(DELAY_CNT);
    }
    */
    delay_2ms(500);
    wr_reg(0x00, 0x00); //Soft Reset    Display control (1)
    delay_2ms(1);
    wr_reg(0x00, 0x02); // Soft Reset Over


#if 1
    wr_reg(0x01, 0x20); //Display control (2)
    wr_reg(0x02, 0x90); //Display control (3) HBP
    wr_reg(0x03, 0x10); //Display control (4) VBP
    wr_reg(0x05,/*0x80*/0x98); //Display control (5)
    wr_reg(0x06, 0x00); //Display control (6)
    wr_reg(0x07, 0x03); //Input Mounting mode set
#else
    wr_reg(0x01, 0x20);
    wr_reg(0x02, 0x18);
    wr_reg(0x03, 0x18);
    wr_reg(0x05, 0x80);
    wr_reg(0x06, 0x00);
    wr_reg(0x07, 0x00);
#endif

    wr_reg(0x08, 0x40);
    wr_reg(0x09, 0x40);

    wr_reg(0x0A, 0x1E); //Degital γ ajustment R1-B5
    wr_reg(0x0B, 0x3C);
    wr_reg(0x0C, 0x78);
    wr_reg(0x0D, 0xB0);
    wr_reg(0x0E, 0xE0);

    wr_reg(0x0F, 0x1E);
    wr_reg(0x10, 0x3C);
    wr_reg(0x11, 0x78);
    wr_reg(0x12, 0xB0);
    wr_reg(0x13, 0xE0);

    wr_reg(0x14, 0x24);
    wr_reg(0x15, 0x48);
    wr_reg(0x16, 0x8C);
    wr_reg(0x17, 0xCA);
    wr_reg(0x18, 0xF2); //Degital γ ajustment B5

    wr_reg(0x19, 0x80);
    wr_reg(0x1A, 0x80);
    wr_reg(0x1B, 0x80);
    wr_reg(0x1C, 0x00);


    wr_reg(0x20, 0x03); //Voltage control 1
    wr_reg(0x21, 0x12);
    wr_reg(0x22, 0x26);
    wr_reg(0x23, 0x40);
    wr_reg(0x24, 0x25);
    wr_reg(0x25, 0x15);
    wr_reg(0x26, 0x00);
    wr_reg(0x30, 0x94); //Voltage control 8

    wr_reg(0x31, 0x01); //Source timing
    wr_reg(0x32, 0x01); //LCD signal timing 1
    wr_reg(0x33, 0x00); //LCD signal timing 2
    wr_reg(0x34, 0x09); //LCD signal timing 3

    wr_reg(0x35, 0x00); //VCOMH control set
    wr_reg(0x36, 0x1B); //Bias current
    wr_reg(0x37, 0x1C); //Gate off timing

    wr_reg(0x40, 0x08); //γ control 1
    wr_reg(0x41, 0x08);
    wr_reg(0x42, 0x64);
    wr_reg(0x43, 0x0D);
    wr_reg(0x44, 0x08);
    wr_reg(0x45, 0xC7);
    wr_reg(0x46, 0xA2);
    wr_reg(0x47, 0xC3);
    wr_reg(0x48, 0xD5);
    wr_reg(0x49, 0x83);
    wr_reg(0x4A, 0x17);
    wr_reg(0x4B, 0x12);
    wr_reg(0x4C, 0xB9);
    wr_reg(0x4D, 0x3D);
    wr_reg(0x4E, 0x3D); //γ control 15

    wr_reg(0x55, 0x00);
    wr_reg(0x57, 0x00);
    wr_reg(0x58, 0x00);
    wr_reg(0x59, 0x00);
    wr_reg(0x5A, 0x00);
    wr_reg(0x5B, 0x60);

    wr_reg(0x77, 0x80); //Voltage control 9*/
    /*
    	PORTD_DIR &=~ BIT(5);
    	u8 pulse = 600;
    	while(pulse--)
    	{
            PORTD_OUT |= BIT(5);
            delay(DELAY_CNT);
            PORTD_OUT &=~ BIT(5);
            delay(DELAY_CNT);
        }
    */
    wr_reg(0x00, 0x03); //Display ON

    delay_2ms(350);
    //wr_reg(0x04,0x17);//0x14


    int i;
    for (i = 0; i < 0x7f; i++) {
        printf("addr:0x%02x  val:0x%x\n", i, rd_reg(i));
    }
}

static void lcd_lyws030gq_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE(lcd_lyws030gq_dev) = {
    .info = {
        .test_mode 	     = false,
        .color           = 0x0000FF,
        .rotate_en       = false,	    // 旋转使能
        .hori_mirror_en  = false,	    // 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能


        .len 			 = LEN_256,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,
        .itp_mode_en     = FALSE,
        .sample          = LCD_DEV_SAMPLE,
        .format          = FORMAT_YUV422,

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

    /* .image_width     = 720, */
    .drive_mode      = MODE_RGB_DE_SYNC,
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_8BITS,
    .interlaced_mode = INTERLACED_NONE,
    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN  /*|CLK_EN|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
    .port_sel        = PORT_GROUP_B,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_1,
    .timing = {
        .hori_interval 		= 1728,
        .hori_sync_clk 		= 1,
        .hori_start_clk 	= 144,
        .hori_pixel_width 	= 720,

        .vert_interval 		= 525,
        .vert_sync_clk 		= 1,
        .vert_start_clk 	= 16,
        .vert_start1_clk 	= 16,
        .vert_pixel_width 	= 480,
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type = LCD_DVP_RGB,
    .init = lyws030gq_init,
    .dev  = &lcd_lyws030gq_dev,
    .bl_ctrl = lcd_lyws030gq_backctrl,
};
#endif
