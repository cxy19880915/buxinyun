#ifndef CPU_ADKEY_H
#define CPU_ADKEY_H

#include "asm/cpu.h"
#include "asm/adc.h"
#include "device/key_driver.h"


#define ADKEY_MAX_NUM 16


#include "device/device.h"
#include "device/adkey.h"



#define ADKEY_PLATFORM_DATA_BEGIN(data) \
	static const struct adkey_platform_data data = {


#define ADKEY_PLATFORM_DATA_END() \
	.base_cnt 	= 4,  \
	.long_cnt 	= 75, \
	.hold_cnt 	= 90, \
};



extern const struct device_operations adkey_dev_ops;
















#endif


