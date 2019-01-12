
#ifndef _AV_10_H_
#define _AV_10_H_

#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "generic/jiffies.h"
enum {
    PAL_MODE = 0,
    NTSC_MODE,
    OTHER_MODE,
};
//===================================================================================//
//
//  sfr define
//
//===================================================================================//
#define AV10_PORTA_OUT       0x00
#define AV10_PORTA_IN        0x01
#define AV10_PORTA_DIR       0x02
#define AV10_PORTA_DIE       0x03
#define AV10_PORTA_PU        0x04
#define AV10_PORTA_PD        0x05
#define AV10_PORTA_HD        0x06

#define AV10_PORTB_OUT       0x08
#define AV10_PORTB_IN        0x09
#define AV10_PORTB_DIR       0x0a
#define AV10_PORTB_DIE       0x0b
#define AV10_PORTB_PU        0x0c
#define AV10_PORTB_PD        0x0d
#define AV10_PORTB_HD        0x0e

#define AV10_PORTC_OUT       0x10
#define AV10_PORTC_IN        0x11
#define AV10_PORTC_DIR       0x12
#define AV10_PORTC_DIE       0x13
#define AV10_PORTC_PU        0x14
#define AV10_PORTC_PD        0x15
#define AV10_PORTC_HD        0x16

#define AV10_CLK_CON0        0x20
#define AV10_CLK_CON1        0x21
#define AV10_PLL_CON         0x22
#define AV10_PLL_NF_H        0x23
#define AV10_PLL_NF_L        0x24
#define AV10_PLL_NR_H        0x25
#define AV10_PLL_NR_L        0x26
#define AV10_LDO_CON0        0x27
#define AV10_LDO_CON1        0x28
#define AV10_CHIP_ID         0x29
#define AV10_CHIP_VER        0x2a

#define AV10_CVD_CON0        0x30
#define AV10_CVD_CON1        0x31
#define AV10_CVD_CON2        0x32
#define AV10_CVD_CON3        0x33
#define AV10_CVD_CON4        0x34
#define AV10_CVD_CON5        0x35
#define AV10_CVD_CON6        0x36

#define AV10_AFE_CON0        0x38
#define AV10_AFE_CON1        0x39
#define AV10_AFE_CON2        0x3a
#define AV10_AFE_CON3        0x3b
#define AV10_AFE_CON4        0x3c
#define AV10_AFE_CON5        0x3d
#define AV10_AFE_CON6        0x3e
#define AV10_AFE_CON7        0x3f

#define AV10_SMP_BLVL_H      0x40
#define AV10_SMP_BLVL_L      0x41
#define AV10_SMP_GAIN_H      0x42
#define AV10_SMP_GAIN_L      0x43
#define AV10_GCVT_OFFS       0x44
#define AV10_GCVT_GAIN       0x45

#define AV10_CBS_U_COS       0x50
#define AV10_CBS_U_SIN       0x51
#define AV10_CBS_V_COS       0x52
#define AV10_CBS_V_SIN       0x53
#define AV10_CBS_Y_GAIN      0x54
#define AV10_CBS_Y_OFFS      0x55
#define AV10_CBS_U_GAIN      0x56
#define AV10_CBS_U_OFFS      0x57
#define AV10_CBS_V_GAIN      0x58
#define AV10_CBS_V_OFFS      0x59

#define AV10_CBS_GMA_Y0      0x60
#define AV10_CBS_GMA_Y1      0x61
#define AV10_CBS_GMA_Y2      0x62
#define AV10_CBS_GMA_Y3      0x63
#define AV10_CBS_GMA_Y4      0x64
#define AV10_CBS_GMA_Y5      0x65
#define AV10_CBS_GMA_Y6      0x66
#define AV10_CBS_GMA_Y7      0x67
#define AV10_CBS_GMA_Y8      0x68
#define AV10_CBS_GMA_Y9      0x69
#define AV10_CBS_GMA_Y10     0x6a
#define AV10_CBS_GMA_Y11     0x6b
#define AV10_CBS_GMA_Y12     0x6c
#define AV10_CBS_GMA_Y13     0x6d
#define AV10_CBS_GMA_Y14     0x6e
#define AV10_CBS_GMA_Y15     0x6f

#define AV10_AFET_CON0       0x70
#define AV10_AFET_WTH0       0x71
#define AV10_AFET_WTH1       0x72
#define AV10_AFET_TLMTH      0x73
#define AV10_AFET_TLMTL      0x74
#define AV10_AFET_FLMTH      0x75
#define AV10_AFET_FLMTL      0x76
#define AV10_AFET_DAT0_H     0x77
#define AV10_AFET_DAT0_L     0x78
#define AV10_AFET_DAT1_H     0x79
#define AV10_AFET_DAT1_L     0x7a
#define AV10_MTS_CON0        0x7b

extern s32 av10_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio);
extern s32 av10_initialize(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);
extern s32 av10_power_ctl(u8 isp_dev, u8 is_work);
extern s32 av10_set_output_size(u16 *width, u16 *height, u8 *freq);

extern void av10_mode_cfg(u8 mode_type);
extern u8 av10_mode_det(void);
extern u8 av10_singal_det(void);

#endif
