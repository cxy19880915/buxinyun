#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include "typedef.h"


struct syscfg_data {
    const char *name;
    int len;
};

struct dict_list {
    const char *key;
    u8 value_bits;
    u32 value;
};

#define REGISTER_SYS_CONFIG(_name, _len) \
    const struct syscfg_data syscfg_data_##_name sec(.sys_cfg) = { \
        .name = #_name, \
        .len = _len + 2, \
    }




int sys_config_store(const char *name, void *data, int offset, int len);


int sys_config_restore(const char *name, void *data, int offset, int len);


int sys_config_init(const char *dev_name);







#endif

