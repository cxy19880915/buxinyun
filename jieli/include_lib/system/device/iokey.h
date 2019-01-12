#ifndef DEVICE_IOKEY_H
#define DEVICE_IOKEY_H

#include "typedef.h"
#include "device/device.h"


struct iokey_port {
    u8 port;
    u8 press_value: 1;
    u8 key_value: 7;
};

struct iokey_platform_data {
    u8 num;
    const struct iokey_port *port;
};






extern const struct device_operations iokey_dev_ops;






#endif
