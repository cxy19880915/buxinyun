#ifndef _AV10_SPI_H
#define _AV10_SPI_H


#include "asm/isp_dev.h"
#include "gpio.h"

#include "typedef.h"
#include "device/device.h"
#include "generic/ioctl.h"
#include "system/task.h"


extern void av10_f_xor(u16 addr, u8 wdat);
extern void av10_f_and(u16 addr, u8 wdat);
extern void av10_f_or(u16 addr, u8 wdat);
extern void av10_f_mov(u16 addr, u8 wdat);
extern void av10_drv_rst(u16 addr, u8 wdat);
extern u8 av10_drv_rx(u16 addr);
extern void av10_drv_tx(u16 addr, u8 wdat);
extern u8 spi1_receive(u8 buf);
extern void spi1_send(u8 dat);


struct avin_spi_device;


struct software_spi {
    u8 pin_cs;
    u8 pin_clk;
    u8 pin_in;
    u8 pin_out;
};

struct sw_spi_platform_data {
    struct  software_spi spi;
};


#define SW_SPI_PLATFORM_DATA_BEGIN(data) \
	static const struct sw_spi_platform_data data = { \
		.spi = {

#define SW_SPI_PLATFORM_DATA_END() \
		}, \
	};


struct avin_spi_operations {
    int (*init)(const void *);
    int (*read)(const void *, void *buf, int len);
    int (*write)(const void *, void *buf, int len);
    int (*ioctl)(const void *, int cmd, int arg);
};

struct avin_spi_device {
    const void *spi_dat;
    struct device dev;
    const struct avin_spi_operations *ops;
    OS_MUTEX mutex;
};


#define REGISTER_AVIN_SPI_DEVICE( _ops) \
	static struct avin_spi_device __sw_spi sec(.sw_spi) = { \
		.ops = _ops,\
   	};

extern struct avin_spi_device avin_spi_device_begin[], avin_spi_device_end[];

#define list_for_each_avin_spi_device(p) \
	for (p=avin_spi_device_begin; p<avin_spi_device_end; p++)


#endif // _AV10_SPI_H
