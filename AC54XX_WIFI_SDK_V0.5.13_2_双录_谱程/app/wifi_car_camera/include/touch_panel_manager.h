
#ifndef _TOUCH_PANEL_MANAGER_H
#define _TOUCH_PANEL_MANAGER_H
#include "printf.h"
#include "cpu.h"
#include "iic.h"
#include "timer.h"
#include "app_config.h"
#include "event.h"
#include "system/includes.h"

//extern u8 touch_panel_command(u8 w_chip_id, u16 regr_addr, u16 function_command);
//extern u8 _touch_panel_get_data(u8 w_chip_id, u8 r_chip_id, u16 regr_addr);
extern u8 _touch_panel_write(u8 w_chip_id, u16 reg_addr, u8 *buf, u32 len);
extern u8 _touch_panel_read(u8 w_chip_id, u8 r_chip_id, u16 reg_addr, u8 *buf, u32 len);

typedef struct touch_point {
    u8  e;
    u32 x;
    u32 y;
    u32 c;
} TOUCH_POINT;

typedef struct touch_panel_points {
    u32 point_num;
    TOUCH_POINT  p[5];
} TOUCH_PANEL_POINTS;

typedef struct sw_touch_panel_platform_data {
    u8	 enable;
    char *iic_dev;
    u32  rst_pin;
    u32  int_pin;

    u32  _MAX_POINT;
    u32  _MAX_X;
    u32  _MAX_Y;
    u8 	 _INT_TRIGGER;			//0:int_raising 1:int_falling 2:int_down 3:int_up
    u8 	 _X2Y_EN;
    u8 	 _X_MIRRORING;
    u8 	 _Y_MIRRORING;
    u8 	 _DEBUGP;				//debug law points
    u8 	 _DEBUGE;				//debug touch event

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
    char (*touch_panel_check)(struct sw_touch_panel_platform_data *data);
    char (*touch_panel_init)(void);
    char (*touch_panel_read)(u8 *buf, u32 len, u32 arg);
    char (*touch_panel_ioctl)(u32 cmd, u32 arg);
} _TOUCH_PANEL_INTERFACE;

typedef struct touch_panel_device {
    u8 logo[20];
    _TOUCH_PANEL_INTERFACE *touch_panel_ops;
} TOUCH_PANEL_INTERFACE;


extern TOUCH_PANEL_INTERFACE  touch_panel_dev_begin[];
extern TOUCH_PANEL_INTERFACE  touch_panel_dev_end[];

#define REGISTER_TOUCH_PANEL(touch_panel) \
	static const TOUCH_PANEL_INTERFACE touch_panel sec(.touch_panel_dev) = { \
        .logo = #touch_panel,


#define list_for_each_touch_panel(c) \
	for (c = touch_panel_dev_begin; c < touch_panel_dev_end; c++)


#define SW_TOUCH_PANEL_PLATFORM_DATA_BEGIN(data) \
	static const struct sw_touch_panel_platform_data data = {


#define SW_TOUCH_PANEL_PLATFORM_DATA_END() \
	};

#endif // _TOUCH_PANEL_MANAGER_H


