/*************************************************************************
	> File Name: lib/include/cpu/dv16/asm/jpeg_common.h
	> Author:
	> Mail:
	> Created Time: Mon 31 Oct 2016 11:09:26 AM HKT
 ************************************************************************/

#ifndef _JPEG_COMMON_H_
#define _JPEG_COMMON_H_
#include "typedef.h"

typedef struct jpg_yuv_buffer {
    u8 *y;
    u8 *u;
    u8 *v;
} jpg_yuv_addr;


#define JPEG0_SFR_BEGIN  &JPG0_CON0
#define JPEG1_SFR_BEGIN  &JPG1_CON0


typedef struct JPEG_SFR {
    volatile u32 CON0            ;
    volatile u32 CON1            ;
    volatile u32 CON2            ;
    volatile u32 YDCVAL          ;        //write only
    volatile u32 UDCVAL          ;        //write only
    volatile u32 VDCVAL          ;        //write only
    volatile u32 YPTR0           ;        //write only
    volatile u32 UPTR0           ;        //write only
    volatile u32 VPTR0           ;        //write only
    volatile u32 YPTR1           ;        //write only
    volatile u32 UPTR1           ;        //write only
    volatile u32 VPTR1           ;        //write only
    volatile u32 BADDR           ;        //write only
    volatile u32 BCNT            ;
    volatile u32 MCUCNT          ;
    volatile u32 PRECNT          ;        //write only
    volatile u32 YUVLINE         ;        //write only
    volatile u32 CFGRAMADDR      ;        //write only
//u32 rev[0x1c00-0x12] ;
    volatile u32 CFGRAMVAL       ;  //0x1c00*4
    volatile u32 PTR_NUM         ;
} JPEG_SFR ;


#define be16_to_cpu(x) (((x)[0] << 8) | (x)[1])
#endif
