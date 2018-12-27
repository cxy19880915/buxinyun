#include "asm/cpu.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_9341_360x640

extern void delay_2ms(u32 cnt);

#define LCD_CS_OUT()    gpio_direction_output(IO_PORTH_03,1)
#define LCD_CS_DIS()    gpio_direction_output(IO_PORTH_03,1)
#define LCD_CS_EN()     gpio_direction_output(IO_PORTH_03,0)
#define SPI_CSX_L()     gpio_direction_output(IO_PORTH_03,0)
#define SPI_CSX_H()     gpio_direction_output(IO_PORTH_03,1)

#define LCD_SDA_OUT()   gpio_direction_output(IO_PORTE_05,1);gpio_set_pull_up(IO_PORTE_05,1)
#define SPI_SDA_OUT()   gpio_direction_output(IO_PORTE_05,1);gpio_set_pull_up(IO_PORTE_05,1)
#define LCD_SDA_IN()    gpio_direction_input(IO_PORTE_05)
#define LCD_SDA_H()     gpio_direction_output(IO_PORTE_05,1)
#define LCD_SDA_L()     gpio_direction_output(IO_PORTE_05,0)
#define LCD_SDA_R()     gpio_read(IO_PORTE_05)
#define SPI_SDA_H()     gpio_direction_output(IO_PORTE_05,1)
#define SPI_SDA_L()     gpio_direction_output(IO_PORTE_05,0)

#define LCD_SCL_OUT()   gpio_direction_output(IO_PORTE_04,1)
#define LCD_SCL_H()     gpio_direction_output(IO_PORTE_04,1)
#define LCD_SCL_L()     gpio_direction_output(IO_PORTE_04,0)
#define SPI_SCL_L()     gpio_direction_output(IO_PORTE_04,0)
#define SPI_SCL_H()     gpio_direction_output(IO_PORTE_04,1)

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

	for(i=0;i<8;i++)
	{
		SPI_SCL_L();
		delay_us(100);
		if(cmd&BIT(7))
		{
		    SPI_SDA_H();
		}
		else
		{
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

	for(i=0;i<8;i++)
	{
		SPI_SCL_L();
		delay_us(100);
		if(dat&BIT(7))
		{
		    SPI_SDA_H();
		}
		else
		{
		    SPI_SDA_L();
		}

		dat <<= 1;
		SPI_SCL_H();
		delay_us(500);
	}
	SPI_CSX_H();
	delay_us(100);
}
 

static void lcd_ili8961c_init(void *_data)
{
    u8 i;
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    gpio_direction_output(lcd_reset, 1);
    delay_2ms(20);
    gpio_direction_output(lcd_reset, 0);
    delay_2ms(20);
    gpio_direction_output(lcd_reset, 1);
    delay_2ms(20);
  
    //printf("ILI8961C initial...\n");

    lcd_io_init();

    
    delay_2ms(120);                //ms
#if 0
    spi_write_cmd( 0x11);   

    delay_2ms(120);
     
    spi_write_cmd( 0xFF); //System Function Command Table2   
    spi_write_dat( 0x77);
    spi_write_dat( 0x01);   
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);   
    spi_write_dat( 0x10); 

    delay_2ms(5);

    spi_write_cmd( 0xC0); //Display Line Setting   
    spi_write_dat( 0x4F); //640
    spi_write_dat( 0x00); 

    delay_2ms(5);

    spi_write_cmd( 0xC1); //Porch Control  
    spi_write_dat( 0x07); //VBP
    spi_write_dat( 0x02); //VFP

    delay_2ms(5);
     
    spi_write_cmd( 0xC2); //Inversion selection & Frame Rate Control  
    spi_write_dat( 0x31); 
    spi_write_dat( 0x05); 

    spi_write_cmd( 0xC3);   
    spi_write_dat( 0x80);//80 
	
	 spi_write_cmd( 0xC7); 
	spi_write_dat( 0x04);
    
    delay_2ms(5);
#if 0
    spi_write_cmd( 0xB0); //Positive Voltage Gamma Control  
    spi_write_dat( 0x00);   
    spi_write_dat( 0x0C);   
    spi_write_dat( 0x12);   
    spi_write_dat( 0x0D);   
    spi_write_dat( 0x11);   
    spi_write_dat( 0x05);   
    spi_write_dat( 0x01);   
    spi_write_dat( 0x08);   
    spi_write_dat( 0x07);   
    spi_write_dat( 0x1F);   
    spi_write_dat( 0x06);   
    spi_write_dat( 0x14);   
    spi_write_dat( 0x13);   
    spi_write_dat( 0xA8);   
    spi_write_dat( 0x2E);   
    spi_write_dat( 0x18); 
#endif
#if 0
	spi_write_cmd(0xB0); //Positive Voltage Gamma Control  
	spi_write_dat( 0x00);	
	spi_write_dat( 0x0A);	
	spi_write_dat( 0x11);	
	spi_write_dat( 0x0C);	
	spi_write_dat( 0x10);	
	spi_write_dat( 0x05);	
	spi_write_dat( 0x00);	
	spi_write_dat( 0x08);	
	spi_write_dat( 0x08);	
	spi_write_dat( 0x1F);	
	spi_write_dat( 0x07);	
	spi_write_dat( 0x13);	
	spi_write_dat( 0x10);	
	spi_write_dat( 0xA9);	
	spi_write_dat( 0x30);	
	spi_write_dat( 0x18);
#endif
    delay_2ms(5);
#if 1
    spi_write_cmd( 0xB1); //Negative Voltage Gamma Control  
    spi_write_dat( 0x00);   
    spi_write_dat( 0x0C);   
    spi_write_dat( 0x13);   
    spi_write_dat( 0x0D);   
    spi_write_dat( 0x10);   
    spi_write_dat( 0x06);   
    spi_write_dat( 0x03);   
    spi_write_dat( 0x08);   
    spi_write_dat( 0x09);   
    spi_write_dat( 0x1F);   
    spi_write_dat( 0x05);   
    spi_write_dat( 0x12);   
    spi_write_dat( 0x0F);   
    spi_write_dat( 0xA4);   
    spi_write_dat( 0x2C);   
    spi_write_dat( 0x18); 
#endif
	spi_write_cmd(0xB1); //Negative Voltage Gamma Control  
	spi_write_dat( 0x00);	
	spi_write_dat( 0x0B);	
	spi_write_dat( 0x11);	
	spi_write_dat( 0x0D);	
	spi_write_dat( 0x0F);	
	spi_write_dat( 0x05);	
	spi_write_dat( 0x02);	
	spi_write_dat( 0x07);	
	spi_write_dat( 0x06);	
	spi_write_dat( 0x20);	
	spi_write_dat( 0x05);	
	spi_write_dat( 0x15);	
	spi_write_dat( 0x13);	
	spi_write_dat( 0xA9);	
	spi_write_dat( 0x30);	
	spi_write_dat( 0x18); 


    delay_2ms(5);
     
    spi_write_cmd( 0xFF); //System Function Command Table2    
    spi_write_dat( 0x77);
    spi_write_dat( 0x01);   
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);   
    spi_write_dat( 0x11); 

    delay_2ms(5);
     
    spi_write_cmd( 0xB0); 
    spi_write_dat( 0x62);//62

    delay_2ms(5);
     
    spi_write_cmd( 0xB1); 
    spi_write_dat( 0x60);//60

    delay_2ms(5);
      
    spi_write_cmd( 0xB2); 
    spi_write_dat( 0x07);//07

    delay_2ms(5);

    spi_write_cmd( 0xB3); 
    spi_write_dat( 0x80);

    delay_2ms(5);

    spi_write_cmd( 0xB5); 
    spi_write_dat( 0x49);

    delay_2ms(5);

    spi_write_cmd( 0xB7); 
    spi_write_dat( 0x85);

    delay_2ms(5);

    spi_write_cmd( 0xB8);  
    spi_write_dat( 0x21); 

    delay_2ms(5);

    spi_write_cmd( 0xC1);
    spi_write_dat( 0x78);

    delay_2ms(5);

    spi_write_cmd( 0xC2); 
    spi_write_dat( 0x78);

    delay_2ms(100);
     
	
    spi_write_cmd( 0xE0); 
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);
    spi_write_dat( 0x02);

    delay_2ms(5);
     
    spi_write_cmd( 0xE1); 
    spi_write_dat( 0x03);
    spi_write_dat( 0xA0);
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);
    spi_write_dat( 0x02);
    spi_write_dat( 0xA0);
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);
    spi_write_dat( 0x33);
    spi_write_dat( 0x33);

    delay_2ms(5);
      
    spi_write_cmd( 0xE2); 
    spi_write_dat( 0x22);
    spi_write_dat( 0x22);
    spi_write_dat( 0x33);
    spi_write_dat( 0x33);
    spi_write_dat( 0x88);
    spi_write_dat( 0xA0);
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);
    spi_write_dat( 0x87);
    spi_write_dat( 0xA0);
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);

    delay_2ms(5);
      
    spi_write_cmd( 0xE3); 
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);
    spi_write_dat( 0x22);
    spi_write_dat( 0x22);

    delay_2ms(5);
     
    spi_write_cmd( 0xE4); 
    spi_write_dat( 0x44);
    spi_write_dat( 0x44);

    delay_2ms(5);
     
    spi_write_cmd( 0xE5); 
    spi_write_dat( 0x04);
    spi_write_dat( 0x84);
    spi_write_dat( 0xA0);
    spi_write_dat( 0xA0);
    spi_write_dat( 0x06);
    spi_write_dat( 0x86);
    spi_write_dat( 0xA0);
    spi_write_dat( 0xA0);
    spi_write_dat( 0x08);
    spi_write_dat( 0x88);
    spi_write_dat( 0xA0);
    spi_write_dat( 0xA0);
    spi_write_dat( 0x0A);
    spi_write_dat( 0x8A);
    spi_write_dat( 0xA0);
    spi_write_dat( 0xA0);

    delay_2ms(5);
      
    spi_write_cmd( 0xE6); 
    spi_write_dat( 0x00);
    spi_write_dat( 0x00);
    spi_write_dat( 0x22);
    spi_write_dat( 0x22);

    delay_2ms(5);
     
    spi_write_cmd( 0xE7); 
    spi_write_dat( 0x44);
    spi_write_dat( 0x44);

    delay_2ms(5);
     
    spi_write_cmd( 0xE8);  
    spi_write_dat( 0x03);
    spi_write_dat( 0x83);
    spi_write_dat( 0xA0);
    spi_write_dat( 0xA0);
    spi_write_dat( 0x05);
    spi_write_dat( 0x85);
    spi_write_dat( 0xA0); 
    spi_write_dat( 0xA0); 
    spi_write_dat( 0x07); 
    spi_write_dat( 0x87); 
    spi_write_dat( 0xA0); 
    spi_write_dat( 0xA0); 
    spi_write_dat( 0x09); 
    spi_write_dat( 0x89); 
    spi_write_dat( 0xA0); 
    spi_write_dat( 0xA0); 

    delay_2ms(5);
     
    spi_write_cmd( 0xEB); 
    spi_write_dat( 0x00);
    spi_write_dat( 0x01);
    spi_write_dat( 0xE4);
    spi_write_dat( 0xE4);
    spi_write_dat( 0x88);
    spi_write_dat( 0x00);
    spi_write_dat( 0x40);

    delay_2ms(5);
     
    spi_write_cmd( 0xEC); 
    spi_write_dat( 0x3C);
    spi_write_dat( 0x01);

    delay_2ms(5);

    spi_write_cmd( 0xED); 
    spi_write_dat( 0xAB);
    spi_write_dat( 0x89);
    spi_write_dat( 0x76);
    spi_write_dat( 0x54);
    spi_write_dat( 0x02);
    spi_write_dat( 0xFF);
    spi_write_dat( 0xFF);
    spi_write_dat( 0xFF);
    spi_write_dat( 0xFF);
    spi_write_dat( 0xFF);
    spi_write_dat( 0xFF);
    spi_write_dat( 0x20);
    spi_write_dat( 0x45);
    spi_write_dat( 0x67);
    spi_write_dat( 0x98);
    spi_write_dat( 0xBA);

    delay_2ms(5);

    
    
    spi_write_cmd( 0x36);
    spi_write_dat( 0x10);
    
    spi_write_cmd( 0x3A);
    spi_write_dat( 0x50);//55
    
    spi_write_cmd( 0x29); 
#else
spi_write_cmd(0x11);   

delay_2ms(120);
 
spi_write_cmd(0xFF); //System Function Command Table2   
spi_write_dat(0x77);
spi_write_dat(0x01);   
spi_write_dat(0x00);
spi_write_dat(0x00);   
spi_write_dat(0x10); 

delay_2ms(5);

spi_write_cmd(0xC0); //Display Line Setting   
spi_write_dat(0x4F); //6404f
spi_write_dat(0x01); 

delay_2ms(5);

spi_write_cmd(0xC1); //Porch Control  
spi_write_dat(0x07); //VBP
spi_write_dat(0x02); //VFP

delay_2ms(5);
 
spi_write_cmd(0xC2); //Inversion selection & Frame Rate Control  
spi_write_dat(0x31); 
spi_write_dat(0x05); 

spi_write_cmd( 0xC3);	
  spi_write_dat( 0x80);//80 
  spi_write_dat( 0x00);
   spi_write_dat( 0x00);
/*
 spi_write_cmd( 0xC7); 
spi_write_dat( 0x04);
*/
delay_2ms(5);
 
spi_write_cmd(0xB0); //Positive Voltage Gamma Control  
spi_write_dat( 0x00);   
spi_write_dat( 0x0A);   
spi_write_dat( 0x11);   
spi_write_dat( 0x0C);   
spi_write_dat( 0x10);   
spi_write_dat( 0x05);   
spi_write_dat( 0x00);   
spi_write_dat(0x08);   
spi_write_dat( 0x08);   
spi_write_dat( 0x1F);   
spi_write_dat( 0x07);   
spi_write_dat( 0x13);   
spi_write_dat( 0x10);   
spi_write_dat( 0xA9);   
spi_write_dat( 0x30);   
spi_write_dat( 0x18);

delay_2ms(5);
 
spi_write_cmd(0xB1); //Negative Voltage Gamma Control  
spi_write_dat(0x00);   
spi_write_dat(0x0B);   
spi_write_dat(0x11);   
spi_write_dat(0x0D);   
spi_write_dat(0x0F);   
spi_write_dat(0x05);   
spi_write_dat(0x02);   
spi_write_dat(0x07);   
spi_write_dat(0x06);   
spi_write_dat(0x20);   
spi_write_dat(0x05);   
spi_write_dat(0x15);   
spi_write_dat(0x13);   
spi_write_dat(0xA9);   
spi_write_dat(0x30);   
spi_write_dat(0x18); 

delay_2ms(5);
 
spi_write_cmd(0xFF); //System Function Command Table2    
spi_write_dat(0x77);
spi_write_dat(0x01);   
spi_write_dat(0x00);
spi_write_dat(0x00);   
spi_write_dat(0x11); 

delay_2ms(5);
 
spi_write_cmd(0xB0); 
spi_write_dat(0x53);
delay_2ms(5);
 
spi_write_cmd(0xB1); 
spi_write_dat(0x60);


delay_2ms(5);
  
spi_write_cmd(0xB2); 
spi_write_dat(0x07);

delay_2ms(5);

spi_write_cmd(0xB3); 
spi_write_dat(0x80);

delay_2ms(5);

spi_write_cmd(0xB5); 
spi_write_dat(0x40);//49

delay_2ms(5);

spi_write_cmd(0xB7); 
spi_write_dat(0x85);

delay_2ms(5);

spi_write_cmd(0xB8);  
spi_write_dat(0x33); //21

delay_2ms(5);

spi_write_cmd(0xC1);
spi_write_dat(0x78);

delay_2ms(5);

spi_write_cmd(0xC2); 
spi_write_dat(0x78);

delay_2ms(100);
 
spi_write_cmd(0xE0); 
spi_write_dat(0x00);
spi_write_dat(0x00);
spi_write_dat(0x02);

delay_2ms(5);
 
spi_write_cmd(0xE1); 
spi_write_dat(0x03);
spi_write_dat(0xA0);
spi_write_dat(0x00);
spi_write_dat(0x00);
spi_write_dat(0x02);
spi_write_dat(0xA0);
spi_write_dat(0x00);
spi_write_dat(0x00);
spi_write_dat(0x00);
spi_write_dat(0x33);
spi_write_dat(0x33);

delay_2ms(5);
  
spi_write_cmd(0xE2); 
spi_write_dat(0x22);
spi_write_dat(0x22);
spi_write_dat(0x33);
spi_write_dat(0x33);
spi_write_dat(0x88);
spi_write_dat(0xA0);
spi_write_dat(0x00);
spi_write_dat(0x00);
spi_write_dat(0x87);
spi_write_dat(0xA0);
spi_write_dat(0x00);
spi_write_dat(0x00);

delay_2ms(5);
  
spi_write_cmd(0xE3); 
spi_write_dat(0x00);
spi_write_dat(0x00);
spi_write_dat(0x22);
spi_write_dat(0x22);

delay_2ms(5);
 
spi_write_cmd(0xE4); 
spi_write_dat(0x44);
spi_write_dat(0x44);

delay_2ms(5);
 
spi_write_cmd(0xE5); 
spi_write_dat(0x04);
spi_write_dat(0x84);
spi_write_dat(0xA0);
spi_write_dat(0xA0);
spi_write_dat(0x06);
spi_write_dat(0x86);
spi_write_dat(0xA0);
spi_write_dat(0xA0);
spi_write_dat(0x08);
spi_write_dat(0x88);
spi_write_dat(0xA0);
spi_write_dat(0xA0);
spi_write_dat(0x0A);
spi_write_dat(0x8A);
spi_write_dat(0xA0);
spi_write_dat(0xA0);

delay_2ms(5);
  
spi_write_cmd(0xE6); 
spi_write_dat(0x00);
spi_write_dat(0x00);
spi_write_dat(0x22);
spi_write_dat(0x22);

delay_2ms(5);
 
spi_write_cmd(0xE7); 
spi_write_dat(0x44);
spi_write_dat(0x44);

delay_2ms(5);
 
spi_write_cmd(0xE8);  
spi_write_dat(0x03);
spi_write_dat(0x83);
spi_write_dat(0xA0);
spi_write_dat(0xA0);
spi_write_dat(0x05);
spi_write_dat(0x85);
spi_write_dat(0xA0); 
spi_write_dat(0xA0); 
spi_write_dat(0x07); 
spi_write_dat(0x87); 
spi_write_dat(0xA0); 
spi_write_dat(0xA0); 
spi_write_dat(0x09); 
spi_write_dat(0x89); 
spi_write_dat(0xA0); 
spi_write_dat(0xA0); 

delay_2ms(5);
 
spi_write_cmd(0xEB); 
spi_write_dat(0x00);
spi_write_dat(0x01);
spi_write_dat(0xE4);
spi_write_dat(0xE4);
spi_write_dat(0x88);
spi_write_dat(0x00);
spi_write_dat(0x40);

delay_2ms(5);
 
spi_write_cmd(0xEC); 
spi_write_dat(0x3C);
spi_write_dat(0x01);

delay_2ms(5);

spi_write_cmd(0xED); 
spi_write_dat(0xAB);
spi_write_dat(0x89);
spi_write_dat(0x76);
spi_write_dat(0x54);
spi_write_dat(0x02);
spi_write_dat(0xFF);
spi_write_dat(0xFF);
spi_write_dat(0xFF);
spi_write_dat(0xFF);
spi_write_dat(0xFF);
spi_write_dat(0xFF);
spi_write_dat(0x20);
spi_write_dat(0x45);
spi_write_dat(0x67);
spi_write_dat(0x98);
spi_write_dat(0xBA);

delay_2ms(5);
spi_write_cmd(0x36);
spi_write_dat(0x00);

spi_write_cmd(0x3A);
spi_write_dat(0x55);
 
spi_write_cmd(0x29); 


#endif
    delay_2ms(20);
    
    printf("ILI8961C initial end\n");
}


/******************************************
ILI8961C接线说明
CS	-> PH8
SDA	-> PE5
SCL	-> PE4
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
		printf("=====lcd_ili8961c_backctrl=========\n");
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE(lcd_ili8961c_dev) = {
    .info = {

        .xres = 640,
        .yres = 480,
        .format = FORMAT_RGB565,

        .len 			 = LEN_256,
        .rotate_en       = true,	    // 旋转使能
        .hori_mirror_en  = false,//true,	    // 水平镜像使能
        .vert_mirror_en  = true,//false,		// 垂直镜像使能

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

    .ncycle          = CYCLE_ONE,
    .interlaced_mode = INTERLACED_NONE,

    .dclk_set        = CLK_EN | SIGNAL_HSYNC | SIGNAL_VSYNC |SIGNAL_DEN| CLK_NEGATIVE,
    //.dclk_set        = CLK_EN | SIGNAL_HSYNC | SIGNAL_VSYNC,
   // .sync0_set       = SIGNAL_DEN|CLK_EN|CLK_NEGATIVE,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,


    .drive_mode      = MODE_RGB_SYNC,
    .data_width      = PORT_16BITS,
    .port_sel        = PORT_GROUP_AA,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_4,

    .timing = {
    .hori_interval 		=   496+16,//496,//1560/4,496
    .hori_sync_clk 		=  1,//1,// 1
    .hori_start_clk 	=   15,//15,//240/4,15
    .hori_pixel_width 	=  480,//1280/4,480

    .vert_interval 		=  657,// 657
    .vert_sync_clk 		=   1,// 1
    .vert_start_clk 	=   6-2,// 8
    .vert_start1_clk 	=  6-2,// 8
    .vert_pixel_width 	=   642,// 640
    },
};

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .init = lcd_ili8961c_init,
    .type = LCD_DVP_RGB,
    .dev  = &lcd_ili8961c_dev,
    .bl_ctrl = lcd_ili8961c_backctrl,
};

#endif
