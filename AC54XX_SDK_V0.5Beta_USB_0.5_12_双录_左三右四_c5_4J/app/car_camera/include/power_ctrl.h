
#ifndef __POWER_CTRL_H
#define __POWER_CTRL_H

#include "asm/rtc.h"
#include "system/includes.h"
#include "app_config.h"
extern void auto_close_time_set(int sec);
extern void send_key_long_msg();
extern void clear_usb_charg_off_sec_cnt();


#define POWER_PLATFORM_DATA_BEGIN(data) \
	struct power_platform_data data = {


#define POWER_PLATFORM_DATA_END() \
	};

extern void system_shutdown(u8 force);

#endif
