
/*******************************************************************************************
 File Name: GC1004.c

 Version: 1.00

 Discription: GC1004

 Author:xuhuajun

 Email :

 Date:2014.8.19

 Copyright:(c)JIELI  2011  @ , All Rights Reserved.
*******************************************************************************************/
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "gc1004.h"
#include "asm/isp_alg.h"

static u32 reset_gpio[2] = {-1, -1};


extern void *GC1004_get_ae_params();
extern void *gc1004_get_awb_params();
extern void *gc1004_get_iq_params();

extern void gc1004_ae_ev_init(u32 fps);

u32 gc1004_pclk;
u32 gc1004_hblank;
u32 gc1004_vblank;
static u32 gc_pll_div5b;


typedef struct {
    u8 addr;
    u8 value;
} Sensor_reg_ini;

Sensor_reg_ini GC1004_INI_REG[] = {
#if (IS_GC1014==0)
    {0xfe, 0x80},
    {0xfe, 0x80},
    {0xfe, 0x80},
    {0xf2, 0x0f},//sync_pad_io_ebi
    {0xf6, 0x00},//up down
    {0xfc, 0xc6},

    //{0xf7, 0xb9},//pll enable
    //{0xf8, 0x03},//Pll mode 2
    {0xf7, (0xb9 | (GC_PLL_DIV2 << 1))}, //pll enable
    {0xf8, (GC_PLL_DIV5B & 0x1f)}, //Pll mode 2

    //{0xf8, 0x08},//Pll mode 2
    {0xf9, 0x2e},//[0] pll enable
    {0xfa, 0x20},//div
    {0xfe, 0x00},
    ////////////////   ANALOG & CISCTL	 ////////////////
    {0x03, 0x02},
    {0x04, 0xb5},
    {0x05, 0x02},
    {0x06, 0x77},
    {0x07, 0x00},
    {0x08, 0x0c},
    //{0x08, 0x5a},

    //{0x0d, 0x02},
    //{0x0e, 0xd8},
    //{0x0f, 0x05},
    //{0x10, 0x10},
    {0x11, (REG_SH_DELAY) >> 8},
    {0x12, (REG_SH_DELAY) & 0xFF},
    {0x17, 0x14},
    {0x18, 0x0a},
    {0x19, 0x06},
    {0x1a, 0x09},
    {0x1b, 0x4f},
    {0x1c, 0x21},
    {0x1d, 0xe0},//f0
    {0x1e, 0x7c},
    {0x1f, 0x08},//08//comv_r
    {0x20, 0xa9},//×ó±ßÊúÏß
    {0x21, 0x6f},//2f//20//reg
    {0x22, 0xb0},
    {0x23, 0x32},//38
    {0x24, 0x2f},//PAD drive
    {0x25, 0xc0},
    {0x2a, 0x00},
    {0x2c, 0xb0},
    {0x2d, 0x0f},
    {0x2e, 0xf0},
    {0x2f, 0x1f},//exp not work


    //////////////////////	 ISP   //////////////////////
    {0xfe, 0x00},
    {0x8a, 0x00},
    {0x8c, 0x02},
    {0x8e, 0x02},//luma value

#if (GC1004_OUTPUT_W==1120 && GC1004_OUTPUT_H==640)
    {0x90, 0x01}, //crop window mode
    {0x91, 0x00},
    {0x92, 0x28},
    {0x93, 0x00},
    {0x94, 0x38},
    {0x95, 0x02},
    {0x96, 0x80},
    {0x97, 0x04},
    {0x98, 0x60},
#elif (GC1004_OUTPUT_W==1280 && GC1004_OUTPUT_H==720)

    {0x90, 0x01}, //crop window mode
    {0x94, 0x02},
    {0x95, 0x02},
    {0x96, 0xd0},
    {0x97, 0x05},
    {0x98, 0x00},
#elif (GC1004_OUTPUT_W==800 && GC1004_OUTPUT_H==448)

    {0x90, 0x01}, //crop window mode
    {0x91, 0x00},
    {0x92, 0x88},
    {0x93, 0x00},
    {0x94, 0xd8},
    {0x95, 0x01},
    {0x96, 0xc0},
    {0x97, 0x03},
    {0x98, 0x20},
#else
#error "GC1004.c not support current output size"
#endif
    //////////////////////	 MIPI	/////////////////////

    {0xfe, 0x03},
    {0x01, 0x00},
    {0x02, 0x00},
    {0x03, 0x00},
    {0x06, 0x00},
    {0x10, 0x00},
    {0x15, 0x00},
    //////////////////////	 BLK	/////////////////////
    {0xfe, 0x03},
    {0x18, 0x02},
    {0x1a, 0x01},
    {0x40, 0x23},
    {0x5e, 0x00},
    {0x66, 0x20},
    //////////////////////	 Dark SUN	/////////////////////
    {0xfe, 0x00},//dark sun
    {0xfe, 0x02},
    {0x49, 0x23},
    {0x04, 0x00},
    {0xfe, 0x00},
    //////////////////////	 Gain	/////////////////////
    //{0xb0, 0x40}, //reis 201503
    {0xb0, 0x40},
    {0xb1, 250 >> 8},
    {0xb2, 250 & 0xff},
    {0xb6, 0x02},
    {0xb3, 0x40},
    {0xb4, 0x40},
    {0xb5, 0x40},
    {0xfe, 0x00},

    //////////////////////8bit
    {0x89, 0x80},
    //////////////////////exposure
    {0x03, 0x02},
    {0x04, 0xb5},
    //////////////////////div & dummy
#if 0 //reis
    {0xf7, 0x33},
    {0xf8, 0x08},
    {0xfa, 0x00},//div//
    //{0xfa, 0x11},//div
#else
    {0xfa, GC_DCLK_DIV},
#endif
    {0xfd, 0x11},
    //{0x05, 0x01},
    //{0x06, 0x77},
    {0x05, (REG_HBLANK >> 8)},
    {0x06, (REG_HBLANK & 0xFF)},
    {0x07, REG_VBLANK >> 8},
    {0x08, (REG_VBLANK & 0xFF)},

    {0x8c, 0x02},
    {0xd2, 0x90},
    {0xd0, 0xcb},
    {0xd3, 0x48},
// 		//not crop windows
    //{0x90, 0x01}, //!crop window mode
//

    {0x0d, REG_WIN_HEIGHT >> 8},
    {0x0e, REG_WIN_HEIGHT & 0xff},
    {0x0f, REG_WIN_WIDTH >> 8},
    {0x10, REG_WIN_WIDTH & 0xff},
//        {0x8b, 0xb2},
//        {0x8d, 0xf3},
#else
/////////////////////////////////////////////////////
//////////////////////,0x SYS,0x //////////////////////
/////////////////////////////////////////////////////
    {0xfe, 0x80},
    {0xfe, 0x80},
    {0xfe, 0x80},
    {0xf2, 0x02},
    {0xf6, 0x00},
    {0xfc, 0xc6},
    {0xf7, (0xb9 | (GC_PLL_DIV2 << 1))}, //pll enable
    {0xf8, (GC_PLL_DIV5B & 0x1f)}, //Pll mode 2
    {0xf9, 0x0e}, //{0xf9,0x2e},//20140901
    {0xfa, GC_DCLK_DIV},
    {0xfe, 0x00},
/////////////////////////////////////////////////////
////////////////,0x ANALOG & CISCTL,0x ////////////////
/////////////////////////////////////////////////////
    {0x03, 0x02},
    {0x04, 0xb5},
    {0x05, (REG_HBLANK >> 8)},
    {0x06, (REG_HBLANK & 0xFF)},
    {0x07, REG_VBLANK >> 8},
    {0x08, (REG_VBLANK & 0xFF)},
    {0x0d, REG_WIN_HEIGHT >> 8},
    {0x0e, REG_WIN_HEIGHT & 0xff},
    {0x0f, REG_WIN_WIDTH >> 8},
    {0x10, REG_WIN_WIDTH & 0xff},
    {0x11, (REG_SH_DELAY) >> 8},
    {0x12, (REG_SH_DELAY) & 0xFF},
    {0x17, 0x14}, //reverse
    {0x19, 0x06},
    {0x1b, 0x4f},
    {0x1c, 0x41},
    {0x1d, 0xe0},
    {0x1e, 0xfc},
    {0x1f, 0x38},
    {0x20, 0x81},
    {0x21, 0x2f}, //6f//2f
    {0x22, 0xc0},
    {0x23, 0xf2},
    {0x24, 0x2f},
    {0x25, 0xd4},
    {0x26, 0xa8},
    {0x29, 0x3f}, //54//3f
    {0x2a, 0x00},
    {0x2c, 0xd0}, //DC--30fps,0x d0---50fps
    {0x2d, 0x0f},
    {0x2e, 0x00},
    {0x2f, 0x1f},
    {0xcc, 0x25},
    {0xce, 0xf3},
    {0x3f, 0x08},
    {0x30, 0x00},
    {0x31, 0x01},
    {0x32, 0x02},
    {0x33, 0x03},
    {0x34, 0x04},
    {0x35, 0x05},
    {0x36, 0x06},
    {0x37, 0x07},
    {0x38, 0x0f},
    {0x39, 0x17},
    {0x3a, 0x1f},
/////////////////////////////////////////////////////
//////////////////////,0x ISP,0x //////////////////////
/////////////////////////////////////////////////////
    {0xfe, 0x00},
    {0x8a, 0x00},
    {0x8c, 0x02},
    {0x8e, 0x02},
    {0x8f, 0x15}, //added on 20150901 for 50fps
    {0x90, 0x01}, //crop window mode
    {0x94, 0x02},
    {0x95, GC1004_OUTPUT_H >> 8},
    {0x96, GC1004_OUTPUT_H & 0xff},
    {0x97, GC1004_OUTPUT_W >> 8},
    {0x98, GC1004_OUTPUT_W & 0xff},
/////////////////////////////////////////////////////
//////////////////////	 MIPI	/////////////////////
/////////////////////////////////////////////////////
    {0xfe, 0x03},
    {0x01, 0x00},
    {0x02, 0x00},
    {0x03, 0x00},
    {0x06, 0x00},
    {0x10, 0x00},
    {0x15, 0x00},
/////////////////////////////////////////////////////
//////////////////////	 BLK	/////////////////////
/////////////////////////////////////////////////////
    {0xfe, 0x00},
    {0x18, 0x0a},
    {0x1a, 0x11},
    {0x40, 0x2b}, //2b
    {0x5e, 0x00},
    {0x66, 0x80},
/////////////////////////////////////////////////////
////////////////////// Dark SUN /////////////////////
/////////////////////////////////////////////////////
#if 1
    {0xfe, 0x00},
    {0xcc, 0x25},
    {0xce, 0xf3},
    {0x3f, 0x08},
#else
    {0xfe, 0x02},
    {0x49, 0x23},
    {0xa4, 0x00},
#endif
/////////////////////////////////////////////////////
//////////////////////	 Gain	/////////////////////
/////////////////////////////////////////////////////
    {0xfe, 0x00},
    {0xb0, 0x40},
    {0xb3, 0x40},
    {0xb4, 0x40},
    {0xb5, 0x40},
    {0xb6, 0x00},

    {0x89, 0x80}, //comment on 20150901
/////////////////////////////////////////////////////
//////////////////////,0x pad enable,0x ///////////////
/////////////////////////////////////////////////////
    {0xf2, 0x0f},
    {0xfe, 0x00},




#endif
};


static void *iic = NULL;


unsigned char wrGC1004Reg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x78)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }
__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;

}

unsigned char rdGC1004Reg(unsigned char regID, unsigned char *regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x78)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x79)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat)) {
        ret = 0;
        goto __rend;
    }
__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}

unsigned char wrGC1004RegArray(u8 *array, u32 size)
{

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    dev_ioctl(iic, IIC_IOCTL_SET_NORMAT_RATE, 2);
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    dev_write(iic, array, size);
    return 1;

}


/*************************************************************************************************
    sensor api
*************************************************************************************************/



void GC1004_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;


    GC1004_set_output_size(width, height, frame_freq);

    for (i = 0; i < sizeof(GC1004_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrGC1004Reg(GC1004_INI_REG[i].addr, GC1004_INI_REG[i].value);
    }
#if 1
    if (*frame_freq == 15) {
        gc1004_hblank = 0x58c;
        gc1004_vblank = 0x10;
        gc_pll_div5b = 0x03;//0x3;
    } else if (*frame_freq == 30) {
#if 1 //for larger hblank;
        gc1004_hblank = 0x27c;
        gc1004_vblank = 0x10;
        gc_pll_div5b = 0x4;
#else
        gc1004_hblank = 0x177;
        gc1004_vblank = 0x10;
        gc_pll_div5b = 0x3;
#endif
    } else if (*frame_freq == 50) {
        gc1004_hblank = 0x1c2;
        gc1004_vblank = 0x4;
        gc_pll_div5b = 0x6;
    } else if (*frame_freq == 60) {
        gc1004_hblank = 0x18d;
        gc1004_vblank = 0x4;
        gc_pll_div5b = 0x7;
    } else {
#if 1 //for larger hblank;
        gc1004_hblank = 0x27c;
        gc1004_vblank = 0x10;
        gc_pll_div5b = 0x4;
#else
        gc1004_hblank = 0x177;
        gc1004_vblank = 0x10;
        gc_pll_div5b = 0x3;
#endif
    }
    gc1004_pclk = ((HOST_MCLK * (gc_pll_div5b + 1) / (1 + GC_PLL_DIV2)) / (2 * (GC_DCLK_DIV + 1)));
    wrGC1004Reg(0xf8, (gc_pll_div5b & 0x1f));
    wrGC1004Reg(0x05, gc1004_hblank >> 8);
    wrGC1004Reg(0x06, gc1004_hblank & 0xff);
    wrGC1004Reg(0x07, gc1004_vblank >> 8);
    wrGC1004Reg(0x08, gc1004_vblank & 0xff);
#endif

    *format = SEN_IN_FORMAT_RGGB;

    wrGC1004Reg(0x8d, 0x12);

    gc1004_ae_ev_init(*frame_freq);


    return;
}


s32 GC1004_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
#if 0
    static Sensor_reg_ini regs[] = {
        {0xfe, 0x00},
        {0x90, 0x01}, //crop window mode
        {0x91, 0x00},
        {0x92, 0x28},
        {0x93, 0x00},
        {0x94, 0x38},
        {0x95, 0x02},
        {0x96, 0x80},
        {0x97, 0x04},
        {0x98, 0x60},
    };
#else
    static Sensor_reg_ini regs[] = {
        {0xfe, 0x00},
        {0x90, 0x01}, //crop window mode
        {0x91, 0x00},
        {0x92, 0x00},
        {0x93, 0x00},
        {0x94, 0x00},
        {0x95, 0x01},
        {0x96, 0xe0},
        {0x97, 0x02},
        {0x98, 0x80},
    };
#endif

    u32 i;
    //return 0;

    if (*width == 640 && *height == 480) {

        for (i = 0; i < sizeof(regs) / sizeof(Sensor_reg_ini); i++) {
            wrGC1004Reg(regs[i].addr, regs[i].value);
        }
    }

    return 0;
}


s32 GC1004_power_ctl(u8 isp_dev, u8 is_work)
{

    return 0;
}

s32 GC1004_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    for (i = 0; i < 3; i++) { //
        rdGC1004Reg(0xf0, &pid);
        rdGC1004Reg(0xf1, &ver);
    }

    puts("Sensor PID \n");
    put_u8hex(pid);
    put_u8hex(ver);
    puts("\n");

    if (pid != 0x10 || ver != 0x04) {
        puts("\n----not GC1004-----\n");
        return -1;
    }
    puts("\n----hello GC1004-----\n");
    return 0;
}

void GC1004_reset(u8 isp_dev)
{
    u32 gpio;

    if (isp_dev == ISP_DEV_0) {
        gpio = reset_gpio[0];
    } else {
        gpio = reset_gpio[1];
    }

    gpio_direction_output(gpio, 0);
    delay(40000);
    gpio_direction_output(gpio, 1);
    delay(40000);
}


static u8 cur_sensor_type = 0xff;

s32 GC1004_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio)
{
    puts("\n\n GC1004_check \n\n");
    if (!iic) {
        if (isp_dev == ISP_DEV_0) {
            iic = dev_open("iic0", 0);
        } else {
            iic = dev_open("iic1", 0);
        }
        if (!iic) {
            return -1;
        }
    } else {
        if (cur_sensor_type != isp_dev) {
            return -1;
        }
    }
    printf("\n\n isp_dev =%d\n\n", isp_dev);

    reset_gpio[isp_dev] = _reset_gpio;

    GC1004_reset(isp_dev);

    puts("gc1004_id_check\n");
    if (0 != GC1004_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}

void resetStatic();
s32 GC1004_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n gc1004_init22 \n\n");

    GC1004_config_SENSOR(width, height, format, frame_freq);


    resetStatic();



    return 0;
}


void resetStatic()
{

}


u32 gc1004_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 line = (((gc1004_pclk / 2) * (exp_time_us)) / (gc1004_hblank + REG_SH_DELAY + (REG_WIN_WIDTH / 2) + 4));
    u32 line_time = (line * (gc1004_hblank + REG_SH_DELAY + (REG_WIN_WIDTH / 2) + 4) / (gc1004_pclk / 2));
    u32 ratio = (exp_time_us) * (1 << 10) / line_time;

    gain = (gain - 1024) * 70 / 100 + 1024;
    shutter->ae_exp_line =  line;
    shutter->ae_gain = (((gain * ratio) >> 10) & (~0xf));
    shutter->ae_exp_clk = 0;

    return 0;

}

#define GC1004_ANALOG_GAIN_1 64 // 1.00x
#define GC1004_ANALOG_GAIN_2 90 // 1.4x
#define GC1004_ANALOG_GAIN_3 118 // 1.8x
#define GC1004_ANALOG_GAIN_4 163 // 2.56x
#define GC1004_ANALOG_GAIN_5 218 // 3.40x
#define GC1004_ANALOG_GAIN_6 304 // 4.7x
#define GC1004_ANALOG_GAIN_7 438 // 6.84x
#define GC1004_ANALOG_GAIN_8 602 // 9.4x
#define GC1004_ANALOG_GAIN_9 851 // 13.2x

#define  GC1004_DGAIN_PRECISE  64

void decompose_gain(int total_gain/*q10*/, u32 *aG, u32 *dG)
{
    int iReg = (total_gain + (1 << 3)) >> 4;


    if (iReg < GC1004_ANALOG_GAIN_1) {
        iReg = GC1004_ANALOG_GAIN_1;
    }

    if ((GC1004_ANALOG_GAIN_1 <= iReg) && (iReg < GC1004_ANALOG_GAIN_2)) {
        *aG = 0;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_1;

    } else if ((GC1004_ANALOG_GAIN_2 <= iReg) && (iReg < GC1004_ANALOG_GAIN_3)) {
        *aG = 1;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_2;

    } else if ((GC1004_ANALOG_GAIN_3 <= iReg) && (iReg < GC1004_ANALOG_GAIN_4)) {
        *aG = 2;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_3;

    } else if ((GC1004_ANALOG_GAIN_4 <= iReg) && (iReg < GC1004_ANALOG_GAIN_5)) {
        *aG = 3;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_4;

    } else if ((GC1004_ANALOG_GAIN_5 <= iReg) && (iReg < GC1004_ANALOG_GAIN_6)) {
        *aG = 4;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_5;

    } else if ((GC1004_ANALOG_GAIN_6 <= iReg) && (iReg < GC1004_ANALOG_GAIN_7)) {
        *aG = 5;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_6;

    } else if ((GC1004_ANALOG_GAIN_7 <= iReg) && (iReg < GC1004_ANALOG_GAIN_8)) {
        *aG = 6;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_7;

    } else if ((GC1004_ANALOG_GAIN_8 <= iReg) && (iReg < GC1004_ANALOG_GAIN_9)) {
        *aG = 7;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_8;

    } else {
        *aG = 8;
        *dG = GC1004_DGAIN_PRECISE * iReg / GC1004_ANALOG_GAIN_9;
    }
}

static u8 gc1004_shutter_cmd[6][3] = {
    {0x78, 0xb6, 0x00},  //again
    {0x78, 0xb1, 0x00},  //dgainL
    {0x78, 0xb2, 0x00},  //dgainH
    {0x78, 0xb0, 0x00},  //ggaim
    {0x78, 0x04, 0x00},  //exptime L
    {0x78, 0x03, 0x00},   //exptime H
} ;


u32 gc1004_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 gain;
    u32 again, dgain;




    gain = (shutter->ae_gain);

    decompose_gain(gain, &again, &dgain);

    //printf("time=%d, again=%d, dgain=%d\n", shutter->ae_exp_line, again, dgain);

    gc1004_shutter_cmd[0][2] = again ;
    gc1004_shutter_cmd[1][2] = dgain >> 6 ;

    gc1004_shutter_cmd[2][2] = (dgain << 2) & 0xfc;

    gc1004_shutter_cmd[3][2] = 0x40 ;

    gc1004_shutter_cmd[4][2] = shutter->ae_exp_line & 0xFF ;
    gc1004_shutter_cmd[5][2] = (shutter->ae_exp_line >> 8) & 0x1F;

    wrGC1004RegArray(gc1004_shutter_cmd[0], 18);



    return 0;
}

void GC1004_sleep()
{

    wrGC1004Reg(0xf6, 0x01);
}

void GC1004_wakeup()
{

    wrGC1004Reg(0xf6, 0x00);
}

void GC1004_W_Reg(u16 addr, u16 val)
{
    printf("update reg%x with %x\n", addr, val);
    wrGC1004Reg((u8)addr, (u8)val);
}
u16 GC1004_R_Reg(u16 addr)
{
    u8 val;
    rdGC1004Reg((u8)addr, &val);
    return val;
}


void reset_gc10X4()
{
    gc1004_pclk = 0;
    gc1004_hblank = 0;
    gc1004_vblank = 0;
    gc_pll_div5b = 0;

}


REGISTER_CAMERA(GC1004) = {
    .logo 				= 	"GC1004",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_RGGB,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B,
    .fps         		= 	15,

    .sen_size 			= 	{GC1004_OUTPUT_W, GC1004_OUTPUT_H},
    .isp_size 			= 	{GC1004_OUTPUT_W, GC1004_OUTPUT_H},

    .cap_fps         		= 	15,
    .sen_cap_size 			= 	{GC1004_OUTPUT_W, GC1004_OUTPUT_H},
    .isp_cap_size 			= 	{GC1004_OUTPUT_W, GC1004_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	GC1004_check,
        .init 		        = 	GC1004_init,
        .set_size_fps 		=	GC1004_set_output_size,
        .power_ctrl         =   GC1004_power_ctl,

        .get_ae_params  	=	GC1004_get_ae_params,
        .get_awb_params 	=	gc1004_get_awb_params,
        .get_iq_params 	    =	gc1004_get_iq_params,

        .sleep 		        =	GC1004_sleep,
        .wakeup 		    =	GC1004_wakeup,
        .write_reg 		    =	GC1004_W_Reg,
        .read_reg 		    =	GC1004_R_Reg,

    }
};


