#ifndef DEVICE_UART_H
#define DEVICE_UART_H




#define UART_DMA_SUPPORT 	0x00000001
#define UART_TX_USE_DMA 	0x00000003
#define UART_RX_USE_DMA 	0x00000005
#define UART_DEBUG 			0x00000008

struct uart_outport {
    u8  tx_pin;
    u8  rx_pin;
    u16 value;
};

struct uart_platform_data {
    u8  irq;
    u8  tx_pin;
    u8  rx_pin;
    u32 flags;
    u32 baudrate;
};


extern void putbyte(char a);



#endif

