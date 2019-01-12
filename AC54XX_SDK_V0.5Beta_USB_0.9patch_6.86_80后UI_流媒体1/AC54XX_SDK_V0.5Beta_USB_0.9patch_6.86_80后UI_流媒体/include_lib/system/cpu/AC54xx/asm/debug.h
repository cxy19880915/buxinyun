#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "generic/typedef.h"
//LSB
#define DBG_SD0C                0x00
#define DBG_SD0D                0x01
#define DBG_SD1C                0x02
#define DBG_SD1D                0x03
#define DBG_SD2C                0x04
#define DBG_SD2D                0x05
//
#define DBG_ALNK                0x10
#define DBG_AUDIO               0x11
#define DBG_PAP                 0x12

#define DBG_CHECKSUM            0x14       //no write
#define DBG_ETH                 0x15
#define DBG_SHA1                0x16
//
#define DBG_FUSB                0x20
#define DBG_HUSB0               0x21
#define DBG_HUSB1               0x22
//
#define DBG_SPI0                0x30
#define DBG_SPI1                0x31
#define DBG_SPI2                0x32
//
#define DBG_UART0_WR            0x40
#define DBG_UART0_RD            0x41
#define DBG_UART1_WR            0x42
#define DBG_UART1_RD            0x43
#define DBG_UART2_WR            0x44
#define DBG_UART2_RD            0x45
#define DBG_UART3_WR            0x46
#define DBG_UART3_RD            0x47
//
#define DBG_IIC0                0x50       //no write
#define DBG_IIC1                0x51       //no write

//HSB AXI
#define DBG_IMC_CH0             0x80
#define DBG_IMC_CH1             0x81
#define DBG_IM

void debug_init();
/*
×¢Òâ£º²»¿ÉÒÔÊ¹ÓÃÒ»žöŽ°¿ÚÑ¡ÔñÒ»Æ¬ÄÚŽæÔÊÐíÍâÉèAÐŽ£¬Í¬Ê±Ê¹ÓÃÁíÒ»žöŽ°¿ÚœûÖ¹ÍâÉèBÐŽ¡£

    limit_index:
        debugŽ°¿ÚË÷Òý
    low_addr:
        Ž°¿ÚµÄµÍµØÖ·
    high_addr:
        Ž°¿ÚµÄžßµØÖ·
    is_allow_write:
        1--Öž¶šÍâÉèÐŽŽ°¿ÚÄÚ²¿²»Ž¥·¢Òì³£
        0--ÈÎÒâÍâÉèÐŽŽ°¿ÚÄÚ²¿Ž¥·¢Òì³£
        µ±²ÎÊý¡Ÿis_allow_write¡¿Îª0µÄÊ±ºò£¬²ÎÊý¡Ÿdev¡¿ÎÞÐ§
    dev:
        Öž¶šÍâÉèµÄ±àºÅ
 */
u32 dev_write_range_limit(u32 limit_index, //debugŽ°¿ÚË÷Òý
                          void *low_addr,  //·¶Î§ÄÚµÄµÍµØÖ·
                          void *high_addr, //Ž°¿ÚµÄžßµØÖ·
                          u32 is_allow_write,  //1--Öž¶šÍâÉèÐŽŽ°¿ÚÄÚ²¿²»Ž¥·¢Òì³£ 0--ÈÎÒâÍâÉèÐŽŽ°¿ÚÄÚ²¿Ž¥·¢Òì³£ µ±²ÎÊý¡Ÿis_allow_write¡¿Îª0µÄÊ±ºò£¬²ÎÊý¡Ÿdev¡¿ÎÞÐ§
                          u32 dev  //Öž¶šÍâÉèµÄ±àºÅ
                         );

u32 cpu0_write_range_limit(u32 limit_index, void *low_addr, void *high_addr);
u32 cpu1_write_range_limit(u32 limit_index, void *low_addr, void *high_addr);
u32 cpu_write_range_limit(void *low_addr, u32 win_size);
void pc0_rang_limit(void *low_addr, void *high_addr);
void pc1_rang_limit(void *low_addr, void *high_addr);
void pc_rang_limit(void *low_addr, void *high_addr);
void exception_isr(void);
u32 sdr_write_range_limit(void *low_addr, u32 win_size, u32 is_allow_write, u32 dev);
void debug_clear();

#define     dev_write_capture(laddr,win_size,is_allow_write,dev)	dev_write_range_limit(-1,\
        (void*)(laddr),\
        ((void*)laddr)+win_size,\
        (is_allow_write),\
        (dev))


extern u32 text_rodata_begin;
extern u32 text_rodata_end;
#define line_inf printf("%s %s %d \r\n" ,__FILE__ ,__FUNCTION__ ,__LINE__)
#define IO_C  PORTH_DIR &=~BIT(15) ;PORTH_OUT^=BIT(15)
#define IO_L  PORTH_DIR &=~BIT(15) ;PORTH_OUT&=~BIT(15)
#define IO_H  PORTH_DIR &=~BIT(15) ;PORTH_OUT|=BIT(15)

#define IO1_C  PORTH_DIR &=~BIT(13) ;PORTH_OUT^=BIT(13)
#define IO1_L  PORTH_DIR &=~BIT(13) ;PORTH_OUT&=~BIT(13)
#define IO1_H  PORTH_DIR &=~BIT(13) ;PORTH_OUT|=BIT(13)

#endif
