#include "generic/typedef.h"
#include "asm/imb_driver.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/pap.h"
#include "asm/lcd_config.h"
#include "os/os_api.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#define delay2ms(t) delay_2ms(t)

#if (LCD_TYPE == LCD_HX8369A_MCU)

#define DBI_8BITS   0//BS[3..0]:0100  PC[15..8] <-> DB[7..0]                                                                            OK
#define DBI_9BITS   1//BS[3..0]:0101  PC[15..7] <-> DB[8..0]                                                                            OK
#define DBI_16BITS  2//BS[3..0]:0110  LE{PC[15..0]}/BE{PC[7..0],PC[15..8]} <-> DB[15..0]                                                OK
#define DBI_18BITS  3//BS[3..0]:0111  PD[1..0],PC[15..0] <-> DB[17..0]                                                                  OK
#define DBI_24BITS  4//BS[3..0]:1010  PD[1..0],PC[15..12],PD[11..10] PC[11..6],PD[9..8] PC[5..0],PD[7..6]   DB[23..0]                   OK
#define DBI_FORMAT  DBI_8BITS


/* #define SOFT_SIM  //use pap module */

//PC5->PV5 RESET
#define LCD_RST_OUT()   PORTD_DIR &=~ BIT(9)
#define LCD_RST_H()     LCD_RST_OUT();PORTD_OUT |=  BIT(9)
#define LCD_RST_L()     LCD_RST_OUT();PORTD_OUT &=~ BIT(9)

//PC6->PV6 CS
#define LCD_CS_OUT()    PORTD_DIR &=~ BIT(10)
#define LCD_CS_H()      LCD_CS_OUT();PORTD_OUT |=  BIT(10)
#define LCD_CS_L()      LCD_CS_OUT();PORTD_OUT &=~ BIT(10)

//PC7->PV7  RS
#define LCD_RS_OUT()    PORTD_DIR &=~ BIT(11)
#define LCD_RS_H()      LCD_RS_OUT();PORTD_OUT |=  BIT(11)
#define LCD_RS_L()      LCD_RS_OUT();PORTD_OUT &=~ BIT(11)

#ifdef SOFT_SIM
#define LCD_WR_OUT()    PORTD_DIR &=~ BIT(3)
#define LCD_WR_H()      /*LCD_WR_OUT();*/PORTD_OUT |= BIT(3)
#define LCD_WR_L()      /*LCD_WR_OUT();*/PORTD_OUT &=~BIT(3)

#define LCD_RD_OUT()    PORTD_DIR &=~ BIT(4)
#define LCD_RD_H()      /*LCD_RD_OUT();*/PORTD_OUT |= BIT(4)
#define LCD_RD_L()      /*LCD_RD_OUT();*/PORTD_OUT &=~BIT(4)

#define LCD_PORT_OUT()  PORTC_DIR &=~ 0xff00;PORTC_PU |= 0xff00
#define LCD_PORT_IN()   PORTC_DIR |=  0xff00;PORTC_PU |= 0xff00
#define LCD_PORT_W      PORTC_OUT
#define LCD_PORT_R      PORTC_IN
#endif//end of SOFT_SIM

static void *pap = NULL;

extern void delay(volatile u32  t);

#define REGFLAG_DELAY 0xFF

typedef struct {
    u8 cmd;
    u8 cnt;
    u8 dat[128];
} InitCode;

static const InitCode code1[] = {
    {0x01, 0},//soft reset
    {REGFLAG_DELAY, 50},
    {0xB9, 3,  {0xFF, 0x83, 0x69}}, // SET password
    {0xB1, 19, {0x01, 0x00, 0x34, 0x06, 0x00, 0x11, 0x11, 0x2A, 0x32, 0x3F, 0x3F, 0x07, 0x23, 0x01, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6}}, //Set Power
    {0xB2, 15, {0x00, 0x20, 0x05, 0x03, 0x70, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x01,}}, // SET Display  480x800
    {0xB4, 5,  {0x00, 0x0f, 0x82, 0x0c, 0x03}},
    {0xB6, 2,  {0x42, 0x42}}, // SET VCOM
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
    {0x36, 1,  {0xa0}},
#if ((DBI_FORMAT==DBI_8BITS)||(DBI_FORMAT==DBI_16BITS))
    {0x3a, 1,  {0x05}},//pixel format setting
#elif ((DBI_FORMAT==DBI_9BITS)||(DBI_FORMAT==DBI_18BITS))
    {0x3a, 1,  {0x06}},//pixel format setting
#else
    {0x3a, 1,  {0x07}},//pixel format setting
#endif
    {0x11, 0},//Sleep out
    {REGFLAG_DELAY, 150},
    {0x29, 0},
    {REGFLAG_DELAY, 100},
};

static void LCD_Reset()
{
    printf("reset \n");
    LCD_RST_OUT();
    LCD_RS_OUT();
    LCD_CS_OUT();
#ifdef SOFT_SIM
    LCD_RD_OUT();
    LCD_WR_OUT();

    LCD_RD_H();
    LCD_WR_H();
#else //end of SOFT_SIM
#if (DBI_FORMAT == DBI_8BITS)
    struct pap_info pap_info_t;
    pap_info_t.datawidth 	= PAP_PORT_8BITS;
    pap_info_t.endian    	= PAP_BE;//8bit必须为大端
    pap_info_t.cycle     	= PAP_CYCLE_ONE;
    pap_info_t.group_sel 	= PAP_GROUP_PC_PD;
    pap_info_t.timing_setup = 2;
    pap_info_t.timing_hold  = 2;
    pap_info_t.timing_width = 5;
    pap = dev_open("pap", &pap_info_t);
#elif (DBI_FORMAT == DBI_16BITS)||(DBI_FORMAT == DBI_18BITS)
    struct pap_info pap_info_t;
    pap_info_t.datawidth 	= PAP_PORT_16BITS;
    pap_info_t.endian    	= PAP_LE;
    pap_info_t.cycle     	= PAP_CYCLE_ONE;
    pap_info_t.group_sel 	= PAP_GROUP_PC_PD;
    pap_info_t.timing_setup = 2;
    pap_info_t.timing_hold  = 2;
    pap_info_t.timing_width = 5;
    pap = dev_open("pap", &pap_info_t);
#endif
#endif

    LCD_RST_H();
    delay2ms(60);
    LCD_RST_L();
    delay2ms(10);
    LCD_RST_H();
    delay2ms(100);
}

#ifdef SOFT_SIM

#if (DBI_FORMAT == DBI_16BITS)||(DBI_FORMAT == DBI_18BITS)
static void LCDWrite(u16 dat)
{
    LCD_PORT_OUT();
    LCD_WR_L();

    LCD_PORT_W = dat;
    PORTD_DIR &= ~ BIT(12);
    if (dat & BIT(1)) {
        PORTD_OUT |= BIT(12);
    } else {
        PORTD_OUT &= ~BIT(12);
    }

    //delay(100);
    LCD_WR_H();
}

static u16 LCDRead()
{
    u16 ret;
    LCD_PORT_IN();
    LCD_RD_L();
    //delay(100);
    LCD_RD_H();

    ret = LCD_PORT_R;
    PORTD_DIR |= BIT(12);
    if (PORTD_IN & BIT(12)) {
        ret |= BIT(1);
    } else {
        ret &= ~ BIT(1);
    }

    return ret;
}
#elif (DBI_FORMAT == DBI_8BITS)

static void LCDWrite(u16 dat)
{
    LCD_PORT_OUT();
    LCD_WR_L();
    LCD_PORT_W &= ~0xff00;
    LCD_PORT_W |= (dat & 0xff) << 8;
    LCD_WR_H();
}

static u16 LCDRead()
{
    LCD_PORT_IN();
    LCD_RD_L();
    LCD_RD_H();

    return (LCD_PORT_R >> 8) & 0xff;
}
#elif (DBI_FORMAT == DBI_9BITS)
static void LCDWrite(u16 dat)
{
    LCD_PORT_OUT();
    LCD_WR_L();
    LCD_PORT_W = (dat & 0x1ff) << 7;
    //delay(5);
    LCD_WR_H();
    //delay(5);
}

static u16 LCDRead()
{
    LCD_PORT_IN();
    LCD_RD_L();
    //delay(5);
    LCD_RD_H();
    //delay(5);
    return (LCD_PORT_R >> 7) & 0x1ff;
}
#elif (DBI_FORMAT == DBI_24BITS)
static void LCDWrite(u16 dat)
{
    u8 dath, datl;

    datl = dat & 0xff;
    dath = (dat >> 8) & 0xff;

    LCD_PORT_OUT();

    LCD_WR_L();
    //[1..0]
    PORTD_DIR &= ~ 0xc0;
    PORTD_OUT &= ~ 0xc0;
    PORTD_OUT |= (datl & 0x03) << 6;
    //[7..2]
    LCD_PORT_W &= ~ 0x3f;
    LCD_PORT_W &= ~ 0x3f;
    LCD_PORT_W |=  datl >> 2;

    //9..8
    PORTD_DIR &= ~ 0x300;
    PORTD_OUT &= ~ 0x300;
    PORTD_OUT |= (dath & 0x03) << 8;
    //[15..10]
    LCD_PORT_W &= ~ 0xfc0;
    LCD_PORT_W &= ~ 0xfc0;
    LCD_PORT_W |= (dath >> 2) << 6;

    LCD_WR_H();
}

static u16 LCDRead()
{
    u16 ret;
    LCD_PORT_IN();
    LCD_RD_L();
    //delay(100);
    LCD_RD_H();

    ret = (LCD_PORT_R & 0x3f) << 2;

    PORTD_DIR |= 0xc0;
    ret |= (PORTD_IN & 0xc0) >> 6;

    return ret & 0xff;
}
#endif
#endif

static void WriteComm(u16 cmd)
{
    LCD_CS_L();
    LCD_RS_L();
#ifndef SOFT_SIM
    dev_write(pap, &cmd, sizeof(cmd));
#else
    LCD_RD_H();
    //delay(100);
    LCDWrite(cmd);
#endif
    LCD_CS_H();
}

static void WriteData(u16 dat)
{
    LCD_CS_L();
    LCD_RS_H();
#ifndef SOFT_SIM
    dev_write(pap, &dat, sizeof(dat));
#else
    LCD_RD_H();
    //delay(100);
    LCDWrite(dat);
#endif
    LCD_CS_H();
}

static u16 ReadData()
{
    u16 ret = 0;

    LCD_CS_L();
    LCD_RS_H();
#ifndef SOFT_SIM
    dev_read(pap, &ret, sizeof(ret));
#else
    LCD_WR_H();
    ret = LCDRead();
#endif
    LCD_CS_H();

    return ret;
}


static void hx8369a_init_code(const InitCode *code, u8 cnt)
{
    u8 i, j;

    for (i = 0; i < cnt; i++) {
        if (code[i].cmd == REGFLAG_DELAY) {
            delay2ms(code[i].cnt);
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

static void lcd_hx8369a_setxy(u16 Xstart, u16 Xend, u16 Ystart, u16 Yend)
{
    //HX8352-C

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

    LCD_CS_L();
    LCD_RS_H();
}

static void lcd_hx8369a_clear(u16 w, u16 h, u32 color)
{
    u16 i, j;
    u32 dat;
    u32 len;

    len = w * h;
    lcd_hx8369a_setxy(0, w - 1, 0, h - 1);

#if (DBI_FORMAT == DBI_8BITS)
    dat = ((color >> 19) << 11) & 0xf800 | ((color >> 10) << 5) & 0x07e0 | (color >> 3) & 0x001f;
#elif (DBI_FORMAT == DBI_9BITS)
    dat = ((color >> 19) << 12) & 0x3f000 | ((color >> 10) << 6) & 0x0fc0 | (color >> 3) & 0x003f;
#elif (DBI_FORMAT == DBI_16BITS)
    dat = ((color >> 19) << 11) & 0xf800 | ((color >> 10) << 5) & 0x07e0 | (color >> 3) & 0x001f;
#elif (DBI_FORMAT == DBI_18BITS)
    dat = ((color >> 19) << 12) & 0x3f000 | ((color >> 10) << 6) & 0x0fc0 | (color >> 3) & 0x003f;
#endif

    while (len--) {
#if (DBI_FORMAT == DBI_8BITS)
        WriteData(dat >> 8);
        WriteData(dat);
#elif (DBI_FORMAT == DBI_9BITS)
        WriteData(dat >> 9);
        WriteData(dat);
#elif (DBI_FORMAT == DBI_16BITS)
        WriteData(dat);
#elif (DBI_FORMAT == DBI_18BITS)
        WriteData(dat);
#elif (DBI_FORMAT == DBI_24BITS)
        WriteData(color);
#endif
    }
}

static void lcd_hx8369a_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u16 ret;
    u8 i;

    printf("\nlcd hx8369 init...\n");

    LCD_Reset();

    //WriteComm(0x01);
    //delay2ms(100);

    WriteComm(0x11);
    delay2ms(120);
#if ((DBI_FORMAT==DBI_8BITS)||(DBI_FORMAT==DBI_16BITS))
    WriteComm(0x2D);
    for (i = 0; i <= 63; i++) {
        WriteData(i * 8);
    }

    for (i = 0; i <= 63; i++) {
        WriteData(i * 4);
    }

    for (i = 0; i <= 63; i++) {
        WriteData(i * 8);
    }
#elif ((DBI_FORMAT==DBI_9BITS)||(DBI_FORMAT==DBI_18BITS))
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
#endif

    hx8369a_init_code(code1, sizeof(code1) / sizeof(code1[0]));

    WriteComm(0x0C);
    ReadData();
    ret = ReadData();//0x05
    printf("ret = %x\n", ret);

    WriteComm(0xF4);
    ret = ReadData();
    //printf("ret = %x\n",ret);
    ret = ReadData();//0x69
    printf("ret = %x\n", ret);
    ret = ReadData();//0x02
    printf("ret = %x\n", ret);

    /* lcd_hx8369a_clear(800, 480, 0xff0000); */
    /* delay2ms(500); */
    /* lcd_hx8369a_clear(800, 480, 0x00ff00); */
    /* delay2ms(500); */
    /* lcd_hx8369a_clear(800, 480, 0x0000ff); */
    /* delay2ms(500); */
    /* lcd_hx8369a_clear(800, 480, 0x000000); */
    /* delay2ms(500); */
    /* lcd_hx8369a_clear(800, 480, 0xffffff); */
    /* delay2ms(500); */

    lcd_hx8369a_setxy(0, 800 - 1, 0, 480 - 1);

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

REGISTER_LCD_DEVICE(lcd_hx8369a_mcu_dev) = {
    .info = {
        .test_mode 	     = false,
        .color           = 0x00FF00,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,
        .sample          = SAMP_YUV420,

        .itp_mode_en     = FALSE,
        .len 			 = LEN_256,
        .rotate_en       = false,	    // 旋转使能
        .hori_mirror_en  = false,	    // 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能

#if (DBI_FORMAT == DBI_8BITS)
        .format          = FORMAT_RGB565,
#elif (DBI_FORMAT == DBI_9BITS)
        .format          = FORMAT_RGB666,
#elif (DBI_FORMAT == DBI_16BITS)
        .format          = FORMAT_RGB565,
#elif (DBI_FORMAT == DBI_18BITS)
        .format          = FORMAT_RGB666,
#elif (DBI_FORMAT == DBI_24BITS)
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
        },
    },

    .drive_mode      = MODE_MCU,
#if (DBI_FORMAT == DBI_8BITS)
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_8BITS,
#elif (DBI_FORMAT == DBI_9BITS)
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_9BITS,
#elif (DBI_FORMAT == DBI_16BITS)
    .ncycle          = CYCLE_ONE,
    .data_width      = PORT_16BITS,
#elif (DBI_FORMAT == DBI_18BITS)
    .ncycle          = CYCLE_ONE,
    .data_width      = PORT_18BITS,
#elif (DBI_FORMAT == DBI_24BITS)
    .ncycle          = CYCLE_ONE,
    .data_width      = PORT_24BITS,
#endif
    .interlaced_mode = INTERLACED_NONE,

    .dclk_set        = /*CLK_EN |*/CLK_NEGATIVE,
    .sync0_set       =  CLK_EN | SIGNAL_DEN  /*|CLK_NEGATIVE*/,
    .sync1_set       = /*CLK_EN|*/SIGNAL_HSYNC | CLK_NEGATIVE,
    .sync2_set       = /*CLK_EN|*/SIGNAL_VSYNC | CLK_NEGATIVE,

    .port_sel        = PORT_GROUP_B,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_1,

    .timing = {
#if (DBI_FORMAT == DBI_16BITS)||(DBI_FORMAT == DBI_18BITS)||(DBI_FORMAT == DBI_24BITS)
        .hori_interval 		= 800 + 40,
        .hori_sync_clk 		= 20,
        .hori_start_clk 	= 20,
        .hori_pixel_width 	= 800,

        .vert_interval 		= 480 + 80,
        .vert_sync_clk 		= 40,
        .vert_start_clk 	= 40,
        .vert_start1_clk 	= 0,
        .vert_pixel_width 	= 480,
#else
        .hori_interval 		= (800 + 20) * 2,
        .hori_sync_clk 		= 10 * 2,
        .hori_start_clk 	= 10 * 2,
        .hori_pixel_width 	= 800,

        .vert_interval 		= 480 + 80,
        .vert_sync_clk 		= 40,
        .vert_start_clk 	= 40,
        .vert_start1_clk 	= 0,
        .vert_pixel_width 	= 480,
#endif
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_HX8369A_MCU),
    .logo    = "hx8369a",
    .type    = LCD_DVP_MCU,
    .init    = lcd_hx8369a_init,
    .dev     = &lcd_hx8369a_mcu_dev,
    .setxy   = lcd_hx8369a_setxy,
    .bl_ctrl = lcd_hx8369a_backctrl,
};

#endif
