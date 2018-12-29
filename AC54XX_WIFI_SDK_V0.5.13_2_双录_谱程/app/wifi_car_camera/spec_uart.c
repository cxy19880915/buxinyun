#include "device/uart.h"

/*****************************************************************************
 Description:特殊串口初始化函数
******************************************************************************/

static void *uart_dev_handle;
void spec_uart_init(void)
{
    static char uart_circlebuf[1 * 1024] __attribute__((aligned(32))); //串口循环数据buf,根据需要设置大小

    int parm;
    uart_dev_handle = dev_open("uart3", 0);

    dev_ioctl(uart_dev_handle, UART_SET_CIRCULAR_BUFF_ADDR, (int)uart_circlebuf);


    parm = sizeof(uart_circlebuf);

    dev_ioctl(uart_dev_handle, UART_SET_CIRCULAR_BUFF_LENTH, (int)&parm);


#if 0 // 是否设置为 接收完指定长度数据, spec_uart_recv才出来
    parm = 1;
    dev_ioctl(uart_dev_handle, UART_SET_RECV_ALL, (int)&parm);
#endif

#if 1 // 是否设置为阻塞方式读
    parm = 1;
    dev_ioctl(uart_dev_handle, UART_SET_RECV_BLOCK, (int)&parm);
#endif

    dev_ioctl(uart_dev_handle, UART_START, (int)0);
#if 0
    int spec_uart_recv(char *buf, u32 len);
    int spec_uart_send(char *buf, u32 len);
    char buf[200];
    int len;
    while (1) {
        len = spec_uart_recv(buf, 200);
        os_time_dly(1);
        spec_uart_send(buf, len);
        os_time_dly(1);
    }
#endif
}

int spec_uart_recv(char *buf, u32 len)
{
    return dev_read(uart_dev_handle, buf, len);
}

int spec_uart_send(char *buf, u32 len)
{
    return dev_write(uart_dev_handle, buf, len);
}
