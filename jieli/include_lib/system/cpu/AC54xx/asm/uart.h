#ifndef ASM_UART_H
#define ASM_UART_H


#define UART_NUM 			4
#define UART_OUTPORT_NUM 	4


#include "device/uart.h"
#include "device/device.h"

#define UART0_PLATFORM_DATA_BEGIN(data) \
	static const struct uart_platform_data data = {


#define UART0_PLATFORM_DATA_END()  \
	.irq 					= UART0_INT, \
};


#define UART1_PLATFORM_DATA_BEGIN(data) \
	static const struct uart_platform_data data = {


#define UART1_PLATFORM_DATA_END()  \
	.irq 					= UART1_INT, \
};


#define UART2_PLATFORM_DATA_BEGIN(data) \
	static const struct uart_platform_data data = {

#define UART2_PLATFORM_DATA_END()  \
	.irq 					= UART2_INT, \
};


#define UART3_PLATFORM_DATA_BEGIN(data) \
	static const struct uart_platform_data data = {

#define UART3_PLATFORM_DATA_END()  \
	.irq 					= UART3_INT, \
};

extern const struct device_operations uart_dev_ops;


extern int uart_init(const struct uart_platform_data *);



#endif


