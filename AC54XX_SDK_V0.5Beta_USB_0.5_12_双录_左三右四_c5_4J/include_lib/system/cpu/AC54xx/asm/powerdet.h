#ifndef CPU_POWERDETECT_H
#define CPU_POWERDETECT_H

#include "asm/cpu.h"
#include "asm/adc.h"
#include "device/key_driver.h"
#include "device/device.h"

#define POWER_DET_GET_LEVEL	0

// #define LOW_POWER           (sizeof(power_level) / sizeof(power_level[0]))
#define PWR_SCAN_TIMES 	    10
#define ENABLE_SAMPLE_VAL 	1

enum {
    BATTERY_CHARGING,
    BATTERY_CONSUME,
};

/*
阈值计算：
((value/3.0)*1024)/3.3 = PWR_VALUEXXX  1/3分压
*/

#define LDO_REFERENCE_VOL	124 //1.24基准电压
#define VOL_3_70			370 //3.7V
#define VOL_3_75			375 //3.7V
#define VOL_3_85			385 //3.85V
#define VOL_4_00			400 //4.00V
#define VOL_4_05			405 //4.05V

struct powerdet_platform_data {
    u8 nlevel;				//电量分多少等级
    u16 power_level[10];    //电量阈值设置
};

#define POWERDET_PLATFORM_DATA_BEGIN(data) \
	static const struct powerdet_platform_data data = {


#define POWERDET_PLATFORM_DATA_END() \
	};


extern const struct device_operations powerdet_dev_ops;

#endif

