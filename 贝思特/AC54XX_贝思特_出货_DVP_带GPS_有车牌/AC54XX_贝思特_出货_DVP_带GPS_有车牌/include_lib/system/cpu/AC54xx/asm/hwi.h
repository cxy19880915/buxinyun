/*******************************************************************************************
 File Name: HWI.h

 Version: 1.00

 Discription:


 Author:yulin deng

 Email :flowingfeeze@163.com

 Date:2013-09-14 16:47:39

 Copyright:(c)JIELI  2011  @ , All Rights Reserved.
*******************************************************************************************/

#ifndef ASM_HWI_H
#define ASM_HWI_H

#include "typedef.h"

#define EXCEP_INT	      	0
#define C0_TTIMER_INT 		8
#define C1_TTIMER_INT 		9
#define TIME0_INT 			10
#define TIME1_INT 			11
#define TIME2_INT 			12
#define TIME3_INT 			13
#define TIME4_INT           14
#define SPI0_INT            16
#define SPI1_INT            17
#define SPI2_INT            18
#define SD0_INT       		20
#define SD1_INT       		21
#define SD2_INT       		22
#define UART0_INT           24
#define UART1_INT           25
#define UART2_INT           26
#define UART3_INT			27
#define IIC0_INT			30
#define IIC1_INT			31
#define IOPROT_INT          32
#define PAP_INT				33
#define DMA_COPY_INT		34
#define AUDIO_INT			35
#define SARADC_INT			36
#define LVD_INT				40
#define WATCHDOG_INT 		41
#define OSA_INT				42
#define RTC_INT             43
#define HUSB0_SOF_INT       50
#define HUSB0_CTL_INT       51
#define HUSB1_SOF_INT       52
#define HUSB1_CTL_INT       53
#define HUSB0_SOF           50
#define HUSB0_CTL           51
#define HUSB1_SOF           52
#define HUSB1_CTL           53
#define MTPMW_INT           56
#define ETH_INT             60
#define AVC_INT 			64
#define JPG0_INT            65
#define ISC_INT             66
#define ISP0_INT            67
#define ISP1_INT            68
#define IMC_INT             69
#define IMB_INT			    70
#define IMD_INT				71
#define CSI_INT			    72
#define DSI_INT             73
#define JIVE_INT            74
#define JLMD_INT            75

#define ISR_ENTRY 0x3f0fc00
/*#define ISR_ENTRY 0x3f13c00*/

void sfc_set_unenc_addr_range(u32 low_addr, u32 high_addr);
void bit_clr_ie(unsigned char index);
void bit_set_ie(unsigned char index);
void reg_set_ip(unsigned char index, unsigned char dat);
void request_irq(unsigned char index, unsigned char priority, void (*handler)(void), unsigned char cpu_id);
bool check_pending(tu8 index);
void interrupt_init() ;
u8 set_cpu1_enter_addr(void (*fun)());
//---------------------------------------------//
// interrupt enable
//---------------------------------------------//

#ifdef IRQ_TIME_COUNT_EN
void irq_handler_enter(int irq);

void irq_handler_exit(int irq);

void irq_handler_times_dump();
#else

#define irq_handler_enter(irq)      do { }while(0)
#define irq_handler_exit(irq)       do { }while(0)
#define irq_handler_times_dump()    do { }while(0)

#endif

#endif

