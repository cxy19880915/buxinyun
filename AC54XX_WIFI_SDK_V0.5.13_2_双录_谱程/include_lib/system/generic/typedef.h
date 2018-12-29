/*************************************************************
File: typedef.h
Author:Juntham
Discriptor:
    数据类型重定义
Version:
Date：
*************************************************************/
#ifndef _typedef_h_
#define _typedef_h_


#include "asm/cpu.h"



#ifdef __GNUC__
#define ALIGNE(x) 		__attribute__((aligned(x)))
#define sec(x) 			__attribute__((section(#x),used))
#define _GNU_PACKED_	__attribute__((packed))
#else
#define sec(x)
#define AT(x)
#define SET(x)
#define _GNU_PACKED_
#endif


#if CPU_ENDIAN == LITTLE_ENDIAN
//#define ntohl(x) (u32)((x>>24)|((x>>8)&0xff00)|(x<<24)|((x&0xff00)<<8))
//#define ntoh(x) (u16)((x>>8&0x00ff)|x<<8&0xff00)
#undef ntohl
#undef ntoh
#define ntohl(x) (u32)((((u32)(x))>>24) | ((((u32)(x))>>8)&0xff00) | (((u32)(x))<<24) | ((((u32)(x))&0xff00)<<8))
#define ntoh(x) (u16)((((u32)(x))>>8&0x00ff) | (((u32)(x))<<8&0xff00))

#define NTOH(x) (x) = ntoh(x)
#define NTOHL(x) (x) = ntohl(x)
#define LD_WORD(ptr)        (u16)(*(u16*)(u8*)(ptr))
#define LD_DWORD(ptr)        (u32)(*(u32*)(u8*)(ptr))
#define ST_WORD(ptr,val)    *(u16*)(u8*)(ptr)=(u16)(val)
#define ST_DWORD(ptr,val)    *(u32*)(u8*)(ptr)=(u32)(val)
#else
#define ntohl(x) (x)
#define ntoh(x) (x)
#define NTOH(x) (x) = ntoh(x)
#define NTOHL(x) (x) = ntohl(x)
#endif



#define FALSE    	0
#define TRUE    	1
#define false    	0
#define true    	1

#ifndef NULL
#define NULL    	(void *)0
#endif

#ifndef __ASSEMBLY__
typedef struct S_PICTURE_SIZE {
    u16 w;
    u16 h;
} pic_size_t;
#endif


#define     BIT(n)              (1UL << (n))
#define     BitSET(REG,POS)     ((REG) |= (1L << (POS)))
#define     BitCLR(REG,POS)     ((REG) &= (~(1L<< (POS))))
#define     BitXOR(REG,POS)     ((REG) ^= (~(1L << (POS))))
#define     BitCHK_1(REG,POS)   (((REG) & (1L << (POS))) == (1L << (POS)))
#define     BitCHK_0(REG,POS)   (((REG) & (1L << (POS))) == 0x00)
#define     testBit(REG,POS)    ((REG) & (1L << (POS)))

#define     clrBit(x,y)         (x) &= ~(1L << (y))
#define     setBit(x,y)         (x) |= (1L << (y))


#define readb(addr)   *((volatile unsigned char*)(addr))
#define readw(addr)   *((volatile unsigned short *)(addr))
#define readl(addr)   *((volatile unsigned long*)(addr))

#define writeb(addr, val)  *((volatile unsigned char*)(addr)) = (u8)(val)
#define writew(addr, val)  *((volatile unsigned short *)(addr)) = (u16)(val)
#define writel(addr, val)  *((volatile unsigned long*)(addr)) = (u32)(val)

#define ALIGN_4BYTE(size)   ((size+3)&0xfffffffc)



#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


#define ARRAY_SIZE(array)  (sizeof(array)/sizeof(array[0]))


#define likely(x) 	__builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define SFR(sfr, start, len, dat) \
	(sfr = (sfr & ~((~(0xffffffff << (len))) << (start))) | \
	 (((dat) & (~(0xffffffff << (len)))) << (start)))


#include "errno-base.h"
#include "string.h"
#include "system/malloc.h"


void delay(unsigned int);

void delay_us(unsigned int);


#endif



