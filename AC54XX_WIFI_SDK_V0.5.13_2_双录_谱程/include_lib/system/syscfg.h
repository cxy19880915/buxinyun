#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H


#include "generic/typedef.h"
#include "sys/tags.h"


struct syscfg_arg {
    u32 tag;
    u32 arg;
};


struct syscfg_handler {
    u32 tag;
    int (*handler)(void *data);
};


#define __system_cfg_arg(_tag, _arg) \
	const struct syscfg_arg __cfgarg_##_tag sec(.syscfg.arg) = { \
		.tag = _tag, \
		.arg = _arg, \
	}



#define __system_cfg_handler(_tag, _handler) \
	const struct syscfg_handler __cfghandler##_tag sec(.syscfg.handler) = { \
		.tag = _tag, \
		.handler = _handler, \
	}



int system_cfg(u32 tag, u32 arg);


#endif

