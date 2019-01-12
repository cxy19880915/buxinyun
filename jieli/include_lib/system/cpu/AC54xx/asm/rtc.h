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





struct _pr_wkup {
    unsigned int gpio;
    u8 edge;
    u8 port_en;
};

struct _pr_main_en {
    u8 wkup_en;
    unsigned int wkup_power_gpio;
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


void power_off_wkup_set(unsigned int usb_wkup_gpio, u8 en);

#endif

