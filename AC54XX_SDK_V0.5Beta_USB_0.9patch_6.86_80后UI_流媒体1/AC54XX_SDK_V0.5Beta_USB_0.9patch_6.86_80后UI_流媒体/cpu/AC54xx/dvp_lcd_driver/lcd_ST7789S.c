#include "generic/typedef.h"
#include "asm/imb_driver.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/pap.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "os/os_api.h"
#include "gpio.h"


#define delay2ms(t) delay_2ms(t)

#if (LCD_TYPE == LCD_ST7789S_MCU)


#define SOFT_SIM  //use pap module

//PC5->PV5 RESET
#define LCD_RST_OUT()   PORTG_DIR &=~ BIT(4)
#define LCD_RST_H()     LCD_RST_OUT();PORTG_OUT |=  BIT(4)
#define LCD_RST_L()     LCD_RST_OUT();PORTG_OUT &=~ BIT(4)

//PC6->PV6 CS
#define LCD_CS_OUT()    PORTH_DIR &=~ BIT(1)
#define LCD_CS_H()      LCD_CS_OUT();PORTH_OUT |=  BIT(1)
#define LCD_CS_L()      LCD_CS_OUT();PORTH_OUT &=~ BIT(1)

//PC7->PV7  RS
#define LCD_RS_OUT()    PORTH_DIR &=~ BIT(0)
#define LCD_RS_H()      LCD_RS_OUT();PORTH_OUT |=  BIT(0)
#define LCD_RS_L()      LCD_RS_OUT();PORTH_OUT &=~ BIT(0)


#ifdef SOFT_SIM

#define LCD_WR_OUT()    PORTH_DIR &=~ BIT(3)
#define LCD_WR_H()      LCD_WR_OUT();PORTH_OUT |= BIT(3)
#define LCD_WR_L()      LCD_WR_OUT();PORTH_OUT &=~BIT(3)

#define LCD_RD_OUT()    PORTH_DIR &=~ BIT(4)
#define LCD_RD_H()      LCD_RD_OUT();PORTH_OUT |= BIT(4)
#define LCD_RD_L()      LCD_RD_OUT();PORTH_OUT &=~BIT(4)

#define LCD_PORT_OUT()  PORTG_DIR &=~ 0xff00;PORTG_PU |= 0xff00
#define LCD_PORT_IN()   PORTG_DIR |=  0xff00;PORTG_PU |= 0xff00
#define LCD_PORT_W      PORTG_OUT
#define LCD_PORT_R      PORTG_IN
#endif//end of SOFT_SIM

typedef struct {
    u8 cmd;
    u8 cnt;
    u8 dat[128];
} InitCode;

#define REGFLAG_DELAY 0xFF

static const InitCode code1[] = {
    {0x01, 0},//soft reset
    {REGFLAG_DELAY, 120},
    //{0x11, 0},
    //{REGFLAG_DELAY, 120},
    {0x36, 1, {0x00}},
    {0x21, 0},
    {0xB2, 5, {0x05, 0x05, 0x00, 0x33, 0x33}},
    {0xB7, 1, {0x75}},
    {0xBB, 1, (0x22)},
    {0xC0, 1, {0x2C}},
    {0xC2, 1, {0x01}},
    {0xC3, 1, {0x13}},
    {0xC4, 1, {0x20}},
    {0xC6, 1, {0x11}},
    {0xD0, 2, {0xA4, 0xA1}},
    {0xD6, 1, {0xA1}},
    {0xE0, 14, {0xD0, 0x05, 0x0A, 0x09, 0x08, 0x05, 0x2E, 0x44, 0x45, 0x0F, 0x17, 0x16, 0x2B, 0x33}},
    {0xE1, 14, {0xD0, 0x05, 0x0A, 0x09, 0x08, 0x05, 0x2E, 0x43, 0x45, 0x0F, 0x16, 0x16, 0x2B, 0x33}},
    {0x3A, 1, {0x05}},
    {0x35, 1},

    {0x11, 0},//Sleep out
    {REGFLAG_DELAY, 150},
    {0x29, 0},
    {REGFLAG_DELAY, 100},
};
static void *pap = NULL;

extern void delay(volatile u32  t);

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
#else
    struct pap_info pap_info_t;
    pap_info_t.datawidth 	= PAP_PORT_8BITS;
    pap_info_t.endian    	= PAP_BE;//8bit必须为大端
    pap_info_t.cycle     	= PAP_CYCLE_ONE;
    pap_info_t.group_sel 	= PAP_GROUP_PG_PH;
    pap_info_t.wr_rd_sel    = PAP_PH3_PH4;
    pap_info_t.timing_setup = 2;
    pap_info_t.timing_hold  = 2;
    pap_info_t.timing_width = 5;
    pap = dev_open("pap", &pap_info_t);
#endif

    LCD_RST_H();
    delay2ms(60);
    LCD_RST_L();
    delay2ms(10);
    LCD_RST_H();
    delay2ms(100);
}

#ifdef SOFT_SIM

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

#endif

static void WriteComm(u16 cmd)
{
    LCD_CS_L();
    LCD_RS_L();
#ifndef SOFT_SIM
    dev_write(pap, &cmd, sizeof(cmd));
#else
    LCD_RD_H();
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

static void st7789s_init_code(const InitCode *code, u8 cnt)
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

static void lcd_st7789s_setxy(u16 Xstart, u16 Xend, u16 Ystart, u16 Yend)
{
    //HX8352-C
    /* printf("%x:%x:%x:%x\n",Xstart,Xend,Ystart,Yend); */

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


static void lcd_st7789s_clear(u16 w, u16 h, u32 color)
{
    u32 dat;
    u32 len;
    u16 i, j;

    lcd_st7789s_setxy(0, w - 1, 0, h - 1);

    dat = ((color >> 19) << 11) & 0xf800 | ((color >> 10) << 5) & 0x07e0 | (color >> 3) & 0x001f;
    len = w * h;
    while (len--) {
        WriteData(dat >> 8);
        WriteData(dat);
    }
}

static void lcd_st7789s_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u16 ret;
    u8 i;
    printf("\nlcd hx8369 init...\n");

    LCD_Reset();

    WriteComm(0x11);
    delay2ms(120);

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

    st7789s_init_code(code1, sizeof(code1) / sizeof(code1[0]));
    gpio_direction_output(IO_PORTG_02, 1);
    // while(0)
    {
        WriteComm(0x04);
        ret = ReadData();
        printf("0x04 1 = 0x%x\n", ret);
        ret = ReadData();
        printf("0x04 2 = 0x%x\n", ret);
        ret = ReadData();
        printf("0x04 3 = 0x%x\n", ret);
        ret = ReadData();
        printf("0x04 4 = 0x%x\n", ret);
    }

    lcd_st7789s_clear(240, 320, 0xff0000);
    delay2ms(500);
    lcd_st7789s_clear(240, 320, 0x00ff00);
    delay2ms(500);
    lcd_st7789s_clear(240, 320, 0x0000ff);
    delay2ms(500);

    lcd_st7789s_setxy(0, 240 - 1, 0, 320 - 1);

    LCD_CS_L();
    LCD_RS_H();
#ifdef SOFT_SIM
    LCD_RD_H();
#endif

    /*
     * 开背光
     */
    PORTG_DIR &= ~ BIT(2);
    PORTG_OUT |=  BIT(2);
}

static void lcd_st7789s_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE(lcd_st7789s_dev) = {
    .info = {
        .test_mode       = false,
        .color           = 0x0000FF,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,
        .sample          = SAMP_YUV420,
        .len 			 = LEN_256,
        .itp_mode_en     = FALSE,

        .rotate_en 		 = true,		// 旋转使能
        .hori_mirror_en  = true,		// 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能

        .format          = FORMAT_RGB565,
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
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_8BITS,
    .interlaced_mode = INTERLACED_NONE,

    .dclk_set        = /*CLK_EN |*/CLK_NEGATIVE,
    .sync0_set       = CLK_EN | SIGNAL_DEN /*| CLK_NEGATIVE*/,
    .sync1_set       = /*CLK_EN|*/SIGNAL_HSYNC | CLK_NEGATIVE,
    .sync2_set       = /*CLK_EN|*/SIGNAL_VSYNC | CLK_NEGATIVE,

    .port_sel        = PORT_GROUP_AA,
#if 1
    .clk_cfg	 	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_2,
#else
    .clk_cfg	 	 = IMD_INTERNAL | OSC_32K | PLL1_CLK | DIVA_3 | DIVB_1 | DIVC_1,
    .pll1_nr         = 3662,/* 3662*0.032768=120M */
    .pll1_nf         = 1,
#endif

    .timing = {
        .hori_interval 		= (240 + 20) * 2,
        .hori_sync_clk 		= 10 * 2,
        .hori_start_clk 	= 10 * 2,
        .hori_pixel_width 	= 240,

        .vert_interval 		= 320 + 40,
        .vert_sync_clk 		= 20,
        .vert_start_clk 	= 20,
        .vert_start1_clk 	= 0,
        .vert_pixel_width 	= 320,
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable  = IF_ENABLE(LCD_ST7789S_MCU),
    .logo    = "st7789s",
    .type    = LCD_DVP_MCU,
    .init    = lcd_st7789s_init,
    .dev   	 = &lcd_st7789s_dev,
    .setxy   = lcd_st7789s_setxy,
    .bl_ctrl = lcd_st7789s_backctrl,
};

#endif
