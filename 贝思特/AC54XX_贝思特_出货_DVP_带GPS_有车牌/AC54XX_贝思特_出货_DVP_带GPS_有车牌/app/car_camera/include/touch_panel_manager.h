
#ifndef _TOUCH_PANEL_MANAGER_H
#define _TOUCH_PANEL_MANAGER_H
#include "printf.h"
#include "cpu.h"
#include "iic.h"
#include "timer.h"
#include "app_config.h"
#include "event.h"
#include "system/includes.h"

//extern unsigned char touch_panel_command(unsigned char w_chip_id, unsigned char register_address, unsigned char function_command);
//extern unsigned char _touch_panel_get_data(unsigned char w_chip_id, unsigned char r_chip_id, unsigned char register_address);
extern unsigned char _touch_panel_write(unsigned char w_chip_id, unsigned short register_address, unsigned char *buf, unsigned int len);
extern unsigned char _touch_panel_read(unsigned char w_chip_id, unsigned char r_chip_id, unsigned short register_address, unsigned char *buf, unsigned int len);
extern unsigned char _touch_panel_read_32BIT(unsigned char w_chip_id, unsigned char r_chip_id, unsigned int register_address, unsigned char *buf, unsigned int len);
extern unsigned char _touch_panel_write_32BIT(unsigned char w_chip_id, unsigned int register_address, unsigned char *buf, unsigned int len);
extern void dev_close_touch();

typedef struct touch_point {
    unsigned char e;
    unsigned int  x;
    unsigned int  y;
    unsigned int  c;
} TOUCH_POINT;

typedef struct touch_panel_points {
    unsigned int point_num;
    TOUCH_POINT  p[5];
} TOUCH_PANEL_POINTS;

typedef struct sw_touch_panel_platform_data {
    unsigned char	enable;
    char  			*iic_dev;
    unsigned int 	rst_pin;
    unsigned int 	int_pin;

    unsigned int 	_MAX_POINT;
    unsigned int 	_MAX_X;
    unsigned int 	_MAX_Y;
    unsigned char 	_INT_TRIGGER;			//0:int_raising 1:int_falling 2:int_down 3:int_up
    unsigned char 	_X2Y_EN;
    unsigned char 	_X_MIRRORING;
    unsigned char 	_Y_MIRRORING;
    unsigned char 	_DEBUGP;				//debug law points
    unsigned char 	_DEBUGE;				//debug touch event

    TOUCH_PANEL_POINTS points;

} SW_TOUCH_PANEL_PLATFORM_DATA;

typedef enum {
    TOUCH_PANEL_CMD_SLEEP = 0,
    TOUCH_PANEL_CMD_WAKEUP,
    TOUCH_PANEL_CMD_RESET,
    TOUCH_PANEL_CMD_DISABLE,
    TOUCH_PANEL_CMD_ENABLE,
} TOUCH_PANEL_CMD;

typedef struct {
    unsigned char   logo[20];
    char (*touch_panel_check)(struct sw_touch_panel_platform_data *data);
    char (*touch_panel_init)(void);
    char (*touch_panel_read)(u8 *buf, u32 len, u32 arg);
    char (*touch_panel_ioctl)(u32 cmd, u32 arg);
} _TOUCH_PANEL_INTERFACE;

typedef struct touch_panel_device {
    struct device dev;
    _TOUCH_PANEL_INTERFACE *touch_panel_ops;
} TOUCH_PANEL_INTERFACE;


extern TOUCH_PANEL_INTERFACE  touch_panel_dev_begin[];
extern TOUCH_PANEL_INTERFACE  touch_panel_dev_end[];

#define REGISTER_TOUCH_PANEL(touch_panel) \
	static TOUCH_PANEL_INTERFACE touch_panel sec(.touch_panel_dev) = {


#define list_for_each_touch_panel(c) \
	for (c=touch_panel_dev_begin; c<touch_panel_dev_end; c++)


#define SW_TOUCH_PANEL_PLATFORM_DATA_BEGIN(data) \
	static /* const */ struct sw_touch_panel_platform_data data = {


#define SW_TOUCH_PANEL_PLATFORM_DATA_END() \
	};

#endif // _TOUCH_PANEL_MANAGER_H


