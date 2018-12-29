#ifndef _PAP_H_
#define _PAP_H_

#include "typedef.h"
#include "asm/cpu.h"


#define PAP_WAIT()			do{while(!(PAP_CON&BIT(7)));PAP_CON |= BIT(6);}while(0)//等PND并清PND
#define PAP_WAIT_NCLR()     do{while(!(PAP_CON&BIT(7)));}while(0)   //等PND但不清PND

#define PAPDEN  14  //PAP接口数据信号引脚(PGx)使能
#define PAPREN  13  //PAP接口读信号引脚(PH4)使能
#define PAPWEN  12  //PAP接口写信号引脚(PH3)使能

#define PAP_IOS         0//0:PG/PH as PAP singal 1:PC/PD as PAP singal
#define PAP_AIOS        0//0:PH3 as WR,PH4 as RD 1:PG2 as WR,PG4 as RD
#define PAP_PORT_EN()   IOMC0 |= ((1L<<PAPDEN)|(1L<<PAPREN)|(1L<<PAPWEN))   //使能PAP接口
#define PAP_PORT_DIS()  IOMC0 &=~((1L<<PAPDEN)|(1L<<PAPREN)|(1L<<PAPWEN))   //使能PAP接口
#define PAP_PORT_SEL()  IOMC0 &=~ (3<<16);IOMC0 |= (PAP_IOS<<16)|(PAP_AIOS<<17)


#define PAP_EXT_EN()    PAP_CON |= BIT(16)      //使能PAP扩展模式
#define PAP_EXT_DIS()   PAP_CON &= ~BIT(16)     //禁止PAP扩展模式
#define PAP_EXT_M2L()   PAP_CON |= BIT(17)  //扩展模式顺序MSB到LSB
#define PAP_EXT_L2M()   PAP_CON &= ~BIT(17) //扩展模式顺序LSB到MSB
#define PAP_IE(x)       PAP_CON = (PAPCON & ~BIT(18)) | ((x & 0x1)<<18)

//pap config

#define PAP_PORT_8BITS      0
#define PAP_PORT_16BITS     1
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//valid only for 16bit
#define PAP_LE              0
#define PAP_BE              1
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//valid only for 8bit
#define PAP_CYCLE_ONE       0  //ReadWrite one times
#define PAP_CYCLE_TWO       1  //ReadWrite two times
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define PAP_GROUP_PG_PH     0
#define PAP_GROUP_PC_PD     1
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//valid only for GROUP_PG_PH
#define PAP_PH3_PH4         0
#define PAP_PG2_PG4         1
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct pap_info {
    u8 datawidth;
    u8 endian;
    u8 cycle;
    u8 timing_setup;  //数据建立时间(2bits)
    u8 timing_hold;	  //数据保持时间(2bits)
    u8 timing_width;  //读/写使能信号宽度  系统时钟 0:16个 1:1个 2:2个 依此类推(4bits)
    u8 group_sel;
    u8 wr_rd_sel;
};

extern const struct device_operations pap_dev_ops;

#endif
