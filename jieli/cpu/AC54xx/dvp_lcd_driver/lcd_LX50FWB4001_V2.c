#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/pap.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#if (LCD_TYPE == LCD_LX50FWB4001_V2)

//resolution
#define TFT_LCD_WIDTH   854
#define TFT_LCD_HIGH  	480

//PH4->PH2 RESET  Pin37
#define SPI_RST_OUT()   PORTD_DIR &=~ BIT(12)
#define SPI_RST_H()     PORTD_OUT |=  BIT(12)
#define SPI_RST_L()     PORTD_OUT &=~ BIT(12)

#define SPI_DCX_OUT()
#define SPI_DCX_L()
#define SPI_DCX_H()

//PC5->PV5 CS     Pin40
#define SPI_CSX_OUT()   PORTB_DIR &=~ BIT(5)
#define SPI_CSX_H()     PORTB_OUT |=  BIT(5)
#define SPI_CSX_L()     PORTB_OUT &=~ BIT(5)

//PC6->PV6 SCL    Pin39
#define SPI_SCL_OUT()   PORTB_DIR &=~ BIT(6)
#define SPI_SCL_H()     PORTB_OUT |=  BIT(6)
#define SPI_SCL_L()     PORTB_OUT &=~ BIT(6)

//PC7->PV7 SDA    Pin38
#define SPI_SDA_OUT()   PORTB_DIR &=~ BIT(7)
#define SPI_SDA_IN()    PORTB_DIR |= BIT(7)
#define SPI_SDA_H()     PORTB_OUT |=  BIT(7)
#define SPI_SDA_L()     PORTB_OUT &=~ BIT(7)
#define SPI_SDA_R()     PORTB_IN & BIT(7)


/*
void delay_us(u16 cnt)
{
    while(cnt--)
    {
        delay(0);
    }
}
*/

static void spi_io_init()
{
    SPI_DCX_OUT();
    SPI_SCL_OUT();
    SPI_CSX_OUT();
    SPI_SDA_OUT();
    SPI_RST_OUT();

    SPI_SDA_H();
    SPI_CSX_H();
    SPI_DCX_L();
    SPI_SCL_L();
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

    for (i = 0; i < 8; i++) {
        SPI_SCL_L();
        delay_us(100);
        if (cmd & BIT(7)) {
            SPI_SDA_H();
        } else {
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

    for (i = 0; i < 8; i++) {
        SPI_SCL_L();
        delay_us(100);
        if (dat & BIT(7)) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }

        dat <<= 1;
        SPI_SCL_H();
        delay_us(500);
    }
    SPI_CSX_H();
    delay_us(100);
}

static void SPI_Cmd16(u16 tx)
{
    u8 i;
    u8 dat;
    SPI_CSX_L();

    dat = 0x20;
    for (i = 0; i < 8; i++) {
        delay_us(100);
        SPI_SCL_L();
        delay_us(100);
        if (dat & 0x80) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }
        delay_us(100);
        SPI_SCL_H();
        delay_us(100);
        dat <<= 1;
    }
    dat = tx >> 8;
    for (i = 0; i < 8; i++) {
        delay_us(100);
        SPI_SCL_L();
        delay_us(100);
        if (dat & 0x80) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }
        delay_us(100);
        SPI_SCL_H();
        delay_us(100);
        dat <<= 1;
    }
    dat = 0x00;
    for (i = 0; i < 8; i++) {
        delay_us(100);
        SPI_SCL_L();
        delay_us(100);
        if (dat & 0x80) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }
        delay_us(100);
        SPI_SCL_H();
        delay_us(100);
        dat <<= 1;
    }
    dat = tx;
    for (i = 0; i < 8; i++) {
        delay_us(100);
        SPI_SCL_L();
        delay_us(100);
        if (dat & 0x80) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }
        delay_us(100);
        SPI_SCL_H();
        delay_us(100);
        dat <<= 1;
    }

    SPI_CSX_H();
    delay_us(200);

}
static void SPI_Dat16(u8 tx)
{
    u8 i;
    u8 dat;

    SPI_CSX_L();

    dat = 0x40;
    for (i = 0; i < 8; i++) {
        delay_us(100);
        SPI_SCL_L();
        delay_us(100);
        if (dat & 0x80) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }
        delay_us(100);
        SPI_SCL_H();
        delay_us(100);
        dat <<= 1;
    }
    dat = tx;
    for (i = 0; i < 8; i++) {
        delay_us(100);
        SPI_SCL_L();
        delay_us(100);
        if (dat & 0x80) {
            SPI_SDA_H();
        } else {
            SPI_SDA_L();
        }
        delay_us(100);
        SPI_SCL_H();
        delay_us(100);
        dat <<= 1;
    }
    SPI_CSX_H();
    delay_us(200);

}

static void lcd_lx50fwb4001_v2_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    spi_io_init();

    //RESET
    SPI_RST_H();
    delay_2ms(5);
    SPI_RST_L();
    delay_2ms(10);
    SPI_RST_H();
    delay_2ms(50);

    //5寸 LX50FWB4001-V2
    //////////////////Initial  CODE///////////////////////
    SPI_Cmd16(0xF000);
    SPI_Dat16(0x55);
    SPI_Cmd16(0xF001);
    SPI_Dat16(0xAA);
    SPI_Cmd16(0xF002);
    SPI_Dat16(0x52);
    SPI_Cmd16(0xF003);
    SPI_Dat16(0x08);
    SPI_Cmd16(0xF004);
    SPI_Dat16(0x02);

    SPI_Cmd16(0xF600);
    SPI_Dat16(0x60);
    SPI_Cmd16(0xF601);
    SPI_Dat16(0x40);

    SPI_Cmd16(0xFE00);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xFE01);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xFE02);
    SPI_Dat16(0x09);
    SPI_Cmd16(0xFE03);
    SPI_Dat16(0x09);

    delay_2ms(10);

    SPI_Cmd16(0xF000);
    SPI_Dat16(0x55);
    SPI_Cmd16(0xF001);
    SPI_Dat16(0xAA);
    SPI_Cmd16(0xF002);
    SPI_Dat16(0x52);
    SPI_Cmd16(0xF003);
    SPI_Dat16(0x08);
    SPI_Cmd16(0xF004);
    SPI_Dat16(0x01);

    SPI_Cmd16(0xB000);
    SPI_Dat16(0x07);

    SPI_Cmd16(0xB100);
    SPI_Dat16(0x07);



    SPI_Cmd16(0xB500);
    SPI_Dat16(0x08);

    SPI_Cmd16(0xB600);
    SPI_Dat16(0x54);

    SPI_Cmd16(0xB700);
    SPI_Dat16(0x44);

    SPI_Cmd16(0xB800);
    SPI_Dat16(0x24);

    SPI_Cmd16(0xB900);
    SPI_Dat16(0x34);

    SPI_Cmd16(0xBA00);
    SPI_Dat16(0x14);

    SPI_Cmd16(0xBC00);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xBC01);
    SPI_Dat16(0x78);
    SPI_Cmd16(0xBC02);
    SPI_Dat16(0x13);

    SPI_Cmd16(0xBD00);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xBD01);
    SPI_Dat16(0x78);
    SPI_Cmd16(0xBD02);
    SPI_Dat16(0x13);

    SPI_Cmd16(0xBE00);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xBE01);
    SPI_Dat16(0x1A); //0x3E

    SPI_Cmd16(0xD100);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD101);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD102);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD103);
    SPI_Dat16(0x51);
    SPI_Cmd16(0xD104);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD105);
    SPI_Dat16(0x7E);
    SPI_Cmd16(0xD106);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD107);
    SPI_Dat16(0x9A);
    SPI_Cmd16(0xD108);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD109);
    SPI_Dat16(0xB0);
    SPI_Cmd16(0xD10A);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD10B);
    SPI_Dat16(0xD2);
    SPI_Cmd16(0xD10C);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD10D);
    SPI_Dat16(0xEE);
    SPI_Cmd16(0xD10E);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD10F);
    SPI_Dat16(0x1A);
    SPI_Cmd16(0xD110);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD111);
    SPI_Dat16(0x3C);
    SPI_Cmd16(0xD112);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD113);
    SPI_Dat16(0x71);
    SPI_Cmd16(0xD114);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD115);
    SPI_Dat16(0x9C);
    SPI_Cmd16(0xD116);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD117);
    SPI_Dat16(0xDF);
    SPI_Cmd16(0xD118);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD119);
    SPI_Dat16(0x16);
    SPI_Cmd16(0xD11A);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD11B);
    SPI_Dat16(0x18);
    SPI_Cmd16(0xD11C);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD11D);
    SPI_Dat16(0x4B);
    SPI_Cmd16(0xD11E);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD11F);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xD120);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD121);
    SPI_Dat16(0xA1);
    SPI_Cmd16(0xD122);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD123);
    SPI_Dat16(0xCD);
    SPI_Cmd16(0xD124);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD125);
    SPI_Dat16(0xEB);
    SPI_Cmd16(0xD126);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD127);
    SPI_Dat16(0x11);
    SPI_Cmd16(0xD128);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD129);
    SPI_Dat16(0x27);
    SPI_Cmd16(0xD12A);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD12B);
    SPI_Dat16(0x42);
    SPI_Cmd16(0xD12C);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD12D);
    SPI_Dat16(0x4F);
    SPI_Cmd16(0xD12E);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD12F);
    SPI_Dat16(0x5A);
    SPI_Cmd16(0xD130);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD131);
    SPI_Dat16(0x62);
    SPI_Cmd16(0xD132);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD133);
    SPI_Dat16(0xFF);

    SPI_Cmd16(0xD200);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD201);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD202);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD203);
    SPI_Dat16(0x51);
    SPI_Cmd16(0xD204);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD205);
    SPI_Dat16(0x7E);
    SPI_Cmd16(0xD206);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD207);
    SPI_Dat16(0x9A);
    SPI_Cmd16(0xD208);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD209);
    SPI_Dat16(0xB0);
    SPI_Cmd16(0xD20A);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD20B);
    SPI_Dat16(0xD2);
    SPI_Cmd16(0xD20C);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD20D);
    SPI_Dat16(0xEE);
    SPI_Cmd16(0xD20E);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD20F);
    SPI_Dat16(0x1A);
    SPI_Cmd16(0xD210);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD211);
    SPI_Dat16(0x3C);
    SPI_Cmd16(0xD212);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD213);
    SPI_Dat16(0x71);
    SPI_Cmd16(0xD214);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD215);
    SPI_Dat16(0x9C);
    SPI_Cmd16(0xD216);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD217);
    SPI_Dat16(0xDF);
    SPI_Cmd16(0xD218);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD219);
    SPI_Dat16(0x16);
    SPI_Cmd16(0xD21A);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD21B);
    SPI_Dat16(0x18);
    SPI_Cmd16(0xD21C);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD21D);
    SPI_Dat16(0x4B);
    SPI_Cmd16(0xD21E);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD21F);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xD220);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD221);
    SPI_Dat16(0xA1);
    SPI_Cmd16(0xD222);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD223);
    SPI_Dat16(0xCD);
    SPI_Cmd16(0xD224);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD225);
    SPI_Dat16(0xEB);
    SPI_Cmd16(0xD226);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD227);
    SPI_Dat16(0x11);
    SPI_Cmd16(0xD228);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD229);
    SPI_Dat16(0x27);
    SPI_Cmd16(0xD22A);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD22B);
    SPI_Dat16(0x42);
    SPI_Cmd16(0xD22C);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD22D);
    SPI_Dat16(0x4F);
    SPI_Cmd16(0xD22E);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD22F);
    SPI_Dat16(0x5A);
    SPI_Cmd16(0xD230);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD231);
    SPI_Dat16(0x62);
    SPI_Cmd16(0xD232);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD233);
    SPI_Dat16(0xFF);

    SPI_Cmd16(0xD300);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD301);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD302);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD303);
    SPI_Dat16(0x51);
    SPI_Cmd16(0xD304);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD305);
    SPI_Dat16(0x7E);
    SPI_Cmd16(0xD306);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD307);
    SPI_Dat16(0x9A);
    SPI_Cmd16(0xD308);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD309);
    SPI_Dat16(0xB0);
    SPI_Cmd16(0xD30A);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD30B);
    SPI_Dat16(0xD2);
    SPI_Cmd16(0xD30C);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD30D);
    SPI_Dat16(0xEE);
    SPI_Cmd16(0xD30E);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD30F);
    SPI_Dat16(0x1A);
    SPI_Cmd16(0xD310);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD311);
    SPI_Dat16(0x3C);
    SPI_Cmd16(0xD312);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD313);
    SPI_Dat16(0x71);
    SPI_Cmd16(0xD314);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD315);
    SPI_Dat16(0x9C);
    SPI_Cmd16(0xD316);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD317);
    SPI_Dat16(0xDF);
    SPI_Cmd16(0xD318);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD319);
    SPI_Dat16(0x16);
    SPI_Cmd16(0xD31A);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD31B);
    SPI_Dat16(0x18);
    SPI_Cmd16(0xD31C);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD31D);
    SPI_Dat16(0x4B);
    SPI_Cmd16(0xD31E);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD31F);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xD320);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD321);
    SPI_Dat16(0xA1);
    SPI_Cmd16(0xD322);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD323);
    SPI_Dat16(0xCD);
    SPI_Cmd16(0xD324);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD325);
    SPI_Dat16(0xEB);
    SPI_Cmd16(0xD326);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD327);
    SPI_Dat16(0x11);
    SPI_Cmd16(0xD328);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD329);
    SPI_Dat16(0x27);
    SPI_Cmd16(0xD32A);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD32B);
    SPI_Dat16(0x42);
    SPI_Cmd16(0xD32C);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD32D);
    SPI_Dat16(0x4F);
    SPI_Cmd16(0xD32E);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD32F);
    SPI_Dat16(0x5A);
    SPI_Cmd16(0xD330);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD331);
    SPI_Dat16(0x62);
    SPI_Cmd16(0xD332);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD333);
    SPI_Dat16(0xFF);

    SPI_Cmd16(0xD400);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD401);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD402);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD403);
    SPI_Dat16(0x51);
    SPI_Cmd16(0xD404);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD405);
    SPI_Dat16(0x7E);
    SPI_Cmd16(0xD406);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD407);
    SPI_Dat16(0x9A);
    SPI_Cmd16(0xD408);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD409);
    SPI_Dat16(0xB0);
    SPI_Cmd16(0xD40A);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD40B);
    SPI_Dat16(0xD2);
    SPI_Cmd16(0xD40C);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD40D);
    SPI_Dat16(0xEE);
    SPI_Cmd16(0xD40E);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD40F);
    SPI_Dat16(0x1A);
    SPI_Cmd16(0xD410);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD411);
    SPI_Dat16(0x3C);
    SPI_Cmd16(0xD412);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD413);
    SPI_Dat16(0x71);
    SPI_Cmd16(0xD414);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD415);
    SPI_Dat16(0x9C);
    SPI_Cmd16(0xD416);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD417);
    SPI_Dat16(0xDF);
    SPI_Cmd16(0xD418);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD419);
    SPI_Dat16(0x16);
    SPI_Cmd16(0xD41A);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD41B);
    SPI_Dat16(0x18);
    SPI_Cmd16(0xD41C);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD41D);
    SPI_Dat16(0x4B);
    SPI_Cmd16(0xD41E);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD41F);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xD420);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD421);
    SPI_Dat16(0xA1);
    SPI_Cmd16(0xD422);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD423);
    SPI_Dat16(0xCD);
    SPI_Cmd16(0xD424);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD425);
    SPI_Dat16(0xEB);
    SPI_Cmd16(0xD426);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD427);
    SPI_Dat16(0x11);
    SPI_Cmd16(0xD428);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD429);
    SPI_Dat16(0x27);
    SPI_Cmd16(0xD42A);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD42B);
    SPI_Dat16(0x42);
    SPI_Cmd16(0xD42C);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD42D);
    SPI_Dat16(0x4F);
    SPI_Cmd16(0xD42E);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD42F);
    SPI_Dat16(0x5A);
    SPI_Cmd16(0xD430);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD431);
    SPI_Dat16(0x62);
    SPI_Cmd16(0xD432);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD433);
    SPI_Dat16(0xFF);

    SPI_Cmd16(0xD500);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD501);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD502);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD503);
    SPI_Dat16(0x51);
    SPI_Cmd16(0xD504);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD505);
    SPI_Dat16(0x7E);
    SPI_Cmd16(0xD506);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD507);
    SPI_Dat16(0x9A);
    SPI_Cmd16(0xD508);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD509);
    SPI_Dat16(0xB0);
    SPI_Cmd16(0xD50A);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD50B);
    SPI_Dat16(0xD2);
    SPI_Cmd16(0xD50C);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD50D);
    SPI_Dat16(0xEE);
    SPI_Cmd16(0xD50E);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD50F);
    SPI_Dat16(0x1A);
    SPI_Cmd16(0xD510);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD511);
    SPI_Dat16(0x3C);
    SPI_Cmd16(0xD512);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD513);
    SPI_Dat16(0x71);
    SPI_Cmd16(0xD514);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD515);
    SPI_Dat16(0x9C);
    SPI_Cmd16(0xD516);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD517);
    SPI_Dat16(0xDF);
    SPI_Cmd16(0xD518);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD519);
    SPI_Dat16(0x16);
    SPI_Cmd16(0xD51A);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD51B);
    SPI_Dat16(0x18);
    SPI_Cmd16(0xD51C);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD51D);
    SPI_Dat16(0x4B);
    SPI_Cmd16(0xD51E);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD51F);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xD520);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD521);
    SPI_Dat16(0xA1);
    SPI_Cmd16(0xD522);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD523);
    SPI_Dat16(0xCD);
    SPI_Cmd16(0xD524);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD525);
    SPI_Dat16(0xEB);
    SPI_Cmd16(0xD526);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD527);
    SPI_Dat16(0x11);
    SPI_Cmd16(0xD528);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD529);
    SPI_Dat16(0x27);
    SPI_Cmd16(0xD52A);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD52B);
    SPI_Dat16(0x42);
    SPI_Cmd16(0xD52C);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD52D);
    SPI_Dat16(0x4F);
    SPI_Cmd16(0xD52E);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD52F);
    SPI_Dat16(0x5A);
    SPI_Cmd16(0xD530);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD531);
    SPI_Dat16(0x62);
    SPI_Cmd16(0xD532);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD533);
    SPI_Dat16(0xFF);

    SPI_Cmd16(0xD600);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD601);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD602);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD603);
    SPI_Dat16(0x51);
    SPI_Cmd16(0xD604);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD605);
    SPI_Dat16(0x7E);
    SPI_Cmd16(0xD606);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD607);
    SPI_Dat16(0x9A);
    SPI_Cmd16(0xD608);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD609);
    SPI_Dat16(0xB0);
    SPI_Cmd16(0xD60A);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD60B);
    SPI_Dat16(0xD2);
    SPI_Cmd16(0xD60C);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xD60D);
    SPI_Dat16(0xEE);
    SPI_Cmd16(0xD60E);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD60F);
    SPI_Dat16(0x1A);
    SPI_Cmd16(0xD610);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD611);
    SPI_Dat16(0x3C);
    SPI_Cmd16(0xD612);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD613);
    SPI_Dat16(0x71);
    SPI_Cmd16(0xD614);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD615);
    SPI_Dat16(0x9C);
    SPI_Cmd16(0xD616);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xD617);
    SPI_Dat16(0xDF);
    SPI_Cmd16(0xD618);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD619);
    SPI_Dat16(0x16);
    SPI_Cmd16(0xD61A);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD61B);
    SPI_Dat16(0x18);
    SPI_Cmd16(0xD61C);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD61D);
    SPI_Dat16(0x4B);
    SPI_Cmd16(0xD61E);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD61F);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xD620);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD621);
    SPI_Dat16(0xA1);
    SPI_Cmd16(0xD622);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD623);
    SPI_Dat16(0xCD);
    SPI_Cmd16(0xD624);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xD625);
    SPI_Dat16(0xEB);
    SPI_Cmd16(0xD626);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD627);
    SPI_Dat16(0x11);
    SPI_Cmd16(0xD628);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD629);
    SPI_Dat16(0x27);
    SPI_Cmd16(0xD62A);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD62B);
    SPI_Dat16(0x42);
    SPI_Cmd16(0xD62C);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD62D);
    SPI_Dat16(0x4F);
    SPI_Cmd16(0xD62E);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD62F);
    SPI_Dat16(0x5A);
    SPI_Cmd16(0xD630);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD631);
    SPI_Dat16(0x62);
    SPI_Cmd16(0xD632);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xD633);
    SPI_Dat16(0xFF);
//    delay_2ms(50);

    SPI_Cmd16(0xF000);
    SPI_Dat16(0x55);
    SPI_Cmd16(0xF001);
    SPI_Dat16(0xAA);
    SPI_Cmd16(0xF002);
    SPI_Dat16(0x52);
    SPI_Cmd16(0xF003);
    SPI_Dat16(0x08);
    SPI_Cmd16(0xF004);
    SPI_Dat16(0x03);

    SPI_Cmd16(0xB000);
    SPI_Dat16(0x05);
    SPI_Cmd16(0xB001);
    SPI_Dat16(0x17);
    SPI_Cmd16(0xB002);
    SPI_Dat16(0xF9);
    SPI_Cmd16(0xB003);
    SPI_Dat16(0x53);
    SPI_Cmd16(0xB004);
    SPI_Dat16(0x53);
    SPI_Cmd16(0xB005);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB006);
    SPI_Dat16(0x30);

    SPI_Cmd16(0xB100);
    SPI_Dat16(0x05);
    SPI_Cmd16(0xB101);
    SPI_Dat16(0x17);
    SPI_Cmd16(0xB102);
    SPI_Dat16(0xFB);
    SPI_Cmd16(0xB103);
    SPI_Dat16(0x55);
    SPI_Cmd16(0xB104);
    SPI_Dat16(0x53);
    SPI_Cmd16(0xB105);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB106);
    SPI_Dat16(0x30);

    SPI_Cmd16(0xB200);
    SPI_Dat16(0xFC);
    SPI_Cmd16(0xB201);
    SPI_Dat16(0xFD);
    SPI_Cmd16(0xB202);
    SPI_Dat16(0xFE);
    SPI_Cmd16(0xB203);
    SPI_Dat16(0xFF);
    SPI_Cmd16(0xB204);
    SPI_Dat16(0xF0);
    SPI_Cmd16(0xB205);
    SPI_Dat16(0xED);
    SPI_Cmd16(0xB206);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB207);
    SPI_Dat16(0xC4);
    SPI_Cmd16(0xB208);
    SPI_Dat16(0x08);

    SPI_Cmd16(0xB300);
    SPI_Dat16(0x5B);
    SPI_Cmd16(0xB301);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB302);
    SPI_Dat16(0xFC);
    SPI_Cmd16(0xB303);
    SPI_Dat16(0x5A);
    SPI_Cmd16(0xB304);
    SPI_Dat16(0x5A);
    SPI_Cmd16(0xB305);
    SPI_Dat16(0x03);

    SPI_Cmd16(0xB400);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB401);
    SPI_Dat16(0x01);
    SPI_Cmd16(0xB402);
    SPI_Dat16(0x02);
    SPI_Cmd16(0xB403);
    SPI_Dat16(0x03);
    SPI_Cmd16(0xB404);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB405);
    SPI_Dat16(0x40);
    SPI_Cmd16(0xB406);
    SPI_Dat16(0x04);
    SPI_Cmd16(0xB407);
    SPI_Dat16(0x08);
    SPI_Cmd16(0xB408);
    SPI_Dat16(0xED);
    SPI_Cmd16(0xB409);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB40A);
    SPI_Dat16(0x00);

    SPI_Cmd16(0xB500);
    SPI_Dat16(0x40);
    SPI_Cmd16(0xB501);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB502);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB503);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xB504);
    SPI_Dat16(0x5F);
    SPI_Cmd16(0xB505);
    SPI_Dat16(0x5E);
    SPI_Cmd16(0xB506);
    SPI_Dat16(0x50);
    SPI_Cmd16(0xB507);
    SPI_Dat16(0x50);
    SPI_Cmd16(0xB508);
    SPI_Dat16(0x33);
    SPI_Cmd16(0xB509);
    SPI_Dat16(0x33);
    SPI_Cmd16(0xB50A);
    SPI_Dat16(0x55);

    SPI_Cmd16(0xB600);
    SPI_Dat16(0xBC);
    SPI_Cmd16(0xB601);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB602);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB603);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB604);
    SPI_Dat16(0x2A);
    SPI_Cmd16(0xB605);
    SPI_Dat16(0x80);
    SPI_Cmd16(0xB606);
    SPI_Dat16(0x00);

    SPI_Cmd16(0xB700);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB701);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB702);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB703);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB704);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB705);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB706);
    SPI_Dat16(0x00);
    SPI_Cmd16(0xB707);
    SPI_Dat16(0x00);

    SPI_Cmd16(0xB800);
    SPI_Dat16(0x11);
    SPI_Cmd16(0xB801);
    SPI_Dat16(0x60);
    SPI_Cmd16(0xB802);
    SPI_Dat16(0x00);

    SPI_Cmd16(0xB900);
    SPI_Dat16(0x90);

    SPI_Cmd16(0xBA00);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBA01);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBA02);
    SPI_Dat16(0x08);
    SPI_Cmd16(0xBA03);
    SPI_Dat16(0xAC);
    SPI_Cmd16(0xBA04);
    SPI_Dat16(0xE2);
    SPI_Cmd16(0xBA05);
    SPI_Dat16(0x64);
    SPI_Cmd16(0xBA06);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBA07);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBA08);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBA09);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBA0A);
    SPI_Dat16(0x47);
    SPI_Cmd16(0xBA0B);
    SPI_Dat16(0x3F);
    SPI_Cmd16(0xBA0C);
    SPI_Dat16(0xDB);
    SPI_Cmd16(0xBA0D);
    SPI_Dat16(0x91);
    SPI_Cmd16(0xBA0E);
    SPI_Dat16(0x54);
    SPI_Cmd16(0xBA0F);
    SPI_Dat16(0x44);

    SPI_Cmd16(0xBB00);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBB01);
    SPI_Dat16(0x43);
    SPI_Cmd16(0xBB02);
    SPI_Dat16(0x79);
    SPI_Cmd16(0xBB03);
    SPI_Dat16(0xFD);
    SPI_Cmd16(0xBB04);
    SPI_Dat16(0xB5);
    SPI_Cmd16(0xBB05);
    SPI_Dat16(0x14);
    SPI_Cmd16(0xBB06);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBB07);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBB08);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBB09);
    SPI_Dat16(0x44);
    SPI_Cmd16(0xBB0A);
    SPI_Dat16(0x40);
    SPI_Cmd16(0xBB0B);
    SPI_Dat16(0x4A);
    SPI_Cmd16(0xBB0C);
    SPI_Dat16(0xCE);
    SPI_Cmd16(0xBB0D);
    SPI_Dat16(0x86);
    SPI_Cmd16(0xBB0E);
    SPI_Dat16(0x24);
    SPI_Cmd16(0xBB0F);
    SPI_Dat16(0x44);

    SPI_Cmd16(0xBC00);
    SPI_Dat16(0xE0);
    SPI_Cmd16(0xBC01);
    SPI_Dat16(0x1F);
    SPI_Cmd16(0xBC02);
    SPI_Dat16(0xF8);
    SPI_Cmd16(0xBC03);
    SPI_Dat16(0x07);

    SPI_Cmd16(0xBD00);
    SPI_Dat16(0xE0);
    SPI_Cmd16(0xBD01);
    SPI_Dat16(0x1F);
    SPI_Cmd16(0xBD02);
    SPI_Dat16(0xF8);
    SPI_Cmd16(0xBD03);
    SPI_Dat16(0x07);
//    delay_2ms(50);

    SPI_Cmd16(0xF000);
    SPI_Dat16(0x55);
    SPI_Cmd16(0xF001);
    SPI_Dat16(0xAA);
    SPI_Cmd16(0xF002);
    SPI_Dat16(0x52);
    SPI_Cmd16(0xF003);
    SPI_Dat16(0x08);
    SPI_Cmd16(0xF004);
    SPI_Dat16(0x00);

//    SPI_Cmd16(0xB000); SPI_Dat16(0x07);
    SPI_Cmd16(0xB000);
    SPI_Dat16(0x00);
//    SPI_Cmd16(0xB001); SPI_Dat16(0x1c);//VBP
//    SPI_Cmd16(0xB002); SPI_Dat16(0x1c);//VFP
//    SPI_Cmd16(0xB003); SPI_Dat16(0x10);//HBP
//    SPI_Cmd16(0xB004); SPI_Dat16(0x10);//HFP

//    SPI_Cmd16(0xB000); SPI_Dat16(0x80);
//    SPI_Cmd16(0xB001); SPI_Dat16(0x05);//VBP
//    SPI_Cmd16(0xB002); SPI_Dat16(0x02);//VFP
//    SPI_Cmd16(0xB003); SPI_Dat16(0x05);//HBP
//    SPI_Cmd16(0xB004); SPI_Dat16(0x02);//HFP

//    SPI_Cmd16(0xB500); SPI_Dat16(0x6C);
    SPI_Cmd16(0xB500);
    SPI_Dat16(0x6B);
    SPI_Cmd16(0xBC00);
    SPI_Dat16(0x00);

    SPI_Cmd16(0x1100);
    SPI_Dat16(0x00);

    delay_2ms(50);

    SPI_Cmd16(0x2900);
    SPI_Dat16(0x00);

    delay_2ms(50);
}


static void lcd_lx50fwb4001_v2_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE(lcd_lx50fwb4001_v2_dev) = {
    .info = {
        .test_mode 	     = false,
        .color           = 0x0000FF,
        .rotate_en       = true,	    // 旋转使能
        .hori_mirror_en  = true,	    // 水平镜像使能
        .vert_mirror_en  = false,		// 垂直镜像使能

        .len 			 = LEN_256,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .buf_num  		 = LCD_DEV_BNUM,
        .buf_addr 		 = LCD_DEV_BUF,
        .itp_mode_en     = FALSE,
        .sample          = SAMP_YUV422,
        .format          = FORMAT_RGB888,

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
    /* .image_width     = 480, */
    .drive_mode      = MODE_RGB_DE_SYNC,
    .ncycle          = CYCLE_ONE,
    .data_width      = PORT_24BITS,
    .interlaced_mode = INTERLACED_NONE,
    .dclk_set        = CLK_EN /*|CLK_NEGATIVE*/,
    .sync0_set       = SIGNAL_DEN  | CLK_EN/*|CLK_NEGATIVE*/,
    .sync1_set       = SIGNAL_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_set       = SIGNAL_VSYNC | CLK_EN | CLK_NEGATIVE,
    .port_sel        = PORT_GROUP_B,
    .clk_cfg    	 = PLL2_CLK | DIVA_3 | DIVB_2 | DIVC_2,

    .timing = {
        .hori_interval 		=    551,//49Hz@25MHz:551  60Hz@37.5MHz:679
        .hori_sync_clk 		=    2,
        .hori_start_clk 	=    20,
        .hori_pixel_width 	=    480,

        .vert_interval 		=    856 + 34 + 30, //920
        .vert_sync_clk 		=    2,//3,
        .vert_start_clk 	=    28,//29+2,
        .vert_start1_clk 	=    0,
        .vert_pixel_width 	=    856
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .enable	 = IF_ENABLE(LCD_LX50FWB4001_V2),
    .logo 	 = "rm68172ga1_2",
    .type 	 = LCD_DVP_RGB,
    .init 	 = lcd_lx50fwb4001_v2_init,
    .dev  	 = &lcd_lx50fwb4001_v2_dev,
    .bl_ctrl = lcd_lx50fwb4001_v2_backctrl,
};

#endif
