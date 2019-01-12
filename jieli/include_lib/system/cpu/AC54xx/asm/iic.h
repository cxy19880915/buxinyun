/*******************************************************************************************
  File Name: iic.h

Version: 1.00

Discription:  IIC 驱动

Author:yulin deng

Email :flowingfeeze@163.com

Date:星期五, 四月 19 2013

Copyright:(c)JIELI  2011  @ , All Rights Reserved.
 *******************************************************************************************/
#ifndef ASM_IIC_H
#define ASM_IIC_H

#include "typedef.h"
#include "asm/cpu.h"
#include "device/iic.h"



#define IIC_OUTPORT_NUM 	4


struct iic_reg {
    volatile u16 con;
    volatile u16 rvs0;
    volatile u8 buf;
    volatile u8 rvs1[3];
    volatile u8 baud;
    volatile u8 rvs2[3];
    volatile u32 dma_adr;
    volatile u16 dma_cnt;
    volatile u16 rvs3;
    volatile u8 dma_nrate;
    volatile u8 rvs4[3];
};

struct iic_outport {
    u8  clk_pin;
    u8  dat_pin;
    u32 value;
};


struct hardware_iic {
    u8 clk_pin;
    u8 dat_pin;
    u32 baudrate;
    u32 occupy_reg;
    u32 occupy_io_mask;
    u32 occupy_io_value;
    struct iic_reg *reg;
    struct iic_outport outport_map[IIC_OUTPORT_NUM];
};

#define iic_pnd(iic)             	(iic->reg->con & BIT(15))
#define iic_clr_pnd(iic)         	iic->reg->con |= BIT(13)//清空普通pending
#define iic_ack_in(iic)             (iic->reg->con & BIT(11))
#define iic_pu_ack_out(iic)         iic->reg->con |= BIT(10)
#define iic_pd_ack_out(iic)         iic->reg->con &= ~BIT(10)
#define iic_clr_start(iic)          iic->reg->con |= BIT(8)//清空起始位标志


#define iic_clr_stop(iic)           iic->reg->con |= BIT(12)//清空结束位标志
#define iic_add_start_bit(iic)      iic->reg->con |= BIT(2)//加起始位
#define iic_add_end_bit(iic)        iic->reg->con |= BIT(3)//加结束位
#define iic_set_ack(iic)         	iic->reg->con &= ~BIT(10)


struct hw_iic_platform_data {
    struct iic_platform_data head;
    struct  hardware_iic iic;
};


#define HW_IIC0_PLATFORM_DATA_BEGIN(data0) \
	static const struct hw_iic_platform_data data0 = { \
		.head = { \
			.type = IIC_TYPE_HW, \
		}, \
		.iic = {


#define HW_IIC0_PLATFORM_DATA_END() \
			.occupy_reg = (u32)&IOMC1, \
			.occupy_io_mask = ~(BIT(6)|BIT(7)), \
			.reg = (struct iic_reg *)&IIC0_CON, \
			.outport_map = { \
				{IO_PORTG_06, IO_PORTG_07, 0}, \
				{IO_PORTH_12, IO_PORTH_14, BIT(6)}, \
				{IO_PORTB_04, IO_PORTB_03, BIT(7)}, \
				{IO_PORTD_14, IO_PORTD_15, (BIT(6) | BIT(7))}, \
			}, \
		}, \
	};



#define HW_IIC1_PLATFORM_DATA_BEGIN(data1) \
	static const struct hw_iic_platform_data data1 = { \
		.head = { \
			.type = IIC_TYPE_HW, \
		}, \
		.iic = {



#define HW_IIC1_PLATFORM_DATA_END() \
			.occupy_reg = (u32)&IOMC3, \
			.occupy_io_mask = ~(BIT(18)|BIT(19)), \
			.reg = (struct iic_reg *)&IIC1_CON, \
			.outport_map = { \
				{IO_PORTB_00, IO_PORTB_01, 0}, \
				{IO_PORTA_02, IO_PORTA_01, BIT(18)}, \
				{IO_PORTE_02, IO_PORTE_03, BIT(19)}, \
				{IO_PORTE_04, IO_PORTE_05, (BIT(18) | BIT(19))}, \
			}, \
		}, \
	};






#endif

