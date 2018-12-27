#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/pap.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_LQ026B3UX01
//PC5->PV5
#define TFT_CS_OUT()    PORTB_DIR &=~BIT(5)
#define TFT_CS_EN()     PORTB_OUT &=~BIT(5)
#define TFT_CS_DIS()    PORTB_OUT |= BIT(5)
//PC6->PV6
#define TFT_RST_OUT()   PORTB_DIR &=~BIT(6)
#define TFT_RST_H()     PORTB_OUT |= BIT(6)
#define TFT_RST_L()     PORTB_OUT &=~BIT(6)
//PC7->PV7
#define TFT_RS_OUT()    PORTB_DIR &=~BIT(7)
#define TFT_RS_H()      PORTB_OUT |= BIT(7)
#define TFT_RS_L()      PORTB_OUT &=~BIT(7)

//常用颜色表
#define COLOUR_RED       0xF800
#define COLOUR_BLUE      0x001F
#define COLOUR_GREEN     0x07E0
#define COLOUR_WHITE     0xFFFF
#define COLOUR_BLACK     0x0000

/*
8位接口
16bit PAP8BIT  RGB565 TWO_CYCLE   PORT_8BITS 8bitsx2
18bit PAP8BIT  RGB666 THREE_CYCLE PORT_6BITS 6bitsx3 DB[7:2]

9位接口
16bit RGB565 TWO_CYCLE 8bitsx2
18bit RGB666 TWO_CYCLE 9bitsx2

16位接口
16bit PAP16BIT 小端  RGB565 ONE_CYCLE 16bitsx1
16bit PAP16BIT 大端  RGB565 TWO_CYCLE 8bitsx2
18bit PAP16BIT 大端  RGB666 THREE_CYCLE 6bitsx3 DB[7:2]

18位接口
18bit PAP16BIT 小端  RGB666 ONE_CYCLE 18bitsx1
16bit PAP16BIT 大端  RGB565 TWO_CYCLE PORT_8BITS 8bitsx2
16bit PAP16BIT 小端  RGB565 ONE_CYCLE 16bitsx1


IM0 IM1
 L   L		18bit
 L   H		16bit
 H   L		9bit
 H   H		8bit


reg 00H = 0x0a
reg 01H = 0x04
reg 10H = 0xf0
reg 11H = 0x90


RGB565:       R7|R6|R5|R4|R3|G7|G6|G5|G4|G3|G2|B7|B6|B5|B4|B3
RGB666: R7|R6|R5|R4|R3|R2|G7|G6|G5|G4|G3|G2|B7|B6|B5|B4|B3|B2
*/

#define PORT08          (0<<0)
#define PORT09          (1<<0)
#define PORT16          (2<<0)
#define PORT18          (3<<0)
#define PORT_MASK        0x03

#define PIXEL16         (0<<2)
#define PIXEL18         (1<<2)
#define PIXEL_MASK       0x04

#define RGB565          (0<<3)
#define RGB666          (1<<3)
#define RGB_MASK         0x08

#define CYCLE1          (0<<4)
#define CYCLE2          (1<<4)
#define CYCLE3          (2<<4)
#define CYCLE_MASK       0x30

#define PORT08_PIXEL16_RGB565_CYCLE2    (PORT08|PIXEL16|RGB565|CYCLE2)//正常
#define PORT08_PIXEL18_RGB666_CYCLE3    (PORT08|PIXEL18|RGB666|CYCLE3)//正常

#define PORT09_PIXEL16_RGB565_CYCLE2    (PORT09|PIXEL16|RGB565|CYCLE2)
#define PORT09_PIXEL18_RGB666_CYCLE2    (PORT09|PIXEL18|RGB666|CYCLE2)

#define PORT16_PIXEL16_RGB565_CYCLE1    (PORT16|PIXEL16|RGB565|CYCLE1)//LE OK
#define PORT16_PIXEL16_RGB565_CYCLE2    (PORT16|PIXEL16|RGB565|CYCLE2)
#define PORT16_PIXEL18_RGB666_CYCLE3    (PORT16|PIXEL18|RGB666|CYCLE3)

#define PORT18_PIXEL18_RGB666_CYCLE1    (PORT18|PIXEL18|RGB666|CYCLE1)//le ok
#define PORT18_PIXEL16_RGB565_CYCLE2    (PORT18|PIXEL16|RGB565|CYCLE2)
#define PORT18_PIXEL16_RGB565_CYCLE1    (PORT18|PIXEL16|RGB565|CYCLE1)

#define DISP_MODE  PORT18_PIXEL18_RGB666_CYCLE1

#define DIRECTION_HORI  0//400x240
#define DIRECTION_VERT  1//240x400
#define DIRECTION       DIRECTION_VERT

#define BE              0
#define LE              1
#define ENDIAN          LE

static u8 *dest_buf_addr;
#define ADDR_ALIGN  0x20
static void *pap = NULL;

static void write_cmd(u8 dat, u8 lsb)
{
    TFT_CS_EN();
    TFT_RS_L();
#if ((DISP_MODE & PORT_MASK) == PORT09)
    PORTC_DIR &= ~ BIT(7);
    delay(2);
    if (lsb == 0) {
        PORTC_OUT &= ~ BIT(7);
    } else {
        PORTC_OUT |= BIT(7);
    }
    dev_write(pap, &dat, sizeof(dat));
#else
    dat <<= 1;
    dat += lsb;
    dev_write(pap, &dat, sizeof(dat));
#endif
    TFT_CS_DIS();
}

static void write_dat(u8 dat, u8 lsb)
{
    TFT_CS_EN();
    TFT_RS_H();
#if ((DISP_MODE & PORT_MASK) == PORT09)
    PORTC_DIR &= ~ BIT(7);
    delay(2);
    if (lsb == 0) {
        PORTC_OUT &= ~ BIT(7);
    } else {
        PORTC_OUT |= BIT(7);
    }
    dev_write(pap, &dat, sizeof(dat));
#else
    dat <<= 1;
    dat += lsb;
    dev_write(pap, &dat, sizeof(dat));
#endif
    TFT_CS_DIS();
}

static u8 read_dat()
{
    u8 val;
    TFT_CS_EN();
    TFT_RS_H();
#if ((DISP_MODE & PORT_MASK) == PORT09)
    PORTC_DIR |= BIT(7);
    delay(2);
    dev_read(pap, &val, sizeof(val));
    val <<= 1;
    if (PORTC_IN & BIT(7)) {
        val |= BIT(0);
    }
#else
    dev_read(pap, &val, sizeof(val));
#endif
    TFT_CS_DIS();

    return val;
}

static void wr_reg(u8 index, u8 dat)
{
//    TFT_CS_EN();
//    TFT_RS_L();
//    dev_write(pap,&index,sizeof(index));
//    TFT_RS_H();
//    dev_write(pap,&dat,sizeof(dat));
//    TFT_CS_DIS();
    write_cmd(index >> 1, index & BIT(0));
    write_dat(dat >> 1, dat & BIT(0));
}

static u16 rd_reg(u8 index)
{
    u16 val;
//    TFT_CS_EN();
//    TFT_RS_L();
//    dev_write(pap,&index,sizeof(index));
//    TFT_RS_H();
//    dev_read(pap,&val,sizeof(val));
//    TFT_CS_DIS();

    write_cmd(index >> 1, index & BIT(0));
    val = read_dat();
    printf("\nreg %02xH = 0x%02x", index, val);
    return val;
}

static void tft_setrange(u16 x0, u16 x1, u16 y0, u16 y1)
{
    wr_reg(0x13, x0);
    wr_reg(0x16, x1);

    wr_reg(0x14, y0);
    wr_reg(0x15, y0 >> 8);

    wr_reg(0x17, y1);
    wr_reg(0x18, y1 >> 8);

    wr_reg(0x19, 0x01);
}

static void tft_set_pos(u16 x0, u16 x1, u16 y0, u16 y1)
{
    tft_setrange(x0, x1, y0, y1);
    write_cmd(0x03 >> 1, 0x03 & BIT(0));
    TFT_CS_EN();
    TFT_RS_H();
}

#if (DISP_MODE == PORT08_PIXEL16_RGB565_CYCLE2)||(DISP_MODE == PORT09_PIXEL16_RGB565_CYCLE2)||(DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE2)||(DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE2)
#elif (DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE1)||(DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE1)
#elif (DISP_MODE == PORT08_PIXEL18_RGB666_CYCLE3)||(DISP_MODE == PORT16_PIXEL18_RGB666_CYCLE3)
#elif (DISP_MODE == PORT09_PIXEL18_RGB666_CYCLE2)
#elif (DISP_MODE == PORT18_PIXEL18_RGB666_CYCLE1)
#endif

u8 buf[240 * 3];
void flush_dcache(register void *ptr, register s32 len);
static void tft_ClearScreen(u32 color)
{
    u8 *pbuf;
    u32 i, j;
    u32 rgb16;
    u32 rgb18;
    u16 bytes_per_line;
    u8 step;

    rgb16 = (((color >> 19) << 11) & 0xf800) | (((color >> 10) << 5) & 0x07e0) | ((color >> 3) & 0x001f);
    rgb18 = (((color >> 19) << 12) & 0x3f000) | (((color >> 10) << 6) & 0x0fc0) | ((color >> 3) & 0x003f);

    pbuf = buf;

    tft_set_pos(0, 239, 0, 399);
    //buf = malloc(240*400*2);
#if 1
#if ((DISP_MODE&RGB_MASK) == RGB565)
    bytes_per_line = 240 * 2;
    step = 2;
#elif ((DISP_MODE&RGB_MASK) == RGB666)
    bytes_per_line = 240 * 3;
    step = 3;
#endif

    for (i = 0; i < bytes_per_line; i += step) {
#if ((DISP_MODE&RGB_MASK) == RGB565)
        pbuf[i  ] = rgb16 >> 8;
        pbuf[i + 1] = rgb16;
#elif ((DISP_MODE&RGB_MASK) == RGB666)
        pbuf[i  ] = ((rgb18 >> 12) & 0x3f) << 2;
        pbuf[i + 1] = ((rgb18 >> 6) & 0x3f) << 2;
        pbuf[i + 2] = ((rgb18) & 0x3f) << 2;
#endif
    }
    flush_dcache(pbuf, bytes_per_line);
    //printf_buf(pbuf,bytes_per_line);

    for (i = 0; i < 400; i++) {
        dev_write(pap, pbuf, bytes_per_line);
    }
    PAP_WAIT_NCLR();
#else
    for (i = 0; i < 240; i++) {
        for (j = 0; j < 400; j++) {
            u8 dat;
            dat = color >> 8;
            dev_write(pap, &dat, sizeof(dat));
            dat = color;
            dev_write(pap, &dat, sizeof(dat));
        }
    }
#endif

    TFT_CS_DIS();
    //free(buf);
}
/*
void tft_ClearScreen(u16 color)
{
    u8 *buf;
    u32 i;

    tft_set_pos(0,239,0,399);
    buf = malloc(240*400*3);

    for(i=0;i<240*3;i+=3)
    {
        buf[i] = 0x00;
        buf[i+1] = 0xff;
        buf[i+2] = 0xff;
    }

    for(i=0;i<400;i++)
    {
        dev_write(pap,buf,240*3);
    }
    PAP_WAIT_NCLR();
    TFT_CS_DIS();
    free(buf);
}*/

static void lcd_lq026b3ux01_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u8 id;
    u32 i, j;
    u8 *buf;

    printf("\nLQ026B3UX01 init ...");
    TFT_CS_OUT();
    TFT_RS_OUT();
    TFT_RST_OUT();

    TFT_RST_H();
    delay_2ms(100);
    TFT_RST_L();
    delay_2ms(100);
    TFT_RST_H();
#if (DISP_MODE == PORT08_PIXEL16_RGB565_CYCLE2)||(DISP_MODE == PORT08_PIXEL18_RGB666_CYCLE3)||(DISP_MODE == PORT09_PIXEL18_RGB666_CYCLE2)
    struct pap_info pap_info_t;
    pap_info_t.datawidth = PAP_PORT_8BITS;
    pap_info_t.endian    = PAP_BE;
    pap_info_t.cycle     = PAP_CYCLE_ONE;
    pap_info_t.group_sel = PAP_GROUP_PC_PD;
    pap_info_t.timing_setup = 2;
    pap_info_t.timing_hold  = 2;
    pap_info_t.timing_width = 5;
    pap = dev_open("pap", &pap_info_t);
#elif (DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE1)
    struct pap_info pap_info_t;
    pap_info_t.datawidth = PAP_PORT_16BITS;
#if ENDIAN == LE
    pap_info_t.endian    = PAP_LE;
#else
    pap_info_t.endian    = PAP_BE;
#endif
    pap_info_t.cycle     = PAP_CYCLE_ONE;
    pap_info_t.group_sel = PAP_GROUP_PC_PD;
    pap_info_t.timing_setup = 2;
    pap_info_t.timing_hold  = 2;
    pap_info_t.timing_width = 5;
    pap = dev_open("pap", &pap_info_t);
#elif (DISP_MODE == PORT18_PIXEL18_RGB666_CYCLE1)
    struct pap_info pap_info_t;
    pap_info_t.datawidth = PAP_PORT_16BITS;
    pap_info_t.endian    = PAP_LE;
    pap_info_t.cycle     = PAP_CYCLE_ONE;
    pap_info_t.group_sel = PAP_GROUP_PC_PD;
    pap_info_t.timing_setup = 2;
    pap_info_t.timing_hold  = 2;
    pap_info_t.timing_width = 5;
    pap = dev_open("pap", &pap_info_t);
#endif

    wr_reg(0x68, 0x07);
    wr_reg(0x69, 0x80);

    //8bit
#if (DISP_MODE == PORT08_PIXEL16_RGB565_CYCLE2)
    wr_reg(0x01, 0x02); //8bit+8bit 2 times
#elif (DISP_MODE == PORT08_PIXEL18_RGB666_CYCLE3)
    wr_reg(0x01, 0x04); //6bit+6bit+6bit 3 times DB[7..2]
#elif (DISP_MODE == PORT09_PIXEL16_RGB565_CYCLE2)
    //9bit
    wr_reg(0x01, 0x02); //8bit+8bit 2 times
#elif (DISP_MODE == PORT09_PIXEL18_RGB666_CYCLE2)
    wr_reg(0x01, 0x00); //9bit+9bit 2 times
#elif (DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE1)
    //16bit
    wr_reg(0x01, 0x02); //16bit 1 times
#elif (DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE2)
    wr_reg(0x01, 0x03); //8bit+8bit 2 times
#elif (DISP_MODE == PORT16_PIXEL18_RGB666_CYCLE3)
    wr_reg(0x01, 0x05); //6bit+6bit+6bit 3 times DB[7..2]
#elif (DISP_MODE == PORT18_PIXEL18_RGB666_CYCLE1)
    //18bit
    wr_reg(0x01, 0x00); //18bit 1 times
#elif (DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE2)
    wr_reg(0x01, 0x02); //8bit+8bit 2times
#elif (DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE1)
    wr_reg(0x01, 0x03); //16bit 1 times
#endif

    wr_reg(0x08, 0x00);
    wr_reg(0x10, 0xF0);
    wr_reg(0x11, 0x90);
    wr_reg(0x12, 0x01);
    wr_reg(0x1A, 0xef);
    wr_reg(0x1B, 0x00);
    wr_reg(0x1C, 0x00);

    /*
    screen
    1|2|3
    4|5|6
    7|8|9
    */
#if (DIRECTION == DIRECTION_VERT)
    wr_reg(0x1D, 0x04); //3->2->1->6->5->4->9->8->7
    //wr_reg(0x1D,0x00);//1->2->3->4->5->6->7->8->9
    //wr_reg(0x1D,0x02);//7->8->9->4->5->6->1->2->3
    //wr_reg(0x1D,0x06);//9->8->7->6->5->4->3->2->1
#elif (DIRECTION == DIRECTION_HORI)
    //wr_reg(0x1D,0x01);//1->4->7->2->5->8->3->6->9
    //wr_reg(0x1D,0x05);//3->6->9->2->5->8->1->4->7
    wr_reg(0x1D, 0x03); //7->4->1->8->5->2->9->6->3
    //wr_reg(0x1D,0x07);//9->6->3->8->5->2->7->4->1
#endif
    wr_reg(0x1E, 0x50);
    wr_reg(0x30, 0x81); //wr_reg(0x30,0x80);
    wr_reg(0x33, 0x02);
    wr_reg(0x42, 0x07);
    wr_reg(0x43, 0x01);
    wr_reg(0x44, 0x04);
    wr_reg(0x45, 0x08);
    wr_reg(0x46, 0x06);
    wr_reg(0x48, 0x01);
    wr_reg(0x4A, 0x01);
    wr_reg(0x4B, 0xA8);
    wr_reg(0x4C, 0x07);
    wr_reg(0x5A, 0x00);
    wr_reg(0x62, 0x21);
    wr_reg(0x65, 0x02);
    wr_reg(0x66, 0x00);
    wr_reg(0x6A, 0x02);
    wr_reg(0x81, 0x44);
    wr_reg(0x82, 0x15);
    wr_reg(0x83, 0x25);
    wr_reg(0x84, 0x10);
    wr_reg(0x85, 0x25);
    wr_reg(0x86, 0x17);
    wr_reg(0x90, 0x25);
    wr_reg(0x91, 0x04);
    wr_reg(0x92, 0x15);
    wr_reg(0x31, 0x01);
    wr_reg(0x3E, 0x01);

    if (1) {
        tft_ClearScreen(0xFF0000);
        delay_2ms(500);
        tft_ClearScreen(0x00FF00);
        delay_2ms(500);
        tft_ClearScreen(0x0000FF);
        delay_2ms(500);
    }

    printf("\ninit out ...");
    rd_reg(0x00);
    rd_reg(0x01);
    rd_reg(0x10);
    rd_reg(0x11);

    tft_set_pos(0, 239, 0, 399);
    dev_close(pap);
}

static void lcd_lq026b3ux01_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

REGISTER_LCD_DEVICE(lcd_lq026b3ux01_dev) = {
    .info = {
#if (DISP_MODE == PORT08_PIXEL16_RGB565_CYCLE2)||(DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE2)||(DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE2)
        .format          = FORMAT_RGB565,
#elif (DISP_MODE == PORT09_PIXEL16_RGB565_CYCLE2)
        .format          = FORMAT_RGB565,
#elif (DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE1)||(DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE1)
        .format          = FORMAT_RGB565,
#elif (DISP_MODE == PORT08_PIXEL18_RGB666_CYCLE3)||(DISP_MODE == PORT16_PIXEL18_RGB666_CYCLE3)
        .format          = FORMAT_RGB666,
#elif (DISP_MODE == PORT09_PIXEL18_RGB666_CYCLE2)
        .format          = FORMAT_RGB666,
#elif (DISP_MODE == PORT18_PIXEL18_RGB666_CYCLE1)
        .format          = FORMAT_RGB666,
#endif
        .len 			 = LEN_256,
        .test_mode 	     = false,
        .color           = 0x0000FF,
        .rotate_en       = true,	    // 旋转使能
        .hori_mirror_en  = true,	    // 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能

        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,

        .itp_mode_en     = FALSE,
        .sample          = LCD_DEV_SAMPLE,
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
    .drive_mode      = MODE_MCU,

#if (DISP_MODE == PORT08_PIXEL16_RGB565_CYCLE2)||(DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE2)||(DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE2)
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_8BITS,
#elif (DISP_MODE == PORT09_PIXEL16_RGB565_CYCLE2)
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_9BITS,
#elif (DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE1)||(DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE1)
    .ncycle          = CYCLE_ONE,
    .data_width      = PORT_16BITS,
#elif (DISP_MODE == PORT08_PIXEL18_RGB666_CYCLE3)||(DISP_MODE == PORT16_PIXEL18_RGB666_CYCLE3)
    .ncycle          = CYCLE_THREE,
    .data_width      = PORT_6BITS,
#elif (DISP_MODE == PORT09_PIXEL18_RGB666_CYCLE2)
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_9BITS,
#elif (DISP_MODE == PORT18_PIXEL18_RGB666_CYCLE1)
    .ncycle          = CYCLE_ONE,
    .data_width      = PORT_18BITS,
#endif

    .interlaced_mode = INTERLACED_NONE,

    .dclk_set        = 0/*CLK_EN | CLK_NEGATIVE*/,
    .sync0_set       = SIGNAL_DEN | CLK_EN | CLK_NEGATIVE,
    .sync1_set       = SIGNAL_HSYNC/*|CLK_EN|CLK_NEGATIVE*/,
    .sync2_set       = SIGNAL_VSYNC/*|CLK_EN|CLK_NEGATIVE*/,

    .port_sel        = PORT_GROUP_B,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_4,

    .timing = {
#if (DISP_MODE == PORT08_PIXEL16_RGB565_CYCLE2)||\
	(DISP_MODE == PORT09_PIXEL16_RGB565_CYCLE2)||\
	(DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE2)||\
	(DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE2)
        .hori_interval 		= (240  + 20) * 2,
        .hori_sync_clk 		=    10 * 2,
        .hori_start_clk 	=    10 * 2,
        .hori_pixel_width 	=    240,

        .vert_interval 		=    400 + 40,
        .vert_sync_clk 		=    20,
        .vert_start_clk 	=    20,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    400,
#elif (DISP_MODE == PORT16_PIXEL16_RGB565_CYCLE1)||(DISP_MODE == PORT18_PIXEL16_RGB565_CYCLE1)
        .hori_interval 		=    240 + 40,
        .hori_sync_clk 		=    20,
        .hori_start_clk 	=    20,
        .hori_pixel_width 	=    240,

        .vert_interval 		=    400 + 40,
        .vert_sync_clk 		=    20,
        .vert_start_clk 	=    20,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    400,
#elif (DISP_MODE == PORT08_PIXEL18_RGB666_CYCLE3)||(DISP_MODE == PORT16_PIXEL18_RGB666_CYCLE3)
        .hori_interval 		= (240 + 20) * 3,
        .hori_sync_clk 		=    10 * 3,
        .hori_start_clk 	=    10 * 3,
        .hori_pixel_width 	=    240,

        .vert_interval 		=    400 + 40,
        .vert_sync_clk 		=    20,
        .vert_start_clk 	=    20,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    400,
#elif (DISP_MODE == PORT09_PIXEL18_RGB666_CYCLE2)
        .hori_interval 		= (240 + 20) * 2,
        .hori_sync_clk 		=    10 * 2,
        .hori_start_clk 	=    10 * 2,
        .hori_pixel_width 	=    240,

        .vert_interval 		=    400 + 40,
        .vert_sync_clk 		=    20,
        .vert_start_clk 	=    20,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    400,
#elif (DISP_MODE == PORT18_PIXEL18_RGB666_CYCLE1)
        .hori_interval 		=    240 + 40,
        .hori_sync_clk 		=    20,
        .hori_start_clk 	=    20,
        .hori_pixel_width 	=    240,

        .vert_interval 		=    400 + 40,
        .vert_sync_clk 		=    20,
        .vert_start_clk 	=    20,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    400,
#endif
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_DVP_RGB,
    .init 	 = lcd_lq026b3ux01_init,
    .dev  	 = &lcd_lq026b3ux01_dev,
    .bl_ctrl = lcd_lq026b3ux01_backctrl,
};
#endif
