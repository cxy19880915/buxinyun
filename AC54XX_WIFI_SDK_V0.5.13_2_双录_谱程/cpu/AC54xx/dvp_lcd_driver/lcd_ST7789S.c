#include "asm/cpu.h"
#include "asm/imd.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#define SOFT_SIM

#ifdef LCD_ST7789S_MCU
//PG4
#define LCD_RST_OUT()    //gpio_direction_output(IO_PORTG_04, 0)
#define LCD_RST_H()      gpio_direction_output(IO_PORTG_07, 1)
#define LCD_RST_L()      gpio_direction_output(IO_PORTG_07, 0)

//PG5
#define LCD_CS_OUT()    //gpio_direction_output(IO_PORTG_05, 0)
#define LCD_CS_H()      gpio_direction_output(IO_PORTH_04, 1)
#define LCD_CS_L()      gpio_direction_output(IO_PORTH_04, 0)

//PH5
#define LCD_RS_OUT()    //gpio_direction_output(IO_PORTH_05, 0)
#define LCD_RS_H()      gpio_direction_output(IO_PORTH_02, 1)
#define LCD_RS_L()      gpio_direction_output(IO_PORTH_02, 0)

//PH3
#define LCD_WR_OUT()    //gpio_direction_output(IO_PORTH_03, 0)
#define LCD_WR_H()      gpio_direction_output(IO_PORTH_03, 1)
#define LCD_WR_L()      gpio_direction_output(IO_PORTH_03, 0)
//PH4
#define LCD_RD_OUT()    //gpio_direction_output(IO_PORTH_04, 0)
#define LCD_RD_H()      //gpio_direction_output(IO_PORTH_04,1)
#define LCD_RD_L()      //gpio_direction_output(IO_PORTH_04, 0)


#define delay2ms(t) delay_2ms(t)


static void LCD_Reset()
{
    printf("reset \n");
    /* LCD_RST_OUT(); */
    /* LCD_RS_OUT(); */
    /* LCD_CS_OUT(); */

    /* LCD_RD_OUT(); */
    /* LCD_WR_OUT(); */

    /* LCD_RD_H(); */
    /* LCD_WR_H(); */

    LCD_RST_H();
    delay2ms(50);
    LCD_RST_L();
    delay2ms(50);
    LCD_RST_H();
    delay2ms(50);
}

unsigned int LCD_data_table[] = {IO_PORTG_08,IO_PORTG_09,IO_PORTG_10,IO_PORTG_11,IO_PORTG_12,IO_PORTG_13,IO_PORTG_14,IO_PORTG_15};
//unsigned int LCD_data_table[] = {IO_PORTG_15,IO_PORTG_14,IO_PORTG_13,IO_PORTG_12,IO_PORTG_11,IO_PORTG_10,IO_PORTG_09,IO_PORTG_08};


u8 is_curr_bit_1(u8 value, u8 n)
{

	if((((value>>n)&1)==0))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void mcu_write (u8 data)
{
    u16 temp = 0;
    u16 dat;
	u8 i = 0;

	for(i = 0; i < 8; i++)
	{
		if(is_curr_bit_1(data, i))
		{
			gpio_direction_output(LCD_data_table[i], 1);
		}
		else
		{
			gpio_direction_output(LCD_data_table[i], 0);
		}
	}
}



void Trans_Com(u16 com) //0xaa aa
{
    u8 temp = 0;

    LCD_CS_L();
    temp = (com>>8);
    LCD_RS_L();//写地址

    mcu_write(temp);
    LCD_WR_L();
    LCD_WR_H();

    //delay(10);
    temp = (com);
    mcu_write(temp);
    LCD_WR_L();
    LCD_WR_H();
    LCD_CS_H();
}
void Trans_Dat(u16 dat)
{
    u8 temp = 0;

    LCD_CS_L();
    temp = (dat>>8);
    LCD_RS_H();//写数??

    mcu_write(temp);
    LCD_WR_L();
    LCD_WR_H();

    temp = (dat);
    mcu_write(temp);
    LCD_WR_L();
    LCD_WR_H();
    LCD_CS_H();
}

void tft_write_cmd(u8 com) //0xaa aa
{
    u8 temp = 0;

    LCD_CS_L();
    temp = com;
    LCD_RS_L();//写地址

    mcu_write(temp);
    LCD_WR_L();
    LCD_WR_H();

    LCD_CS_H();
}
void tft_write_data(u8 dat)
{
    u8 temp = 0;

    LCD_CS_L();
    temp = dat;
    LCD_RS_H();//写数??

    mcu_write(temp);
    LCD_WR_L();
    LCD_WR_H();

    LCD_CS_H();
}
void tft_SetRange(u16 x0, u16 x1, u16 y0, u16 y1)
{
    tft_write_cmd(0x2a);
	tft_write_data(x0>>8);	        //??λ???8λ
	tft_write_data(x0&0xff);        //??λ???8λ
	tft_write_data(x1>>8);	        //??λ???8λ
	tft_write_data(x1&0xff);        //??λ???8λ

    tft_write_cmd(0x2b);
	tft_write_data(y0>>8);	        //??λ???8λ
	tft_write_data(y0&0xff);        //??λ???8λ
	tft_write_data(y1>>8);	        //??λ???8λ
	tft_write_data(y1&0xff);        //??λ???8λ

    tft_write_cmd(0x2c);
    LCD_CS_L();
    LCD_RS_H();
}

extern void clr_wdt();

void LCD_Clear(u16 color)
{
    u32 i,j;

    for(i=0;i<320*240;i++)
    {
        /* for(j = 0; j < 240; j++) */
        {
            tft_write_data(0xf8);
            tft_write_data(0x00);
        }

    }
}

static void LCD_R69419_init(void *_data)
{
	u16 para = 0;
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    printf("R69419 initial....................................\n");

    LCD_Reset();

	delay_2ms(25); /* delay 50 ms */
	delay_2ms(25); /* delay 50 ms */
	
	#if 1 //7789
	tft_write_cmd(0x11);
    delay_2ms(120); //Delay 1
    //---------------------
    tft_write_cmd (0x36);
    tft_write_data (0x60);
    tft_write_cmd( 0x3a);
    tft_write_data (0x05);

    //tft_write_cmd( 0x34); 
    #if 1
    tft_write_cmd( 0x35);
    tft_write_data( 0x00);

    tft_write_cmd( 0x44);
    tft_write_data( 0x00);
    tft_write_data (0x00);
    #endif
    //---------------------

	
    tft_write_cmd (0xb2);
    #if 1  //????JFK??
    tft_write_data (0x0c);
    tft_write_data (0x0c);
    tft_write_data (0x00);
    tft_write_data (0x33);
    tft_write_data (0x33);
    #else
    tft_write_data (0x3f);
    tft_write_data (0x7f);
    tft_write_data (0x00);
    tft_write_data (0x0a);
    tft_write_data (0x14);
    #endif
    
    tft_write_cmd (0xb7);
    tft_write_data (0x35);
    //---------------------
    tft_write_cmd (0xbb);
    tft_write_data (0x1C); 
    
    tft_write_cmd (0xc0);
    tft_write_data (0x2c);
    
    tft_write_cmd (0xc2);
    tft_write_data (0x01);
    
    tft_write_cmd (0xc3);
    tft_write_data (0x0b);
    
    tft_write_cmd (0xc4);
    tft_write_data (0x20);
    
    tft_write_cmd (0xc6);
    tft_write_data (0x0f);
    
    tft_write_cmd (0xd0);
    tft_write_data (0xa4);
    tft_write_data (0xa1);
    //---------------------
    tft_write_cmd(0xE0);
    tft_write_data(0xd0);
    tft_write_data(0x00);
    tft_write_data(0x03);
    tft_write_data(0x09);
    tft_write_data(0x13);
    tft_write_data(0x1c);
    tft_write_data(0x3a);
    tft_write_data(0x55);
    tft_write_data(0x48);
    tft_write_data(0x18);
    tft_write_data(0x12);
    tft_write_data(0x0e);
    tft_write_data(0x19);
    tft_write_data(0x1e);

    tft_write_cmd(0xE1);
    tft_write_data(0xd0);
    tft_write_data(0x00);
    tft_write_data(0x03);
    tft_write_data(0x09);
    tft_write_data(0x05);
    tft_write_data(0x25);
    tft_write_data(0x3a);
    tft_write_data(0x55);
    tft_write_data(0x50);
    tft_write_data(0x3d);
    tft_write_data(0x1c);
    tft_write_data(0x1d);
    tft_write_data(0x1d);
    tft_write_data(0x1e);
    
    tft_write_cmd (0x29);
    delay_2ms(120);
	tft_write_cmd(0x2c);
	tft_SetRange(0, 319, 0, 239);
	#endif
	
	#if 0 //9341
	tft_write_cmd(0xcf);
	tft_write_data(0x00);
	tft_write_data(0xc8);
	tft_write_data(0x30);

	tft_write_cmd(0xed);
	tft_write_data(0x64);
	tft_write_data(0x03);
	tft_write_data(0x12);
	tft_write_data(0x81);

	tft_write_cmd(0xe8);
	tft_write_data(0x85);
	tft_write_data(0x10);
	tft_write_data(0x79);

	tft_write_cmd(0xcb);
	tft_write_data(0x39);
	tft_write_data(0x2c);
	tft_write_data(0x00);
	tft_write_data(0x34);
	tft_write_data(0x02);

	tft_write_cmd(0xf7);
	tft_write_data(0x20);

	tft_write_cmd(0xea);
	tft_write_data(0x00);
	tft_write_data(0x00);


    tft_write_cmd(0xb5);//v bank
	tft_write_data(0x3f);//0x02
	tft_write_data(0x7f);//0x02
	tft_write_data(0x0a);//0x0a
	tft_write_data(0x14);//0x14
	
	tft_write_cmd(0xb6);
	tft_write_data(0x0a);
	tft_write_data(0xa2);

	tft_write_cmd(0x2a);
	tft_write_data(0x00);
	tft_write_data(0x00);
	tft_write_data(0x01);
	tft_write_data(0x3f);

	tft_write_cmd(0x2b);
	tft_write_data(0x00);
	tft_write_data(0x00);
	tft_write_data(0x00);
	tft_write_data(0xef);

	tft_write_cmd(0xc0);
	tft_write_data(0x3e);

	tft_write_cmd(0xc1);
	tft_write_data(0x10);

	tft_write_cmd(0xc5);
	tft_write_data(0x2a);
	tft_write_data(0x3f);

	tft_write_cmd(0xc7);
	tft_write_data(0xb5);

	tft_write_cmd(0x36);

	tft_write_data(0x68);
    
	tft_write_cmd(0x3a);
	tft_write_data(0x05);
    #if 0
    tft_write_cmd(0xC0);    //Power control 
    tft_write_data(0x21);   //VRH[5:0] 
     
    tft_write_cmd(0xC1);    //Power control 
    tft_write_data(0x01);   //SAP[2:0];BT[3:0] 
    #endif 
    tft_write_cmd(0xC5);    //VCM control 
    tft_write_data(0x2a); 
    tft_write_data(0x2f); 
     
    tft_write_cmd(0xC7);    //VCM control2 
    tft_write_data(0xB8);  
    
	tft_write_cmd(0xb0);
    tft_write_data (0x00);
    tft_write_data (0x00);
    tft_write_data (0x7f); //??????????
    tft_write_data (0x7f); //??????????

    tft_write_cmd(0xb1);
    tft_write_data (0x00);
    tft_write_data (0x7f);

	tft_write_cmd(0xf6);
	tft_write_data(0x01);
	tft_write_data(0x30);
//	tft_write_data(0x06);

	tft_write_cmd(0xf2);
	tft_write_data(0x00);

	tft_write_cmd(0x26);
	tft_write_data(0x01);
/*
	tft_write_cmd(0x0c);
	tft_write_data(0x00);
	tft_write_data(0x50);

	tft_write_cmd(0x0e);
	tft_write_data(0x00);
	tft_write_data(0x3c);
*/
	tft_write_cmd(0xe0);
	tft_write_data(0x0f);
	tft_write_data(0x1a);
	tft_write_data(0x12);
	tft_write_data(0x0a);
	tft_write_data(0x0d);
	tft_write_data(0x0f);
	tft_write_data(0x50);
	tft_write_data(0x67);
	tft_write_data(0x40);
	tft_write_data(0x09);
	tft_write_data(0x15);
	tft_write_data(0x0f);
	tft_write_data(0x15);
	tft_write_data(0x15);
	tft_write_data(0x0f);

	tft_write_cmd(0xe1);
	tft_write_data(0x00);
	tft_write_data(0x17);
	tft_write_data(0x11);
	tft_write_data(0x05);
	tft_write_data(0x12);
	tft_write_data(0x05);
	tft_write_data(0x37);
	tft_write_data(0x13);
	tft_write_data(0x47);
	tft_write_data(0x04);
	tft_write_data(0x0c);
	tft_write_data(0x05);
	tft_write_data(0x33);
	tft_write_data(0x36);
	tft_write_data(0x0f);

	tft_write_cmd(0x35);
	tft_write_data(0x00);

	tft_write_cmd(0x11);

    delay_2ms(120);

	tft_write_cmd(0x29);
    delay_2ms(10);    
    tft_write_cmd(0x2c);

	#endif

    LCD_CS_L();
    LCD_RS_H();
}


/******************************************
lcd_R69419接线说明
CS  -> PF12
SDA -> PF13
SCL -> PF14
HSYNC  -> PH4
VSYNC  -> PG4
DCLK   -> PH5
DB7 -> R5(PE7)
DB6 -> R4(PF7)
DB5 -> R3(PC0)
DB4 -> R2(PD0)
DB3 -> G7(PE2)
DB2 -> G6(PF2)
DB1 -> G5(PE3)
DB0 -> G4(PF3)
*******************************************/



static void LCD_R69419_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    if (on)
    {
    	imd_dmm_con &= ~BIT(5);
        delay_2ms(100);
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    }
    else
    {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}


REGISTER_LCD_DEVICE(LCD_R69419_dev) =
{
    .info = {
        .test_mode       = 0,
        .color           = 0x00ff00,
/*         .xres            = 240, */
        /* .yres            = 320, */
        .xres            = 320,
        .yres            = 240,
       .buf_num         = LCD_DEV_BNUM,
        .buf_addr        = LCD_DEV_BUF,
        .sample          = SAMP_YUV444,
        .len             = LEN_256,
        .itp_mode_en     = FALSE,

        .rotate_en       = false,        // 旋转使能
        .hori_mirror_en  = false,        // 水平镜像使能
        .vert_mirror_en  = false,       // 垂直镜像使能

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

    .dclk_set        = /*CLK_EN |*/CLK_NEGATIVE,//clk
    .sync0_set       =CLK_EN | SIGNAL_DEN  ,//wr
    /* .sync0_set       =CLK_EN   | CLK_NEGATIVE,//wr */
    .sync1_set       = /*CLK_EN|*/SIGNAL_HSYNC/*| CLK_NEGATIVE*/,//rs
    .sync2_set       = /*CLK_EN|*/SIGNAL_VSYNC /*| CLK_NEGATIVE*/,//vs

    .port_sel        = PORT_GROUP_AA,
#if 1
    .clk_cfg         = PLL2_CLK | DIVA_1 | DIVB_4 | DIVC_2,
#else
    .clk_cfg         = IMD_INTERNAL | OSC_32K | PLL1_CLK | DIVA_3 | DIVB_1 | DIVC_1,
    .pll1_nr         = 3662,/* 3662*0.032768=120M */
    .pll1_nf         = 1,
#endif

    .timing = {
       
		#if 0
        .hori_interval      =320*2+60, //406,//456,//400 * 2 + 30,
        .hori_sync_clk      = 20,
        .hori_start_clk     = 20,
        .hori_pixel_width   = 320,//400,

        .vert_interval      = 240+60,//400,
        .vert_sync_clk      = 20,
        .vert_start_clk     = 21,
        .vert_start1_clk    = 12,
        .vert_pixel_width   = 240,//360//,
        #else
		.hori_interval      =320*2+60, //406,//456,//400 * 2 + 30,
        .hori_sync_clk      = 20,
        .hori_start_clk     = 20,
        .hori_pixel_width   = 320,//400,

        .vert_interval      = 240+60,//400,
        .vert_sync_clk      = 20,
        .vert_start_clk     = 21,
        .vert_start1_clk    = 12,
        .vert_pixel_width   = 240,//360//,
		#endif
   },
};

REGISTER_LCD_DEVICE_DRIVE(dev)  =
{
    .init = LCD_R69419_init,
    .type = LCD_DVP_MCU,
    .dev  = &LCD_R69419_dev,
    /* .setxy = tft_SetRange, */
    .bl_ctrl = LCD_R69419_backctrl,
};

#endif
