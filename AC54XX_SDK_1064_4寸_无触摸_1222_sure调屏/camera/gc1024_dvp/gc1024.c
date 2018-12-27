
/*******************************************************************************************
 File Name: GC1024.c

 Version: 1.00

 Discription: GC1024

 Author:xuhuajun

 Email :

 Date:2014.8.19

 Copyright:(c)JIELI  2011  @ , All Rights Reserved.
*******************************************************************************************/
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "gc1024.h"
#include "asm/isp_alg.h"

static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};

extern void *GC1024_get_ae_params();
extern void *gc1024_get_awb_params();
extern void *gc1024_get_iq_params();

extern void gc1024_ae_ev_init(u32 fps);
static u32 cur_gain = -1;
u32 sensor_id = 0;

#define NOT_MAX_GAIN 	0XFF
#define IS_MAX_GAIN		0XF1

#define ROTALL_180 1
typedef struct {
    u8 addr;
    u8 value;
} Sensor_reg_ini;

Sensor_reg_ini GC1024_INI_REG[] = {
    {0xfe, 0x80},
    {0xfe, 0x80},
    {0xfe, 0x80},
    {0xf2, 0x02},
    {0xf6, 0x00},
    {0xfc, 0xc6},
    {0xf7, 0xb9}, //pll enable
    {0xf8, 0x03}, //Pll mode 2
    {0xf9, 0x0e}, //{0xf9,0x2e},//20140901
    {0xfa, 0x00},
    {0xfe, 0x00},
/////////////////////////////////////////////////////
////////////////,0x ANALOG & CISCTL,0x ////////////////
/////////////////////////////////////////////////////
    {0x03, 0x02},
    {0x04, 0xb5},
	{0x05,0x01},
	{0x06,0x6f},//78
	{0x07,0x00},
	{0x08,0x07},//14
	{0x0d,0x02}, //height	728
	{0x0e,0xd8},
	{0x0f,0x05}, //widths	1288
	{0x10,0x08},
	{0x11,0x00},
	{0x12,0x18}, //sh_delay
    {0x17, 0x14}, //reverse
    {0x19, 0x06},
    {0x1b, 0x4f},
    {0x1c, 0x41},
    {0x1d, 0xe0},
    {0x1e, 0xfc},
    {0x1f, 0x38},
    {0x20, 0x81},
    {0x21, 0x2f}, //6f//2f
    {0x22, 0xc2},
    {0x23, 0x32},
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
    {0x3f, 0x18},
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
    {0x95, 0x02},
    {0x96, 0xd0},
    {0x97, 0x05},
    {0x98, 0x00},
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
    {0xfe, 0x00},
    {0xcc, 0x25},
    {0xce, 0xf3},
    {0x3f, 0x18},
/////////////////////////////////////////////////////
//////////////////////	 Gain	/////////////////////
/////////////////////////////////////////////////////
    {0xfe, 0x00},
    {0xb0, 0x50},
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
};

Sensor_reg_ini GC1064_INI_REG[] = {
	{0xfe, 0x80},
	{0xfe, 0x80},
	{0xfe, 0x80},
	{0xf2, 0x02},
	{0xf6, 0x00},
	{0xfc, 0xc6},
	{0xf7, 0xb9}, //pll enable
	{0xf8, 0x03}, //Pll mode 2
	{0xf9, 0x0e}, //{0xf9,0x2e},//20140901
	{0xfa, 0x00},
	{0xfe, 0x00},
	/////////////////////////////////////////////////////
	////////////////,0x ANALOG & CISCTL,0x ////////////////
	/////////////////////////////////////////////////////
	{0x03, 0x02},
	{0x04, 0xb5},
	{0x05,0x01},
	{0x06,0x78},
	{0x07,0x00},
	{0x08,0x14},
	{0x0d,0x02}, //height	728
	{0x0e,0xd8},
	{0x0f,0x05}, //widths	1288
	{0x10,0x08},
	{0x11,0x00},
	{0x12,0x18}, //sh_delay

	{0x16,0xc0},
	#if ROTALL_180
	{	0x17,0x17}, //01//14//[0]mirror [1]flip
	#else
	{0x17,0x14}, //01//14//[0]mirror [1]flip
	#endif
	{0x19,0x06},
	{0x1b,0x4f},
	{0x1c,0x11},
	{0x1d,0x10},//exp<1frame Í¼ÏñÉÁË¸
	{0x1e,0xf8},//fc ×ó²à·¢×Ï
	{0x1f,0x38},
	{0x20,0x81},
	{0x21,0x1f},//6f//2f
	{0x22,0xc0},//c2 ×ó²à·¢×Ï
	{0x23,0x82},//f2 ×ó²à·¢×Ï
	{0x24,0x2f},
	{0x25,0xd4},
	{0x26,0xa8},
	{0x29,0x3f},//54//3f
	{0x2a,0x00},
	{0x2b,0x00},//00--powernoise  03---Ç¿¹âÍÏÎ²
	{0x2c,0xe0},//×óÓÒrange²»Ò»ÖÂ
	{0x2d,0x0a},
	{0x2e,0x00},
	{0x2f,0x16},//1f--ºáÉÁÏß
	{0x30,0x00},
	{0x31,0x01},
	{0x32,0x02},
	{0x33,0x03},
	{0x34,0x04},
	{0x35,0x05},
	{0x36,0x06},
	{0x37,0x07},
	{0x38,0x0f},
	{0x39,0x17},
	{0x3a,0x1f},
	{0x3f,0x18},//¹ØµôVclamp µçÑ¹
	/////////////////////////////////////////////////////
	//////////////////////,0x ISP,0x //////////////////////
	/////////////////////////////////////////////////////
	{0xfe, 0x00},
	{0x8a, 0x00},
	{0x8c, 0x06},
	{0x8e, 0x02},
	{0x8f, 0x15}, //added on 20150901 for 50fps
	{0x90, 0x01}, //crop window mode
	#if ROTALL_180
	{0x92,0x01},
	{0x94,0x03},
	#else
	{0x92,0x00},
	{0x94,0x02},
	#endif
	{0x95,0x02},
	{0x96,0xd0},
	{0x97,0x05},
	{0x98,0x00},
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
	{0x18, 0x02},
	{0x1a, 0x11},
	{0x40, 0x23}, //2b
	{0x41,0x00},
	{0x5e, 0x00},
	{0x66, 0x80},
	/////////////////////////////////////////////////////
	////////////////////// Dark SUN /////////////////////
	/////////////////////////////////////////////////////
	{0xfe, 0x00},
	{0xcc, 0x25},
	{0xce, 0xf3},
	{0x3f, 0x18},
	/////////////////////////////////////////////////////
	//////////////////////	 Gain	/////////////////////
	/////////////////////////////////////////////////////
	{0xfe, 0x00},
	{0xb0, 0x50},
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
};
static void *iic = NULL;


unsigned char wrGC1024Reg(unsigned char regID, unsigned char regDat)
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

unsigned char rdGC1024Reg(unsigned char regID, unsigned char *regDat)
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

unsigned char wrGC1024RegArray(u8 *array, u32 size)
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



void GC1024_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;



    if(sensor_id==0x1024)
    {
		for (i = 0; i < sizeof(GC1064_INI_REG) / sizeof(Sensor_reg_ini); i++)
		{
			wrGC1024Reg(GC1064_INI_REG[i].addr, GC1064_INI_REG[i].value);
		}
			}
	else if(sensor_id == 0x1004)
	{
		for (i = 0; i < sizeof(GC1024_INI_REG) / sizeof(Sensor_reg_ini); i++)
		{
			wrGC1024Reg(GC1024_INI_REG[i].addr, GC1024_INI_REG[i].value);
		}
	}

	if (*frame_freq == 25){
		wrGC1024Reg(0x08,0xad); //0x9c
	}else if (*frame_freq == 30){
		wrGC1024Reg(0x08,0x14);
	}
    return;
}


s32 GC1024_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}


s32 GC1024_power_ctl(u8 isp_dev, u8 is_work)
{

    return 0;
}

s32 GC1024_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    for (i = 0; i < 3; i++) { //
        rdGC1024Reg(0xf0, &pid);
        rdGC1024Reg(0xf1, &ver);
        printf("pid value is  %x  ver value is %x    i value is %d\n",pid,ver,i);
    }

    puts("Sensor PID \n");
    put_u8hex(pid);
    put_u8hex(ver);
    puts("\n");

    sensor_id = (pid << 8) | ver;
    printf("pid value is  %x  ver value is %x \n",pid,ver);
     if (sensor_id == 0x1004) {
        puts("\n hello GC1024-----\n");
        return 0;
    } else if (sensor_id == 0x1024){
        puts("\n hello GC1064-----\n");
        return 0;
	}

    puts("\n----not GC1024 or GC1064-----\n");
    return -1;
}

void GC1024_reset(u8 isp_dev)
{
    u32 reset_gpio;
    u32 pwdn_gpio;

    if (isp_dev == ISP_DEV_0) {
        reset_gpio = reset_gpios[0];
        pwdn_gpio = pwdn_gpios[0];
    } else {
        reset_gpio = reset_gpios[1];
        pwdn_gpio = pwdn_gpios[1];
    }

    gpio_direction_output(pwdn_gpio, 1);
    gpio_direction_output(reset_gpio, 0);
    delay(40000);
    gpio_direction_output(reset_gpio, 1);
    gpio_direction_output(pwdn_gpio, 0);
    delay(40000);
}


static u8 cur_sensor_type = 0xff;

s32 GC1024_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio)
{
    puts("\n\n GC1024_check \n\n");
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

    reset_gpios[isp_dev] = _reset_gpio;
    pwdn_gpios[isp_dev] = pwdn_gpio;

    GC1024_reset(isp_dev);

    puts("gc1024 or gc1064 id check\n");
    if (0 != GC1024_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}

s32 GC1024_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n gc1024 or gc1064 init \n\n");

    GC1024_config_SENSOR(width, height, format, frame_freq);

    return 0;
}

u32 gc1024_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 text_align , ratio;
    u32 line = exp_time_us * 48 /2096;

    //gain = (gain - 1024) * 70 / 100 + 1024;
    if (!line)
        line = 1;

    text_align = line *2096 / 48;
    ratio = exp_time_us *1024 / text_align;
    if (ratio < 1024)
        ratio = 1024;
    shutter->ae_exp_line =  line;
    shutter->ae_gain = (gain>>5) * (ratio >> 5);
    shutter->ae_exp_clk = 0;
	//printf("exp_time_us = %d\n",exp_time_us);
    return 0;

}

#define GC1024_ANALOG_GAIN_0   1024 // 1.00x
#define GC1024_ANALOG_GAIN_1   1679 // 1.65x
#define GC1024_ANALOG_GAIN_2   1904 // 1.87x
#define GC1024_ANALOG_GAIN_3   3297 // 3.08x
#define GC1024_ANALOG_GAIN_4   3758 // 3.5x
#define GC1024_ANALOG_GAIN_5   6369 // 5.82x
#define GC1024_ANALOG_GAIN_6   7444 // 6.7x
#define GC1024_ANALOG_GAIN_7   12400 // 10.7x
#define GC1024_ANALOG_GAIN_8   18380 // 15.8x
#define GC1024_ANALOG_GAIN_9   25436 // 21.4x
#define GC1024_ANALOG_GAIN_10  37427 // 30.8x

#define  GC1024_DGAIN_PRECISE  64

static u32 gc1024_gain_steps[] = {
    GC1024_ANALOG_GAIN_0,
    GC1024_ANALOG_GAIN_1,
    GC1024_ANALOG_GAIN_2,
    GC1024_ANALOG_GAIN_3,
    GC1024_ANALOG_GAIN_4,
    GC1024_ANALOG_GAIN_5,
    GC1024_ANALOG_GAIN_6,
    GC1024_ANALOG_GAIN_7,
    GC1024_ANALOG_GAIN_8,
    GC1024_ANALOG_GAIN_9,
    GC1024_ANALOG_GAIN_10,
};

#define GC1064_ANALOG_GAIN_0 1024    // 1.00x
#define GC1064_ANALOG_GAIN_1 1689    // 1.65x
#define GC1064_ANALOG_GAIN_2 1914    // 1.87x
#define GC1064_ANALOG_GAIN_3 3153    // 3.08x
#define GC1064_ANALOG_GAIN_4 3584    // 3.5x
#define GC1064_ANALOG_GAIN_5 5959    // 5.82x
#define GC1064_ANALOG_GAIN_6 6860    // 6.7x
#define GC1064_ANALOG_GAIN_7 10956   // 10.7x
#define GC1064_ANALOG_GAIN_8 16179   // 15.8x
#define GC1064_ANALOG_GAIN_9 21913   // 21.4x
#define GC1064_ANALOG_GAIN_10 31539  // 30.8x

#define  GC1064_DGAIN_PRECISE  64

static u32 gc1064_gain_steps[] = {
    GC1064_ANALOG_GAIN_0,
    GC1064_ANALOG_GAIN_1,
    GC1064_ANALOG_GAIN_2,
    GC1064_ANALOG_GAIN_3,
    GC1064_ANALOG_GAIN_4,
    GC1064_ANALOG_GAIN_5,
    GC1064_ANALOG_GAIN_6,
    GC1064_ANALOG_GAIN_7,
    GC1064_ANALOG_GAIN_8,
    GC1064_ANALOG_GAIN_9,
    GC1064_ANALOG_GAIN_10,
};



static void GC1024_decompose_gain(int total_gain/*q10*/, u32 *a_gain, u32 *d_gain)
{
    int i = 0;
    int size=0;

    if(sensor_id == 0x1004)
    {
      size = sizeof(gc1024_gain_steps) / sizeof(u32);
       for (i = 0; i < size; i++) {
        if (gc1024_gain_steps[i] > total_gain) {
            break;
        }
       }

        i = i - 1;
    if (i < 0) {
        i = 0;
    }
        *a_gain = i;
        *d_gain =GC1024_DGAIN_PRECISE * total_gain / gc1024_gain_steps[i];
	}
	else if(sensor_id == 0x1024)
    {
     size = sizeof(gc1064_gain_steps) / sizeof(u32);

    for (i = 0; i < size; i++) {
        if (gc1064_gain_steps[i] > total_gain) {
            break;
        }
    }

        i = i - 1;
    if (i < 0) {
        i = 0;
    }

        *a_gain = i;
        *d_gain =GC1064_DGAIN_PRECISE * total_gain / gc1064_gain_steps[i];

    }


}

static u8 gc1024_shutter_cmd[6][3] = {
    {0x78, 0xb6, 0x00},  //again
    {0x78, 0xb1, 0x00},  //dgainL
    {0x78, 0xb2, 0x00},  //dgainH
    {0x78, 0xb0, 0x00},  //ggaim
    {0x78, 0x04, 0x00},  //exptime L
    {0x78, 0x03, 0x00},   //exptime H
} ;

void GC1024_W_Reg(u16 addr, u16 val);
u32 gc1024_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 gain;
    u32 again, dgain;
	int i;

    //if(shutter->ae_exp_line<8) shutter->ae_exp_line=8;

    gain = (shutter->ae_gain);

    GC1024_decompose_gain(gain, &again, &dgain);

    //printf("time=%d, again=%d, dgain=%d\n", shutter->ae_exp_line, again, dgain);

    gc1024_shutter_cmd[0][2] = again ;
    gc1024_shutter_cmd[1][2] = dgain >> 6 ;

    gc1024_shutter_cmd[2][2] = (dgain << 2) & 0xfc;

    gc1024_shutter_cmd[3][2] = 0x40 ;

    gc1024_shutter_cmd[4][2] = shutter->ae_exp_line & 0xFF ;
    gc1024_shutter_cmd[5][2] = (shutter->ae_exp_line >> 8) & 0x1F;

    //wrGC1024RegArray(gc1024_shutter_cmd[0], 18);
	for (i = 0; i < 6; i++){
		GC1024_W_Reg(gc1024_shutter_cmd[i][1],gc1024_shutter_cmd[i][2]);
	}


    return 0;
}

void GC1024_sleep()
{

    wrGC1024Reg(0xf6, 0x01);
}

void GC1024_wakeup()
{

    wrGC1024Reg(0xf6, 0x00);
}

void GC1024_W_Reg(u16 addr, u16 val)
{
    //printf("update reg%x with %x\n", addr, val);
    wrGC1024Reg((u8)addr, (u8)val);
}
u16 GC1024_R_Reg(u16 addr)
{
    u8 val;
    rdGC1024Reg((u8)addr, &val);
    return val;
}



REGISTER_CAMERA(GC1024) = {
    .logo 				= 	"GC1024",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_RGGB,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B,
    .fps         		= 	25,

    .sen_size 			= 	{GC1024_OUTPUT_W, GC1024_OUTPUT_H},
    .isp_size 			= 	{GC1024_OUTPUT_W, GC1024_OUTPUT_H},

    .cap_fps         		= 	25,
    .sen_cap_size 			= 	{GC1024_OUTPUT_W, GC1024_OUTPUT_H},
    .isp_cap_size 			= 	{GC1024_OUTPUT_W, GC1024_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	GC1024_check,
        .init 		        = 	GC1024_init,
        .set_size_fps 		=	GC1024_set_output_size,
        .power_ctrl         =   GC1024_power_ctl,

        .get_ae_params  	=	GC1024_get_ae_params,
        .get_awb_params 	=	gc1024_get_awb_params,
        .get_iq_params 	    =	gc1024_get_iq_params,

        .sleep 		        =	GC1024_sleep,
        .wakeup 		    =	GC1024_wakeup,
        .write_reg 		    =	GC1024_W_Reg,
        .read_reg 		    =	GC1024_R_Reg,

    }
};


