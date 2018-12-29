#ifndef __CPU_RTC_H__
#define __CPU_RTC_H__

#include "typedef.h"
#include "device/device.h"
#include "generic/ioctl.h"
#include "system/time.h"


#define IOCTL_PORT_PR_IN        _IOR('R',  'T'+0,  0)
#define IOCTL_PORT_PR_OUT       _IOW('R',  'T'+1,  0)
#define IOCTL_PORT_PR_PU        _IOR('R',  'T'+2,  0)
#define IOCTL_PORT_PR_PD        _IOR('R',  'T'+3,  0)
#define IOCTL_PORT_PR_HD        _IOW('R',  'T'+4,  0)
#define IOCTL_PORT_PR_DIE       _IOW('R',  'T'+5,  0)
#define IOCTL_PORT_PR_READ      _IOR('R',  'T'+6,  0)

#define  BAT_POWER_FIRST               BIT(0)
#define  USB_IO_WKUP                   BIT(1)
#define  PARK_IO_WKUP                  BIT(2)
#define  USER_IO_UKUP                  BIT(3)
#define  ABNORMAL_RESET                BIT(5)




struct _pr_wkup {
    unsigned int gpio;
    u8 edge;
    u8 port_en;
};

struct _pr_main_en {
    u8 wkup_en;
    unsigned int wkup_power_gpio;
};

struct power_platform_data {
    struct _pr_wkup  usb_wkup;
    struct _pr_wkup  gsen_int_wkup;
    struct _pr_wkup  user_int_wkup;
    struct _pr_main_en 	en_pr;
};


extern const struct device_operations rtc_dev_ops;


int rtc_port_pr_in(u8 port);

int rtc_port_pr_read(u8 port);

int rtc_port_pr_out(u8 port, bool on);

int rtc_port_pr_hd(u8 port, bool on);

int rtc_port_pr_pu(u8 port, bool on);

int rtc_port_pr_pd(u8 port, bool on);

int rtc_port_pr_die(u8 port, bool on);

int rtc_early_init();


void rtc_wkup_ctrl(struct power_platform_data *wkup_cfg);

u32 rtc_wkup_reason(struct power_platform_data *wkup_cfg, u8 key_power);


void alarm_wkup_ctrl(int sec);
#endif

