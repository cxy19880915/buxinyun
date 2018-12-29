#ifndef SYS_KEY_DRIVER_H
#define SYS_KEY_DRIVER_H

#include "typedef.h"
#include "device/device.h"


#define NO_KEY 		0xff


struct key_driver;


struct key_driver_ops {
    int (*init)(struct key_driver *, void *arg);
    u16(*get_value)(struct key_driver *);
};

struct key_driver {
    u8 prev_value;
    u8 prev_event;
    u8 last_key;
    u8 filter_cnt;
    u8 base_cnt;
    u32 long_cnt;
    u32 hold_cnt;
    u32 press_cnt;
    u32 scan_time;
    const char *name;
    const struct key_driver_ops *ops;
};

extern struct key_driver key_driver_begin[];
extern struct key_driver key_driver_end[];

const struct device_operations key_dev_ops;

#define REGISTER_KEY_DRIVER(driver) \
	static struct key_driver driver sec(.key_driver)


#define list_for_each_key_dirver(p) \
	for (p=key_driver_begin; p<key_driver_end; p++)




#endif
