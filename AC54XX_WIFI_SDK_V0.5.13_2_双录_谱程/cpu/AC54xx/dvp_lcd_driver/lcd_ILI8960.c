#include "asm/cpu.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_ILI8960

#define LCD_CS_OUT()    PORTB_DIR &=~BIT(6)
#define LCD_CS_DIS()    PORTB_OUT |= BIT(6)
#define LCD_CS_EN()     PORTB_OUT &=~BIT(6)

#define LCD_SDA_OUT()   PORTB_DIR &=~BIT(5);PORTB_PU |= BIT(5)
#define LCD_SDA_IN()    PORTB_DIR |= BIT(5)
#define LCD_SDA_H()     PORTB_OUT |= BIT(5)
#define LCD_SDA_L()     PORTB_OUT &=~BIT(5)
#define LCD_SDA_R()     PORTB_IN  & BIT(5)

#define LCD_SCL_OUT()   PORTB_DIR &=~BIT(7)
#define LCD_SCL_H()     PORTB_OUT |= BIT(7)
#define LCD_SCL_L()     PORTB_OUT &=~BIT(7)

#define RGB_8BIT                        0
#define RGB_8BIT_DUMMY_320x240_NTSC     1
#define RGB_8BIT_DUMMY_320x288_PAL      2
#define RGB_8BIT_DUMMY_360x240_NTSC     3
#define RGB_8BIT_DUMMY_360x288_PAL      4
#define YUV720x240_NTSC                 5
#define YUV720x288_PAL                  6
#define YUV640x240_NTSC                 7
#define YUV640x288_PAL                  8
#define CCIR656_720x240_NTSC            9
#define CCIR656_720x288_PAL             10
#define CCIR656_640x240_NTSC            11
#define CCIR656_640x288_PAL             12

#define DRIVE_MODE                      RGB_8BIT

#define INTERLACED_TEST
#define Delta_Color_Filter_Test



static void lcd_io_init()
{
    LCD_CS_OUT();
    LCD_SDA_OUT();
    LCD_SCL_OUT();

    LCD_CS_DIS();
    LCD_SDA_H();
    LCD_SCL_H();
}

static void delay_50ns(u16 cnt)//380ns
{
    while (cnt--) {
        delay(100);//50ns
    }
}

static void wr_reg(u8 reg, u8 dat)
{
    char i;
    LCD_CS_EN();
    LCD_SDA_OUT();
    delay_50ns(1);

    for (i = 7; i >= 0; i--) {
        LCD_SCL_L();
        delay_50ns(1);

        if (reg & BIT(7)) {
            LCD_SDA_H();
        } else {
            LCD_SDA_L();
        }
        reg <<= 1;

        LCD_SCL_H();
        delay_50ns(1);
    }

    for (i = 7; i >= 0; i--) {
        LCD_SCL_L();
        delay_50ns(1);

        if (dat & BIT(7)) {
            LCD_SDA_H();
        } else {
            LCD_SDA_L();
        }
        dat <<= 1;

        LCD_SCL_H();
        delay_50ns(1);
    }
    LCD_CS_DIS();
    delay_50ns(8);
}

static u8 rd_reg(u8 reg)
{
    char i;
    u8 dat = 0;
    LCD_CS_EN();
    LCD_SDA_OUT();
    delay_50ns(1);

    for (i = 7; i >= 0; i--) {
        LCD_SCL_L();
        delay_50ns(1);

        if (reg & BIT(7)) {
            LCD_SDA_H();
        } else {
            LCD_SDA_L();
        }
        reg <<= 1;

        LCD_SCL_H();
        delay_50ns(1);
    }

    LCD_SDA_IN();
    for (i = 7; i >= 0; i--) {
        LCD_SCL_L();
        delay_50ns(1);

        LCD_SCL_H();
        delay_50ns(1);

        dat <<= 1;
        if (LCD_SDA_R()) {
            dat |= BIT(0);
        }

        delay_50ns(1);
    }
    LCD_CS_DIS();
    delay_50ns(8);
    return dat;
}

u8 reg_tab[] = {
    0x00, 0x01, 0x03, 0x04, 0x06, 0x07, 0x08, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x2B, 0x55, 0x56, 0x57,
    0x61,
};

static void lcd_ili8960_init(void *_data)
{
    u8 i;
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    printf("ILI8960 initial...\n");

    lcd_io_init();

    wr_reg(0x05, 0x5F); //GRB STB
    delay_2ms(5);
    wr_reg(0x05, 0x1F); //GRB STB  Reset All Regesiter
    delay_2ms(25);
    wr_reg(0x05, 0x5F); //GRB STB
    delay_2ms(50);

    wr_reg(0x00, 0x09); //VCOMAC  00   04
    wr_reg(0x01, 0x92); //VCOMDC  95   92
    wr_reg(0x03, 0x40); //brightness setting
    wr_reg(0x04, 0x0F); //8-bit RGB interface,VDIR/HDIR   0B
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 0x46);
    wr_reg(0x08, 0x00);
    wr_reg(0x0B, 0x83);
    wr_reg(0x0C, 0x06);

    wr_reg(0x0D, 0x40); //contrast setting
    wr_reg(0x0E, 0x40);
    wr_reg(0x0F, 0x40);
    wr_reg(0x10, 0x40);
    wr_reg(0x11, 0x40);

    wr_reg(0x12, 0x00);
#ifdef Delta_Color_Filter_Test
    wr_reg(0x13, 0x00); //Delta color filter   01
#else
    wr_reg(0x13, 0x01); //Delta color filter   01
#endif

    wr_reg(0x16, 0x04); //Gamma setting  2.2
    wr_reg(0x17, 0x54);
    wr_reg(0x18, 0x54);
    wr_reg(0x19, 0x43);
    wr_reg(0x1A, 0x54);

    wr_reg(0x95, 0x00);
    wr_reg(0x96, 0x03);
    wr_reg(0x97, 0x02);
    wr_reg(0xa1, 0x00);

    //////////////////////////
#if (DRIVE_MODE == RGB_8BIT)
    wr_reg(0x00, 0x09);
    wr_reg(0x04, 0x0f); //8-bit RGB
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 24 * 3);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == RGB_8BIT_DUMMY_320x240_NTSC)
    wr_reg(0x00, 0x09);
    wr_reg(0x04, 0x1f); //8-bit RGB dummy  320*240 NTSC
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 60 * 4);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == RGB_8BIT_DUMMY_320x288_PAL)
    wr_reg(0x00, 0x09);
    wr_reg(0x04, 0x13); //8-bit RGB dummy  320*288 PAL
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 60 * 4);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == RGB_8BIT_DUMMY_360x240_NTSC)
    wr_reg(0x00, 0x09);
    wr_reg(0x04, 0x2f); //8-bit RGB dummy  360*240 NTSC
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 60 * 4);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == RGB_8BIT_DUMMY_360x288_PAL)
    wr_reg(0x00, 0x09);
    wr_reg(0x04, 0x23); //8-bit RGB dummy  360*288 PAL
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 60 * 4);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == YUV720x240_NTSC)
    wr_reg(0x00, 0x49); //CCIR601
    wr_reg(0x04, 0x6f); //YUV720
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 240);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == YUV720x288_PAL)
    wr_reg(0x00, 0x49); //CCIR601
    wr_reg(0x04, 0x63); //YUV720
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 240);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == YUV640x240_NTSC)
    wr_reg(0x00, 0x49); //CCIR601
    wr_reg(0x04, 0x4f); //YUV640
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 240);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == YUV640x288_PAL)
    wr_reg(0x00, 0x49); //CCIR601
    wr_reg(0x04, 0x43); //YUV640
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 240);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == CCIR656_720x240_NTSC)
    wr_reg(0x00, 0x09); //C601_EN = 0
    wr_reg(0x04, 0x47); //CCIR656 NTSC
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 240);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == CCIR656_720x288_PAL)
    wr_reg(0x00, 0x09); //C601_EN = 0
    wr_reg(0x04, 0x43); //CCIR656 PAL
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 240);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == CCIR656_640x240_NTSC)
    wr_reg(0x00, 0x09); //C601_EN = 0
    wr_reg(0x04, 0x67); //CCIR656 NTSC
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 240);
    wr_reg(0x0c, 0x46);
#elif (DRIVE_MODE == CCIR656_640x288_PAL)
    wr_reg(0x00, 0x09); //C601_EN = 0
    wr_reg(0x04, 0x63); //CCIR656 PAL
    wr_reg(0x06, 0x15);
    wr_reg(0x07, 240);
    wr_reg(0x0c, 0x46);
#endif

    wr_reg(0x2B, 0x01); //Exit STB mode and Auto display ON
    //wr_reg(0x2B,0x00);//Enter STB mode

    for (i = 0; i < sizeof(reg_tab) / sizeof(reg_tab[0]); i++) {
        u8 reg = reg_tab[i];
        reg &= ~BIT(7);
        reg |= (reg & BIT(6)) << 1;
        reg |= BIT(6);
        printf("\nreg:%02x val:%02x", reg_tab[i], rd_reg(reg));
    }
}


/******************************************
ILI8960接线说明
CS	-> PF12
SDA	-> PF13
SCL	-> PF14
HSYNC  -> PH4
VSYNC  -> PG4
DCLK   -> PH5
DB7	-> R5(PE7)
DB6	-> R4(PF7)
DB5	-> R3(PC0)
DB4	-> R2(PD0)
DB3	-> G7(PE2)
DB2	-> G6(PF2)
DB1	-> G5(PE3)
DB0	-> G4(PF3)
*******************************************/


static void lcd_ili8960_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE(lcd_ili8960_dev) = {
    .info = {
#if DRIVE_MODE == RGB_8BIT
        .xres = 320,
        .yres = 240,
        .format          = FORMAT_RGB888,
#elif DRIVE_MODE == RGB_8BIT_DUMMY_320x240_NTSC
        .xres = 320,
        .yres = 240,
        .format          = FORMAT_RGB888,
#elif DRIVE_MODE == RGB_8BIT_DUMMY_320x288_PAL
        .xres = 320,
        .yres = 288,
        .format          = FORMAT_RGB888,
#elif DRIVE_MODE == RGB_8BIT_DUMMY_360x240_NTSC
        .xres = 368,
        .yres = 240,
        .format          = FORMAT_RGB888,
#elif DRIVE_MODE == RGB_8BIT_DUMMY_360x288_PAL
        .xres = 368,
        .yres = 288,
        .format          = FORMAT_RGB888,
#elif DRIVE_MODE == YUV720x240_NTSC
        .xres = 720,
        .yres = 480,
        .format          = FORMAT_YUV422,
#elif DRIVE_MODE == YUV720x288_PAL
        .xres = 720,
        .yres = 576,
        .format          = FORMAT_YUV422,
#elif DRIVE_MODE == YUV640x240_NTSC
        .xres = 640,
        .yres = 480,
        .format          = FORMAT_YUV422,
#elif DRIVE_MODE == YUV640x288_PAL
        .xres = 640,
        .yres = 576,
        .format          = FORMAT_YUV422,
#elif DRIVE_MODE == CCIR656_720x240_NTSC
        .xres = 720,
        .yres = 480,
        .format = FORMAT_ITU656,
#elif DRIVE_MODE == CCIR656_720x288_PAL
        .xres = 720,
        .yres = 576,
        .format = FORMAT_ITU656,
#elif DRIVE_MODE == CCIR656_640x240_NTSC
        .xres = 640,
        .yres = 480,
        .format = FORMAT_ITU656,
#elif DRIVE_MODE == CCIR656_640x288_PAL
        .xres = 640,
        .yres = 576,
        .format = FORMAT_ITU656,
#endif
        .len 			 = LEN_256,
        .rotate_en       = false,	    // 旋转使能
        .hori_mirror_en  = false,	    // 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能

        .sample          = LCD_DEV_SAMPLE,
        .itp_mode_en     = FALSE,
        .test_mode 	     = false,
        .color           = 0x0000FF,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,

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
#if (DRIVE_MODE == YUV720x240_NTSC)||\
	(DRIVE_MODE == YUV640x240_NTSC)||\
	(DRIVE_MODE == YUV720x288_PAL)||\
	(DRIVE_MODE == YUV640x288_PAL)
    .ncycle          = CYCLE_TWO,
#ifdef INTERLACED_TEST
    .interlaced_mode = INTERLACED_ALL,
#else
    .interlaced_mode = INTERLACED_NONE,
#endif
    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN/*|CLK_EN|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
#elif (DRIVE_MODE == RGB_8BIT)
    /* .image_width     = 320, */
    .ncycle          = CYCLE_THREE,
    .interlaced_mode = INTERLACED_NONE,

    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN/*|CLK_EN|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,

#ifdef Delta_Color_Filter_Test
    .raw_odd_phase   = PHASE_R,
    .raw_even_phase  = PHASE_G,
#endif

#elif (DRIVE_MODE == CCIR656_720x288_PAL) || (DRIVE_MODE == CCIR656_720x240_NTSC) || (DRIVE_MODE == CCIR656_640x240_NTSC) || (DRIVE_MODE == CCIR656_640x288_PAL)
#if ((DRIVE_MODE == CCIR656_720x288_PAL)||(DRIVE_MODE == CCIR656_720x240_NTSC))
    /* .image_width  = 720, */
#else
    /* .image_width = 640, */
#endif
    .ncycle = CYCLE_TWO,
#ifdef INTERLACED_TEST
    .interlaced_mode = INTERLACED_ALL,
#else
    .interlaced_mode = INTERLACED_NONE,
#endif
    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN/*|CLK_EN|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC |/*CLK_EN|*/CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC |/*CLK_EN|*/CLK_NEGATIVE,
#elif (DRIVE_MODE == RGB_8BIT_DUMMY_320x240_NTSC)||(DRIVE_MODE == RGB_8BIT_DUMMY_320x288_PAL)||(DRIVE_MODE == RGB_8BIT_DUMMY_360x240_NTSC)||(DRIVE_MODE == RGB_8BIT_DUMMY_360x288_PAL)

    /* #if ((DRIVE_MODE == RGB_8BIT_DUMMY_320x240_NTSC)||(DRIVE_MODE == RGB_8BIT_DUMMY_320x288_PAL)) */
    /* .image_width     = 320, */
    /* #else */
    /* .image_width     = 368, */
    /* #endif */
    .ncycle          = CYCLE_FOUR,
#ifdef INTERLACED_TEST
    .interlaced_mode = INTERLACED_TIMING,
#else
    .interlaced_mode = INTERLACED_NONE,
#endif
    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    .sync0_set       = SIGNAL_DEN/*|CLK_EN|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
#endif

    .drive_mode      = MODE_RGB_SYNC,
    .data_width      = PORT_8BITS,
    .port_sel        = PORT_GROUP_B,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_2,

    .timing = {
#if (DRIVE_MODE == RGB_8BIT)
        //8-bit RGB
        .hori_interval 		=    1716,//1716,//1716,//1716,//1716/3
        .hori_sync_clk 		=    3,
        .hori_start_clk 	=    72,//72/3
        .hori_pixel_width 	=    320,//960/3

        .vert_interval 		=    263,//263,
        .vert_sync_clk 		=    1,
        .vert_start_clk 	=    21,
        .vert_start1_clk 	=    21,
        .vert_pixel_width 	=    240,
#endif

#if (DRIVE_MODE == RGB_8BIT_DUMMY_320x240_NTSC)
        //8-bit RGB + dummy 320*240 NTSC
        .hori_interval 		=   1560,//1560/4,
        .hori_sync_clk 		=   1,
        .hori_start_clk 	=   240,//240/4,
        .hori_pixel_width 	=   320,//1280/4,

        .vert_interval 		=   263,
        .vert_sync_clk 		=   1,
        .vert_start_clk 	=   21,
        .vert_start1_clk 	=   21,
        .vert_pixel_width 	=   240,
#endif

#if  (DRIVE_MODE == RGB_8BIT_DUMMY_320x288_PAL)
        //8-bit RGB + dummy 320*240 PAL
        .hori_interval 		=    1560,//1560/4,
        .hori_sync_clk 		=    1,
        .hori_start_clk 	=    240,//240/4,
        .hori_pixel_width 	=    320,//1280/4,

        .vert_interval 		=    313,//263,
        .vert_sync_clk 		=    1,
        .vert_start_clk 	=    23,//21,
        .vert_start1_clk 	=    24,//21,
        .vert_pixel_width 	=    288,//240,
#endif

#if  (DRIVE_MODE == RGB_8BIT_DUMMY_360x240_NTSC)
        //8-bit RGB + dummy 360*240 NTSC
        .hori_interval 		=    1716,//1716/4,
        .hori_sync_clk 		=    1,
        .hori_start_clk 	=    240,//240/4,
        .hori_pixel_width 	=    360,//1440/4,

        .vert_interval 		=    263,
        .vert_sync_clk 		=    1,
        .vert_start_clk 	=    21,
        .vert_start1_clk 	=    21,
        .vert_pixel_width 	=    240,
#endif

#if  (DRIVE_MODE == RGB_8BIT_DUMMY_360x288_PAL)
        //8-bit RGB + dummy 360*240 PAL
        .hori_interval 		=    1716,//1716/4,
        .hori_sync_clk 		=    1,
        .hori_start_clk 	=    240,//240/4,
        .hori_pixel_width 	=    360,//1440/4,

        .vert_interval 		=    313,//263,
        .vert_sync_clk 		=    1,
        .vert_start_clk 	=    23,//21,
        .vert_start1_clk 	=    24,//21,
        .vert_pixel_width 	=    288,//240,
#endif

#if  (DRIVE_MODE == YUV640x240_NTSC)
//YUV640 640x240 two clock
        .hori_interval 		=    1560,//1560/2,
        .hori_sync_clk 		=    1,
        .hori_start_clk 	=    240,//240/2,
        .hori_pixel_width 	=    640,//1280/2,

        .vert_interval 		=    263,
        .vert_sync_clk 		=    1,//42,
        .vert_start_clk 	=    21,
        .vert_start1_clk 	=    22,
        .vert_pixel_width 	=    240,
#endif

#if (DRIVE_MODE == YUV640x288_PAL)
        //YUV640 640x288 two clock
        .hori_interval 		=    1560,//1560/2,
        .hori_sync_clk 		=    1,
        .hori_start_clk 	=    240,//240/2,
        .hori_pixel_width 	=    640,//1280/2,

        .vert_interval 		=    313 + 1,
        .vert_sync_clk 		=    1,//42,
        .vert_start_clk 	=    24,
        .vert_start1_clk 	=    25,
        .vert_pixel_width 	=    288,
#endif


#if  (DRIVE_MODE == YUV720x240_NTSC)
        //YUV720 720x240 two clock
        .hori_interval 		=    1716,//1716/2,
        .hori_sync_clk 		=    1,
        .hori_start_clk 	=    240,//240/2,
        .hori_pixel_width 	=    720,//1440/2,

        .vert_interval 		=    263,
        .vert_sync_clk 		=    1,//42,
        .vert_start_clk 	=    21,
        .vert_start1_clk 	=    23,
        .vert_pixel_width 	=    240,
#endif

#if (DRIVE_MODE == YUV720x288_PAL)
        //YUV720 720x288 two clock
        .hori_interval 		=    1728,//1728/2,
        .hori_sync_clk 		=    1,
        .hori_start_clk 	=    240,//240/2,
        .hori_pixel_width 	=    720,//1440/2,

        .vert_interval 		=    313 + 1,
        .vert_sync_clk 		=    1,//42,
        .vert_start_clk 	=    24,
        .vert_start1_clk 	=    25,
        .vert_pixel_width 	=    288,
#endif

#if (DRIVE_MODE == CCIR656_720x240_NTSC)
        .hori_interval 		=   1716,
        .hori_sync_clk 		=   268 - 4, //264
        .hori_start_clk 	=   268,
        .hori_pixel_width 	=   720,

        .vert_interval 		=   263,
        .vert_sync_clk 		=   1,
        .vert_start_clk 	=   21,
        .vert_start1_clk 	=   22,
        .vert_pixel_width 	=   240,
#endif

#if (DRIVE_MODE == CCIR656_720x288_PAL)
        //YUV720 720x288 two clock
        .hori_interval 		=    1728,
        .hori_sync_clk 		=    138 * 2, //276
        .hori_start_clk 	=    140 * 2, //280
        .hori_pixel_width 	=    720,

        .vert_interval 		=    313,
        .vert_sync_clk 		=    1,
        .vert_start_clk 	=    23,
        .vert_start1_clk 	=    24,
        .vert_pixel_width 	=    288,
#endif

#if (DRIVE_MODE == CCIR656_640x240_NTSC)
        不支持
        //CCIR656 640x240 two clock
        .hori_interval 		=    1556,
        .hori_sync_clk 		=    264,
        .hori_start_clk 	=    268,
        .hori_pixel_width 	=    640,

        .vert_interval 		=    263,
        .vert_sync_clk 		=    1,
        .vert_start_clk 	=    21,
        .vert_start1_clk 	=    22,
        .vert_pixel_width 	=    240,
#endif

#if (DRIVE_MODE == CCIR656_640x288_PAL)
        不支持
        //CCIR656 640x288 two clock
        .hori_interval 		=    1568,
        .hori_sync_clk 		=    276,
        .hori_start_clk 	=    280,
        .hori_pixel_width 	=    640,

        .vert_interval 		=    313,
        .vert_sync_clk 		=    1,
        .vert_start_clk 	=    23,
        .vert_start1_clk 	=    24,
        .vert_pixel_width 	=    288,
#endif
    },
};

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .init = lcd_ili8960_init,
    .type = LCD_DVP_RGB,
    .dev  = &lcd_ili8960_dev,
    .bl_ctrl = lcd_ili8960_backctrl,
};

#endif
