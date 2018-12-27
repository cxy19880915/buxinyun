#ifndef DEVICE_ADKEY_H
#define DEVICE_ADKEY_H


#include "typedef.h"



struct adkey_value_table {
    u16 ad_value[ADKEY_MAX_NUM];
    u8  key_value[ADKEY_MAX_NUM];
};

struct adkey_platform_data {
    u8 io;
    u8 ad_channel;
    u8 base_cnt;
    u8 long_cnt;
    u8 hold_cnt;
    /*struct gpio_reg *port;*/
    struct adkey_value_table table;
};




#endif

