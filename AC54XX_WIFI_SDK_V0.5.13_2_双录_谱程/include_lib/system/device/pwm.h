#ifndef _TIME_PWM_H
#define _TIME_PWM_H

#include "includes.h"
#include "device/device.h"

#define PWM_MAX_NUM		6	//0-5，最多,6个通道


/***PWMCHx***/

#define PWMCH0  		1	//(1<<0)
#define PWMCH1  		2	//(1<<1)
#define PWMCH2  		4	//(1<<2)
#define PWMCH3  		8	//(1<<3)
#define PWMCH4  		16	//(1<<4)
#define PWMCH5  		32	//(1<<5)

/***ioctrl cmd***/
#define PWM_STOP				_IOW('P',0,u32)
#define PWM_RUN					_IOW('P',1,u32)
#define PWM_FORDIRC				_IOW('P',2,u32) //正向
#define PWM_REVDIRC				_IOW('P',3,u32) //反向
#define PWM_SET_DUTY			_IOW('P',4,u32)
#define PWM_SLE_TIMER			_IOW('P',5,u32)

#define PWMCH0_CMP_REG_W		_IOW('P',6,u32)
#define PWMCH1_CMP_REG_W		_IOW('P',7,u32)
#define PWMCH2_CMP_REG_W		_IOW('P',8,u32)
#define PWMCH3_CMP_REG_W		_IOW('P',9,u32)
#define PWMCH4_CMP_REG_W		_IOW('P',10,u32)
#define PWMCH5_CMP_REG_W		_IOW('P',11,u32)

#define PWMCH0_PRD_REG_R		_IOR('P',0,u32)
#define PWMCH1_PRD_REG_R		_IOR('P',1,u32)
#define PWMCH2_PRD_REG_R		_IOR('P',2,u32)
#define PWMCH3_PRD_REG_R		_IOR('P',3,u32)
#define PWMCH4_PRD_REG_R		_IOR('P',4,u32)
#define PWMCH5_PRD_REG_R		_IOR('P',5,u32)


#define PWM_PORTG		0x20
#define PWM_PORTC		0x21


struct pwm_reg {
    volatile u32 con;
    volatile u32 cnt;
    volatile u32 prd;
    volatile u32 cmp;
};

struct pwm_platform_data {
    struct pwm_reg *reg;
    u8  port;
    u8  pwm_ch;
    u8  duty;
    u32 freq;
};

struct pwm_operations {
    int (*open)(struct pwm_platform_data *pwm_data);
    int (*close)(struct pwm_platform_data *pwm_data);
    int (*write)(struct pwm_platform_data *pwm_data, u32 duty, u32 channel);
    int (*read)(struct pwm_platform_data *pwm_data, u32 channel);
    int (*ioctl)(struct pwm_platform_data *pwm_data, u32 cmd, u32 arg);
};

struct pwm_device {
    char *name;
    struct pwm_operations *ops;
    struct device dev;
    void *priv;
};

#define PWM_PLATFORM_DATA_BEGIN(data) \
		static struct pwm_platform_data data={

#define PWM_PLATFORM_DATA_END() \
		.reg=NULL, \
	};

#define REGISTER_PWM_DEVICE(dev) \
	struct pwm_device dev sec(.pwm)

extern const struct pwm_device pwm_device_begin[];
extern const struct pwm_device pwm_device_end[];

#define list_for_each_pwm_device(dev) \
	for(dev=pwm_device_begin;dev<=pwm_device_end;dev++)

extern const struct device_operations pwm_dev_ops;

#endif




