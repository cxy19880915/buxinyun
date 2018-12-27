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
#define LCD_RST_H()      gpio_direction_output(IO_PORTH_01, 1)
#define LCD_RST_L()      gpio_direction_output(IO_PORTH_01, 0)

//PG5
#define LCD_CS_OUT()    //gpio_direction_output(IO_PORTG_05, 0)
#define LCD_CS_H()      gpio_direction_output(IO_PORTH_02, 1)
#define LCD_CS_L()      gpio_direction_output(IO_PORTH_02, 0)

//PH5
#define LCD_RS_OUT()    //gpio_direction_output(IO_PORTH_05, 0)
#define LCD_RS_H()      gpio_direction_output(IO_PORTH_05, 1)
#define LCD_RS_L()      gpio_direction_output(IO_PORTH_05, 0)

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


void Init_data(u16 com,u16 dat)
{
	Trans_Com(com);
	Trans_Dat(dat);
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
#if 0
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
#endif
           Init_data(0x0210, 0x0000);	//X_Start);	//x start
	Init_data(0x0211, 0x00ef);	//X_End);		//x end
	Init_data(0x0212, 0x0000);	//Y_Start);	//y start
	Init_data(0x0213, 0x018f);	//Y_End);		//y end
	Init_data(0x0201, 0x0000);	//Y_Start);	//y addres
	Init_data(0x0200, 0x0000);	//X_Start);	//x addres

    Trans_Com(0x0202);

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

    printf("R69419 initial....111111................................\n");

    LCD_Reset();

	delay_2ms(25); /* delay 50 ms */
	delay_2ms(25); /* delay 50 ms */

	//--------------- Normal set ---------------//
	Init_data(0x0000, 0x0000); 
	Init_data(0x0001, 0x0100); 
	Init_data(0x0002, 0x0100); 
	Init_data(0x0003, 0x1030); 
	Init_data(0x0008, 0x0808); 
	Init_data(0x0009, 0x0001); 
	Init_data(0x000B, 0x0010); 
	Init_data(0x000C, 0x0000); 
	Init_data(0x000F, 0x0000); 
	Init_data(0x0007, 0x0001); 
//--------------- Panel interface control 1~6 ---------------//

	Init_data(0x0010, 0x0011);	//0x0012 0010
	Init_data(0x0011, 0x0200);	//0x0202
	Init_data(0x0012, 0x0300);	//
	Init_data(0x0020, 0x021E);	//
	Init_data(0x0021, 0x0202);	//
	Init_data(0x0022, 0x0100);	//
	Init_data(0x0090, 0x8000);	//


//--------------- Power control 1~6 ---------------//
        Init_data(0x0100, 0x17B0); 
	Init_data(0x0101, 0x0147);
	Init_data(0x0102, 0x0138);

	Init_data(0x0103, 0x3000);

	Init_data(0x0107, 0x0000);
	Init_data(0x0110, 0x0001);
	Init_data(0x0280, 0x0000); // NVM write / read
	Init_data(0x0281, 0x0000); // Vcom high voltage 1//0000
	Init_data(0x0282, 0x0000); // Vcom high voltage 2


//--------------- Gamma 2.2 control ---------------//


    Init_data(0x0300, 0x1011);	//0x0101	
	Init_data(0x0301, 0x2524);	//0x0024
	Init_data(0x0302, 0x2F20);	//0x1326
	Init_data(0x0303, 0x202F);	//0x2313
	Init_data(0x0304, 0x2325);	//0x2400
	Init_data(0x0305, 0x1110);	//0x0100
	Init_data(0x0306, 0x0e04);	//0x1704
	Init_data(0x0307, 0x040e);	//0x0417
	Init_data(0x0308, 0x0005);	//0x0007
	Init_data(0x0309, 0x0003);	//0x0105
	Init_data(0x030A, 0x0F04);	//0x0F05
	Init_data(0x030B, 0x0F00);	//0x0F01
	Init_data(0x030C, 0x000F);	//0x010F
	Init_data(0x030D, 0x040F);	//0x050F
	Init_data(0x030E, 0x0300);	//0x0501
	Init_data(0x030F, 0x0500);	//0x0700
	Init_data(0x0400, 0x3500);	//0x3100
	Init_data(0x0401, 0x0001);	//0x0001
	Init_data(0x0404, 0x0000);	//0x0000



//--------------- Partial display ---------------//
	Init_data(0x0201, 0x0000);
	Init_data(0x0200, 0x0000);
	Init_data(0x0210, 0x0000); 
	Init_data(0x0211, 0x00EF); 
	Init_data(0x0212, 0x0000); 
	Init_data(0x0213, 0x018F);
 
	Init_data(0x0500, 0x0000);
	Init_data(0x0501, 0x0000);
	Init_data(0x0502, 0x0000);
	Init_data(0x0503, 0x0000);
	Init_data(0x0504, 0x0000);
	Init_data(0x0505, 0x0000);
	Init_data(0x0600, 0x0000); 
	Init_data(0x0606, 0x0000); 
	Init_data(0x06F0, 0x0000); 
//--------------- Orise mode ---------------//
	Init_data(0x07F0, 0x5420);
	Init_data(0x07de, 0x0000);
	Init_data(0x07F2, 0x00df);
	Init_data(0x07F3, 0x088e);	//0x288A
	Init_data(0x07F4, 0x0022);
	Init_data(0x07F5, 0x0001);	//
	Init_data(0x07F0, 0x0000);
	Init_data(0x0007, 0x0173); // Display on

	delay_2ms(150);
	

    LCD_CS_L();
    LCD_RS_H();
}

void LCD_rotate(int rot)
{
    //ui_lcd_light_off();
  	
    if(rot)
    {
    	imb_rot_con |= BIT(1); 
	imb_rot_con |= BIT(2);
    }else{
    	imb_rot_con &= ~BIT(1); 
	imb_rot_con &= ~BIT(2);
    }
	//ui_lcd_light_on();
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
       /*  .xres            = 240, */
       /*  .yres            = 320, */
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
	#if 0
	.te_mode = {
			.te_mode_en = true,
			/* .te_mode_en = false, */
			.event = EVENT_IO_2,
			.edge = EDGE_POSITIVE,
			/* .edge = EDGE_NEGATIVE, */
			.gpio = IO_PORTH_01,
		},
	#endif
    .drive_mode      = MODE_MCU,
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_8BITS,
    //.hook = tft_SetRange,
    .interlaced_mode = INTERLACED_NONE,

    .dclk_set        = /*CLK_EN |*/CLK_NEGATIVE,//clk
    .sync0_set       =CLK_EN | SIGNAL_DEN  ,//wr
    /* .sync0_set       =CLK_EN   | CLK_NEGATIVE,//wr */
    .sync1_set       = /*CLK_EN|*/SIGNAL_HSYNC/*| CLK_NEGATIVE*/,//rs
    .sync2_set       = /*CLK_EN|*/SIGNAL_VSYNC /*| CLK_NEGATIVE*/,//vs

    .port_sel        = PORT_GROUP_AA,
#if 1
    .clk_cfg         = PLL2_CLK | DIVA_1 | DIVB_2 | DIVC_3,
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
     .setxy = tft_SetRange, 
    .bl_ctrl = LCD_R69419_backctrl,
   // .te_mode_dbug = true, 
};

#endif
