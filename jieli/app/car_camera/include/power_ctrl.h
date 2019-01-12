
#ifndef __POWER_CTRL_H
#define __POWER_CTRL_H

#include "asm/rtc.h"
#include "system/includes.h"
#include "app_config.h"
extern void port_wkup_ctrl(u8 en);
extern int first_time_power_on();
extern void auto_close_time_set(int sec);
extern void send_key_long_msg();
extern void clear_usb_charg_off_sec_cnt();

struct power_platform_data {
    struct _pr_wkup  usb_wkup;
    struct _pr_wkup  gsen_int_wkup;
    struct _pr_main_en 	en_pr;
};

#ifdef FLASE_POWER_OFF
u8 get_power_off_state();
#endif

#define POWER_PLATFORM_DATA_BEGIN(data) \
	static const struct power_platform_data data = {


#define POWER_PLATFORM_DATA_END() \
	};


extern const struct device_operations power_dev_ops;

typedef struct {
    int (*power_init)(void);
    int (*power_open)(void *);
    int (*power_ioctrl)(int cmd, int arg);
    int (*power_close)(void);
} _power_ops;

typedef struct power_device {
    struct device dev;
    _power_ops  *power_ops;
} power_device;

extern power_device power_dev_begin[];
extern power_device power_dev_end[];


#define REGISTER_POWER_DEV(power) \
	static power_device  power sec(.power_dev) = {


#define list_for_each_power_dev(c) \
	for (c=power_dev_begin; c<power_dev_end; c++)



#endif
