#include "generic/typedef.h"
#include "asm/imb_driver.h"
#include "asm/imd.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_HX8369A_SPI_RGB
//R          G         B
#define DPI_24BITS  0//DB[23..16] DB[15..8] DB[7..0]
#define DPI_18BITS  1//DB[21..16] DB[13..8] DB[5..0]
#define DPI_16BITS  2//DB[21..17] DB[13..8] DB[5..1]  ok
#define DPI_FORMAT DPI_24BITS

#define SPI_3WIRE   0//9bit  BS[3..0]:1101
#define SPI_4WIRE   1//8bit  BS[3..0]:1111
#define SPI_WIRE SPI_3WIRE


//PC5->PV5 RESET
#define SPI_RST_OUT()    PORTB_DIR &=~ BIT(5)
#define SPI_RST_H()      PORTB_OUT |=  BIT(5)
#define SPI_RST_L()      PORTB_OUT &=~ BIT(5)

#if (SPI_WIRE == SPI_4WIRE)
//PH5  DCX
#define SPI_DCX_OUT()    PORTD_DIR &=~ BIT(13)
#define SPI_DCX_H()      PORTD_OUT |=  BIT(13)
#define SPI_DCX_L()      PORTD_OUT &=~ BIT(13)
#elif(SPI_WIRE == SPI_3WIRE)
//PH5  DCX
#define SPI_DCX_OUT()    //PORTD_DIR &=~ BIT(13)
#define SPI_DCX_H()      //PORTD_OUT |=  BIT(13)
#define SPI_DCX_L()      //PORTD_OUT &=~ BIT(13)
#endif

//PH4->PH2 CS
#define SPI_CSX_OUT()    PORTD_DIR &=~ BIT(12)
#define SPI_CSX_H()      PORTD_OUT |=  BIT(12)
#define SPI_CSX_L()      PORTD_OUT &=~ BIT(12)

//PC6->PV6 SCL
#define SPI_SCL_OUT()    PORTB_DIR &=~ BIT(6)
#define SPI_SCL_H()      PORTB_OUT |=  BIT(6)
#define SPI_SCL_L()      PORTB_OUT &=~ BIT(6)

//PC7->PV7 SDA
#define SPI_MOSI_OUT()   PORTB_DIR &=~ BIT(7)
#define SPI_MOSI_H()     PORTB_OUT |=  BIT(7)
#define SPI_MOSI_L()     PORTB_OUT &=~ BIT(7)

#define SPI_MISO_IN()    //PORTB_DIR |= BIT(5);PORTB_PU |= BIT(5)
#define SPI_MISO_R()     0//PORTB_IN & BIT(5)

#define DELAY_CNT       100

extern void delay(volatile u32  t);

typedef struct {
    u8 cmd;
    u8 cnt;
    u8 dat[128];
} InitCode;

#define REGFLAG_DELAY 0xFF

static InitCode code1[] = {
    //{0x01, 0},//soft reset
    //{REGFLAG_DELAY, 50},
    {0xB9, 3,  {0xFF, 0x83, 0x69}}, // SET password
    {0xB1, 19, {0x85, 0x00, 0x34, 0x07, 0x00, 0x11, 0x11, 0x2A, 0x32, 0x19, 0x19, 0x07, 0x23, 0x01, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6}}, //Set Power
    {0xB2, 15, {0x00, 0x2b, 0x05, 0x05, 0x70, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x05, 0x05, 0x00, 0x01,}}, // SET Display  480x800
    {0xB4, 5,  {0x00, 0x0f, 0x82, 0x0c, 0x03}},
    {0xB6, 2,  {0x10, 0x10}}, // SET VCOM
    {0xD5, 26, {0x00, 0x04, 0x03, 0x00, 0x01, 0x04, 0x1A, 0xff, 0x01, 0x13, 0x00, 0x00, 0x40, 0x06, 0x51, 0x07, 0x00, 0x00, 0x41, 0x06, 0x50, 0x07, 0x07, 0x0F, 0x04, 0x00,}},
    {0xE0, 34, {0x00, 0x13, 0x19, 0x38, 0x3D, 0x3F, 0x28, 0x46, 0x07, 0x0D, 0x0E, 0x12, 0x15, 0x12, 0x14, 0x0F, 0x17, 0x00, 0x13, 0x19, 0x38, 0x3D, 0x3F, 0x28, 0x46, 0x07, 0x0D, 0x0E, 0x12, 0x15, 0x12, 0x14, 0x0F, 0x17,}},
    {
        0xC1, 127, {
            0x01,
            0x04, 0x13, 0x1a, 0x20, 0x27, 0x2c, 0x32, 0x36, 0x3f, 0x47, 0x50, 0x59, 0x60, 0x68, 0x71, 0x7B, 0x82, 0x89, 0x91, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC1, 0xC9, 0xD0, 0xD7, 0xE0, 0xE7, 0xEF, 0xF7, 0xFE, 0xCF, 0x52, 0x34, 0xF8, 0x51, 0xF5, 0x9D, 0x75, 0x00,
            0x04, 0x13, 0x1a, 0x20, 0x27, 0x2c, 0x32, 0x36, 0x3f, 0x47, 0x50, 0x59, 0x60, 0x68, 0x71, 0x7B, 0x82, 0x89, 0x91, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC1, 0xC9, 0xD0, 0xD7, 0xE0, 0xE7, 0xEF, 0xF7, 0xFE, 0xCF, 0x52, 0x34, 0xF8, 0x51, 0xF5, 0x9D, 0x75, 0x00,
            0x04, 0x13, 0x1a, 0x20, 0x27, 0x2c, 0x32, 0x36, 0x3f, 0x47, 0x50, 0x59, 0x60, 0x68, 0x71, 0x7B, 0x82, 0x89, 0x91, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC1, 0xC9, 0xD0, 0xD7, 0xE0, 0xE7, 0xEF, 0xF7, 0xFE, 0xCF, 0x52, 0x34, 0xF8, 0x51, 0xF5, 0x9D, 0x75, 0x00,
        }
    },
    //{0x36, 1,  {0x10}},
#if (DPI_FORMAT==DPI_16BITS)
    //R:DB[21..17] G:DB[13..8] B:DB[5..1]
    {0x3a, 1,  {0x55}},
#elif (DPI_FORMAT==DPI_18BITS)
    //R:DB[21..16] G:DB[13..8] B:DB[5..0]
    {0x3a, 1,  {0x65}},
#elif (DPI_FORMAT==DPI_24BITS)
    //R:DB[23..16] G:DB[15..8] B:DB[7..0]
    {0x3a, 1,  {0x75}},
#endif
    {0x11, 0},//Sleep out
    {REGFLAG_DELAY, 150},
    {0x29, 0},
    {REGFLAG_DELAY, 100},
};

static void spi_io_init()
{
    printf("reset \r\n");

    SPI_RST_OUT();
    SPI_DCX_OUT();
    SPI_CSX_OUT();
    SPI_MOSI_OUT();
    SPI_MISO_IN();

    SPI_CSX_H();
    SPI_DCX_H();
    SPI_RST_H();
    SPI_MOSI_L();
    SPI_SCL_H();


    SPI_RST_H();
    delay_2ms(500);
    SPI_RST_L();
    delay_2ms(500);
    SPI_RST_H();
}

static void spi_tx_byte(u16 dat)
{
    u8 i;
#if (SPI_WIRE == SPI_3WIRE)
    for (i = 0; i < 9; i++) {
        SPI_SCL_L();
        if (dat & BIT(8)) {
            SPI_MOSI_H();
        } else {
            SPI_MOSI_L();
        }
        dat <<= 1;
        delay(DELAY_CNT);

        SPI_SCL_H();
        delay(DELAY_CNT);
    }
#elif (SPI_WIRE == SPI_4WIRE)

    for (i = 0; i < 8; i++) {
        SPI_SCL_L();
        if (dat & BIT(7)) {
            SPI_MOSI_H();
        } else {
            SPI_MOSI_L();
        }
        dat <<= 1;
        delay(DELAY_CNT);

        SPI_SCL_H();
        delay(DELAY_CNT);
    }

#endif
}

static u8 spi_rx_byte()
{
    u8 dat = 0;
    u8 i;

    SPI_MISO_IN() ;
    for (i = 0; i < 8; i++) {
        SPI_SCL_L();
        delay(DELAY_CNT);

        dat <<= 1;
        if (SPI_MISO_R()) {
            dat |= BIT(0);
        }

        SPI_SCL_H();
        delay(DELAY_CNT);

    }

    return dat;
}


static void WriteComm(u8 cmd)
{
    SPI_CSX_L();
    SPI_DCX_L();

    spi_tx_byte(cmd);

    SPI_CSX_H();
}

static void WriteData(u8 dat)
{
    SPI_CSX_L();
    SPI_DCX_H();
#if (SPI_WIRE == SPI_3WIRE)
    spi_tx_byte(BIT(8) | dat);
#elif (SPI_WIRE == SPI_4WIRE)
    spi_tx_byte(dat);
#endif
    SPI_CSX_H();
}

static u8 ReadData()
{
    u8 ret;
    SPI_CSX_L();
    SPI_DCX_H();
    ret = spi_rx_byte();
    SPI_CSX_H();

    return ret;
}


static void hx8369a_init_code(InitCode *code, u8 cnt)
{
    u8 i, j;

    for (i = 0; i < cnt; i++) {
        if (code[i].cmd == REGFLAG_DELAY) {
            delay_2ms(code[i].cnt);
        } else {
            WriteComm(code[i].cmd);
            printf("cmd:%x ", code[i].cmd);
            //printf("cnt:%x ",code[i].cnt);
            //printf("dat:%x ",code[i].dat);
            for (j = 0; j < code[i].cnt; j++) {
                WriteData(code[i].dat[j]);
                printf("%02x ", code[i].dat[j]);
            }
            printf("\n");
        }
    }
}



static void setxy(u16 Xstart, u16 Xend, u16 Ystart, u16 Yend)
{
    //HX8352-C
    printf("setxy %d %d %d %d\n", Xstart, Xend, Ystart, Yend);

    WriteComm(0x2a);
    WriteData(Xstart >> 8);
    WriteData(Xstart & 0xff);
    WriteData(Xend >> 8);
    WriteData(Xend & 0xff);

    WriteComm(0x2b);
    WriteData(Ystart >> 8);
    WriteData(Ystart & 0xff);
    WriteData(Yend >> 8);
    WriteData(Yend & 0xff);

    WriteComm(0x2c);

    SPI_CSX_L();
    SPI_DCX_H();
}

static void lcd_hx8369a_init(void *_data)
{
    u8 i;
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    printf("lcd hx8369 init...\r\n");

    spi_io_init();

    WriteComm(0x01);
    delay_2ms(10);

    WriteComm(0x11);
    delay_2ms(120);


    WriteComm(0x2D);
    for (i = 0; i <= 63; i++) {
        WriteData(i * 4);
    }

    for (i = 0; i <= 63; i++) {
        WriteData(i * 4);
    }

    for (i = 0; i <= 63; i++) {
        WriteData(i * 4);
    }

    hx8369a_init_code(code1, sizeof(code1) / sizeof(code1[0]));

    setxy(0, 479, 0, 799);
}

static void lcd_hx8369a_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE(lcd_hx8369a_spi_rgb_dev) = {
    .info = {
        .test_mode 	     = false,
        .color           = 0x0000FF,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,
        .itp_mode_en     = false,
        .sample          = LCD_DEV_SAMPLE,

        .len 			 = LEN_256,
        .rotate_en       = false,	    // 旋转使能
        .hori_mirror_en  = false,	    // 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能
#if (DPI_FORMAT==DPI_16BITS)
        .format          = FORMAT_RGB565,
#elif (DPI_FORMAT==DPI_18BITS)
        .format          = FORMAT_RGB666,
#elif (DPI_FORMAT==DPI_24BITS)
        .format          = FORMAT_RGB888,
#endif
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
#if (DPI_FORMAT==DPI_16BITS)
    .data_width      = PORT_16BITS,
#elif (DPI_FORMAT==DPI_18BITS)
    .data_width      = PORT_18BITS,
#elif (DPI_FORMAT==DPI_24BITS)
    .data_width      = PORT_24BITS,
#endif
    .interlaced_mode = INTERLACED_NONE,

    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN  | CLK_EN/*|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,

    .port_sel        = PORT_GROUP_B,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_2,

    .timing = {
        /* .hori_interval 		=    525,//12000000/60/288,// */
        /* .hori_sync_clk 		=    5, */
        /* .hori_start_clk 	=    40, */
        /* .hori_pixel_width 	=    480, */
        /* .vert_interval 		=    810, */
        /* .vert_sync_clk 		=    5, */
        /* .vert_start_clk 	=    5, */
        /* .vert_start1_clk 	=    0, */
        /* .vert_pixel_width 	=    800, */

        .hori_interval 		=    595,//12000000/60/288,//
        .hori_sync_clk 		=    5,
        .hori_start_clk 	=    40,
        .hori_pixel_width 	=    480,
        .vert_interval 		=    880,
        .vert_sync_clk 		=    5,
        .vert_start_clk 	=    5,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    800,
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_DVP_RGB,
    .init 	 = lcd_hx8369a_init,
    .dev     = &lcd_hx8369a_spi_rgb_dev,
    .bl_ctrl = lcd_hx8369a_backctrl,
};

#endif
