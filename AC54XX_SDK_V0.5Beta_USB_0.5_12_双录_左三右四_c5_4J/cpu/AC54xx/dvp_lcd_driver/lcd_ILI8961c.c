#include "asm/cpu.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_ILI8961C

extern void delay_2ms(u32 cnt);


#define LCD_CS_OUT()    gpio_direction_output(IO_PORTH_03,1)
#define LCD_CS_DIS()    gpio_direction_output(IO_PORTH_03,1)
#define LCD_CS_EN()     gpio_direction_output(IO_PORTH_03,0)

#define LCD_SDA_OUT()   gpio_direction_output(IO_PORTG_06,0);gpio_set_pull_up(IO_PORTG_06,1)
#define LCD_SDA_IN()    gpio_direction_input(IO_PORTG_06)
#define LCD_SDA_H()     gpio_direction_output(IO_PORTG_06,1)
#define LCD_SDA_L()     gpio_direction_output(IO_PORTG_06,0)
#define LCD_SDA_R()     gpio_read(IO_PORTG_06)

#define LCD_SCL_OUT()   gpio_direction_output(IO_PORTG_07,0);gpio_set_pull_up(IO_PORTG_07,1)
#define LCD_SCL_H()     gpio_direction_output(IO_PORTG_07,1)
#define LCD_SCL_L()     gpio_direction_output(IO_PORTG_07,0)

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

#define DRIVE_MODE                      RGB_8BIT_DUMMY_320x240_NTSC//  RGB_8BIT  //

#define INTERLACED_TEST
#define Delta_Color_Filter_Test

u8 reg_tab[]=
{
    0x00,0x01,0x03,0x04,0x06,0x07,0x08,0x0B,
    0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,
    0x16,0x17,0x18,0x19,0x1A,0x2B,0x2f,0x55,0x5a,0x56,0x57,
    0x61,
};

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
        delay(100*5);//50ns
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


static void SPI_WriteComm(u8 reg)
{
	u8 i;
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
	
	LCD_CS_DIS();
    delay_50ns(8);
}

static void SPI_WriteData(u8 dat)
{
	u8 i;
	LCD_CS_EN();
    LCD_SDA_OUT();
    delay_50ns(1);
	
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

static void spi_write_cmd(u8 cmd)
{	u8 i;
	LCD_CS_EN();	
	LCD_SCL_L();	
	LCD_SDA_OUT();	
	LCD_SDA_L();//cmd	
	delay_us(100);	
	LCD_SCL_H();	
	delay_us(100);	
for(i=0;i<8;i++)	{		
	LCD_SCL_L();		
	delay_us(1);		
	if(cmd&BIT(7))			
		LCD_SDA_H();		
	else			
		LCD_SDA_L();		
	cmd <<= 1;		
	LCD_SCL_H();		
	delay_us(200);	
	}	
LCD_CS_DIS();	
delay_us(100);
}
static void spi_write_dat(u8 dat)
{	
	u8 i;
	LCD_CS_EN();	
	LCD_SCL_L();	
	LCD_SDA_OUT();	
	LCD_SDA_H();//dat	
	delay_us(100);	
	LCD_SCL_H();    
	delay_us(100);	
for(i=0;i<8;i++)	{
	LCD_SCL_L();		
	delay_us(1);		
	if(dat&BIT(7))			
		LCD_SDA_H();		
	else			
		LCD_SDA_L();		
	dat <<= 1;		
	LCD_SCL_H();		
	delay_us(500);	
	}	
	LCD_CS_DIS();	
	delay_us(100);
}
 
#if 0
static void lcd_ili8961c_init(void *_data)
{
    u8 i;
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    printf("ILI8961C initial...\n");

    lcd_io_init();


    wr_reg(0x2B,0x01);   //
	wr_reg(0x00,0x08);  //
	wr_reg(0x01,0x90);  //wr_reg(0x0192);  //wr_reg(0x0183);  //可以设置wr_reg(0x0181-wr_reg(0x0186 条纹代码
	wr_reg(0x04,0x18);  //8-bit RGB interface);如果显示异常，可能是方案要匹配，可以设置0418 0408 041B  040B
	wr_reg(0x06,0x15);  //
	wr_reg(0x07,0x46);  //
	wr_reg(0x08,0x00);  //
	wr_reg(0x0B,0x83);  //
	wr_reg(0x0C,0x06);  //
	wr_reg(0x0D,0x46);  //contrast setting 对比度，可以考虑设置
	wr_reg(0x0E,0x40);  //
	wr_reg(0x0F,0x40);  //
	wr_reg(0x10,0x40);  //
	wr_reg(0x11,0x40);  //
	wr_reg(0x95,0x80);  //
	wr_reg(0x12,0x00);  //
//	wr_reg(0x13,0x01);  //Delta color filter  只有全志才要打开此代码
	wr_reg(0x16,0x00);  //gamma
	wr_reg(0x17,0x52);  //gamma
	wr_reg(0x18,0x77);  //gamma
	wr_reg(0x19,0x75);  //gamma
	wr_reg(0x1A,0x77);  //gamma
	wr_reg(0x3C,0x77);  //gamma
	wr_reg(0x2F,0x69);  //gamma
	wr_reg(0xA5,0x22);  //gamma 可以不要
	wr_reg(0xA6,0x12);  //gamma 可以不要
	wr_reg(0xA7,0x06);  //gamma 可以不要
	wr_reg(0xAC,0xD0);  //gamma 可以不要



#if (DRIVE_MODE == RGB_8BIT)
    wr_reg(0x00,0x09);
    wr_reg(0x04,0x0f);//8-bit RGB
    wr_reg(0x06,0x15);
    wr_reg(0x07,70);
    wr_reg(0x0c,0x46);
#elif (DRIVE_MODE == RGB_8BIT_DUMMY_320x240_NTSC)
    wr_reg(0x00,0x09);
    wr_reg(0x04,0x1f);//8-bit RGB dummy  320*240 NTSC
    wr_reg(0x06,0x15);
    wr_reg(0x07,240);
    wr_reg(0x0c,0x46);
    wr_reg(0x0c,0x46);
#endif
 for(i=0;i<sizeof(reg_tab)/sizeof(reg_tab[0]);i++)
    {
        u8 reg = reg_tab[i];
        reg &=~BIT(7);
        reg |= (reg&BIT(6))<<1;
        reg |= BIT(6);
        printf("\nreg:%02x val:%02x %02x",reg_tab[i],rd_reg(reg),reg);
    }
    printf("ILI8961C initial end\n");
}
#endif

//#else
static void lcd_ili8961c_init(void *_data)
{
    u8 i;
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    //printf("ILI8961C initial...\n");

    lcd_io_init();
    gpio_direction_output(lcd_reset,1);
	delay_2ms(10);
	gpio_direction_output(lcd_reset,0);
	delay_2ms(10);
	gpio_direction_output(lcd_reset,1);
	delay_2ms(10);
	
	spi_write_cmd(0x01);
    delay_2ms(120);

	spi_write_cmd(0xC8);
	spi_write_dat(0xFF);
	spi_write_dat(0x93);
	spi_write_dat(0x42);

	spi_write_cmd(0xC0);
	spi_write_dat(0x0F);//0F
	spi_write_dat(0x0F);//0F

	spi_write_cmd(0xC1);
	spi_write_dat(0x01);//01

	spi_write_cmd(0xC5);
	spi_write_dat(0xDB);//DB

	spi_write_cmd(0x36);
	spi_write_dat(0x08);     

	spi_write_cmd(0x3A);
	spi_write_dat(0x66);     //18BIT PIX FOMART

	spi_write_cmd(0xB0);
	spi_write_dat(0xE0);

	spi_write_cmd(0xB4);
	spi_write_dat(0x02);//02  inversion

	spi_write_cmd(0xB7);
	spi_write_dat(0x07);

	spi_write_cmd(0xF6);
	spi_write_dat(0x01);//01
	spi_write_dat(0x00);
	spi_write_dat(0x07);

	spi_write_cmd(0xE0); //Set Gamma
	spi_write_dat(0x00);
	spi_write_dat(0x05);
	spi_write_dat(0x08);
	spi_write_dat(0x02);
	spi_write_dat(0x1A);
	spi_write_dat(0x0C);
	spi_write_dat(0x42);
	spi_write_dat(0x7A);
	spi_write_dat(0x54);
	spi_write_dat(0x08);
	spi_write_dat(0x0D);
	spi_write_dat(0x0C);
	spi_write_dat(0x23);
	spi_write_dat(0x25);
	spi_write_dat(0x0F);

	spi_write_cmd(0xE1); //Set Gamma
	spi_write_dat(0x00);
	spi_write_dat(0x29);
	spi_write_dat(0x2F);
	spi_write_dat(0x03);
	spi_write_dat(0x0F);
	spi_write_dat(0x05);
	spi_write_dat(0x42);
	spi_write_dat(0x55);
	spi_write_dat(0x53);
	spi_write_dat(0x06);
	spi_write_dat(0x0F);
	spi_write_dat(0x0C);
	spi_write_dat(0x38);
	spi_write_dat(0x3A);
	spi_write_dat(0x0F);

	spi_write_cmd(0x20);

	spi_write_cmd(0x11); //Exit Sleep
     delay_2ms(120);
	spi_write_cmd(0x29); //Display on

	 delay_2ms(120);
    printf("ILI8961C initial end\n");
}
//#endif


/******************************************
ILI8961C接线说明
CS	-> PH3
SDA	-> PG6
SCL	-> PG7
HSYNC  -> PH4
VSYNC  -> PH5
DCLK   -> PH2
DB7	-> R5(PG15)
DB6	-> R4(PG14)
DB5	-> R3(PG13)
DB4	-> R2(PG12)
DB3	-> G7(PG11)
DB2	-> G6(PG10)
DB1	-> G5(PG9)
DB0	-> G4(PG8)
*******************************************/


static void lcd_ili8961c_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        imd_dmm_con &= ~BIT(5);
        delay_2ms(100);
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE(lcd_ili8961c_dev) = {
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
        },
    },
#if (DRIVE_MODE == YUV720x240_NTSC)||\
	(DRIVE_MODE == YUV640x240_NTSC)||\
	(DRIVE_MODE == YUV720x288_PAL)||\
	(DRIVE_MODE == YUV640x288_PAL)
    .ncycle          = CYCLE_TWO,
#ifdef INTERLACED_TEST
    .interlaced_mode = INTERLACED_NONE,
#else
    .interlaced_mode = INTERLACED_NONE,
#endif
    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    //.sync0_set       = SIGNAL_DEN/*|CLK_EN|CLK_NEGATIVE*/,
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
    .interlaced_mode = INTERLACED_NONE,
#else
    .interlaced_mode = INTERLACED_NONE,
#endif

    .dclk_set        = CLK_EN | CLK_NEGATIVE,
    //.sync0_set       = SIGNAL_DEN/*|CLK_EN|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
#endif

    .drive_mode      = MODE_RGB_SYNC,
    .data_width      = PORT_8BITS,
    .port_sel        = PORT_GROUP_AA,
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
    .init = lcd_ili8961c_init,
    .type = LCD_DVP_RGB,
    .dev  = &lcd_ili8961c_dev,
    .bl_ctrl = lcd_ili8961c_backctrl,
};



#endif
