#ifndef SDMMC_MODULE_H
#define SDMMC_MODULE_H


#include "generic/typedef.h"
#include "generic/ioctl.h"


#define     SD_CMD_DECT 	0
#define     SD_CLK_DECT  	1
#define     SD_IO_DECT 		2

#define    SD_CLASS_0      0
#define    SD_CLASS_2      1
#define    SD_CLASS_4      2
#define    SD_CLASS_6      3
#define    SD_CLASS_10     4

#define SD_IOCTL_GET_CLASS  _IOR('S', 0, 4)


struct sdmmc_platform_data {
    char port;
    u8 irq;
    u8 data_width;
    u8 priority;
    u8 detect_mode;
    u8 detect_time_interval;
    u32 speed;
    volatile u32 *sfr;
    int (*detect_func)(const struct sdmmc_platform_data *);
    void (*port_init)(const struct sdmmc_platform_data *, int mode);
    void (*power)(int on);
};












#endif

