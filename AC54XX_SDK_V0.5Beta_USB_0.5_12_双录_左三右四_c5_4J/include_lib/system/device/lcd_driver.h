#ifndef __LCD_DRIVER_H__
#define __LCD_DRIVER_H__

#include "generic/typedef.h"
#include "device/device.h"
#include "asm/cpu.h"
#include "asm/imd.h"

#define 	LCD_ESD_CHECK_CTRL 			_IOW('F', 0, sizeof(int))

struct lcd_platform_data {
    enum LCD_IF  interface;
    u8  lcd_mode;
    struct {
        u8 backlight;
        u8 backlight_value;

        u8 lcd_reset;
        u8 lcd_cs;
        u8 lcd_rs;
        u8 lcd_spi_ck;
        u8 lcd_spi_di;
        u8 lcd_spi_do;
    } lcd_io;
};

#define LCD_PLATFORM_DATA_BEGIN(data) \
static const struct lcd_platform_data data = { \

#define LCD_PLATFORM_DATA_END() \
};

extern const struct device_operations lcd_dev_ops;

int lcd_reinit();
struct lcd_dev_drive *lcd_get_dev_hdl();
int  lcd_setxy(void);
void lcd_backlight_ctrl(u8 onoff);
int  lcd_get_type();
void lcd_esd_check_reset();
u8 lcd_get_bl_sta();
u8 lcd_get_esd_freq();

#endif
